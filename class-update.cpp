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

update::update()
{
	child_pid = child.fd = parent = 0;
}

bool update::forkAndGo(char *site, const char *idpw, bool silent)
{
	//already updating
	if(child.fd)
	{
		net.send(HAS_N, "[*] Already updating");
		net.send(HAS_B, "[*] To stop update: .bc %s stopupdate", (const char*) config.handle);
		return false;
	}

	//creating child -> parent pipe
	int tmp[2];

	//pipe faild
	if(pipe(tmp))
	{
		net.send(HAS_N, "[-] pipe() failed: %s", strerror(errno));
		end();
		return false;
	}
	child.fd = tmp[0];
	parent = tmp[1];

	pid = fork();

	switch(pid)
	{
		case -1:
			net.send(HAS_N, "[-] fork() failed: %s", strerror(errno));
			end();
			return false;

		case 0:
			if(!fcntl(parent, F_SETFL, O_NONBLOCK))
			{
				child_pid = getpid();
				//redirect
				//dup2(parent, fileno(stdout));
				//dup2(parent, fileno(stderr));
				if(doUpdate(site, idpw, silent))
				{
					sleep(1);
					kill(pid, SIGUSR1); //that should restart parent;
				}
				else
				{
					sleep(1);
					kill(pid, SIGUSR2); //that should notify parent that update has failed
				}
			}
			end();
			_exit(0);

		default:
			return true;
	}
}

void update::end()
{
	child.close();
	close(parent);
	if(child_pid > 0)
	    kill(child_pid, 9);

	child_pid = child.fd = parent = 0;
}

bool update::doUpdate(const char *site, const char *idpw, bool silent)
{
	int n;
	char buf[MAX_LEN], sendbuf[MAX_LEN];
	char updir[MAX_LEN], dir[MAX_LEN];

	http php, file;

	/* preparing */
	memset(buf, 0, sizeof(buf));
	memset(updir, 0, sizeof(updir));
	memset(dir, 0, sizeof(dir));
	php.sendInfo=true;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
        setvbuf(stdin, NULL, _IONBF, 0);

	http::url updateSite((site && *site) ? site : "http://updates.psotnic.com/");

	if(!silent)
	{
		snprintf(sendbuf, MAX_LEN, "[*] Connecting to http://%s\n", updateSite.host);
		write(parent, sendbuf, strlen(sendbuf));
	}

	//get link to the newest version
	if((n = php.get(updateSite)) > 0)
	{
		http::url link(php.data);

		//DEBUG(printf("[D] php.data = %s; n = %d\n", php.data, n));
		if(match("You already use the newest version", php.data))
		{
			if(!silent)
			{
				snprintf(sendbuf, MAX_LEN, "[-] I already use the newest version\n");
				write(parent, sendbuf, strlen(sendbuf));
			}

			return false;
		}

		if(link.ok() && match("*.tar.gz", link.file) && strlen(link.file) < MAX_LEN)
		{
			if(!silent)
			{
				snprintf(sendbuf, MAX_LEN, "[+] The newest version is: %s, downloading (this may take a while)\n", link.file);
				write(parent, sendbuf, strlen(sendbuf));
			}

			snprintf(updir, 256, ".update-%d", (int) getpid());

			if(mkdir(updir, 0700))
			{
				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[-] Cannot create update directory: %s\n", strerror(errno));
					write(parent, sendbuf, strlen(sendbuf));
				}

				return false;
			}

			if(chdir(updir))
			{
				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[-] Cannot change directory to %s: %s\n", updir, strerror(errno));
					write(parent, sendbuf, strlen(sendbuf));
				}

				return false;
			}

			if(idpw)
				file.setAuth(idpw);

			if((n = file.get(link, link.file)) > 0)
			{
				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[+] Unpacking\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

				int len = strlen(link.file) - 7;

				strncpy(dir, link.file, len);
				dir[len] = '\0';

				snprintf(buf, MAX_LEN, "tar -zxf %s && mv %s/* .", link.file, dir);
				if(system(buf))
					goto dupa;

				//printf("[+] Restoring seeds (creating seed.h)\n");
				FILE *f = fopen("seed.h", "w+");
				if(!f)
					goto dupa;

				unsigned char seed[16];

				fprintf(f, "#ifndef PSOTNIC_SEED_H\n");
				fprintf(f, "#define PSOTNIC_SEED_H 1\n");

				gen_cfg_seed(seed);
				fprintf(f, "static unsigned char cfg_seed[] = \"");
				for(int i=0; i<16; ++i)
					fprintf(f, "\\x%02x", seed[i]);
				fprintf(f, "\";\n");

				gen_ul_seed(seed);
				fprintf(f, "static unsigned char ul_seed[] = \"");
				for(int i=0; i<16; ++i)
					fprintf(f, "\\x%02x", seed[i]);
				fprintf(f, "\";\n");
				fprintf(f, "#endif\n");

				memset(seed, 0, 16);
				if(fclose(f))
					goto dupa;

				snprintf(buf, MAX_LEN, "./configure --prefix=%s", INSTALL_PREFIX);

#ifdef HAVE_ANTIPTRACE
				strcat(buf, " --with-antiptrace");
#endif

#ifdef HAVE_ADNS_FIREDNS
				strcat(buf, " --with-firedns");
#endif

#ifndef HAVE_ADNS
				strcat(buf, " --disable-adns");
#endif

#ifndef HAVE_MODULES
				strcat(buf, " --disable-modules");
#endif

				if(system(buf))
					goto dupa;

				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[*] Compiling (this may take a longer while)\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

#ifdef HAVE_DEBUG
				if(system("make debug"))
#else
				if(system("make"))
#endif
					goto dupa;

				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[*] Copying files\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

				if(system("make install_bin install_includes"))
					goto dupa;

				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[*] Cleaning up\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

				if(chdir("..") || rmdirext(updir))
				{
					if(!silent)
					{
						snprintf(sendbuf, MAX_LEN, "[-] Failed to remove %s: %s\n", updir, strerror(errno));
						write(parent, sendbuf, strlen(sendbuf));
					}

					return false;
				}

				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[+] Update successful\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

				return true;
			}
			else
			{
				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[-] Unable to download file: %s\n", file.error(n));
					write(parent, sendbuf, strlen(sendbuf));
				}

				dupa:
				if(!silent)
				{
					snprintf(sendbuf, MAX_LEN, "[-] Error occured during last operation: %s\n", strerror(errno));
					write(parent, sendbuf, strlen(sendbuf));
					snprintf(sendbuf, MAX_LEN, "[*] Cleaning up after a failure\n");
					write(parent, sendbuf, strlen(sendbuf));
				}

				if(chdir("..") || rmdirext(updir))
				{
					if(!silent)
					{
						snprintf(sendbuf, MAX_LEN, "[-] Failed to remove %s: %s\n", updir, strerror(errno));
						write(parent, sendbuf, strlen(sendbuf));
					}
				}

				return false;
			}
		}
		else
		{
			if(!silent)
			{
				snprintf(sendbuf, MAX_LEN, "[-] Invalid response (%s)\n", php.data);
				write(parent, sendbuf, strlen(sendbuf));
			}

			return false;
		}
	}
	else
	{
		if(!silent)
		{
			snprintf(sendbuf, MAX_LEN, "[-] Error occured: %s\n", php.error(n));
			write(parent, sendbuf, strlen(sendbuf));
		}

		return false;
	}
}
