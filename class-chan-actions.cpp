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

/* STRANGE ACTIONS */
void chan::updateLimit()
{
	char buf[MAX_LEN];
	int tolerance;

	if(!chset->LIMIT || nextlimit == -1 || !synced())
		return;

	if(chset->LIMIT_TOLERANCE > 0)
		tolerance = chset->LIMIT_TOLERANCE;
	else
		tolerance = (chset->LIMIT_TOLERANCE * chset->LIMIT_OFFSET)/(-100);

	if(nextlimit <= NOW && me->flags & IS_OP && myTurn(chset->LIMIT_BOTS))
	{
		if((limit() <= users.entries() + chset->LIMIT_OFFSET - tolerance) || (limit() > users.entries() + chset->LIMIT_OFFSET + tolerance))
		{
			sprintf(buf, "%d", users.entries() + chset->LIMIT_OFFSET);
			net.irc.send("MODE %s +l %s", (const char *) name, buf);
		}

		nextlimit = NOW + chset->LIMIT_TIME_UP + rand() % 10;
	}
}

void chan::enforceLimits()
{
	if(chset->ENFORCE_LIMITS && limit() && synced())
	{
		int n = users.entries() - limit();
		ptrlist<chanuser>::iterator u = users.begin();

		if(n > 0)
		{
			while(n && u)
			{
				if(!(u->flags & HAS_F))
				{
					u->setReason(set.LIMIT_KICKREASON);
					toKick.sortAdd(u);
					--n;
				}
				u++;
			}
		}
	}
}

/* MULTIUSER ACTIONS */
int chan::massKick(const int who, int lock)
{
	ptrlist<chanuser>::iterator u = users.begin();
	int i = 0;

	if(lock && myTurn(chset->PUNISH_BOTS, who))
	{
		modeQ[PRIO_HIGH].add(NOW, "+i");
		modeQ[PRIO_HIGH].flush(PRIO_HIGH);
	}

	while(u)
	{
		if(!(u->flags & HAS_F))
		{
			if((who & MK_OPS && u->flags & IS_OP) || (who & MK_NONOPS && !(u->flags & IS_OP)))
			{
				toKick.sortAdd(u);
				++i;
			}
		}
		u++;
	}
	return i;
}

int chan::op(chanuser **MultHandle, int num)
{
	char *a = NULL;
	int i, j=0;

	if(!num) return 0;

	if(penalty < 10 && me->flags & IS_OP)
	{
		for(i=j=0; i<num; ++i)
		{
			if(!(MultHandle[i]->flags & OP_SENT))
			{
				a = push(a, " ", MultHandle[i]->nick, NULL);
				MultHandle[i]->flags += OP_SENT;
				++j;
			}
		}
		if(j)
		{
			net.irc.send("MODE %s %s %s", (const char *) name, j == 3 ? " +ooo" : j == 2 ? " +oo" : " +o", a);
			free(a);
		}
	}
	return j;
}

int chan::kick4(chanuser **MultHandle, int num)
{
	if(num < 1) return 0;
	if(num > 4) num = 4;

	char *a = NULL;
	char *reason = NULL;
	int i, j=0;

	if(penalty < 10 && me->flags & IS_OP)
	{
		for(i=j=0; i<num && j<ME.server.isupport.max_kick_targets; ++i)
		{
			if(*MultHandle[i]->nick && !(MultHandle[i]->flags & KICK_SENT))
			{
				a = push(a, a ? (char *) "," : (char *) " ", MultHandle[i]->nick, NULL);
				MultHandle[i]->flags |= KICK_SENT;
				++j;

				if(MultHandle[i]->reason)
					reason = MultHandle[i]->reason;
			}
		}
		if(j)
		{
			net.irc.send("KICK %s %s :%s", (const char *) name, a, reason ? reason : (const char *) set.KICKREASON);
			free(a);
			sentKicks += j;
		}
	}
	return j;
}

int chan::kick6(chanuser **MultHandle, int num)
{
	if(num < 1) return 0;
	if(num < 5 || penalty > 5) return kick4(MultHandle, num);
	else
	{
		if(num > 6) num = 6;

		if(penalty < 2)
			return kick4(MultHandle, 2) + kick4(MultHandle+2, num-2);
		else
			return kick4(MultHandle, 1) + kick4(MultHandle+1, num-1);
	}
}

