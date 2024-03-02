/* oidentd spoofing module for psotnic
 *
 * This module uses the local port of a connecting bot to assign its ident in oidentd.conf.
 * Even if multiple bots connect at the same time each should get the ident set in "ident" variable.
*/

// - configuration -

// define this, if your oidentd config is not located in $HOME/.oidentd.conf
//#define OS_CONFIG "/home/patrick/.oidentd.conf"

// reconnect if ident set in config and the one the bot actually got are not the same
//#define OS_RECONNECT

// - end -

#include <prots.h>
#include <global-var.h>

char oidentd_cfg[MAX_LEN];

void oidentd_spoofing();

void hook_connecting()
{
  oidentd_spoofing();
}

void oidentd_spoofing()
{
  FILE *fhr, *fhw;
  char buffer[MAX_LEN], *lport=net.irc.getMyPortName(), oidentd_cfg_tmp[MAX_LEN], *p, *p2;
  int len;

  snprintf(oidentd_cfg_tmp, MAX_LEN, "%s.tmp", oidentd_cfg);

  if(!lport || !strcmp(lport, "0")) {
    net.send(HAS_N, "[oidentd] getMyPortName() failed. oidentd spoofing will not work.");
    return;
  }

  fhw=fopen(oidentd_cfg_tmp, "w");

  if(!fhw) {
    net.send(HAS_N, "[oidentd] cannot open '%s'. oidentd spoofing will not work.", oidentd_cfg);
    return;
  }

  fhr=fopen(oidentd_cfg, "r");

  if(fhr) {
    // search for lines in oidentd.conf that can be removed
    while(fgets(buffer, MAX_LEN, fhr)) {
      len=strlen(buffer);

      if(buffer[len-1] == '\n') {
        buffer[len-1]='\0';
        len--;
      }

      if(len != 0) {
        p=strstr(buffer, "# psotnic ");

        if(p) {
          p=p+10;

          p2=strchr(p, ' ');

          if(p2) {
            *p2='\0';
            p2++;

            // my previous entry
            if(!strcasecmp(p, config.handle))
              continue;

            // an obsolete entry created by this function
            if(*p2 && (time(NULL) - atol(p2) > 3600))
              continue;

            // restore string because the line will not be removed
            *p2--;
            *p2=' ';
          }
        }

        fprintf(fhw, "%s\n", buffer);
      }
    }

    fclose(fhr);
  }

  fprintf(fhw, "lport %s { reply \"%s\" } # psotnic %s %ld\n", lport, (const char *) config.ident, (const char *) config.handle, time(NULL));
  fclose(fhw);
  unlink(oidentd_cfg);
  rename(oidentd_cfg_tmp, oidentd_cfg);
}

#ifdef OS_RECONNECT
void hook_connected()
{
  if(!match(ME.ident, config.ident)) {
    net.send(HAS_N, "[oidentd] I did not get the ident I wanted. reconnecting..");
    net.irc.send("QUIT :reconnecting");
    net.irc.close("reconnecting");
    ME.nextConnToIrc=NOW+set.IRC_CONN_DELAY;
  }
}
#endif

extern "C" module *init()
{
  module *m=new module("oidentd spoofing", "patrick <patrick@psotnic.com>", "0.2");

#ifdef OS_CONFIG
  strncpy(oidentd_cfg, OS_CONFIG, MAX_LEN);
#else
  struct passwd *pw;

  if(!(pw=getpwuid(getuid()))) {
    net.send(HAS_N, "[oidentd] getpwuid() failed. oidentd spoofing will not work"); 
    return m;
  }

  snprintf(oidentd_cfg, MAX_LEN, "%s/.oidentd.conf", pw->pw_dir);
#endif
  m->hooks->connecting=hook_connecting;
#ifdef OS_RECONNECT
  m->hooks->connected=hook_connected;
#endif

  return m;
}

extern "C" void destroy()
{
}
