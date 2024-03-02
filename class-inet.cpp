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

char __port[16];

#ifdef HAVE_SSL
SSL_CTX *inet::server_ctx = NULL;
#endif

/* figure out whether or not this system has va_copy. FIXME: this could
   be done by configure */
#ifdef va_copy
#define MY_VA_COPY va_copy
#elif defined(__va_copy)
#define MY_VA_COPY __va_copy
#else
/* must be freebsd <= 4.11 ;-P if they don't have va_copy, then most of the
   time simple assignment will do */
#define MY_VA_COPY(a, b) ((a) = (b))
#endif

/* Network Connection Handling */

inetconn *inet::findMainBot()
{
	for(int i=0; i<max_conns; ++i)
		if(net.conn[i].isMain() && net.conn[i].fd)
			return &net.conn[i];

	return NULL;
}

inet::inet()
{
	conn = (inetconn *) malloc(sizeof(inetconn));
	memset(&conn[0], 0, sizeof(inetconn));
	max_conns = 1;
	conns = maxFd = 0;
}

inet::~inet()
{
	ptrlist<inet::listen_entry>::iterator le;

	net.hub.close("Going down");
	net.irc.close("Going down");

	for(le=net.listeners.begin(); le; le++)
		killSocket(le->fd);

	for(int i=0; i<max_conns; ++i)
		conn[i].close("Going down");

	resize();
}

void inet::resize()
{
	if(max_conns <= conns + 1)
	{
		int size = max_conns * 2;
		int i;

		conn = (inetconn *) realloc(conn, size*sizeof(inetconn));
		for(i=max_conns; i<size; ++i) memset(&conn[i], 0, sizeof(inetconn));
		max_conns = size;
		//DEBUG(printf("max_conns(up): %d\n", max_conns));

	}
	else if(conns <= max_conns / 4 && conns > 4)
	{
		int size = max_conns / 2;

		inetconn *old = conn;
		conn = (inetconn *) malloc(size*sizeof(inetconn));
		int i, j;

		for(i=j=0; i<max_conns; ++i)
		{
			if(old[i].fd)
			{
				memcpy(&conn[j], &old[i], sizeof(inetconn));
				++j;
			}
		}
		for(; j<size; ++j)
			memset(&conn[j], 0, sizeof(inetconn));

		free(old);
		max_conns = size;

		//DEBUG(printf("max_conns(down): %d\n", max_conns));
	}
}

inetconn *inet::addConn(int fd)
{
	int i;

	if(fd < 1) return NULL;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd < 1)
		{
			memset(&conn[i], 0, sizeof(inetconn));
			conn[i].fd = fd;
			conn[i].status = STATUS_CONNECTED;
			++conns;
			if(fd > maxFd) maxFd = fd;
			return &conn[i];
		}
	}
	return NULL;
}

/* connection can only have name when is registered, so this function returns only
   registered connections */
inetconn *inet::findConn(const char *name)
{
	int i;

	if(!conn) return NULL;

	if(hub.fd > 0 && hub.name) if(!strcmp(hub.name, name)) return &hub;
	for(i=0; i<max_conns; ++i)
		if(conn[i].fd > 0 && conn[i].name && !strcmp(conn[i].name, name)) return &conn[i];
	return NULL;
}

inetconn *inet::findConn(const HANDLE *h)
{
	int i;
	if(!conn || !h) return NULL;

	if(hub.handle == h && h) return &hub;
	for(i=0; i<max_conns; ++i)
		if(conn[i].handle == h && h) return &conn[i];
	return NULL;
}

int inet::closeConn(inetconn *c, const char *reason)
{
	int i, pos, fd, red, ret;

	if(!c || !conn) return 0;

	fd = c->fd;
	red = c->status & STATUS_REDIR;

	/* is this right connection? */
	pos = abs(c - &conn[0]);

	//DEBUG(printf("pos: %d\n", pos));

	if ((pos >= 0 && pos < max_conns) || c == &net.hub)
	{
		/* close given connection */
		if(c->isRegBot() && !c->isRedir()) net.propagate(c, "%s %s", S_BQUIT, reason);
		DEBUG(printf("kill: %s (%s)\n", c->name, reason));
		c->_close(reason);
		if(c != &net.hub) --conns;
		ret = 1;
	}
	else ret = 0;

	/* close redir conns */
	if(!red)
	{
		for(i=max_conns-1; i != -1; --i)
		{
			if(conn[i].fd == fd)
			{
				if(ret) net.propagate(&conn[i], "%s %s", S_BQUIT, reason);
				DEBUG(printf("red kill: %s\n", conn[i].name));
				conn[i]._close(reason);
				--conns;
			}
		}
	}
	return ret;
}

