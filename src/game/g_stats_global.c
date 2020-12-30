/*
===========================================================================
L0 - g_http_stats.c
All essential WebStats functionality resides here.

Created: 29.07 / 14
Last Updated: /
===========================================================================
*/
#include "g_local.h"


/*
==============================
stats_addEntry

Linked list
==============================
*/
void stats_addEntry(char* entry) {
	struct statEntry *node;

	node = (struct statEntry *)malloc(sizeof(struct statEntry));
	Q_strncpyz(node->info, entry, sizeof(node->info));

	if (statHead == NULL) {
		statHead = node;
	}
	else {
		statTail->next = node;
	}
	statTail = node;
	node->next = NULL;
}

/*
============
Convert MODs
============
*/
statsMODs MODtoStats(meansOfDeath_t mod) {
	switch (mod) {
	case MOD_MP40:
		return STATS_MP40;
	case MOD_THOMPSON:
		return STATS_THOMPSON;
	case MOD_STEN:
		return STATS_STEN;
	case MOD_MAUSER:
		return STATS_MAUSER;
	case MOD_SNIPERRIFLE:
		return STATS_SNIPERRIFLE;
	case MOD_FLAMETHROWER:
		return STATS_FLAMETHROWER;
	case MOD_ROCKET_SPLASH:
		return STATS_PANZERFRAUST;
	case MOD_VENOM:
		return STATS_VENOM;
	case MOD_GRENADE_PINEAPPLE:
		return STATS_GRENADE;
	case MOD_LUGER:
		return STATS_LUGER;
	case MOD_COLT:
		return STATS_COLT;
	case MOD_DYNAMITE_SPLASH:
		return STATS_DYNAMITE;
	case MOD_MACHINEGUN:
		return STATS_MG42;
	case MOD_KNIFE2:
		return STATS_KNIFE;
	case MOD_KNIFE_STEALTH:
		return STATS_KNIFESTEALTH;
	case MOD_KNIFETHROW:
		return STATS_KNIFETHROW;
	case MOD_AIRSTRIKE:
		return STATS_AIRSTRIKE;
	case MOD_ARTILLERY:
		return STATS_ARTILLERY;
	case MOD_POISONED:
		return STATS_POISON;
	case MOD_GOOMBA:
		return STATS_GOOMBA;
	case MOD_FALLING:
		return STATS_FALLING;
	case MOD_MORTAR_SPLASH:
		return STATS_MORTAR;
	case MOD_SELFKILL:
		return STATS_SUICIDE;
	case MOD_SUICIDE:
		return STATS_SUICIDE;
	case MOD_CHICKEN:
		return STATS_CHICKEN;
	case MOD_WATER:
		return STATS_DROWN;
	case MOD_TRIGGER_HURT:
		return STATS_WORLD;
	case MOD_CRUSH:
		return STATS_WORLD;
	case MOD_ADMIN:
		return STATS_ADMIN;
	default:
		return STATS_MAX;
	}
}

/*
========================================
stat_mpWeapToStatsWeap

Takes WP_* data to output STATS_* data
========================================
*/
statsMODs weaponStats(weapon_t weapon) {
	switch (weapon) {
	case WP_MP40:
		return STATS_MP40;
	case WP_THOMPSON:
		return STATS_THOMPSON;
	case WP_STEN:
		return STATS_STEN;
	case WP_MAUSER:
		return STATS_MAUSER;
	case WP_SNIPERRIFLE:
		return STATS_SNIPERRIFLE;
	case WP_FLAMETHROWER:
		return STATS_FLAMETHROWER;
	case WP_PANZERFAUST:
		return STATS_PANZERFRAUST;
	case WP_VENOM:
		return STATS_VENOM;
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		return STATS_GRENADE;
	case WP_LUGER:
		return STATS_LUGER;
	case WP_COLT:
		return STATS_COLT;
	case WP_KNIFE:
	case WP_KNIFE2:
		return STATS_KNIFE;
	case WP_ARTY:
		return STATS_ARTILLERY;
	case WP_SMOKE_GRENADE:
	case WP_MEDIC_SYRINGE:
		return STATS_POISON;
	case WP_DYNAMITE:
	case WP_DYNAMITE2:
		return STATS_DYNAMITE;
	case WP_AMMO:
	case WP_MEDKIT:
	// Manually forced stuff (FYI: default = 47 max..)
	case 48: 
		return STATS_MG42;
	default:
		return STATS_MAX;
	}
}

