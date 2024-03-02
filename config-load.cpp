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

void CONFIG::load(const char *file, bool decrypted)
{
	char arg[10][MAX_LEN], buf[MAX_LEN], cwd[MAX_LEN];
	int line = 0, n;
	int errors = 0;
	int alts = 0;
	int i;
	bool can_link_bots=false;
//	bool listening=false;
	bool listening_ssl=false;
	options::event *e;
	inetconn f;

	config.file = file;

	getcwd(cwd, MAX_LEN);

	if(decrypted)
		printf("[*] Loading decrypted config from '%s/%s'\n", cwd, file);
	else
		printf("[*] Loading config from '%s/%s'\n", cwd, file);

	if(f.open(file, O_RDWR) < 0)
	{
		printf("[-] Cannot open config file '%s/%s': %s\n", cwd, file, strerror(errno));
		exit(1);
	}

	if(!decrypted)
		f.enableLameCrypt();

	while(1)
	{
		n = f.readln(buf, MAX_LEN);
		if(n == -1) break;
		if(!n) continue;

		++line;

		str2words(arg[0], buf, 10, MAX_LEN);
		if(arg[0][0] == '#' || !*arg[0]) continue;

		e = setVariable(arg[0], rtrim(srewind(buf, 1)));
		if(!e->ok)
		{
			printf("[-] %s/%s:%d: %s\n", cwd, file, line, (const char *) e->reason);
			++errors;
		}
	}

	/* Basic checks */
	if(!nick.getLen())
	{
		printf("[-] Nick is not set\n");
		++errors;
	}

	if(errors)
	{
		plonk:
		printf("[-] Failed to load config\n");
		exit(1);
	}

        for(i=0; i<MAX_LISTENPORTS; i++)
        {
                if(config.listenport[i].isDefault())
                        continue;

                if(config.listenport[i].getHost().isSSL())
                        listening_ssl=true;

//                listening=true;

		if(!strcmp(config.listenport[i].getHandle(), "all") || !strcmp(config.listenport[i].getHandle(), "bots"))
			can_link_bots=true;
        }

	if(!config.hub.getPort())
	{
		printf("[*] Acting as MAIN\n");
		config.bottype = BOT_MAIN;
	}

	else if(can_link_bots)
	{
		printf("[*] Acting as SLAVE\n");
		config.bottype = BOT_SLAVE;
	}

	else
	{
		printf("[*] Acting as LEAF\n");
		config.bottype = BOT_LEAF;
	}

        if(config.bottype != BOT_LEAF)
        {
                if(alts)
                {
                        printf("[-] Alternating slaves can only be used for leafes\n");
                        goto plonk;
                }
        }
        else
        {
                if(!*config.hub.getHandle() && alts)
                {
                        printf("[-] Invalid hub's handle\n");
                        goto plonk;
                }
        }

	polish();

	if(imUp())
	{
		printf("[-] Signs on the sky tell me that i am already up\n");
		printf("[*] Terminating\n");
		exit(1337);
	}

	printf("[+] Config loaded\n");

#ifdef HAVE_SSL
	if(listening_ssl)
	{
		char tmp[MAX_LEN];
		char cwd[MAX_LEN];

		getcwd(cwd, MAX_LEN);

		printf("[*] Creating SSL context\n");
		
		if(!(inet::server_ctx = SSL_CTX_new(SSLv23_method())) || !SSL_CTX_set_mode(inet::server_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE))
		{
			printf("[-] Creation of SSL conext failed\n");
			exit(1);
		}

		snprintf(tmp, MAX_LEN, "server.key");
		printf("[*] Loading RSA private key from `%s/%s'\n", cwd, tmp);
		if(!SSL_CTX_use_RSAPrivateKey_file(inet::server_ctx, tmp, SSL_FILETYPE_PEM))
		{
			printf("[-] Error while loading key\n");
			exit(1);
		}
		
		snprintf(tmp, MAX_LEN, "server.crt");
		printf("[*] Loading server certificate from `%s/%s'\n", cwd, tmp);
		if(!SSL_CTX_use_certificate_file(inet::server_ctx, tmp, SSL_FILETYPE_PEM))
		{
			printf("[-] Error while loading cert\n");
			exit(1);
		}
	}
#endif

        for(i=0; i<MAX_LISTENPORTS; i++)
        {
                if(!config.listenport[i].isDefault())
                {
                        inet::listen_entry *e=new inet::listen_entry();

			e->access=0;

			if(!strcmp(config.listenport[i].getHandle(), "all"))
				e->access=LISTEN_ALL;
			else if(!strcmp(config.listenport[i].getHandle(), "users"))
				e->access=LISTEN_USERS;
			else if(!strcmp(config.listenport[i].getHandle(), "bots"))
				e->access=LISTEN_BOTS;

                        printf("[*] Opening listening socket at %s:%d\n", config.listenport[i].getHost().isIpv6() ? (const char *)config.listenport[i].getHost().ip6 : (const char *)config.listenport[i].getHost().ip4, (int) config.listenport[i].getPort());

                        if(config.listenport[i].isSSL())
                                e->use_ssl=true;

                        e->fd=startListening(config.listenport[i].getHost().isIpv6() ? (const char*)config.listenport[i].getHost().ip6 : (const char*)config.listenport[i].getHost().ip4, config.listenport[i].getPort());
                        net.listeners.add(e);

                        if(e->fd < 1)
                        {
                                printf("[-] Cannot open socket (%s)\n", strerror(errno));
                                exit(1);
                        }

                        printf("[+] Socket awaits incomming connections\n");
                }
        }
}
