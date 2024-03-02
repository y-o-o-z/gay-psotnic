/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

protmodelist::entry::entry()
{
}

protmodelist::entry::entry(const char *_mask, const char *_by, const char *_reason, time_t _when, time_t _expires, bool _sticky)
{
	mem_strcpy(mask, _mask);
	mem_strcpy(by, _by);
	if(_reason && *_reason)
		mem_strcpy(reason, _reason);
	else
		mem_strcpy(reason, "requested");

	when = _when;
	expires = _expires;
	sticky = _sticky;
}

protmodelist::entry::~entry()
{
	free(mask);
	free(by);
	free(reason);
}

char *protmodelist::entry::fullReason()
{
	static char buf[MAX_LEN];

	if(expires && expires - NOW <= 20*3600)
	{
		snprintf(buf, MAX_LEN, "%s - expires: %s", reason,
			timestr("%T", expires));
	}

	else
	{
		snprintf(buf, MAX_LEN, "%s - expires: %s", reason,
			expires ? timestr("%d/%m/%Y", expires) : "never");
	}

	return buf;
}

protmodelist::protmodelist(int _type, char _mode)
{
	type = _type;
	mode = _mode;
	data.removePtrs();
}

protmodelist::~protmodelist()
{
	data.clear();
}

protmodelist::entry *protmodelist::wildMatch(const char *mask)
{
	ptrlist<entry>::iterator l = data.begin();

	while(l)
	{
		if(::wildMatch(mask, l->mask))
			return l;
		l++;
	}

	return NULL;
}

protmodelist::entry *protmodelist::conflicts(const char *mask)
{
	ptrlist<entry>::iterator i = data.begin();

	while(i)
	{
		if(::match(i->mask, mask))
			return i;
		i++;
	}
	return NULL;
}

protmodelist::entry *protmodelist::match(const chanuser *u)
{
	ptrlist<entry>::iterator l = data.begin();

	while(l)
	{
		if(matchBan(l->mask, u))
			return l;
		l++;
	}
	return NULL;
}

protmodelist::entry *protmodelist::add(const char *mask, const char *by, time_t when, time_t expires, const char *reason, bool sticky)
{
	entry *e = new entry(mask, by, reason, when, expires, sticky);
	data.addLast(e);
	return e;
}

int protmodelist::remove(const char *mask)
{
	ptrlist<entry>::iterator l = data.begin();

	if(*mask == '#')
	{
		if(!*mask || !isdigit(mask[1]))
			return 0;

		return remove(atoi(mask+1));
	}

	while(l)
	{
		if(!strcmp(mask, l->mask))
		{
			data.removeLink(l);
			return 1;
		}
		l++;
	}
	return 0;
}

int protmodelist::remove(int n)
{
	ptrlist<entry>::iterator l = data.getItem(n-1);

	if(l)
	{
		data.removeLink(l);
		return 1;
	}

	return 0;
}

int protmodelist::sendBansToOwner(inetconn *c, const char *name, int i)
{
	//int i = start;
	ptrlist<entry>::iterator s = data.begin();

	if(s)
	{
		if(type == BAN)
			c->send(name, " bans: ");
		else if(type == INVITE)
			c->send(name, " invites: ");
		else if(type == EXEMPT)
			c->send(name, " exempts: ");
		else if(type == REOP)
			c->send(name, " reops: ");

		while(s)
		{
			++i;
			c->send("%s %s %s %s %s %s %s", i > 99 ? "[" : i > 9 ? "[ " : "[  ", itoa(i), "]: ", s->mask,
					" (expires: ", s->expires ?
					timestr("%d/%m/%Y %T", s->expires) : "never", ")");

			c->send("%s %s %s %s", s->sticky ? "[ * ]  " : "       ", s->by, ": ", s->reason);
			c->send("       created: %s", timestr("%d/%m/%Y %T", s->when));
			s++;
		}
	}
	return i;
}

