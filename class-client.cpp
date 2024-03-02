/***************************************************************************
 *   Copyright (C) 2003-2006 by Grzegorz Rusin                             *
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

void client::checkMyHost(const char *to,  bool justConnected)
{
	static char buf[MAX_LEN];

	if(!net.irc.isReg())
		return;

	if(ident[0] == '+' || ident[0] == '-' || ident[0] == '=')
	{
		int2units(buf, MAX_LEN, ircConnFailDelay, ut_time);
		net.sendOwner(to, "\002My connection is restricted, reconnecting in %s\002", buf);

		if(justConnected)
		{
			if(nextReconnect <= NOW)
				nextReconnect = NOW + ircConnFailDelay;

			ircConnFailDelay *= 3;
			if(ircConnFailDelay > 2*3600) ircConnFailDelay = 2*3600;
		}
		joinDelay = -1;
		waitForMyHost = true;
	}
	else
	{
		if(userlist.wildFindHost(userlist.me(), mask) == -1)
		{
			//joinDelay = -1;
			joinDelay = NOW + penalty;
			waitForMyHost = true;

			if(ident[0] == '^' || ident[0] == '~')
			{
				snprintf(buf, MAX_LEN, "%s!%s@%s", (const char *) nick, (const char *) ident+1, (const char *) host);
				if(userlist.wildFindHost(userlist.me(), buf) != -1)
				{
					int2units(buf, MAX_LEN, ME.ircConnFailDelay, ut_time);
					net.sendOwner(to, "\002I lost my ident, reconnecting in %s\002", buf);

					if(justConnected)
					{
						if(nextReconnect <= NOW)
							nextReconnect = NOW + ircConnFailDelay;

						ircConnFailDelay *= 3;
						if(ircConnFailDelay > 2*3600) ircConnFailDelay = 2*3600;
					}
					return;
				}
			}
			net.sendOwner(to, "\002My host %s is not added\002", (const char *) mask);
		}
		else
		{
			ircConnFailDelay = 15;
			joinDelay = NOW + penalty;
		}
	}
}

void client::sendStatus(const char *name)
{
	net.sendUser(name, "- about me:");
	net.sendUser(name, "Hi. I'm %s and I'm running psotnic %s", (const char *) config.handle, S_VERSION);

	char buf[MAX_LEN];
	int min, max, sum;
	double avg;

	struct rlimit rlim;

	int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);
	net.sendUser(name, "Up for: %s", buf);

	if(net.irc.isReg())
	{
		net.sendUser(name, "Connected to %s as %s", net.irc.origin, (const char *) ME.mask);

		if(!net.irc.lagcheck.sent.tv_usec)
			net.sendUser(name, "Lag: not checked");
		else if(net.irc.lagcheck.inProgress)
			net.sendUser(name, "Lag: %d (waiting for reply)", net.irc.lagcheck.getLag()/1000);
		else
			net.sendUser(name, "Lag: %f (last checked %ds ago)", net.irc.lagcheck.lag/1000.0, NOW - net.irc.lagcheck.sent.tv_sec);
	}

	net.sendUser(name, "I have %s bots and %s owners on-line", itoa(net.bots()), itoa(net.owners()));

	if(ME.channels)
	{
		net.sendUser(name, "- my channels: ");
		chan *ch = ME.first;
		while(ch)
		{
			ch->users.stats(min, avg, max, sum);
			snprintf(buf, MAX_LEN, "[hash: %d/%g/%d/%d]", min, avg, max, sum);
			net.sendUser(name, ch->name, " (%s, %s ops, %s total) %s", ch->getModes(), itoa(ch->chops()),
					itoa(ch->users.entries()), buf);
			ch = ch->next;
		}
	}

	net.sendUser(name, "I'm configured as follows:");
#ifdef HAVE_ANTIPTRACE
	net.sendUser(name, "Antiptrace:               Enabled");
#else
	net.sendUser(name, "Antiptrace:               Disabled");
#endif
#ifdef HAVE_IRC_BACKTRACE
	net.sendUser(name, "IRC Backtrace:            Enabled");
#else
	net.sendUser(name, "IRC Backtrace:            Disabled");
#endif
#ifdef HAVE_SSL
	net.sendUser(name, "SSL support:              Enabled"); //TODO: add version string
#else
	net.sendUser(name, "SSL support:              Disabled");
#endif
#ifdef HAVE_IPV6
	net.sendUser(name, "IPv6 support:             Enabled");
#else
	net.sendUser(name, "IPv6 support:             Disabled");
#endif
#ifdef HAVE_ADNS
	net.sendUser(name, "Asynchronus DNS Resolver: Enabled");
#else
	net.sendUser(name, "Asynchronus DNS Resolver: Disabled");
#endif
#ifdef HAVE_ADNS_PTHREAD
	net.sendUser(name, "Resolving Threads:        Enabled, using %d threads", (int) config.resolve_threads);
#else
	net.sendUser(name, "Resolving Threads:        Disabled");
#endif
#ifdef HAVE_ADNS_FIREDNS
	net.sendUser(name, "FireDNS Resolver:         Enabled");
#else
	net.sendUser(name, "FireDNS Resolver:         Disabled");
#endif
#ifdef HAVE_LITTLE_ENDIAN
	net.sendUser(name, "Endianness:               Little");
#else
	net.sendUser(name, "Endianness:               Big");
#endif
#ifdef HAVE_DEBUG
	net.sendUser(name, "Debug Mode:               Enabled");
#else
	net.sendUser(name, "Debug Mode:               Disabled");
#endif
	if(getrlimit(RLIMIT_CORE, &rlim) == 0)
	{
		if(rlim.rlim_cur == RLIM_INFINITY)
			net.sendUser(name, "Core Limit:               unlimited");
		else
			net.sendUser(name, "Core Limit:               %ld (max: %ld)", rlim.rlim_cur, rlim.rlim_max);
	}

	else
		net.sendUser(name, "Core Limit:               %s", strerror(errno));

#ifdef HAVE_MODULES
	net.sendUser(name, "Module Support:               Enabled");
#else
	net.sendUser(name, "Module Support:               Disabled");
#endif
}

entServer *client::getRandomServer()
{
	//count servers
	int i, num;

#ifdef HAVE_SSL
	for(num=i=0; i<MAX_SERVERS*2; i++)
#else
	for(num=i=0; i<MAX_SERVERS; i++)
#endif
		if(!config.server[i].isDefault())
			++num;


	if(!num)
		return 0;

	DEBUG(printf("[D] got irc servers: %d\n", num));
	//get random one
	num = (rand() % num + 1);
	DEBUG(printf("[D] connecting to: %d\n", num));

	for(i=0; ;++i)
	{
		if(!config.server[i].isDefault())
			--num;

		if(!num)
			return &config.server[i];
	}
}

void client::autoRecheck()
{
	if(nextRecheck && nextRecheck <= NOW)
	{
		recheckFlags();
		nextRecheck = 0;
	}
}

void client::newHostNotify()
{
	if(hostNotify)
	{
		//lame check ;-)
		if(!(userlist.me()->flags[GLOBAL] & HAS_P))
		{
			hostNotify = 0;
			return;
		}

		if(config.bottype == BOT_MAIN)
		{
			userlist.addHost(userlist.first->next, mask, NULL, 0, MAX_HOSTS-1);
			net.send(HAS_B, "%s %s %s", S_PROXYHOST, (const char *) config.handle, (const char *) mask);
			userlist.updated(false);
			hostNotify = 0;
		}

		else
		{
			net.sendHub("%s %s %s", S_PROXYHOST, (const char *) config.handle, (const char *) mask);
			hostNotify = 0;
		}
	}
}

void client::restart()
{
	char buf[MAX_LEN];

	net.~inet();
#ifdef HAVE_ADNS
	delete resolver;
#endif
	snprintf(buf, MAX_LEN, "pid.%s", (const char *) config.nick);
	unlink(buf);

#ifdef HAVE_DEBUG
	execl(thisfile, thisfile, "-d", (const char *) config.file, NULL);
#else
	execl(thisfile, thisfile, (const char *) config.file, NULL);
#endif

	exit(1);
}

int client::jump(int number)
{
    net.send(HAS_N, "Jumping to %s port %d", (const char *) config.server[number].getHost().connectionString, (int) config.server[number].getPort());
    net.irc.send("QUIT :changing servers");
    net.irc.close("changing servers");
    config.currentServer = NULL;
    connectToIRC(&config.server[number]);
    return 0;
}

void client::inviteRaw(const char *str)
{
	char arg[3][MAX_LEN];

	str2words(arg[0], str, 3, MAX_LEN, 0);

	if(!strcmp(arg[0], "INVITE"))
	{
		chan *ch = ME.findChannel(arg[2]);
		if(ch && !ch->getUser(arg[1]))
			ch->invite(arg[1]);
	}
}

void client::registerWithNewNick(char *nick)
{
	nickCreator(nick);
	net.irc.send("NICK %s", nick);
}

void client::rejoin(const char *name, int t)
{
	int i = userlist.findChannel(name);

	if(i != -1 && userlist.isRjoined(i))
	{
		userlist.chanlist[i].nextjoin = NOW + t;
		userlist.chanlist[i].status &= ~(JOIN_SENT | WHO_SENT);
		if(!t)
			rejoinCheck();
	}
}

void client::rejoinCheck()
{
	int i;
	if(!(net.irc.status & STATUS_REGISTERED))
		return;

	for(i=0; i<MAX_CHANNELS && penalty < 10; ++i)
	{
		if(userlist.chanlist[i].name && userlist.chanlist[i].nextjoin <= NOW &&
			!(userlist.chanlist[i].status & JOIN_SENT) &&
			(!(userlist.chanlist[i].status & PRIVATE_CHAN) || userlist.isRjoined(i)))
		{
			if(!ME.findNotSyncedChannel(userlist.chanlist[i].name))
			{
				net.irc.send("JOIN %s %s", (const char *) userlist.chanlist[i].name, (const char *) userlist.chanlist[i].pass);
				userlist.chanlist[i].status |= JOIN_SENT;
				userlist.chanlist[i].status &= ~WHO_SENT;
			}
		}
	}
}

void client::joinAllChannels()
{
	if(joinDelay ==  -1)
		return;

	int i, j;

	Pchar buf;

	buf.push("JOIN ");
	int n;

	for(j=i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].name && !*userlist.chanlist[i].pass && userlist.isRjoined(i) &&
			!(userlist.chanlist[i].status & JOIN_SENT) && !findNotSyncedChannel(userlist.chanlist[i].name))
		{
			userlist.chanlist[i].status |= JOIN_SENT;
			userlist.chanlist[i].status &= ~WHO_SENT;
			n = strlen(userlist.chanlist[i].name) + 1;
			if(userlist.chanlist[i].pass)
				n += strlen(userlist.chanlist[i].pass) + 1;

			if(buf.len + n > 500)
			{
				net.irc.send("%s", buf.data);
				buf.clean();
				buf.push("JOIN ");
				j = 0;
			}

			if(buf.len != 5)
				buf.push(",");
			buf.push((const char *) userlist.chanlist[i].name);
			/*
			if(userlist.chanlist[i].pass && *userlist.chanlist[i].pass)
			{
				buf.push(" ");
				buf.push(userlist.chanlist[i].pass);
			}
			*/
			++j;
		}
	}
	if(j)
		net.irc.send("%s", buf.data);
	buf.clean();

	// FIXME: WHO flood

	buf.push("WHO ");
	for(j=i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].name && !*userlist.chanlist[i].pass && userlist.isRjoined(i) &&
			!(userlist.chanlist[i].status & WHO_SENT) && !findNotSyncedChannel(userlist.chanlist[i].name))
		{
			userlist.chanlist[i].status |= WHO_SENT;

			if(buf.len != 4)
				buf.push(",");
			buf.push((const char *) userlist.chanlist[i].name);
			++j;

			if(j == ME.server.isupport.max_who_targets)
			{
				net.irc.send("%s", buf.data);
				buf.clean();
				buf.push("WHO ");
				j = 0;
			}
		}
	}

	if(j) net.irc.send("%s", buf.data);
}