void inet::sendexcept(int excp, int who, const char *lst, ...)
{
	va_list list;
	int i;
	char buf[MAX_LEN];

	va_start(list, lst);
	vsnprintf(buf, MAX_LEN, lst, list);
	va_end(list);

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd > 0 && conn[i].status & STATUS_REGISTERED && conn[i].checkFlag(who)
		 && !(conn[i].status & STATUS_REDIR) && conn[i].fd != excp) 
			conn[i].send("%s", buf);
	}
	if(hub.fd > 0 && hub.fd != excp && hub.isRegBot()  && hub.checkFlag(who))
		net.hub.send("%s", buf);
}

void inet::sendOwner(const char *who, const char *lst, ...)
{
	va_list list;
	char buf[MAX_LEN];

	if(!who) return;

	if(config.bottype != BOT_MAIN)
	{
		HANDLE *h;

		if(!userlist.first)
			return;

		if(!userlist.first->next)
			return;

		h = userlist.first->next->next;

		while(h)
		{
			if(userlist.isMain(h))
			{
				inetconn *c = findConn(h);

				if(c)
				{
					va_start(list, lst);
					vsnprintf(buf, MAX_LEN, lst, list);
					va_end(list);
	
					c->send("%s %s %s %s", S_OREDIR, userlist.first->next->name, who, buf);
				}
			}
			h = h->next;
		}
	}
	//else
	//{
		for(int i=0; i<max_conns; ++i)
			if(conn[i].isRegOwner() && match(who, net.conn[i].handle->name))
			{
				va_start(list, lst);
				vsnprintf(buf, MAX_LEN, lst, list);
				va_end(list);

				conn[i].send("%s", buf);
			}
	//}
}


/*void inet::sendOwners(int level, const char *lst, ...)
{
        va_list list;
        int i;
        char buffer[MAX_LEN];

        va_start(list, lst);
        vsnprintf(buffer, MAX_LEN, lst, list);
        va_end(list);

        sendOwner("*", "%s", buffer);
}*/

void inet::sendUser(const char *who, const char *lst, ...)
{
	va_list list;
	char buf[MAX_LEN];

	if(!who) return;

	if(config.bottype != BOT_MAIN)
	{
		HANDLE *h;

		if(!userlist.first)
			return;

		if(!userlist.first->next)
			return;

		h = userlist.first->next->next;

		while(h)
		{
			if(userlist.isMain(h))
			{
				inetconn *c = findConn(h);

				if(c)
				{
					va_start(list, lst);
					vsnprintf(buf, MAX_LEN, lst, list);
					va_end(list);
	
					c->send("%s %s %s %s", S_OREDIR, userlist.first->next->name, who, buf);
				}
			}
			h = h->next;
		}
	}
	//else
	//{
		for(int i=0; i<max_conns; ++i)
			if(conn[i].isRegUser() && match(who, net.conn[i].handle->name))
			{
				va_start(list, lst);
				vsnprintf(buf, MAX_LEN, lst, list);
				va_end(list);

				conn[i].send("%s", buf);
			}
	//}
}

void inet::send(int who, const char *lst, ...)
{
	va_list list;
	int i;
	char buffer[MAX_LEN];

	va_start(list, lst);
	vsnprintf(buffer, MAX_LEN, lst, list);
	va_end(list);

	if(who == HAS_N)
	{
		sendOwner("*", "%s", buffer);
		return;
	}

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd > 0 && conn[i].status & STATUS_REGISTERED && conn[i].checkFlag(who)
		 && !(conn[i].status & STATUS_REDIR)) 
			conn[i].send("%s", buffer);
	}
}

