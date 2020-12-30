/*
 * g_admin_system.c
 *
 * L0 - Deals with processing bans, AB (admin bot) system etc.
 * Created: 7th.Sept/13
 * Updated: 
 *
 * NOTE: 
 * Ported from wolfX.
 *
 * NOTE 2:
 * Most of core banning system is derived from old S4NDMoD source (old banning system).
 * It's a time tested solution and proves to be a solid approach.
 * For the record - A while ago I did ask Martin for permission to re-use some of the source. :)
 */
#include "g_local.h"

// OSX really doesn't like it in g_local.h..
char *TempBannedMessage;

/*
===========
Banned (file check version)

Took this from old S4NDMoD source as it simply works (time saver). 
===========
*/
void BreakIP(const char *IP, char *charip1, char* charip2, char* charip3, char* charip4){
	int i = 0, a=0, b=0, c=0, d=0;
	int foundperiod=0;

	while(IP[i] != 0){
		if(!foundperiod){//havent found any periods
			if(IP[i] == '.'){
				foundperiod = 1;
				charip1[a]=0;
			}else
				charip1[a]=IP[i];
			i++;
			a++;
		}else if(foundperiod == 1 ){ // We have found 1 period
			if(IP[i] == '.'){
				foundperiod = 2;
				charip2[b]=0;
			}else
				charip2[b]=IP[i];
			i++;
			b++;
		}else if(foundperiod == 2 ){ // We have found 2 periods
			if(IP[i] == '.'){
				foundperiod = 3;
				charip3[c]=0;
			}else
				charip3[c]=IP[i];
			i++;
			c++;
		}else if(foundperiod == 3 ){ // We have found 3 periods
			if(IP[i] == '.'){
				foundperiod = 4;
				charip4[d]=0;
			}else
				charip4[d]=IP[i];
			i++;
			d++;
		}
	}
}

/*
===========
Tempbanned 

Port from old S4NDMoD (time saver)

AvPMOD one is more confusing and has potential
to cause problems in current situation so I'm 
re-using old S4NDMoD's one as I'm to lazy to write new one.
===========
*/
void TEMPBAN_CLIENT(gentity_t *ent, const int minsbanned)
{
	FILE	*tempbannedfile;	
	time_t rawtime;
	time ( &rawtime );  

	tempbannedfile=fopen("tempbanned.txt","a+");//Open file
	fputs(va("%i.%i.%i.%i %i\n", 
		ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], 
		ent->client->sess.ip[3], (int)((rawtime - 1103760000)+(minsbanned*60))), tempbannedfile);	
	fclose(tempbannedfile);
}

void rewrite_tempbanned(void)
{
	FILE		*bannedfile;
	FILE		*tempfile;
	char		tempbannedips[64];

	bannedfile=fopen("tempbanned.txt","w+");
	tempfile=fopen("tempbannedtemp.txt","r+");

	while (fgets(tempbannedips,64,tempfile)!=NULL)
	{
		fputs(tempbannedips,bannedfile);
	}

	fclose(bannedfile); 
	fclose(tempfile);
	remove("tempbannedtemp.txt");
}

void write_tempbannedtemp(void)
{
	FILE		*bannedfile;
	FILE		*tempfile;
	char		tempbaninfo[64];
	char		tempbannedip[39];
	int			tempbannedsec;
	time_t		rawtime;
	time		( &rawtime ); 

	bannedfile=fopen("tempbanned.txt","r+");

	if(bannedfile){
		tempfile=fopen("tempbannedtemp.txt","w+");

		if (tempfile) {
			while (fgets(tempbaninfo,64,bannedfile)!=NULL)
			{
				sscanf(tempbaninfo,"%s %i",tempbannedip, &tempbannedsec);
				if( (tempbannedsec-(rawtime - 1103760000)) > 0 )
					fputs(tempbaninfo,tempfile);
			}

			fclose(tempfile);
		}

		fclose(bannedfile); 
		rewrite_tempbanned();
	}
}

void clean_tempbans(void)
{
	write_tempbannedtemp();
}

