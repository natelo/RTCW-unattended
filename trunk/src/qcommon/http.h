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
#define WEB_MOTD		"http://" MOTD_SERVER_NAME "/stats/query/motd"
#define WEB_AUTH		"http://" AUTHORIZE_SERVER_NAME	"/stats/query/auth"
#define WEB_CLIENT_AUTH "http://" CLIENT_AUTH_SERVER_NAME "/stats/query/client"
#define WEB_UPDATE		"http://" UPDATE_SERVER_NAME "/stats/query/update"

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
