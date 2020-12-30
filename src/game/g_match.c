/*
===========================================================================
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 8.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"

/*
=================
FlagBalance

Checks if teams are uneven and favours one with less to claim last take-retake 
till teams even up again, then it unlocks so it can be (re)claimed again.
=================
*/
int FlagBalance( void ) {

	if (!g_balanceFlagRetake.integer)
		return 0;

	// Even or difference by 1
	if (level.axisPlayers == level.alliedPlayers)
		return 0;
	// Axis (less)
	else if ((level.alliedPlayers - level.axisPlayers) > 1)
		return 1;
	// Allied (less)
	else if ((level.axisPlayers - level.alliedPlayers) > 1)
		return 2;
	// We don't know what happen...
	else
		return 0;
}

/*
================
Selected weapon
================
*/
void setCustomMG( gentity_t* ent, int type ) {

	if (!g_customMGs.integer) 
	{
		CP("cp \"Custom MGs are ^3disabled ^7on this server!\n\"2");
		return;
	}

	if ((ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ||
		ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER ) ||
		ent->client->sess.sessionTeam == TEAM_SPECTATOR) {

		if (type == 1) {
			if (ent->client->sess.selectedWeapon == WP_MP40)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3MP40^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_MP40;
			}

		} else if (type == 2) {
			if (ent->client->sess.selectedWeapon == WP_THOMPSON)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3THOMPSON^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_THOMPSON;
			}

		} else if (type == 3) {
			if (ent->client->sess.selectedWeapon == WP_STEN)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3STEN^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_STEN;
			}
		} 

	// Lt's and soliders can't choose weapons..
	} else {
		CP("cp \"Only Medics and Engineers can choose custom SMG!\n\"2");
	}
}

