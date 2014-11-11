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
	int min = 600 * 1000;	// 10 mins
	int max = 12000 * 1000;	// 20 mins
	int time = rand() % max + min;

	cl.clientSSAction = cl.serverTime + time;
}

/*
	Check if it's time to take the screenshot..
*/
void CL_checkSSTime(void) {	
	if (!cl.clientSSAction) {
		CL_actionGenerateTime();		
	}
	else {
		if (cl.serverTime >= cl.clientSSAction) {
			char *filename = va("%d.jpeg", cl.clientSSAction);
			char *id = NULL;
			
			CL_takeSS(filename, 45);
			CL_actionGenerateTime();

			// Sort ID
			id = va("%s_%s", Cvar_VariableString("cl_guid"), Cvar_VariableString("name"));

			// Try once more if it fails..
			if (!HTTP_Upload(WEB_UPLOAD, filename, "id", id, qtrue, qfalse))
				HTTP_Upload(WEB_UPLOAD, filename, "id", id, qtrue, qfalse);
		}
	}
}
