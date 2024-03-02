/**************************************************************************
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
#include <string>

#define SUCCESS 1

int pl_adduser(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_deluser(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addhost(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delhost(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addbot(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delbot(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_mjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_rjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_sjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_mpart(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_rpart(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_spart(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addchan(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delchan(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_chattr(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_chpass(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_chaddr(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_chhandle(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_match(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_whois(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_set(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_gset(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_dset(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_chset(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_mcycle(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_rcycle(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_mk(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_exit(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_export(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_import(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_boot(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_rflags(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_list(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addban_exempt_invite_reop(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delban_exempt_invite_reop(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addinfo(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delinfo(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_info(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_bans(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_exempts(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_invites(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_reops(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_bots(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_verify(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_channels(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_offences(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_clearoffences(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_upbots(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_downbots(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_bottree(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_owners(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_users(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_save(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_me(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_echo(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_who_whom_whob(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_idiots(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addidiot(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_delidiot(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_unlink(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_botcmd(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_addaddr(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_deladdr(inetconn *c, char *data, char arg[][MAX_LEN], int argc);
int pl_help(inetconn *c, char *data, char arg[][MAX_LEN], int argc);

partyline_commands partyline_cmds[] =
{
    { "+user", pl_adduser, 1, "<handle> [host]", true },
    { "-user", pl_deluser, 1, "<handle>", true },
    { "+host", pl_addhost, 2, "<handle> <host>", true },
    { "-host", pl_delhost, 2, "<handle> <host>", true },
    { "+bot", pl_addbot, 2, "<handle> <ip>", true },
    { "-bot", pl_delbot, 1, "<handle>", true },
    { "mjoin", pl_mjoin, 1, "<chan> [key] [delay]", true },
    { "rjoin", pl_rjoin, 2, "<bot> <chan> [key]", true },
    { "sjoin", pl_sjoin, 2, "<slave> <chan> [key]", true },
    { "mpart", pl_mpart, 1, "<chan>", true },
    { "rpart", pl_rpart, 2, "<bot> <chan>", true },
    { "spart", pl_spart, 2, "<slave> <chan>", true },
    { "+chan", pl_addchan, 1, "<chan> [key]", true },
    { "-chan", pl_delchan, 1, "<chan>", true },
    { "chattr", pl_chattr, 2, "<handle> <flags> [chan]", true },
    { "chpass", pl_chpass, 2, "<handle> <pass>", true },
    { "chaddr", pl_chaddr, 2, "<handle> <ip>", true },
    { "chhandle", pl_chhandle, 2, "<handle> <new handle>", true },
    { "match", pl_match, 1, "<expr> [flags] [chan]", true },
    { "whois", pl_whois, 1, "<handle>", false },
    { "set", pl_set, 0, "[var] [value]", true },
    { "gset", pl_gset, 0, "[var] [value]", true },
    { "dset", pl_dset, 0, "[var] [value]", true },
    { "chset", pl_chset, 1, "<chan> [var] [value]", true },
    { "chanset", pl_chset, 1, "<chan> [var] [value]", true },
    { "mcycle", pl_mcycle, 1, "<chan>", true },
    { "rcycle", pl_rcycle, 2, "<bot> <chan>", true },
    { "mk", pl_mk, 2, "<o|n|a> <chan> [lock]", true },
    { "export", pl_export, 1, "<file> [pass]", true },
    { "import", pl_import, 1, "<file> [pass]", true },
    { "boot", pl_boot, 2, "<handle> <reason>", true },
    { "rflags", pl_rflags, 2, "<handle> <chan>", true },
    { "rchattr", pl_rflags, 2, "<handle> <chan>", true },
    { "list", pl_list, 1, "<apcdsvuiU> [bot]", true },
    { "+info", pl_addinfo, 3, "<handle> <key> <value>", true },
    { "-info", pl_delinfo, 2, "<handle> <key>", true },
    { "info", pl_info, 1, "<handle>", true },
    { "+ban", pl_addban_exempt_invite_reop, 1, "[chan] [%time] <mask> [reason]", true },
    { "+stick", pl_addban_exempt_invite_reop, 1, "[chan] [%time] <mask> [reason]", true },
    { "+exempt", pl_addban_exempt_invite_reop, 1, "[chan] [%time] <mask> [reason]", true },
    { "+invite", pl_addban_exempt_invite_reop, 1, "[chan] [%time] <mask> [reason]", true },
    { "+reop", pl_addban_exempt_invite_reop, 1, "[chan] [%time] <mask> [reason]", true },
    { "-ban", pl_delban_exempt_invite_reop, 1, "[chan] <mask>", true },
    { "-exempt", pl_delban_exempt_invite_reop, 1, "[chan] <mask>", true },
    { "-invite", pl_delban_exempt_invite_reop, 1, "[chan] <mask>", true },
    { "-reop", pl_delban_exempt_invite_reop, 1, "[chan] <mask>", true },
    { "bans", pl_bans, 0, "[chan]", true },
    { "exempts", pl_exempts, 0, "[chan]", true },
    { "invites", pl_invites, 0, "[chan]", true },
    { "reops", pl_reops, 0, "[chan]", true },
    { "bots", pl_bots, 0, "[expr] [flags]", true },
    { "verify", pl_verify, 0, "[a|p|h|c]", true },
    { "channels", pl_channels, 0, "[chan]", true },
    { "offences", pl_offences, 0, "[handle]", true },
    { "clearoffences", pl_clearoffences, 0, "[handle]", true },
    { "upbots", pl_upbots, 0, "", true },
    { "up", pl_upbots, 0, "", true },
    { "downbots", pl_downbots, 0, "", true },
    { "down", pl_downbots, 0, "", true },
    { "bottree", pl_bottree, 0, "", true },
    { "bt", pl_bottree, 0, "", true },
    { "owners", pl_owners, 0, "", true },
    { "users", pl_users, 0, "", true },
    { "save", pl_save, 0, "", true },
    { "idiots", pl_idiots, 0, "", true },
    { "+idiot", pl_addidiot, 1, "<host>", true },
    { "-idiot", pl_delidiot, 1, "<host>", true },
    { "unlink", pl_unlink, 1, "<bot>", true },
    { "+addr", pl_addaddr, 2, "<handle> <ip>", true },
    { "-addr", pl_deladdr, 2, "<handle> <ip>", true },
    { "exit", pl_exit, 0, "[reason]", false },
    { "quit", pl_exit, 0, "[reason]", false },
    { "who", pl_who_whom_whob, 0, "", false },
    { "whom", pl_who_whom_whob, 0, "", false },
    { "whob", pl_who_whom_whob, 0, "", false },
    { "me", pl_me, 1, "", false },
    { "bc", pl_botcmd, 2, "<bot> <command>", false },
    { "echo", pl_echo, 1, "", false },
    { "help", pl_help, 0, "", false },
    { NULL, NULL, 0, NULL, false }
};

char *find_alias(const char *cmd);

int parse_owner_join(inetconn *c, const char *name, const char *key, int delay, const char *flags, const char *bot=NULL)
{
    int n;

    if (c && !c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (!chan::valid(name))
    {
        if (c) c->send("Invalid channel name");
        return 0;
    }

    //.mjoin
    if (!bot)
    {
        bool sendDset = false;
        bool alreadyAdded = (userlist.findChannel(name)!=-1)?true:false;

        n = userlist.addChannel(name, key, flags);

        if (n < 0)
        {
            if (c)
                c->send("Channel list is full");
            return 0;
        }

        if (!alreadyAdded)
        {
            *userlist.chanlist[n].chset = *userlist.dset;
            sendDset = true;
        }

        if (strchr(flags, 'P'))
            net.send(HAS_N, "# %s # +chan %s %s %s", c->name, name, key, delay ? itoa(delay) : "");
        else if (strchr(flags, '*'))
            net.send(HAS_N, "# %s # mjoin %s %s %s", c->name, name, key, delay ? itoa(delay) : "");
        else
            net.send(HAS_N, "# %s # bugjoin %s %s %d", c->name, name, key, delay);

        if (c)
            c->send("Channel %s has been added to channel list", name);

        if (delay < 0)
        {
            net.send(HAS_B, "%s %s %s %s %s", S_ADDCHAN, flags, name, key, itoa(-delay));
            if (sendDset)
                userlist.sendDSetsToBots(name);
            ME.rejoin(name, -delay);
            if (c)
                c->send("Delaying mass join by %d seconds", -delay);
        }
        else if (delay > 0)
        {
            int j = delay;
            for (int i=0; i<net.max_conns; ++i)
            {
                if (net.conn[i].isRegBot())
                {
                    if (net.conn[i].name && *net.conn[i].name && userlist.isRjoined(n, net.conn[i].handle))
                    {
                        net.conn[i].send("%s %s %s %s %d", S_ADDCHAN, flags, name, key, j);
                        j += delay;
                    }
                    else
                        net.conn[i].send("%s %s %s %s ", S_ADDCHAN, flags, name, key);
                }
            }
            if (sendDset)
                userlist.sendDSetsToBots(name);
            if (c)
                c->send("Setting delay between bots joins to %d seconds", delay);

            ME.rejoin(name, 0);
        }
        else
        {
            net.send(HAS_B, "%s %s %s %s", S_ADDCHAN, flags, name, key);
            if (sendDset)
                userlist.sendDSetsToBots(name);
            ME.rejoin(name, 0);
        }

	userlist.updated();
        return 1;
    }
    // .rjoin
    else
    {
        if (!userlist.isBot(bot))
        {
            if (c)
                c->send("Invalid bot");
            return 0;
        }
        if (userlist.findChannel(name) == -1)
        {
            n = userlist.addChannel(name, key, flags);
            if (n < 0)
            {
                if (c) c->send("Channel list is full");
                return 0;
            }
            net.send(HAS_B, "%s %s %s %s", S_ADDCHAN, flags, name, key);
            *userlist.chanlist[n].chset = *userlist.dset;
            userlist.sendDSetsToBots(name);
        }

        if (c)
        {
            if (strchr(flags, 'P'))
                net.send(HAS_N, "# %s # rjoin %s %s %s", c->name, bot, name, key);
            //else if(strchr(flags, '*')) net.sendCmd(c, "mjoin %s %s %s", bot, name, key);
        }

        userlist.rjoin(bot, name);
        net.send(HAS_B, "%s %s %s", S_RJOIN, bot, name);
	userlist.updated();
        return 1;
    }
}

void parse_owner_sjoin(inetconn *c, const char *name, const char *key, int delay, const char *flags, const char *bot)
{
    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return;
    }
    if (!chan::valid(name))
    {
        c->send("Invalid channel name");
        return;
    }

    inetconn *slave = net.findConn(userlist.findHandle(bot));
    if (slave && slave->isSlave())
    {
        int j=0;
        if (parse_owner_join(NULL, name, key, delay < 0 ? delay : 0, flags, bot))
        {
            net.send(HAS_N, "# %s # sjoin %s %s %s", c->name, bot, name, key);

            for (int i=0; i<net.max_conns; ++i)
                if (net.conn[i].isLeaf() && net.conn[i].fd == slave->fd)
                    parse_owner_join(NULL, name, key, delay < 0 ? delay : j+=delay, flags, net.conn[i].handle->name);
        }
    }
    else c->send("Invalid slave");
}

int parse_owner_rpart(inetconn *c, char *name, char *bot, const char *flags="")
{
    if (c && !c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    switch (userlist.rpart(bot, name))
    {
    case -1:
    {
        if (c) c->send("Invalid bot");
        return 0;
    }
    case -2:
    {
        if (c) c->send("Invalid channel");
        return 0;
    }
    default:
        break;
    }

    if (c)
        net.send(HAS_N, "# %s # rpart %s %s", c->name, bot, name);

    net.send(HAS_B, "%s %s %s %s", S_RPART, bot, name, flags);
    userlist.updated();
    return 1;
}

void parse_owner_spart(inetconn *c, char *name, char *bot)
{
    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return;
    }
    inetconn *slave = net.findConn(userlist.findHandle(bot));
    if (slave && slave->isSlave())
    {
        if (parse_owner_rpart(c, name, bot))
        {
            net.send(HAS_N, "# %s # spart %s %s", c->name, bot, name);

            for (int i=0; i<net.max_conns; ++i)
                if (net.conn[i].isLeaf() && net.conn[i].fd == slave->fd)
                    parse_owner_rpart(NULL, name, net.conn[i].handle->name);
        }
    }
    else c->send("Invalid slave");
}

void parse_owner_mpart(inetconn *c, char *name)
{
    HANDLE *h;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return;
    }

    if (userlist.rpart(config.handle, name, "Q")!=-2)   // part main bot
    {
        net.send(HAS_N, "# %s # mpart %s", c->name, name);
        c->send("Parting all bots from %s", name);

        // part slaves, leafs

        h = userlist.first->next->next;

        while (h)
        {
            if (userlist.isBot(h))
                parse_owner_rpart(NULL, name, h->name, "Q");

            h = h->next;
        }
    }
    else
    {
        if (c)
            c->send("Invalid channel");
    }
}

void parse_owner(inetconn *c, char *data)
{
    char arg[10][MAX_LEN], buf[MAX_LEN], *reason = NULL;
    HANDLE *h;
    int argc, i, n;
    partyline_commands *pt;
    std::string str;

    if (!strlen(data)) return;
    memset(buf, 0, MAX_LEN);
    argc=str2words(arg[0], data, 10, MAX_LEN);

    /* NOT REGISTERED OWNER */

    if (!(c->status & STATUS_REGISTERED))
    {
        if (!(c->status & STATUS_TELNET))
        {
            switch (c->tmpint)
            {
            case 1:
            {
                if (config.checkOwnerpass(arg[0]))
                {
                    c->killTime = NOW + set.AUTH_TIMEOUT;
                    ++c->tmpint;
                    c->send("Enter your password.");
                    return;
                }
                else
                {
                    reason = push(NULL, c->tmpstr, ": bad ownerpass", NULL);
                    break;
                }
            }
            case 2:
            {
                if (strlen(arg[0]) && (h = userlist.matchPassToHandle(arg[0], c->tmpstr)))
                {
                    if (!(h->flags[GLOBAL] & HAS_P))
                    {
                        reason = push(NULL, c->tmpstr, ": no partyline privileges", NULL);
                        break;
                    }

                    c->status |= STATUS_REGISTERED;
                    c->tmpint = 0;
                    c->killTime = 0;
                    c->handle = h;
                    mem_strcpy(c->name, h->name);
                    sendLogo(c);
                    return;
                }
                else
                {
                    reason = push(NULL, c->tmpstr, ": bad userpass", NULL);
                    break;
                }
            }
            default:
                break;
            }
            free(c->tmpstr);
            c->tmpstr = NULL;
        }
        else
        {
            switch (c->tmpint)
            {
            case 1:
            {
                mem_strcpy(c->name, arg[0]);
                ++c->tmpint;

                c->send("");
                c->send("Enter your password.");

                return;
            }
            case 2:
            {
                if ((h = userlist.checkPartylinePass(c->name, arg[0], HAS_P)))
                {
                    if(!h->addr->match(c->getPeerIpName()))
                    {
                        reason = push(NULL, c->name, ": invalid ip", NULL);
                        break;
                    }

                    c->status |= STATUS_CONNECTED | STATUS_PARTY | STATUS_REGISTERED | STATUS_TELNET;
                    c->tmpint = 0;
                    c->killTime = 0;
                    c->handle = h;
                    if (c->tmpstr) free(c->tmpstr);
                    c->tmpstr = NULL;
                    //c->echo(1);
                    sendLogo(c);
                    ignore.removeHit(c->getPeerIp4());

                    if(!(c->status & STATUS_IRCCLIENT))
                    {
                      c->send("[!] Connecting by telnet is obsolete. Please connect with an IRC client.");
                      c->send("[!] e.g. /server%s %s %s <ownerpass>:%s:<your-password>", (c->status & STATUS_SSL) ? " -ssl" : "", c->getMyIpName(), c->getMyPortName(), c->name);
                    }

                    return;
                }
                else
                {
                    if ((h = userlist.findHandle(c->name)))
                    {
                        if (!(h->flags[GLOBAL] & HAS_P))
                            reason = push(NULL, c->name, ": no partyline privileges", NULL);
                        else
                            reason = push(NULL, c->name, ": wrong user password", NULL);
                    }
                    else
                        reason = push(NULL, c->name, ": invalid handle", NULL);

                    break;
                }
            }
            default:
                break;
            }
        }
        if (!reason)
            reason = push(NULL, "Unknown error", NULL);
        c->close(reason);
        free(reason);
        return;
    }

    /* REGISTERED OWNER */

    if((c->status & STATUS_IRCCLIENT))
    {
        if(!strcmp(arg[0], "PING"))
        {
            data[1]='O';
            c->sendRaw("%s", data);
            return;
        }

        else if(!strcmp(arg[0], "PRIVMSG"))
        {
            char *tmp;

            if(strcmp(arg[1], "partyline"))
                return;

            tmp=srewind(data, 2);

            if(tmp)
            {
                tmp++;
                strncpy(data, tmp, MAX_LEN);
                argc=str2words(arg[0], data, 10, MAX_LEN);
            }
        }

        else if(!strcmp(arg[0], "QUIT"))
        {
            char *a;

            a=srewind(data, 1);

            if(*a == ':')
                a++;

            c->close(a ? a : c->handle->name);
            return;
        }

	else
		return;
    }

    if (arg[0][0] == '.')
    {
        char *alias;

        if ((alias=find_alias(data+1)))
        {
            strncpy(data, alias, MAX_LEN);
            argc=str2words(arg[0], data, 10, MAX_LEN);
            free(alias);
        }

        HOOK(partylineCmd, partylineCmd(c->name, c->handle->flags[GLOBAL], arg[0], srewind(data, 1)));

        if (stopParsing)
        {
            stopParsing=false;
            return;
        }

        for (pt=partyline_cmds; pt->command != NULL; pt++)
        {
            if (pt->main_only && config.bottype != BOT_MAIN)
                continue;

            if (!strcmp(pt->command, arg[0]+1))
            {
                if (argc-1 < pt->min_args)
                    c->send("syntax: .%s %s", pt->command, pt->syntax);

                else
                {
                    i=pt->func(c, data, arg, argc-1);

                    if(i == SUCCESS)
                    {
                        strcpy(arg[0], arg[0]+1);

                        for(n=0; n < 10; n++)
                        {
                            if(*arg[n])
                            {
                                if(n > 0)
                                    str+=" ";

                                str+=arg[n];
                            }

                            else
                                break;
                        }

                        net.send(HAS_N, "# %s # %s", c->name, str.c_str());	
                    }
                }

                return;
            }
        } // for

        c->send("What? You need .help?");
        return;
    }

    for (i=0; i<net.max_conns; ++i)
    {
        if (net.conn[i].fd && net.conn[i].isRegUser()
                && ((c->status & STATUS_NOECHO) ? c != &net.conn[i] : 1))
            net.conn[i].send("<%s> %s", c->name, data);
    }

    //for sake of compiler warnings
    n = 0;
}