/*
============
Store client's MOD (means of death)

NOTE:
- Called from g_combat.c
- Main structure resides in g_stats.h
============
*/
void globalStats_writeMOD(gentity_t *victim, meansOfDeath_t MOD) {	

	MOD = MODtoStats(MOD);
	if (MOD < STATS_MAX) {
		victim->client->pers.MODs[MOD].count = victim->client->pers.MODs[MOD].count + 1;
	}
	return;
}

/*
============
Store Client's "hit list"

NOTE: Index is client slot, we keep a track of their (per slot) guid
	  so we can compare it with new entry. If entry doesn't match
	  we flush it to a file and start from zero. This approach is
	  prone to collisions in "NO_GUID" scenarios but we really
	  do not want to run mod with no guid support anyway..
============
*/
extern void client_flushHitList(gentity_t *ent, int slot);
void globalStats_hitList(gentity_t *victim, gentity_t *attacker) {
	int killer = attacker->client->ps.clientNum;

	if (victim->client->pers.hitList[killer].count > 0) {
		if (!Q_stricmp(victim->client->pers.hitList[killer].guid, attacker->client->sess.guid)) {
			victim->client->pers.hitList[killer].count = victim->client->pers.hitList[killer].count + 1;
		}
		else {
			client_flushHitList(victim, killer);

			victim->client->pers.hitList[killer].count = victim->client->pers.hitList[killer].count + 1;
			Q_strncpyz(victim->client->pers.hitList[killer].guid, attacker->client->sess.guid, sizeof(victim->client->pers.hitList[killer].guid));
		}
	}
	else {
		victim->client->pers.hitList[killer].count = victim->client->pers.hitList[killer].count + 1;		
		Q_strncpyz(victim->client->pers.hitList[killer].guid, attacker->client->sess.guid, sizeof(victim->client->pers.hitList[killer].guid));
	}
}

/*
============
Tracks client's classes (how many spawns)
============
*/
void globalStats_playerClass(int client, int type) {
	gentity_t *ent = &g_entities[client];

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || 
		g_gamestate.integer != GS_PLAYING)
		return;

	ent->client->pers.playerClass[type].count++;
}

/*
============
Timers for clients class/team
============
*/
void globalStats_playerTimers(gclient_t *client) {
	int cClass = client->sess.playerType;

	client->pers.playerClass[cClass].time++;

	// Adds to Spy counter
	if (client->ps.isSpy && client->ps.stats[STAT_HEALTH] > 0) {
		client->pers.playerClass[4].time++;
	}
}

/*
============
Timers for clients class/team
============
*/
void globalStats_statsTimers(gclient_t *client) {	
	client->pers.statsTimers.time++;

	if (client->pers.statsTimers.time > client->pers.statsTimers.alivePeak)
		client->pers.statsTimers.alivePeak = client->pers.statsTimers.time;
}

