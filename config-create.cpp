/***************************************************************************
 *   Copyright (C) 2008 by Stefan Valouch                                  *
 *   stefanvalouch@googlemail.com                                          *
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

void expertMode();

/*! Reads a string from a terminal and sets a pstrings value to it.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param var The pstring to set.
 * \param defaultValue The default to use as pstring does not have that.
 * \return true if a value was set, even if it is \e defaultValue. false if not set (i.e. if no
 * default and nothing entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
bool readUserInput( const char *prompt, pstring<> &var, const char *defaultValue )
{
	string buf;
	do
	{
		if(strlen(defaultValue))
		{
			printPrompt( "%s [%s]: ", prompt, defaultValue );
		}
		else
		{
			printPrompt( "%s: ", prompt );
		}
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() >= 1)
		{
			var = buf.c_str();
			return true;
		}
		else if (strlen(defaultValue))
		{
			var = defaultValue;
			return true;
		}
		else
		{
			continue;
		}
	}
	while( true );
}

/*! Reads a boolean value from a terminal and sets an entities value to that value.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \param force Wether or not to repeat the question till valid data is entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entBool &entity, bool force )
{
	string buf;
	options::event *e;
	do
	{
		printPrompt( "%s (ON or OFF) [%s]: ", prompt, (entity.defaultValue ? "ON" : "OFF") );
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() >= 1)
		{
			e = entity.setValue(entity.name, buf.c_str());
			if(e->ok)
			{
				break;
			}
			else if(!force)
			{
				entity.setValue(entity.name, itoa(entity.defaultValue));
				break;
			}
			else
			{
				printError( (const char*)e->reason );
			}
		}
	}
	while( force );
	//return false;
}

/*! Reads an integer value from stdin and sets an entities value to it.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \param force Wether or not to repeat the question till valid data is entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entInt &entity, bool force )
{
	string buf;
	options::event *e;

	do
	{
		if( entity.defaultValue )
		{
			printPrompt( "%s (%d to %d) [%d]: ", prompt, entity.min, entity.max, entity.defaultValue );
		}
		else
		{
			printPrompt( "%s (%d to %d): ", prompt, entity.min, entity.max );
		}
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 1)
		{
			if(force)
			{
				printError( "You have to enter something here" );
				continue;
			}
			else
			{
				entity.setValue(entity.name, itoa(entity.defaultValue));
				break;
			}
		}
		e = entity.setValue(entity.name, buf.c_str());
		if(e->ok)
		{
			break;
		}
		else
		{
			printError( (const char*)e->reason );
		}
	}
	while ( true );
}

/*! Read a host address from stdin and set an entities value to it.
 * \overload
 * \param prompt The question to ask for the value.
 * \param entity The entits.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entHost &entity, bool allowEmpty )
{
	string buf;
	do
	{
		if(strcmp((const char*)entity.ip, "0.0.0.0"))
		{
			printPrompt( "%s [%s]: ", prompt, (const char*)entity.ip );
		}
		else
		{
			printPrompt( "%s: ", prompt );
		}
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if( allowEmpty && buf.empty() )
		{
			break;
		}
		options::event *e = entity.setValue(entity.name, buf.c_str());
		if(!e->ok)
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
	}
	while( true );
}

/*! Reads an IP Port Passwort Handle combination from stdin and sets an entities value to it.
 * \todo Implement checks!!!
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entHub &entity )
{
	char buf[MAX_LEN];
	options::event *e;
	entHost host = entHost( "tmp", entHost::ipv4 | entHost::ipv6 | entHost::domain | entHost::use_ssl);
	entInt     port   = entInt( "tmp", 1, 65535, 0 );
	entMD5Hash pass   = entMD5Hash( "tmp" );
	entWord    handle = entWord( "tmp", 0, 15 );

	printMessage( "%s: ", prompt );
	do
	{
		readUserInput( "\tHost address (can be prefixed by \"ssl:\", \"ipv4:\", \"ipv6:\")", host );
		readUserInput( "\tPort", port );
		readUserInput( "\tPassword", pass );
		readUserInput( "\tHandle", handle, true );

		snprintf( buf, MAX_LEN, "%s %s %s %s", (const char*)host.getValue(), port.getValue(), pass.getValue(), (const char*)handle.getValue() );
		e = entity.setValue( entity.name, buf );

		if( !e->ok )
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
	}
	while( true );
}

/*! Reads a md5 string from stdin.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entMD5Hash &entity )
{
	string buf;
	options::event *e;
	do
	{
		printPrompt( "%s: ", prompt );
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 8) // password length okay?
		{
			printError( "Passwords should have a length of at least 8 characters!" );
		}
		else // okay, throw it in
		{
			e = entity.setValue(entity.name, buf.c_str());
			if(e->ok)
			{
				break;
			}
			else
			{
				printError( (const char*)e->reason );
			}
		}
	}
	while( true );
}

/*! Asks a user for information to build an entServer object.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entServer &entity )
{
	string buf;
	options::event *e;
	char tmp[MAX_LEN];

	entHost host = entHost( "tmp", entHost::ipv4 | entHost::ipv6 | entHost::domain
#ifdef HAVE_SSL
| entHost::use_ssl
#endif
);
	entInt  port = entInt( "tmp", 1, 65535, 0 );
	entWord pass = entWord( "tmp", 0, 256, "" );

	printMessage( "%s: ", prompt );
	do
	{
		readUserInput( "\tHost address (can be prefixed by \"ssl:\", \"ipv4:\", \"ipv6:\")", host );
		readUserInput( "\tPort number", port );
		readUserInput( "\tPassword", pass, true );

		snprintf(tmp, MAX_LEN, "%s %s %s", (const char*)host.getValue(), port.getValue(), (const char*)pass.string);
		e = entity.setValue(entity.name, tmp);

		if( !e->ok )
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
			
	}
	while( true );
}

/*! Reads a string value from stdin and sets an entities value to it.
 * If the \e entity has no default value, the user is forced to enter something.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entString &entity, bool allowEmpty )
{
	string buf;
	options::event *e;
	do
	{
		if(entity.defaultString)
		{
			printPrompt( "%s (%d to %d chars) [%s]: ", prompt, entity.min, entity.max,
				(const char*)entity.defaultString );
		}
		else
		{
			printPrompt( "%s (%d to %d chars): ", prompt, entity.min, entity.max );
		}
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 1)
		{
			if(entity.defaultString)
			{
				entity.setValue(entity.name, entity.defaultString);
				break;
			}

			else if(allowEmpty)
				break;
			else
				continue;
		}
		e = entity.setValue(entity.name, buf.c_str());
		if(e->ok)
		{
			break;
		}
		else
		{
			printError( (const char*)e->reason );
		}
	}
	while( true );
}

/** missing.
 *
 * \author patrick <patrick@psotnic.com>
 */