/** finds an alias and returns the real command.
 *
 * \author patrick <patrick@psotnic.com>
 * \param cmd a command, the first word should be an alias (not starting with '.')
 * \return the real command
 */

char *find_alias(const char *cmd)
{
    char alias_name[MAX_LEN], cmd_argv[10][MAX_LEN], *p=NULL, *end;
    int idx, cmdarg_idx;
    string str;

    cmdarg_idx=str2words(cmd_argv[0], cmd, 10, MAX_LEN, 0);

    // look if the command is an alias
    for (idx=0; idx < MAX_ALIASES; idx++)
    {
        str2words(alias_name, config.alias[idx], 1, MAX_LEN, 0);

        if (!strcasecmp(alias_name, cmd_argv[0]))
        {
            p=srewind(config.alias[idx], 1);
            break;
        }
    }

    if (!p)
        return NULL;

    str=".";

    while (*p)
    {
        /* replace $n by the n'th word of the command
         * and $n- by the n'th and all following words.
         */

        if (*p=='$')
        {
            *p++;
            idx=strtoul(p, &end, 10);
            idx++;

            if (*end=='-')
            {
                if (idx < cmdarg_idx)
                    str+=srewind(cmd, idx);

                *end++;
            }

            else if (idx < cmdarg_idx)
                str+=cmd_argv[idx];

            p=end;
        }

        else
        {
            str+=*p+std::string();
            *p++;
        }
    }

    p=NULL;

    if (str.length() > 0)
    {
        p=(char *) malloc(str.length()+1);
        strcpy(p, str.c_str());
    }

    return p;
}

