/*
===========================================================================
	L0 - http_main.c

	Main HTTP related functionality.
===========================================================================
*/
#include "http.h"

/*
	Wrapper for Curl downloading..
*/
qboolean _HTTP_Download(char *url, char *file, qboolean verbose) {
	if (!HTTP_Download(url, file, verbose)) {
		return HTTP_Download(url, file, verbose);
	}
	return qtrue;
}

/*
	Wrapper for CG http call
*/
void CL_submitCmd(char *url, char *cmd) {
	CL_HTTP_Post(url, cmd);
	return;
}

/*
	Wrapper for CG http call
*/
void CL_postCmd(char *url, char *cmd) {
	char *reply = CL_HTTP_PostQuery(url, cmd);

	Com_Printf("%s\n", reply);
	return;
}

/*
	Wrapper for CG http call
*/
void CL_queryCmd(char *url) {
	char *reply = CL_HTTP_Query(url);

	Com_Printf("%s\n", reply);
	return;
}
