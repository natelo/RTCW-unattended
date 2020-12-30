/*
---------------------------
Nate 'L0 - g_http_func.c
Created: 26.07/14
Updated: 05.07/14

All http client related commands.
---------------------------
*/
#include "g_local.h"

/*
===========
http_processMessage

Prints message back to public/client.
===========
*/
void http_processMessage(int clientNum, char *msg, qboolean toClient) {

	if (toClient)
		CPx(clientNum, va("print \"^5WebStats^7:%s\n", msg));
	else {
		AP(va("chat \"^5WebStats^7 requested by %s", &g_entities[clientNum].client->pers.netname));
		AP(va("chat \"^7%s\n", msg));
	}
	return;
}

/*
===========
http_sendQuery

Sends client query and reads reply.
===========
*/
void *http_sendQuery(void *args) {
	g_http_cmd_t *cmdSt = (g_http_cmd_t *)args;
	char  *out = NULL;
	char *url = va("%s/%s", g_httpStatsAPI.string, cmdSt->bit);
		
	if (g_httpStatsAPI.string) {
		out = http_Query(url, cmdSt->cmd);
	}

	if (g_httpDebug.integer > 1)
		G_Printf(va("[httpQuery] WebReply: %s\n", out));

	// Figure out what to do with reply..
	if (out)
		http_processMessage(cmdSt->id, out, cmdSt->toClient);
	else
		CPx(cmdSt->id, "print \"^3HTTP ERROR^7: Could not connect to a webserver.\n");

	// Cleanup
	free(cmdSt);	

	if (g_httpDebug.integer)
		G_Printf("Thread destroyed.\n");

	return 0;
}

/*
===========
http_clientCommand

Entry point for client commands
===========
*/
void http_clientCommand(gentity_t *ent, char *bit, char *cmd, qboolean toClient) {
	g_http_cmd_t *post_cmd = (g_http_cmd_t*)malloc(sizeof(g_http_cmd_t));

	// A cheap flood protection
	if (ent->client->pers.httpCmdIssued >= level.time)
	{
		CP(va("print \"^3Error: ^7Web commands will be available in ^3%i ^7seconds.\n", (((ent->client->pers.httpCmdIssued - level.time) / 1000) + 1)  ));

		free(post_cmd);
		return;
	}
	else {
		ent->client->pers.httpCmdIssued = level.time + 6000; // 6 sec should be enough..
	}

	// Client Stats	
	post_cmd->bit = va("%s", bit);
	post_cmd->cmd = va("%s", cmd);		
	

	// Fill the generic data..	
	post_cmd->id = ent->client->ps.clientNum;
	post_cmd->toClient = toClient;

	// Do the magic..	
	create_thread(http_sendQuery, (void*)post_cmd);

	return;
}

/*
===========
Stats Structure
===========
*/
typedef struct {
	char *command;
	void(*pCommand)(gentity_t *ent, qboolean fParam);
	qboolean fParam;
	const char *help;
	const char *usage;
	const char *extra;
} globalStats_cmds_t;

/*
	Commands..
*/
static const globalStats_cmds_t statsCmd[] = {
	{ "top",		sCmd_top,		qfalse, "Shows top player on this server", "!stats top", "" },
	{ "bottom",		sCmd_bottom,	qfalse, "Shows worst player on this server.", "!stats top", "" },
	{ "rank",		sCmd_rank,		qfalse, "Shows your or selected players rank.", "!stats rank <slot>", "Use Player's slot to get his rank" },
	{ "chances",	sCmd_chances,	qfalse, "Compares you with a player", "!stats chances <slot>", "Obtain client's slot with /getstatus command." },
	{ "info",		sCmd_info,		qfalse, "Prints your or targeted player's info stats.", "!stats info <kr/eff/acc/hs/skill> <slot>", "Slot is optional." },
	//{ "lastseen",	sCmd_lastseen,	qfalse, "Prints info when player was last seen.", "!stats lastseen <unique part of name>", "" }, // TODO: Has to be cleaned/filtered in DB..
	{ "whois",		sCmd_aliases,	qtrue,	"Prints last 10 known names of targeted client", "!stats whois <slot>", "" },
	{ NULL, NULL, qfalse, NULL, NULL }
};

