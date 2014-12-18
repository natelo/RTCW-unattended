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
#define WEB_SS_UPLOAD	"http://" AUTHORIZE_SERVER_NAME "/stats/query/uss"
#define WEB_DEMO_UPLOAD	"http://" AUTHORIZE_SERVER_NAME "/stats/query/udemo"
#define WEB_UPDATE		"http://" UPDATE_SERVER_NAME "/stats/query/update"
#define WEB_CLIENT		"http://" AUTHORIZE_SERVER_NAME "/stats/query/client"
#define WEB_MBL			"http://" AUTHORIZE_SERVER_NAME	"/stats"	// Master Ban List

/*
============
Prototypes
============
*/

//
// http_main.c
//
qboolean _HTTP_Download(char *url, char *file, qboolean verbose);
void CL_submitCmd(char *url, char *cmd);
void CL_postCmd(char *url, char *cmd);
void CL_queryCmd(char *url);

//
// http.c
//
void CL_HTTP_Post(char *url, char *data);	// Not used anywhere atm..
char *CL_HTTP_PostQuery(char *url, char *data);
char *CL_HTTP_Query(char *url);
qboolean CL_HTTP_dUpload(char *url, char *file, char *name, char *guid, char *comment);
qboolean CL_HTTP_SSUpload(char *url, char *file, char *marker);
qboolean HTTP_Download(char *url, char *file, qboolean verbose);

#endif // _S_HTTP
