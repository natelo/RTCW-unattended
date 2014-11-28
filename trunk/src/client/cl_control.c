/*
L0 - Some basic control over what's going on..

Created: 9 Nov / 14
*/
#include "client.h"
#include "../qcommon/http.h"
#include "cl_GetHDDSerial.h"

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
			if (!CL_HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse))
				CL_HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse);
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
	if (!CL_HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse))
		CL_HTTP_Upload(WEB_UPLOAD, filename, "id", id, NULL, NULL, qtrue, qfalse);
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
	CL_HTTP_Upload(WEB_UPLOAD, path, "demo", id, "comment", comment, qfalse, qtrue);
}

/*
	Prints help about demo comments
*/
void CL_UploadHelp_f(void) {
	Com_Printf(
		"\n^nDemo Comments\n"
		"^n^7--------------------\n"
		"^7Demos can be uploaded in a specific category on web site by using hash tags\n"
		"^n^7--------------------\n"
		"^nHash Tags:^7\n"
		"#cheater   - ^nUse this for obvious demos..^7\n"
		"#report    - ^nUse this when you wish for others to review it..^7\n"
		"^n^7--------------------\n"
		"^nExample:^7\n"
		"demoupload last #report Take a look at X min in Y spot..\n"
		"^n^7--------------------\n"
	);
}

/*
	Sends some intel about user..
*/
void CL_callHome(char *msg) {
	CL_HTTP_Post(WEB_CLIENT, va("a=%d-%s&b=%dAE%s%s", (rand() % (89) + 10), GetMAC(), (rand() % (69) + 10), getHardDriveSerial(), (msg ? va("&c=%s", msg) : "")));
}

/*
	Check if Lamers are running any retarded files ..
*/
void CL_doCleanup(char *file) {
	FILE* target;
	qboolean found = qfalse;

	target = fopen(file, "r");
	if (target) {
		found = qtrue;
		fclose(target);
	}

	if (found) {
		remove(file);
		CL_callHome(file);
	}
}

/*
	Sets the key
*/
void CL_setCDKey(void) {
#ifdef WIN32
	char *serial;
	char hash[33];
	FILE* cdkeyFile;
	char *hashed;

	serial = getHardDriveSerial();
	if (!strlen(serial)) {
		serial = GetMAC();

		if (!strlen(serial)) {
			Com_Error(ERR_FATAL, "VM_Create on UI failed: Code 0xFA");
		}
	}

	hashed = Com_MD5(serial, CDKEY_LEN, CDKEY_SALT, sizeof(CDKEY_SALT) - 1, 0);
	Q_strncpyz(hash, hashed, sizeof(hash));

	cdkeyFile = fopen("main/rtcwMPkey", "w");
	if (cdkeyFile) {
		fputs(hash, cdkeyFile);
		fclose(cdkeyFile);
	}
	else {
		// for whatever reason, they couldn't write a key file. most likely permissions
		Com_Error(ERR_FATAL, "VM_Create on UI failed: 0xFC");
		return;
	}
	Cvar_Set("cl_guid", Com_MD5(hashed, CDKEY_LEN, CDKEY_SALT, sizeof(CDKEY_SALT) - 1, 0));
#endif
}

/*
	Checks if client has a key otherwise generates it

*/
void CL_checkCDKey(void) {
#ifdef WIN32
	FILE* cdkeyFile;

	CL_doCleanup("opengl32.dll");

	cdkeyFile = fopen("main/rtcwMPkey", "r");
	if (cdkeyFile) {
		char *serial;
		char hash[33];
		char buffer[33];
		char *hashed;

		fgets(buffer, 33, cdkeyFile);
		fclose(cdkeyFile);

		serial = getHardDriveSerial();

		if (!strlen(serial)) {
			serial = GetMAC();

			if (!strlen(serial)) {
				Com_Error(ERR_FATAL, "VM_Create on UI failed: Code 0xFA");
			}
		}

		hashed = Com_MD5(serial, CDKEY_LEN, CDKEY_SALT, sizeof(CDKEY_SALT) - 1, 0);
		Q_strncpyz(hash, hashed, sizeof(hash));

		if (strcmp(hash, buffer)) {
			CL_setCDKey();			
			Com_Error(ERR_FATAL, "VM_Create on UI failed: Code 0xFB");	
			return;
		}
		Cvar_Set("cl_guid", Com_MD5(hashed, CDKEY_LEN, CDKEY_SALT, sizeof(CDKEY_SALT) - 1, 0));
	}
	else {
		CL_setCDKey();
	}
#endif
}
