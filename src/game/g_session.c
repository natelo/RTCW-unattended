#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %s %i %i %i %i %i %i",		// DHM - Nerve
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.playerType,			// DHM - Nerve
		client->sess.playerWeapon,			// DHM - Nerve
		client->sess.playerItem,			// DHM - Nerve
		client->sess.playerSkin,			// DHM - Nerve
		client->sess.spawnObjectiveIndex,	// DHM - Nerve
		client->sess.latchPlayerType,		// DHM - Nerve
		client->sess.latchPlayerWeapon,		// DHM - Nerve
		client->sess.latchPlayerItem,		// DHM - Nerve
		client->sess.latchPlayerSkin,		// DHM - Nerve

		// L0 - New stuff
		client->sess.admin,
		client->sess.incognito,
		client->sess.ip[0],
		client->sess.ip[1],
		client->sess.ip[2],
		client->sess.ip[3],
		client->sess.guid &&
			( !client->sess.guid || !Q_stricmp( client->sess.guid, "" ) ) ? "NOGUID" : client->sess.guid,
		client->sess.ignored,
		client->sess.selectedWeapon,
		client->sess.clientFlags,
		client->sess.specInvited,
		client->sess.specLocked,
		client->sess.uci

		);

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
OSPx - G_ClientSwap

Client swap handling
================
*/
void G_ClientSwap(gclient_t *client) {
	int flags = 0;

	if (client->sess.sessionTeam == TEAM_RED) {
		client->sess.sessionTeam = TEAM_BLUE;
	}
	else if (client->sess.sessionTeam == TEAM_BLUE) {
		client->sess.sessionTeam = TEAM_RED;
	}

	// Swap spec invites as well
	if (client->sess.specInvited & TEAM_RED) {
		flags |= TEAM_BLUE;
	}
	if (client->sess.specInvited & TEAM_BLUE) {
		flags |= TEAM_RED;
	}

	client->sess.specInvited = flags;

	// Swap spec follows as well
	flags = 0;
	if (client->sess.specLocked & TEAM_RED) {
		flags |= TEAM_BLUE;
	}
	if (client->sess.specLocked & TEAM_BLUE) {
		flags |= TEAM_RED;
	}

	client->sess.specLocked = flags;
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	qboolean test;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %s %i %i %i %i %i %i",		// DHM - Nerve
		(int *)&client->sess.sessionTeam,
		&client->sess.spectatorTime,
		(int *)&client->sess.spectatorState,
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&client->sess.playerType,			// DHM - Nerve
		&client->sess.playerWeapon,			// DHM - Nerve
		&client->sess.playerItem,			// DHM - Nerve
		&client->sess.playerSkin,			// DHM - Nerve
		&client->sess.spawnObjectiveIndex,	// DHM - Nerve
		&client->sess.latchPlayerType,		// DHM - Nerve
		&client->sess.latchPlayerWeapon,	// DHM - Nerve
		&client->sess.latchPlayerItem,		// DHM - Nerve
		&client->sess.latchPlayerSkin,		// DHM - Nerve

		// L0 - New stuff
		(int *)&client->sess.admin,
		&client->sess.incognito,		
		(int *)&client->sess.ip[0],			
		(int *)&client->sess.ip[1],			
		(int *)&client->sess.ip[2],			
		(int *)&client->sess.ip[3],
		client->sess.guid,
		&client->sess.ignored,
		&client->sess.selectedWeapon,
		&client->sess.clientFlags,
		&client->sess.specInvited,
		&client->sess.specLocked,
		&client->sess.uci

		);

	// NERVE - SMF
	if ( g_altStopwatchMode.integer )	
		test = qtrue;
	else
		test = g_currentRound.integer == 1;

	if ( g_gametype.integer == GT_WOLF_STOPWATCH && level.warmupTime > 0 && test ) {
		if ( client->sess.sessionTeam == TEAM_RED ) {
			client->sess.sessionTeam = TEAM_BLUE;
		}
		else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			client->sess.sessionTeam = TEAM_RED;
		}
	}

	if ( g_swapteams.integer ) {
		trap_Cvar_Set( "g_swapteams", "0" );

		if ( client->sess.sessionTeam == TEAM_RED ) {
			client->sess.sessionTeam = TEAM_BLUE;
		}
		else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			client->sess.sessionTeam = TEAM_RED;
		}
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		// always spawn as spectator in team games
		sess->sessionTeam = TEAM_SPECTATOR;	
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	// DHM - Nerve
	sess->latchPlayerType = sess->playerType = 0;
	sess->latchPlayerWeapon = sess->playerWeapon = 0;
	sess->latchPlayerItem = sess->playerItem = 0;
	sess->latchPlayerSkin = sess->playerSkin = 0;
	sess->spawnObjectiveIndex = 0;
	// dhm - end

	// L0 - New stuff
	sess->admin = ADM_NONE;		// Start as non-admin
	sess->incognito = 0;		// Incognito for admins
	sess->ignored = 0;			// Starts as non-ignored (unless forced elsewhere)
	sess->selectedWeapon = 0;	// Starts with default
	sess->clientFlags = 0;		// Will be overwritten in clientConnect..
	sess->specInvited = 0;
	sess->specLocked = 0;
	sess->uci = 255;
	// End

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char s[MAX_STRING_CHARS];
	int gt;

	trap_Cvar_VariableStringBuffer("session", s, sizeof(s));
	gt = atoi(s);

	// if the gametype changed since the last session, don't use any
	// client sessions
	if (g_gametype.integer != gt) {
		level.newSession = qtrue;
		G_Printf("Gametype changed, clearing session data.\n");
	}
	// OSPx - Stats
	else {
		char *data[256];
		qboolean test = (g_altStopwatchMode.integer != 0 || g_currentRound.integer == 1);

		Q_Tokenize(s, data, " ");

		// Get team stuff		
		teamInfo[TEAM_RED].spec_lock = atoi(data[1]) ? qtrue : qfalse;
		teamInfo[TEAM_BLUE].spec_lock = atoi(data[2]) ? qtrue : qfalse;

		teamInfo[TEAM_RED].team_lock = atoi(data[3]) ? qtrue : qfalse;
		teamInfo[TEAM_BLUE].team_lock = atoi(data[4]) ? qtrue : qfalse;

		teamInfo[TEAM_RED].team_score = atoi(data[5]);
		teamInfo[TEAM_BLUE].team_score = atoi(data[6]);

		// Make sure spec locks follow the right teams
		if (g_gametype.integer == GT_WOLF_STOPWATCH && g_gamestate.integer != GS_PLAYING && test) {
			G_swapTeamLocks();
		}

		if (g_swapteams.integer) {
			G_swapTeamLocks();
		}

	} // -OSPx
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	// OSPx - Speclock
	trap_Cvar_Set("session",
		va("%i %i %i %i %i %i %i",
			g_gametype.integer,
			(teamInfo[TEAM_RED].spec_lock ? 1 : 0),
			(teamInfo[TEAM_BLUE].spec_lock ? 1 : 0),
			(teamInfo[TEAM_RED].team_lock ? 1 : 0),
			(teamInfo[TEAM_BLUE].team_lock ? 1 : 0),
			teamInfo[TEAM_RED].team_score,
			teamInfo[TEAM_BLUE].team_score
		)
	);

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
