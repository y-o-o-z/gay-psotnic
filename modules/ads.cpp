/* advertisement module for psotnic (test version)
   written for: www.mioprofilo.it

   commands:
             query bot (only for global +n):
              !textadd <#channel> <delay> [%expiration_time] <text>
              !textremove <#channel> <pattern | number>
              !textlist <channel>

             message on channel (also for local +n):
              !textadd <delay> [%expiration_time] <text>
              !textremove <pattern | number>
              !textlist

   examples:
             !textadd #chan 10 %1d hello. what's up?
               - sends "hello. what's up?" to #chan every 10 minutes, expires in 1 day
             !textremove #chan *hello*
               - removes all messages that contain the word "hello"
             !textremove #chan 1
               - removes first entry (use !textlist to get the numbers)

   hints:
          * this module requires psotnic 0.2.14 or newer
          * messages to random people are disabled by default
*/


#include <prots.h>
#include <global-var.h>

// configuration
// all values in seconds

// minimum delay between messages on channel
#define AD_MSG_MIN_DELAY 10

#define AD_SAVE_DELAY 10

// if enabled, the bot will message random people which are on the channel
#undef MSG_RAND_PEOPLE

#ifdef MSG_RAND_PEOPLE
    // delay between messages
    #define AD_MSG_RAND_PPL_DELAY 20

    // message people that are not longer idle than this time
    #define AD_MAX_IDLE_RAND_PPL  60
#endif

// end of configuration

#define AD_BSIZE 512

time_t ADS_NEXT_SAVE;
module *m;
fifo *ads_queue;

void _privmsg(const char *, const char *, ...);
void _notice(const char *, const char *, ...);
bool has_global_flag(const char *, int);
void load_ads();
void save_ads();
#ifdef MSG_RAND_PEOPLE
void msg_people();
#endif

class ad : public CustomDataObject
{
 public:
    class entry
    {
     public:
	entry(const char *, int, time_t);

        int delay;
        time_t expiration;
        char text[AD_BSIZE];
	time_t sendtime;
    };

    ptrlist<entry> data;
    ptrlist<entry>::iterator current;
    CHANLIST *cl;
    time_t lastsend;

    ad(CHANLIST *);
    bool add(const char *, int, time_t, bool justAdded=false);
    int remove(const char *);
    int remove(int);
    bool find(const char *);
    void show(const char *); 
    void writeFile(FILE *);
    void sendToChan();
#ifdef MSG_RAND_PEOPLE
    void sendToUser(const char*);
#endif
    void checkExpiration();
};

ad::entry::entry(const char *_text, int _delay, time_t _expiration)
{
    strncpy(this->text, _text, AD_BSIZE-1);
    this->text[AD_BSIZE-1]='\0';
    this->delay=_delay;
    this->expiration=_expiration;
}

ad::ad(CHANLIST *_cl)
{
    cl=_cl;
    current=NULL;
    lastsend=0;
    data.removePtrs();
}

bool ad::add(const char *text, int delay, time_t expiration, bool justAdded)
{
    // look for duplicates
    if(find(text))
        return false;

    entry *e=new entry(text, delay, expiration);

    if(justAdded) // added by !textadd, send ad right now
      e->sendtime=NOW;

    else
      e->sendtime=NOW+(e->delay * 60);

    data.addLast(e);
    return true;
}

int ad::remove(const char *pattern)
{
    int cnt=0, number=0;
    ptrlist<entry>::iterator i=data.begin(), j;

    // delete by number
    if(isdigit((int)pattern[0]))
        number=atoi(pattern);

    if(number)
        return remove(number);

    // delete by pattern
    while(i)
    {
        j=i;
        j++;

        if(match(pattern, i->text))
        {
            data.remove(i);
            cnt++;
        }

        i=j;
    }

    return cnt;
}

int ad::remove(int number)
{
    int cnt=1;
    ptrlist<entry>::iterator i;

    for(i=data.begin(); i; i++)
    {
        if(cnt==number)
        {
            data.remove(i);
            return 1; 
        }
        cnt++;
    }

    return 0;
}

bool ad::find(const char *_text)
{
    ptrlist<entry>::iterator i;

    for(i=data.begin(); i; i++)
    {
        if(!strcmp(i->text, _text))
            return true;
    }

    return false;
}

void ad::show(const char *nick)
{
    ptrlist<entry>::iterator i;
    int cnt=1;
    const char *exp_str;

    _notice(nick, "advertisements for %s:", (const char*) cl->name);

    for(i=data.begin(); i; i++)
    {
        if(i->expiration)
            exp_str=timestr("%d/%m/%Y %T", i->expiration);

        else
            exp_str="never";

        _notice(nick, "[%d] delay: %d expiration: %s - %s", cnt++, i->delay, exp_str, i->text);
    }
}

