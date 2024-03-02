/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *   Copyright (C) 2009-2010 psotnic.com development team                  *
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

static char arg[10][MAX_LEN];
static char *reason;

void emulateIRCDIntro(inetconn *c);

void parse_bot(inetconn *c, char *data)
{
	reason = NULL;

	if(!c)
		return;

	if(!data || *data == '\0')
		return;

	str2words(arg[0], data, 10, MAX_LEN);

	/* REGISTER CONNECTION */
	if(!(c->status & STATUS_REGISTERED))
	{
		switch(c->tmpint)
		{
			case 1:
			{
				if(!strcmp(arg[0], config.botnetword) && c->listener_opt & (LISTEN_ALL | LISTEN_BOTS))
				{
					c->enableCrypt((const char *) config.botnetword, strlen(config.botnetword));
					c->tmpstr = (char *) malloc(AUTHSTR_LEN + 1);
					++c->tmpint;
					MD5CreateAuthString(c->tmpstr, AUTHSTR_LEN);
					c->tmpstr[32] = '\0';
					c->send("%s", c->tmpstr);
					return;
				}
				
				/* maybe that's owner */
				if(/*config.bottype == BOT_MAIN &&*/ c->listener_opt & (LISTEN_ALL | LISTEN_USERS))
				{
					if(!strncmp(data, "CAP LS", 6))
						return;

					if(!strcmp(data, "JOIN :")) {
						// irssi compatibility
						c->sendRaw(":%s 451 * :You have not registered", (const char*)config.partyline_servername);
						return;
					}

					if(!strcmp(arg[0], "PASS") && strlen(arg[1]))
					{
						// it's an irc client
						// format: PASS username:password
						// -- patrick
						char *ownerpass, *handle, *pass, *p;
						HANDLE *h;

						if(arg[1][0] == ':')
							strcpy(arg[1], arg[1]+1);

						if((ownerpass=strtok_r(arg[1], ":", &p))
						   && (handle=strtok_r(NULL, ":", &p))
						   && (pass=strtok_r(NULL, ":", &p)))
						{
						  if(config.checkOwnerpass(ownerpass))
						  {
							mem_strcpy(c->name, handle);

							if ((h = userlist.checkPartylinePass(c->name, pass, HAS_P)))
							{
								if(!h->addr->match(c->getPeerIpName()))
								{
									reason = push(NULL, c->name, ": invalid ip", NULL);
									break;
								}

								c->status |= STATUS_CONNECTED | STATUS_PARTY | STATUS_REGISTERED | STATUS_TELNET;
								c->status &= ~STATUS_BOT;
								c->tmpint = 0;
								c->killTime = 0;
								c->handle = h;
								if (c->tmpstr) free(c->tmpstr);
								c->tmpstr = NULL;
								//c->echo(1);
								emulateIRCDIntro(c);
								c->status |= STATUS_IRCCLIENT;
								sendLogo(c);
								ignore.removeHit(c->getPeerIp4());
								return;
							}
							else
								reason = push(NULL, "invalid username or password", NULL);
                                                  }
					          else
						    reason = push(NULL, "bad ownerpass", NULL);
						}
						else
							reason = push(NULL, "invalid PASS (syntax error)", NULL);
						break;
					}

					if(config.checkOwnerpass(arg[0]))
					{
						c->status |= STATUS_CONNECTED | STATUS_PARTY | STATUS_TELNET;
						c->status &= ~STATUS_BOT;
						c->tmpint = 1;
						c->killTime = NOW + set.AUTH_TIMEOUT;

						c->send("");
						c->send("Welcome %s", c->getPeerIpName());
						c->send("");
						c->send("Enter your login.");

						return;
					}
				}
				DEBUG(printf("[D] telnet creep: %s [%d]\n", arg[0], strlen(arg[0])));
				reason = push(NULL, "telnet creep", NULL);
				break;
			}
			case 2:
			{
				if(strlen(arg[1]))
				{
					struct sockaddr_in peer;
					HANDLE *h = userlist.findHandle(arg[0]);

					if(h && net.findConn(h))
					{
						reason = push(NULL,  arg[0], ": duplicate connection",NULL);
						break;
					}
					// let leafs link to main -- patrick
					if(h && config.bottype == BOT_MAIN && !userlist.isSlave(h) && !userlist.isLeaf(h))
					{
						reason = push(NULL, arg[0], ": neither a slave nor a leaf", NULL);
						break;
					}

					if(h && config.bottype == BOT_SLAVE && !userlist.isLeaf(h))
					{
						reason = push(NULL, arg[0], ": not a leaf", NULL);
						break;
					}

					socklen_t peersize = sizeof(struct sockaddr_in);
					getpeername(c->fd, (sockaddr *) &peer, &peersize);

					if(!h)
					{
						if(isRealStr(arg[0]) && strlen(arg[0]) <= MAX_HANDLE_LEN)
							reason = push(NULL, arg[0], ": not a bot", NULL);
						else
							reason = push(NULL, "(crap here): not a bot", NULL);

						break;
					}
					if(!h->pass)
					{
						reason = push(NULL, arg[0], ": no password set", NULL);

						break;
					}
					if(!h->ip || peer.sin_addr.s_addr == h->ip || (h->addr->data.entries() && h->addr->match(c->getPeerIpName())))
					{
   						if(MD5HexValidate(arg[1], c->tmpstr, strlen(c->tmpstr), h->pass, 16))
						{
							++c->tmpint;
							c->handle = h;
							free(c->tmpstr);
							c->tmpstr = NULL;
							return;
						}
						else
						{
							reason = push(NULL, arg[0], ": wrong botpass", NULL);
							break;
						}
					}
					else
					{
						reason = push(NULL, arg[0], ": invalid botip", NULL);
						break;
					}
				}
				reason = push(NULL, "This should not happen (1)", NULL);
				break;
			}
			case 3:
			{
				if(strlen(arg[0]))
				{
					char hash[33];

					++c->tmpint;
					MD5HexHash(hash, arg[0], AUTHSTR_LEN, c->handle->pass, 16);
					c->send("%s %s %s", (const char *) config.handle, userlist.first->next->creation->print(), hash);
					return;
				}
				reason = push(NULL, "This should not happen (2)", NULL);
				break;
			}
			case 4:
			{
				/* S_REGISTER <S_VERSION> <userlist.SN> [ircnick [irc server] ] */
				if(!strcmp(arg[0], S_REGISTER) && strlen(arg[2]))
				{
					//if(strcmp(arg[1], S_VERSION))
					//{
					//	net.send(HAS_N, "[!] %s has different version: %s", c->handle->name, arg[1]);
					//}

					mem_strcpy(c->name, arg[3]);
					mem_strcpy(c->origin, arg[4]);
					c->status |= STATUS_CONNECTED | STATUS_REGISTERED | STATUS_BOT;
					c->tmpint = 0;
					c->killTime = NOW + set.CONN_TIMEOUT;
					c->lastPing = NOW;

					c->send("%s %s", S_REGISTER, (const char *) ME.nick);

					c->enableCrypt(c->handle->pass, 16);

					/* update ul */
					if(userlist.SN != strtoull(arg[2], NULL, 10))
					{
						net.send(HAS_N, "%s is linked (sending userlist)", c->handle->name);
						userlist.send(c);
					}
					else net.send(HAS_N, "%s is linked and operational", c->handle->name);


					/* send list of bots */
					net.sendBotListTo(c);
					net.propagate(c, "%s %s %s", S_BJOIN, c->name, c->origin);

					/* check bot host */
					if(config.bottype == BOT_MAIN)
						c->send("%s *", S_CHKHOST);

					ignore.removeHit(c->getPeerIp4());
					return;
				}

				if(!strcmp(arg[0], S_IUSEMODULES))
					return;

				reason = push(NULL, "This should not happen (3)", NULL);
				break;
			}
			default:
			break;
		}
		/* HUH */
		if(!reason)
			reason = push(NULL, "Unknown error", NULL);
		c->close(reason);
		free(reason);
		return;
	}

	/* PARSE DATA FROM REGISTERED BOT */
	c->killTime = NOW + set.CONN_TIMEOUT;

	if(!strcmp(arg[0], S_UL_UPLOAD_START))
	{
		c->close("Go fuck yourself");
		return;
	}
	if(!strcmp(arg[0], S_ULOK))
	{
		net.send(HAS_N, "%s is operational", c->handle->name);
		return;
	}
	parse_botnet(c, data);
}

/** missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

void emulateIRCDIntro(inetconn *c)
{
    c->sendRaw(":%s 001 %s :Welcome to the Internet Relay Network %s!%s@%s", (const char*)config.partyline_servername, (const char*)c->name, c->name, c->name, c->getPeerIpName());
    c->sendRaw(":%s 002 %s :Your host is %s, running version %s", (const char*)config.partyline_servername, (const char*)c->name, S_BOTNAME, S_VERSION);
    c->sendRaw(":%s 003 %s :This server was created ...", (const char*)config.partyline_servername, (const char*)c->name);
    c->sendRaw(":%s 004 %s %s %s %s %s", (const char*)config.partyline_servername, (const char*)c->name, (const char*)config.partyline_servername, S_VERSION, "oirw", "abeIiklmnopqstv");
    c->sendRaw(":%s 251 %s :There are 0 users and 0 invisible on 1 servers", (const char*)config.partyline_servername, (const char*)c->name);
    c->sendRaw(":%s 255 %s :I have 0 clients, 0 services and 0 servers", (const char*)config.partyline_servername, (const char*)c->name);
    c->sendRaw(":%s 422 %s :MOTD File is missing", (const char*)config.partyline_servername, (const char*)c->name);
}
