/* coded for mioprofilo.it
 *
 * works only on irc 2.11.2
 * FIXME: crashes on +realname_pattern (without arg)
 */

#include <prots.h>
#include <global-var.h>

#define BAD_REALNAME_BAN_TIME 600
#define BAD_REALNAME_BAN_REASON "utente sgradito"
#define SID_BAN_TIME 600
#define SID_BAN_REASON "utente sgradito"

#define WHO_DELAY 600

#define MAX_REALNAMES 32

//const char *banned_realname_patterns[] = { "*dhioldfk*" };
const char *banned_sid[] = { }; // ban only these SIDs (leave banned_sid_exceptions empty)
const char *banned_sid_exceptions[] = { "380*" }; // ban all SIDs but those which are listed here (wildcards allowed, leave banned_sid empty)

time_t next_who;
void __punish(chan *ch, chanuser *cu, const char *reason, time_t bantime);

class br_settings : public options
{
 public:
  entMult realname_pattern_storage;
  entWord realname_pattern[MAX_REALNAMES];

  br_settings();
};

br_settings::br_settings()
{
  registerObject(realname_pattern_storage = entMult("realname_pattern"));

  for(int i=0; i < MAX_REALNAMES; i++)
  {
    registerObject(realname_pattern[i] = entWord("realname_pattern", 1, 128));
    realname_pattern[i].setDontPrintIfDefault(true);
    realname_pattern_storage.add(&realname_pattern[i]);
  }
}

br_settings br_set;

void hook_raw(const char *data)
{
  char arg[11][MAX_LEN];
  unsigned int idx, size;
  chan *ch;
  chanuser *cu;
  char *realname;
  char buffer[MAX_LEN];

  str2words(arg[0], data, 11, MAX_LEN, 1);

  if(!strcmp(arg[1], "352")) {
    /* arg[2] = <my nick>
     * arg[3] = <channel>
     * arg[4] = <user>
     * arg[5] = <host>
     * arg[6] = <server>
     * arg[7] = <nick>
     * arg[8] = <H|G>[*][@|+]
     * arg[9] = :<hopcount>
     * arg[10] = <SID> (since irc 2.11.2)
     * arg[11-n] = <real name>
     */

    ch=ME.findChannel(arg[3]);

    if(!ch)
      return;

    if(!(ch->me->flags & IS_OP))
      return;

    cu=ch->getUser(arg[7]);

    if(!cu || cu->flags & (HAS_V | HAS_O | IS_OP))
      return;

    /////////////////// ban SID

    size=sizeof(banned_sid_exceptions)/sizeof(banned_sid_exceptions[0]);

    if(size != 0) {
      bool do_punish=true;

      // ban all SIDs but sid_ban_exceptions
      for(idx=0; idx < size; idx++) {
        if(match(banned_sid_exceptions[idx], arg[10])) {
          do_punish=false;
          break;
        }
      }

      if(do_punish)
        __punish(ch, cu, SID_BAN_REASON, SID_BAN_TIME);
    }

    else {
      // ban only SIDs of banned_sid
      for(idx=0,size=sizeof(banned_sid)/sizeof(banned_sid[0]); idx < size; idx++) {
        if(match(banned_sid[idx], arg[10])) {
          __punish(ch, cu, SID_BAN_REASON, SID_BAN_TIME);
          return;
        }
      }
    }

    /////////////////// ban realname

    realname=srewind(data, 11);

    if(!realname)
      return;

    for(idx=0; idx < MAX_REALNAMES; idx++) {
      if(br_set.realname_pattern[idx].isDefault())
        continue;

      if(match(br_set.realname_pattern[idx], realname)) {
        __punish(ch, cu, BAD_REALNAME_BAN_REASON, BAD_REALNAME_BAN_TIME);
        return;
      } // match
    } // for
  } // 352
}

void __punish(chan *ch, chanuser *cu, const char *reason, time_t bantime)
{
  char buffer[MAX_LEN];

  snprintf(buffer, MAX_LEN, "*!*%s@%s", cu->ident, cu->host);

  if(set.BOTS_CAN_ADD_BANS
     && protmodelist::addBan(ch->name, buffer, "badrealname", bantime, reason))
          return;

  else
    ch->knockout(cu, reason, bantime);
}

void hook_timer()
{
  // lame
  std::string buffer;
  chan *ch;
  int count=0;

  if(penalty == 0 && NOW >= next_who) {

    for(ch=ME.first; ch; ch=ch->next) {
      if(!buffer.empty())
        buffer+=",";

      buffer+=(const char*) ch->name;
      count++;

      if(count == ME.server.isupport.max_who_targets) {
        net.irc.send("WHO %s", buffer.c_str());
        buffer.clear();
        count=0;
      }
    }

    if(count > 0)
      net.irc.send("WHO %s", buffer.c_str());

    next_who=NOW+WHO_DELAY;
  }
}

void hook_botnetcmd(const char *from, const char *cmd)
{
  char arg[10][MAX_LEN];

  str2words(arg[0], cmd, 10, MAX_LEN, 0);

  if(match(arg[1], "badrealname"))
  {
    if(br_set.parseUser(arg[0], arg[2], srewind(cmd, 3), "badrealname"))
      ; // save
  }
}

extern "C" module *init()
{
    module *m = new module("badrealname + SID ban (lame version)", "patrick", "0.1");
    next_who=NOW+300;
    m->hooks->rawirc=hook_raw;
    m->hooks->timer=hook_timer;
    m->hooks->botnetcmd=hook_botnetcmd;
    return m;
}

extern "C" void destroy()
{
}