void ad::sendToChan()
{
    if(!current)
        current=data.begin();

    while(current)
    {
	if(NOW>=current->sendtime)
        {
            if(NOW-lastsend<AD_MSG_MIN_DELAY)
               break;

            _privmsg(cl->name, current->text);
            current->sendtime=NOW+(current->delay * 60);
            lastsend=NOW;
            current++;
            break;
        }

        else
            current++;
    }
}
#ifdef MSG_RAND_PEOPLE
void ad::sendToUser(const char *nick)
{
    ptrlist<entry>::iterator i;
    int cnt=0;

    for(i=data.begin(); i; i++)
    {
        if(cnt == 2) // limit
            break;

        privmsg(nick, i->text);
        cnt++;
    }
}
#endif
void ad::writeFile(FILE *fh)
{
    ptrlist<entry>::iterator i;

    for(i=data.begin(); i; i++)
	fprintf(fh, "%s %d %ld %s\n", (const char*) cl->name, i->delay, i->expiration, i->text);
}

void ad::checkExpiration()
{
    ptrlist<entry>::iterator i, j;

    for(i=data.begin(); i; i=j)
    {
        j=i;
        j++;

        if(i->expiration && NOW > i->expiration)
        {
            data.remove(i);
            ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY;
        }
    }
}

#ifdef MSG_RAND_PEOPLE
class info : public CustomDataObject
{
 public:
    info()
    {
        lastspoke=0;
        gotmsg=false;
    }

    void updateLastspoke()
    {
        lastspoke=NOW;
    }

    time_t lastspoke;
    bool gotmsg;
};
#endif

chan *AD_CURRENT_CHAN;
#ifdef MSG_RAND_PEOPLE
time_t ADS_NEXT_MSG_RAND_PPL;
#endif

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    CHANLIST *cl;
    char arg[10][MAX_LEN];
    int x;

    if(match(ME.nick, to)) // query
    {
        if(!has_global_flag(from, HAS_N))
            return;

        str2words(arg[0], msg, 10, MAX_LEN, 0);

        if(match("!textadd", arg[0]))
        {
            int delay, expiration=0;

	    if(!strlen(arg[3]))
            {
                _notice(from, "!textadd <#channel> <delay> [%%expiration_time] <text>");
                return;
            }

            if(!(cl=userlist.findChanlist(arg[1])))
            {
                _notice(from, "channel `%s' has not been found.", arg[1]);
                return;
            }

	    delay=atoi(arg[2]);

            if(delay<=0)
            {
                _notice(from, "delay must be greater than 0.");
                return;
            }

            if(arg[3][0] == '%')
            {
                if(arg[3][1] && units2int(arg[3]+1, ut_time, expiration) == 1 && expiration > 0)
                    expiration+=NOW;

                else
                    _notice(from, "Invalid expiration time");
            }

            if(((ad*)cl->customData(m->desc))->add(srewind(msg, expiration ? 4 :3), delay, expiration, true))
            {
                _notice(from, "advertisement has been added on channel `%s'.", (const char*) cl->name);
                ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY;
            }

            else
                _notice(from, "this advertisement is already added.");
        }

        else if(match("!textremove", arg[0]))
        {
            if(!strlen(arg[2]))
            {
                _notice(from, "!textremove <#channel> <pattern | number>");
                return;
            }

            if(!(cl=userlist.findChanlist(arg[1])))
            {
                _notice(from, "channel `%s' has not been found.", arg[1]);
                return;
            }

            if((x=((ad*)cl->customData(m->desc))->remove(arg[2]))>0)
            {
                _notice(from, "%d entries have been deleted.", x);
                ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY;
            }

            else
                _notice(from, "delete failed.");
        }

        else if(match("!textlist", arg[0]))
        {
            if(strlen(arg[1]))
            {
                if(!(cl=userlist.findChanlist(arg[1])))
                {
                    _notice(from, "channel `%s' has not been found.", arg[1]);
                    return;
                }

                ((ad*)cl->customData(m->desc))->show(from);
            }

            else
                _notice(from, "!textlist <channel>");

            /*else
            { 
                for(int i=0; i<MAX_CHANNELS; i++)
                {
                    if(userlist.chanlist[i].name)
                        ((ad*)userlist.chanlist[i].customData(m->desc))->show(from);
                }
            }*/
        }
    }

    else
    {
        chan *ch;
        chanuser *u;

        if(!(ch=ME.findChannel(to)))
            return;

        if(!(u=ch->getUser(from)))
            return;
#ifdef MSG_RAND_PEOPLE
        ((info*)u->customData(m->desc))->updateLastspoke();
#endif
        if(!(u->flags & HAS_N)) // allow local +n too
            return;

        str2words(arg[0], msg, 10, MAX_LEN, 0);

        if(match("!textadd", arg[0]))
        {
            int delay, expiration=0;

	    if(!strlen(arg[2]))
            {
                _notice(from, "!textadd <delay> [%%expiration_time] <text>");
                return;
            }

            if(!(cl=userlist.findChanlist(ch->name)))
                return;

	    delay=atoi(arg[1]);

            if(delay<=0)
            {
                _notice(from, "delay must be greater than 0.");
                return;
            }

            if(arg[2][0] == '%')
            {
                if(arg[2][1] && units2int(arg[2]+1, ut_time, expiration) == 1 && expiration > 0)
                    expiration+=NOW;

                else
                    _notice(from, "Invalid expiration time");
            }

            if(((ad*)cl->customData(m->desc))->add(srewind(msg, expiration ? 3 : 2), delay, expiration, true))
            {
                _notice(from, "advertisement has been added on channel `%s'.", (const char*) cl->name);
                ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY;
            }

            else
                _notice(from, "this advertisement is already added.");
        }

        else if(match("!textremove", arg[0]))
        {
            if(!strlen(arg[1]))
            {
                _notice(from, "!textremove <pattern | number>");
                return;
            }

            if(!(cl=userlist.findChanlist(ch->name)))
                return;

            if((x=((ad*)cl->customData(m->desc))->remove(arg[1]))>0)
            {
                _notice(from, "%d entries have been deleted.", x);
                ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY;
            }

            else
                _notice(from, "delete failed.");
        }

        else if(match("!textlist", arg[0]))
        {
            if(!(cl=userlist.findChanlist(ch->name)))
                return;

            ((ad*)cl->customData(m->desc))->show(from);
        }
    }
}