void protmodelist::sendToUserlist(inetconn *c, const char *name)
{
	ptrlist<entry>::iterator s = data.begin();
	const char *botnet_cmd;

	while(s)
	{
		switch(type)
		{
			case BAN : if(s->sticky)
				   	botnet_cmd = S_ADDSTICK;
				   else
					botnet_cmd = S_ADDBAN;
				   break;
			case INVITE : botnet_cmd = S_ADDINVITE; break;
			case EXEMPT : botnet_cmd = S_ADDEXEMPT; break;
			case REOP : botnet_cmd = S_ADDREOP; break;
			default : return;
		}

		c->send("%s %s %s %s %d %d %s", botnet_cmd, name, s->mask, s->by,
				s->when, s->expires, s->reason);
		s++;
	}
}

int protmodelist::sendBansToOwner(inetconn *c, int type, const char *channel, const char *)
{
	int matches = 0;
	CHANLIST *chLst;

	if((!channel || !*channel || !strcmp(channel, "*")) && c->checkFlag(HAS_N))
	{
		matches += userlist.protlist[type]->sendBansToOwner(c, "Global");
	}

	if(channel && *channel)
	{
		foreachMatchingChanlist(chLst, channel)
		{
			if(c->checkFlag(HAS_N) || c->checkFlag(HAS_N, _i))
				matches += chLst->protlist[type]->sendBansToOwner(c, chLst->name);
		}
	}

	if(!matches)
		c->send("No matches has been found");
	else
		c->send("--- Found %d %s", matches, matches == 1 ? " match" : " matches");

	return matches;
}


protmodelist::entry *protmodelist::find(const char *str)
{
	ptrlist<entry>::iterator s = data.begin();

	if(str[0]=='#')
	{
		if(!isdigit(str[1]))
			return NULL;

		if((s=data.getItem(atoi(str+1)-1)))
			return s;
		else
			return NULL;	
	}

	while(s)
	{
		if(!strcmp(str, s->mask))
			return s;

		s++;
	}

	return NULL;
}