qboolean TempBanned(char * Clientip){

	FILE		*tempbannedfile;
	char		tempbannedip[39];
	int			tempbannedsec;
	int			hours;
	int			tens;
	int			mins;
	int			seconds;
	char		tempbannedinfo[1024];
	char		*sortTime;
	char		*theTime;
	time_t		rawtime;
	time		( &rawtime );  

	tempbannedfile=fopen("tempbanned.txt","r");//Open file

	if (tempbannedfile){//if there is a tembbannedfile
		while (fgets(tempbannedinfo,1024,tempbannedfile)!=NULL){
			sscanf(tempbannedinfo,"%s %i", tempbannedip, &tempbannedsec);

			if	(!Q_stricmp (Clientip, tempbannedip) && (tempbannedsec-(rawtime - 1103760000)) > 0 ) {
					fclose(tempbannedfile);
					seconds = (tempbannedsec - (rawtime - 1103760000));
					mins = seconds / 60;
					hours = mins / 60;	
					seconds -= mins * 60;
					tens = seconds / 10;
					seconds -= tens * 10;
					mins -= hours * 60;
					sortTime = (hours > 0) ? ((mins > 9) ? va("%i:", hours) : va("%i:0", hours)  ) : "";
					theTime = va("%s%i:%i%i",sortTime, mins,tens,seconds);				
					TempBannedMessage = va("^7You Are ^3Temporarily Banned ^7for ^3%s", theTime );
					return qtrue;
				}	
		}

		fclose(tempbannedfile);
	}

	return qfalse;
}

/*
===========
Password bypass

Basically we re-use existing stuff..
===========
*/
qboolean bypassing (char *password) {

	// check for password first
	if (strlen(g_password.string))	{
		char *token, *text;
		text = g_password.string;

		while (1)
		{
			token = COM_Parse(&text);
			if (!token || !token[0])
				break;
			
				if (!Q_stricmp(password, token))
					return qtrue;
				else 
					return qfalse;
		}
	}
	return qfalse;
}

/*
===========
Banned (file check version)

Took this from old S4NDMoD source as it simply works (time saver). 
===========
*/
qboolean Banned(char * Clientip, char * password)
{
	FILE * bannedfile;
	char		ip1[10];
	char		ip2[10];
	char		ip3[10];
	char		ip4[10];
	char		banip1[10];
	char		banip2[10];
	char		banip3[10];
	char		banip4[10];
	char		bannedips[1024];
	Q_strncpyz ( ip1, "", sizeof( ip1 ) );
	Q_strncpyz ( ip2, "", sizeof( ip2 ) );
	Q_strncpyz ( ip3, "", sizeof( ip3 ) );
	Q_strncpyz ( ip4, "", sizeof( ip4 ) );

	BreakIP(va("%s\n",Clientip),ip1,ip2,ip3,ip4);	
	bannedfile=fopen("banned.txt","r");

	if (bannedfile){
		while(fgets(bannedips,1024,bannedfile)!=NULL){
			Q_strncpyz ( banip1, "", sizeof( banip1 ) );
			Q_strncpyz ( banip2, "", sizeof( banip2 ) );
			Q_strncpyz ( banip3, "", sizeof( banip3 ) );
			Q_strncpyz ( banip4, "", sizeof( banip4 ) );
	
			BreakIP(bannedips, banip1, banip2, banip3, banip4);			

			if (!Q_stricmp (banip1, ip1) || !Q_stricmp (banip1, "*")){
				if (!Q_stricmp (banip2, ip2) || !Q_stricmp (banip2, "*")){
					if (!Q_stricmp (banip3, ip3) || !Q_stricmp (banip3, "*")){
						if (!Q_stricmp (banip4, ip4) || !Q_stricmp (banip4, "*\n")){	

							fclose(bannedfile);
							if (bypassing(password)) 
								return qfalse;
							else 
								return qtrue;
						}
					}
				}
			}
		}
		fclose(bannedfile);
	}
	return qfalse;
}

/*
===========
Guid bans (file check version)

Guid based bans, no bypass is required.
===========
*/
qboolean guid_banning(char * guid)
{
	FILE *bannedfile;
	char guids[1024];
	char bannedGuid[32];

	if (!Q_stricmp (guid, "")) 
		return qfalse;

	bannedfile=fopen("bannedGuids.txt","r");

	if (bannedfile){
		while(fgets(guids,1024,bannedfile)!=NULL){
			sscanf(guids,"%s",bannedGuid);
			
			if (!Q_stricmp (bannedGuid, guid)) {				
				fclose(bannedfile);
			return qtrue;	
			}
		}
		fclose(bannedfile);
	}
	return qfalse;
}

/*
===========
Tempbanned Guid

Rip from s4ndmod's ip based tempbanning.
===========
*/
void TEMPBAN_GUID_CLIENT(char *guid, const int minsbanned)
{
	FILE	*tempbannedfile;
	time_t rawtime;
	time ( &rawtime );	

	tempbannedfile=fopen("tempbannedGuids.txt","a+");//Open file
	fputs(va("%s %i\n", guid, (int)((rawtime - 1103760000)+(minsbanned*60))), tempbannedfile);		
	fclose(tempbannedfile);
}