void hook_userlistLoaded()
{
    /* we load ads.txt here because the module is loaded before the userlist,
       so we cannot load it in init(). also chanlist's customData gets lost every
       time the bot receives an userlist.
    */

    load_ads();
}

void hook_timer()
{
    for(int i=0; i<MAX_CHANNELS; i++)
    {
        if(userlist.chanlist[i].name)
        {
            chan *ch=ME.findChannel(userlist.chanlist[i].name);

            if(ch && ch->synlevel>=9)
                ((ad*)userlist.chanlist[i].customData(m->desc))->sendToChan();

            ((ad*)userlist.chanlist[i].customData(m->desc))->checkExpiration();
        } 
    }

#ifdef MSG_RAND_PEOPLE
    if(NOW>=ADS_NEXT_MSG_RAND_PPL)
    {
       msg_people();
       ADS_NEXT_MSG_RAND_PPL=NOW+AD_MSG_RAND_PPL_DELAY;
    }
#endif

    if(ADS_NEXT_SAVE!=0 && NOW>=ADS_NEXT_SAVE)
        save_ads();

    if(penalty < 5)
        if(ads_queue->flush(&net.irc))
            penalty+=2;
}

void load_ads()
{
    FILE *fh;
    CHANLIST *ch;
    char buffer[AD_BSIZE];
    char arg[10][MAX_LEN];
    char conf_file[MAX_LEN];

    snprintf(conf_file, MAX_LEN, "%s/ads.txt", MODULES_DIR);
    net.send(HAS_N, "[*] (re-)loading ads.txt");

    if(!(fh=fopen(conf_file, "r")))
    {
        net.send(HAS_N, "[!] error while loading ads: cannot open %s: %s", conf_file, strerror(errno));
        return;
    }

    while((fgets(buffer, AD_BSIZE, fh)))
    {
       buffer[strlen(buffer)-1]='\0';
       str2words(arg[0], buffer, 10, MAX_LEN, 0);

        if(!strlen(arg[2]))
            continue;

        if(!(ch=userlist.findChanlist(arg[0])))
            continue;

        ((ad*)ch->customData(m->desc))->add(srewind(buffer, 3), atoi(arg[1]), atoi(arg[2]), false);
    }

    fclose(fh);
}

void save_ads()
{
    FILE *fh;
    CHANLIST *cl;
    char conf_file[MAX_LEN];

    snprintf(conf_file, MAX_LEN, "%s/ads.txt", MODULES_DIR);

    if(!(fh=fopen(conf_file, "w")))
    {
        net.send(HAS_N, "[!] error while saving ads: cannot open %s: %s", conf_file, strerror(errno));
        ADS_NEXT_SAVE=NOW+AD_SAVE_DELAY; // try again later
        return;
    }

    for(int i=0; i<MAX_CHANNELS; i++)
    {
        if((cl=userlist.findChanlist(userlist.chanlist[i].name)))
	    ((ad*)cl->customData(m->desc))->writeFile(fh);
    }

    fclose(fh);
    ADS_NEXT_SAVE=0;
}