void inet::sendBotListTo(inetconn *c)
{
	int i;

	if(c)
	{
		for(i=0; i<max_conns; ++i)
		{
			if(&conn[i] != c && conn[i].isRegBot())
			{
				c->send("%s %s %s %s %s %s", S_FORWARD, conn[i].handle->name, " * ",
					S_BJOIN, conn[i].name, conn[i].origin);
			}
		}
		if(net.hub.fd && c != &net.hub && net.hub.isRegBot())
			c->send("%s %s %s %s %s %s", S_FORWARD, hub.handle->name, " * ",
					S_BJOIN, hub.name, hub.origin);

	}
}

void inet::propagate(inetconn *from, const char *str, ...)
{
  char buffer1[MAX_LEN], buffer2[MAX_LEN];
  va_list list;

  if(!str || *str == '\0')
    return;

  snprintf(buffer1, MAX_LEN, "%s %s * ", S_FORWARD, from ? from->handle->name : (const char *) config.handle);

  va_start(list, str);
  vsnprintf(buffer2, MAX_LEN, str, list);
  va_end(list);

  strncat(buffer1, buffer2, sizeof(buffer1) - strlen(buffer1) - 1);
  net.sendexcept(from ? from->fd : 0, HAS_B, "%s", buffer1);
}

#ifdef HAVE_DEBUG
void inet::display()
{
	int i;

	if(hub.fd) printf("hub: %s\n", hub.name);
	for(i=0; i<max_conns; ++i)
		if(conn[i].fd > 0) printf("conn[%d]: %s\n", i, conn[i].name);
}
#endif

inetconn *inet::findRedirConn(inetconn *c)
{
	int i;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd == c->fd && conn[i].isRegBot() && !(conn[i].status & STATUS_REDIR))
			return &conn[i];
	}
	if(hub.fd == c->fd && hub.isRegBot()) return &hub;

	return NULL;
}

int inet::bidMaxFd(int fd)
{
	if(fd > maxFd) maxFd = fd;
	return maxFd;
}


/*
 *
 * Specific Internet Connection Handling
 *
 */

inetconn::inetconn()
{
	memset(this, 0, sizeof(inetconn));
	write.buf = NULL;
	write.pos = 0;
	write.len = 0;
#ifdef HAVE_SSL
	ssl_buffer = NULL;
#endif
}

inetconn::~inetconn()
{
	close();
}

void inetconn::close(const char *reason)
{
	if(fd > 0)
	{
		if(!net.closeConn(this, reason)) _close(reason);
	}
	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}
}

void inetconn::_close(const char *reason)
{
	/* no sense of killing nothing */
	//if(!fd) return;
	int killhim = !(status & STATUS_REDIR);

	if(status & STATUS_CONNECTED)
	{
		if(this == &net.irc)
		{
			if(status & STATUS_KLINED)
			{
				HOOK(klined, klined(reason));
				if(stopParsing)
					stopParsing=false;
			}
			else
			{
                        	if(status & STATUS_REGISTERED)
                        	{
                                	net.propagate(NULL, "%s", S_CHNICK);

                                	HOOK(disconnected, disconnected(reason));
                                	if(stopParsing)
                                        	stopParsing=false;
                                	else
                                        	net.send(HAS_N, "[-] Disconnected from server %s (%s)", net.irc.name, reason);
                        	}
				else
					net.send(HAS_N, "[-] Cannot connect to %s (%s)", config.currentServer ? (const char *) config.currentServer->getHost().connectionString : "unknown server", reason);

			}
			ME.reset();
		}
		else if(this == &net.hub)
		{
			if(net.hub.status &  STATUS_REGISTERED)
			{
				net.propagate(&net.hub, "%s %s", S_BQUIT, reason);
			}
			else
			{
				++config.currentHub->failures;
				if(config.hub.failures > 0 && config.currentHub != &config.hub)
					--config.hub.failures;
			}

			if(userlist.ulbuf)
			{
				delete userlist.ulbuf;
				userlist.ulbuf = NULL;
			}
		}
		else
		{
			if(status & STATUS_REGISTERED)
			{
				if(status & STATUS_PARTY)
				{
					net.send(HAS_N, "%s has left the partyline (%s)", handle->name, reason);
					if(!checkFlag(HAS_N))
						send("%s has left the partyline (%s)", handle->name, reason);
				}
				else if(status & STATUS_BOT)
				{
					if(!(status & STATUS_REDIR)) net.send(HAS_N, "[-] Lost connection to %s (%s)", handle->name, reason);
				}
			}
			else if(!(status & STATUS_SILENT)) net.send(HAS_N, "[-] Lost %s / %s (%s)", getPeerIpName(), getPeerPortName(), reason);
		}
	}
	else
	{
		if(this == &net.hub)
		{
			++config.currentHub->failures;
			if(config.hub.failures > 0 && config.currentHub != &config.hub)
				--config.hub.failures;
		}
	}


	if(read.buf) free(read.buf);
	if(write.buf) { free(write.buf); write.buf=NULL; }
	if(name) free(name);
	if(tmpstr) free(tmpstr);
	if(origin) free(origin);
	if(fd && killhim) killSocket(fd);
	if(blowfish) delete blowfish;
#ifdef HAVE_SSL
	if(ssl_ctx) SSL_CTX_free(ssl_ctx);
	if(ssl) SSL_free(ssl);
	if(ssl_buffer) free(ssl_buffer);
#endif
	memset(this, 0, sizeof(inetconn));
}