void client::gotNickChange(const char *from, const char *to)
{
	char *a = strchr((char *)from, '!');
	char *fromnick;
	chan *p = first;

	if(a) mem_strncpy(fromnick, from, abs(a - from) + 1);
	else mem_strcpy(fromnick, from);

	DEBUG(printf("[*] NICK change %s <-> %s\n", fromnick, to));
	while(p)
	{
		if(p->synced()) p->gotNickChange(from, to);
		p = p->next;
	}
	if(!strcmp(ME.nick, fromnick))
	{
		ME.nick = to;
		ME.mask = ME.nick;
		ME.mask += "!";
		ME.mask += ME.ident;
		ME.mask += "@";
		ME.mask += ME.host;

		net.propagate(NULL, "%s %s", S_CHNICK, (const char *) ME.nick);
		net.send(HAS_N, "I am known as %s", to);
		hostNotify = 1;
	}

	HOOK(nick, nick(fromnick, to));
	stopParsing=false;
	free(fromnick);
}

#ifdef HAVE_DEBUG
void client::display()
{
	chan *p = first;

	printf("### Channels:\n");
	while(p)
	{
		printf("### '%s'\n", (const char *) p->name);
		p = p->next;
	}
}
#endif

void client::recheckFlags()
{
	chan *ch = ME.first;

	while(ch)
	{
		if(ch->synced()) ch->recheckFlags();
		ch = ch->next;
	}
}

