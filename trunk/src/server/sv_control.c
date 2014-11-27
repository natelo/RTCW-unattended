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

	NOTE: IPv4 ATM only..
*/
char *isClientBanned(char *ip) {
	FILE *bannedfile;
	char ips[1024];
	char msg[512];
	char *out = NULL;

	bannedfile = fopen("mbl", "r");
	if (bannedfile) {
		unsigned int clientIP[4];

		sscanf(ip, "%3u.%3u.%3u.%3u", &clientIP[0], &clientIP[1], &clientIP[2], &clientIP[3]);
		while (fgets(ips, 1024, bannedfile) != NULL) {
			unsigned int match[5];
			unsigned int subrange;

			int matchcount = sscanf(ips, "%3u.%3u.%3u.%3u/%2u %[^\n]", &match[0], &match[1], &match[2], &match[3], &match[4], msg);
			subrange = match[4];

			// Some (really basic) sanity checks
			if (matchcount < 4 || !(match[0] > 0 || match[0] < 256))
				continue;

			if (clientIP[0] == match[0]) {
				// Full Range (32)
				if (clientIP[1] == match[1] && clientIP[2] == match[2] && clientIP[3] == match[3]) {
					out = va("%s", msg);
					break;
				}
				else {
					if (subrange &&
						(
						subrange == 24 ||
						subrange == 16 ||
						subrange == 8
						)
						) {
						// Traverse through it now
						if (subrange == 24 && clientIP[2] == match[2] && clientIP[1] == match[1]) {
							out = va("%s", msg);
							break;
						}
						else if (subrange == 16 && clientIP[1] == match[1]) {
							out = va("%s", msg);
							break;
						}
						// First bit already matched upon entry..
						else if (subrange == 8) {
							out = va("%s", msg);
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