/*
===========
Prints Categories
===========
*/
void sCmd_listCommands(gentity_t *ent) {
	char *cmds = "";
	unsigned int i, \
		limit= ARRAY_LEN(statsCmd);
	const globalStats_cmds_t *sCMD;

	// Build the list
	for (i = 0; i < (limit - 1) ; i++) {
		sCMD = &statsCmd[i];

		cmds = va("%s!stats %s\n", cmds, sCMD->command);
	}	

	// Print commands..
	cmds = va(
		"^3Global Stats Commands^7\n"
		"----------------------------\n"
		"%s"
		"----------------------------\n"
		"^3Tip: ^7Use ?stats <type> to view info & usage\n",
		cmds
	);
	CP(va("print \"%s\n", cmds));
}

/*
===========
http_isHttpCommand

Wrapper for client commands
===========
*/
qboolean isHttpCommand(gentity_t *ent, char *cmd, char *param, qboolean help) {
	char cmd1[128];
	char cmd2[128];
	char cmd3[128];
	char arg[MAX_SAY_TEXT];
	unsigned int i, \
		cmds = ARRAY_LEN(statsCmd);
	const globalStats_cmds_t *sCMD;
	qboolean wasUsed = qfalse;
	
	// Bail out if stuff is not set..
	if (_CMD(g_httpStatsUrl.string, "") ||
		_CMD(g_httpStatsAPI.string, "") ||
		_CMD(g_httpToken.string, "") ||
		_CMD(g_httpToken.string, "none"))
	{
		if (_CMD(g_httpToken.string, "none") || !g_httpToken.string)
			CP("print \"^3INFO^7: WebStats are disabled on this server.\n");
		else
			CP("print \"^3INFO^7: Owner of this server has not configured WebStats correctly..\n");
		return qtrue;
	}

	// Do not allow any web commands right before restart as it will crash due threads & unloading of dll..
	if (level.warmupTime && level.time > level.warmupTime - 2400 || level.intermissiontime) {
		return qtrue;
	}

	// Sort commands
	ParseAdmStr(param, cmd1, arg);
	ParseAdmStr(arg, cmd2, cmd3);
	Q_strncpyz(ent->client->pers.cmd1, cmd1, sizeof(ent->client->pers.cmd1));
	Q_strncpyz(ent->client->pers.cmd2, cmd2, sizeof(ent->client->pers.cmd2));
	Q_strncpyz(ent->client->pers.cmd3, cmd3, sizeof(ent->client->pers.cmd3));

	// Command itself is a helper..
	if (_CMD(cmd, "")) {
		sCmd_listCommands(ent);
		wasUsed = qtrue;
	} 
	else {
		for (i = 0; i < cmds; i++) {
			sCMD = &statsCmd[i];

			if (NULL != sCMD->command && _CMD(cmd, sCMD->command)) {
				// Helpers have no function..but they exist for some
				// reason, so print usage info as chances are it's help..
				if (sCMD->pCommand == NULL)
					help = qtrue;

				if (help) {				
						CP(va("print \"\n^3Command: ^7%s\n-------------------\n^3Info : ^7%s\n^3Usage: ^7%s\n%s-------------------\n\n", 
							cmd, 
							sCMD->help, 
							sCMD->usage, 
							(_CMD(sCMD->extra, "") ? "" : va("^3Extra: ^7%s\n", sCMD->extra))
						));
				}
				else {
					sCMD->pCommand(ent, sCMD->fParam);
				}
				wasUsed = qtrue;
			}
		}
	}
	return wasUsed;
}