void client::recheckFlags(const char *channel)
{
	chan *ch = findChannel(channel);
	if(ch) ch->recheckFlags();
}

int client::connectToHUB()
{
	int fd, i, proto;
#ifdef HAVE_ADNS
	if(!(config.currentHub && config.currentHub->getHost().resolve_pending))
	{
#endif
		if(config.hub.failures < 3)
		{
			config.currentHub = &config.hub;
			DEBUG(printf(">>> hub = hub\n"));
		}
		else
		{
			for(i=0; i<MAX_ALTS; ++i)
			{
				if(!config.alt[i].isDefault())
				{
					if(!config.currentHub)
					{
						config.currentHub = &config.alt[i];
						DEBUG(printf(">>> hub = alt[%d]\n", i));
					}
					else if(config.currentHub->failures > config.alt[i].failures)
					{
						config.currentHub = &config.alt[i];
						DEBUG(printf(">>> hub = alt[%d]\n", i));
					}
				}
			}
		}

		if(!config.currentHub) config.currentHub = &config.hub;
#ifdef HAVE_ADNS
	}

	i = config.currentHub->getHost().updateDnsEntry();

	switch(i)
	{
		// resolve error or still resolving
		case -1 : ++config.currentHub->failures;
		case  0 : return -1;
	}
#endif
        if(config.currentHub->getHost().isIpv4())
                proto=4;
        else if(config.currentHub->getHost().isIpv6())
                proto=6;
	else
	{
            if(*config.currentHub->getHost().ip6)
                proto=6;
            else
                proto=4;
	}

	DEBUG(printf("[*] Connecting to HUB: %s (%s) port %d\n",
		  (const char *) config.currentHub->getHost().connectionString, 
		  proto==6 ? (const char *) config.currentHub->getHost().ip6 : (const char *) config.currentHub->getHost().ip4,
		  (int) config.currentHub->getPort()));

#ifdef HAVE_IPV6
	if(proto == 6)
		fd = doConnect6((const char *) config.currentHub->getHost().ip6, config.currentHub->getPort(), config.myipv6, -1);
	else
#endif
		fd = doConnect((const char *) config.currentHub->getHost().ip4, config.currentHub->getPort(), config.myipv4, -1);

	if(fd > 0)
	{
		net.hub.fd = fd;
		net.hub.status = STATUS_SYNSENT;
		net.hub.killTime = set.AUTH_TIMEOUT + NOW;

#ifdef HAVE_SSL
		if(config.currentHub->getHost().isSSL())
		{
			net.hub.enableSSL();
			net.hub.status |= STATUS_SSL_WANT_CONNECT | STATUS_SSL_HANDSHAKING;
		}
#endif

		return fd;
	}
	++config.currentHub->failures;
	return -1;
}