int inetconn::sendRaw(const char *lst, ...)
{
    char buf[MAX_LEN];
    va_list list;
    bool foo=false;
    int bla;

    va_start(list, lst);
    vsnprintf(buf, MAX_LEN, lst, list);
    va_end(list);

    if(status & STATUS_REGISTERED)
    {
        status&=~STATUS_REGISTERED;
        foo=true;
    }

    bla=send("%s", buf);

    if(foo)
        status|=STATUS_REGISTERED;

    return bla;
}

int inetconn::send(const char *lst, ...)
{
	int size; 
	char *p, *q;
	char buf[MAX_LEN];
	va_list list;

        va_start(list, lst);
        vsnprintf(buf, MAX_LEN, lst, list);
        va_end(list);

	inetconn *conn = status & STATUS_REDIR ? net.findRedirConn(this) : this;

	if(!(status & STATUS_REDIR) && !isBot() && isRegUser())
	{
		char tmp[MAX_LEN], timestamp[MAX_LEN];

		if(status & STATUS_IRCCLIENT)
		{
			strcpy(tmp, buf);
			snprintf(buf, MAX_LEN, ":partyline!partyline@%s PRIVMSG %s :%s", (const char*)config.partyline_servername, name, tmp);
		}

		else
		{
		    strftime(timestamp, MAX_LEN, "[%H:%M]", localtime(&NOW));
		    strcpy(tmp, buf);
		    snprintf(buf, MAX_LEN, "%s %s", timestamp, tmp);
		}
	}

#ifdef HAVE_DEBUG
	if(debug)
	{
		if(!conn)
		{
			printf("### class inet is broken\n");
			printf("### info this->name: %s\n", name);
			printf("### exit(1)\n");
			exit(1);
		}

		if(conn->fd < 1)
		{
			printf("### %s|%s: bad fd\n", (const char *) config.handle, conn->name);
        	conn->close("Bad file descryptor");
			return 0;
		}
	}
#endif

	if(conn->fd < 1) return 0;

	//p = va_push(NULL, ap, lst, size);
	size = strlen(buf) + 3;
	p = (char *) malloc(size);
        memset(p, size, 0);
        strcpy(p, buf);

	if(status & STATUS_REDIR)
	{
		q = push(NULL, S_FORWARD, " ", (const char *) config.handle, " ", handle->name, " ", p, "###", NULL);
		free(p);
		p = q;
		size = strlen(p);
	}

	if(conn == &net.irc)
	{
		p[size-3] = '\r';
		p[size-2] = '\n';
		p[size-1] = '\0';
		--size;

		if(pset.debug_show_irc_write & 1
#ifdef HAVE_DEBUG
		|| debug
#endif
		)
//			printf("[D] send[%s]: %s", conn->name, p);
		if(pset.debug_show_irc_write & 2)
			net.send(HAS_N, "[D] send[%s]: %s", conn->name ? conn->name : "???", p);
	}
	else
	{
		p[size-3] = '\n';
		p[size-2] = '\0';
		size -= 2;
	}

	if(conn->blowfish)
	{
		char *blowbuf = (char *) malloc(size + 8);
		char *tmp = (char *) malloc(size + 8);
		memset(tmp, 0, size + 8);
		strcpy(tmp, p);
		free(p);
		p = tmp;
		size = conn->blowfish->Encode((unsigned char *) p, (unsigned char *) blowbuf, size);
		DEBUG(printf("[C] send[%s]: %s", conn->name, p));
		free(p);
		p = blowbuf;
	}
	else
	{
		if(status & STATUS_SSL)
		{
			DEBUG(printf("[S] send[%s]: %s", conn->name, p));
		}
		else
		{
			DEBUG(printf("[*] send[%s]: %s", conn->name, p));
		}
	}

	if(conn == &net.irc/* && (net.irc.status & STATUS_REGISTERED)*/)
		penalty.calculatePenalty(p);

	if(conn->write.buf)
	{
		conn->write.buf = (char *) realloc(conn->write.buf, conn->write.len + size);
		memcpy(conn->write.buf + conn->write.len, p, size);
		conn->write.len += size;

	}
	else
	{
		int n;
#ifdef HAVE_SSL
		if(status & STATUS_SSL)
		{
			if(!ssl_buffer)
			{
				// when SSL_write() fails, it must be repeated exactly with the same parameters
				// - patrick

				ssl_buffer=strdup(p);
				n = SSL_write(status & STATUS_REDIR ? net.hub.ssl : ssl, ssl_buffer, size);
				if(n < 0)
				{
					int ret = SSL_get_error(ssl, n) ;
					if(ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE)
					{
					
						DEBUG(printf("[D] SSL: write error: want read/write\n"));
					}
				}

				else
				{
					free(ssl_buffer);
					ssl_buffer=NULL;
				}
			}

			else
				n = -1;
		}
		else
			n = ::write(conn->fd, p, size);
#else
		n = ::write(conn->fd, p, size);
#endif
		if(n == -1)
			n = 0;
		if(n != size)
		{
			conn->write.len = size-n;
			conn->write.pos = 0;
			conn->write.buf = (char *) malloc(size-n);
			memcpy(conn->write.buf, p+n, size-n);

		}
	}

	free(p);
	return size;
}

