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

int bc_pset(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_cfg(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_cfg_save(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_modules(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_rehash(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_die(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_restart(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_raw(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_names(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_cwho(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_update(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_stopupdate(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_jump(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_status(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_help(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);
int bc_debug(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc);

bot_commands bot_cmds[] =
{
    { "pset", bc_pset, 0, "[var] [value]" },
    { "cfg", bc_cfg, 0, "[var] [value]" },
    { "cfg-save", bc_cfg_save, 0, "" },
    { "modules", bc_modules, 0, "" },
    { "rehash", bc_rehash, 0, "" },
    { "debug", bc_debug, 0, "" },
    { "die", bc_die, 0, "" },
    { "restart", bc_restart, 0, "" },
    { "raw", bc_raw, 1, "<text>" },
    { "names", bc_names, 1, "<chan>" },
    { "cwho", bc_cwho, 1, "<chan> [v|o|l|b]" },
    { "update", bc_update, 0, "[id:passwd]" },
    { "stopupdate", bc_stopupdate, 0, "" },
    { "jump", bc_jump, 1, "<#number>" },
    { "status", bc_status, 0, "" },
    { "help", bc_help, 0, "" },
    { NULL, NULL, 0, NULL }
};

void botnetcmd(const char *from, const char *data)
{
    char arg[10][MAX_LEN];
    int argc;
    bot_commands *bc;
    HANDLE *h;

    HOOK(botnetcmd, botnetcmd(from, data));

    if (stopParsing)
    {
        stopParsing=false;
        return;
    }

    argc=str2words(arg[0], data, 10, MAX_LEN);

    h=userlist.findHandle(arg[0]);

    if(!h)
        return;

    for (bc=bot_cmds; bc->command != NULL; bc++)
    {
        if (!strcmp(bc->command, arg[1]))
        {
            if (argc-2 < bc->min_args)
                net.sendUser(arg[0], "syntax: .bc %s %s %s", (const char*) config.handle, bc->command, bc->syntax);

            else
                bc->func(h, data, arg, argc-2);

            return;
        }
    }
}

int bc_pset(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    pset.parseUser(arg[0], arg[2], arg[3], "pset");

    return 0;
}

int bc_cfg(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    config.parseUser(arg[0], arg[2], srewind(data, 3), "cfg");

    return 0;
}

int bc_cfg_save(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    options::event *e = config.save();
    net.sendUser(arg[0], "cfg-save: %s", (const char *) e->reason);

    return 0;
}

int bc_modules(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    int n = 0;
    ptrlist<module>::iterator i = modules.begin();

    while (i)
    {
        net.sendUser(arg[0], "module: %s (v%s by %s)", (const char *) i->file, (const char *) i->version, (const char *) i->author);
        i++;
        n++;
    }

    net.sendUser(arg[0], "%d modules have been found", n);
    return 0;
}

int bc_rehash(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    ptrlist<module>::iterator i = modules.begin();
    int num = 0;

    while (i)
    {
        if (!*arg[2] || !strcmp(arg[2], i->file))
        {
            net.send(HAS_N, "rehashing module: %s", (const char *) i->file);
            ++num;

        }

        i++;
    }

    HOOK(rehash, rehash());
    return 0;
}

int bc_die(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    userlist.autoSave(1);
    net.send(HAS_N, "\002Terminated by %s\002", arg[0]);
    ME.quit();
    safeExit();
    return 0;
}

int bc_restart(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    userlist.autoSave(1);
    net.send(HAS_N, "\002Restarted by %s\002", arg[0]);
    ME.restart();
    return 0;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int bc_raw(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    char *text;

    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    text=srewind(data, 2);

    if(penalty < 10)
        net.irc.send("%s", text);

    else
        net.sendUser(arg[0], "Penalty is too high. Please wait a while and try again.");

    return 0;
}

int bc_names(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    chan *ch = ME.findChannel(arg[2]);

    if (ch)
    {
        if(h->flags[GLOBAL] & HAS_N || h->flags[ch->channum] & HAS_N)
            ch->names(arg[0]);

        else
        {
            net.sendUser(arg[0], "%s", S_NOPERM);
            return 0;
        }
    }

    else
        net.sendUser(arg[0], "Invalid channel");

    return 0;
}

int bc_cwho(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    chan *ch = ME.findChannel(arg[2]);

    if (ch)
    {
        if(h->flags[GLOBAL] & HAS_N || h->flags[ch->channum] & HAS_N)
            ch->cwho(arg[0], arg[3]);

        else
        {
            net.sendUser(arg[0], "%s", S_NOPERM);
            return 0;
        }
    }

    else
        net.sendUser(arg[0], "Invalid channel");

    return 0;
}

int bc_update(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    psotget.forkAndGo(NULL, argc == 1 ? arg[2] : NULL);
    return 0;
}

int bc_stopupdate(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    psotget.end();
    return 0;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int bc_jump(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    int idx;

    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    if(arg[2][0] == '#') {
      idx=atoi(arg[2]+1);
      idx--;

      if(idx < 0 || idx >= MAX_SERVERS || config.server[idx].isDefault())
        net.sendUser(arg[0], "wrong number");

      else
        ME.jump(idx);
    }

    else {
      net.sendUser(arg[0], "please use: .bc %s jump <#number>", (const char*) config.handle);
      net.sendUser(arg[0], "use '.bc %s cfg server' to get the list of servers", (const char*) config.handle);
      net.sendUser(arg[0], "example: .bc %s jump #1", (const char*) config.handle);
    }

    return 0;
}

int bc_status(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    if(!(h->flags[GLOBAL] & HAS_X))
    {
        net.sendUser(arg[0], "%s", S_NOPERM);
        return 0;
    }

    ME.sendStatus(arg[0]);
    return 0;
}

/* missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

int bc_help(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    bot_commands *bc;

    if(argc == 0)
    {
        net.sendUser(arg[0], "Available commands:");

        for(bc=bot_cmds; bc->command != NULL; bc++)
            net.sendUser(arg[0], ".bc %s %s %s", (const char*) config.handle, bc->command, bc->syntax);

        return 0;
    }

    else if(argc > 0)
    {
        for(bc=bot_cmds; bc->command != NULL; bc++)
        {
            if(!strcmp(bc->command, arg[2]))
            {
                net.sendUser(arg[0], "syntax: .bc %s %s %s", (const char*) config.handle, bc->command, bc->syntax);
                return 0;
            }
        }
    }

    return 0;
}

int bc_debug(HANDLE *h, const char *data, char arg[][MAX_LEN], int argc)
{
    net.sendUser(arg[0], "timestamp: %ld", userlist.timestamp);
    return 0;
}
