/*
---------------------------
Nate 'L0 - http.h
Created: 8.11/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _S_HTTP
#define _S_HTTP

#include "../game/q_shared.h"
#include "qcommon.h"

// URL Mappings
#define WEB_MOTD MOTD_SERVER_NAME				"/stats/query/motd"
#define WEB_AUTH AUTHORIZE_SERVER_NAME			"/stats/query/auth"
#define WEB_CLIENT_AUTH CLIENT_AUTH_SERVER_NAME "/stats/query/client"
#define WEB_UPDATE UPDATE_SERVER_NAME			"/stats/query/update"

// So if it's ported it can be set here..
#define STATS_VERSION Q3_VERSION " auth"

// So it's little easier..
#define _CMD(x, y) !Q_stricmp(x, y)

/*
============
Prototypes
============
*/
//
// g_http.c
//
void HTTP_Post(char *url, char *data);
char *HTTP_PostQuery(char *url, char *data);
char *HTTP_Query(char *url);

#endif // _S_HTTP