void rewrite_tempbanned_guids(void)
{
	FILE		*bannedfile;
	FILE		*tempfile;
	char		tempbannedGuids[256];

	bannedfile=fopen("tempbannedGuids.txt","w+");
	tempfile=fopen("tempbannedGuidsTemp.txt","r+");

	while (fgets(tempbannedGuids,64,tempfile)!=NULL)
	{
		fputs(tempbannedGuids,bannedfile);
	}

	fclose(bannedfile); 
	fclose(tempfile);
	remove("tempbannedGuidsTemp.txt");
}

void write_tempbannedtemp_guids( void )
{
	FILE		*bannedfile;
	FILE		*tempfile;
	char		tempbaninfo[64];
	char		tempbannedGuid[32];
	int			tempbannedsec;
	time_t		rawtime;
	time		( &rawtime ); 

	bannedfile=fopen("tempbannedGuids.txt","r+");

	if(bannedfile){
		tempfile=fopen("tempbannedGuidsTemp.txt","w+");

		if (tempfile) {
			while (fgets(tempbaninfo,64,bannedfile)!=NULL)
			{
				sscanf(tempbaninfo,"%s %i",tempbannedGuid, &tempbannedsec);
				if( (tempbannedsec-(rawtime - 1103760000)) > 0 )
					fputs(tempbaninfo,tempfile);
			}

			fclose(tempfile);
		}

		fclose(bannedfile); 
		rewrite_tempbanned_guids();
	}
}

void clean_tempbans_guids(void)
{
	write_tempbannedtemp_guids();
}


qboolean TempBanned_guids(char * Guid){

	FILE		*tempbannedfile;
	char		tempbannedGuid[32];
	int			tempbannedsec;
	int			hours;
	int			tens;
	int			mins;
	int			seconds;
	char		tempbannedinfo[1024];
	char	    *theTime;
	char		*sortTime;
	time_t		rawtime;
	time		( &rawtime );  

	tempbannedfile=fopen("tempbannedGuids.txt","r");//Open file

	if (tempbannedfile){//if there is a tembbannedfile
		while (fgets(tempbannedinfo,1024,tempbannedfile)!=NULL){
			sscanf(tempbannedinfo,"%s %i", tempbannedGuid, &tempbannedsec);

			if	(!Q_stricmp (Guid, tempbannedGuid) && (tempbannedsec-(rawtime - 1103760000)) > 0 ) {
					fclose(tempbannedfile);
					seconds = (tempbannedsec - (rawtime - 1103760000));
					mins = seconds / 60;
					hours = mins / 60;	
					seconds -= mins * 60;
					tens = seconds / 10;
					seconds -= tens * 10;
					mins -= hours * 60;
					sortTime = (hours > 0) ? ((mins > 9) ? va("%i:", hours) : va("%i:0", hours)  ) : "";
					theTime = va("%s%i:%i%i",sortTime, mins,tens,seconds);				
					TempBannedMessage = va("^7You Are ^3Temporarily Banned ^7for ^3%s", theTime );
					return qtrue;
				}		
		}		

		fclose(tempbannedfile);
	}

	return qfalse;
}

/*
===========
Check banned

Checks if user is banned or tempbanned..
===========
*/
int checkBanned(char *data, char * password, qboolean guid) {

	// IPs
	if (!guid) {
		if (Banned(data, password))
			return 1;		
		if (TempBanned(data)) 
			return 2;

	// GUIDs
	} else {
		if (guid_banning(data))
			return 3;
		if (TempBanned_guids(data))
			return 4;		
	}
	return 0;
}

/*
===========
Adds GUID to ignored list..
===========
*/
void IGNORE_CLIENT(char *guid)
{
	FILE	*ignoredfile;	

	ignoredfile=fopen("ignored.txt","a+");	//Open file
	fputs(va("%s\n", guid), ignoredfile);		
	fclose(ignoredfile);
}

/*
===========
Rewrite ignored with new data..
===========
*/
void rewrite_ignored_guids(void)
{
	FILE		*ignoredfile;
	FILE		*tempfile;
	char		tempignoredGuids[256];

	ignoredfile=fopen("ignored.txt","w+");
	tempfile=fopen("tempignored.txt","r+");

	while (fgets(tempignoredGuids,64,tempfile)!=NULL)
	{
		fputs(tempignoredGuids,ignoredfile);
	}

	fclose(ignoredfile); 
	fclose(tempfile);
	remove("tempignored.txt");
}

