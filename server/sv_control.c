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
	- Based upon Nobo's banning from rtcwPub

	NOTE: IPv4 ATM only..	
*/
char *isClientBanned(char *ip, char *password) {
	FILE *bannedfile;
	char ips[1024];
	char msg[512];
	char *pass = NULL;
	char *out = NULL;

	if (strlen(password) > 0)
		pass = Com_MD5(password, strlen(password), CDKEY_SALT, sizeof(CDKEY_SALT) - 1, 0);

	bannedfile = fopen("mbl", "r");
	if (bannedfile) {
		unsigned int clientIP[4];

		sscanf(ip, "%3u.%3u.%3u.%3u", &clientIP[0], &clientIP[1], &clientIP[2], &clientIP[3]);
		while (fgets(ips, 1024, bannedfile) != NULL) {
			unsigned int match[5];
			unsigned int subrange;
			char bypass[33];

			int matchcount = sscanf(ips, "%3u.%3u.%3u.%3u/%2u %s %[^\n]", &match[0], &match[1], &match[2], &match[3], &match[4], bypass, msg);
			subrange = match[4];

			// Some (really basic) sanity checks
			if (matchcount < 4 || !(match[0] > 0 || match[0] < 256))
				continue;

			if (clientIP[0] == match[0]) {
				// Full Range (32)
				if (clientIP[1] == match[1] && clientIP[2] == match[2] && clientIP[3] == match[3]) {
					out = ((pass != NULL && !Q_stricmp(pass, bypass)) ? NULL : va("%s", msg));
					break;
				}
				else {
					if (subrange &&
						(
							subrange == 24 ||
							subrange == 16 ||
							subrange == 8
						)
					) 
					{
						// Traverse through it now
						if (subrange == 24 && clientIP[2] == match[2] && clientIP[1] == match[1]) {
							out = ((pass != NULL && !Q_stricmp(pass, bypass)) ? NULL : va("%s", msg));
							break;
						}
						else if (subrange == 16 && clientIP[1] == match[1]) {
							out = ((pass != NULL && !Q_stricmp(pass, bypass)) ? NULL : va("%s", msg));
							break;
						}
						// First bit already matched upon entry..
						else if (subrange == 8) {
							out = ((pass != NULL && !Q_stricmp(pass, bypass)) ? NULL : va("%s", msg));
							break;
						}
					}
				}
			}
		}
		fclose(bannedfile);
	}
	return out;
}

/*
	Requests screenshot from all connected clients
*/
void autoSSTime(void) {
	int i;
	client_t    *cl;

	if (!sv_ssEnable->integer)
		return;

	if (svs.time < svs.ssTime)
		return;

	for (i = 0; i < sv_maxclients->integer; i++) {
		cl = &svs.clients[i];

		if (cl->state)
			SV_SendSSRequest(i, sv_ssQuality->value);
	}

	// Sort some stuff
	if (sv_ssMinTime->integer < 300)
		Cvar_Set("sv_ssMinTime", "300");
	else if (sv_ssMinTime->integer > 1200)
		Cvar_Set("sv_ssMinTime", "1200");
	
	if (sv_ssMaxTime->integer > 1500)
		Cvar_Set("sv_ssMaxTime", "1500");
	else if (sv_ssMaxTime->integer < 450)
		Cvar_Set("sv_ssMaxTime", "450");

	if (sv_ssQuality->integer > 100)
		Cvar_Set("sv_ssQuality", "100");
	else if (sv_ssQuality->integer < 30)
		Cvar_Set("sv_ssQuality", "30");

	// Set new timer
	{
		int min = sv_ssMinTime->integer * 1000;
		int max = sv_ssMaxTime->integer * 1000;
		int newTime = rand() % max + min;

		svs.ssTime = svs.time + newTime;
	}
}
