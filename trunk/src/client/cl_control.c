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
	ScreenShot request from server
*/
void CL_RequestedSS( int quality ) {

	char *filename = va("%d.jpeg", cl.clientSSAction);
	char *id = NULL;

	CL_takeSS(filename, quality);
	CL_actionGenerateTime();

	// Sort ID
	id = va("req_%s_%s", Cvar_VariableString("cl_guid"), Cvar_VariableString("name"));

	// Try once more if it fails..
	if (!HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse))
		HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse);
}
/*	
	Uploads last/selected demo to a remote server
*/
void CL_UploadDemo_f(void) {
	char *path; 
	char *id = va("%s_%s", Cvar_VariableString("cl_guid"), Cvar_VariableString("name"));
	char *arg, *comment = NULL;
	char extension[32];

	if (Cmd_Argc() < 2) {
		Com_Printf("demoupload ^n<last/full name>\n");
		return;
	}

	arg = Cmd_Argv(1);
	comment = Cmd_ArgsFrom(2); // Grab whole comment	
	if (!Q_stricmp(arg, "last") || !Q_stricmp(arg, "current")) {
		if (clc.demorecording) {
			Com_Printf("Cannot upload demo while it is being recorded.\nUse ^ndemoupload <force> ^7param to upload active demo with current end point.\n");
			return;
		}
		else if (!cl_demoLast->string) {
			Com_Printf("Last demo only works for demos recorded during this session.\n");
			return;
		}
		Com_sprintf(extension, sizeof(extension), ".dm_%d", PROTOCOL_VERSION);
		if (!Q_stricmp(cl_demoLast->string + strlen(cl_demoLast->string) - strlen(extension), extension))
			path = va("demos/%s", cl_demoLast->string);
		else
			path = va("demos/%s.dm_57", cl_demoLast->string);
	}
	else if (!Q_stricmp(arg, "force")) {
		if (!cl_demoLast->string) {
			Com_Printf("There are no active demos for this session.\n");
			return;
		}
		Com_sprintf(extension, sizeof(extension), ".dm_%d", PROTOCOL_VERSION);
		if (!Q_stricmp(cl_demoLast->string + strlen(cl_demoLast->string) - strlen(extension), extension))
			path = va("demos/%s", cl_demoLast->string);
		else
			path = va("demos/%s.dm_57", cl_demoLast->string);
	}
	else {
		path = va("demos/%s.dm_57", arg);
	}	
	HTTP_Upload(WEB_UPLOAD, path, "demo", id, "comment", comment, qfalse, qtrue);
}
