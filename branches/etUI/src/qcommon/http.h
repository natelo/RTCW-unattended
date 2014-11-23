/*
---------------------------
Nate 'L0 - http.h
Created: 8.11/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _S_HTTP
#define _S_HTTP

#include <curl/curl.h>
#include <curl/easy.h>
#include "../game/q_shared.h"
#include "qcommon.h"

// URL Mappings
#define WEB_MOTD		"http://" AUTHORIZE_SERVER_NAME "/stats/query/motd"
#define WEB_CLIENT_AUTH "http://" AUTHORIZE_SERVER_NAME "/stats/query/client"
#define WEB_UPLOAD		"http://" AUTHORIZE_SERVER_NAME "/stats/query/upload"
#define WEB_UPDATE		"http://" UPDATE_SERVER_NAME "/stats/query/update"

#define WEB_MBL			"http://" AUTHORIZE_SERVER_NAME	// Master Ban List



/*
============
Prototypes
============
*/

//
// http_main.c
//
qboolean _HTTP_Download(char *url, char *file, qboolean verbose);

//
// http.c
//
void HTTP_Post(char *url, char *data);	// Not used anywhere atm..
char *CL_HTTP_PostQuery(char *url, char *data);
char *CL_HTTP_Query(char *url);
qboolean CL_HTTP_Upload(char *url, char *file, char *field, char *data, char *extraField, char *extraData, qboolean deleteFile, qboolean verbose);
qboolean HTTP_Download(char *url, char *file, qboolean verbose);

#endif // _S_HTTP