/*
=================
Weapon limiting

See if player can spawn with weapon...
TODO: Add first in line...
=================
*/
int sortWeaponLimit(int weap) {

	if (weap == 6) {		
		if (g_maxTeamSniper.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamSniper.integer;
	}

	if (weap == 8) {		
		if (g_maxTeamPF.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamPF.integer;
	}

	if (weap == 9) {	
		if (g_maxTeamVenom.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamVenom.integer;
	}

	if (weap == 10) {		
		if (g_maxTeamFlamer.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamFlamer.integer;
	}

	return g_maxclients.integer;
}

int isWeaponLimited( gclient_t *client, int weap ) {
	int count=0;

	// Limit
	if (( weap == 6 ) && ( client->pers.restrictedWeapon != WP_MAUSER ) )
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisSniper : level.alliedSniper;
	else if (( weap == 8 ) && ( client->pers.restrictedWeapon != WP_PANZERFAUST ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisPF : level.alliedPF;
	else if (( weap == 9 )  && ( client->pers.restrictedWeapon != WP_VENOM ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisVenom : level.alliedVenom;
	else if (( weap == 10 ) && ( client->pers.restrictedWeapon != WP_FLAMETHROWER ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer : level.alliedFlamer;

	if (count >= sortWeaponLimit(weap))
		return 1;
	else
		return 0;

	return 0;	
}

/*
=================
Weapon balance

Checks if there's enough of players per team so player
can spawn with heavy weapon..
=================
*/
qboolean isWeaponBalanced( int weapon ) {

	// Sniper
	if (g_balanceSniper.integer && (weapon == 6)) {
		if ((level.axisPlayers >= g_balanceSniper.integer) && (level.alliedPlayers >= g_balanceSniper.integer))
			return qtrue;
		else
			return qfalse;	
	// PF check
	} else if (g_balancePF.integer && (weapon == 8)) {
		if ((level.axisPlayers >= g_balancePF.integer) && (level.alliedPlayers >= g_balancePF.integer))
			return qtrue;
		else
			return qfalse;
	// Venom check
	} else if (g_balanceVenom.integer && (weapon == 9)) {
		if ((level.axisPlayers >= g_balanceVenom.integer) && (level.alliedPlayers >= g_balanceVenom.integer))
			return qtrue;
		else
			return qfalse;
	// Flamer check
	} else if (g_balancePF.integer && (weapon == 10)) {
		if ((level.axisPlayers >= g_balanceFlamer.integer) && (level.alliedPlayers >= g_balanceFlamer.integer))
			return qtrue;
		else
			return qfalse;	
	}
	return qtrue;
}

/*
================
Default weapon 

Accounts for "selected weapon" as well.
================
*/
///////////
// Deals only with soldier for weapon restrictions (To avoid breaking anything..).
void setDefWeap(gclient_t *client, int clips) {
	if (client->sess.sessionTeam == TEAM_RED)
	{	
		COM_BitSet(client->ps.weapons, WP_MP40);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * clips);
		client->ps.weapon = WP_MP40;
	} else {
		COM_BitSet(client->ps.weapons, WP_THOMPSON);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * clips);
		client->ps.weapon = WP_THOMPSON;
	}
}

// NOTE: Selected weapons only works for eng and med..sold and lt can pick their weapons already..
//       so setting it can potentialy overlap with client spawn scripts..
void setDefaultWeapon(gclient_t *client, qboolean isSold) {
	int ammo;

	// This deals with weapon restrictions.
	if (isSold)  {
		setDefWeap(client, g_soldierClips.integer);
		return;
	}

	// Sort any bit flags..
	// NOTE: If client during game types e.g. /thompson they will spawn with it during server session..
	if (client->sess.clientFlags & CFLAGS_MP40 && g_customMGs.integer && !client->sess.selectedWeapon)
		client->sess.selectedWeapon = WP_MP40;
	else if (client->sess.clientFlags & CFLAGS_THOMPSON  && g_customMGs.integer && !client->sess.selectedWeapon)
		client->sess.selectedWeapon = WP_THOMPSON;
	else if (client->sess.clientFlags & CFLAGS_STEN  && g_customMGs.integer && !client->sess.selectedWeapon)
		client->sess.selectedWeapon = WP_STEN;

	// Sorts ammo
	ammo = (client->sess.selectedWeapon == WP_THOMPSON) ? 30 : 32;

	// Medic
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_medicClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
		return;
		} else {
			setDefWeap(client, g_medicClips.integer);
		return;
		}
	}

	// Engineer
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_engineerClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
		return;
		} else {
			setDefWeap(client, g_engineerClips.integer);
		return;
		}
	}	
}

/*
=================
Chicken Run

Originally from etPUB (i think)
=================
*/
gentity_t *G_FearCheck( gentity_t *ent ) {
	qboolean fear = qfalse;
	gentity_t *attacker = &level.gentities[ent->client->lasthurt_client];

	if(g_chicken.integer && attacker && attacker->client &&
		(level.time - ent->client->lasthurt_time) < g_chicken.integer &&
		attacker->client->sess.sessionTeam != ent->client->sess.sessionTeam &&
		attacker->health > 0) {

		fear = qtrue;
	}

	return (fear ? attacker : NULL);
}

/*
===========
Global sound
===========
*/
void APSound(char *sound){
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);  
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t *ent, char *sound){
	gentity_t *te;	

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t *ent, char *sound){
	gentity_t   *te;

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
}

/*
=================
AutoBalanceTeam

Checks if any team needs balancing
=================
*/
void checkEvenTeams( void ) {
	if (!g_teamAutoBalance.integer ||		
		(g_gamestate.integer != GS_PLAYING) ||		
		!g_teamForceBalance.integer ||	
		g_needBalance.integer ||
		g_deathMatch.integer)
	return;

	sortedActivePlayers();

	if ((level.alliedPlayers - level.axisPlayers) > 1){
		trap_Cvar_Set( "g_needBalance", "1" );		
		AP("chat \"console: ^1Axis ^7team will be balanced in ^115^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;
	}

	if ((level.axisPlayers - level.alliedPlayers) > 1) {
		trap_Cvar_Set( "g_needBalance", "1" );		
		AP("chat \"console: ^4Allied ^7team will be balanced in ^415^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;	
	}
}

void balanceTeams( void ) {
	int lowScoreClient = -1;
	int lowScore=0;
	int i;

	while ( ((level.axisPlayers - level.alliedPlayers) > 1) || 
			((level.alliedPlayers - level.axisPlayers) > 1) ) {

		sortedActivePlayers();

		lowScoreClient = -1;

		if ((level.axisPlayers - level.alliedPlayers) > 1) {

			for (i=0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_RED)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				} else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam( &g_entities[lowScoreClient], "b", qtrue);			
			AP(va("chat \"console: %s ^7was forced to ^4Allies ^7to balance the teams.\n\"",
				level.clients[lowScoreClient].pers.netname));
		}

		if ((level.alliedPlayers - level.axisPlayers) > 1) {

			for (i=0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_BLUE)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				} else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam( &g_entities[lowScoreClient], "r", qtrue);			
			AP(va("chat \"console: %s ^7was forced to ^1Axis ^7to balance the teams.\n\"", 
				level.clients[lowScoreClient].pers.netname));
		}
	}
	trap_Cvar_Set( "g_needBalance", "0" );
}

/*
=================
Countdown 

Causes some troubles on client side so done it here.
=================
*/
void CountDown( void ) {
	char *index="";

	if (level.cnStarted == qfalse) {
		return;
	}
		// Countdown...
	if (level.cnNum == 0)
		{ index = "prepare.wav"; AP( "cp \"Prepare to fight^2!\n\"2" );}	
	if (level.cnNum == 1) 
		{ index = "cn_5.wav"; /*AP( "cp \"Match starts in: ^25\n\"2" );*/}
	if (level.cnNum == 2) 
		{ index = "cn_4.wav"; /*AP( "cp \"Match starts in: ^24\n\"2" );*/}	
	if (level.cnNum == 3) 
		{ index = "cn_3.wav"; /*AP( "cp \"Match starts in: ^23\n\"2" );*/}	
	if (level.cnNum == 4)
		{ index = "cn_2.wav"; /*AP( "cp \"Match starts in: ^22\n\"2" );*/}
	if (level.cnNum == 5) 
		{ index = "cn_1.wav"; /*AP( "cp \"Match starts in: ^21\n\"2" );*/}
	if (level.cnNum == 6 ) 
		{ index = "fight.wav"; AP( "print \"^2Fight!\n\""); }	

	// Prepare to fight takes 2 seconds..
	if(level.cnNum == 0) {
		level.cnPush = level.time+2000;

		// Auto shuffle if enabled and treshhold is reached
		if (g_autoShuffle.integer)
		{
			// Do not bother with this if there's not at least 6 players
			if (level.axisPlayers > 2 && level.alliedPlayers > 2) {
				if (shuffleTracking.integer >= g_autoShuffle.integer)
				{
					trap_SendConsoleCommand(EXEC_APPEND, "shuffle @print\n");
					trap_Cvar_Set("shuffleTracking", 0);
					AP("chat \"console: Teams were ^3Auto shuffled^7!\n\"");
				}
				// Notify that shuffle will occur next round..
				else if ((g_autoShuffle.integer > 2) &&
					(shuffleTracking.integer == (g_autoShuffle.integer - 1)))
				{
					AP("chat \"^3Notice: ^7Teams will be ^3Auto Shuffled ^7next round^3!\n\"");
				}
			}
		}

		// Ensure this is always set in Tournament..
		if (g_tournamentMode.integer == TOURNY_FULL && int_match_started.integer) {
			// Spec Lock
			teamInfo[TEAM_RED].spec_lock = qtrue;
			teamInfo[TEAM_BLUE].spec_lock = qtrue;

			// Team Lock
			teamInfo[TEAM_RED].team_lock = qtrue;
			teamInfo[TEAM_BLUE].team_lock = qtrue;
		}

	// Just enough to fix the bug and skip to action..
	} else if (level.cnNum == 6) {
		level.cnPush = level.time+200;
	// Otherwise, 1 second.
	} else {
		level.cnPush = level.time+1000;  
	} 
	
	// We're done.. restart the game
	if (level.cnNum == 7) {
		
		level.warmupTime += 10000;
		trap_Cvar_Set( "g_restarted", "1" );
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;			
		
		return;
	}
		
	if (level.clients->pers.connected == CON_CONNECTED) 
		APS(va("xmod/sound/scenaric/countdown/%s", index));

	level.cnNum++; 
}

/*
=================
Match Info

Basically just some info prints..
=================
*/
// Gracefully taken from s4ndmod :p
char *GetLevelTime( void ) {
	int Objseconds,Objmins,Objtens;

	Objseconds = ( ( ( g_timelimit.value * 60 * 1000 ) - ( ( level.time - level.startTime ) ) ) / 1000 ); //begin martin - this line was a bitch :-)
	Objmins = Objseconds / 60;
	Objseconds -= Objmins * 60;
	Objtens = Objseconds / 10;
	Objseconds -= Objtens * 10;

	if ( Objseconds < 0 ) {
		Objseconds = 0;
	}
	if ( Objtens < 0 ) {
		Objtens = 0;
	}
	if ( Objmins < 0 ) {
		Objmins = 0;
	}

	return va( "%i:%i%i",Objmins,Objtens,Objseconds );  //end martin
}
// Prints stuff
void matchInfo( unsigned int type, char *msg ) {

	// End of Match info
	if (type == MT_EI)
	{
		if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
			if ( g_currentRound.integer == 1 ) {  
				AP( va( "print \"*** ^3Clock set to: ^7%d:%02d\n\"",
						g_nextTimeLimit.integer,
						(int)( 60.0 * (float)( g_nextTimeLimit.value - g_nextTimeLimit.integer ) ) ) );
			} else {
				float val = (float)( ( level.timeCurrent - ( level.startTime + level.time - level.intermissiontime ) ) / 60000.0 );
				if ( val < g_timelimit.value ) 
				{
					AP( va( "print \"*** ^3Objective reached at ^7%d:%02d ^3(original: ^7%d:%02d^3)\n\"",
						(int)val,
						(int)( 60.0 * ( val - (int)val ) ),
						g_timelimit.integer,
						(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) ) );
				} else {
					AP( va( "print \"*** ^3Objective NOT reached in time (^7%d:%02d^3)\n\"",
						g_timelimit.integer,
						(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) ) );
				}
			}
		}
	}
	// Match events
	else if ( type == MT_ME )
	{	
		if (g_printMatchInfo.integer)
			AP(va("print \"[%s] ^3%s \n\"", GetLevelTime(), msg));
		else
			AP(va("cp \"%s \n\"1", msg));
	}
}

/*
=================
L0 - Max lives

Basically it's an enhanced version of Xian's code.

Now it tracks lives, it accounts for a fact that client
may drop during game so in that case upon reconnection
they get same amount of lives they got when they left.

If they try to evade max lives check by reconnecting they 
will be able to enter the game but wont be able to join any team
until round ends. 

NOTE 1:
	Clients can still be forced to team by admin..they'll just end up with 1 life.
NOTE 2:
	If g_allowLateJoiners is enabled, it will check how much lives would client got
	by default and how much (s)he had when (s)he left. It will use the lowest value.
=================
*/
typedef struct GUID_s
{
	char	guid[PB_GUID_LENGTH];
	int		axisLives;
	int		alliedLives;
} GUID_t;

#define MAX_GUIDS 1024
static GUID_t	lifeGUIDs[MAX_GUIDS];
static int		lifeEntries = 0;

// Adds guid to the list
void AddMaxLivesGUID( char *guid )
{
	if (lifeEntries == MAX_GUIDS)
	{
		G_Printf ("[MaxLives] Structure is full!\n");
		return;
	}
	Q_strncpyz (lifeGUIDs[lifeEntries].guid, guid, PB_GUID_LENGTH);	
	lifeGUIDs[lifeEntries].axisLives = -2;
	lifeGUIDs[lifeEntries].alliedLives = -2;
	lifeEntries++;
}

// Checks if entry exists
void CheckMaxLivesGUID( char *guid )
{
	int		i;
	for (i=0 ; i < lifeEntries ; i++)
	{	
		if ( !Q_stricmp (lifeGUIDs[i].guid, guid ) )
		{				
			return;
		}
	}
	AddMaxLivesGUID( guid );
	return;
}

// Guid was found on the list...sort lives
int SortMaxLivesGUID( gentity_t *ent )
{
	int		i;
	char	*guid = ent->client->sess.guid;
	int		team = ent->client->sess.sessionTeam;
	int		calculate = CalculateLives( ent );
	int		bounce = 0;

	for (i=0 ; i < lifeEntries ; i++)
	{		
		if ( !Q_stricmp (lifeGUIDs[i].guid, guid ) )
		{	
			int axis = lifeGUIDs[i].axisLives; 
			int allied = lifeGUIDs[i].alliedLives;	

			// Make sure player didn't wasted all lifes on one team and tries to join other
			if ((axis == 0) || (allied == 0))
			{
				ent->client->pers.evadingMaxLives = qtrue;
				return 0;				
			}

			if (team == TEAM_RED)
			{
				if (axis != -2) // Prefer lower
					return ( g_allowLateJoiners.integer ? ((calculate > axis) ? axis : calculate) : axis );
				else if (g_allowLateJoiners.integer)
					return calculate;
				// Nothing..go to bottom (gives all lives..)
			}
			else if (team == TEAM_BLUE)
			{
				if (allied != -2) // Prefer lower
					return ( g_allowLateJoiners.integer ? ((calculate > allied) ? allied : calculate) : allied );
				else if (g_allowLateJoiners.integer)
					return calculate;
				// Nothing..go to bottom (gives all lives..)
			}
		}
	}

	// We came so far...assume it's start of the round or whatever.
	if (g_maxlives.integer)
		bounce = (g_allowLateJoiners.integer ? calculate : g_maxlives.integer-1);
	
	if (ent->client->sess.sessionTeam == TEAM_RED && g_axismaxlives.integer)	
		bounce = (g_allowLateJoiners.integer ? calculate : g_axismaxlives.integer-1);
	else if (ent->client->sess.sessionTeam == TEAM_BLUE && g_alliedmaxlives.integer)
		bounce = (g_allowLateJoiners.integer ? calculate : g_alliedmaxlives.integer-1);

	return bounce;
}

// Track lives
void TrackMaxLivesGUID( char *guid, int lives, int team )
{
	int		i;
	for (i=0 ; i < lifeEntries ; i++)
	{
		if ( !Q_stricmp (lifeGUIDs[i].guid, guid ) )
		{
			if (team == TEAM_RED )
				lifeGUIDs[i].axisLives = lives;
			else
				lifeGUIDs[i].alliedLives = lives;
		}
	}
	return;
}

// Clean the list
void ClearMaxLivesGUID ( void )
{
	int	i;	
	for (i=0 ; i < lifeEntries ; i++ ) {
		lifeGUIDs[i].guid[0] = '\0';		
		lifeGUIDs[i].axisLives = -2;
		lifeGUIDs[i].alliedLives = -2;
	}
	lifeEntries = 0;
}

// checks if player can join
qboolean canJoinMaxLives( gentity_t *ent )
{
	int check = -1;

	if (g_allowLateJoiners.integer)
	{	
		check = ( !g_handleLateJoiners.integer ? CalculateLives( ent ) : SortMaxLivesGUID( ent ));
	}
	else
	{
		if (g_handleLateJoiners.integer)
		{
			check = SortMaxLivesGUID( ent );
		}
	}

	return ( (check == 0) ? qfalse : qtrue);
}

/*
=================
Calcuate lives

Basically checks how far into a match are we and sets lives for any new 
player according to it.

'L0 :: I don't get it..for some1 that barely counts without stones I sure do a lot of math lately.
=================
*/
int CalculateLives(gentity_t *ent)
{
	int round = (g_timelimit.integer * 60);
	int passed = ((( g_timelimit.value * 60 * 1000 ) - (( level.time - level.startTime ))) / 1000 );	
	float perc = 0;	
	int take = 0;
	int result = 0;
	int lives = 0;

	// Sort team and value
	if (g_maxlives.integer)
		lives = g_maxlives.integer;

	// Owerwrite maxlives if axis/allied is set
	if (ent->client->sess.sessionTeam == TEAM_RED && g_axismaxlives.integer)	
		lives = g_axismaxlives.integer;
	else if (ent->client->sess.sessionTeam == TEAM_BLUE && g_alliedmaxlives.integer)
		lives = g_alliedmaxlives.integer;

	// Do the math
	perc = (100 - (((float)passed/ (float)round) * 100));	
	take = ((perc / 100) * lives);
	result = lives - take;
	// Open a beer

	// No specific reason but just to be sure..
	return ((result > 0) ? (result-1) : 1);	// Give 1 life if at the end (XXX: Keep an eye on this).
}

/*
=================
Match settings

Pretty much a dump from et..
=================
*/
void G_loadMatchGame(void) {
	unsigned int i, dwBlueOffset, dwRedOffset;
	unsigned int aRandomValues[MAX_REINFSEEDS];
	char strReinfSeeds[MAX_STRING_CHARS];

	// Set up the random reinforcement seeds for both teams and send to clients
	dwBlueOffset = rand() % MAX_REINFSEEDS;
	dwRedOffset = rand() % MAX_REINFSEEDS;
	strcpy(strReinfSeeds, va("%d %d", (dwBlueOffset << REINF_BLUEDELT) + (rand() % (1 << REINF_BLUEDELT)),
		(dwRedOffset << REINF_REDDELT) + (rand() % (1 << REINF_REDDELT))));

	for (i = 0; i < MAX_REINFSEEDS; i++) {
		aRandomValues[i] = (rand() % REINF_RANGE) * aReinfSeeds[i];
		strcat(strReinfSeeds, va(" %d", aRandomValues[i]));
	}

	level.dwBlueReinfOffset = 1000 * aRandomValues[dwBlueOffset] / aReinfSeeds[dwBlueOffset];
	level.dwRedReinfOffset = 1000 * aRandomValues[dwRedOffset] / aReinfSeeds[dwRedOffset];

	trap_SetConfigstring(CS_REINFSEEDS, strReinfSeeds);
}

/*
=================
Pause

Deals with pause related functionality
=================
*/
void G_delayPrint(gentity_t *dpent) {
	int think_next = 0;
	qboolean fFree = qtrue;

	switch (dpent->spawnflags) {
	case DP_PAUSEINFO:
	{
		if (level.match_pause > PAUSE_UNPAUSING) {
			int cSeconds = match_timeoutlength.integer * 1000 - (level.time - dpent->timestamp);

			if (cSeconds > 1000) {
				think_next = level.time + 1000;
				fFree = qfalse;

				if (cSeconds > 30000) {
					AP(va("popin \"Timeouts Available: [^1Axis^7] %d - [^4Allies^7] %d\n\"y",
						teamInfo[TEAM_RED].timeouts, teamInfo[TEAM_BLUE].timeouts));
				}
			}
			else {
				APS("xmod/sound/scenaric/countdown/prepare.wav");
				level.match_pause = PAUSE_UNPAUSING;
				G_spawnPrintf(DP_UNPAUSING, level.time + 7.2, NULL);
			}
		}
		break;
	}

	case DP_UNPAUSING:
	{
		if (level.match_pause == PAUSE_UNPAUSING) {
			int cSeconds = 8 * 1000 - (level.time - dpent->timestamp);

			if (cSeconds > 1000) {
				think_next = level.time + 1000;
				fFree = qfalse;
				APS(va("xmod/sound/scenaric/countdown/cn_%d.wav", cSeconds / 1000));
			}
			else {
				level.match_pause = PAUSE_NONE;
				AP("print \"^1FIGHT!\n\"");
				APS("xmod/sound/scenaric/countdown/fight.wav");
				trap_SetConfigstring(CS_PAUSED, "0");
				trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime + level.timeDelta));
			}
		}
		break;
	}
	default:
		break;
	}

	dpent->nextthink = think_next;
	if (fFree) {
		dpent->think = 0;
		G_FreeEntity(dpent);
	}
}