/*
===========
Temp ignored list
===========
*/
void write_tempignored( char *guid )
{
	FILE		*ignoredfile;
	FILE		*tempfile;
	char		tempignoredInfo[64];
	char		tempignoredGuid[32];

	ignoredfile=fopen("ignored.txt","r+");

	if(ignoredfile){
		tempfile=fopen("tempignored.txt","w+");

		if (tempfile) {
			while (fgets(tempignoredInfo,64,ignoredfile)!=NULL)
			{
				sscanf(tempignoredInfo,"%s",tempignoredGuid);
				if( !Q_stricmp (tempignoredGuid, guid) )
					fputs(tempignoredInfo,tempfile);
			}

			fclose(tempfile);
		}

		fclose(ignoredfile); 
		rewrite_ignored_guids();
	}
}

/*
===========
Checks if user is on ignored list
===========
*/
qboolean client_ignored(char * guid)
{
	FILE *ignoredfile;
	char guids[1024];
	char ignoredGuid[32];

	if (!Q_stricmp (guid, "")) 
		return qfalse;

	ignoredfile=fopen("ignored.txt","r");

	if (ignoredfile){
		while(fgets(guids,1024,ignoredfile) != NULL){
			sscanf(guids,"%s",ignoredGuid);
			
			// We found a match..
			if (!Q_stricmp (ignoredGuid, guid)) {
				fclose(ignoredfile);
			return qtrue;	
			}
		}
		fclose(ignoredfile);
	}
	return qfalse;
}

/*
===========
Removes ignored guid from the list

NOTE: Not really a cheap function and sh!t load of bouncing data around..
===========
*/
void UNIGNORE_CLIENT(char *guid)
{
	if (!Q_stricmp (guid, "")) 
		return;

	write_tempignored(guid);

	return;
}

/*
===========
Check Ignored
===========
*/
int checkIgnored(char *guid) {	
	return (client_ignored(guid) ? qtrue : qfalse);
}

/****/
/******************* SB (Server Bot) system ************************/
/****/

/*
===========
Kicks for teamkills
===========
*/
void SB_maxTeamKill( gentity_t *ent ) {	
	int count = ent->client->pers.stats.teamKills+1 - ent->client->pers.sb_TKforgiven;

	if (level.warmupTime || !sb_system.integer || sb_maxTKs.integer == (-1))
	{
		// Just count it for stats
		if (!level.warmupTime)
			ent->client->pers.stats.teamKills++;
		return;
	}

	if ( sb_maxTKs.integer - count == 1 && !ent->client->pers.sb_TKwarned) 		
	{
		AP(va("chat \"^3[WARNING]: ^7%s ^7gets kicked on next ^3Team Kill^7!\n\"", ent->client->pers.netname));
		ent->client->pers.sb_TKwarned = qtrue;
	}

	// Give them some time to make it up.. (ie tk-revive)
	ent->client->pers.sb_TKkillTime = level.time + 10000;
	ent->client->pers.stats.kills++;
	return;
}

/*
===========
Kicks for bleeding
===========
*/
void SB_maxTeamBleed( gentity_t *ent ) {	
	int count = ent->client->pers.sb_teamBleed;

	if (level.warmupTime || !sb_system.integer || sb_maxTeamBleed.integer == (-1))
		return;

	if ( sb_maxTeamBleed.integer - count == 10 ) 		
		AP(va("chat \"^3[WARNING]: ^7%s ^7is getting close to being kicked for ^3Bleeding^7!\n\"", ent->client->pers.netname));

	if ((count >= sb_maxTeamBleed.integer) && count) {

		// Tempban
		if (sb_maxTeamBleedTempbanMins.integer)
		{
			if (sb_tempbanIP.integer && IP_handling.integer)
				trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i", ent->client->ps.clientNum, sb_maxTeamBleedTempbanMins.integer ));
			else
				trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %i", ent->client->sess.guid, sb_maxTeamBleedTempbanMins.integer));	
		}

		trap_DropClient( ent-g_entities, "Kicked \n^3For Team Wounding." );	
		AP(va("chat \"^3SB^7: %s ^7got kicked for ^3Team Wounding^7.\n\"", ent->client->pers.netname));
	}
	return;
}