int inetconn::readln(char *buf, int len, int *ok)
{
	int n, i, ret;

	//repeat:
#ifdef HAVE_SSL
	if((ret = status & STATUS_SSL ? SSL_read(ssl, buf, 1) : ::read(fd, buf, 1)) > 0)
#else
	if((ret = ::read(fd, buf, 1)) > 0)
#endif
	{
		//printf("--- read: 0x%x %c\n", buf[0], buf[0]);
		if(blowfish && !blowfish->smartDecode(buf[0], buf))
			return 0;

		for(i=0; i < (blowfish ? 8 : 1); ++i)
		{
			if(buf[i] == '\n' || buf[i] == '\0')
			{
				if(read.buf)
				{
					da_end:
					n = read.len;
					strncpy(buf, read.buf, n);
					if(n && (buf[n-1] == '\r' || buf[n-1] == '\n'))
						--n;
					
					buf[n] = '\0';
					free(read.buf);
					memset(&read, 0, sizeof(read));
				}
				else
				{
					n = 0;
					buf[0] = '\0';
				}
				
				if(n)
				{
					DEBUG(printf("[%c] read[%s]: %s\n", blowfish ? 'C' : '*', name, buf));

					if(this == &net.irc)
					{
						if(pset.debug_show_irc_read & 1)
							printf("[D] read[%s]: %s\n", name, buf);
						if(pset.debug_show_irc_read & 2)
							net.send(HAS_N, "[D] read[%s]: %s", name ? name : "???", buf);
					}
				}
				
				if(ok)
					*ok = 1;
				
				return n;
			}
			else
			{
				if(!read.buf)
				{
					read.buf = (char *) malloc(len);
					read.buf[0] = buf[i];
					read.len = 1;
				}
				else
				{
					if(read.len == MAX_LEN - 2)
					{
						//buffer overflow attempt
						if(ok)
							*ok = 0;
						return -1;
					}
					read.buf[read.len++] = buf[i];
					
				}
			}
		}
		
		if(ok)
			*ok = 0;
		return 0;
	}
#ifdef HAVE_SSL
	else if(status & STATUS_SSL && ((n = SSL_get_error(ssl, ret)) == SSL_ERROR_WANT_READ || n == SSL_ERROR_WANT_WRITE))
	{
		DEBUG(printf("[D] SSL: read error: want read/write\n"));
		//goto repeat;
		if(ok)
			*ok = 0;
		return 0;
	}
#endif
	else if(read.buf)
		goto da_end;

	if(ok)
		*ok = ret ? 0 : 1;
	
	return -1;
}

