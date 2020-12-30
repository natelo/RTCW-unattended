/*
 * g_admin.c
 *
 * L0 - Admin handling
 * Created: 5th.Sept/13
 * Updated:  
 */
#include "g_local.h"

/*
===========
Sort tag
===========
*/
char *sortTag(gentity_t *ent) {
	char n1[MAX_STRING_TOKENS]; // Nobo - this needs to be atleast 256.. matching vmCvar's string (otherwise crash if > 36(previous size))

	if (ent->client->sess.admin == ADM_1)
		SanitizeString(a1_tag.string, n1, qtrue);
	else if (ent->client->sess.admin == ADM_2)
		SanitizeString(a2_tag.string, n1, qtrue);
	else if (ent->client->sess.admin == ADM_3)
		SanitizeString(a3_tag.string, n1, qtrue);
	else if (ent->client->sess.admin == ADM_4)
		SanitizeString(a4_tag.string, n1, qtrue);
	else if (ent->client->sess.admin == ADM_5)
		SanitizeString(a5_tag.string, n1, qtrue);
	else
		Q_strncpyz(n1, "", sizeof(n1));

	// Has to be done twice to cope with double carrots..
	Q_CleanStr(n1);
	//n2[10] = 0; Nobo - It may look nicer, but the size should be left to the descretion of the server owner.

	return va("%s", n1);
}

/*
===========
Login
===========
*/
void cmd_do_login (gentity_t *ent, qboolean silent) { 
	char str[MAX_TOKEN_CHARS];
	qboolean error;
	char *log;

	error = qfalse;
	trap_Argv( 1, str, sizeof( str ) );

	// Make sure user is not already logged in.
	if (!ent->client->sess.admin == ADM_NONE ) 
	{		
		CP("print \"You are already logged in^1!\n\"");
	return;	
	}	
	// Prevent bogus logins	
	if (( !Q_stricmp( str, "\0")) || ( !Q_stricmp( str, "")) || ( !Q_stricmp( str, "\"")) || ( !Q_stricmp( str, "none")) ) {		
		CP("print \"Incorrect password^1!\n\"");
		// No log here to avoid login by error..
	return;
	}

		// Else let's see if there's a password match.
		if ( (Q_stricmp(str, a1_pass.string) == 0) 
			|| (Q_stricmp(str, a2_pass.string) == 0) 
			|| (Q_stricmp(str, a3_pass.string) == 0) 
			|| (Q_stricmp(str, a4_pass.string) == 0) 
			|| (Q_stricmp(str, a5_pass.string) == 0) ) 
		{	
			// Always start with lower level as if owner screws it up and sets the same passes for more levels, the lowest is the safest bet.
			if (Q_stricmp(str, a1_pass.string) == 0) {
				ent->client->sess.admin = ADM_1;			
			} else if (Q_stricmp(str, a2_pass.string) == 0) {
				ent->client->sess.admin = ADM_2;			
			} else if (Q_stricmp(str, a3_pass.string) == 0) {
				ent->client->sess.admin = ADM_3;			
			} else if (Q_stricmp(str, a4_pass.string) == 0) {
				ent->client->sess.admin = ADM_4;			
			} else if (Q_stricmp(str, a5_pass.string) == 0) {
				ent->client->sess.admin = ADM_5;			
			} else {
				error = qtrue;
			} 
			// Something went to hell..
			if (error == qtrue) {
				// User shouldn't be anything but regular so make sure..
				ent->client->sess.admin = ADM_NONE;						
					CP("print \"Error has occured while trying to log you in^1!\n\"");
			return;
			}

			// We came so far so go with it..
			if (silent) {						
				CP("print \"Silent Login successful^2!\n\"");
				ent->client->sess.incognito = 1; // Hide them

				// Log it
				log =va("Player %s (IP:%i.%i.%i.%i) has silently logged in as %s.", 
				ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
				ent->client->sess.ip[3], sortTag(ent));
				logEntry (ADMLOG, log);
			} else {					
				AP(va("chat \"console:^7 %s ^7has logged in as %s^3!\n\"", ent->client->pers.netname, sortTag(ent)));

				// Log it
				log =va("Player %s (IP:%i.%i.%i.%i) has logged in as %s.", 
				ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
				ent->client->sess.ip[3], sortTag(ent));
				logEntry (ADMLOG, log);
			}

			// Make sure logged in user bypasses any spec lock instantly.
			ent->client->sess.specInvited = 3;

			return;
		// No match..
		} else {			
			CP("print \"Incorrect password^1!\n\"");

			// Log it
			log =va("Player %s (IP:%i.%i.%i.%i) has tried to login using password: %s", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], str);
			logEntry (PASSLOG, log);
		return;
		}
}

/*
===========
Logout
===========
*/
void cmd_do_logout(gentity_t *ent) {
	// If user is not logged in do nothing
	if (ent->client->sess.admin == ADM_NONE) {
		return;
	} else {
		// Admin is hidden so don't print 
		if (ent->client->sess.incognito) 			
			CP("print \"You have successfully logged out^3!\n\"");
		else			
			AP(va("chat \"console:^7 %s ^7has logged out.\n\"", ent->client->pers.netname));

		// Log them out now
		ent->client->sess.admin = ADM_NONE;

		// Set incognito to visible..
		ent->client->sess.incognito = 0;

		// Clear speclock		
		ent->client->sess.specInvited = 0;

		// Black out client if needed
		G_setClientSpeclock(ent);

	return;
	}
}

/*********************************** FUNCTIONALITY ************************************/

/*
===========
Time for log, getstatus..
===========
*/
extern int trap_RealTime ( qtime_t * qtime );
const char *cMonths[12] = {
"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*
===========
Log Admin related stuff
===========
*/
void logEntry (char *filename, char *info)
{	
	fileHandle_t	f;
	char *varLine;
	qtime_t		ct;
	trap_RealTime( &ct );	

	if (!g_extendedLog.integer)
		return;
 
	strcat (info, "\r");
	trap_FS_FOpenFile( filename, &f, FS_APPEND);	

	varLine=va("Time: %02d:%02d:%02d/%02d %s %d : %s \n",
				ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday,
				cMonths[ct.tm_mon], 1900+ct.tm_year, info);

	trap_FS_Write(varLine, strlen( varLine ), f);		
	trap_FS_FCloseFile(f);
return;
}

/*
===========
Get client number from name
===========
*/
int ClientNumberFromNameMatch( char *name, int *matches ) {
	int i, textLen;
	char nm[32];
	char c;
	int index = 0;

	Q_strncpyz(nm, name, sizeof(nm));	
	Q_CleanStr(nm);						
	textLen = strlen(nm);
	c = *nm;							

	for (i = 0; i < level.maxclients; i++)	
	{
		int j, len;
		char playerName[32];

		if ((!g_entities[i].client) || (g_entities[i].client->pers.connected != CON_CONNECTED) )
			continue;
		
		Q_strncpyz(playerName, g_entities[i].client->pers.netname, sizeof(playerName));	
		Q_CleanStr(playerName);		
		len = strlen(playerName);

		for (j = 0; j < len; j++)	
		{
			if (tolower(c) == tolower(playerName[j]))	
			{			
				if (!Q_stricmpn(nm, playerName+j, textLen))	
				{
					matches[index] = i;		
					index++;				
					break;					
				}
			}
		}
	}
return index;	
}

/*
===========
Parse string (if I recall right this bit is from S4NDMoD)
===========
*/
void ParseAdmStr(const char *strInput, char *strCmd, char *strArgs)
{
	int i = 0, j=0;
	int foundspace=0;

	while(strInput[i] != 0){
		if(!foundspace){
			if(strInput[i] == ' '){
				foundspace = 1;
				strCmd[i]=0;
			}else
				strCmd[i]=strInput[i];
			i++;
		}else{ 
			strArgs[j++]=strInput[i++];
		}
	}
	if(!foundspace)
		strCmd[i]=0;

strArgs[j]=0;
}


/*
===========
Deals with customm commands
===========
*/
void cmdCustom(gentity_t *ent, char *cmd) {
	char *tag, *log;	

	tag = sortTag(ent);

	if (!strcmp(ent->client->pers.cmd2,"")) {		
		CP(va("print \"Command ^1%s ^7must have a value^1!\n\"", cmd));
	return; 
	} else {
		// Rconpasswords or sensitve commands can be changed without public print..
		if (!strcmp(ent->client->pers.cmd3,"@"))  			
			CP(va("print \"Info: ^2%s ^7was silently changed to ^2%s^3!\n\"", cmd, ent->client->pers.cmd2));
		else			
			AP(va("chat \"console:^7 %s ^7changed ^3%s ^7to ^3%s %s\n\"", tag, cmd, ent->client->pers.cmd2, ent->client->pers.cmd3));
		// Change the stuff
		trap_SendConsoleCommand( EXEC_APPEND, va("%s %s %s", cmd, ent->client->pers.cmd2, ent->client->pers.cmd3));	
		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) has changed %s to %s %s.", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], cmd, ent->client->pers.cmd2, ent->client->pers.cmd3);
		logEntry (ADMACT, log);
	return;
	}
}

/*
===========
Can't use command msg..
===========
*/
void cantUse(gentity_t *ent) {
	char alt[128];
	char cmd[128];

	parseCmds(ent->client->pers.cmd1, alt, cmd, qfalse);
	
	CP(va("print \"Command ^3%s ^7is not allowed for your level^3!\n\"", cmd));
return;
}