/* SINGLEUSER */
int chan::op(chanuser *p)
{
	if(!p || !p->nick || !*p->nick) return 0;

	if(!(p->flags & OP_SENT) && me->flags & IS_OP)
	{
		if(penalty < 10)
		{
			net.irc.send("MODE %s +o %s", (const char *) name, p->nick);
			p->flags |= OP_SENT;
			return 1;
		}
	}
	return 0;
}

/*
int chan::deOp(chanuser *p)
{
	if(!p || !p->nick || !*p->nick) return 0;

	if(!(p->flags & DEOP_SENT))
	{
		if(penalty < 10)
		{
			net.irc.send("MODE %s -o %s", name, p->nick);
			p->flags |= DEOP_SENT;
			penalty += 4;
			return 1;
		}
	}
	return 0;
}
*/


int chan::kick(chanuser *p, const char *reason)
{
	if(!p || !p->nick || !*p->nick) return 0;

	if(!(p->flags & KICK_SENT) && me->flags & IS_OP)
	{
		if(penalty < 10)
		{
			if(reason) net.irc.send("KICK %s %s :%s", (const char *) name, p->nick, reason);
			else net.irc.send("KICK %s %s :%s", (const char *) name, p->nick, (const char *) set.KICKREASON);
			p->flags |= KICK_SENT;
			++sentKicks;
			return 1;
		}
	}
	return 0;
}

int chan::invite(const char *nick)
{
	if(!nick || !*nick) return 0;

	if(!getUser(nick))
	{
		char *a, *nick2;
		mem_strcpy(nick2, nick);

		a = strchr(nick2, '!');
		if(a) *a = '\0';

		::invite.wisePush("INVITE %s %s", nick2, (const char *) name);

		if(penalty < 5)
			::invite.flush(&net.irc);

		if(nick2)
			free(nick2);

		return 1;
	}
	return 0;
}

int chan::applyBan(const protmodelist::entry *s, int force)
{
	int i = 0;

	if(s->sticky)
	{
		modeQ[PRIO_HIGH].add(NOW, "+b", s->mask);
		return 1;
	}

	if(force || myTurn(chset->GUARDIAN_BOTS, hash32(s->mask)))
	{
		ptrlist<chanuser>::iterator u = users.begin();

		while(u)
		{
			if(!(u->flags & HAS_F) && u->matchesBan(s->mask))
			{
				modeQ[PRIO_HIGH].add(NOW, "+b", s->mask);
				++i;
			}
			u++;
		}
	}
	return i;
}

void chan::enforceBan(const char *ban, chanuser *u, const char *reason, const bool autoKick)
{
	ptrlist<chanuser>::iterator p = users.begin();

	int ulevel = userLevel(u);

	while(p)
	{
		if(ulevel > userLevel(&p))
		{
			if(p->matchesBan(ban))
			{
				if(reason)
					p->setReason(reason);
				if(autoKick)
					toKick.sortAdd(p);
			}
		}
		p++;
	}
}

void chan::punishClones(const char *mask, bool isMyTurn)
{
	if(chset->LOCKDOWN)
	{
		modeQ[PRIO_HIGH].add(NOW, "+i");
		modeQ[PRIO_LOW].add(NOW+chset->LOCKDOWN_TIME, "-i");

		modeQ[PRIO_LOW].add(NOW, "+b", mask);

		enforceBan(mask, me, !set.CLONECHECK_KICKREASON.isDefault() ? set.CLONECHECK_KICKREASON : set.KICKREASON);

		if(isMyTurn)
			modeQ[PRIO_HIGH].flush(PRIO_HIGH);
	}
	else
	{
		modeQ[PRIO_HIGH].add(NOW, "+b", mask);

		enforceBan(mask, me, !set.CLONECHECK_KICKREASON.isDefault() ? set.CLONECHECK_KICKREASON : set.KICKREASON, false);

		if(isMyTurn)
			modeQ[PRIO_HIGH].flush(PRIO_HIGH);
	}
}

void chan::knockout(chanuser *u, const char *reason, int delay)
{
	static char buf[MAX_LEN];

	snprintf(buf, MAX_LEN, "*!%s@%s", u->ident, u->host);
	modeQ[PRIO_HIGH].add(NOW, "+b", buf);
	modeQ[PRIO_LOW].add(NOW+delay, "-b", buf)->backupmode = true;
	modeQ[PRIO_HIGH].flush(PRIO_HIGH);

	kick(u, reason);
	u->setReason(reason);
}
