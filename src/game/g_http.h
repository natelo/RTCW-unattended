/*
---------------------------
Nate 'L0 - g_http.h
Created: 23.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP
#define _G_HTTP

// So if it's ported it can be set here..
#ifndef RETAIL_MOD
#define STATS_VERSION GAME_VERSION " 1.4"
#else
#define STATS_VERSION GAME_VERSION " 1.0"
#endif

// Upload is capped at 10mb..anything more @ your own risk.
#define UFILE_SIZELIMIT 10000000

// Player stats commands
#define PSTATS_GLOBAL "stats"

// So it's little easier..
#define _CMD(x, y) !Q_stricmp(x, y)

/*
============
Client commands structure
============
*/
typedef struct {
	char *bit;
	char *cmd;
	unsigned int id;
	qboolean toClient;
} g_http_cmd_t;

/*
============
Prototypes
============
*/
//
// g_http.c
//
void http_Submit(char *url, char *data);
void http_SubmitFile(char *url, char *file, qboolean wipe);
char *http_Query(char *url, char *data);

//
// g_http_stats.c
//
void globalStats_submit(void);

//
// g_http_func.c
//
void http_clientCommand(gentity_t *ent, char *bit, char *cmd, qboolean toClient);
qboolean isHttpCommand(gentity_t *ent, char *cmd, char *param, qboolean help);

//
// g_http_cmds.c
//
void sCmd_top(gentity_t *ent, qboolean fParam);
void sCmd_bottom(gentity_t *ent, qboolean fParam);
void sCmd_rank(gentity_t *ent, qboolean fParam);
void sCmd_chances(gentity_t *ent, qboolean fParam);
void sCmd_info(gentity_t *ent, qboolean fParam);
void sCmd_lastseen(gentity_t *ent, qboolean fParam);
void sCmd_aliases(gentity_t *ent, qboolean fParam);

#endif // _G_HTTP