int pl_adduser(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    char buf[MAX_LEN];
    int i;

    if (!c->checkFlag(HAS_N))
    {
        bool noperm=true;

        for(i=0; i<MAX_CHANNELS; i++)
            if(c->checkFlag(HAS_N, i))
                noperm=false;

        if(noperm)
        {
            c->send("%s", S_NOPERM);
            return 0;
        }
    }
    if (!isRealStr(arg[1]))
    {
        c->send("Invalid handle");
        return 0;
    }
    if (strlen(arg[1]) > MAX_HANDLE_LEN)
    {
        c->send("Too long handle name");
        return 0;
    }
    if (strlen(arg[2]))
    {
        if (!extendhost(arg[2], buf, MAX_LEN))
        {
            c->send("Invalid hostname");
            return 0;
        }

        if ((h = userlist.addHandle(arg[1], 0, 0, 0, 0, c->name)))
        {
            userlist.addHost(h, buf, c->name, NOW);
            c->send("Adding user %s with host %s", arg[1], buf);
            net.send(HAS_B, "%s %s %s", S_ADDUSER, arg[1], h->creation->print());
            net.send(HAS_B, "%s %s %s", S_ADDHOST, arg[1], buf);
            //ME.recheckFlags();
            userlist.SN++;
            userlist.updated();
            return SUCCESS;
        }
        else c->send("Handle exists");
    }
    else
    {
        if ((h = userlist.addHandle(arg[1], 0, 0, 0, 0, c->name)))
        {
            c->send("Adding user %s", arg[1]);
            net.send(HAS_B, "%s %s %s", S_ADDUSER, arg[1], h->creation->print());
            userlist.updated();
            return SUCCESS;
        }
        else c->send("Handle exists");
    }

    return 0;
}

int pl_deluser(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    h = userlist.findHandle(arg[1]);

    if (!h || userlist.isBot(h))
    {
        c->send("Invalid handle");
        return 0;
    }

    switch (userlist.hasWriteAccess(c, arg[1]))
    {
    case 0:
    {
        c->send("%s", S_NOPERM);
        break;
    }
    case 1:
    {
        if (userlist.removeHandle(arg[1]) == -1)
        {
            c->send("This handle is immortal");
            return 0;
        }
        c->send("Removing user %s", arg[1]);
        net.send(HAS_B, "%s %s", S_RMUSER, arg[1]);
        //ME.recheckFlags();
        userlist.updated();
        return SUCCESS;
    }
    }
    return 0;
}