int inetconn::checkFlag(int flag, int where)
{
	return handle ? (handle->flags[where] & flag) : 0;
}

int inetconn::isRedir()
{
	return fd > 0 && (status & STATUS_REDIR);
}

int inetconn::isRegBot()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_B);
}

int inetconn::isRegOwner()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_N);
}

int inetconn::isRegUser()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_P);
}

int inetconn::sendPing()
{
	if(this == &net.irc)
	{
		if(!lagcheck.inProgress && penalty < 5 && (status & STATUS_REGISTERED) && NOW >= lagcheck.next)
		{
			send("PING %s", ME.server.name);
			lagcheck.inProgress=true;
			gettimeofday(&lagcheck.sent, NULL);
		}
	}


	else if(!(status & STATUS_REDIR) && (status & STATUS_REGISTERED) && lastPing && NOW - lastPing > set.CONN_TIMEOUT / 2)
	{
		send("%s", S_FOO);
		lastPing = NOW;
		return 1;
	}
	return 0;
}

int inetconn::timedOut()
{
	if(status & STATUS_REDIR) return 0;

	if(this == &net.irc)
	{
		if(lagcheck.getLag()/1000 > set.CONN_TIMEOUT)
		{
			close("Ping timeout");
			return 1;
		}
	}

	else if(killTime && killTime <= NOW)
	{
		close("Ping timeout");
		return 1;
	}
	sendPing();
	return 0;
}
int inetconn::enableCrypt(const char *key, int len)
{
	return enableCrypt((const unsigned char *) key, len);
}

int inetconn::enableCrypt(const unsigned char *key, int len)
{
	if(fd < 0) return 0;

#ifdef HAVE_SSL
	if(status & STATUS_SSL)
		return 0;
#endif

	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}

	blowfish = new CBlowFish;

	if(len == -1) len = strlen((char *) key);
	if(len > 16) blowfish->Initialize(const_cast<unsigned char*>(key), len);
	else
	{
		unsigned char digest[16];
		MD5Hash(digest, (char *) key, len);
		blowfish->Initialize(digest, len);
	}
	return 1;
}

int inetconn::disableCrypt()
{
#ifdef HAVE_SSL
	if(status & STATUS_SSL)
		return 0;
#endif
	
	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
		return 1;
	}
	else return 0;
}

#define GETXPEERFUN int (*)(int s, struct sockaddr *name, socklen_t *namelen)

const char *inetconn::getPeerIpName()
{
	return getipstr(fd, getIpVersion(fd), (GETXPEERFUN) getpeername);
}

const char *inetconn::getMyIpName()
{
	return getipstr(fd, getIpVersion(fd), (GETXPEERFUN) getsockname);
}

char *inetconn::getPeerPortName()
{
	return itoa(getport(fd, (GETXPEERFUN) getpeername));
}

char *inetconn::getMyPortName()
{
	return itoa(getport(fd, (GETXPEERFUN) getsockname));
}


unsigned int inetconn::getPeerIp4()
{
	return getip4(fd, (GETXPEERFUN) getpeername);
}

unsigned int inetconn::getMyIp4()
{
	return getip4(fd, (GETXPEERFUN) getsockname);
}

int inetconn::open(const char *pathname, int flags, mode_t mode)
{
	if(mode) fd = ::open(pathname, flags, mode);
	else fd = ::open(pathname, flags);

	if(fd >= 0)
	{
		status = STATUS_FILE;
		mem_strcpy(name, getFileName((char *) pathname));
	}
	return fd;
}