static char *pszDPInfo[] = {
	"DPRINTF_PAUSEINFO",
	"DPRINTF_UNPAUSING",
	"DPRINTF_CONNECTINFO",
	"DPRINTF_MVSPAWN",
	"DPRINTF_UNK1",
	"DPRINTF_UNK2",
	"DPRINTF_UNK3",
	"DPRINTF_UNK4",
	"DPRINTF_UNK5"
};

void G_spawnPrintf(int print_type, int print_time, gentity_t *owner) {
	gentity_t   *ent = G_Spawn();

	ent->classname = pszDPInfo[print_type];
	ent->clipmask = 0;
	ent->parent = owner;
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->s.eType = ET_ITEM;

	ent->spawnflags = print_type;
	ent->timestamp = level.time;

	ent->nextthink = print_time;
	ent->think = G_delayPrint;

	// Set it here so client can do it's own magic..
	if (print_type == DP_PAUSEINFO)
		trap_SetConfigstring(CS_PAUSED, va("%d", match_timeoutlength.integer));
	else if (print_type == DP_UNPAUSING)
		trap_SetConfigstring(CS_PAUSED, "10000");
}


/*
=================
Tournament handling..

This will auto sort stuff if we are running tourney mode at 2..
NOTE: Will also handle web stats data later on..
=================
*/
void G_TourneyHandle(qboolean dReset, qboolean dClear) {

	if (g_tournamentMode.integer != TOURNY_FULL) {
		return;
	}

	if (dReset) {
		trap_Cvar_Set("int_match_started", "0");		
		trap_Cvar_Set("g_ignoreSpecs", "0");

		if (dClear) {
			G_teamReset(TEAM_BLUE, qtrue);
			G_teamReset(TEAM_RED, qtrue);
		}

		if (!teamInfo[TEAM_RED].team_score && !teamInfo[TEAM_BLUE].team_score)
			return;		
		else if (teamInfo[TEAM_RED].team_score > teamInfo[TEAM_BLUE].team_score)
			AP("chat \"^1AXIS ^7have ^3WON ^7the Match!\n");
		else
			AP("chat \"^4ALLIES ^7have ^3WON ^7the Match!\n");
	}
	else {
		// This is only used to start it...
		if (int_match_started.integer) {
			return;
		}

		trap_Cvar_Set("int_match_started", "1");
		trap_Cvar_Set("g_ignoreSpecs", "1");
		trap_Cvar_Set("g_doWarmup", "1");

		// Reset it
		G_teamReset(TEAM_BLUE, qtrue);
		G_teamReset(TEAM_RED, qtrue);

		// Set it now..
		trap_SendConsoleCommand(EXEC_APPEND, va("reset_match"));
	}
}

