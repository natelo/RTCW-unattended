/*
===========================================================================
	L0 - http_main.c

	Main HTTP related functionality.
===========================================================================
*/
#include "http.h"

qboolean _HTTP_Download(char *url, char *file, qboolean verbose) {
	if (!HTTP_Download(url, file, verbose)) {
		return HTTP_Download(url, file, verbose);
	}
	return qtrue;
}