/*
	version == 0					- 0.2.2rciles
	version == 1					- 0.2.2rciles
	version == 2					- 0.2.2rc13 (config file)
	version == 2 + STATUS_ULCRYPT	- 0.2.2rc13 (userfile)
*/
int inetconn::enableLameCrypt()
{
	struct stat info;

	if(fd < 0) return 0;

	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}

	if(!fstat(fd, &info))
	{
		blowfish = new CBlowFish;
		unsigned int version = 3 + (status & STATUS_ULCRYPT);

		psotnicHeader h;

		if(!info.st_size)
		{
			strcpy(h.id, "psotnic");
			h.version = version;
			::write(fd, &h, sizeof(psotnicHeader));
		}
		else
		{
			::read(fd, &h, sizeof(psotnicHeader));
			if(!strcmp(h.id, "psotnic"))
			{

				if(!(status & STATUS_ULCRYPT))
					h.version &= ~STATUS_ULCRYPT;
				version = h.version;

			}
			else
			{
				lseek(fd, 0, SEEK_SET);
				version = 0;
			}
		}

		unsigned char seed[16];
		if(version == (3 + STATUS_ULCRYPT) || version == htonl(3 + STATUS_ULCRYPT))
		{
			gen_ul_seed(seed);
			blowfish->Initialize(seed, 16);
			memset(seed, 0, 16);
		}
		else if(version == 3 || version == htonl(3))
		{
			gen_cfg_seed(seed);
			blowfish->Initialize(seed, 16);
			memset(seed, 0, 16);
		}
		else
		{
			printf("[!] Unsupported file format\n");
			exit(1);
		}

		return 1;
	}
	return 0;
}

int inetconn::isSlave()
{
	return (handle ? ul::isSlave(handle) : 0);
}

int inetconn::isLeaf()
{
	return (handle ? ul::isLeaf(handle) : 0);
}

int inetconn::isMain()
{
	return (handle ? ul::isMain(handle) : 0);
}

int inet::bots()
{
	int num = 0;

	for(int i=0; i<max_conns; ++i)
		if(conn[i].isRegBot()) ++num;

	return num;
}

int inet::owners()
{
	int num = 0;

	for(int i=0; i<max_conns; ++i)
		if(conn[i].isRegOwner()) ++num;

	return num;
}

//synchronious lookup
int inet::gethostbyname(const char *host, char *buf, int protocol)
{
	if(isValidIp(host))
	{
		strncpy(buf, host, MAX_LEN);
		return 1;
	}
#ifdef NO_6DNS
	struct hostent *h = ::gethostbyname(host);
#else
	struct hostent *h = gethostbyname2(host, protocol);
#endif
	if(h)
	{
		if(inet_ntop(protocol, h->h_addr, buf, MAX_LEN))
			return 1;
	}
	return 0;
}

void inetconn::echo(int n, char *app)
{
	if(n)
	{
		DEBUG(printf("send[%s]: \\xFF\\xFC\\x01\n", name));
		::write(fd, "\xFF\xFC\x01", 3);
	}
	else
	{
		DEBUG(printf("send[%s]: \\xFF\\xFB\\x01\n", name));
		::write(fd, "\xFF\xFB\x01", 3);
	}

	if(app)
	{
		DEBUG(printf("send[%s]: ^append^\n", name));
		::write(fd, app, strlen(app));
	}
}

int inetconn::isReg()
{
	return status & STATUS_REGISTERED;
}

int inetconn::isConnected()
{
	return status & STATUS_CONNECTED;
}

int inetconn::isBot()
{
	return status & STATUS_BOT;
}

void inetconn::writeBufferedData()
{
	int n;
#ifdef HAVE_SSL
	if(ssl_buffer)
	{
		int size = strlen(ssl_buffer);

		if(fd >= 0 && status & STATUS_SSL)
		{
			if((n = SSL_write(ssl, ssl_buffer, size)) == size)
			{
				free(ssl_buffer);
				ssl_buffer=NULL;
			}

			else if(n < 0)
			{
				int ret = SSL_get_error(ssl, n);

				if(ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE)
					DEBUG(printf("[D] SSL: write error (from buffer): want read/write\n"));
			}
		}

		return;
	}
#endif
	if(write.buf && !(status & STATUS_REDIR) && fd >= 0)
	{
		//FIXME: we should write more then 1 byte at a time
#ifdef HAVE_SSL
		if(status & STATUS_SSL)
		{
			if((n = SSL_write(ssl, write.buf + write.pos, 1)) == 1)
				++write.pos;
			else if(n < 0)
			{
				int ret = SSL_get_error(ssl, n);
				if(ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE)
				{
					DEBUG(printf("[D] SSL: write error (from buffer): want read/write\n"));
				}
			}
		}
		else
#endif
		if(::write(fd, write.buf + write.pos, 1) == 1)
			++write.pos;
				
		if(write.pos == write.len)
		{
			free(write.buf);
			write.buf=NULL;
			write.pos=0;
			write.len=0;
		}
	}
}