void readUserInput(const char *prompt, entListener &entity)
{
  options::event *e;
  char buf[MAX_LEN];
  entHost host=entHost("tmp", entHost::ipv4 | entHost::ipv6 | entHost::use_ssl);
  entInt port=entInt("tmp", 1, 65535, 0);
  entWord options=entWord("tmp", 0, 15);

  printMessage("%s: ", prompt);

  do {
    readUserInput("\tIP address (can be prefixed by \"ssl:\")", host);
    readUserInput("\tPort number", port);

    do {
    readUserInput("\tusage (\"all\", \"users\", \"bots\")", options);
    } while(strcmp(options, "all") && strcmp(options, "users") && strcmp(options, "bots"));

    snprintf(buf, MAX_LEN, "%s %s %s", (const char*)host.getValue(), port.getValue(), (const char*)options.string);
    e=entity.setValue(entity.name, buf);

    if(!e->ok)
      printError((const char*)e->reason);

    else {
      break;
    }

  } while(true);
}

/*! Asks a multiple choice question and returns the answer-id.
 * \note Be sure that \e defChoice is whithin the range of \e choices.
 * \param prompt The question to ask.
 * \param choices A descriptive text to make the decision easier.
 * \param defChoice The ID of the default value.
 * \return The selected ID or the default if nothing (valid) was entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
int readUserMC( const char *prompt, const char *choices[], size_t len, unsigned int defChoice )
{
	string buf;
	printMessage( "%s:", prompt );
	for( unsigned int i = 0; i < len; i++ )
	{
		if(i == defChoice)
		{
			printItem( "\t[%d]\t%s", i, choices[i] );
		}
		else
		{
			printItem( "\t %d \t%s", i, choices[i] );
		}
	}

	do
	{
		printPrompt( "Your choice: " );
		getline(cin, buf);
	
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 1)
		{
			return defChoice;
		}
		else
		{
			unsigned int inp = atoi(buf.c_str());
			if( inp > len-1 )
			{
				printError( "Please choose a valid value!" );
				continue;
			}

			for( unsigned int i = 0; i < sizeof(choices); i++ )
			{
				if(i == inp)
					return inp;
			}
			return defChoice;
		}
	}
	while( true );
}

/*! Asks the user a simple Yes/No question.
 * \param prompt The question.
 * \param defaultValue The default value.
 * \return true if the user entered yes, false if no.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
bool readUserYesNo( const char *prompt, bool defaultValue )
{
	string buf;
	printPrompt( "%s (Y or N) [%s]: ", prompt, (defaultValue ? "Y" : "N") );
	getline(cin, buf);
	if(buf.size() < 1)
	{
		return defaultValue;
	}
	if(buf == "Y" || buf == "y" || buf == "Yes" || buf == "yes" || buf == "1")
	{
		return true;
	}
	return false;
}

/*! Interactive generator for config files.
 * This function asks the user for the data and generates the initial config without needing to have
 * the unencrypted config file laying around waiting for encryption.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void createInitialConfig(bool expMode)
{
	int n;
	options::event *e;

	printMessage( "Bot now runs in config creation mode" );
	printMessage( "The options are explained at http://www.psotnic.com/documentation/config-file" );

    if(expMode)
    {
	expertMode();
    }

    else
    {
	printMessage( "Mandatory options for all bot types:" );
	//readUserInput( "Which nick should the bot use?", config.nick );
	readUserInput( "nick?", config.nick );
	//readUserInput( "What is the bot's username?", config.ident );
	readUserInput( "username?", config.ident );
	//readUserInput( "What should be used as the bot's realname?", config.realname );
	readUserInput( "realname?", config.realname );
	//readUserInput( "Which characters should get appended to the nick if it is already taken?", config.nickappend );
	//readUserInput( "Nick to use when the normal nick is already taken:", config.altnick );
	readUserInput( "alternative nick?", config.altnick );
        readUserInput( "Which IPv4 or IPv6 address should be used to connect to IRC (default: determined by OS)", config.vhost, true );
	config.myipv4.setValue("myipv4", config.vhost);
	readUserInput( "Which IPv4 address should be used to connect to other bots?", config.myipv4, true );

	const char *choices[3] = {
		"Main",
		"Slave",
		"Leaf"
	};
	int bottype = readUserMC( "What type of bot do you want?", choices, 3, 0 );

	switch( bottype )
	{
		case 0:
			printMessage( "Configuration for bot type: MAIN" );
			config.bottype = BOT_MAIN;

			n=0;
			do
			{
				readUserInput("What ports should the bot listen on?", config.listenport[n]);
				n++;
			} while(n < MAX_LISTENPORTS && readUserYesNo("Do you want to add more ports?", false));

			n=0;
			do
			{
				readUserInput("ownerpass?", config.ownerpass[n]);
				n++;
			} while(n < MAX_OWNERPASSES && readUserYesNo("Do you want to add more ownerpasses?", false));
			
			break;
		case 1:
			printMessage( "Configuration for bot type: SLAVE" );
			config.bottype = BOT_SLAVE;

			n=0;
			do
                        {
                                readUserInput("What ports should the bot listen on?", config.listenport[n]);
                                n++;
                        } while(n < MAX_LISTENPORTS && readUserYesNo("Do you want to add more ports?", false));

			readUserInput( "Please enter the connection options for connecting to the main bot.", config.hub );
			
			break;
		case 2:
			printMessage( "Configuration for bot type: LEAF" );
			config.bottype = BOT_LEAF;
			readUserInput( "Connection options to connect to hub?", config.hub );
			break;
		default:
			printError( "ERROR: unknown bot type: " + bottype );
			exit(1);
			break;
	}

	string buf;

	// XXX: move to expert mode? -- patrick
	// we don't make a function for that entMult stuff, its easier this way!
	if( config.bottype == BOT_LEAF )
	{
		if( readUserYesNo( "Do you want to add alternative hubs?", false ) )
		{
			printMessage( "Valid lines are: host port" );
			for( n = 0; n < MAX_ALTS; )
			{
				printPrompt( "\tAltHub %d: ", n );
				getline(cin, buf);
				if(!cin.good())
				{
					cout << endl;
					exit(1);
				}
				if( buf.size() < 1 )
				{
					break;
				}
				e = config.alt[n].setValue( config.alt[n].name, buf.c_str() );
				if( !e->ok )
				{
					printError( (const char*)e->reason );
					continue;
				}
				n++;
			}
			printMessage( "Okay, added %d alternative hubs.", n );
		}
	}

	if( readUserYesNo( "Do you want to add some servers?", true ) )
	{
		n=0;

		do
		{
			readUserInput("Adding server", config.server[n]);
			n++;
		} while(n < MAX_SERVERS && readUserYesNo("Do you want to add more servers?", false));
	}

	if( readUserYesNo( "Do you want to add some modules?", false ) )
	{
		for( n = 0; n < MAX_MODULES; )
		{
			printPrompt( "\tModule %d: ", n );
			getline(cin, buf);
			if(!cin.good())
			{
				cout << endl;
				exit(1);
			}
			if(buf.size() < 1)
			{
				break;
			}
			e = config.module_load[n].setValue(config.module_load[n].name, buf.c_str());
			if(!e->ok)
			{
				printError( (const char*)e->reason );
				continue;
			}
			n++;
		}
		printMessage( "Okay, added %d modules.", n );
	}

/*	if( readUserYesNo( "Do you want to set up the optional stuff?", false ) )
	{
		printMessage( "Detailed configuration" );

		config.handle.defaultString = config.nick.string;
		readUserInput( "What is the bot's botnet handle?", config.handle );

        // FIXME: not understandable for noobs.
        //         myipv4 will be used as source ip for linking and dcc chat and everything else
        //         but not IRC (-> vhost) and not for listen (ip is defined in listen line)
                readUserInput( "Enter the bot's IPv4 address, use 0.0.0.0 for any", config.myipv4 );
		readUserInput( "Enter the bot's IPv6 address, use :: for any", config.myipv6);

		config.userlist_file.defaultString = config.nick.string + ".ul";
		readUserInput( "Where should the bot place its userlist?", config.userlist_file );

		readUserInput( "Which ctcp version do you want? 0 = none, 1 = psotnic, 2 = irssi, 3 = epic, 4 = lice, 5 = bitchx, 6 = dzony loker, 7 = luzik, 8 = mirc 6.14", config.ctcptype );

		readUserInput( "Keepnick?", config.keepnick );
	}
*/
	if(readUserYesNo("Do you want to switch to expert mode and set further variables?", false))
	{
		expertMode();
	}
    }

        // find out bottype again
	bool listening=false;

        for(int i=0; i<MAX_LISTENPORTS; i++)
        {
                if(config.listenport[i].isDefault())
                        continue;

                listening=true;
		break;
        }

        if(listening)
        {
                if(config.hub.getPort())
                        config.bottype = BOT_SLAVE;
                else
                        config.bottype = BOT_MAIN;
        }

        else
                config.bottype = BOT_LEAF;

	string defaultcfgfile = config.nick.getValue() + string(".cfg");
	readUserInput( "Where should the config file be saved?", config.file, defaultcfgfile.c_str());

	e = config.save();
	if (!e->ok)
	{
		printError( (const char*)e->reason );
		exit(1);
	}
	printMessage( (const char*)e->reason );

	if(config.bottype == BOT_MAIN && readUserYesNo("Do you want to create the userlist?", true)) // FIXME: alt, check if ul exists
	{
		HANDLE *h;
		entWord handle, pass;
		config.polish();
		readUserInput("\tusername?", handle);
		readUserInput("\tpassword?", pass);

		userlist.addHandle("idiots", 0, 0, 0, 0, config.handle);
		userlist.first->flags[MAX_CHANNELS] = HAS_D;

		if(config.bottype == BOT_MAIN)
			userlist.addHandle(config.handle, 0, B_FLAGS | HAS_H, 0, 0, config.handle);

		h = userlist.addHandle((const char *) handle, 0, 0, 0, 0, (const char *) handle);

		if(h)
		{
			userlist.changePass(h->name, (char *) (const char *) pass);
			userlist.changeFlags(h->name, "tpx", "");
			userlist.updated(false);
			userlist.save(config.userlist_file);
			printMessage("userlist saved as %s", (const char *) config.userlist_file);
		}
	}

	exit(0);
}

/** missing.
 * \author patrick <patrick@psotnic.com>
 */

void expertMode()
{
  string buf, var, val;
  char buf2[MAX_LEN], arg[2][MAX_LEN];

  printMessage("commands:");
  printMessage("set [<variable> [<value>]]");
  printMessage("exit");
  printMessage("");
  printMessage("set - shows all variables and their values");
  printMessage("set nick - shows value of variables starting with \"nick\"");
  printMessage("set nick gay-psotnic - sets variable \"nick\" to value \"gay-psotnic\"");

  while(1) {
    getline(cin, buf);

    if(!cin.good()) {
      cout << endl;
      exit(1);
    }

    strncpy(buf2, buf.c_str(), MAX_LEN);
    str2words(arg[0], buf2, 2, MAX_LEN);

    if(!strcmp(arg[0], "set"))
      config.parseUserSTDout(arg[1], srewind(buf2, 2), "cfg");

    else if(!strcmp(arg[0], "exit"))
      return;
  }
}