/*
============
Record overall playing time..
============
*/
void globalStats_overallTimer(void) {
	int i;

	for (i = 0; i < level.maxclients; i++){
		if (level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if ((level.clients[i].sess.sessionTeam == TEAM_RED || level.clients[i].sess.sessionTeam == TEAM_BLUE)
			&& level.time > level.oneSecActions) 
		{
			level.clients[i].pers.statsTimers.overall++;
			// Team Timers
			if (level.clients[i].sess.sessionTeam == TEAM_RED)
				level.clients[i].pers.statsTimers.axisTeam++;
			else
				level.clients[i].pers.statsTimers.alliedTeam++;
		}
	}
}

/*
============
Dump rename directly to structure
============
*/
void globalStats_rename(gclient_t *client, char *name) {
	char *data;

	// Warmup renames are usually tied to animation spam fest...
	if (g_gamestate.integer != GS_PLAYING)
		return;

	data = va("rename\\%s\\%s", client->sess.guid, Q_CharReplace(name, '&', '~'));

	// We do not care which round or when as we trap
	// server id on web part and build alias list on it's own..
	stats_addEntry(data);
}

/*
============
Track weapon shots  per weapon
============
*/
void globalStats_weaponShots(gentity_t *ent, int sWeapon) {
	statsMODs weapon;

	weapon = weaponStats(sWeapon);	
	if (weapon < STATS_MAX)
		ent->client->pers.stats.wShotsFired[weapon]++;
}

/*
============
Track weapon hits per weapon
============
*/
void globalStats_weaponHits(gentity_t *attacker, gentity_t *target, int mod, qboolean headshot) {
	statsMODs weapon;
	
	weapon = MODtoStats(mod);
	if (weapon < STATS_MAX) {
		attacker->client->pers.stats.wShotsHit[weapon]++;
		target->client->pers.stats.wShotsRec[weapon]++;

		if (headshot) {
			attacker->client->pers.stats.wHeadshots[weapon]++;
			target->client->pers.stats.wHeadshotsRec[weapon]++;
		}		
	}
}

/*
============
Track damage stats per weapon
============
*/
void globalStats_damageStats(gentity_t *attacker, gentity_t *target, int mod, int dmg, qboolean onSameTeam) {
	statsMODs weapon;

	// Fix door..gives 10k damage.. 
	if (mod == MOD_TELEFRAG)
		dmg = 100;

	weapon = MODtoStats(mod);
	if (weapon < STATS_MAX && target && target->client && target->health > 0) {
		if (onSameTeam) {
			if (attacker->client)
				attacker->client->pers.stats.wTDmgGvn[weapon] += dmg;
			if (target->client)
				target->client->pers.stats.wTDmgRcv[weapon] += dmg;
		}
		else {
			if (attacker->client)
				attacker->client->pers.stats.wDmgGvn[weapon] += dmg;
			if (target->client)
				target->client->pers.stats.wDmgRcv[weapon] += dmg;
		}
	}
}

/*
============
Track Gibs for weapons
============
*/
void globalStats_gibStats(gentity_t *attacker, gentity_t *target, int uWeapon) {
	statsMODs weapon;

	weapon = MODtoStats(uWeapon);
	if (weapon < STATS_MAX) {
		attacker->client->pers.stats.wGibs[weapon]++;
		target->client->pers.stats.wGibsBy[weapon]++;
	}
}

/*
============
Dumps in file that will be later send to web (stats) server
============
*/
void globalStats_dump( void ) {
	struct statEntry *node;
	FILE *statsDump;

	statsDump = fopen(WEBSTATS_LOG, "a+"); 	
	while (statHead != NULL) {
		node = statHead;
		fputs(va("%s\n", node->info), statsDump);
		statHead = statHead->next;
	}
	fclose(statsDump);
}

/*
============
Get any custom gametype (deathmatch, goldrush..)
============
*/
int altGameType(void) {
	if (g_deathMatch.integer)
		return 1;
	else
		return 0;
}

/*
============
Dumps event into file so it can be parsed on Web side of things..
============
*/
void globalStats_logEvent(void) {
	if (Q_stricmp(g_httpEvent.string, "") && Q_stricmp(g_httpEvent.string, "none") && !g_httpEventPaused.integer)
		stats_addEntry(va("event\\%s", Q_CharReplace(g_httpEvent.string, '&', '~')));
}

/*
============
Generates unique round token so clients can be remaped..
============
*/
void globalStats_roundToken( void ) {
	if (!level.roundID)
		level.roundID = va("%s", getTime(qtrue));

	stats_addEntry(va("stamp:%s", getTime(qtrue)));
}

/*
============
Builds round stats
============
*/
char *buildRound( void ) {
	char mapName[64];

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	return va("round\\%s\\%s\\%s\\%i\\%i\\%i\\%i\\%s\\%s\\%s\\%s\\%s\\%s",
		getTime(qfalse),
		GAMEVERSION,
		mapName,
		g_currentRound.integer,
		g_gametype.integer,
		altGameType(),
		level.winningTeam,
		level.firstBloodAttacker,
		level.firstBloodVictim,
		level.firstHeadshotAttacker,
		level.firstHeadshotVictim,
		level.lastBloodAttacker,
		level.lastBloodVictim
	);
}

/*
============
Flushes single hitList entry 

Called when guid (per slot) doesn't match so we 
need to start a new count
============
*/
void client_flushHitList(gentity_t *ent, int slot) {
	char data[2048];

	Q_strncpyz(data, va("hitlist\\%s", ent->client->sess.guid), sizeof(data));
	Q_strcat(data, sizeof(data), va("\\%s:%d", ent->client->pers.hitList[slot].guid, ent->client->pers.hitList[slot].count));

	stats_addEntry(data);

	memset(&ent->client->pers.hitList[slot], 0, sizeof(ent->client->pers.hitList[slot]));
}

/*
============
Builds client hit list (players who killed him)
============
*/
void client_hitList(gentity_t *ent) {
	int i;
	char data[2048];
	qboolean addEntry = qfalse;

	Q_strncpyz(data, va("hitlist\\%s", ent->client->sess.guid), sizeof(data));
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (ent->client->pers.hitList[i].count > 0) {
			Q_strcat(data, sizeof(data), va("\\%s:%d", ent->client->pers.hitList[i].guid, ent->client->pers.hitList[i].count));

			if (!addEntry)
				addEntry = qtrue;
		}
	}

	if (addEntry)
		stats_addEntry(data);
}