#ifdef HAVE_SSL
bool inetconn::enableSSL()
{
	if(status & STATUS_SSL)
		return false;

	status |= STATUS_SSL;
	ssl_ctx = SSL_CTX_new(SSLv23_method());
	if(!ssl_ctx)
		return false;
		
	DEBUG(printf("[D] ssl_ctx: %p\n", (void *) ssl_ctx));
	SSL_CTX_set_mode(ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
	ssl = SSL_new(ssl_ctx);
	if(!ssl_ctx)
		return false;

	DEBUG(printf("[D] net.irc.ssl: %p\n", (void *) ssl));
	SSL_set_fd(ssl, fd);

	return true;
}

int inetconn::isSSL()
{
	return status & STATUS_SSL;
}

void inetconn::SSLHandshake()
{
	if(status & STATUS_SSL_HANDSHAKING)
	{
		DEBUG(printf("[D] SSL: %s\n", SSL_state_string_long(ssl)));
		int ret;

		if(status & STATUS_SSL_WANT_CONNECT)
			ret = SSL_connect(ssl);
		else if(status & STATUS_SSL_WANT_ACCEPT)
			ret = SSL_accept(ssl);

		DEBUG(printf("[D] SSL connect: %d\n", ret));
		DEBUG(printf("[D] SSL cipher: %s\n", SSL_get_cipher_name(ssl)));
		DEBUG(printf("[D] SSL state string: %s\n", SSL_state_string_long(ssl)));

		switch(ret)
		{
			case 1:
				killTime = NOW + set.AUTH_TIMEOUT;
				status |= STATUS_CONNECTED;
				status &= ~(STATUS_SSL_HANDSHAKING | STATUS_SSL_WANT_ACCEPT | STATUS_SSL_WANT_CONNECT);
				DEBUG(printf("[D] SSL: CONNECTED ;-))))\n"));
				break;
			case 0:
				//ssl conn was closed
				DEBUG(printf("[D] SSL: Other end closed connection\n"));
				close("[D] SSL: handshake terminated");
				break;
			default:
				//check for non blocking specinfig errors;
				switch(SSL_get_error(ssl, ret))
				{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						//we will read/write later on...
						DEBUG(printf("[D] SSL: want someting\n"));
						break;
					default:
						//error
						DEBUG(printf("[D] SSL: something is wrong... ;/\n"));
						close("SSL handshake error");
				}
		}
	}
}

#endif

/** Sends a message to the hub.
 *
 * \author patrick <patrick@psotnic.com>
 * \param str the text which should be sent
 * \return 1 if message was sent, 0 if it failed.
 */

int inet::sendHub(const char *str, ...)
{
  int i;
  char buffer[MAX_LEN];
  va_list list;

  va_start(list, str);
  vsnprintf(buffer, MAX_LEN, str, list);
  va_end(list);

  if(net.hub.fd && net.hub.isMain()) {
    net.hub.send("%s", buffer);
    return 1;
  }

  else {
    for(i=0; i<net.max_conns; ++i) {
      if(net.conn[i].isMain() && net.conn[i].fd) {
        net.conn[i].send("%s", buffer);
        return 1;
      }
    }
  }

  return 0;
}

/**
 * \author patrick <patrick@psotnic.com>
 * \return current lag in ms or -1 if the bot is not connected or no lag check was performed
 */

int inetconn::Lagcheck::getLag()
{
  if(!(net.irc.status & STATUS_REGISTERED)) // FIXME: back pointer?
    return -1;
 
  if(!net.irc.lagcheck.sent.tv_usec) // FIXME: back pointer?
    return -1;

  if(inProgress)
    return (NOW - sent.tv_sec) * 1000;

  else
    return lag;
}