int client::connectToIRC(entServer *s)
{
	int n;
	int opt = 0;
	int proto = 0;

	net.irc.status = 0;
#ifdef HAVE_ADNS
	if(!(config.currentServer && config.currentServer->getHost().resolve_pending))
#else
	if(1)
#endif
	{
		if(s)
			config.currentServer = s;

		else
			config.currentServer = getRandomServer();

		if(!config.currentServer)
			return 0;
	}

#ifdef HAVE_ADNS
        n = config.currentServer->getHost().updateDnsEntry();

        switch(n)
        {
                // resolve error or still resolving
                case -1 :
                case  0 : return -1;
        }
#endif

	// if server name is an ipv4 or ipv6 address use the protocol of that address
	if(config.currentServer->getHost().isIpv4())
		proto=4;
	else if(config.currentServer->getHost().isIpv6())
		proto=6;
	else
	{
		// we resolved some hostname
		if(*config.currentServer->getHost().ip4  && *config.currentServer->getHost().ip6)
		{
			// hostname has an ipv4 and ipv6 address
			// priority: protocol of vhost (if the variable is set)
			if(!config.vhost.isDefault() && (proto=isValidIp(config.vhost)) !=0)
				;
			else
				// vhost is not set, prefer ipv6
				proto=6;
		}
		else
		{
			// use what it has
			if(*config.currentServer->getHost().ip6)
				proto=6;
			else
				proto=4;
		}
	}

        DEBUG(printf("[*] Connecting to IRC server: %s (%s) port %d with vhost %s\n",
                  (const char *) config.currentServer->getHost().connectionString,
                  proto==6 ? (const char *) config.currentServer->getHost().ip6 : (const char *) config.currentServer->getHost().ip4,
                  (int) config.currentServer->getPort(), (const char*) config.vhost));

#ifdef HAVE_IPV6
	if(proto == 6)
		n = doConnect6(config.currentServer->getHost().ip6, config.currentServer->getPort(), config.vhost, -1);
	else
#endif
		n = doConnect((const char *) config.currentServer->getHost().ip4, config.currentServer->getPort(), config.vhost, -1);

	if(n > 0)
	{
		const char *pass;
		memset(&net.irc, 0, sizeof(inetconn));
		net.irc.fd = n;
		net.irc.status |= STATUS_SYNSENT | opt;
		net.irc.killTime = set.AUTH_TIMEOUT + NOW;
		net.irc.pass = NULL;

		if(config.currentServer)
		{
		    pass = (const char *) config.currentServer->getPass();

		    if(pass && *pass)
			net.irc.pass = (char *) pass;
		}

#ifdef HAVE_SSL
		if(config.currentServer->isSSL())
		{
			net.irc.enableSSL();
			net.irc.status |= STATUS_SSL_WANT_CONNECT | STATUS_SSL_HANDSHAKING;
		}
#endif
		HOOK(connecting, connecting());
		stopParsing=false;
	}

	//config.currentServer=NULL;
	return n > 0 ? n : 0;
}