/*
============
Builds client MODs stats
============
*/
void client_buildMODList(gentity_t *ent) {
	int i;
	char data[2048];
	qboolean addEntry = qfalse;

	Q_strncpyz(data, va("mods\\%s", ent->client->sess.guid), sizeof(data));
	for (i = 0; i < STATS_MAX; i++) {
		if (ent->client->pers.MODs[i].count > 0) {
			Q_strcat(data, sizeof(data), va("\\%d:%d", i, ent->client->pers.MODs[i].count));

			if (!addEntry)
				addEntry = qtrue;
		}
	}
	
	if (addEntry)
		stats_addEntry(data);
}

/*
============
Builds client MODs stats
============
*/
void client_buildWeaponStats(gentity_t *ent) {
	char data[2048];
	int i;
	qboolean addEntry = qfalse;

	Q_strncpyz(data, va("weaponStats\\%s", ent->client->sess.guid), sizeof(data));
	for (i = 0; i < STATS_MAX; i++) {
		if (
			ent->client->pers.stats.wShotsFired[i] > 0 ||
			ent->client->pers.stats.wShotsHit[i] > 0 ||
			ent->client->pers.stats.wShotsRec[i] > 0 ||
			ent->client->pers.stats.wHeadshots[i] > 0 ||
			ent->client->pers.stats.wHeadshotsRec[i] > 0 ||
			ent->client->pers.stats.wDmgGvn[i] > 0 ||
			ent->client->pers.stats.wDmgRcv[i] > 0 ||
			ent->client->pers.stats.wTDmgGvn[i] > 0 ||
			ent->client->pers.stats.wTDmgRcv[i] > 0 || 
			ent->client->pers.stats.wGibs[i] > 0 ||
			ent->client->pers.stats.wGibsBy[i] > 0
		) {		
			Q_strcat(data, sizeof(data), va("\\%i %i %i %i %i %i %i %i %i %i %i %i",
				i,
				ent->client->pers.stats.wShotsFired[i],
				ent->client->pers.stats.wShotsHit[i],
				ent->client->pers.stats.wShotsRec[i],
				ent->client->pers.stats.wHeadshots[i],
				ent->client->pers.stats.wHeadshotsRec[i],
				ent->client->pers.stats.wDmgGvn[i],
				ent->client->pers.stats.wDmgRcv[i],
				ent->client->pers.stats.wTDmgGvn[i],
				ent->client->pers.stats.wTDmgRcv[i],
				ent->client->pers.stats.wGibs[i],
				ent->client->pers.stats.wGibsBy[i]
			));

			if (!addEntry)
				addEntry = qtrue;
		}
	}

	if (addEntry)
		stats_addEntry(data);
}

/*
============
Builds client stats
============
*/
char *client_buildStats(gentity_t *ent) {	
	return va("\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d",
		ent->client->pers.stats.kills,
		ent->client->pers.stats.deaths,
		ent->client->pers.stats.headshots,
		ent->client->pers.stats.teamKills,
		ent->client->pers.stats.poison,
		ent->client->pers.stats.revives,
		ent->client->pers.stats.revivesRec,
		ent->client->pers.stats.ammoGiv,
		ent->client->pers.stats.ammoRec,
		ent->client->pers.stats.medGiv,
		ent->client->pers.stats.medRec,		
		ent->client->pers.stats.gibs,
		ent->client->pers.stats.suicides,
		ent->client->pers.stats.goomba,
		ent->client->pers.stats.knife,
		ent->client->pers.stats.knifeStealth,
		ent->client->pers.stats.knifeThrow,
		ent->client->pers.stats.killPeak,
		ent->client->pers.stats.deathPeak,
		ent->client->pers.stats.shotsFired,
		ent->client->pers.stats.shotsHit,
		ent->client->pers.stats.dynoPlanted,
		ent->client->pers.stats.dynoDisarmed,
		ent->client->pers.stats.mgsRepaired,
		ent->client->pers.stats.ASCalled,
		ent->client->pers.stats.ASThrown,
		ent->client->pers.stats.ASBlocked,
		ent->client->pers.stats.chickenRun,
		ent->client->pers.stats.dmgTeam,
		ent->client->pers.stats.dmgGiv,
		ent->client->pers.stats.dmgRec,
		ent->client->pers.stats.objSteals,
		ent->client->pers.stats.flagCapture,
		ent->client->pers.stats.flagReclaim
	);
}

