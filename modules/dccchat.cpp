/* support for DCC CHAT */

#include <prots.h>
#include <global-var.h>

#define ALLOW_SLAVE_DCC_CHAT

void hook_ctcp(const char *from, const char *to, const char *data)
{
  char arg[10][MAX_LEN], *a, buf[MAX_LEN], who[MAX_LEN];
  int n;

  if(!strlen(data))
    return;

  str2words(arg[0], data, 10, MAX_LEN);

  if(!strcmp(arg[0], "DCC") && !strcmp(arg[1], "CHAT") && strlen(arg[4]))
  {
    if(isValidIp(buf) == 6)
      strncpy(buf, arg[3], MAX_LEN-1);
    else
      snprintf(buf, MAX_LEN, "%s", inet2char(htonl(strtoul(arg[3], NULL, 10))));

#ifndef ALLOW_SLAVE_DCC_CHAT
    if(config.bottype == BOT_MAIN && userlist.hasPartylineAccess(from))
#else
    if(userlist.hasPartylineAccess(from))
#endif
    {
      switch(isValidIp(buf))
      {
#ifdef HAVE_IPV6
        case 6:
        n = doConnect6(buf, atoi(arg[4]), 0, -1);
        break;
#endif

        default:
        n = doConnect(buf, atoi(arg[4]), config.myipv4, -1);
      }

      if(n > 0)
      {
        inetconn *c = net.addConn(n);
        c->status = STATUS_SYNSENT + STATUS_PARTY;
        c->killTime = NOW + set.AUTH_TIMEOUT;
        c->tmpint = 0;
        mem_strcpy(c->tmpstr, from);
      }

      else
      {
        ME.privmsg(who, "Cannot establish connection (%s)", strerror(errno));
      }
    }
#ifndef ALLOW_SLAVE_DCC_CHAT
    else if(config.bottype == BOT_MAIN)
#else
    else
#endif
    {
      net.send(HAS_N, "[-] Illegal dcc chat from: %s", from);
    }
  }

}

extern "C" module *init()
{
    module *m=new module("dcc chat", "pks, patrick", "0.1");
    m->hooks->ctcp=hook_ctcp;
    return m;
}

extern "C" void destroy()
{
}