/*
===========
Determine if admin level allows command
===========
*/
qboolean canUse(gentity_t *ent, qboolean isCmd) {
	char *permission="";
	char *token, *parse;
	char alt[128];
	char cmd[128];

	switch (ent->client->sess.admin) {
		case ADM_NONE:// So linux stops complaining..
			return qfalse;
		break;
		case ADM_1:
			permission = a1_cmds.string;
		break;
		case ADM_2:
			permission = a2_cmds.string;
		break;
		case ADM_3:
			permission = a3_cmds.string;
		break;
		case ADM_4:
			permission = a4_cmds.string;
		break;
		case ADM_5:
			if (a5_allowAll.integer && isCmd) // Return true if allowAll is enabled and is a command.
				return qtrue;
			else
				permission = a5_cmds.string;  // Otherwise just loop thru string and see if there's a match.
		break;			
	}

	parseCmds(ent->client->pers.cmd1, alt, cmd, qfalse);
			
	if (strlen(permission)) {
		parse = permission;
		while (1) {
			token = COM_Parse(&parse);
			if (!token || !token[0])		
				break;

			if (!Q_stricmp(cmd, token))	{				
					return qtrue;		
			}	
		}
		return qfalse;
	}
return qfalse;
}

/*
===========
Check if string matches IP pattern
===========
*/
void flip_it (char *s, char in, char out) {
	while (*s != 0) {
		if (*s == in)
			*s = out;
		s++;
	}
}
// It's not perfect but it helps..
qboolean IPv4Valid( char *s)
{
    int c, i, len = strlen(s);
	unsigned int d[4];
	char vrfy[16];		

    if (len < 7 || len > 15)
        return qfalse;
    
    vrfy[0] = 0;		
	flip_it(s, '*', (char)256);
    
    c = sscanf( s, "%3u.%3u.%3u.%3u%s", 
				&d[0], &d[1], &d[2], &d[3], vrfy);

    if (c != 4 || vrfy[0])
        return qfalse;

    for (i = 0; i < 4; i++)
        if (d[i] > 256)
            return qfalse;

	flip_it(s, (char)256, '*');

return qtrue;
}

/*********************************** COMMANDS ************************************/

/*
===========
Toggle incognito
===========
*/
void cmd_incognito(gentity_t *ent) {
	if (ent->client->sess.admin == ADM_NONE)
		return;

	if (ent->client->sess.incognito == 0) {
		ent->client->sess.incognito = 1;			
			CP("cp \"You are now incognito^3!\n\"2");
	return;
	} else {
		ent->client->sess.incognito = 0;				
			CP("cp \"Your status is now set to visible^3!\n\"2");
	return;
	}
}

/*
===========
Kick player + optional <msg>
===========
*/
void cmd_kick(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0) {			
		CP("print \"Client not on server^1!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	} 
		
	for (i = 0; i < count; i++) {
		trap_DropClient( nums[i], va( "^3kicked by ^3%s \n^7%s", tag, ent->client->pers.cmd3));			
		AP(va("chat \"console:^7 %s has kicked player %s^7! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) has kicked user %s. %s", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3);
		logEntry (ADMACT, log);
	}

	return;
}

/*
===========
Kick player based upon clientnumber + optional <msg>
===========
*/
void cmd_clientkick( gentity_t *ent) {
	int	player_id;
	gentity_t	*targetclient;	
	char *tag, *log;

	tag = sortTag(ent);	
	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );

	if ( player_id == -1 ) { 
		return;
	}

	targetclient = g_entities + player_id; 

	//kick the client
	trap_DropClient( player_id, va( "^3kicked by ^3%s \n^7%s", tag, ent->client->pers.cmd3));	
	AP(va("chat \"console:^7 %s has kicked player %s^7! ^3%s\n\"", tag, targetclient->client->pers.netname, ent->client->pers.cmd3));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has clientKicked user %s. %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], targetclient->client->pers.netname,ent->client->pers.cmd3);
	logEntry (ADMACT, log);

return;
}

/*
===========
Slap player
===========
*/
void cmd_slap(gentity_t *ent)
{
	int clientid;
	int damagetodo;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	// Sort log
	log =va("Player %s (IP:%i.%i.%i.%i) has slapped ", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3]);

	clientid = atoi(ent->client->pers.cmd2);
	damagetodo = 20; // do 20 damage

	if ((clientid < 0) || (clientid >= MAX_CLIENTS) )
	{
		CP("print \"Invalid client number^3!\n\"");
		return;
	}

	if ((!g_entities[clientid].client) || (level.clients[clientid].pers.connected != CON_CONNECTED))
	{
		CP("print \"Invalid client number^3!\n\"");
		return;
	}

	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {		
		CP("print \"You cannot slap a spectator^3!\n\"");
	return;
	}
	
	ent = &g_entities[clientid];

	if (ent->client->ps.stats[STAT_HEALTH] <= 20) {
		G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMIN);		
		AP(va("chat \"console:^7 %s ^7was slapped to death by %s^3!\n\"", ent->client->pers.netname, tag));
		player_die( ent, ent, ent, 100000, MOD_ADMIN );
	
		// Log it
		log2 =va("%s to death player %s.", log, ent->client->pers.netname);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log2);

		return;
	} else {
		G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMIN);		
		AP(va("chat \"console:^7 %s ^7was slapped by %s^3!\n\"", ent->client->pers.netname, tag));
		// it's broadcasted globaly but only to near by players	
		G_AddEvent(ent, EV_GENERAL_SOUND, G_SoundIndex("sound/multiplayer/vo_revive.wav")); // L0 - TODO: Add sound in pack...
		
		// Log it
		log2 =va("%s player %s.", log, ent->client->pers.netname);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log2);

		return;
	}		
}

/*
===========
Kill player
===========
*/
void cmd_kill(gentity_t *ent)
{
	int clientid;
	int damagetodo;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	// Sort log
	log =va("Player %s (IP:%i.%i.%i.%i) has killed ", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3]);

	clientid = atoi(ent->client->pers.cmd2);
	damagetodo = 250; // Kill the player on spot

	if ((clientid < 0) || (clientid >= MAX_CLIENTS))
	{
		CP("print \"Invalid client number^3!\n\"");
		return;
	}

	if ((!g_entities[clientid].client) || (level.clients[clientid].pers.connected != CON_CONNECTED))
	{		
		CP("print \"Invalid client number^3!\n\"");
		return;
	}

	if (!g_entities[clientid].client->ps.stats[STAT_HEALTH] > 0) {		
		CP("print \"Player is already dead^3!\n\"");
	return;
	}

	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {		
		CP("print \"You cannot kill a spectator^3!\n\"");
	return;
	}
	
	ent = &g_entities[clientid];
	G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMIN);	
	AP(va("chat \"console:^7 %s ^7was killed by %s^3!\n\"", ent->client->pers.netname, tag));
	player_die( ent, ent, ent, 100000, MOD_ADMIN );
	
	// Log it
	log2 =va("%s user %s.", log, ent->client->pers.netname);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log2);

	return;	
}

/*
===========
Flip client's view
===========
*/
void cmd_fling(gentity_t *ent, int type) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0) {
		CP("print \"Client not on server^1!\n\"");
		return;
	}
	else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	}

	for (i = 0; i < count; i++) {
		vec3_t dir, flingvec;
		char *action = (!type ? "fling" : (type == 1 ? "throw" : "launch"));
		
		if (g_entities[nums[i]].health <= 0)
			return;
		
		// Fling
		if (type == 0) {
			VectorSet(dir, crandom() * 50, crandom() * 50, 10);
		}
		// Throw
		else if (type == 1) {
			AngleVectors(g_entities[nums[i]].client->ps.viewangles, dir, NULL, NULL);
			dir[2] = .25f;
		}
		// Launch
		else {	
			VectorSet(dir, 0, 0, 10);
		}

		VectorNormalize(dir);
		VectorScale(dir, 1500, flingvec);
		VectorAdd(g_entities[nums[i]].s.pos.trDelta, flingvec, g_entities[nums[i]].s.pos.trDelta);
		VectorAdd(g_entities[nums[i]].client->ps.velocity,
			flingvec,
			g_entities[nums[i]].client->ps.velocity);

		AP(va("chat \"console: ^7%s used ^3%s ^7on a %s^7.\n\"", tag, action, g_entities[nums[i]].client->pers.netname));

		// Log it
		log = va("Player %s (IP:%i.%i.%i.%i) used %s on a %s.",
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
			ent->client->sess.ip[3], action, g_entities[nums[i]].client->pers.netname);
		logEntry(ADMACT, log);
	}
}

/*
===========
Force user to spectators
===========
*/
void cmd_specs(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);		
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0){
		CP("print \"Client not on server^3!\n\"");
		return;
	} else if (count > 1) {		
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	}		
		
	for (i = 0; i < count; i++) {
		if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_SPECTATOR) {
			CP(va("print \"Player %s ^7is already a spectator^3!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
		} else
			SetTeam( &g_entities[nums[i]], "spectator", qtrue );				
			AP(va("chat \"console: ^7%s has forced player %s ^7to ^3spectators^7!\n\"", tag, g_entities[nums[i]].client->pers.netname));

			// Log it
			log =va("Player %s (IP:%i.%i.%i.%i) has forced user %s to spectators.", 
				ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
				ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname);
			if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
				logEntry (ADMACT, log);
	}		
	return;
}