/*
===========
Kicks for low score
===========
*/
void SB_minLowScore( gentity_t *ent ) {	
												// Positive values are ignored.
	if (level.warmupTime || !sb_system.integer || sb_minLowScore.integer >= 0)
		return;

	if (ent->client->ps.persistant[PERS_SCORE] < sb_minLowScore.integer){ 	

		// Tempban
		if (sb_minLowScoreTempbanMins.integer)
		{
			if (sb_tempbanIP.integer && IP_handling.integer)
				trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i", ent->client->ps.clientNum, sb_minLowScoreTempbanMins.integer ));
			else
				trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %i", ent->client->sess.guid, sb_minLowScoreTempbanMins.integer));	
		}

		AP(va("chat \"^3SB^7: %s ^7got kicked for ^3Low Score^7.\n\"", ent->client->pers.netname));
		trap_DropClient(ent->client->ps.clientNum, "Kicked \n^3For Low Score.");
	}
	return;
}

/*
===========
Kicks laggers
===========
*/
void SB_maxPingFlux( gclient_t *client ) {
	int max=sb_maxPingFlux.integer;
	int times=sb_maxPingHits.integer;

	// 50 is sanity check..so someone doesn't screw it with sab_maxPingFlux "1"
	if (level.warmupTime || !sb_system.integer || max < 50 || 
		client->sess.sessionTeam != TEAM_SPECTATOR || client->ps.pm_type != PM_DEAD)
		return;
		
	if (client->pers.sb_ping >= times ) {
		trap_DropClient(client - level.clients, "Kicked \nDue unstable ping or max ping limit.");			
		AP(va("chat \"^3SB^7: %s ^7got kicked due ^3Unstable Ping^7.\n\"", client->pers.netname));
		return;

	// 1 hit per second
	} else if (client->ps.ping >= max && level.time > client->pers.sb_maxPing) {
			client->pers.sb_ping++;
			client->pers.sb_maxPing = level.time + 1000; 
		return;
	}
	return;
}

/*
===========
Censoring penalty
===========
*/
void SB_chatWarn(gentity_t *ent) {
	int n = rand() % 4; // Randomize messages
	
	if (!sb_system.integer || (sb_system.integer && !sb_censorPenalty.integer)) 
		return;

	// Only for non logged in users..
	// Chat will still get censored they just wont get kicked or ignored for it..
	if (ent->client->sess.admin != ADM_NONE) 
		return;

	if (ent->client->pers.sb_chatWarned == 0) {
		if (n == 0) 			
			CP("chat \"^3Strike one! ^7You should really wash your mouth.\n\"");
		else if (n == 1)
			CP("chat \"^3Strike one! ^7You got warned for foul language..\n\"");		
		else if (n == 2)
			CP("chat \"^3Strike one! ^7This is not your local pub..\n\"");		
		else 			
			CP("chat \"^3Strike one! ^7Cursing is not allowed here.\n\"");
	}
	else if (ent->client->pers.sb_chatWarned == 1) {
		if (n == 0) 		
			CP("chat \"^3Strike two! ^7Don't push it..\n\"");
		else if (n == 1) 			
			CP("chat \"^3Strike two! ^7You where warned..\n\"");
		else if (n == 2) 			
			CP("chat \"^3Strike two! ^7Do you talk to your parents like this?\n\"");
		else
			CP("chat \"^3Strike two! ^7Foul language is not allowed here.\n\"");			
	}
	else if (ent->client->pers.sb_chatWarned == 2) {
		if (n == 0) 
			CP("chat \"^3Strike three! ^7Last warning!\n\"");			
		else if (n == 1)
			CP("chat \"^3Strike three! ^7There wont be strike four..\n\"");			
		else if (n == 2)
			CP("chat \"^3Strike three! ^7There's no more warnings after this one.\n\"");		
		 else 
			CP("chat \"^3Strike three! ^7Care to see how strike four looks like?\n\"");
	} else {		
		if (sb_censorPenalty.integer == 1) {
			ent->client->sess.ignored = 1;
			AP(va("chat \"^3SB^7: %s ^7has been ignored due foul language^3!\n\"", ent->client->pers.netname));
		} else {

			// Tempban
			if (sb_censorPentalityTempbanMin.integer)
			{
				if (sb_tempbanIP.integer && IP_handling.integer)
					trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i", ent->client->ps.clientNum, sb_censorPentalityTempbanMin.integer ));
				else
					trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %i", ent->client->sess.guid, sb_censorPentalityTempbanMin.integer));	
			}

			AP(va("chat \"^3SB^7: %s ^7got kicked for foul language^3!\n\"", ent->client->pers.netname));
			trap_DropClient(ent-g_entities,va("^3kicked for ^3Foul ^3Language!"));
		}		
	return;
	}

	// Count it..
	ent->client->pers.sb_chatWarned++;	
	CPS(ent, "xmod/sound/client/warn.wav");

	return;
}

