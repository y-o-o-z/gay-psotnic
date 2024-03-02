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
#include <algorithm>

/*
** Mode queue
*/

modeq::modeq(chan *channel)
{
	ch = channel;
	data.removePtrs();
	sent.removePtrs();
}

modeq::modeq_ent *modeq::add(time_t exp, const char *m, const char *a)
{
	DEBUG(printf(">>> modeq.add(%d, %s, %s)\n", int(exp-NOW), m, a));
	modeq_ent *x = new modeq_ent(exp, m, a);
	data.add(x);
	return x;
}

modeq::modeq_ent *modeq::addBackup(time_t exp, const char *m, const char *a)
{
        DEBUG(printf(">>> modeq.addBackup(%d, %s, %s)\n", int(exp-NOW), m, a));
        modeq_ent *x = new modeq_ent(exp, m, a);
	x->backupmode = true;
        data.add(x);
        return x;
}

void modeq::setChannel(chan *channel)
{
	ch = channel;
}

int modeq::validate(modeq_ent *e, int &a, int &n)
{
	chanuser *u;
	masklist_ent *m;
	char type=chan::getTypeOfChanMode(e->mode[1]);

	if(e->reject == true)
	{
		DEBUG(printf(">>> e->reject is true for: %s %s / %s\n", e->mode, e->arg, (const char *) ch->name));
		return 0;
	}

	if(e->mode[0] == '+')
	{
		switch(e->mode[1])
		{
			case 'o':
				if(a == 3) return -1;
				u = ch->getUser(e->arg);

				if(u && (u->flags & (HAS_O | IS_OP | OP_SENT)) == HAS_O)
				{
					u->flags |= OP_SENT;
					++a;
					return 1;
				}
				else return 0;

			case 'v':
				if(a == 3) return -1;
				u = ch->getUser(e->arg);
				if(u && (u->flags & (IS_VOICE | VOICE_SENT)) == 0)
				{
					u->flags |= VOICE_SENT;
					++a;
					return 1;
				}
				else return 0;
			case 'k':
				if(a == 3) return -1;
				if(ch->key() && !strcmp(ch->key(), e->arg))
				{
					return 0;
				}
				/*if(!ch->key() || !*ch->key())
				{
					++a;
					return 1;
				}
				else return 0;
				*/
				++a;
				return 1; // FIXME: doesnt work on qnet
			case 'l':
				if(a == 3) return -1;
				if(e->arg && ch->limit() != atoi(e->arg))
				{
					++a;
					return 1;
				}
				else return 0;
			case 'b':
				if(a == 3) return -1;
				if(!ch->list[BAN].find(e->arg) && !ch->sentList[BAN].find(e->arg))
				{
					++a;
					ch->sentList[BAN].add(e->arg, "*", 0);
					return 1;
				}
				else return 0;
			case 'I':
				if(a == 3) return -1;
				if(!ch->list[INVITE].find(e->arg) && !ch->sentList[INVITE].find(e->arg))
				{
					++a;
					ch->sentList[INVITE].add(e->arg, "*", 0);
					return 1;
				}
				else return 0;
			case 'e':
				if(a == 3) return -1;
				if(!ch->list[EXEMPT].find(e->arg) && !ch->sentList[EXEMPT].find(e->arg))
				{
					++a;
					ch->sentList[EXEMPT].add(e->arg, "*", 0);
					return 1;
				}
				else return 0;
			case 'R':
				if(a == 3) return -1;
				if(!ch->list[REOP].find(e->arg) && !ch->sentList[REOP].find(e->arg))
				{
					++a;
					ch->sentList[REOP].add(e->arg, "*", 0);
					return 1;
				}
				else return 0;

			//no arg modes:
			default:
				if(n == 5) return -1;
				if(!ch->hasFlag(e->mode[1]) || type == 'B')
				{
					if(ch->sentModes.find(e->mode, e->arg) != 0)
						return 0;

					++n;
					ch->sentModes.add(e->mode, e->arg);
					return 1;
				}
				else return 0;
		}
	}
	else if(e->mode[0] == '-')
	{
		switch(e->mode[1])
		{
			case 'o':
				return 0;
			case 'v':
				if(a == 3) return -1;
				u = ch->getUser(e->arg);
				if(u && u->flags & IS_VOICE)
				{
					++a;
					return 1;
				}
				else return 0;
			case 'b':
				if(a == 3) return -1;
				if((m = ch->list[BAN].find(e->arg)) && !m->sent)
				{
					m->sent = true;
					++a;
					return 1;
				}
				else return 0;
			case 'I':
				if(a == 3) return -1;
				if((m = ch->list[INVITE].find(e->arg)) && !m->sent)
				{
					m->sent = true;
					++a;
					return 1;
				}
				else return 0;
			case 'e':
				if(a == 3) return -1;
				if((m = ch->list[EXEMPT].find(e->arg)) && !m->sent)
				{
					m->sent = 1;
					++a;
					return 1;
				}
				else return 0;
			case 'R':
				if(a == 3) return -1;
				if((m = ch->list[REOP].find(e->arg)) && !m->sent)
				{
					m->sent = 1;
					++a;
					return 1;
				}
				else return 0;
			case 'k':
				if(a == 3) return -1;
				if(ch->key() && *ch->key() && !strcmp(e->arg, ch->key()))
				{
					++a;
					return 1;
				}
				else return 0;
			case 'l':
				if(n == 5) return -1;
				if(ch->limit() && !find_sent(e->mode, e->arg))
				{
					++n;
					add_sent(e->mode);
					return 1;
				}
				else return 0;

			default:
				if(n == 5) return -1;
				if(ch->hasFlag(e->mode[1]))
				{
                                        if(find_sent(e->mode, e->arg) != 0)
                                                return 0;

                                        ++n;
                                        add_sent(e->mode, e->arg);
                                        return 1;
				}
				else return 0;
		}
	}
	return 0;
}