/*
===========
Force user to Axis
===========
*/
void cmd_axis(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);		
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0){			
		CP("print \"Client not on server^3!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	}		

	for (i = 0; i < count; i++) {
		if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_RED) {
			CP(va("print \"Player %s ^7is already playing^3!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
		}  else
			SetTeam( &g_entities[nums[i]], "red", qtrue );		
			AP(va("chat \"console: ^7%s has forced player %s ^7to ^1Axis ^7team!\n\"", tag, g_entities[nums[i]].client->pers.netname));

			// Log it
			log =va("Player %s (IP:%i.%i.%i.%i) has forced user %s to Axis team.", 
				ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
				ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname);
			if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
				logEntry (ADMACT, log);
	}		
	return;
}

/*
===========
Force user to Allied
===========
*/
void cmd_allied(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);
		
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0) {			
		CP("print \"Client not on server^3!\n\"");
		return;
	 } else if (count > 1) {	
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	}

	for (i = 0; i < count; i++) {
		if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_BLUE){				
			CP(va("print \"Player %s ^7is already playing^3!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
		}  else
			SetTeam( &g_entities[nums[i]], "blue", qtrue );				
			AP(va("chat \"console:^7 %s has forced player %s ^7to ^4Allied ^7team!\n\"", tag, g_entities[nums[i]].client->pers.netname));

			// Log it
			log =va("Player %s (IP:%i.%i.%i.%i) has forced user %s to Axis team.", 
				ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
				ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname);
			if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
				logEntry (ADMACT, log);
	}		
	return;
}

/*
===========
Forces all matching players to that team
===========
*/
void cmd_forceClan(gentity_t *ent) {
	int count = 0;
	int i;
	char *cTeam = ent->client->pers.cmd2;
	int team;
	int nums[MAX_CLIENTS];
	char *tag, *log;
	qboolean found = qfalse;

	tag = sortTag(ent);

	if (!Q_stricmp(cTeam, "r") &&
		!Q_stricmp(cTeam, "b") &&
		!Q_stricmp(cTeam, "s") &&
		!Q_stricmp(cTeam, "red") &&
		!Q_stricmp(cTeam, "blue") &&
		!Q_stricmp(cTeam, "axis") &&
		!Q_stricmp(cTeam, "allies") &&
		!Q_stricmp(cTeam, "allied")
		) {
		CP("print \"You need to specify a team^3!\n\"");
		return;
	}

	// Sort Team now
	if (!Q_stricmp(cTeam, "r") || !Q_stricmp(cTeam, "red") || 
		!Q_stricmp(cTeam, "axis")) {
		team = TEAM_RED;
		cTeam = "red";
	} 
	else if (!Q_stricmp(cTeam, "b") || !Q_stricmp(cTeam, "blue") || 
		!Q_stricmp(cTeam, "allies") || !Q_stricmp(cTeam, "allied")) {
		team = TEAM_BLUE;
		cTeam = "blue";
	} 
	else {
		team = TEAM_SPECTATOR;
		cTeam = "spectator";
	}

	count = ClientNumberFromNameMatch(ent->client->pers.cmd3, nums);
	if (count == 0) {
		CP("print \"No matching Clan name found^3!\n\"");
		return;
	}

	for (i = 0; i < count; i++) {
		if (g_entities[nums[i]].client->sess.sessionTeam != team) {
			SetTeam(&g_entities[nums[i]], cTeam, qtrue);
			found = qtrue;
		}
	}

	if (found) {

		AP(va("chat \"console:^7 %s has forced all %s players ^7to %s ^7team!\n\"",
			tag,
			ent->client->pers.cmd3,
			(!Q_stricmp(cTeam, "blue") ? "^4Allied" :
				(!Q_stricmp(cTeam, "red") ? "^1Axis" : "^3Spectators")
			)
		));

		// Log it
		log = va("Player %s (IP:%i.%i.%i.%i) has forced user %s into %s team.",
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
			ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname,
			(!Q_stricmp(cTeam, "blue") ? "Allied" :
				(!Q_stricmp(cTeam, "red") ? "Axis" : "Spectators")
			)
		);

		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry(ADMACT, log);
	}

	return;
}

/*
===========
This is just a wrapper to notify about correct way..
===========
*/
void cmd_force(gentity_t *ent) {
	CP("print \"^3INFO: ^7Use !axis, !specs and !allies to force a player to a desirable team.\n\"");
	return;
}

/*
===========
Execute command
===========
*/
void cmd_exec(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!strcmp(ent->client->pers.cmd3,"@"))		
		CP(va("print \"^3Info: ^7%s has been executed^3!\n\"", ent->client->pers.cmd2));
	else		
		AP(va("print \"console: ^7%s has executed ^3%s^7 config.\n\"", tag, ent->client->pers.cmd2));	
	
	trap_SendConsoleCommand( EXEC_INSERT, va("exec \"%s\"", ent->client->pers.cmd2));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has executed %s config.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], ent->client->pers.cmd2);
	logEntry (ADMACT, log);

	return;
}

/*
===========
Nextmap
===========
*/
void cmd_nextmap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"console: ^7%s has set ^3nextmap ^7in rotation.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("vstr nextmap"));		

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has set nextmap.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Load map
===========
*/
void cmd_map(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);	
	AP(va("chat \"console: ^7%s has loaded ^3%s ^7map. \n\"", tag, ent->client->pers.cmd2));
	trap_SendConsoleCommand( EXEC_APPEND, va("map %s", ent->client->pers.cmd2));				

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has loaded %s map.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], g_entities->client->pers.cmd2);
	logEntry (ADMACT, log);

	return;
}

/*
===========
Vstr 

Loads next map in rotation (if any)
===========
*/
void cmd_vstr(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);	
	AP(va("chat \"console: ^7%s set vstr to ^3%s^7.\n\"", tag, ent->client->pers.cmd2));
	trap_SendConsoleCommand( EXEC_APPEND, va("vstr %s", ent->client->pers.cmd2));		

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has set vstr to %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], g_entities->client->pers.cmd2);
	logEntry (ADMACT, log);

	return;
}

/*
===========
Center prints message to all
===========
*/
void cmd_cpa(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);	
	AP(va("cp \"^1ADMIN WARNING^7! \n%s\n\"", s));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) issued CPA warning: %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Shows message to selected user in center print
===========
*/
void cmd_cp(gentity_t *ent) {	
	int	player_id;
	gentity_t	*targetclient;	
	char *s, *log;

	s = ConcatArgs(3);	
	
	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) { 
		return;
	}

	targetclient = g_entities + player_id; 

	// CP to user	
	CPx(targetclient-g_entities, va("cp \"^1ADMIM WARNING^7! \n%s\n\"2", s));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) issued to user %s CP warning: %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], targetclient->client->pers.netname, s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Shows message to all in console and center print
===========
*/
void cmd_warn(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("cp \"^1ADMIM WARNING^7: \n%s\n\"2", s));
	AP(va("chat \"^1ADMIM WARNING^7: \n%s\n\"", s));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) issued global warning: %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Shows message to all in console
===========
*/
void cmd_chat(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);	
	AP(va("chat \"^1ADMIM WARNING^7: \n%s\n\"", s));		

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) issued CHAT warning: %s", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}
/*
===========
Cancels any vote in progress
===========
*/
void cmd_cancelvote(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);	
	if ( level.voteTime ) {
		level.voteNo = level.numConnectedClients;
		CheckVote();
		AP(va("cp \"%s has ^3Cancelled ^7the vote.\n\"2", tag));
		AP("chat \"console: Turns out everyone voted No^3!\n\"");	
		
		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) cancelled a vote.", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3]);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log);

		return;
	}

	return;
}

/*
===========
Passes any vote in progress
===========
*/
void cmd_passvote(gentity_t *ent){
	char *tag, *log;

	tag = sortTag(ent);	
	if ( level.voteTime ) {
		level.voteYes = level.numConnectedClients;
		CheckVote();		
		AP(va("cp \"%s has ^3Passed ^7the vote.\n\"2", tag));
		AP("chat \"console: Turns out everyone voted Yes^3!\n\"");		

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) passed a vote.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log);
		return;
	}

	return;
}

/*
===========
Map restart
===========
*/
void cmd_restart(gentity_t *ent) {
	char *tag, *log;

	if (g_doWarmup.integer && g_tournamentMode.integer == TOURNY_FULL) {
		CP("print \"Please use !allready instead of map_restart in Tournament mode..");
		return;
	}

	tag = sortTag(ent);	
	AP(va("chat \"console: %s has ^3restarted ^7map.\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0"));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has restarted map.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
	logEntry (ADMACT, log);

	return;
}