int pl_addhost(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    char    buf[MAX_LEN];

    if (!extendhost(arg[2], buf, MAX_LEN))
    {
        c->send("Invalid hostname");
        return 0;
    }
    h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (userlist.hasWriteAccess(c, arg[1]))
        {
            if (userlist.addHost(h, buf, c->name, NOW) != -1)
            {
                c->send("Adding host %s to handle %s", buf, arg[1]);
                net.send(HAS_B, "%s %s %s", S_ADDHOST, arg[1], buf);
                //ME.recheckFlags();
		userlist.updated();
                return SUCCESS;
            }
            else c->send("Host exists");
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_delhost(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    int n;

    h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (userlist.hasWriteAccess(c, arg[1]))
        {
            if ((n = userlist.findHost(h, arg[2])) != -1)
            {
                c->send("Removing host %s from handle %s", h->host[n], h->name);
                net.send(HAS_B, "%s %s %s", S_RMHOST,  h->name, h->host[n]);
                userlist.removeHost(h, arg[2]);
		userlist.updated();
                return SUCCESS;
            }
            else c->send("Invalid host");
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_addbot(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (!isRealStr(arg[1]))
    {
        c->send("Invalid handle");
        return 0;
    }

    if (strlen(arg[1]) > MAX_HANDLE_LEN)
    {
        c->send("Too long handle name");
        return 0;
    }

    if (!isValidIp(arg[2]))
    {
        c->send("Invalid IPv4 address");
        return 0;
    }

    if ((h = userlist.addHandle(arg[1], inet_addr("1.1.1.1"), B_FLAGS, 0, 0, c->name)))
    {
        c->send("Adding new bot %s", arg[1]);
	h->addr->add(arg[2]);
        net.send(HAS_B, "%s %s %s %s", S_ADDBOT, arg[1], h->creation->print(), "1.1.1.1");
        net.send(HAS_B, "%s %s %s", S_ADDADDR, arg[1], arg[2]);
        ++userlist.SN;
        c->send("Adding addr %s to handle %s", arg[2], arg[1]);
        //ME.recheckFlags();
	userlist.updated();
        return SUCCESS;
    }
    else c->send("Handle exits");
    return 0;
}

int pl_delbot(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int n;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    if ((n = userlist.removeHandle(arg[1])) == 1)
    {
        c->send("Removing handle %s", arg[1]);
        net.send(HAS_B, "%s %s", S_RMUSER, arg[1]);
        //ME.recheckFlags();
        userlist.updated();
        return SUCCESS;
    }
    else if (n == -1)
    {
        c->send("This handle is immortal");
        return 0;
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_mjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_join(c, arg[1], arg[2], atoi(arg[3]), "*");
    return 0;
}

int pl_rjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_join(c, arg[2], arg[3], 0, "P", arg[1]);
    return 0;
}

int pl_sjoin(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_sjoin(c, arg[2], arg[3], 0, "P", arg[1]);
    return 0;
}

int pl_mpart(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_mpart(c, arg[1]);
    return 0;
}


int pl_rpart(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_rpart(c, arg[2], arg[1]);
    return 0;
}

int pl_spart(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_spart(c, arg[2], arg[1]);
    return 0;
}

int pl_addchan(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    parse_owner_join(c, arg[1], arg[2], 0, "P");
    return 0;
}

int pl_delchan(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    if (userlist.removeChannel(arg[1], arg[2]))
    {
        c->send("Removing %s from channel list", arg[2]);
        net.send(HAS_B, "%s %s", S_RMCHAN, arg[2]);
        net.irc.send("PART %s :%s", arg[2], (const char *) set.PARTREASON);
        userlist.updated();
        return SUCCESS;
    }
    else c->send("Invalid channel");
    return 0;
}

int pl_chattr(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int n;
    char buf[MAX_LEN];

    n = userlist.changeFlags(arg[1], arg[2], arg[3], c);
    switch (n)
    {
    case -1:
        c->send("Invalid handle");
        return 0;

    case -2:
        c->send("Invalid channel");
        return 0;

    case -3:
        c->send("%s", S_NOPERM);
        return 0;

    case -4:
        c->send("Invalid channel flags");
        return 0;

    case -5:
        c->send("Invalid flags");
        return 0;

    case -6:
        c->send("Invalid global flags");
        return 0;

    case -7:
        c->send("Flags conflict");
        return 0;

    default:
        break;
    }
    userlist.flags2str(n, buf);

    net.send(HAS_N, "# %s # chattr %s %s (now: %s) %s", c->name, arg[1], arg[2], buf, arg[3]);

    if (strlen(arg[3]))
    {
        c->send("Changing %s flags for %s to `%s'", arg[3], arg[1], buf);
        net.send(HAS_B, "%s %s %s %s", S_CHATTR, arg[1], arg[2], arg[3]);
        //ME.recheckFlags(arg[3]);
    }
    else
    {
        if (!userlist.isBot(arg[1]))
        {
            c->send("Changing global flags for %s to `%s'", arg[1], buf);
            if (!(n & HAS_P))
            {
                inetconn *u;
                while ((u = net.findConn(arg[1])))
                    u->close("Lost partyline privileges");
            }
        }
        else c->send("Changing botnet flags for %s to `%s'", arg[1], buf);

        net.send(HAS_B, "%s %s %s", S_CHATTR, arg[1], arg[2]);


        //ME.recheckFlags();
    }
    userlist.updated();
    ME.nextRecheck = NOW + SAVEDELAY;
    return 0;
}

int pl_chpass(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h = userlist.findHandle(arg[1]);
    char buf[MAX_LEN];

    if (!h)
        c->send("Invalid handle");

    else if (userlist.hasWriteAccess(c, h))
    {
        if (strlen(arg[2]) < 8)
        {
            c->send("Password must be at least 8 charactes long");
            return 0;
        }
        if (!(h = userlist.changePass(arg[1], arg[2]))) c->send("Invalid handle");
        else
        {
            snprintf(arg[2], MAX_LEN, "[something]");
            c->send("Changing password for %s", arg[1]);
            net.send(HAS_B, "%s %s %s", S_PASSWD, arg[1], quoteHexStr(h->pass, buf));
            userlist.updated();
            return SUCCESS;
        }
    }
    else c->send("%s", S_NOPERM);
    return 0;
}

int pl_chaddr(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    if (userlist.hasWriteAccess(c, arg[1]))
    {
        if (!(h = userlist.changeIp(arg[1], arg[2])))
            c->send("Invalid handle or ip");

        else
        {
            c->send("Changing ip address of %s", arg[1]);
            net.send(HAS_B, "%s %s %s", S_ADDR, arg[1], arg[2]);
            snprintf(arg[2], MAX_LEN, "[something]");
            userlist.updated();
            c->send("\002.chaddr is obsolete and will be removed soon. Please use .+/-addr instead!\002");
            return SUCCESS;
        }
    }

    else c->send("%s", S_NOPERM);
    return 0;
}

int pl_chhandle(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    HANDLE *h = userlist.findHandle(arg[1]);
    if (!h)
        return 0;
    if (userlist.hasWriteAccess(c, h))
    {
        if (h == userlist.first)
        {
            c->send("Let those idiots alone ;-)");
            return 0;
        }
        if (userlist.isBot(h))
        {
            c->send("Cannot change bot's handle name");
            return 0;
        }
        if (userlist.findHandle(arg[2]))
        {
            c->send("Destination handle exists");
            return 0;
        }
        if (!isRealStr(arg[2]))
        {
            c->send("Invalid destination handle");
            return 0;
        }

        int changed = 0;
        for (int i=0; i<net.max_conns; ++i)
        {
            if (net.conn[i].name && !strcmp(net.conn[i].name, arg[1]))
            {
                free(net.conn[i].name);
                mem_strcpy(net.conn[i].name, arg[2]);
                changed = 1;
            }
        }
        if (changed)
            net.send(HAS_N, "%s is now known as %s", arg[1], arg[2]);

        free(h->name);
        mem_strcpy(h->name, arg[2]);
        userlist.updated();
        net.send(HAS_B, "%s %s %s", S_CHHANDLE, arg[1], arg[2]);
        return SUCCESS;
    }
    else c->send("%s", S_NOPERM);
    return 0;
}

int pl_match(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h = userlist.first;;
    int i = 0;
    int flags = userlist.str2userFlags(arg[2]);
    int num = GLOBAL;
    bool req = false;

    if (*arg[3])
        num = userlist.findChannel(arg[3]);
    if (num == -1)
    {
        c->send("Invalid channel");
        return 0;
    }

    if (!strcmp(arg[2], "*"))
        *arg[3] = '\0';

    if (!strcmp(arg[2], "-"))
        req = true;

    while (h)
    {
        if (userlist.isBot(h) || !userlist.hasReadAccess(c, h))
        {
            h = h->next;
            continue;
        }

        if (*arg[2] && !flags)
        {
            if (h->flags[num])
            {
                h = h->next;
                continue;
            }
        }
        else if ((h->flags[num] & flags) != flags)
        {
            h = h->next;
            continue;
        }
        if (match(arg[1], h->name) || userlist.wildFindHostExt(h, arg[1]) != -1)
        {
            if (i < set.MAX_MATCHES || !set.MAX_MATCHES)
            {
                if (i) c->send("---");
                userlist.sendHandleInfo(c, h, arg[1]);
            }
            ++i;
        }
        h = h->next;
    }
    if (i >= set.MAX_MATCHES && set.MAX_MATCHES)
        c->send("(more than %d matches, list truncated)", (int) set.MAX_MATCHES);

    if (!i) c->send("No matches has been found");
    else c->send("--- Found %d match%s for '%s'", i, i == 1 ? "" : "es", arg[1]);
    return SUCCESS;
}

int pl_whois(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (userlist.hasReadAccess(c, h))
        {
            userlist.sendHandleInfo(c, h, arg[2]);
            return SUCCESS;
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_set(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    a = srewind(data, 2);

    if (set.parseUser(c->name, arg[1], a, "set"))
    {
        net.send(HAS_B, "%s %s %s", S_SET, arg[1], a ? a : "");
        userlist.updated();
        return 0;
    }
    return 0;

}

int pl_gset(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int i;
    char *a;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    a = srewind(data, 2);
    i = -1;

    if (userlist.globalChset(c, arg[1], a, &i))
    {
        // COMPATIBILITY REASONS
        net.send(HAS_B, "%s %s %s", S_GCHSET, arg[1], userlist.chanlist[i].chset->getValue(arg[1]));
        userlist.updated();
        return 0;
    }
    return 0;

}

int pl_chset(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int i;
    char *a;

    i = userlist.findChannel(arg[1]);

    if (i != -1)
    {
        if(!c->checkFlag(HAS_N) && !c->checkFlag(HAS_N, i))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }

        a = srewind(data, 3);

        if (userlist.chanlist[i].chset->parseUser(c->name, arg[2], a ? a : "", arg[1], "chset"))
        {
            net.send(HAS_B, "%s %s %s %s", S_CHSET, arg[1], arg[2], userlist.chanlist[i].chset->getValue(arg[2]));
            userlist.updated();
            return 0;
        }
    }
    else c->send("Invalid channel");
    return 0;

}

int pl_mcycle(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    if (userlist.findChannel(arg[1]) != -1)
    {
        if (ME.findChannel(arg[1]))
        {
            net.irc.send("PART %s :%s", arg[1], !set.CYCLEREASON.isDefault() ? (const char *) set.CYCLEREASON : (const char *) set.PARTREASON);
            ME.rejoin(arg[1], set.CYCLE_DELAY);
        }
        net.send(HAS_B, "%s %s", S_CYCLE, arg[1]);
        c->send("Doing mass cycle on %s", arg[1]);
        return SUCCESS;
    }
    else c->send("Invalid channel");
    return 0;

}

int pl_rcycle(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    if (userlist.findChannel(arg[2]) != -1)
    {
        if (!strcmp(arg[1], config.handle))
        {
            net.irc.send("PART %s :%s", arg[2], !set.CYCLEREASON.isDefault() ? (const char *) set.CYCLEREASON : (const char *) set.PARTREASON);
            ME.rejoin(arg[1], set.CYCLE_DELAY);
            net.send(HAS_N, "Doing cycle on %s", arg[2]);
        }
        else
        {
            inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
            if (bot && bot->isRegBot()) bot->send("%s %s %s", S_CYCLE, arg[2], S_FOO);
            else c->send("Invalid bot");
        }
        return SUCCESS;
    }
    else c->send("Invalid channel");
    return 0;
}

int pl_mk(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int i;

    if ((i=userlist.findChannel(arg[2])) == -1)
    {
        c->send("Invalid channel");
        return 0;
    }

    if(!c->checkFlag(HAS_N) && !c->checkFlag(HAS_N, i))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    int lock = !strcmp(arg[3], "close") || !strcmp(arg[3], "lock");

    chan *ch = ME.findChannel(arg[2]);

    if (!strcmp(arg[1], "a") || !strcmp(arg[1], "all"))
    {
        if (ch)
            ch->massKick(MK_ALL, lock);

        net.propagate(NULL, "%s %s %s", S_MKA, arg[2], arg[3]);
        c->send("Doing mass kick all on %s", arg[2]);
        return SUCCESS;
    }

    else if (!strcmp(arg[1], "o") || !strcmp(arg[1], "ops"))
    {
        if (ch)
            ch->massKick(MK_OPS, lock);

        net.propagate(NULL, "%s %s", S_MKO, arg[2]);
        c->send("Doing mass kick ops on %s", arg[2]);
        return SUCCESS;
    }

    else if (!strcmp(arg[1], "n") || !strcmp(arg[1], "nonops") || !strcmp(arg[1], "lames"))
    {
        if (ch)
            ch->massKick(MK_NONOPS, lock);

        net.propagate(NULL, "%s %s", S_MKN, arg[2]);
        c->send("Doing mass kick nonops on %s", arg[2]);
        return SUCCESS;
    }

    else
        c->send("No such user class");

    return 0;
}

int pl_exit(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;

    a = srewind(data, 1);
    c->close(a ? a : c->handle->name);
    return 0;

}

int pl_export(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int n;

    if (c->checkFlag(HAS_X))
    {
        if (strlen(arg[2]))
        {
            n = userlist.save(arg[1], 1, arg[2]);
            snprintf(arg[2], MAX_LEN, "[something]");
        }

        else
            n = userlist.save(arg[1], 0);

        if (n)
        {
            c->send("[+] Exported");
            return SUCCESS;
        }
    }

    else
        c->send("%s", S_NOPERM);

    return 0;
}

int pl_import(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int n;

    if (c->checkFlag(HAS_X))
    {
        userlist.reset();
        userlist.addHandle("idiots", 0, 0, 0, 0, config.handle);
        userlist.addHandle(config.handle, 0, B_FLAGS | HAS_H, 0, 0, config.handle);
        userlist.first->flags[MAX_CHANNELS] = HAS_D;

        if (strlen(arg[2]))
        {
            net.send(HAS_N, "# %s # import %s [something]", c->name, arg[1]);
            n = userlist.load(arg[1], 1, arg[2]);
        }

        else
        {
            net.send(HAS_N, "# %s # import %s [something]", c->name, arg[1]);
            n = userlist.load(arg[1], 0);
        }

        if (n == 1)
        {
            net.send(HAS_N, "Userfile imported");
            ME.quit();
            userlist.save(config.userlist_file);
        }

        else if (n == -1)
        {
            net.send(HAS_N, "Broken userlist");
            ME.quit();
            exit(1);
        }

        else if (n == 0)
        {
            net.send(HAS_N, "Cannot import: %s", strerror(errno));
            ME.quit();
            exit(1);
        }
    }
    return 0;

}

int pl_boot(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a, buf[MAX_LEN];
    HANDLE *h;

    h = userlist.findHandle(arg[1]);

    if (h)
    {
        inetconn *o = net.findConn(h);

        if (o && o->isReg() && o->checkFlag(HAS_P))
        {
            if ((o->checkFlag(HAS_X) && !c->checkFlag(HAS_X)) ||
                    (o->checkFlag(HAS_S) && !c->checkFlag(HAS_S)) ||
                    (o->checkFlag(HAS_N) && !c->checkFlag(HAS_N)))
            {
                c->send("%s", S_NOPERM);
                return 0;
            }

            a = srewind(data, 2);

            if (a)
            {
                snprintf(buf, MAX_LEN, "(Booted by %s) %s", c->name, a);
                o->close(buf);
            }

            else
                o->close("Booted");

            return SUCCESS;
        }
        else c->send("Invalid owner");
    }
    else c->send("Invalid owner");
    return 0;
}

int pl_rflags(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    /* FIXME: Add support for restoring global flags */
    HANDLE *h;
    int n;

    h = userlist.findHandle(arg[1]);
    if (h && h != userlist.first)
    {
        if (!userlist.hasReadAccess(c, h))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }
        if (!h->history || !h->history->data.entries())
        {
            c->send("No offence history found; flags not restored");
            return 0;
        }

        if (strlen(arg[2]))
        {
            n = userlist.findChannel(arg[2]);
            if (n == -1)
            {
                c->send("Channel not found; flags not restored");
                return 0;
            }

            ptrlist<offence::entry>::iterator o = h->history->data.begin();

            while (o)
            {
                if (!strcmp(o->chan, arg[2]))
                    break;
                o++;
            }

            if (!o)
            {
                c->send("No offence history found for %s; flags not restored", arg[2]);
                return 0;
            }

            if ((unsigned) h->flags[n] == (unsigned) o->fromFlags)
            {
                c->send("Flags are already the same; not changed");
                return 0;
            }

            char flags1[32], flags2[32];

            userlist.flags2str(o->fromFlags, flags1);
            userlist.flags2str(h->flags[n], flags2);

            h->flags[n] = o->fromFlags;

            c->send("Restoring %s flags for %s to `%s'", arg[1], arg[2], flags1);
            net.send(HAS_B, "%s %s -%s+%s %s", S_CHATTR, arg[1], flags2, flags1, arg[2]);
            userlist.updated();
            return SUCCESS;
        }
    }

    return 0;
}

int pl_list(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (strchr("apsvdcUui", arg[1][0]) && !arg[1][1])
    {
        if (strlen(arg[2]))
        {
            if (!strcmp(arg[2], config.handle))
                listcmd(arg[1][0], c->name);

            else
            {
                inetconn *bot = net.findConn(userlist.findHandle(arg[2]));
                if (bot)
                {
                    if (arg[1][0]=='p')
                        bot->send("%s %s %s %s %s", S_LIST, arg[1], c->name, itoa(NOW), itoa(nanotime()));
                    else
                        bot->send("%s %s %s", S_LIST, arg[1], c->name);
                }

                else
                    c->send("Invalid bot");
            }
        }

        else if (listcmd(arg[1][0], c->name) == 1)
            net.propagate(NULL, "%s %s %s", S_LIST, arg[1], c->name);

        return SUCCESS;
    }

    else
        c->send("Invalid option");

    return 0;

}

int pl_delban_exempt_invite_reop(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    protmodelist *protlist;
    protmodelist::entry *s;
    chan *ch;
    int arg_idx=1, type = -1;
    const char *botnet_cmd = NULL;
    char mode[3];
    bool validChannel=chan::valid(arg[1]);

    if (!strcmp(arg[0], ".-ban"))
    {
        type = BAN;
        botnet_cmd = S_RMBAN;
    }

    else if (!strcmp(arg[0], ".-invite"))
    {
        type = INVITE;
        botnet_cmd = S_RMINVITE;
    }

    else if (!strcmp(arg[0], ".-exempt"))
    {
        type = EXEMPT;
        botnet_cmd = S_RMEXEMPT;
    }

    else if (!strcmp(arg[0], ".-reop"))
    {
        type = REOP;
        botnet_cmd = S_RMREOP;
    }

         // ".-ban #1" should remove the first global ban
    if (!(!strlen(arg[2]) && arg[1][0] == '#' && isdigit(arg[1][1])) && validChannel)
    {
        int i = userlist.findChannel(arg[1]);

        if (i != -1)
        {
            protlist = userlist.chanlist[i].protlist[type];

            if (!(c->handle->flags[i] & HAS_N) && !c->checkFlag(HAS_N))
            {
                c->send("%s", S_NOPERM);
                return 0;
            }
        }

        else
        {
            c->send("Invalid channel");
            return 0;
        }

        arg_idx++;
    }

    else
    {
        if (!(c->checkFlag(HAS_N)))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }
        protlist = userlist.protlist[type];
    }

    if ((s=protlist->find(arg[arg_idx])))
    {
        mode[0] = '-';
        mode[1] = protlist->mode;
        mode[2] = '\0';

        if (validChannel)
        {
            if ((ch=ME.findChannel(arg[1])))
                if (ch->synced() && ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
                    ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
        }

        else
            foreachSyncedChannel(ch)
            if (ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
                ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
    }

    if (protlist->remove(arg[arg_idx]))
    {
        c->send("%s has been removed", arg[0]+2);

        if (validChannel)
            net.send(HAS_B, "%s %s %s", botnet_cmd, arg[arg_idx], arg[1]);
        else
            net.send(HAS_B, "%s %s", botnet_cmd, arg[arg_idx]);

        userlist.updated();
        return SUCCESS;
    }

    else
        c->send("Invalid mask");

    return 0;
}

int pl_addinfo(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (userlist.hasWriteAccess(c, arg[1]))
        {
            if (h->info && h->info->data.entries() > 5)
            {
                c->send("Too many entries");
                return 0;
            }

            char *a;
            a = srewind(data, 3);

            if (!h->info)
                h->info = new comment;

            if (h->info->add(arg[2], a))
            {
                c->send("Info updated");
                userlist.nextSave = NOW + SAVEDELAY;
                return SUCCESS;
            }
            else c->send("Invalid data format");

        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_delinfo(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h = userlist.findHandle(arg[1]);

    if (h)
    {
        if (userlist.hasWriteAccess(c, arg[1]))
        {
            if (!h->info || !h->info->del(arg[2]))
                c->send("Invalid key");

            else
            {
                c->send("Info removed");
                userlist.nextSave = NOW + SAVEDELAY;
                return SUCCESS;
            }
        }

        else
            c->send("%s", S_NOPERM);
    }

    else
        c->send("Invalid handle");
    return 0;
}

int pl_info(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;
    HANDLE *h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (!userlist.hasReadAccess(c, h))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }
        if (!h->info || !h->info->data.entries())
        {
            c->send("No info available");
            return 0;
        }

        ptrlist<comment::entry>::iterator e = h->info->data.begin();
        a = NULL;

        while (e)
        {
            a = push(a, a ? (char *) ", " : (char *) " ", e->key, ": ",
                     e->value, NULL);
            e++;
        }

        c->send("%s's info: %s", h->name, a);
        free(a);
        return SUCCESS;
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_addban_exempt_invite_reop(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char buf[MAX_LEN];
    int stick=!strcmp(arg[0], ".+stick") || !strcmp(arg[0], ".+invite") || !strcmp(arg[0], ".+exempt") || !strcmp(arg[0], ".+reop");
    char *channel = NULL, *reason = NULL;
    char *expires = NULL;
    char *mask = NULL;
    int expTime = 0;
    int i;
    int chanNum = GLOBAL;
    int type;
    const char *botnet_cmd;
    protmodelist *shit;
    protmodelist::entry *s;
    chan *ch;

    if (!strcmp(arg[0], ".+stick"))
    {
        type = BAN;
        botnet_cmd = S_ADDSTICK;
    }

    if (!strcmp(arg[0], ".+ban"))
    {
        type = BAN;
        botnet_cmd = S_ADDBAN;
    }

    else if (!strcmp(arg[0], ".+invite"))
    {
        type = INVITE;
        botnet_cmd = S_ADDINVITE;
    }

    else if (!strcmp(arg[0], ".+exempt"))
    {
        type = EXEMPT;
        botnet_cmd = S_ADDEXEMPT;
    }

    else if (!strcmp(arg[0], ".+reop"))
    {
        type = REOP;
        botnet_cmd = S_ADDREOP;
    }

    shit = userlist.protlist[type];

    for (i=1; i<4 && *arg[i]; ++i)
    {
        //channel
        if (chan::valid(arg[i]))
        {
            if (channel)
            {
                c->send("Too many channels given");
                return 0;
            }
            else if (arg[i][1] && (chanNum = userlist.findChannel(arg[i])) != -1)
            {
                channel = arg[i];
                shit = userlist.chanlist[chanNum].protlist[type];
            }
            else
            {
                c->send("Invalid channel");
                return 0;
            }
        }

        //expiration time
        else if (*arg[i] == '%')
        {
            if (expires)
            {
                c->send("Too many expiration dates were given");
                return 0;
            }
            else if (arg[i][1] && units2int(arg[i]+1, ut_time, expTime) == 1 && expTime > 0)
            {
                expires = arg[i]+1;
            }
            else
            {
                c->send("Invalid expiration time");
                return 0;
            }
        }
        //mask
        else
        {
            if (extendhost(arg[i], buf, MAX_LEN))
                mask = buf;
            break;
        }
    }

    if (!mask || !strcmp(mask, "*!*@*"))
    {
        c->send("Invalid mask");
        return 0;
    }

    char *ex;
    if ((ex = strchr(mask, '!')) - mask > 15)
    {
        c->send("Invalid mask: more than 15 chars before `!'");
        return 0;
    }

    if (strchr(mask, '@') - ex > 11)
    {
        c->send("Invalid mask: more than 10 chars between `!' and `@'");
        return 0;
    }

    /* look for conflicts */
    if ((s = shit->conflicts(mask)))
    {
        c->send("Mask conflicts with %s", s->mask);
        return 0;
    }

    if (!c->checkFlag(HAS_N) && !c->checkFlag(HAS_N, chanNum))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (expTime)
        expTime += NOW;

    reason = srewind(data, i+1);
    s = shit->add(mask, c->handle->name, NOW, expTime, reason, stick);

    net.send(HAS_B, "%s %s %s %s %s %d %s", (char *) botnet_cmd, channel ? channel : "*", 
             s->mask, s->by, " 0 ", s->expires, s->reason);

    userlist.updated();

    if (channel)
    {
        c->send("Added %s %s on %s", arg[0]+2, mask, channel);
        if (type == BAN && (ch = ME.findChannel(channel)))
            ch->applyBan(s);
    }

    else
    {
        c->send("Added %s %s", arg[0]+2, mask);

        if (type == BAN)
            foreachSyncedChannel(ch)
            ch->applyBan(s);
    }

    return SUCCESS;
}

int pl_who_whom_whob(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int n;
    char buf[MAX_LEN];
    std::string connType;

    if (!strcmp(arg[0], ".whom") || !strcmp(arg[0], ".who"))
    {
        if (!c->checkFlag(HAS_N))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }

        int i, f;

        for (i=n=0; i<net.max_conns; ++i)
        {
            if (net.conn[i].fd && net.conn[i].isReg() && net.conn[i].checkFlag(HAS_P) && !net.conn[i].isBot())
            {
		connType="via";

		if(net.conn[i].status & STATUS_IRCCLIENT)
			connType+=" irc client";
		else if(net.conn[i].status & STATUS_TELNET)
			connType+=" telnet";
		else
			connType+=" dcc chat";

		if(net.conn[i].status & STATUS_SSL)
			connType+=", SSL";

                f = net.conn[i].handle->flags[GLOBAL];
                snprintf(buf, MAX_LEN, "[#%02d]", n+1);
                c->send("%s %s (%s, %s port %s, %s)", buf, net.conn[i].handle->name,
                        f & HAS_X ? "perm" : f & HAS_S ? "super owner" : f & HAS_N ? "owner" : "channel owner",
                        net.conn[i].getPeerIpName(), net.conn[i].getPeerPortName(), connType.c_str());
                ++n;
            }
        }
        if (strcmp(arg[0], ".who")) return SUCCESS;
    }

    if (!strcmp(arg[0], ".whob") || !strcmp(arg[0], ".who"))
{
        int flags[2];

        int i;

        if (!c->checkFlag(HAS_S))
            return 0;

        if (!strcmp(arg[0], ".who"))
            flags[1] = flags[0] = STATUS_PARTY;
        else
        {
            flags[0] = (STATUS_BOT | STATUS_REDIR);
            flags[1] = STATUS_BOT;
        }

        n = 0;

        for (i=0; i<net.max_conns; ++i)
        {
            if (net.conn[i].fd >0 && net.conn[i].isRegBot() &&
                    (net.conn[i].status & flags[0]) == flags[1])
            {
                snprintf(buf, MAX_LEN, "[#%02d]", n+1);
                c->send("%s %s (slave, %s port %s)", buf, net.conn[i].handle->name,
                        net.conn[i].getPeerIpName(), net.conn[i].getPeerPortName());
                ++n;
            }
        }
        return SUCCESS;
    }
    return SUCCESS;
}

int pl_bans(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    protmodelist::sendBansToOwner(c, BAN, arg[1], arg[2]);
    return SUCCESS;

}

int pl_exempts(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    protmodelist::sendBansToOwner(c, EXEMPT, arg[1], arg[2]);
    return SUCCESS;

}

int pl_invites(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    protmodelist::sendBansToOwner(c, INVITE, arg[1], arg[2]);
    return SUCCESS;

}

int pl_reops(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    protmodelist::sendBansToOwner(c, REOP, arg[1], arg[2]);
    return SUCCESS;
}

int pl_bots(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    int i;
    char *a;

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (userlist.bots)
    {
        h = userlist.first->next;
        a = NULL;

        i = 0;

        while (h)
        {
            if (userlist.isBot(h))
            {
                a = push(a, h->name, " ", NULL);
                ++i;
            }
            h = h->next;
        }
        c->send("Bots(%d): %s", i, a);
        if (a) free(a);
    }
    else c->send("No bot handles present");
    return SUCCESS;

}

int pl_verify(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;
    int i;
    HANDLE *h;
    char buf[MAX_LEN];

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    int _test = -1;
    if (!strlen(arg[1]) || !strcmp(arg[1], "a") || !strcmp(arg[1], "all"))
        _test = 0;
    else if (!strcmp(arg[1], "p") || !strcmp(arg[1], "pass") || !strcmp(arg[1], "passwords"))
        _test = 1;
    else if (!strcmp(arg[1], "h") || !strcmp(arg[1], "hosts"))
        _test = 2;
    else if (!strcmp(arg[1], "c") || !strcmp(arg[1], "flags") || !strcmp(arg[1], "chflags"))
        _test = 3;

    if (_test == -1)
    {
        c->send("What? You need .help?");
        return 0;
    }

    /* verify passwds */
    a = NULL;
    char *_a = NULL, *__a = NULL;
    int _i = 0, __i = 0;
    int j, _j;
    h = userlist.first->next;
    i = 0;

    while (h)
    {
        if (isNullString((char *) h->pass, 16) && (_test == 0 || _test == 1))
        {
            a = push(a, h->name, " ", NULL);
            ++i;
        }

        if (_test == 0 || _test == 2)
        {
            int hidx;
            bool no_hosts=true;

            for (hidx=0; hidx<MAX_HOSTS; hidx++)
            {
                if (h->host[hidx] && *h->host[hidx])
                {
                    no_hosts=false;
                    break;
                }
            }
            if (no_hosts)
            {
                _a = push(_a, h->name, " ", NULL);
                _i++;
            }
        }

        if (!h->flags[GLOBAL] && (_test == 0 || _test == 3))
        {
            for (j = 0, _j = -1; _j == -1 && j < MAX_CHANNELS; j++)
                if (h->flags[j] && userlist.chanlist[j].name)
                    _j = j;

            if (_j == -1)
            {
                __a = push(__a, h->name, " ", NULL);
                __i++;
            }
        }
        h = h->next;
    }
    if (i)
    {
        c->send("Found %d handle%s with no password set: %s", i, i == 1 ? "" : "s", a);
        free(a);
    }

    if (_i)
    {
        c->send("Found %d handle%s with no hosts set: %s", _i,  _i == 1 ? "" : "s", _a);
        free(_a);
    }
    if (__i)
    {
        sprintf(buf, "%d", __i);
        c->send("Found %d handle%s with no flags set: %s", __i, __i == 1 ? "" : "s", __a);
        free(__a);
    }
    return SUCCESS;
}

int pl_channels(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int i, n;
    char *a, *b;
    HANDLE *h;
    char buf[MAX_LEN];

    // .channel <chan>
    if (argc == 1)
    {
        if (!c->checkFlag(HAS_N))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }

        i = userlist.findChannel(arg[1]);

        if (i != -1)
        {
            if (userlist.chanlist[i].status & PRIVATE_CHAN)
                c->send("Channel type: private");
            else
                c->send("Channel type: massjoinable");

            h = userlist.first;
            int total = 0;
            int down = 0;
            inetconn *bot;
            a = b = NULL;

            while (h)
            {
                if (userlist.isBot(h) && userlist.isRjoined(i, h))
                {
                    ++total;
                    if ((!(bot = net.findConn(h)) || !bot->name || !*bot->name) && h != userlist.first->next)
                    {
                        ++down;
                        b = push(b, b ? (char *) ", " : (char *) " ", h->name, NULL);
                    }
                    else
                        a = push(a, a ? (char *) ", " : (char *) " ", h->name, NULL);

                    /*
                    if(h == userlist.first->next)
                    {
                    	if(strlen(ME.nick))
                    		a = push(a, a ? (char *) ", " : (char *) " ", h->name, NULL);
                    	else
                    	{
                    		++down;
                    		b = push(b, b ? (char *) ", " : (char *) " ", h->name, NULL);
                    	}
                    }
                    */

                }
                h = h->next;
            }

            if (a)
            {
                c->send("Supposingly joined(%d): %s", total-down, a);
                free(a);
            }
            if (b)
            {
                c->send("Supposingly not there(%d): ", down, b);
                free(b);
            }

            return SUCCESS;
        }
        else c->send("Invalid channel");
        return 0;
    }
    else
    {
        if (!c->checkFlag(HAS_N))
        {
            c->send("%s", S_NOPERM);
            return 0;
        }

        a = NULL;
        int total, down;

        for (n=i=0; i<MAX_CHANNELS; i++)
        {
            down = total = 0;
            if (userlist.chanlist[i].name)
            {
                if (userlist.chanlist[i].status & PRIVATE_CHAN)
                {
                    h = userlist.first;
                    total = down = 0;
                    inetconn *bot;

                    while (h)
                    {
                        if (userlist.isBot(h) && userlist.isRjoined(i, h))
                        {
                            ++total;
                            if ((!(bot = net.findConn(h)) || !bot->name) && h != userlist.first->next)
                                ++down;
                        }
                        h = h->next;
                    }


                    if (userlist.chanlist[i].pass && userlist.chanlist[i].pass)
                        snprintf(buf, MAX_LEN, "%s (n: %d/%d, key: %s)",
                                 (const char *) userlist.chanlist[i].name, total-down, total, (const char *) userlist.chanlist[i].pass);
                    else
                        snprintf(buf, MAX_LEN, "%s (n: %d/%d)",
                                 (const char *) userlist.chanlist[i].name, total-down, total);
                }
                else
                {
                    if (userlist.chanlist[i].pass && userlist.chanlist[i].pass)
                        snprintf(buf, MAX_LEN, "%s (key: %s)",
                                 (const char *) userlist.chanlist[i].name, (const char *) userlist.chanlist[i].pass);
                    else
                        snprintf(buf, MAX_LEN, "%s",
                                 (const char *) userlist.chanlist[i].name);
                }

                a = push(a, a ? (char *) ", " : (char *) " ", buf, NULL);
                ++n;
            }
        }
        if (n)
        {
            c->send("Channels: %s", a);
            free(a);
        }
        else c->send("No channels found");
        return SUCCESS;
    }
    return 0;
}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_offences(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    int i;

    if (strlen(arg[1]))
    {
        h = userlist.findHandle(arg[1]);

        if (h)
        {
            if (!userlist.hasReadAccess(c, h))
            {
                c->send("%s", S_NOPERM);
                return 0;
            }
            if (!h->history || !h->history->data.entries())
            {
                c->send("No offence history available");
                return 0;
            }

            ptrlist<offence::entry>::iterator e = h->history->data.begin();
            char flags1[32], flags2[32];

            i = 1;

            c->send("%s's offence history: ", h->name);
            while (e)
            {
                userlist.flags2str(e->fromFlags, flags1);
                userlist.flags2str(e->toFlags, flags2);
                c->send("[%3d]: %s(%d): %s", i++, e->chan, e->count, e->mode);
                c->send("       %s flags decreased from `%s' to `%s'", e->global ? "Global" : "Channel", flags1, flags2);
                c->send("       Created: %s", timestr("%d/%m/%Y %T", e->time));
                e++;
            }
            return SUCCESS;
        }
        else
        {
            c->send("Invalid handle");
        }
    }
    else
    {
        userlist.reportNewOffences(c, true);
    }
    return 0;

}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_clearoffences(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    int i, n;
    char *a;

    if (strlen(arg[1]))
    {
        h = userlist.findHandle(arg[1]);

        if (h)
        {
            if (!userlist.hasReadAccess(c, h))
            {
                c->send("%s", S_NOPERM);
                return 0;
            }
            if (!h->history || !h->history->data.entries())
            {
                c->send("No offences history available");
                return 0;
            }

            h->history->data.clear();

            c->send("Offences history removed");
            //++userlist.SN;
            // userlist.updated();
            userlist.nextSave = NOW + SAVEDELAY;

            delete h->history; // we should free unused memory ;)
            h->history = NULL;
            return SUCCESS;
        }
        else
        {
            c->send("Invalid handle");
        }
    }
    else
    {
        i = n = 0;
        h = userlist.first;
        a = NULL;
        while (h)
        {
            if (h->history && h->history->data.entries())
            {
                if (userlist.hasReadAccess(c, h))
                {
                    h->history->data.clear();
                    i++;
                    a = push(a ? a : NULL, a ? (const char *) ", " : (const char *) "", (const char *) h->name, NULL);

                    delete h->history;
                    h->history = NULL;
                }
                n++;
            }
            h = h->next;
        }
        if (n)
        {

            if (n == i)   // all cleared
            {
                c->send("Offences history removed");
            }
            else
            {
                c->send("Offences history removed for handles(%d): %s", i, a);
            }
            //++userlist.SN;
            // userlist.udated();
            userlist.nextSave = NOW + SAVEDELAY;

            if (a) free(a);
            return SUCCESS;
        }
        else
        {
            c->send("No offences history available");
        }
    }
    return 0;
}

int pl_upbots(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    char *a;
    int i;

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (userlist.bots)
    {
        h = userlist.first->next->next;
        a = NULL;
        i = 0;
        while (h)
        {
            if (userlist.isBot(h) && net.findConn(h) && net.findConn(h)->isRegBot())
            {
                a = push(a, a ? (char *) ", " : (char *) " ", h->name, NULL);
                ++i;
            }
            h =h->next;
        }
        if (i)
            c->send("Bots on-line(%d): %s", i, a);

        else c->send("All bots are down");
        if (a) free(a);
    }
    else c->send("No bot handles present");
    return SUCCESS;
}

int pl_downbots(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    char *a;
    int i;

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if (userlist.bots)
    {
        h = userlist.first->next->next;
        a = NULL;
        i = 0;

        while (h)
        {
            if (userlist.isBot(h) && (!net.findConn(h) || !net.findConn(h)->isRegBot()))
            {
                a = push(a, a ? (char *) ", " : (char *) " " , h->name, NULL);
                ++i;
            }
            h = h->next;
        }
        if (i)
        {
            c->send("Bots off-line(%d): %s", i, a);
        }
        else c->send("All bots are up");
        if (a) free(a);
    }
    else c->send("No bot handles present");
    return SUCCESS;
}

int pl_bottree(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    userlist.sendBotTree(c);
    return SUCCESS;

}

int pl_owners(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;
    int i;

    if (!c->checkFlag(HAS_N))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    a = NULL;

    for (i=0; i<net.max_conns; ++i)
    {
        if (net.conn[i].isRegOwner())
            a = push(a, net.conn[i].name, " ", NULL);
    }

    c->send("Owners on-line(%d): %s", net.owners(), a);
    if (a) free(a);
    return SUCCESS;
}

int pl_users(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!userlist.users) c->send("No users in userlist");
    else userlist.sendUsers(c);
    return SUCCESS;
}

int pl_save(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    c->send("Saving userlist");
    userlist.save(config.userlist_file);
    net.send(HAS_B, "%s", S_ULSAVE);
    ME.nextRecheck = NOW + 5;
    return SUCCESS;

}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_me(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;

    a = srewind(data, 1);

    if (a)
        net.sendUser("*", "* %s %s", c->name, a);

    return 0;
}

int pl_echo(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    if (!strcmp(arg[1], "on"))
    {
        c->send("Your echo is now on");
        c->status &= ~STATUS_NOECHO;
        return SUCCESS;
    }
    else if (!strcmp(arg[1], "off"))
    {
        c->send("Your echo is now off");
        c->status |= STATUS_NOECHO;
        return SUCCESS;
    }
    else
        c->send("Invalid argument");
    return 0;
}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_idiots(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    h = userlist.findHandle(userlist.first->name);
    if (h)
    {
        if (userlist.hasReadAccess(c, h))
        {
            userlist.sendHandleInfo(c, h, arg[1]);
            return SUCCESS;
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_addidiot(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char buf[MAX_LEN];
    HANDLE *h;

    if (!extendhost(arg[1], buf, MAX_LEN))
    {
        c->send("Invalid hostname");
        return 0;
    }
    h = userlist.findHandle(userlist.first->name);
    if (h)
    {
        if (userlist.hasWriteAccess(c, h))
        {
            if (userlist.addHost(h, buf, c->name, NOW) != -1)
            {
                c->send("Adding host %s to handle %s", buf, h->name);
                net.send(HAS_B, "%s %s %s", S_ADDHOST, h->name, buf);
                //ME.recheckFlags();
                userlist.updated();
                return SUCCESS;
            }
            else c->send("Host exists");
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

/* missing.
 *
 * \author Esio <esio@hoth.amu.edu.pl>
 */

int pl_delidiot(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;
    int n;

    h = userlist.findHandle(userlist.first->name);

    if (h)
    {
        if (userlist.hasWriteAccess(c, h))
        {
            if ((n = userlist.findHost(h, arg[1])) != -1)
            {
                c->send("Removing host %s from handle %s", h->host[n], h->name);
                net.send(HAS_B, "%s %s %s", S_RMHOST, h->name, h->host[n]);
                userlist.removeHost(h, arg[1]);
                userlist.updated();
                return SUCCESS;
            }
            else c->send("Invalid host");
        }
        else c->send("%s", S_NOPERM);
    }
    else c->send("Invalid handle");
    return 0;
}

int pl_dset(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    char *a;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    a = srewind(data, 2);

    if (userlist.dset->parseUser(c->name, arg[1], a ? a : "", "dset"))
        userlist.nextSave = NOW + SAVEDELAY;

    return 0;
}

int pl_unlink(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    if (!c->checkFlag(HAS_S))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }
    h = userlist.findHandle(arg[1]);
    if (h)
    {
        if (userlist.isSlave(h) || userlist.isLeaf(h))
        {
            inetconn *bot = net.findConn(h);
            if (bot)
            {
                if (bot->status & STATUS_REDIR)
                {
                    inetconn *slave = net.findRedirConn(bot);
                    if (slave) slave->send("%s %s", S_UNLINK, arg[1]);
                }
                else bot->close("Forced unlink");
                return SUCCESS;
            }
            else c->send("Bot is down");
        }
        else c->send("Invalid bot");
        return 0;
    }
    else c->send("No such handle");
    return 0;
}

int pl_botcmd(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    int ret=0;

    if (!strcmp(arg[1], config.handle))
    {
        pstring<> str(c->name);
        str += " ";
        str += srewind(data, 2);
        botnetcmd(config.handle, str);
        ret=SUCCESS;
    }
    else
    {
        HANDLE *p;
        inetconn *bot;

        for (p = userlist.first; p; p=p->next)
        {
            if (!(p->flags[GLOBAL] & HAS_B))
                continue;

            if (match(arg[1], p->name))
            {
                if (!strcmp(p->name, config.handle))
                {
                    pstring<> str(c->name);
                    str += " ";
                    str += srewind(data, 2);

                    botnetcmd(config.handle, str);
                    ret=SUCCESS;
                }

                else
                {
                    bot = net.findConn(p);

                    if (bot)
                    {
                        bot->send("%s %s %s", S_BOTCMD, c->name, srewind(data, 2));
                        ret=SUCCESS;
                    }
                }
            }
        }
    }

    return ret;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int pl_addaddr(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    h=userlist.findHandle(arg[1]);

    if(!h)
    {
        c->send("Invalid handle");
        return 0;
    }

    if(!userlist.hasWriteAccess(c, h))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    h->addr->add(arg[2]);
    c->send("Adding addr %s to handle %s", arg[2], arg[1]);
    net.send(HAS_B, "%s %s %s", S_ADDADDR, arg[1], arg[2]);
    userlist.updated();
    return SUCCESS;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int pl_deladdr(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    HANDLE *h;

    h=userlist.findHandle(arg[1]);

    if(!h)
    {
        c->send("Invalid handle");
        return 0;
    }

    if(!userlist.hasWriteAccess(c, h))
    {
        c->send("%s", S_NOPERM);
        return 0;
    }

    if(h->addr->remove(arg[2]))
    {
        c->send("Removing addr %s from handle %s", arg[2], arg[1]);
        net.send(HAS_B, "%s %s %s", S_RMADDR, arg[1], arg[2]);
        userlist.updated();
        return SUCCESS;
    }

    else
        c->send("Invalid addr");

    return 0;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int pl_help(inetconn *c, char *data, char arg[][MAX_LEN], int argc)
{
    partyline_commands *pt;

    if (argc == 0)
    {
        c->send("Available commands:");

        for (pt=partyline_cmds; pt->command != NULL; pt++)
        {
            if(pt->main_only && config.bottype != BOT_MAIN)
                continue;

            c->send(".%s %s", pt->command, pt->syntax);
        }

        return SUCCESS;
    }

    else if (argc > 0)
    {
        for (pt=partyline_cmds; pt->command != NULL; pt++)
        {
            if(pt->main_only && config.bottype != BOT_MAIN)
                continue;

            if (!strcmp(pt->command, arg[1]))
            {
                c->send("syntax: %s %s", pt->command, pt->syntax);
                return SUCCESS;
            }
        }
    }

    return 0;
}
