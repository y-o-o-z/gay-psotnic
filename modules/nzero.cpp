/* 
 * Some IRCnet servers do not allow regstration with "NICK 0".
 * This module sends "NICK 0" after registration.
 */

#include <prots.h>
#include <global-var.h>

void hook_connected()
{
    net.irc.send("NICK 0");
}

extern "C" module *init()
{
    module *m=new module("uid-nick-after-connect", "patrick <patrick@psotnic.com>", "0.1");
    m->hooks->connected=hook_connected;
    return m;
}

extern "C" void destroy()
{
}
