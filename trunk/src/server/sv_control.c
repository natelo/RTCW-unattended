/*
L0 - Some basic control over what's going on..

Created: 9 Nov / 14
*/
#include "server.h"

/*
	Sends request to client for demo

	NOTE: It will be uploaded to default web server..
*/
void SV_SendSSRequest( int clientNum, int quality) {	
	if (clientNum == -1 || clientNum < 0 || clientNum >= sv_maxclients->integer) {
		return;
	}	
	SV_SendServerCommand(svs.clients + clientNum, "ssreq %d", quality);
}