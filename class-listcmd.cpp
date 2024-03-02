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

//  return values:
//  0 - local
//  1 - remote
// -1 - error
int listcmd(char what, const char *from, const char *arg1, const char *arg2, inetconn *c)
{
	char *tmp = NULL;
	int i;

	switch(what)
	{
		//local and remote :)
		case 'p':
		{
			//owner orders hub to ping bots
			if(!c)
			{
				net.propagate(NULL, "%s p %s %s %s", S_LIST, from, itoa(NOW), itoa(nanotime()));
			}
			else
			{
				if(config.bottype == BOT_MAIN)
				{
					time_t NANO = nanotime();
					time_t T = atoi(arg1);
					time_t N = atoi(arg2);
					char buf[MAX_LEN];

					if(NANO >= N)
						snprintf(buf, MAX_LEN, "%.3f", float(NOW-T) + float((NANO - N)/1e6));
					else
						snprintf(buf, MAX_LEN, "%.3f", float(NOW - T -1) + float(((1e6 - N + NANO)/1e6)));

					net.sendUser(from, "[p] reply from %s in %s secs", c->handle->name, buf);
				}
				else
				{
					if(c->isMain())
						c->send("%s %s %s %s %s", S_LIST, " p ", from, arg1, arg2);
					else
					{
						for(i=0; i<net.max_conns; ++i)
							if(net.conn[i].isMain())
								net.conn[i].send("%s %s %s %s %s", S_LIST, " p ", from, arg1, arg2);
					}
				}
			}
			return 0;
		}

		//remote
		case 's':
		{
			pstring <128> str;

			str = "[s] current: ";
			str += net.irc.name ? net.irc.name : "none";
			str += ", in config: ";

			for(i=0; i<MAX_SERVERS; ++i)
			{
				if(!config.server[i].isDefault())
				{
					str += config.server[i].getHost().connectionString;
					str += ":";
					str += itoa(config.server[i].getPort());
					str += " ";
				}
			}
			net.sendUser(from, "%s", (const char *) str);
			return 1;
		}
		//remote
		case 'v':
		{
			net.sendUser(from, "[v] %s version: %s ; SVN rev: %s", S_BOTNAME, S_VERSION, SVN_REVISION);
			return 1;
		}

		//remote
		case 'U':
		{
			char buf[MAX_LEN];
			struct rusage r;

			int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);

			if(getrusage(RUSAGE_SELF, &r))
				net.sendUser(from, "[U] up for: %s n/a, sys: n/a, mem: n/a", buf);
			else
			{
				int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);
				net.sendUser(from, "[U] up for: %s, usr: %s.%ss, sys: %s.%ss", buf,
					itoa(r.ru_utime.tv_sec), itoa(r.ru_utime.tv_usec / 10000),
					itoa(r.ru_stime.tv_sec), itoa(r.ru_stime.tv_usec / 10000));
					//", mem: ", itoa(r.ru_idrss + r.ru_isrss + r.ru_ixrss), "k", NULL);
			}
			return 1;
		}

		//remote
		case 'd':
		{
			if(!net.irc.isReg())
				net.sendUser(from, "[d] not on irc");

			return 1;
		}
		//remote
		case 'c':
		{

			for(i=0; i<MAX_CHANNELS; ++i)
			{
				if(userlist.chanlist[i].name && userlist.isRjoined(i))
				{
					chan *ch = ME.findChannel(userlist.chanlist[i].name);
					tmp = push(tmp, ch ? (ch->synced() ?
						((ch->me->flags & IS_OP) ? (char *) "@" : (char *) "") : (char *) "?")
						: (char *) "-", (const char *) userlist.chanlist[i].name, " ", NULL);
				}
			}
			if(tmp)
			{
				net.sendUser(from, "[c] %s", tmp);
				free(tmp);
			}
			else net.sendUser(from, "[c] no channels");
			return 1;
		}
		case 'u':
		{
			struct utsname u;
			if(!uname(&u))
			{
				net.sendUser(from, "[u] %s %s %s %s %s", u.sysname, u.nodename, u.release,
						u.version, u.machine);
			}
			else
			{
				net.sendUser(from, "[u] %s", strerror(errno));
			}
			return 1;
		}
		case 'i':
		{
			if(net.irc.isReg())
			{
				if(userlist.wildFindHost(userlist.me(), ME.mask) != -1)
					net.sendUser(from, "[i] connected to %s as %s", net.irc.name, (const char *) ME.mask);
				else
					net.sendUser(from, "[i] connected to %s as %s (not in userlist)", net.irc.name, (const char *) ME.mask);
			}
			else
			{
				net.sendUser(from, "[i] not on irc");
			}
			return 1;
		}
	}
	return -1;
}