/*
============
Builds client timer stats
============
*/
char *client_buildTimerStats(gentity_t *ent) {

	return va("\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%i\\%d\\%d\\%d\\%d",
		// Class Stats
		(ent->client->pers.playerClass[0].count > 0 ? ent->client->pers.playerClass[0].count : 0),
		(ent->client->pers.playerClass[0].time > 0 ? ent->client->pers.playerClass[0].time : 0),
		(ent->client->pers.playerClass[1].count > 0 ? ent->client->pers.playerClass[1].count : 0),
		(ent->client->pers.playerClass[1].time > 0 ? ent->client->pers.playerClass[1].time : 0),
		(ent->client->pers.playerClass[2].count > 0 ? ent->client->pers.playerClass[2].count : 0),
		(ent->client->pers.playerClass[2].time > 0 ? ent->client->pers.playerClass[2].time : 0),
		(ent->client->pers.playerClass[3].count > 0 ? ent->client->pers.playerClass[3].count : 0),
		(ent->client->pers.playerClass[3].time > 0 ? ent->client->pers.playerClass[3].time : 0),
		/*(ent->client->pers.playerClass[4].time > 0 ? ent->client->pers.playerClass[4].time : 0),*/
		0,	// FIXME - SPY Stats are wrong...
		// Team stats
		(ent->client->pers.statsTimers.axisTeam > 0 ? ent->client->pers.statsTimers.axisTeam : 0),
		(ent->client->pers.statsTimers.alliedTeam > 0 ? ent->client->pers.statsTimers.alliedTeam : 0),
		// General stats
		(ent->client->pers.statsTimers.overall > 0 ? ent->client->pers.statsTimers.overall : 0),
		(ent->client->pers.statsTimers.alivePeak > 0 ? ent->client->pers.statsTimers.alivePeak : 0)
	);
}

/*
============
Builds client general info
============
*/
char *client_buildGeneral(gentity_t *ent) {

	return va("client\\%s\\%d.%d.%d.%d\\%s\\%d\\%d\\%d%s",
		ent->client->sess.guid,
		ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], ent->client->sess.ip[3],
		Q_CharReplace(ent->client->pers.netname, '&', '~'),
		ent->client->sess.sessionTeam,
		ent->client->ps.ping,
		ent->client->ps.persistant[PERS_SCORE],
		client_buildTimerStats(ent)
	);
}

/*
============
Builds client data and dumps it in file
============
*/
void stats_clientData(gentity_t *ent) {
	char *data;

	data = va("%s%s",		
		client_buildGeneral(ent),
		client_buildStats(ent)
	);

	stats_addEntry(data);
}

/*
============
Dumps in file all the details
============
*/
void globalStats_clientDisconnect(gentity_t *ent) {
	char *data;

	// We only want game stats..
	if (!g_gamestate.integer == GS_PLAYING)
		return;
	
	// Build MODs
	client_buildMODList(ent);
	// Build HitList
	client_hitList(ent);
	// Weapon Stats
	client_buildWeaponStats(ent);	

	// Build all the data..
	data = va("%s%s",
		client_buildGeneral(ent),
		client_buildStats(ent)
	);

	stats_addEntry(data);
}

/*
==========================
Wipe it

Clears/frees linked lists
==========================
*/
void globalStats_cleanList(void) {
	struct statEntry *node;

	while (statHead != NULL) {
		node = statHead;
		statHead = statHead->next;
		free(node);
	}
}

/*
============
Dump stats now
============
*/
void globalStats_buildStats(void) {
	int i;

	// Dump round info
	stats_addEntry(buildRound());

	for (i = 0; i < g_maxclients.integer; i++) {		
		if (level.clients[i].pers.connected != CON_CONNECTED) 
			continue;

		stats_clientData(&g_entities[i]);		
		client_buildMODList(&g_entities[i]);		
		client_hitList(&g_entities[i]);
		client_buildWeaponStats(&g_entities[i]);
	}

	// Dump (any) Event info
	globalStats_logEvent();
}