/*
===========
Reset match
===========
*/
void cmd_resetmatch(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);	
	AP(va("chat \"console: %s has ^3resetted ^7match^z.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("reset_match"));		

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has resetted match.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Swap teams
===========
*/
void cmd_swap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);	
	AP(va("chat \"console: %s has ^3swapped ^7the teams.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("swap_teams"));		

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has swapped teams.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
===========
Shuffle
===========
*/
void cmd_shuffle(gentity_t *ent) {
	char  *log;

	G_teamReset(TEAM_RED, qtrue);
	G_teamReset(TEAM_BLUE, qtrue);

	AP(va("print \"console: %s has ^3shuffled ^7teams.\n\"", sortTag(ent)));

	// Shuffle it...	
	trap_SendConsoleCommand(EXEC_APPEND, va("shuffle%s", (!Q_stricmp(ent->client->pers.cmd2, "@") ? " @" : "")));

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has shuffled teams.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return;
}

/*
==================
Move lagged out or downloading clients to spectators
==================
*/
qboolean cmd_specs999(gentity_t *ent) {
	int i;
	qboolean moved=qfalse;
	char *tag, *log;

	tag = sortTag(ent);	
	for(i = 0; i < level.maxclients; i++) {
		ent = &g_entities[i];
		if(!ent->client) continue;
		if(ent->client->pers.connected != CON_CONNECTED) continue;	
		if(ent->client->ps.ping >= 999) {
			SetTeam(ent, "s", qtrue);
			moved = qtrue;		
		}
	}

	if (moved==qtrue)		
		AP(va("chat \"console: %s moved all lagged out players to spectators^3!\n\"", tag));
	else
		CP("print \"No one to move to spectators^3!\n\"");	

		// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) forced all 999 to spectators.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3]);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

	return qtrue;
}

/* 
===========
Rename player
===========
*/
void cmd_rename(gentity_t *ent) {
	int          clientNum;
	gclient_t	 *client;
	char *tag, *log;
	char userinfo[MAX_INFO_STRING];

	tag = sortTag(ent);
	
	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		return; 
	}

	// Ugly..
	log = va("Player %s (IP:%i.%i.%i.%i) has renamed user", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], ent->client->sess.ip[3]);

	ent = g_entities + clientNum;
	client = ent->client;
	
	log = va("%s %s to", log, client->pers.netname);

	// Print first..
	AP(va("chat \"console:^7 %s has renamed player %s ^7to %s^3!\n\"", tag, client->pers.netname, ConcatArgs(3)));

	// Rename..
	trap_GetUserinfo( client->ps.clientNum, userinfo, sizeof( userinfo ) );	
	Info_SetValueForKey( userinfo, "name", ConcatArgs(3));
	trap_SetUserinfo( client->ps.clientNum, userinfo );
	ClientUserinfoChanged(client->ps.clientNum);	

	// Log it
	log =va("%s %s", log, ConcatArgs(3));
	// Not vital..
	if (g_extendedLog.integer > 1)
		logEntry (ADMACT, log);

	return;
}

/*
==================
Reveal location of a player.
==================
*/
void cmd_revealCamper(gentity_t *ent) {
	char location[64];
	int	clientNum;
	char *tag, *log, *log2;

	tag = sortTag(ent);	
	log2 = va("Player %s (IP:%i.%i.%i.%i)", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],ent->client->sess.ip[3] );

	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		return; //check if target is not a client
	}

	// Give values to these
	ent = g_entities + clientNum;

	Team_GetLocationMsg(ent, location, sizeof(location), qtrue);		
	AP(va("chat \"console: %s has releaved that player %s^7 is hidding at ^3%s^7.\n\"", tag, ent->client->pers.netname, location));

	// Log it
	log =va("%s has revealed %s location.",log2, ent->client->pers.netname );
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);
}

/*
==================
Ignore user (uses name)
==================
*/
void cmd_ignore(gentity_t *ent, qboolean unignore) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log, *action;

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0) {			
		CP("print \"Client not on server^1!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	} 
		
	for (i = 0; i < count; i++) {	

		// Unignores players
		if (unignore) {
			if (!g_entities[nums[i]].client->sess.ignored) {
				CP(va("print \"%s ^7is already unignored^1!\n\"", g_entities[nums[i]].client->pers.netname));
				return;
			} else {
				AP(va("chat \"console: %s has unignored player %s^3!\n\"", tag, g_entities[nums[i]].client->pers.netname));
				g_entities[nums[i]].client->sess.ignored = 0;
			}
			action = va("unignored");

		// Ignores player
		} else {
			if (g_entities[nums[i]].client->sess.ignored) {
				CP(va("print \"%s ^7is already ignored^1!\n\"", g_entities[nums[i]].client->pers.netname));
				return;
			} else {
				AP(va("chat \"console: %s has ignored player %s^3!\n\"", tag, g_entities[nums[i]].client->pers.netname));
				g_entities[nums[i]].client->sess.ignored = 1;
			}
			action = va("ignored");
		}

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) has %s user %s.", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], action, g_entities[nums[i]].client->pers.netname);
		logEntry (ADMACT, log);
	}
}

/*
==================
Ignore user (uses clientslot)
==================
*/
void cmd_clientignore(gentity_t *ent, qboolean unignore) {
	int	clientNum;
	gentity_t	*targetclient;	
	char *tag, *log, *action;

	tag = sortTag(ent);

	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		CP("print \"Client not on server^1!\n\"");
		return; 
	}

	// Give values to these
	targetclient = g_entities + clientNum;	

	// Unignores players
	if (unignore) {
		if (!targetclient->client->sess.ignored) {
			CP(va("print \"%s ^7is already unignored^1!\n\"", targetclient->client->pers.netname));
			return;
		} else {
			AP(va("chat \"console: %s has unignored player %s^3!\n\"", tag, targetclient->client->pers.netname));
			targetclient->client->sess.ignored = 0;
		}
		action = va("unignored");

	// Ignores player
	} else {
		if (targetclient->client->sess.ignored) {
			CP(va("print \"%s ^7is already ignored^1!\n\"", targetclient->client->pers.netname));
			return;
		} else {
			AP(va("chat \"console: %s has ignored player %s^3!\n\"", tag, targetclient->client->pers.netname));
			targetclient->client->sess.ignored = 1;
		}
		action = va("ignored");
	}

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has %s user %s.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], action, targetclient->client->pers.netname);
	logEntry (ADMACT, log);
}

/*
==================
Permanently Ignores user (uses name)
==================
*/
void cmd_permignore(gentity_t *ent, qboolean unignore) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log, *action;

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0) {			
		CP("print \"Client not on server^1!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	} 
		
	for (i = 0; i < count; i++) {	

		// Unignores players
		if (unignore) {
			// Poor man's solution to (semi)avoid useless messages
			if (g_entities[nums[i]].client->sess.ignored)			
				AP(va("chat \"console: %s has removed permanent ignore from player %s^3!\n\"", tag, g_entities[nums[i]].client->pers.netname));
			else
				CP(va("print \"You removed permanent ignore from player %s^3!\n\"", g_entities[nums[i]].client->pers.netname));

			g_entities[nums[i]].client->sess.ignored = 0;
			UNIGNORE_CLIENT(g_entities[nums[i]].client->sess.guid);		
			action = va("unignored");

		// Ignores player
		} else {
			if (g_entities[nums[i]].client->sess.ignored) {
				CP(va("print \"%s ^7is already ignored^1!\n\"", g_entities[nums[i]].client->pers.netname));
				return;
			} else {
				AP(va("chat \"console: %s has ^3permanently ignored ^7player %s^3!\n\"", tag, g_entities[nums[i]].client->pers.netname));
				g_entities[nums[i]].client->sess.ignored = 2;
				IGNORE_CLIENT(g_entities[nums[i]].client->sess.guid);
			}
			action = va("ignored");
		}

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) has permanently %s user %s.", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], action, g_entities[nums[i]].client->pers.netname);
		logEntry (ADMACT, log);
	}
}

/*
==================
Ignore user (uses clientslot)
==================
*/
void cmd_permclientignore(gentity_t *ent, qboolean unignore) {
	int	clientNum;
	gentity_t	*targetclient;	
	char *tag, *log, *action;

	tag = sortTag(ent);

	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		CP("print \"Client not on server^1!\n\"");
		return; 
	}

	// Give values to these
	targetclient = g_entities + clientNum;	

	// Unignores player
	if (unignore) {	
		// Poor man's solution to (semi)avoid useless messages
		if (targetclient->client->sess.ignored)
			AP(va("chat \"console: %s has removed permanent ignore from player %s^3!\n\"", tag, targetclient->client->pers.netname));
		else
			CP(va("print \"You removed permanent ignore from player %s^3!\n\"", targetclient->client->pers.netname));

		targetclient->client->sess.ignored = 0;
		UNIGNORE_CLIENT(targetclient->client->sess.guid);		
		action = va("unignored");

	// Ignores player
	} else {
		if (targetclient->client->sess.ignored) {
			CP(va("print \"%s ^7is already ignored^1!\n\"", targetclient->client->pers.netname));
			return;
		} else {
			AP(va("chat \"console: %s has ^3permanently ignored ^7player %s^3!\n\"", tag, targetclient->client->pers.netname));
			targetclient->client->sess.ignored = 2;
			IGNORE_CLIENT(targetclient->client->sess.guid);
		}
		action = va("ignored");
	}

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) has permanently %s user %s.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], action, targetclient->client->pers.netname);
	logEntry (ADMACT, log);
}

/*
===========
Ban guid
===========
*/
void cmd_banGuid(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){			
			CP("print \"Client not on server^3!\n\"");
		return;
		} else if (count > 1) {			
			CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
		} 
		
		for (i = 0; i < count; i++){
			// Ban player			
			trap_SendConsoleCommand(EXEC_APPEND, va("addguid %s", g_entities[nums[i]].client->sess.guid));

			// Kick player now
			trap_DropClient( nums[i], va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));			
			AP(va("chat \"console: %s has banned player %s^1! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));		

			// Log it
			log =va("Player %s (GUID: %s) has banned user %s (GUID: %s)", 
				ent->client->pers.netname, ent->client->sess.guid, g_entities[nums[i]].client->pers.netname, g_entities[nums[i]].client->sess.guid);
			logEntry (ADMACT, log);
		}
	return;
}

/*
===========
Ban client guid

May seem as an overkill but ppl go over all sort of
length to avoid bans (renaming while processed..)
===========
*/
void cmd_banClientGuid(gentity_t *ent) {
	int	player_id;
	gentity_t	*targetclient;	
	char *tag, *log;

	tag = sortTag(ent);
	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) { 
		return;
	}

	targetclient = g_entities + player_id; 

	// Ban player	
	trap_SendConsoleCommand(EXEC_APPEND, va("addguid %s", targetclient->client->sess.guid));

	// Kick the client
	trap_DropClient( player_id, va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));	
	AP(va("chat \"console: %s has banned player %s^3! ^3%s\n\"", tag, targetclient->client->pers.netname, ent->client->pers.cmd3));

	// Log it
	log =va("Player %s (GUID: %s) has banned user %s (GUID: %s)", 
		ent->client->pers.netname, ent->client->sess.guid, targetclient->client->pers.netname,targetclient->client->sess.guid);
	logEntry (ADMACT, log);

	return;
}

