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

static char mode[2][MODES_PER_LINE];
static char *arg[MODES_PER_LINE];
static modeq::modeq_ent *mqc[MODES_PER_LINE];
static char *a, *b;
static char sign;
static char modeBuf[3];
static chanuser *nickHandle = NULL, *argHandle, serverHandle("");
static chanuser *multHandle[6];
static ptrlist<chanuser>::iterator p;
static int i, j, just_opped, oped, mypos, n[6], hash, ops, mq, tmp;
static unsigned int number; // offence counter

void chan::gotMode(const char *modes, const char *args, const char *mask)
{
    /* initialization */
    bool hasArg;
    const char *oldParam=NULL;
    bool needKick=false;
    char foo[3];
    char mode_type;
    bool checkModeProtections;
    bool botGotOp=false;
    modesType::iterator iter;
    oped = just_opped = 0;
    mypos = -1;
    mq = 0;
    memset(mode, 0, sizeof(mode));
    memset(arg, 0, sizeof(arg));
    sign = '+';

    number = 0;

    /* user, ircerver detection */
    nickHandle = getUser(mask);
    if (!nickHandle) nickHandle = &serverHandle;

    /* mode striper */
    for (i=j=0,ops=0; i<MODES_PER_LINE && (unsigned int) j < strlen(modes); i++, j++)
    {
        if (modes[j] == '+' || modes[j] == '-')
        {
            sign = modes[j];
            j++;
        }
        mode[0][i] = sign;
        mode[1][i] = modes[j];
        if (sign == '+' && mode[1][i] == 'o') ops++;
    }

    /* we backup args in `b' var cause address will be changed in arg striper */
    if ((int) chset->IDIOTS && nickHandle != &serverHandle)
    {
        if (args && *args)
            b = push(NULL, (const char *) "mode ", modes, (const char *) " ", args, NULL);
        else
            b = push(NULL, (const char *) "mode ", modes, NULL);

        /* should we remove spaces from end of reason? */
        //b = rtrim(b);
    }

    /* arg striper */
    for (i = 0; i<MODES_PER_LINE; i++)
    {
        if (chanModeRequiresArgument(mode[0][i], mode[1][i]))
        {
            a = strchr((char *)args, ' ');
            if (a) mem_strncpy(arg[i], args, abs(args - a)+1);
            else
            {
                if (strlen(args)) mem_strcpy(arg[i], args);
                break;
            }
            args = a+1;
        }
    }

    /* mode parser */
    for (i=0; i<MODES_PER_LINE; i++)
    {
        checkModeProtections=false;
        hasArg=chanModeRequiresArgument(mode[0][i], mode[1][i]);
        needKick=false;
        oldParam=NULL;
        mode_type=chan::getTypeOfChanMode(mode[1][i]);

        if (!strchr(ME.server.isupport.chan_status_flags, mode[1][i]) && mode_type != 'A')
        {
            std::list<string>::iterator smit;

            oldParam=getModeParam(mode[1][i]);
            snprintf(foo, 3, "%c%c", mode[0][i], mode[1][i]);

            if (mode[0][i] == '+')
            {
                iter=this->modes.find(mode[1][i]);

                if (iter != this->modes.end())
                    this->modes.erase(iter);

                this->modes.insert(std::pair<char, string> (mode[1][i], hasArg ? arg[i] : ""));
                sentModes.remove(foo, hasArg ? arg[i] : NULL);

                if (*key() && set.REMEMBER_OLD_KEYS)
                {
                    userlist.chanlist[channum].pass=key();
                    userlist.nextSave = NOW + SAVEDELAY;
                }
            }
            else
            {
                iter=this->modes.find(mode[1][i]);

                if (iter != this->modes.end())
                    this->modes.erase(iter);

                sentModes.remove(foo, hasArg ? arg[i] : NULL);
            }

            checkModeProtections=true; // protect-chmodes and mode-lock
        }

        if (!mode[1][i])
            continue;

        if (mode[0][i] == '+')
        {
            switch (mode[1][i])
            {
                ///////////////////
            case 'o':
            {
                bool wasOp;

                argHandle = getUser(arg[i]);
		wasOp=(argHandle->flags & IS_OP) ? true : false;

                if ((argHandle->flags & (HAS_B | IS_OP)) == HAS_B)
                {
                    botsToOp.remove(argHandle);
                    opedBots.sortAdd(argHandle);
                    oped++;
                    initialOp = NOW;
                }

                if (argHandle->flags & HAS_O)
                    toOp.remove(argHandle);

                tmp = argHandle->flags & IS_OP;

                argHandle->flags |= IS_OP;
                argHandle->flags &= ~OP_SENT;

                if ((!(nickHandle->flags & HAS_M) || (ops > 1 && !(nickHandle->flags & HAS_N)))
                        && !tmp && ((nickHandle == &serverHandle) ?
                                    (chset->STOP_NETHACK ? (chset->WASOPTEST ? !wasop->remove(argHandle) : 0) : 0) : chset->BITCH))
                {
                    if (((nickHandle == &serverHandle) ? 1 : !(argHandle->flags & HAS_O)) &&
                            !(argHandle->flags & HAS_F))
                    {
                        if (!(argHandle->flags & (HAS_B | HAS_N))) toKick.sortAdd(argHandle);
                        if (*nickHandle->nick)
                        {
                            number++;
                            toKick.sortAdd(nickHandle);
                        }
                    }
                }
                if (argHandle->flags & HAS_D) toKick.sortAdd(argHandle);

                if (chset->BITCH && set.DONT_TRUST_OPS && argHandle->flags & HAS_O && !tmp &&
                        nickHandle != &serverHandle && !(nickHandle->flags & HAS_F))
                {
                    if (set.DONT_TRUST_OPS == 2 && !(argHandle->flags & HAS_S))
                        toKick.sortAdd(argHandle);
                    if (set.DONT_TRUST_OPS == 1 && !(argHandle->flags & HAS_S) && !(argHandle->flags & HAS_B))
                        toKick.sortAdd(argHandle);

                    toKick.sortAdd(nickHandle);
                    number++;
                }

                if (me == argHandle)
                {
                    if (since + set.ASK_FOR_OP_DELAY <= NOW ||
                            opedBots.entries() < 5 || toKick.entries() > 4)
                        mypos = oped - 1;

                    if(!wasOp)
                        botGotOp=true;
                }
            }
            break; //+o

            ///////////////////
            case 'v':
                argHandle = getUser(arg[i]);
                argHandle->flags |= IS_VOICE;
                argHandle->flags &= ~VOICE_SENT;

                if (argHandle->flags & HAS_Q && !(nickHandle->flags & HAS_N))
                    mqc[mq++] = modeQ[PRIO_LOW].add(0, "-v", argHandle->nick);
                break; //+v

                ///////////////////
            case 'b':
            {
                bool sticky = protmodelist::isSticky(arg[i], BAN, this);
                list[BAN].add(arg[i], nickHandle->nick, sticky ? 0 : set.BIE_MODE_BOUNCE_TIME);
                sentList[BAN].remove(arg[i]);
                if (gotBan(arg[i], nickHandle) && !sticky)
                {
                    mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-b", arg[i]);
                    number++;
                }
            }
            break; //+b

            ///////////////////
            case 'l':
                if (chset->LIMIT)
                {
                    if (limit() == atoi(oldParam))
                    {
                        checkModeProtections=false;
                        break;
                    }

                    /* +n, +b or irc server has changed the limit */
                    if ((nickHandle->flags & (HAS_N | HAS_B)) || nickHandle == &serverHandle)
                    {
                        /* bot has changed the limit */
                        if (nickHandle->flags & HAS_B)
                        {
                            if (limit() == -1) // FIXME: when is it -1?
                                nextlimit = -1;
                            else
                                nextlimit = NOW + chset->LIMIT_TIME_UP;
                        }
                        /* owner has changed the limit */
                        else if (nickHandle->flags & HAS_N)
                        {
                            if (limit() == -1) nextlimit = -1;
                            else nextlimit = NOW + chset->OWNER_LIMIT_TIME;

                            /* enforce this limit */
                            enforceLimits();
                        }
                        /* server has changed the limit */
                        else if (nickHandle == &serverHandle)
                            nextlimit = NOW + set.SERVER_LIMIT_TIME;
                    }

                    else
                    {
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", oldParam);
                        toKick.sortAdd(nickHandle);
                        number++;
                    }

                    checkModeProtections=false;
                }

                break; //+l

                ///////////////////
            case 'e' :
            case 'I' :
            case 'R' :
            {
                int type;
                int _chset;
                protmodelist::entry *global, *local;

                if (mode[1][i] == 'e')
                {
                    type=EXEMPT;
                    _chset=chset->USER_EXEMPTS;
                }

                else if (mode[1][i] == 'I')
                {
                    type=INVITE;
                    _chset=chset->USER_INVITES;
                }

                else if (mode[1][i] == 'R')
                {
                    if (mode_type != 'A')
                        break;

                    type=REOP;
                    _chset=chset->USER_REOPS;
                }
                else
                    break;

                global=userlist.protlist[type]->find(arg[i]);
                local=protlist[type]->find(arg[i]);

                if (_chset == 1 && !(nickHandle->flags & (HAS_M | HAS_B)) && *nickHandle->nick)
                {
                    snprintf(foo, 3, "-%c", mode[1][i]);
                    mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                    toKick.sortAdd(nickHandle);
                    number++;
                }

                else if (_chset == 2)
                {
                    if (!local && !global)
                    {
                        if (!(nickHandle->flags & (HAS_M | HAS_B)) && *nickHandle->nick)
                        {
                            toKick.sortAdd(nickHandle);
                            number++;
                        }

                        snprintf(foo, 3, "-%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                    }
                }

                list[type].add(arg[i], nickHandle->nick, ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
                sentList[type].remove(arg[i]);
            }
            break; // +eIR
            }

            if (checkModeProtections)
            {
                if (mode_type == 'B' && oldParam && strcmp(oldParam, arg[i]))
                {
                    if (chset->PROTECT_CHMODES && nickHandle != &serverHandle && !(nickHandle->flags & (HAS_N | HAS_B)))
                    {
                        if (oldParam && *oldParam)
                        {
                            snprintf(foo, 3, "+%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        }

                        else
                        {
                            snprintf(foo, 3, "-%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                        }

                        needKick=true;
                    }

                    else if (oldParam && *oldParam && isLockedMode('+', mode[1][i], oldParam))
                    {
                        snprintf(foo, 3, "+%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        needKick=true;
                    }

                    else if ((isLockedMode('-', mode[1][i], arg[i]))
                             || isLockedMode('-', mode[1][i], "*"))
                    {
                        snprintf(foo, 3, "-%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                        needKick=true;

                    }

                }

                if (mode_type == 'C' && oldParam && strcmp(oldParam, arg[i]))
                {
                    if (chset->PROTECT_CHMODES && nickHandle != &serverHandle && !(nickHandle->flags & (HAS_N | HAS_B)))
                    {
                        if (oldParam && *oldParam)
                        {
                            snprintf(foo, 3, "+%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        }

                        else
                        {
                            snprintf(foo, 3, "-%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo);
                        }

                        needKick=true;
                    }

                    else if (oldParam && *oldParam && isLockedMode('+', mode[1][i], oldParam))
                    {
                        snprintf(foo, 3, "+%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        needKick=true;
                    }

                    else if (isLockedMode('-', mode[1][i]))
                    {
                        snprintf(foo, 3, "-%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo);
                        needKick=true;

                    }
                }

                if (mode_type == 'D')
                {
                    if ((chset->PROTECT_CHMODES && *nickHandle->nick && !(nickHandle->flags & (HAS_N | HAS_B)))
                            || isLockedMode('-', mode[1][i]))
                    {
                        snprintf(foo, 3, "-%c", mode[1][i]);
                        mqc[mq++] = modeQ[PRIO_LOW].add(0, foo);
                        needKick=true;
                    }
                }
            }
        }
        else if (mode[0][i] == '-')
        {
            switch (mode[1][i])
            {
                //////////////////
            case 'o':
                argHandle = getUser(arg[i]);
                toKick.remove(argHandle);

                /* user had @ */
                if (argHandle->flags & IS_OP)
                {
                    /* is it a bot ? */
                    if (argHandle->flags & HAS_B)
                    {
                        opedBots.remove(argHandle);
                        botsToOp.sortAdd(argHandle);
                        if (oped > 0) --oped;
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+o", arg[i]);
                    }
                    else if (argHandle->flags & HAS_R && userLevel(argHandle) > userLevel(nickHandle))
                    {
                        mqc[mq++] = modeQ[PRIO_LOW].add(0, "+o", arg[i]);
                    }
                }

                argHandle->flags &= ~IS_OP;

                //punish
                if ((userLevel(nickHandle) < userLevel(argHandle) || nickHandle->flags & HAS_B)
                        && *nickHandle->nick)
                {
                    toKick.sortAdd(nickHandle);
                    number++;
                }
                if (argHandle == me)
                {
                    mypos = -1;
                    p = toKick.begin();
                    while (p)
                    {
                        p->flags &= ~(KICK_SENT | OP_SENT | VOICE_SENT);
                        p++;
                    }
                    sentKicks = 0;
                    if (opedBots.entries()) initialOp = NOW;
                    else initialOp = 0;

                    sentList[BAN].clear();
                    sentList[INVITE].clear();
                    sentList[EXEMPT].clear();
                    sentList[REOP].clear();
                }
                break; //-o

                //////////////////
            case 'v':
                argHandle = getUser(arg[i]);
                argHandle->flags &= ~IS_VOICE;
                break; //-v

                //////////////////
            case 'l':
                if (chset->LIMIT)
                {
                    if (!(nickHandle->flags & (HAS_N | HAS_B)))
                    {
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", oldParam);
                        toKick.sortAdd(nickHandle);
                        number++;
                    }

                    else
                    {
                        if (nickHandle->flags & HAS_N)
                            nextlimit = NOW + chset->OWNER_LIMIT_TIME;
                        else if (nickHandle->flags & HAS_B)
                            nextlimit = NOW + (rand() % 5) + 1;
                    }

                    checkModeProtections=false;
                }
                break; //-l

                //////////////////
            case 'b':
                protmodelist::entry *ban;
                list[BAN].remove(arg[i]);

                if ((ban=protmodelist::findSticky(arg[i], BAN, this)))
                {
                    if (!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
                    {
                        nickHandle->setReason(ban->fullReason());
                        toKick.sortAdd(nickHandle);
                        number++;
                    }
                    mqc[mq++] = modeQ[PRIO_HIGH].add(0 ,"+b", arg[i]);
                }
                break; //-b

                //////////////////
            case 'e' :
            case 'I' :
            case 'R' :
            {
                int type;
                int _chset;
                protmodelist::entry *global, *local;

                if (mode[1][i] == 'e')
                {
                    type=EXEMPT;
                    _chset=chset->USER_EXEMPTS;
                }

                else if (mode[1][i] == 'I')
                {
                    type=INVITE;
                    _chset=chset->USER_INVITES;
                }

                else if (mode[1][i] == 'R')
                {
                    if (mode_type != 'A')
                        break;

                    type=REOP;
                    _chset=chset->USER_REOPS;
                }

                else
                    break;

                if (list[type].remove(arg[i]) && _chset != 0)
                {
                    global=userlist.protlist[type]->find(arg[i]);
                    local=protlist[type]->find(arg[i]);

                    if (_chset == 1 || (_chset == 2 && (local || global)))
                    {
                        if (!(nickHandle->flags & (HAS_M | HAS_B)) && *nickHandle->nick)
                        {
                            if (local && local->sticky)
                                nickHandle->setReason(local->fullReason());

                            else if (global && global->sticky)
                                nickHandle->setReason(global->fullReason());

                            toKick.sortAdd(nickHandle);
                            snprintf(foo, 3, "+%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                            number++;
                        }

                        else if (_chset == 2)
                        {
                            snprintf(foo, 3, "+%c", mode[1][i]);
                            mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, arg[i]);
                        }
                    }
                }
            }
            break; // -eIR
            } // switch

            if (checkModeProtections)
            {
                snprintf(foo, 3, "+%c", mode[1][i]);

                if (mode_type == 'B' || mode_type == 'C')
                {
                    // use oldParam because on some ircds "-k key" works even when a different key was set

                    if (chset->PROTECT_CHMODES && *nickHandle->nick && !(nickHandle->flags & (HAS_N | HAS_B)))
                    {
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        needKick=true;
                    }

                    else if (isLockedMode('+', mode[1][i], oldParam) || isLockedMode('+', mode[1][i], "*"))
                    {
                        mqc[mq++] = modeQ[PRIO_HIGH].add(0, foo, oldParam);
                        needKick=true;
                    }
                }

                if (mode_type == 'D')
                {
                    if (((chset->PROTECT_CHMODES && *nickHandle->nick && !(nickHandle->flags & (HAS_N | HAS_B)))
                            || isLockedMode('+', mode[1][i])))
                    {
                        mqc[mq++] = modeQ[PRIO_LOW].add(0, foo);
                        needKick=true;
                    }
                }
            }
        } //mode

        if (needKick)
        {
            if (*nickHandle->nick && !(nickHandle->flags & (HAS_N | HAS_B)))
            {
                toKick.sortAdd(nickHandle);
                number++;
            }
        }
    } //for


    /* channel modes protection */
    if (mq)
    {
        if (myTurn(chset->GUARDIAN_BOTS, nickHandle->hash32()))
        {
            for (i=0; i<mq; ++i)
                mqc[i]->expire = NOW;

            modeQ[PRIO_HIGH].flush(PRIO_HIGH);
        }
        else
        {
            j = ((myPos() + hash32(name)) % (opedBots.entries() + 1) + 1) * set.BACKUP_MODE_DELAY;
            for (i=0; i<mq; ++i)
            {
                mqc[i]->expire = NOW + j;
                mqc[i]->backupmode = true;
            }
        }
    }

    //defense op code
    if (toKick.entries() < 4 && me->flags & IS_OP)
    {
        if (chset->BOT_AOP_MODE == 2 && mypos == 0 && botsToOp.entries())
        {
            j = getRandomItems(multHandle, botsToOp.begin(), botsToOp.entries(), 3);
            op(multHandle, j);
        }
    }
    //takeover op
    else if (mypos != -1 && me->flags & IS_OP)
    {
        if (chset->BOT_AOP_MODE)
        {
            divide((int *) &n, botsToOp.entries(), oped, set.OPS_PER_MODE);
            if (n[mypos])
            {
                if (mypos == 0) j = getRandomItems(multHandle, botsToOp.begin(), n[mypos], set.OPS_PER_MODE);
                else if (mypos == 1)	j = getRandomItems(multHandle, botsToOp.getItem(n[mypos-1]), n[mypos], set.OPS_PER_MODE);
                else j = getRandomItems(multHandle, botsToOp.getItem(n[mypos-1] + n[mypos-2]), n[mypos], set.OPS_PER_MODE);
                op(multHandle, j);
            }
        }
    }

    //big kick
    if (toKick.entries() > 6 && me->flags & IS_OP)
    {
        j = getRandomItems(multHandle, toKick.begin(), toKick.entries()-sentKicks, 6, KICK_SENT);
        kick6(multHandle, j);
    }

    //kick for sth bad
    else if (toKick.entries() && me->flags & IS_OP)
    {
        p = toKick.begin();

        i = hash = 0;
        while (p)
        {
            if (!(p->flags & KICK_SENT))
            {
                multHandle[i] = p;
                hash += p->hash32();
                if (++i == 6) break;
            }
            p++;
        }

        if (myTurn(chset->PUNISH_BOTS, hash))
        {
            kick6(multHandle, i);

            /* idiots code */
            if (nickHandle != &serverHandle && (int) chset->IDIOTS && number)
            {
                if (userlist.isMain(userlist.me()))
                    userlist.addIdiot(mask, (const char *) name, b, number);
                else
                {
                    if (net.hub.fd && net.hub.isMain())
                        net.hub.send("%s %s %s %d %s", S_ADDIDIOT, mask, (const char *) name, number, b);
                    else
                    {
                        for (i=0; i<net.max_conns; ++i)
                            if (net.conn[i].isMain() && net.conn[i].fd)
                            {
                                net.conn[i].send("%s %s %s %d %s", S_ADDIDIOT, mask, (const char *) name, number, b);
                                break;
                            }
                    }
                }
            }
        }
    }


    if(botGotOp && synlevel >= 3) // got WHO and MODE
    {
        justSyncedAndOped();
    }

    HOOK(mode, mode(this, mode, const_cast<const char**> (arg), mask));
    stopParsing=false;

    for (i=0; i<MODES_PER_LINE; ++i)
    {
        if (mode[0][i] == '-' || mode[0][i] == '+')
        {
            modeBuf[0] = mode[0][i];
            modeBuf[1] = mode[1][i];
            modeBuf[2] = '\0';

            DEBUG(printf(">>> removeBackupModesFor(%s, %s)\n", modeBuf, arg[i]));
            modeQ[PRIO_LOW].removeBackupModesFor(modeBuf, arg[i]);
            modeQ[PRIO_HIGH].removeBackupModesFor(modeBuf, arg[i]);
        }
        if (arg[i]) free(arg[i]);
    }

    if ((int) chset->IDIOTS && nickHandle != &serverHandle) free(b);
}
