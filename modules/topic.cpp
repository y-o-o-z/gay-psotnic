// tries to make sure that the topic does not get lost

#include <prots.h>
#include <global-var.h>

#define TOPIC_SET_DELAY 28800
#define TOPIC_FILE "topics.txt"

chan *__findChannel(const char *);

module *m;

struct topicInfo : public CustomDataObject
{
    topicInfo() : CustomDataObject()
    {
        setNextSet();
    }

    void setNextSet()
    {
        nextSet=NOW+TOPIC_SET_DELAY;
    }

    void setNextSet(time_t t)
    {
        nextSet=t;
    }

    time_t nextSet;
};

struct _topicCache
{
    struct entry
    {
        entry(const char *_channel, const char *_topic)
        {
            channel=_channel;
            topic=_topic;
        }

        pstring<> channel;
        pstring<> topic;
    };

    _topicCache()
    {
        nextSave=0;
        data.removePtrs();
    }

    void add(const char *_channel, const char *_topic, bool save=true)
    {
        entry *e;

        if((e=find(_channel)))
            e->topic=_topic;

        else
        {
            entry *e=new entry(_channel, _topic);
            data.add(e);
        }

        if(save)
            setNextSave();
    }

    entry *find(const char *_channel)
    {
        ptrlist<entry>::iterator i;

        for(i=data.begin(); i; i++)
        {
            if(!strcasecmp(i->channel, _channel))
                return i;
        }

        return NULL;
    }

    void remove(const char *_channel)
    {
        entry *e=find(_channel);

        if(e)
            data.remove(e);

        setNextSave();
    }

    const char *findTopic(const char *_channel)
    {
        entry *e=find(_channel);

        if(e)
            return e->topic;
        else
            return NULL;
    }

    void load()
    {
        FILE *fh;
        char buffer[MAX_LEN], channel[MAX_LEN], *topic;
        int len;

        fh=fopen(TOPIC_FILE, "r");

        if(!fh) return;

         while(fgets(buffer, MAX_LEN, fh))
         {
             len=strlen(buffer);
             buffer[len-1]='\0';
             str2words(channel, buffer, 1, MAX_LEN, 0);
             topic=srewind(buffer, 1);

             if(*channel && topic && *topic)
                 add(channel, topic, false);
         }

         fclose(fh);
    }

    void save()
    {
        FILE *fh;
        ptrlist<entry>::iterator i;

        fh=fopen(TOPIC_FILE, "w");

        if(!fh) return;

        for(i=data.begin(); i; i++)
        {
            if(userlist.findChanlist(i->channel))
                fprintf(fh, "%s %s\n", (const char*) i->channel, (const char*) i->topic);
        }

        fclose(fh);
        nextSave=0;
    }

    void setNextSave()
    {
        nextSave=NOW+SAVEDELAY;
    }

    ptrlist<entry> data;
    time_t nextSave;
};


_topicCache *topicCache;

void hook_timer()
{
    chan *ch;

    for(ch=ME.first; ch && penalty < 5; ch=ch->next)
    {
        if(ch->synced() && ch->me->flags & IS_OP && NOW>=((topicInfo *)ch->customData(m->desc))->nextSet)
        {
            // if the topic is set, just set it again
            if(*ch->topic)
                net.irc.send("TOPIC %s :%s", (const char*) ch->name, (const char*) ch->topic);

            else
            {
                // if the bot doesn't see a topic, search for it in cache
                const char *topic;

                if((topic=topicCache->findTopic(ch->name)))
                     net.irc.send("TOPIC %s :%s", (const char*) ch->name, topic);
            }

            ((topicInfo *)ch->customData(m->desc))->setNextSet();
        }
    }

    if(topicCache->nextSave && NOW >= topicCache->nextSave)
        topicCache->save();
}

void hook_topicChange(chan *ch, const char *topic, chanuser *u, const char *oldtopic)
{
    if(!topic || !*topic)  // topic has been removed
    {
        topicCache->remove(ch->name);
        return;
    }

    topicCache->add(ch->name, topic);
    ((topicInfo *)ch->customData(m->desc))->setNextSet();
}

void hook_justSynced(chan *ch)
{
    // bot joined channel and there is no topic
    if(!*ch->topic)
        ((topicInfo *)ch->customData(m->desc))->setNextSet(NOW+300);
}

void hook_raw(const char *data)
{
    char arg[11][MAX_LEN], *topic;
    str2words(arg[0], data, 11, MAX_LEN, 1);
    time_t t;
    chan *ch;

    if(!strcmp(arg[1], "332")) // RPL_TOPIC
    {
        topic=srewind(data, 4)+1;
        topicCache->add(arg[3], topic);
    }

    else if(!strcmp(arg[1], "333")) // RPL_TOPIC_WHO_TIME
    {
        if((ch=__findChannel(arg[3])))
        {
            t=atol(arg[5]);
            ((topicInfo *)ch->customData(m->desc))->setNextSet(t+TOPIC_SET_DELAY);
        }
    }
}

// client::findChannel() requires that the channel is synced, so I use this
chan *__findChannel(const char *name)
{
    chan *ch;

    for(ch=ME.first; ch; ch=ch->next)
        if(!strcasecmp(ch->name, name))
            return ch;

    return NULL;
}

void hook_new_chan(chan *me)
{
    me->setCustomData(m->desc, new topicInfo);
}


void hook_del_chan(chan *me)
{
    if(me->customData(m->desc))
        delete (topicInfo *) me->customData(m->desc);
}

extern "C" module *init()
{
    chan *ch;
    m=new module("topic", "patrick <patrick@psotnic.com>", "0.1");
    topicCache=new _topicCache();
    topicCache->load();

    if(userlist.SN)
        for(ch=ME.first; ch; ch=ch->next)
            hook_new_chan(ch);

    m->hooks->new_chan=hook_new_chan;
    m->hooks->del_chan=hook_del_chan;
    m->hooks->timer=hook_timer;
    m->hooks->topicChange=hook_topicChange;
    m->hooks->justSynced=hook_justSynced;
    m->hooks->rawirc=hook_raw;
    return m;
}

extern "C" void destroy()
{
    chan *ch;

    topicCache->data.clear();

    for(ch=ME.first; ch; ch=ch->next)
        hook_del_chan(ch);
}