int modeq::flush(int prio)
{
	grouplist<modeq_ent>::iterator gx, g = data.begin();
	modeq_ent *modeList[8];
	ptrlist<modeq_ent>::iterator e, ex;
	int i, a=0, n=0, num=0;

	if(!ch->synced() || !(ch->me->flags & IS_OP))
		return 0;

	if(prio == PRIO_HIGH)
	{
		if(penalty >= 10)
			return 0;
	}
	else if(prio == PRIO_LOW)
	{
		if(penalty)
			return 0;
	}
	else
		return 0;

	for(i=0; i<8; ++i)
		modeList[i] = NULL;

	//get all expired modes
	while(g)
	{
		e = g->begin();
		gx = g;
		gx++;

		if(e->expire <= NOW)
		{
			while(e)
			{
				ex = e;
				ex++;

				if(num >= 8)
					break;

				switch(validate(e, a, n))
				{
					case 0:
						DEBUG(printf(">>> Rejecting: %s %s / %s\n", e->mode, e->arg, (const char *) ch->name));
						data.remove(e);
						break;
					case -1:
						DEBUG(printf(">>> no space(arg: %d, nonarg: %d) for: %s %s / %s\n", a, n, e->mode, e->arg, (const char *) ch->name));
						break;
					case 1:
						DEBUG(printf(">>> Mode OK: %s :%s / %s\n", e->mode, e->arg, (const char *) ch->name));
						modeList[num++] = e;
						break;
				}

				if(a == 3 && n == 5) break;
				e = ex;
			}
		}
		//else break;

		g = gx;
	}

	if(num)
	{
		Pchar modes, args;
		char sign = modeList[0]->mode[0];
		modes.push(sign);

		for(i=0; i<num; ++i)
		{
			if(i > 0)
			{
				if(sign != modeList[i]->mode[0])
				{
					sign = modeList[i]->mode[0];
					modes.push(sign);
				}
			}

			modes.push(modeList[i]->mode[1]);
			if(modeList[i]->arg && *modeList[i]->arg)
			{
				args.push(modeList[i]->arg);
				args.push(' ');
			}
		}

		net.irc.send("MODE %s %s %s", (const char *) ch->name,  modes.data, args.data);
		penalty += 3*num + 1;
		for(i=0; i<num; ++i)
		{
			data.remove(modeList[i]);
		}
	}

	return num;
}

/*
** Mode queue entry
*/

modeq::modeq_ent::modeq_ent(time_t exp, const char *m, const char *a)
{
	expire = exp;
	mode[0] = m[0];
	mode[1] = m[1];
	mode[2] = '\0';
	if(a && *a) mem_strcpy(arg, a);
	else arg = NULL;

	reject = backupmode = false;
}

modeq::modeq_ent::~modeq_ent()
{
	if(arg) free(arg);
}

int modeq::modeq_ent::operator==(modeq_ent &e)
{
	return expire == e.expire && !strcmp(mode, e.mode) &&
			arg ? (e.arg && !strcmp(arg, e.arg)) : 1;
}

int modeq::modeq_ent::operator&(modeq_ent &e)
{
	return expire == e.expire;
}

void modeq::removeBackupModesFor(const char *mode, const char *arg)
{
	ptrlist<ptrlist<modeq_ent> >::iterator g = data.begin();
	ptrlist<modeq_ent>::iterator m;

	while(g)
	{
		m = g->begin();

		while(m)
		{
			if(m->backupmode && !strcmp(m->mode, mode) && (arg ?
				(m->arg && !strcmp(arg, m->arg)) : 1))
			{
				DEBUG(printf(">>> Marking %s %s/%s as reject\n", mode, arg, (const char *) ch->name));
				m->reject = true;
			}
			m++;
		}
		g++;
	}
}

modeq::modeq_ent *modeq::find(const char *mode, const char *arg)
{
	grouplist<modeq_ent>::iterator g = data.begin();
	ptrlist<modeq_ent>::iterator e;

	while(g)
	{
		e = g->begin();

		while(e)
		{
			if(!strncmp(e->mode, mode, 2) && !strcmp(e->arg, arg))
				return e;

			e++;
		}

		g++;
	}

	return 0;
}

#ifdef HAVE_DEBUG
void modeq::display()
{
        grouplist<modeq_ent>::iterator g = data.begin();
        ptrlist<modeq_ent>::iterator e;

        while(g)
        {
                e = g->begin();

                while(e)
                {
			printf("%s %s\n", e->mode, e->arg);
                        e++;
                }

                g++;
        }
}
#endif

void modeq::add_sent(const char *m, const char *a)
{
        modeq_ent *x = new modeq_ent(0, m, a);
        sent.add(x);
}

modeq::modeq_ent *modeq::find_sent(const char *mode, const char *arg)
{
        grouplist<modeq_ent>::iterator g = sent.begin();
        ptrlist<modeq_ent>::iterator e;

        if(!mode || *mode == '\0')
		return 0;

        while(g)
        {
                e = g->begin();

                while(e)
                {
                        if(!strncmp(e->mode, mode, 2))
                        {
                            if(e->arg && arg)
                            {
                                if(!strcmp(e->arg, arg))
                                    return e;
                            }

                            else
                                return e;
                        }

                        e++;
                }

                g++;
        }

        return 0;
}
