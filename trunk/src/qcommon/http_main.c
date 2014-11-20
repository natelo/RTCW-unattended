/*
===========================================================================
	L0 - http_main.c

	Main HTTP related functionality.
===========================================================================
*/
#include "http.h"


// initialize once
static int curl_handle_initialized = 0;

void CURL_Handle_Init(void) {
	if (curl_handle_initialized) {
		return;
	}

	/* Make sure curl has initialized, so the cleanup doesn't get confused */
	curl_global_init(CURL_GLOBAL_ALL);

	curl_multi = curl_multi_init();

	Com_Printf("----- Curl initialization -----\n");
	curl_handle_initialized = 1;
}

/*
================
DL_Shutdown

================
*/
void CURL_Handle_Shutdown(void) {
	if (!curl_handle_initialized) {
		return;
	}

	curl_multi_cleanup(curl_handle);
	curl_multi = NULL;

	curl_global_cleanup();

	curl_handle_initialized = 0;
		
	Com_Printf("----- Curl shutdown -----\n");
}