/*
===========
Temp ban guid
===========
*/
void cmd_tempbanGuid(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);


	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){			
		CP("print \"Client not on server^3!\n\"");
	return;
	} else if (count > 1){			
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
	return;
	} else if (!is_numeric(ent->client->pers.cmd3)) {
		CPx(ent-g_entities, "print \"^1Error: ^7Invalid syntax used for tempban command. Make sure you use numbers only.\n\"");
	return;
	}
		
	for (i = 0; i < count; i++){
		// Ban player	
		trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %s", g_entities[nums[i]].client->sess.guid, ent->client->pers.cmd3));		

		// Kick player now
		trap_DropClient( nums[i], va( "^3Temporarily banned by ^3%s\n^7Tempban will expire in ^3%s ^7minute(s)", tag, ent->client->pers.cmd3));			
		AP(va("chat \"console: %s has tempbanned player %s ^7for ^3%s ^7minute(s)^3!\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));		

		// Log it
		log =va("Player %s (GUID: %s) tempbanned user %s (GUID: %s) for %s minute(s).", 
			ent->client->pers.netname, ent->client->sess.guid, g_entities[nums[i]].client->pers.netname, g_entities[nums[i]].client->sess.guid, ent->client->pers.cmd3 );	
		logEntry (ADMACT, log);
		
	}

	return;
}

/*
===========
Ban ip
===========
*/
void cmd_banIp(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	if (!IP_handling.integer)
	{
		CP("print \"IP handling is disabled on this server. Use GUID related commands.\n\"");
		return;
	}

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);

	if (count == 0){			
		CP("print \"Client not on server^3!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	} 
		
	for (i = 0; i < count; i++){
		// Ban player			
		trap_SendConsoleCommand(EXEC_APPEND, va("addip %i.%i.%i.%i", 
			g_entities[nums[i]].client->sess.ip[0], g_entities[nums[i]].client->sess.ip[1], 
			g_entities[nums[i]].client->sess.ip[2], g_entities[nums[i]].client->sess.ip[3] ));

		// Kick player now
		trap_DropClient( nums[i], va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));			
		AP(va("chat \"console: %s has banned player %s^3! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));		

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) has (IP)banned user %s", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname);
		logEntry (ADMACT, log);
	}

	return;
}

/*
===========
tempBan ip
===========
*/
void cmd_tempBanIp(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	if (!IP_handling.integer)
	{
		CP("print \"IP handling is disabled on this server. Use GUID related commands.\n\"");
		return;
	}

	tag = sortTag(ent);
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){			
		CP("print \"Client not on server^3!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"Too many people with ^3%s ^7in their name^3!\n\"", ent->client->pers.cmd2));
		return;
	} 
		
	for (i = 0; i < count; i++){
		// TempBan player			
		trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %s", nums[i], ent->client->pers.cmd3 ));

		// Kick player now
		trap_DropClient( nums[i], va( "^3temporarily banned by ^3%s \n^7Tempban will expire in ^3%s ^7minute(s)", tag, ent->client->pers.cmd3));			
		AP(va("chat \"console: %s has tempbanned player %s ^7for ^3%s ^7minute(s)^3!\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));		

		// Log it
		log =va("Player %s (IP:%i.%i.%i.%i) tempbanned user %s by IP for %s minute(s).", 
			ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
			ent->client->sess.ip[3], g_entities[nums[i]].client->pers.netname, ent->client->pers.cmd3 );	
		logEntry (ADMACT, log);
	}

	return;
}

/*
===========
Add IP
===========
*/
void cmd_addIp(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!IP_handling.integer)
	{
		CP("print \"IP handling is disabled on this server. Use GUID related commands.\n\"");
		return;
	}

	if (!IPv4Valid(ent->client->pers.cmd2)) {
		CP(va("print \"^1Error: ^7%s is not a valid IPv4 address^1!\n\"", ent->client->pers.cmd2));
		return;
	}

	// Note that this blindly accepts what ever user inputs. Not ideal..
	trap_SendConsoleCommand(EXEC_APPEND, va("addip %s", ent->client->pers.cmd2 ));	
	AP(va("chat \"console: %s has added IP ^3%s ^7to banned file.\n\"", tag, ent->client->pers.cmd2));	

	// Log it
	log =va("Player %s (IP:%i.%i.%i.%i) added IP %s to banned file.", 
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], ent->client->pers.cmd2  );	
	logEntry (ADMACT, log);

	return;
}

/*
===========
Event handling

Pauses or Resumes current (global stats) event
===========
*/
void cmd_Event(gentity_t *ent) {
	char *tag, *log, *cmd;

	tag = sortTag(ent);
	cmd = va("%s", ent->client->pers.cmd2);

	if (!g_httpEvent.string)
	{
		CP("print \"There is no Event set on this server.\n\"");
		return;
	}

	if (!Q_stricmp(cmd, "resume")) {
		if (!g_httpEventPaused.integer) {
			CP("print \"Event is already in progress!\n");
			return;
		}

		trap_Cvar_Set("g_httpEventPaused", "0");
		AP(va("chat \"console: %s has Resumed the %s ^7Event.\n", tag, g_httpEvent.string));
		APS("xmodTE/sound/misc/unlocked.wav");
	}
	else if (!Q_stricmp(cmd, "pause")) {
		if (g_httpEventPaused.integer) {
			CP("print \"Event is already paused!\n");
			return;
		}

		trap_Cvar_Set("g_httpEventPaused", "1");
		AP(va("chat \"console: %s has Paused the %s ^7Event.\n", tag, g_httpEvent.string));
	}
	else {
		CP("print \"Unknown Event Command..Use ?event for info..\n");
		return;
	}

	// Log it
	log = va("Player %s (IP:%i.%i.%i.%i) has %s the event.",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
		ent->client->sess.ip[3], ent->client->pers.cmd2);
	logEntry(ADMACT, log);

	return;
}

/*
==================
Speclock/unlock
==================
*/
qboolean specAlready(int team, qboolean lock) {
	if (team > 0 && team < 3) {
		if (teamInfo[team].spec_lock == lock)
			return qtrue;
		else
			return qfalse;
	}
	else if (team == 3) {
		if ((teamInfo[TEAM_RED].spec_lock == lock) &&
			(teamInfo[TEAM_BLUE].spec_lock == lock))
			return qtrue;
		else
			return qfalse;
	}
	return qfalse;
}

void cmd_specHandle(gentity_t *ent, qboolean lock) {
	int team;
	char *act = ((lock) ? "locked" : "unlocked");
	char *log;

	if (!ent->client->pers.cmd2) {
		CP(va("print \"^1Error: ^7You need to specify a team!\nUse ^3?spec%s ^7for help.\n\"", (lock ? "lock" : "unlock")));
		return;
	}

	if (g_tournamentMode.integer == TOURNY_FULL && !lock) {
		CP("print \"You cannot unlock team for Spectators in Full Tournament Mode!\n\"");
		return;
	}

#define STM(x) !(strcmp(ent->client->pers.cmd2,x))

	if (STM("both")) {
		team = 3;
	}
	else if (STM("red") || STM("axis"))	{
		team = TEAM_RED;
	}
	else if (STM("blue") || STM("allied") || STM("allies")) {
		team = TEAM_BLUE;
	}
	else {
		CP(va("print \"^1Error: ^7Unknown argument ^1%s^7!\nUse ^1?spec%s ^7for help.\n\"",
			ent->client->pers.cmd2, (lock ? "lock" : "unlock")));
		return;
	}

	if (specAlready(team, lock)) {
		CP(va("print \"^1Error^7: %s already spec%s!\n\"",
			((team == 3) ? "^3Both ^7teams are" : va("Team %s is", aTeams[team])), act));
		return;
	}

	// Sanity check
	if (lock) {
		if (team == TEAM_BLUE && !level.alliedPlayers) {
			CP(va("print \"^1Error^7: %s team has no players!\n\"", aTeams[team]));
			return;
		}
		else if (team == TEAM_RED && !level.axisPlayers) {
			CP(va("print \"^1Error^7: %s team has no players!\n\"", aTeams[team]));
			return;
		}
		else if (team == TEAM_SPECTATOR && (!level.axisPlayers || !level.alliedPlayers)) {
			CP("print \"^1Error^7: Not all teams have players!\n\"");
			return;
		}
	}

	if (team != 3) {
		G_updateSpecLock(team, lock);
	}
	else {
		G_updateSpecLock(TEAM_RED, lock);
		G_updateSpecLock(TEAM_BLUE, lock);
	}

	aTeams[team] = (team == 3) ? "^3Both^7" : aTeams[team];
	AP(va("chat \"console: ^7%s has spec%s %s team%s\"", sortTag(ent), act, aTeams[team], ((team == 3) ? "s" : "")));

	log = va("Player %s (IP: %d.%d.%d.%d) has %s team(%s).",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
		ent->client->sess.ip[3], act, aTeams[team]);
	logEntry(ADMACT, log);
}

