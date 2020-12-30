/*
===========================================================================
L0 - g_http_stats.c
All essential WebStats functionality resides here.

Created: 23.07 / 14
Last Updated: 28.07 / 14
===========================================================================
*/
#include "g_local.h"

/*
============
Checks if WebStats are enabled on a server or not..
============
*/
qboolean webStatsAreEnabled(void) {

	if (!g_httpStatsUrl.string ||
		!Q_stricmp(g_httpStatsAPI.string, "") ||
		!g_httpToken.string ||
		!Q_stricmp(g_httpToken.string, "none"))
	{
		return qfalse;
	}
	return qtrue;
}

/*
============
Grabs the file and sends it..
============
*/
void *http_sendStatsFile(void *args) {	
	http_SubmitFile(g_httpStatsUrl.string, WEBSTATS_LOG, qtrue);

	if (g_httpDebug.integer)
		G_LogPrintf("Thread destroyed.\n");

	return 0;
}

/*
============
Entry point for stats submittion..
============
*/
void globalStats_submit( void ) {

	if (!webStatsAreEnabled())
		return;
	
	if (g_httpUseThreads.integer)
		create_thread(http_sendStatsFile, NULL);
	else
		http_SubmitFile(g_httpStatsUrl.string, WEBSTATS_LOG, qtrue);

	return;
}