#ifdef MSG_RAND_PEOPLE
void msg_people()
{
    ptrlist<chanuser>::iterator u;
    chanuser *cu;
    CHANLIST *cl;

    if(!AD_CURRENT_CHAN)
        AD_CURRENT_CHAN=ME->first;

    if(!AD_CURRENT_CHAN)
        return; // not connected?

    for(u=AD_CURRENT_CHAN->users.begin(); u; u++)
    {
        if((cu=AD_CURRENT_CHAN->getUser(u->nick)))
        {
            if(cu->flags & (HAS_O | HAS_V | HAS_F))
                continue;
            if(((info*)cu->customData(m->desc))->gotmsg)
                continue;
            if(NOW-((info*)cu->customData(m->desc))->lastspoke > AD_MAX_IDLE_RAND_PPL)
                continue;
            if(!(cl=userlist.findChanlist(AD_CURRENT_CHAN->name)))
                continue;

            ((ad*)cl->customData(m->desc))->sendToUser(cu->nick);

            ((info*)cu->customData(m->desc))->gotmsg=true;
            break;
        }
    }

    AD_CURRENT_CHAN=AD_CURRENT_CHAN->next;
}
#endif

bool has_global_flag(const char *mask, int flag)
{
    HANDLE *h;

    for(h=userlist.first; h; h=h->next)
    {
        if(!(h->flags[GLOBAL] & flag))
            continue;

        for(int i=0; i<MAX_HOSTS; i++)
            if (h->host[i] && match(h->host[i], mask))
                return true;
    }

    return false;
}

void chanlistConstructor(CHANLIST *me)
{
    me->setCustomData(m->desc, new ad(me));
}

void chanlistDestructor(CHANLIST *me)
{
    if(me->customData(m->desc))
        delete (ad *)me->customData(m->desc);
}
#ifdef MSG_RAND_PEOPLE
void chanuserConstructor(chanuser *me)
{
    me->setCustomData(m->desc, new info);
}

void chanuserDestructor(chanuser *me)
{
    if(me->customData(m->desc))
        delete (info *)me->customData(m->desc);
}
#endif
void prepareCustomData()
{
#ifdef MSG_RAND_PEOPLE
    chan *ch;
    ptrlist<chanuser>::iterator u;
    chanuser *cu;
#endif
    for(int i=0; i<MAX_CHANNELS; i++)
    {
        chanlistConstructor(&(userlist.chanlist[i]));
#ifdef MSG_RAND_PEOPLE
        if((ch=ME.findChannel(userlist.chanlist[i].name)))
        {
            for(u=ch->users.begin(); u; u++)
                if((cu=ch->getUser(u->nick)))
                    chanuserConstructor(cu);
        }
#endif
    }
}

void _privmsg(const char *to, const char *msg, ...)
{
    va_list list;
    char buffer[AD_BSIZE];

    if(!(msg) || (*(msg) == '\0'))
        return;

    va_start(list, msg);
    vsnprintf(buffer, AD_BSIZE, msg, list);
    va_end(list);
    ads_queue->push("PRIVMSG %s :%s", to, buffer);
}

void _notice(const char *to, const char *msg, ...)
{
    va_list list;
    char buffer[AD_BSIZE];

    if(!(msg) || (*(msg) == '\0'))
        return;

    va_start(list, msg);
    vsnprintf(buffer, AD_BSIZE, msg, list);
    va_end(list);
    ads_queue->push("NOTICE %s :%s", to, buffer);
}
extern "C" module *init()
{
    m=new module("advertisement", "patrick <patrick@psotnic.com>", "0.1-pre10)");

    prepareCustomData();
    m->hooks->privmsg=hook_privmsg;
    m->hooks->timer=hook_timer;
    m->hooks->userlistLoaded=hook_userlistLoaded;
    m->hooks->new_CHANLIST=chanlistConstructor;
    m->hooks->del_CHANLIST=chanlistDestructor;
#ifdef MSG_RAND_PEOPLE
    m->hooks->new_chanuser=chanuserConstructor;
    m->hooks->del_chanuser=chanuserDestructor;
#endif
    // for the case that the module is loaded on runtime
    if(userlist.chanlist[0].name)
        load_ads();

    AD_CURRENT_CHAN=NULL;
    ADS_NEXT_SAVE=0;
#ifdef MSG_RAND_PEOPLE
    ADS_NEXT_MSG_RAND_PPL=NOW+300;
#endif

    ads_queue=new fifo(100, 3);
    return m;
}

extern "C" void destroy()
{
    delete ads_queue;
}