/*
===========
Lock/Unlock team

NOTE: Somewhat messy due verbosity
TODO: Clean this eventually..
===========
*/
qboolean canTeamBeLocked(int team)
{
	if (team == TEAM_RED && level.axisPlayers < 1)
		return qfalse;
	else if (team == TEAM_BLUE && level.alliedPlayers < 1)
		return qfalse;
	else
		return qtrue;
}
// Lock/Unlock
void cmd_handleTeamLock(gentity_t *ent, qboolean tLock) {
	char *tag = sortTag(ent);
	char *cmd = ent->client->pers.cmd2;
	char *action = (tLock ? "Lock" : "Unlock");
	int team = TEAM_NUM_TEAMS;
	char *teamTag = "^3Both^7";
	char *log;

	if (!Q_stricmp(cmd, "")) {
		CP(va("print \"^1Error: ^7Please select which team you wish to %s!\n\"", action));
		return;
	}

	// Axis
	if (!Q_stricmp(cmd, "red") || !Q_stricmp(cmd, "axis") || !Q_stricmp(cmd, "r")) {
		team = TEAM_RED;
		teamTag = "^1Axis^7";
	}
	// Allies
	else if (!Q_stricmp(cmd, "blue") || !Q_stricmp(cmd, "allies") || !Q_stricmp(cmd, "allied") || !Q_stricmp(cmd, "b"))	{
		team = TEAM_BLUE;
		teamTag = "^4Allied^7";
	}
	// Both
	else if (!(Q_stricmp(cmd, "both") == 0)) {
		CP(va("print \"^1Error^7: ^7Please select which team you wish to %s!\n\"", action));
		return;
	}

	if (team != TEAM_NUM_TEAMS) {
		if (teamInfo[team].team_lock == tLock) {
			CP(va("print \"^1Error^7: %s team is already %sed!  \n\"", teamTag, action));
			return;
		}
		else {
			if (!canTeamBeLocked(team)) {
				CP(va("print \"^1Error^7: %s team is empty!\n\"", teamTag));
				return;
			}
			AP(va("chat \"console: %s has %sed %s team!\n\"", tag, action, teamTag));
			teamInfo[team].team_lock = tLock;
		}
	}
	else {
		if (teamInfo[TEAM_RED].team_lock != tLock || teamInfo[TEAM_BLUE].team_lock != tLock) {
			teamInfo[TEAM_RED].team_lock = tLock;
			teamInfo[TEAM_BLUE].team_lock = tLock;
			AP(va("chat \"console: %s has %sed %s teams!\n\"", tag, action, teamTag));
		}
		else
			CP(va("print \"Error: Both teams are already %sed!  \n\"", action));
		return;
	}

	log = va("Player %s (IP: %d.%d.%d.%d) has issued %s for %s",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
		ent->client->sess.ip[3], action,
		(team == TEAM_RED ? "Axis team" : (team == TEAM_BLUE ? "Allied team" : "Both teams")));

	logEntry(ADMACT, log);
	return;
}

/*
==================
Pause/Unpause
==================
*/
void cmd_pauseHandle(gentity_t *ent, qboolean fPause) {
	char *status[2] = { "^3UN", "^3" };
	char *log;

	if ((!level.alliedPlayers && !level.axisPlayers) && fPause) {
		CP("print \"^1Error^7: Pause can only be used when at least 1 team has a player!\n\"");
		return;
	}

	if ((PAUSE_UNPAUSING >= level.match_pause && !fPause) || (PAUSE_NONE != level.match_pause && fPause)) {
		CP(va("print \"^1Error^7: The match is already %sPAUSED!\n\"", status[fPause]));
		return;
	}

	// Trigger the auto-handling of pauses
	if (fPause) {
		level.match_pause = 100 + ((ent) ? (1 + ent - g_entities) : 0);
		G_spawnPrintf(DP_PAUSEINFO, level.time + 15000, NULL);
		AP(va("chat \"console: %s ^3PAUSED^7 the match!\n", sortTag(ent)));
	}
	else {
		AP(va("chat \"console: %s ^3UNPAUSED^7 the match.\n\n\"", sortTag(ent)));
		level.match_pause = PAUSE_UNPAUSING;
		G_spawnPrintf(DP_UNPAUSING, level.time + 10, NULL);
	}

	// Log it
	log = va("Player %s (IP: %d.%d.%d.%d) has %s a match.",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2],
		ent->client->sess.ip[3], (fPause ? "paused" : "resumed"));

	logEntry(ADMACT, log);
}

/*
===========
Un/Ready all

Ready or unready all..
===========
*/
void cmd_readyHandle(gentity_t *ent, qboolean unready) {
	char *msg = ((unready) ? "^3UNREADY^7" : "^3READY^7");
	char *log;

	if (!g_doWarmup.integer) {
		CP("print \"Tourny mode is disabled! Command ignored..\n\"");
		return;
	}

	if (!unready) {
		if (g_gamestate.integer != GS_WARMUP) {
			CP("print \"^3ALLREADY ^7command can only be used in warmup!\n\"");
			return;
		}
		G_readyStart();
		AP(va("chat \"console: ^7%s has %s ALL players..\n\"", sortTag(ent), msg));
	}
	else {
		if (g_gamestate.integer != GS_WARMUP_COUNTDOWN) {
			CP("print \"^3UNREADYALL ^7command can only be used during countdown!\n\"");
			return;
		}
		G_readyReset(qtrue);
		AP(va("chat \"console: Countdown has been ^3cancelled ^7by %s..\n\"", sortTag(ent)));
	}

	// Log it
	log = va("Player %s (IP: %d.%d.%d.%d) has %s users.",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], 
		ent->client->sess.ip[2], ent->client->sess.ip[3], msg);
	
	logEntry(ADMACT, log);
}

/*
===========
Tourney

Starts or stops the tourney 
===========
*/
void cmd_tourney(gentity_t *ent, qboolean start) {	
	char *tag = sortTag(ent);
	char *log;

	if (g_tournamentMode.integer != TOURNY_FULL) {
		CP("print \"Tournament mode 2 has to be enabled for this command to become available.\n");
		return;
	}

	if ((!level.alliedPlayers || !level.axisPlayers) && start) {
		CP("print \"^1Error^7: Tournament can only start when Both teams have players!\n\"");
		return;
	}

	if (start) {
		if (int_match_started.integer) {
			CP("print \"^1Error^7: Tournament is already in progress!\n");
			return;
		}
		
		AP(va("chat \"console: %s has ^3STARTED ^7tournament!\n", tag));
		AP(va("@print \"%s has ^3STARTED ^7tournament!\n", ent->client->pers.netname));
		G_TourneyHandle(qfalse, qfalse);
		APS("xmodTE/sound/misc/western.wav");
	}
	else {
		if (!int_match_started.integer) {
			CP("print \"^1Error^7: Tournament is not in progress..\n");
			return;
		}
		
		AP(va("chat  \"console: %s has ^3STOPPED ^7tournament!\n", tag));
		AP(va("@print \"%s has ^3STOPPED ^7tournament!\n", ent->client->pers.netname));
		G_TourneyHandle(qtrue, qtrue);
	}

	// Log it
	log = va("Player %s (IP: %d.%d.%d.%d) has %s the tournament.",
		ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1],
		ent->client->sess.ip[2], ent->client->sess.ip[3], (start ? "started" : "stopped") );

	logEntry(ADMACT, log);
}