/*
=================
Checks current state of the tournament..
=================
*/
void G_TourneyState(void) {
	// We're done..
	if (match_rounds.integer <= (teamInfo[TEAM_RED].team_score + teamInfo[TEAM_BLUE].team_score)) {
		G_TourneyHandle(qtrue, qtrue);
	}
	else {
		G_parseTourneyInfo(qtrue);
	}
}

/*
=================
Send score to client so they can refresh HUD info when needed..
=================
*/
void G_parseTourneyInfo(qboolean refresh) {
	char *tourney;

	if (!g_tournamentMode.integer || 
		(level.toureyInfoSent && !refresh) 
	) {
		return;
	}
	else {
		level.toureyInfoSent = qtrue;
	}

	// L0 - Tourney stuff
	tourney = (va("%d %d %d %d %d %d %i",
			match_rounds.integer,
			match_timeoutcount.integer,
			teamInfo[TEAM_RED].timeouts,
			teamInfo[TEAM_BLUE].timeouts,
			teamInfo[TEAM_RED].team_score,
			teamInfo[TEAM_BLUE].team_score,
			int_match_started.integer
			/* Add more if needed */
		)
	); // ~L0
	
	// Specs and players will sort stuff on their side..
	AP(va("tourneyinfo %s", tourney));
}
