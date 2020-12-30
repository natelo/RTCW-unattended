/*
---------------------------
Nate 'L0 - g_http_cmds.c
Created: 06.Sept/14

All http client related commands.
---------------------------
*/
#include "g_local.h"

/*
===========
Get top player on the server

@format	cmd=dummy
@url	url/TOP
===========
*/
void sCmd_top(gentity_t *ent, qboolean fParam) {
	http_clientCommand(ent, "top", "dummy", fParam);
}

/*
===========
Get worst player on the server

@format	cmd=dummy
@url	url/bottom
===========
*/
void sCmd_bottom(gentity_t *ent, qboolean fParam) {	
	http_clientCommand(ent, "bottom", "dummy", fParam);
}

/*
===========
Get rank of a player/targeted player

@format	cmd=guid
@url	url/rank
===========
*/
void sCmd_rank(gentity_t *ent, qboolean fParam) {
	char *cmd = ent->client->pers.cmd1;

	if (_CMD(cmd, "")) {
		cmd = ent->client->sess.guid;
	}
	else if (!is_numeric(cmd) || (is_numeric(cmd) && (atoi(cmd) > g_maxclients.integer || atoi(cmd) < 0) )) {
		CP("print \"^1Error! ^7Invalid input...use a valid slot number!\n");
		return;
	}
	else {
		gentity_t *target = &g_entities[atoi(cmd)];

		if (target->client && target->client->sess.guid && target->client->pers.connected == CON_CONNECTED)
			cmd = va("%s", target->client->sess.guid);
		else {
			CP(va("print \"^1Error! ^7Slot %s is not active!\n", cmd));
			return;
		}
	}
	http_clientCommand(ent, "rank", cmd, fParam);
}

/*
===========
Calcuate win probability in a dog fight between two players

@format	cmd=guid\\target-guid
@url	url/chances
===========
*/
void sCmd_chances(gentity_t *ent, qboolean fParam) {
	char *cmd = ent->client->pers.cmd1;

	if (_CMD(cmd, "")) {
		CP("print \"^1Error: ^7Enter a slot of a player!\n");
		return;
	}	
	else if (!is_numeric(cmd) || (is_numeric(cmd) && (atoi(cmd) > g_maxclients.integer || atoi(cmd) < 0))) {
		CP("print \"^1Error! ^7Invalid input...use a valid slot number!\n");
		return;
	}
	else if (ent->client->ps.clientNum == atoi(cmd)) {
		CP("print \"^7[^4Blue Screen Of Death^7]\nSchizophrenia is not supported in this mod - Try a different slot.\n");
		return;
	}
	else {
		gentity_t *target = &g_entities[atoi(cmd)];
		
		if (target->client && 
			target->client->sess.guid && 
			target->client->pers.connected == CON_CONNECTED &&
			target->client->ps.clientNum != ent->client->ps.clientNum
		)
			cmd = va("%s\\%s", ent->client->sess.guid, target->client->sess.guid);
		else {
			CP(va("print \"^1Error! ^7Slot %s is not active!\n", cmd));
			return;
		}
	}
	http_clientCommand(ent, "chances", cmd, fParam);
}

/*
===========
Get Info of player/targeted player

@format	cmd=type\\guid
@url	url/info
===========
*/
void sCmd_info(gentity_t *ent, qboolean fParam) {
	char *cmd = ent->client->pers.cmd1;

	if (_CMD(cmd, "")) {
		cmd = "skill";
	}
	else if (_CMD(cmd, "eff")) {
		cmd = "eff";
	}
	else if (_CMD(cmd, "kr")) {
		cmd = "kr";
	}
	else if (_CMD(cmd, "acc")) {
		cmd = "acc";
	}
	else if (_CMD(cmd, "hs")) {
		cmd = "acc";
	}
	else if (_CMD(cmd, "skill")) {
		cmd = "skill";
	}
	else {
		CP(va("print \"^1Error! ^7%s is not a valid command!\n", cmd));
		return;
	}

	if (_CMD(ent->client->pers.cmd2, "")) {
		cmd = va("%s\\%s", cmd, ent->client->sess.guid);
	}
	else {
		char *tCmd = ent->client->pers.cmd2;

		if (!is_numeric(tCmd) || (is_numeric(tCmd) && (atoi(tCmd) > g_maxclients.integer || atoi(tCmd) < 0))) {
			CP("print \"^1Error! ^7Invalid input...use a valid slot number!\n");
			return;
		}
		else {
			gentity_t *target = &g_entities[atoi(tCmd)];

			if (target->client && target->client->sess.guid && target->client->pers.connected == CON_CONNECTED)
				cmd = va("%s\\%s", cmd, target->client->sess.guid);
			else {
				CP(va("print \"^1Error! ^7Slot %s is not active!\n", cmd));
				return;
			}
		}
	}
	http_clientCommand(ent, "info", cmd, fParam);
}

/*
===========
Shows Client's aliases

@format	cmd=type\\guid
@url	url/info
===========
*/
void sCmd_aliases(gentity_t *ent, qboolean fParam) {
	char *cmd = ent->client->pers.cmd1;

	if (_CMD(cmd, "")) {
		CP("print \"^1Error: ^7Enter a slot of a player!\n");
		return;
	}
	else if (!is_numeric(cmd) || (is_numeric(cmd) && (atoi(cmd) > g_maxclients.integer || atoi(cmd) < 0))) {
		CP("print \"^1Error! ^7Invalid input...use a valid slot number!\n");
		return;
	}
	else if (ent->client->ps.clientNum == atoi(cmd)) {
		CP("print \"^7[^4Blue Screen Of Death^7]\nSchizophrenia is not supported in this mod - Try a different slot.\n");
		return;
	}
	else {
		gentity_t *target = &g_entities[atoi(cmd)];

		if (target->client &&
			target->client->sess.guid &&
			target->client->pers.connected == CON_CONNECTED &&
			target->client->ps.clientNum != ent->client->ps.clientNum
			) {

			// Figure out if client is Admin and player is not..
			if (ent->client->sess.admin == ADM_NONE && target->client->sess.admin && target->client->sess.incognito) {
				CP("print \"Client has no known names.\n");
				return;
			}
			else
				cmd = va("%s\\%s", ent->client->sess.guid, target->client->sess.guid);
		}
		else {
			CP(va("print \"^1Error! ^7Slot %s is not active!\n", cmd));
			return;
		}
	}
	http_clientCommand(ent, "aliases", cmd, fParam);
}