/*
===========
Getstatus

Prints IP's, GUIDs and some match info..
===========
*/
void cmd_getstatus(gentity_t *ent) {
	int	j;
	// uptime
	int secs = level.time / 1000;
	int mins = (secs / 60) % 60;
	int hours = (secs / 3600) % 24;
	int days = (secs / (3600 * 24));
	qboolean teamSpecLocked = qfalse;
	qtime_t ct;
	trap_RealTime(&ct);	

	if (teamInfo[TEAM_RED].spec_lock || teamInfo[TEAM_BLUE].spec_lock)
		teamSpecLocked = qtrue;

	CP(va("print \"\n^7Server: %s    ^7%02d:%02d:%02d ^3(^7%02d %s %d^3)\n\"", sv_hostname.string, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, cMonths[ct.tm_mon], 1900+ct.tm_year));
	// N/c..
	if (teamInfo[TEAM_BLUE].spec_lock || teamInfo[TEAM_RED].spec_lock)
		CP(va("print \"Speclocked: %s^7\n\"",
			((teamInfo[TEAM_BLUE].spec_lock && teamInfo[TEAM_RED].spec_lock) ? "^3Both" :
			((teamInfo[TEAM_BLUE].spec_lock ? "^4Allied" : "^1Axis"))
		)));
	if (teamInfo[TEAM_BLUE].team_lock || teamInfo[TEAM_RED].team_lock)
		CP(va("print \"Teamlocked: %s^7\n\"",
			((teamInfo[TEAM_BLUE].team_lock && teamInfo[TEAM_RED].team_lock) ? "^3Both" :
			((teamInfo[TEAM_BLUE].team_lock ? "^4Allied" : "^1Axis"))
		)));
	CP("print \"^3--------------------------------------------------------------------------\n\"");	
	CP("print \"^7Slot : Team : Name       : ^3IP              ^7: ^3Guid         ^7: Status \n\"");
	CP("print \"^3--------------------------------------------------------------------------\n\"");

	for ( j = 0; j <= (MAX_CLIENTS-1); j++) {	

		if ( g_entities[j].client && !( ent->r.svFlags & SVF_BOT ) ) {
			char *team, *slot, *ip, *tag, *sortTag, *extra;
			char n1[MAX_NETNAME]; 
			char n2[MAX_NETNAME];	
			char guid[13];

			// Has to be done twice to cope with double carrots..
			DecolorString(g_entities[j].client->pers.netname, n1);
			SanitizeString(n1, n2, qfalse); 
			Q_CleanStr(n2);	
			n2[10] = 0;	

			// player is connecting
			if ( g_entities[j].client->pers.connected == CON_CONNECTING ) {				
				CP(va("print \"%3d  : >><< : %-10s : ^3>>Connecting<<  ^7:\n\"", j, n2));
			}
			
			// player is connected
			if ( g_entities[j].client->pers.connected == CON_CONNECTED ) { 
					
				// Sort it :C
				sortTag = "";
				slot = va("%3d", j);
				team = (g_entities[j].client->sess.sessionTeam == TEAM_SPECTATOR) ? "^3SPEC^7" : 
					(g_entities[j].client->sess.sessionTeam == TEAM_RED ? "^1Axis^7" : "^4Alld^7" );
									
				ip = ( ent->client->sess.admin == ADM_NONE ) ?
					va("%i.*.*.*", g_entities[j].client->sess.ip[0] ) :
					va("%i.%i.%i.%i", g_entities[j].client->sess.ip[0], g_entities[j].client->sess.ip[1], g_entities[j].client->sess.ip[2], g_entities[j].client->sess.ip[3]  );					
					
				switch (g_entities[j].client->sess.admin) {
					case ADM_NONE:
						sortTag = "";
					break;
					case ADM_1:
						sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a1_tag.string) : va("%s", a1_tag.string);
					break;
					case ADM_2:
						sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a2_tag.string) : va("%s", a2_tag.string);
					break;
					case ADM_3:
						sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a3_tag.string) : va("%s", a3_tag.string);
					break;
					case ADM_4:
						sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a4_tag.string) : va("%s", a4_tag.string);
					break;
					case ADM_5:
						sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a5_tag.string) : va("%s", a5_tag.string);
					break;
				}	

				// Sort Admin tags..
				tag = "";
				extra = (g_entities[j].client->sess.admin == ADM_NONE && g_entities[j].client->sess.ignored) ? 
					((g_entities[j].client->sess.ignored > 1) ? "^3perm Ignored" : "^3Ignored") : "";
				if (ent->client->sess.admin == ADM_NONE) {
					tag = (g_entities[j].client->sess.admin != ADM_NONE && !g_entities[j].client->sess.incognito) ? sortTag : extra;
				} else if (ent->client->sess.admin != ADM_NONE) {
					tag = (g_entities[j].client->sess.admin == ADM_NONE && g_entities[j].client->sess.ignored) ? "^1Ignored" : sortTag;
				}

				// Specing speclocked team (This will override ignored tag but so be it..).
				if (g_entities[j].client->sess.sessionTeam == TEAM_SPECTATOR &&
					!g_entities[j].client->sess.admin && teamSpecLocked) 
				{
					if (g_entities[j].client->sess.specInvited == 1)
						tag = "Spec. Axis";
					else if (g_entities[j].client->sess.specInvited == 2)
						tag = "Spec. Allies";
					else if (g_entities[j].client->sess.specInvited == 3)
						tag = "Spec. Both";
					else if (teamSpecLocked > 0 && !g_entities[j].client->sess.specInvited)
						tag = "^3Speclocked";
					// Hidden admins
				}

				// Guid
				stripChars(g_entities[j].client->sess.guid, guid, 12);
				
					// Print it now
					CP(va("print \"%-4s : %s : %-10s : ^3%-15s ^7: ^3%-12s ^7: %-12s \n\"", 
						slot, 
						team, 
						n2,
						ip,
						guid,
						tag
					));	
			}
		}
	}	
	CP("print \"^3--------------------------------------------------------------------------\n\"");		
	CP( va("print \"^7Uptime: ^3%d ^7day%s ^3%d ^7hours ^3%d ^7minutes\n\"", days, (days != 1 ? "s" : ""), hours, mins));
	CP("print \"\n\"");	
	
	return;
}

/*********************************** INTERACTIONS ************************************/
/*
===========
List commands
===========
*/
void cmd_listCmds(gentity_t *ent) {
	char *cmds;
	
	if (!adm_help.integer) {		
		CP("print \"Admin commands list is disabled on this server^1!\n\"");
	return;
	}

	// Keep an eye on this..so it's not to big..
	cmds = "incognito list_cmds commands cmds kick clientlick "
		   "slap kill specs axis allied exec nextmap map vstr cpa "
		   "cp warn chat cancelvote passvote map_restart reset_match swap_teams shuffle "
		   "@shuffle specs999 whereis rename ignore unignore clientignore clientunignore permignore "	
		   "permunignore permclientignore permclientunignore ban banclient tempban banip tempbanip addip event "
		   "speclock specunlock lock unlock pause unpause start stop forceclan *"
		;

	if (ent->client->sess.admin == ADM_1)
		CP( va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a1_cmds.string ));
	else if (ent->client->sess.admin == ADM_2)
		CP( va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a2_cmds.string ));
	else if (ent->client->sess.admin == ADM_3)
		CP( va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a3_cmds.string ));
	else if (ent->client->sess.admin == ADM_4)
		CP( va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a4_cmds.string ));
	else if (ent->client->sess.admin == 5 && !a5_allowAll.integer)		
		CP( va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a5_cmds.string ));
	else if (ent->client->sess.admin == 5 && a5_allowAll.integer)
		CP( va("print \"^3Available commands are:^7\n%s\n^3Additinal server commands:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", cmds, a5_cmds.string ));		

	return;
}

/*
===========
Admin commands
===========
*/
qboolean do_cmds(gentity_t *ent) { 
	char alt[128];
	char cmd[128];

	parseCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	if (!strcmp(cmd,"incognito"))			{ if (canUse(ent, qtrue)) cmd_incognito(ent); else cantUse(ent);	return qtrue;}
	else if (!strcmp(cmd,"list_cmds"))		{ cmd_listCmds(ent);	return qtrue;}
	else if (!strcmp(cmd,"commands"))		{ cmd_listCmds(ent);	return qtrue;}
	else if (!strcmp(cmd,"cmds"))			{ cmd_listCmds(ent);	return qtrue;}
	else if (!strcmp(cmd,"kick"))			{ if (canUse(ent, qtrue)) cmd_kick(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientkick"))		{ if (canUse(ent, qtrue)) cmd_clientkick(ent);	else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"slap"))			{ if (canUse(ent, qtrue)) cmd_slap(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"kill"))			{ if (canUse(ent, qtrue)) cmd_kill(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"fling"))			{ if (canUse(ent, qtrue)) cmd_fling(ent, 0); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd,"throw"))			{ if (canUse(ent, qtrue)) cmd_fling(ent, 1); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd,"launch"))			{ if (canUse(ent, qtrue)) cmd_fling(ent, 2); else cantUse(ent); return qtrue; }	
	else if (!strcmp(cmd,"specs"))			{ if (canUse(ent, qtrue)) cmd_specs(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"axis"))			{ if (canUse(ent, qtrue)) cmd_axis(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"allies"))			{ if (canUse(ent, qtrue)) cmd_allied(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"exec"))			{ if (canUse(ent, qtrue)) cmd_exec(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"nextmap"))		{ if (canUse(ent, qtrue)) cmd_nextmap(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"map"))			{ if (canUse(ent, qtrue)) cmd_map(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"cpa"))			{ if (canUse(ent, qtrue)) cmd_cpa(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"cp"))			    { if (canUse(ent, qtrue)) cmd_cp(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"chat"))			{ if (canUse(ent, qtrue)) cmd_chat(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"warn"))			{ if (canUse(ent, qtrue)) cmd_warn(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"no"))				{ if (canUse(ent, qtrue)) cmd_cancelvote(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"yes"))			{ if (canUse(ent, qtrue)) cmd_passvote(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"map_restart"))	{ if (canUse(ent, qtrue)) cmd_restart(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"reset_match"))	{ if (canUse(ent, qtrue)) cmd_resetmatch(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"swap_teams"))		{ if (canUse(ent, qtrue)) cmd_swap(ent); else cantUse(ent); return qtrue;} 
	else if (!strcmp(cmd,"shuffle"))		{ if (canUse(ent, qtrue)) cmd_shuffle(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"spec999"))		{ if (canUse(ent, qtrue)) cmd_specs999(ent); else cantUse(ent); return qtrue;} 	
	else if (!strcmp(cmd,"whereis"))		{ if (canUse(ent, qtrue)) cmd_revealCamper(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"rename"))			{ if (canUse(ent, qtrue)) cmd_rename(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"vstr"))			{ if (canUse(ent, qtrue)) cmd_vstr(ent); else cantUse(ent); return qtrue;}	
	else if (!strcmp(cmd,"ignore"))			{ if (canUse(ent, qtrue)) cmd_ignore(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"unignore"))		{ if (canUse(ent, qtrue)) cmd_ignore(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientignore"))	{ if (canUse(ent, qtrue)) cmd_clientignore(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientunignore"))	{ if (canUse(ent, qtrue)) cmd_clientignore(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"permignore"))		{ if (canUse(ent, qtrue)) cmd_permignore(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"permunignore"))	{ if (canUse(ent, qtrue)) cmd_permignore(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"permclientignore"))	{ if (canUse(ent, qtrue)) cmd_permclientignore(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"permclientunignore"))	{ if (canUse(ent, qtrue)) cmd_permclientignore(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"ban"))			{ if (canUse(ent, qtrue)) cmd_banGuid(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"ban"))			{ if (canUse(ent, qtrue)) cmd_banClientGuid(ent); else cantUse(ent); return qtrue;}	
	else if (!strcmp(cmd,"tempban"))		{ if (canUse(ent, qtrue)) cmd_tempbanGuid(ent); else cantUse(ent); return qtrue;}	
	else if (!strcmp(cmd,"banip"))			{ if (canUse(ent, qtrue)) cmd_banIp(ent); else cantUse(ent); return qtrue;}		
	else if (!strcmp(cmd,"tempbanip"))		{ if (canUse(ent, qtrue)) cmd_tempBanIp(ent); else cantUse(ent); return qtrue;}		
	else if (!strcmp(cmd,"addip"))			{ if (canUse(ent, qtrue)) cmd_addIp(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"event"))			{ if (canUse(ent, qtrue)) cmd_Event(ent); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd,"speclock"))		{ if (canUse(ent, qtrue)) cmd_specHandle(ent, qtrue); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd,"specunlock"))		{ if (canUse(ent, qtrue)) cmd_specHandle(ent, qfalse); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "lock"))			{ if (canUse(ent, qtrue)) cmd_handleTeamLock(ent, qtrue); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "unlock"))		{ if (canUse(ent, qtrue)) cmd_handleTeamLock(ent, qfalse); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "pause"))			{ if (canUse(ent, qtrue)) cmd_pauseHandle(ent, qtrue); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "unpause"))		{ if (canUse(ent, qtrue)) cmd_pauseHandle(ent, qfalse); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "allready"))		{ if (canUse(ent, qtrue)) cmd_readyHandle(ent, qfalse); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "unreadyall"))	{ if (canUse(ent, qtrue)) cmd_readyHandle(ent, qtrue); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "start"))			{ if (canUse(ent, qtrue)) cmd_tourney(ent, qtrue); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "stop"))			{ if (canUse(ent, qtrue)) cmd_tourney(ent, qfalse); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "forceclan"))		{ if (canUse(ent, qtrue)) cmd_forceClan(ent); else cantUse(ent); return qtrue; }
	else if (!strcmp(cmd, "force"))			{ cmd_force(ent); return qtrue; }

	// Any other command (server cvars..)
	else if (canUse(ent, qfalse))			{ cmdCustom(ent, cmd); return qtrue; }	

	// It failed on all checks..
	else { CP( va("print \"^1Error^7: Command ^1%s ^7was not found^1!\n\"", cmd )); return qfalse; }
}

