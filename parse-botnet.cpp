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

int requestBan(const char *channel, const char *mask, const char *from, int delay, const char *reason, const char *bot);

static char arg[10][MAX_LEN];
static chan *ch;
static chanuser *p;

int parse_botnet(inetconn *c, char *data)
{
	if(!c)
		return 0;

	if(!data || *data == '\0')
		return 0;

	str2words(arg[0], data, 10, MAX_LEN);

	/* CHNICK [nick [irc server] ] */
	if(!strcmp(arg[0], S_CHNICK))
	{
		if(*arg[2] || !*arg[1])
		{
			free(c->origin);
			free(c->name);
			mem_strcpy(c->origin, arg[2]);
			mem_strcpy(c->name, arg[1]);
		}

		//nick change
		else if(*arg[1])
		{
			free(c->name);
			mem_strcpy(c->name, arg[1]);
		}
		return 1;
	}

	/* BJOIN [ircnick [irc server] ] */
	/*
	if(!strcmp(arg[0], S_BJOIN))
	{
		if(h && userlist.isBot(h) && strcmp(ME.nick, arg[1]) && !net.findConn(h))
		{
			bot = net.addConn(c->fd);
			if(bot)
			{
				mem_strcpy(bot->name, arg[2]);
				mem_strcpy(bot->origin, arg[3]);
                bot->status = STATUS_CONNECTED + STATUS_REGISTERED + STATUS_BOT + STATUS_REDIR;
				bot->handle = h;
    			net.propagate(c, "%s", data);

				if(config.bottype == BOT_LEAF && config.currentHub != &config.hub &&
					!strcmp(arg[1], config.hub.handle))
				{
					config.hub.failures = 0;
					net.hub.close("Jumping to hub");
				}
			}
		}
		return 1;
	}
	*/
	/* BQUIT [reason] */
	if(!strcmp(arg[0], S_BQUIT))
	{
		// FIXME: this lets leaf close the connection to a slave when then main goes down -- patrick
		c->close(srewind(data, 1));
		return 1;
	}

	/* FORWARD <from> <to|*> <data> */
	if(!strcmp(arg[0], S_FORWARD) && strlen(arg[3]))
	{
		inetconn *from = net.findConn(userlist.findHandle(arg[1]));

		if(from || !strcmp(arg[3], S_FORWARD))
		{
			if(from && from->fd != c->fd)
			{
				net.send(HAS_N, "\002Hacked packet from %s@%s: %s\002", c->handle->name, from->name, data); // TODO: colors
				c->close("Hacked packed -- internal error or smb is doing sth nasty"); // TODO: colors
				return 1;
			}
		}
		//bot joined ?
		else
		{
			/* FORWARD <from> "*" BJOIN [nick [ircserver]] */
			if(!strcmp(arg[3], S_BJOIN) && !strcmp(arg[2], "*"))
			{
				HANDLE *h = userlist.findHandle(arg[1]);
				if(!h)
				{
					net.send(HAS_N, "Invalid bot join (dsynced ul?) from %s: %s", c->name, arg[1]); // TODO: colors
					net.send(HAS_N, "This should not happnen, please do .restart %s", c->name); // TODO: colors
					return 1;
				}

				if(h->flags[GLOBAL] & HAS_B)
				{
					//maybe thats my slave ?
					if(config.bottype == BOT_LEAF && config.currentHub != &config.hub &&
							!strcmp(arg[1], config.hub.getHandle()))
					{
						config.hub.failures = 0;
						ME.nextConnToHub = NOW + set.HUB_CONN_DELAY + rand() % set.HUB_CONN_DELAY;
						net.hub.close("Jumping to main slave");
						DEBUG(printf("Jumping to main slave\n"));
						//no point of doing anything else
						//cos we disconnect from other bots in the botnet
						return 1;
					}

					from = net.addConn(c->fd);

					mem_strcpy(from->name, arg[4]);
					mem_strcpy(from->origin, arg[5]);
					from->status = STATUS_CONNECTED + STATUS_REGISTERED + STATUS_BOT + STATUS_REDIR;
#ifdef HAVE_SSL
					if(net.hub.ssl)
						from->status |= STATUS_SSL;
#endif
					from->handle = h;

					if(from->isMain())
						ME.checkMyHost("*");
				}
			}
			else
			{
				net.send(HAS_N, "\002Invalid packet from %s: %s\002", c->name, data); // TODO: colors
				c->close("Invalid packet -- internal error or smb is doing sth nasty"); // TODO: colors
				return 1;
			}
		}

		if(!strcmp(arg[2], config.handle))
		{
                        if(from)
			{
				if(from->isMain())
					parse_hub(srewind(data, 3));
				else
					parse_botnet(from, srewind(data, 3));
			}
		}
		else if(!strcmp(arg[2], "*"))
		{
			if(from)
			{
				net.propagate(from, "%s", srewind(data, 3));

				if(from->isMain())
					parse_hub(srewind(data, 3));
				else
					parse_botnet(from, srewind(data, 3));
			}
		}
		else
		{
			inetconn *to = net.findConn(userlist.findHandle(arg[2]));

			if(to)
			{
				inetconn *slave = net.findRedirConn(to);
				if(slave)
					slave->send("%s", data);
			}
		}
		return 1;
	}
	/* BOP <channel> */
	if(!strcmp(arg[0], S_BOP) && strlen(arg[1]))
	{
		ch = ME.findChannel(arg[1]);
		if(ch)
		{
			p = ch->getUser(c->name);
			if(p && (p->flags & HAS_B) && (set.GETOP_OP_CHECK ? !(p->flags & IS_OP) : 1))
			{
				int i;

				if((i = userlist.findChannel(arg[1])) != -1 && userlist.isRjoined(i, c->handle))
				{
					if(!ch->modeQ[PRIO_HIGH].find("+o", p->nick))
						ch->modeQ[PRIO_HIGH].add(NOW, "+o", p->nick);
				}
				else
				{
					net.send(HAS_N, "\002\0030,4Received illegal OP request from %s on %s - bot is not rjoined!\003\002", c->handle->name, arg[1]);
				}
				// ch->op(p); - changed 05.08.2010
			}
		}
		return 1;
	}
	/* S_INVITE <seed> <channel> */
	if(!strcmp(arg[0], S_INVITE) && strlen(arg[2]))
	{
		ch = ME.findChannel(arg[2]);
		if(ch)
		{
			if(ch->me->flags & IS_OP && ch->myTurn(ch->chset->INVITE_BOTS, atoi(arg[1])))
			{
				if(!ch->getUser(c->name))
					ch->invite(c->name);
			}
		}
		return 1;
	}

	/* S_UNBANME <seed> <nick!ident@host> <channel> [IP] [UID]*/
	if(!strcmp(arg[0], S_UNBANME) && strlen(arg[3]))
	{
		if(penalty >= 10) return 1;
		ch = ME.findChannel(arg[3]);
		if(ch)
		{
			if(ch->me->flags & IS_OP && ch->myTurn(ch->chset->GUARDIAN_BOTS, atoi(arg[1])))
			{
				if(!ch->getUser(c->name))
				{
					if(ch->chset->INVITE_ON_UNBAN_REQUEST)
						ch->invite(c->name);
					else
					{
						masklist_ent *m = ch->list[BAN].matchBan(arg[2], arg[4], arg[5]);
						if(m)
						{
							if(protmodelist::isSticky(m->mask, BAN, ch))
								ch->invite(c->name);
							else
							{
								ch->modeQ[PRIO_HIGH].add(NOW, "-b", m->mask);
								ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);

								c->send("%s %s", S_COMEON, arg[3]);
							}
						}
					}
				}
			}
		}
		return 1;
	}

	/* S_BIDLIMIT <seed> <channel> */
	if(!strcmp(arg[0], S_BIDLIMIT) && strlen(arg[2]))
	{
		net.propagate(c, "%s", data);
		if(penalty >= 10) return 1;

		ch = ME.findChannel(arg[2]);
		if(ch && ch->me->flags & IS_OP && ch->myTurn(ch->chset->LIMIT_BOTS, atoi(arg[1])))
		{
			if(ch->nextlimit != -1)
			{
				if(ch->limit() <= ch->users.entries())
				{
					ch->modeQ[PRIO_HIGH].add(NOW, "+l", itoa(ch->users.entries() + ch->chset->LIMIT_OFFSET));
					if(ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH))
						c->send("%s %s", S_COMEON, arg[2]);
				}
			}
			else ch->invite(c->name);
		}
		return 1;
	}

	/* S_KEY <seed> <channel> */
	if(!strcmp(arg[0], S_KEY) && strlen(arg[2]))
	{
		ch = ME.findChannel(arg[2]);
		if(ch && ch->key() && *ch->key() && ch->myTurn(ch->chset->INVITE_BOTS, atoi(arg[1])))
			c->send("%s %s %s", S_KEYRPL, arg[2], (const char *) ch->key());

		return 1;
	}

	/* S_KEYRPL <channel> <key> */
	if(!strcmp(arg[0], S_KEYRPL) && strlen(arg[2]))
	{
		int i;
		if(!ME.findChannel(arg[1]) && (i = userlist.findChannel(arg[1])) != -1)
		{
			userlist.chanlist[i].pass = arg[2];
			if(userlist.chanlist[i].nextjoin + 2 >= NOW)
				ME.rejoin(arg[1], 2);
		}
		return 1;
	}

	/* S_COMEON <channel> */
	if(!strcmp(arg[0], S_COMEON) && strlen(arg[1]))
	{
		int i;
		if(!ME.findChannel(arg[1]) && (i = userlist.findChannel(arg[1])) != -1 && userlist.isRjoined(i))
		{
			if(userlist.chanlist[i].nextjoin + 2 >= NOW)
				ME.rejoin(arg[1], 2);
		}
		return 1;
	}
	/* S_LIST ? <owner> arg1 arg2 */
	if(!strcmp(arg[0], S_LIST) && strlen(arg[2]))
	{
		listcmd(arg[1][0], arg[2], arg[3], arg[4], c);
			//net.propagate(c, "%s", data);
		return 1;
	}

	/* S_BOTCMD <cmd> */
	if(!strcmp(arg[0], S_BOTCMD) && strlen(arg[1]))
	{
		botnetcmd(c->name, srewind(data, 1));
		return 1;
	}

	if(config.bottype == BOT_MAIN)
	{
		if(!strcmp(arg[0], S_PROXYHOST) && strlen(arg[1]))
		{
			HANDLE *h = userlist.findHandle(arg[1]);
			if(h && userlist.isBot(h) && h->flags[GLOBAL] & HAS_P)
			{
				userlist.addHost(h, arg[2], NULL, 0, MAX_HOSTS-1);
				userlist.save(config.userlist_file);
				net.send(HAS_B, "%s", data);
			}
			return 1;
		}

		/* S_OREDIR <from> <to|*> <[?]> <data> */
		if(!strcmp(arg[0], S_OREDIR) && strlen(arg[4]))
		{
			char *a = srewind(data, 3);
			if(a)
				net.sendUser(arg[2], "(%s) %s", arg[1], a);
			return 1;
		}

		/* S_REQBAN <chan> <mask> <from> <delay> <reason> */
		if(!strcmp(arg[0], S_REQBAN) && strlen(arg[5]))
		{
			if(set.BOTS_CAN_ADD_BANS)
				protmodelist::addBan(arg[1], arg[2], arg[3], atol(arg[4]), srewind(data, 5), c->handle->name);
			else
				net.send(HAS_N, "[?] %s requested ban but bots-can-add-bans is OFF", c->handle->name);

			return 1;
		}
		/* S_ADDIDIOT <mask> <chan> <number> <reason> */
		if(!strcmp(arg[0], S_ADDIDIOT) && strlen(arg[4]))
		{
		    char *a = srewind(data, 4);
		    if(a)
			userlist.addIdiot(arg[1], arg[2], a, arg[3]);
		    return 1;
		}
		/* S_PASSWD <handle> <password> */
		if(!strcmp(arg[0], S_PASSWD) && strlen(arg[2]))
		{
			HANDLE *h;

			if(!set.ALLOW_SET_PASS_BY_MSG)
			{
				net.send(HAS_N, "\002%s failed to set password for %s: allow-set-pass-by-msg is OFF\002", c->handle->name, arg[1]);
				return 1;
			}

			h = userlist.findHandle(arg[1]);

			if(!h)
			{
				net.send(HAS_N, "\002%s failed to set password for %s: no such user\002", c->handle->name, arg[1]);
				return 1;
			}

			if(h->flags[GLOBAL] & (HAS_X | HAS_S | HAS_N | HAS_M))
			{
				net.send(HAS_N, "\002%s failed to set password for %s: too high flags\002", c->handle->name, arg[1]);
				return 1;
			}


			if(!strcmp((const char*) h->pass, "0000000000000000") || !strlen((const char*) h->pass))
			{
				userlist.setPassword(arg[1], arg[2]);
				net.send(HAS_B, "%s %s %s", S_PASSWD, arg[1], arg[2]);
			}

			else
				net.send(HAS_N, "\002%s failed to set password for %s: password is already set\002", c->handle->name, arg[1]);

			return 1;
		}
	}
	return 0;
}
