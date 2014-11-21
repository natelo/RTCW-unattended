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
#define WEB_AUTH		"http://" AUTHORIZE_SERVER_NAME	"/stats/query/auth"
#define WEB_CLIENT_AUTH "http://" AUTHORIZE_SERVER_NAME "/stats/query/client"
#define WEB_UPLOAD		"http://" AUTHORIZE_SERVER_NAME "/stats/query/upload"
#define WEB_UPDATE		"http://" UPDATE_SERVER_NAME "/stats/query/update"

/*
Structures to hold stuff
*/
typedef struct {
	char *url;
	char *headerToken;
	char *data;
} HTTP_postCmd_t;

typedef struct {
	char *url;
	char *file;
	char *field;
	char *data;
	char *extraField;
	char *extraData;
	qboolean deleteFile;
	qboolean verbose;
} HTTP_fileCmd_t;

/*
============
Prototypes
============
*/
static char *HTTPreplyMsg = { NULL };

//
// http_main.c
//
void CL_HTTP_InitThreads(void);
char *CL_HTTP_PostQuery(char *url, char *data);

//
// http.c
//
void HTTP_Post(char *url, char *data);
char *HTTP_PostQuery(char *url, char *data);
char *HTTP_Query(char *url);
qboolean HTTP_Upload(char *url, char *file, char *field, char *data, char *extraField, char *extraData, qboolean deleteFile, qboolean verbose);

#endif // _S_HTTP