/*
===========
Admin help
===========
*/
typedef struct {
	char *command;
	char *help;
	char *usage;
} helpCmd_reference_t;

#define _HELP(x,y,z) {x, y, z},
/**
 * Fairly straight forward approach _HELP(COMMAND, DESCRIPTION, USAGE) 
 * Alternatively, usage can be empty.  
 * Add new as needed..
 */
static const helpCmd_reference_t helpInfo[] = {
	_HELP("help", "Prints help about specific command.", "?COMMAND")
	_HELP("login", "Logs you in as Administrator.", NULL)
	_HELP("@login", "Silently logs you in as Administrator.", NULL)
	_HELP("logout", "Removes you from Administrator status.", NULL)
	_HELP("incognito", "Toggles your Admin status from hidden to visible or other way around.", NULL)
	_HELP("list_cmds", "Shows all available commands for your level.", NULL)
	_HELP("commands", "Shows all available commands for your level.", NULL)
	_HELP("cmds", "Shows all available commands for your level.", NULL)
	_HELP("kick", "Kicks player from server.", "Uses unique part of name! Optionally you can add a message.")
	_HELP("clientkick", "Kicks player from server.", "Uses client slot number! Optionally you can add a message.")
	_HELP("slap", "Slaps player and takes 20hp.", "Uses client slot number!")
	_HELP("kill", "Kills player on spot.", "Uses client slot number!")
	_HELP("fling", "Disorientates player's view.", "Uses unique part of name!")
	_HELP("throw", "Throws player randomly.", "Uses unique part of name!")
	_HELP("launch", "Launches player in air.", "Uses unique part of name!")
	_HELP("specs", "Forces player to spectators.", "Uses unique part of name!")
	_HELP("axis", "Forces player to Axis team.", "Uses unique part of name!")
	_HELP("allies", "Forces player to Allied team.", "Uses unique part of name!")
	_HELP("exec", "Executes server config file.", "You can use @ at the end to silently execute file, e.g. !exec server @")
	_HELP("nextmap", "Loads the nextmap.", NULL)
	_HELP("exec", "Executes config on a server. Note! Write full name.", "E.g. !exec server.cfg")
	_HELP("map", "Loads the map of your choice.", "!map mp_base")
	_HELP("cpa", "Center Prints Admin message to everyone.", "!cpa <msg>")
	_HELP("cp", "Center Prints Admin message to selected user.", "Uses client slot number!")
	_HELP("chat", "Shows warning message to all in global chat.", "!chat <msg>")
	_HELP("warn", "Shows warning message to all in global chat and center print.", "!warn <msg>")
	_HELP("no", "Cancels any vote in progress.", NULL)
	_HELP("yes", "Passes any vote in progress.", NULL)
	_HELP("map_restart", "Restarts the round.", NULL)
	_HELP("reset_match", "Resets the match.", NULL)
	_HELP("swap_teams", "Swaps the teams.", NULL)
	_HELP("shuffle", "Shuffles the teams- Optionally use @ to shuffle without match reset.", "!shuffle @")
	_HELP("spec999", "Moves all lagged (999) players to spectators.", NULL)
	_HELP("whereis", "Reveals players location to all.", "Uses client slot number!")
	_HELP("rename", "Renames players.", "!rename <client slot> <new name>")
	_HELP("vstr", "Loads a level from rotation file. Note - You need to know rotation labels..", "!vstr map1")
	_HELP("ignore", "Takes ability to call votes, chat and use vsay from client.", "!ignore <unique part of name>")
	_HELP("unignore", "Restores client's ability to call votes, chat and use vsay.", "!unignore <unique part of name>")
	_HELP("clientignore", "Takes ability to call votes, chat and use vsay from client.", "!ignore <client slot>")
	_HELP("clientunignore", "Restores client's ability to call votes, chat and use vsay.", "!unignore <client slot>")
	_HELP("permignore", "Permanently ignores players. Note that client will need to be permUnignored as unignore will only work until player leaves.", "!permignore <unique part of name>")
	_HELP("permunignore", "Removes client from permanently ignored list.", "!permunignore <unique part of name>")
	_HELP("permclientignore", "Permanently ignores players. Note that client will need to be permUnignored as unignore will only work until player leaves.", "!permclientignore <client slot>")
	_HELP("permclientunignore", "Removes client from permanently ignored list.", "!permclientunignore <client slot>")
	_HELP("ban", "Bans player (GUID based) from server.", "!ban <unique part of name>")
	_HELP("banclient", "Bans player (GUID based) from server.", "!banclient <client slot number>")
	_HELP("tempban", "Temporarily bans player (GUID based) from server.", "!tempban <unique part of name> <mins>")
	_HELP("banip", "Bans player by IP.", "!banip <unique part of name>")
	_HELP("tempbanip", "Temporarily Bans player by IP.", "!tempbanip <unique part of name> <mins>")
	_HELP("addip", "Adds IP to banned file. You can use wildcards for subrange bans.", "example - !addip 100.*.*.*")
	_HELP("event", "Pauses or Resumes event", "!event <pause/resume>")
	_HELP("speclock", "Locks a player's team from spectators.", "!speclock <team/both>")
	_HELP("specunlock", "Unlocks a player's team for spectators.", "!speclock <team/both>")
	_HELP("lock", "Locks team from joining.", "!lock <team/both>")
	_HELP("unlock", "Unlocks team for joining.", "!speclock <team/both>")
	_HELP("pause", "Pauses a match..", "!pause")
	_HELP("unpause", "Resumes a match.", "!unpause")
	_HELP("readyall", "Sets status of both teams as Ready and goes to countdown.", "!readyall")
	_HELP("unreadyall", "Cancels countdown and returns match back in warmup state.", "!unreadyall")
	_HELP("start", "Starts the tournament.", "!start")
	_HELP("stop", "Stops the tournament.", "!stop")
	_HELP("forceclan", "Forces all players that matches query to a selected team.", "!forceclan <r/b/s> <match>")
	_HELP("*", "Any default command that's allowed per Admin level can be executed accordingly. Note that adding @ at the end will execute it silently otherwise it will be printed to all.", "!g_allowVote 1 or !g_allowVote 1 @ for silent change")
	// --> Add new ones after this line..

	{NULL, NULL, NULL}
};

qboolean do_help(gentity_t *ent) {	
	char alt[128];
	char cmd[128];
	unsigned int i, \
		aHelp=ARRAY_LEN( helpInfo );
	const helpCmd_reference_t *hCM;
	qboolean wasUsed=qfalse;

	parseCmds(ent->client->pers.cmd1, alt, cmd, qtrue);

	for ( i = 0; i < aHelp; i++ ) {
		hCM = &helpInfo[i];
		if ( NULL != hCM->command && 0 == Q_stricmp( cmd, hCM->command ) ) {
			CP(va("print \"%s %s %s\n\"", 
				va(hCM->usage ? "^3Help ^7:" : "^3Help^7:"), 
				hCM->help,
				va("%s", (hCM->usage ? va("\n^3Usage^7: %s\n", hCM->usage) : "")) ));
			wasUsed = qtrue;
		}
	}
	return wasUsed;
}

/*
===========
Commands
===========
*/
qboolean cmds_admin(char cmd[MAX_TOKEN_CHARS], gentity_t *ent) {	

	// We're dealing with command
	if ( Q_stricmp( cmd, "!" ) == 0 ) {
		return do_cmds(ent);
	} 
	// We're dealing with help
	else if ( Q_stricmp( cmd, "?" ) == 0 ) { 
		return do_help(ent);
	}

return qfalse;
}