int protmodelist::expire(const char *channel)
{
	if(config.bottype != BOT_MAIN)
		return 0;

	ptrlist<entry>::iterator n, s = data.begin();
	int i=0;
	chan *ch;
	const char *botnet_cmd, *type_str;
	char _mode[3];

	_mode[0] = '-';
	_mode[1] = mode;
	_mode[2] = '\0';

	switch(type)
	{
		case BAN : botnet_cmd=S_RMBAN; type_str="ban"; break;
		case INVITE : botnet_cmd=S_RMINVITE; type_str="invite"; break;
		case EXEMPT : botnet_cmd=S_RMEXEMPT; type_str="exempt"; break;
		case REOP : botnet_cmd=S_RMREOP; type_str="reop"; break;
		default : return 0;
	}

	while(s)
	{
		n = s;
		n++;
		if(s->expires && s->expires <= NOW)
		{
			net.send(HAS_B, "%s %s %s", botnet_cmd, s->mask, channel);

			if(channel)
			{
				net.send(HAS_N, "%s %s on %s has expired", type_str, s->mask, channel);
	
				if((ch=ME.findChannel(channel)) && ch->synced() && ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
					ch->modeQ[PRIO_LOW].add(NOW+5, _mode, s->mask);
			}

			else
			{
				net.send(HAS_N, "%s %s has expired", type_str, s->mask);

				foreachSyncedChannel(ch)
					if(ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
						ch->modeQ[PRIO_LOW].add(NOW+5, _mode, s->mask);
			}

			data.removeLink(s);
			userlist.updated(false);
			++i;
		}
		s = n;
	}
	return i;
}

int protmodelist::expireAll()
{
	if(config.bottype != BOT_MAIN)
		return 0;

	CHANLIST *chLst;
	int i=0;

	foreachNamedChanlist(chLst)
	{
		i += chLst->protlist[BAN]->expire(chLst->name);
		i += chLst->protlist[INVITE]->expire(chLst->name);
		i += chLst->protlist[EXEMPT]->expire(chLst->name);
		i += chLst->protlist[REOP]->expire(chLst->name);
	}

	i += userlist.protlist[BAN]->expire();
	i += userlist.protlist[INVITE]->expire();
	i += userlist.protlist[EXEMPT]->expire();
	i += userlist.protlist[REOP]->expire();

	if(i)
		userlist.nextSave = NOW + SAVEDELAY;

	return i;
}

void protmodelist::clear()
{
	data.clear();
}

bool protmodelist::isSticky(const char *mask, int type, const chan *ch)
{
	entry *b = ch->protlist[type]->find(mask);

	if((b && b->sticky) || ((b = userlist.protlist[type]->find(mask)) && b->sticky))
		return true;

	return false;
}

protmodelist::entry *protmodelist::findSticky(const char *mask, int type, const chan *ch)
{
	entry *b = ch->protlist[type]->find(mask);

	if((b && b->sticky) || ((b = userlist.protlist[type]->find(mask)) && b->sticky))
		return b;

	return NULL;
}

protmodelist::entry *protmodelist::findByMask(const char *mask)
{
	ptrlist<entry>::iterator i = data.begin();

	while(i)
	{
		if(!strcmp(i->mask, mask))
			return i;
		i++;
	}
	return NULL;
}

protmodelist::entry *protmodelist::findBestByMask(const char *channel, const char *mask, int type)
{
	protmodelist *s;
	entry *e;

	int chanNum;

	if((chanNum = userlist.findChannel(channel)) != -1)
	{
		s = userlist.chanlist[chanNum].protlist[type];
		e = s->findByMask(mask);
		if(e)
			return e;
	}

	return userlist.protlist[type]->findByMask(mask);
}

/** Adds a ban or tells the main bot to add it.
 * This function should only be used when a bot adds a ban.
 * Please check if set.BOTS_CAN_ADD_BANS is true before you use this function.
 *
 * \author patrick <patrick@psotnic.com>
 * \param channel channel, can be "*" if you want global ban, cannot contain spaces
 * \param mask mask, must have format nick!ident@host, cannot contain spaces
 * \param from name of who set the ban, does not have to be a valid user, cannot contain spaces
 * \param delay time in seconds how long the ban should last (e.g. 60)
 * \param reason reason
 * \param bot optional and should only be used by parse-botnet.cpp
 * \return 2 if ban has been added (only for main bots)
 *         1 if ban request has been sent (for slaves or leafs)
 *         0 if an error occurred (main bot is down, mask conflict, channel not found) 
*/

int protmodelist::addBan(const char *channel, const char *mask, const char *from, int delay, const char *reason, const char *bot)
{
    int chanNum;
    protmodelist *ban;
    protmodelist::entry *s;
    chan *ch;

    if(config.bottype == BOT_MAIN)
    {
        if(!strcmp(channel, "*"))
            ban=userlist.protlist[BAN];
        else
        {
            if((chanNum=userlist.findChannel(channel))==-1)
                return 0;

            ban=userlist.chanlist[chanNum].protlist[BAN];
        }

        if((s=ban->conflicts(mask)))
            return 0;

        s=ban->add(mask, from, NOW, NOW+delay, reason, false);
	userlist.updated();
        net.send(HAS_B, "%s %s %s %s %s %d %s", S_ADDBAN , channel, s->mask, s->by, " 0 ", s->expires, s->reason);

        if(!strcmp(channel, "*"))
        {
            if(bot)
                net.send(HAS_N, "Added ban %s requested by bot %s", mask, bot);
            else
                net.send(HAS_N, "Added ban %s", mask);

            foreachSyncedChannel(ch)
                ch->applyBan(s);
        }

        else
        {
            if(bot)
                net.send(HAS_N, "Added ban %s on %s requested by bot %s", mask, channel, bot);
            else
                net.send(HAS_N, "Added ban %s on %s", mask, channel);

            if((ch=ME.findChannel(channel)))
                ch->applyBan(s);
        }

        return 2;
    }

    else
	return net.sendHub("%s %s %s %s %d %s", S_REQBAN, channel, mask, from, delay, reason);

    return 0;
}

