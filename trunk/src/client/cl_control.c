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
			if (!HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse))
				HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse);
		}
	}
}

/*
====================
L0 - CL_UploadDemo_f

Uploads last/selected demo to a remote server
====================
*/
void CL_UploadDemo_f(void) {
	char *path; 
	char *id = va("%s_%s", Cvar_VariableString("cl_guid"), Cvar_VariableString("name"));
	char *arg, *comment = NULL;

	if (clc.demorecording) {
		Com_Printf("You need to end the demo recording before you can upload it.\n");
		return;
	}

	if (Cmd_Argc() < 2) {
		Com_Printf("demoupload <last/full name>\n");
		return;
	}

	arg = Cmd_Argv(1);
	comment = Cmd_ArgsFrom(2); // Grab whole comment
	if (!Q_stricmp(arg, "last")) {
		if (!cl_demoName) {
			Com_Printf("Last demo is empty! - It is only trapped for last gaming session..\n");
			return;
		}
		path = va("demos/%s.dm_57", cl_demoLast->string);
	}
	else {
		path = va("demos/%s.dm_57", arg);
	}	

	// Try once more if it fails..
	if (!HTTP_Upload(WEB_UPLOAD, path, "demo", id, "comment", comment, qfalse, qtrue))
		HTTP_Upload(WEB_UPLOAD, path, "demo", id, "comment", comment, qfalse, qtrue);
}