void client::checkQueue()
{
	if(penalty >= 10) return;
	chan *ch = first;

	autoRecheck();
	protmodelist::expireAll();

	/*
	 *   0 - normal rejoin mode
	 *  >0 - burst join
	 */

	if(waitForMyHost)
	{
		if(userlist.wildFindHost(userlist.me(), ME.mask) != -1)
		{
			joinDelay = NOW + set.QUARANTINE_TIME;
			waitForMyHost = false;
			DEBUG(net.send(HAS_N, "[D] kk, thx"));
		}
	}
	if(!joinDelay)
	{
		rejoinCheck();
	}
	if(joinDelay <= NOW)
	{
		joinDelay = 0;
		joinAllChannels();
	}

	while(ch)
	{
		if(ch->synced())
		{
#ifdef HAVE_ADNS
			ch->updateDnsEntries();
#endif
			ch->recheckBans();

			if(!(ch->me->flags & IS_OP))
			{
				if(!ch->opedBots.entries()) ch->initialOp = 0;
				if((ch->chset->BOT_AOP_MODE == 0) ||
					(ch->chset->BOT_AOP_MODE == 1 && ch->toKick.entries() <= 4) ||
					(ch->initialOp <= NOW - set.ASK_FOR_OP_DELAY && ch->initialOp))
				{
					ch->requestOp();
					ch->initialOp = NOW + set.ASK_FOR_OP_DELAY;
				}
			}
			else
			{
				/*
				if(ch->toKick.entries() - ch->sentKicks)
				{
					j = getRandomItems(MultHandle, ch->toKick.start(), ch->toKick.entries() - ch->sentKicks, 4, KICK_SENT);
					ch->kick4(MultHandle, j);
					return;
				}
				*/
				if(!ch->flushKickQueue())
				{
					ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);
					ch->modeQ[PRIO_LOW].flush(PRIO_LOW);
					ch->updateLimit();
				}
			}

			if(set.CLONE_LIFE_TIME)
			{
				ch->proxyClones.expire(set.CLONE_LIFE_TIME, NOW);
				ch->hostClones.expire(set.CLONE_LIFE_TIME, NOW);
				ch->identClones.expire(set.CLONE_LIFE_TIME, NOW);
			}
			ch->wasop->expire();
		}

		if(!ch->fullSynced && ch->synlevel >= 2 && ch->list[BAN].received
			&& (ch->list[INVITE].received || !ME.server.isupport.find("INVEX"))
			&& (ch->list[EXEMPT].received || !ME.server.isupport.find("EXCEPTS"))
			&& (ch->list[REOP].received || chan::getTypeOfChanMode('R') != 'A'))
		{
			// full sync'd

			if(ch->me->flags & IS_OP)
				ch->justSyncedAndOped();

                        HOOK(justSynced, justSynced(ch));
                        stopParsing=false;

			ch->fullSynced=true;
		}

		if(ch->fullSynced && ch->me->flags & IS_OP)
			ch->listFullCheck();

		ch = ch->next;
	}

	/* handle dynamic bans, invites and exempts */
	ch = first;
	ptrlist<masklist_ent>::link *m;
	while(ch)
	{
		if(ch->synced() && ch->me->flags & IS_OP)
		{

			if(ch->chset->DYNAMIC_BANS)
			{
				m = NULL;
				while((m = ch->list[BAN].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-b ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			if(ch->chset->DYNAMIC_INVITES)
			{
				m = NULL;
				while((m = ch->list[INVITE].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-I ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			if(ch->chset->DYNAMIC_EXEMPTS)
			{
				m = NULL;
				while((m = ch->list[EXEMPT].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-e ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			ch->modeQ[PRIO_LOW].flush(PRIO_LOW);
		}
		ch = ch->next;
	}

	if(penalty) return;

	/* if nothing to do, gather channel information */
	int level, i;
	Pchar buf;
	for(level=1; level<10; level+=2)
	{
		if((level == 5 && !ME.server.isupport.find("INVEX"))
		   || (level == 7 && !ME.server.isupport.find("EXCEPTS"))
		   || (level == 9 && chan::getTypeOfChanMode('R') != 'A'))
		{
			for(ch=first; ch; ch=ch->next)
			{
				if(level == ch->synlevel)
					ch->synlevel+=2;
			}

			continue;
		}

		ch = first;
		i = 0;
		while(ch)
		{
			if(ch->synlevel == level)
			{
				if(i) buf.push(",");
				buf.push((const char *) ch->name);
				++ch->synlevel;
				++i;
				if(i == ME.server.isupport.max_mode_targets) break;
			}
			ch = ch->next;
		}
		if(i)
		{
			switch(level)
			{
				case 1: net.irc.send("MODE %s", buf.data); break;
				case 3: net.irc.send("MODE %s b", buf.data); break;
				case 5: net.irc.send("MODE %s I", buf.data); break;
				case 7: net.irc.send("MODE %s e", buf.data); break;
				case 9: net.irc.send("MODE %s R", buf.data); break;
				default: break;
			}

			return;
		}
	}

	queue->flush(&net.irc);
}

void client::gotUserQuit(const char *mask, const char *reason)
{
	char *a, *nick;
	chan *ch = first;
	int netsplit = reason && wasoptest::checkSplit(reason);

	a = strchr((char *)mask, '!');
	if(a) mem_strncpy(nick, mask, abs(a - mask) + 1);
	else mem_strcpy(nick, mask);

	while(ch)
	{
		if (ch->synced())
		{
			HOOK(pre_part, pre_part(mask, ch->name, reason, true));
			stopParsing=false;

			ch->gotPart(nick, netsplit);
			HOOK(post_part, post_part(mask, ch->name, reason, true));
			stopParsing=false;
		}
		ch = ch->next;
	}

	if(config.keepnick && !netsplit && !strcasecmp(nick, config.nick))
	{
		net.irc.send("NICK %s", (const char *) config.nick);
		ME.nextNickCheck = 0;
	}

	free(nick);
}

void client::removeChannel(const char *name)
{
	chan *ch = first;
	int n;

	if(!channels) return;

	if(!strcasecmp(first->name, name))
	{
		first = first->next;
		if(first) first->prev = NULL;
		delete(ch);
		--channels;
		if(!channels) last = NULL;
	}
	else if(!strcasecmp(last->name, name))
	{
		ch = last->prev;
		ch->next = NULL;
		delete(last);
		--channels;
		last = ch;
	}
	else
	{
		ch = first->next;
		while(ch)
		{
			if(!strcasecmp(ch->name, name))
			{
				ch->prev->next = ch->next;
				if(ch->next) ch->next->prev = ch->prev;
				delete(ch);
				--channels;
				break;
			}
			ch = ch->next;
		}
	}
	current = first;

	n = userlist.findChannel(name);
	if(n != -1) userlist.chanlist[n].status &= ~(JOIN_SENT | WHO_SENT);
}

chan *client::findNotSyncedChannel(const char *name)
{

	if(current)
		if(!strcasecmp(current->name, name)) return current;

	current = first;
	while(current)
	{
		if(!strcasecmp(current->name, name)) return current;
		current = current->next;
	}
	return NULL;
}

chan *client::findChannel(const char *name)
{

	if(current)
		if(!strcasecmp(current->name, name) && current->synced()) return current;

	current = first;
	while(current)
	{
		if(current->synced() && !strcasecmp(current->name, name)) return current;
		current = current->next;
	}
	return NULL;
}


chan *client::createNewChannel(const char *name)
{
	int n = userlist.findChannel(name);

	if(n > -1 && userlist.isRjoined(n))
	{
		if(!channels)
		{
			first = current = last = new(chan);
			current->prev = current->next = NULL;
			current->name = name;
		}
		else
		{
			current = last->next = new(chan);
			current->prev = last;
			current->next = NULL;
			current->name = name;
			last = current;
		}

		userlist.chanlist[n].status &= ~JOIN_SENT;
		current->chset = userlist.chanlist[n].chset;
		current->wasop = userlist.chanlist[n].wasop;
		current->protlist[BAN] = userlist.chanlist[n].protlist[BAN];
		current->protlist[INVITE] = userlist.chanlist[n].protlist[INVITE];
		current->protlist[EXEMPT] = userlist.chanlist[n].protlist[EXEMPT];
		current->protlist[REOP] = userlist.chanlist[n].protlist[REOP];

		current->channum = n;
		if(userlist.chanlist[n].allowedOps &&
			userlist.chanlist[n].allowedOps->since + 60 < NOW)
		{
			delete userlist.chanlist[n].allowedOps;
			userlist.chanlist[n].allowedOps = NULL;
		}
		//current->key = userlist.chanlist[n].pass;

		++channels;
		return current;
	}
	return NULL;
}

/* Constructor */
client::client()
{
	NOW = time(NULL);
	first = last = current = NULL;
	overrider = uid = ircip = nick = ident = host = mask = "";
	nextReconnect = nextRecheck = nextNickCheck = 0;
	nextConnToIrc = nextConnToHub = startedAt = NOW;
	channels = joinDelay = hostNotify = 0;
	waitForMyHost = false;
	ircConnFailDelay = 15;
	queue=new fifo(0, 3);
}

/* Destruction derby */
client::~client()
{
	chan *ch = first;
	chan *p;

	while(ch)
	{
		p = ch;
		ch = ch->next;
		delete p;
	}

	server.reset();
}

void client::reset()
{
	chan *ch = first;
	chan *p;

	while(ch)
	{
		p = ch;
		ch = ch->next;

		//p->wasop is infact chanlist->wasop
		//so it will preserve itself across delete/new
		if(p->wasop && !p->wasop->isEmpty())
		{
			ptrlist<chanuser>::iterator u = p->users.begin();
			while(u)
			{
				if((u->flags & (IS_OP | HAS_F)) == IS_OP && !p->toKick.find(u))
					p->wasop->add(u);

				u++;
			}
		}
		delete p;
	}

	first = last = current = NULL;
	overrider = uid = ircip = nick = ident = host = mask = "";
	nextRecheck = nextNickCheck = 0;
	channels = joinDelay = hostNotify = 0;
	waitForMyHost = false;
	ircConnFailDelay = 15;

	for(int i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].allowedOps)
		{
			delete userlist.chanlist[i].allowedOps;
			userlist.chanlist[i].allowedOps = NULL;
		}
		userlist.chanlist[i].status &= PRIVATE_CHAN;
		//userlist.chanlist[i].nextjoin = 0;
	}

	server.reset();
	queue->data.clear();
}

/** Sends a privmsg.
 * \author patrick <patrick@psotnic.com>
 */

void client::privmsg(const char *target, const char *lst, ...)
{
        va_list list;
        char buffer[MAX_LEN];

        va_start(list, lst);
        vsnprintf(buffer, MAX_LEN, lst, list);
        va_end(list);

        queue->push("PRIVMSG %s :%s", target, buffer);
}

/** Sends a notice.
  * \author patrick <patrick@psotnic.com>
  */

void client::notice(const char *target, const char *lst, ...)
{
        va_list list;
        char buffer[MAX_LEN];

        va_start(list, lst);
        vsnprintf(buffer, MAX_LEN, lst, list);
        va_end(list);

	queue->push("NOTICE %s :%s", target, buffer);
}

void client::quit(const char *reason)
{
    if(net.irc.fd > 0)
        net.irc.send("QUIT :%s", reason ? reason : (const char *) set.QUITREASON);
}
