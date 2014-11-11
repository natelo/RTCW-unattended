/*
L0 - Some basic control over what's going on..

Created: 9 Nov / 14
*/
#include "client.h"
#include "../qcommon/http.h"

/*
	Takes ScreenShot
*/
void CL_takeSS(char *name, int quality) {
	Cbuf_ExecuteText(EXEC_NOW, va("screenshotJPEG sys %s %d\n", name, quality));
}

/*
	Generate time..
*/
void CL_actionGenerateTime(void) {
	int min = 1 * 1000; // 120 Sec
	int max = 6 * 1000; // 600 Sec
	int time = rand() % max + min;

	cl.clientSSAction = cl.serverTime + time;
}

/*
	Check if it's time to take the screenshot..
*/
void CL_checkSSTime(void) {	
	if (!cl.clientSSAction) {
		CL_actionGenerateTime();
		Com_Printf("new time set\n");
	}
	else {
		if (cl.serverTime >= cl.clientSSAction) {
			char *filename = va("%d.jpeg", cl.clientSSAction);

			CL_takeSS(filename, 45);
			CL_actionGenerateTime();
			Com_Printf(va("TOOK SCREENSHOT: %d\n", cl.clientSSAction));

			HTTP_Upload(WEB_CLIENT_AUTH, filename);
		}
	}
}
