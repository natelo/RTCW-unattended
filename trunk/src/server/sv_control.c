/*
L0 - Some basic control over what's going on..

Created: 9 Nov / 14
*/
#include "server.h"

/*
	Sends request to client for screenshot

	NOTE: It will be uploaded to default web server..
*/
void SV_SendSSRequest( int clientNum, int quality) {	
	if (clientNum == -1 || clientNum < 0 || clientNum >= sv_maxclients->integer) {
		return;
	}	
	SV_SendServerCommand(svs.clients + clientNum, "ssreq %d", quality);
}

/*
	Check if client is banned
*/
qboolean isClientBanned(char *guid) {
	FILE *bannedfile;
	char guids[1024];	
	char msg[512];
	char bannedGuid[32];

	if (!Q_stricmp(guid, "")) {
		BannedMessage = "Spoofed/Missing GUID...";
		return qtrue;
	}

	bannedfile = fopen("mbl", "r");
	if (bannedfile) {
		while (fgets(guids, 1024, bannedfile) != NULL) {
			sscanf(guids, "%s %[^\n]", bannedGuid, msg);

			if (!Q_stricmp(bannedGuid, guid)) {
				BannedMessage = va("%s", msg);
				fclose(bannedfile);
				return qtrue;
			}
		}
		fclose(bannedfile);
	}
	return qfalse;
}
