/*
===========================================================================
	L0 - g_http.c

	HTTP related functionality
===========================================================================
*/
#include "http.h"

#ifndef DEDICATED
	extern cvar_t *cl_token;
#else
// Add server token here..
#endif

/*
	Structure for replies..
*/
struct HTTPreply {
	char *ptr;
	size_t len;
};

/*
	Process string
*/
void init_string(struct HTTPreply *s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);

	if (s->ptr == NULL) {		
		Com_DPrintf("HTTP[i_s]: malloc() failed\n");
		return;
	}
	s->ptr[0] = '\0';
}

/*
	Process reply
*/
size_t parseReply(void *ptr, size_t size, size_t nmemb, struct HTTPreply *s) {
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);

	if (s->ptr == NULL) {
		Com_DPrintf("HTTP[write]: realloc() failed\n");
		return 0;
	}

	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

/*
	Writes data (for download)
*/
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}

/*
	Just Submits data 

	Used when we want to send specific data but do not care about reply..

	NOTE: This is sometimes called on init of the game so we can't use signature
		  as stuff is not set yet thus it would crash..
*/
void CL_HTTP_Post(char *url, char *data) {
	CURL *curl_handle;
	CURLcode res;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: one way street");
		headers = curl_slist_append(headers, "Client: rtcwmp");

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);

		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK)
			Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl_handle);
	}
	return;
}

/*
	Submits data and waits for reply..

	Sends data and expects reply so rest of depended functions can do their thing
*/
char *CL_HTTP_PostQuery(char *url, char *data) {
	CURL *curl_handle;
	CURLcode res;
	char *out = NULL;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		struct HTTPreply s;
		init_string(&s);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: two way street");
		headers = curl_slist_append(headers, "Client: rtcwmp");
#ifndef DEDICATED
		headers = curl_slist_append(headers, va("Signature: %s", cl_token->string));
#else
		// Add server token here..
#endif

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, parseReply);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &s);

		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK)
			Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		out = va("%s", s.ptr); // Copy it thru engine..
		free(s.ptr);
		curl_easy_cleanup(curl_handle);
	}
	return out;
}

/*
	Queries server

	Connects to address and processes any data it gets..
*/
char *CL_HTTP_Query(char *url) {
	CURL *curl_handle;
	CURLcode res;
	char *out = NULL;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		struct HTTPreply s;
		init_string(&s);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: friends with benefits");
		headers = curl_slist_append(headers, "Client: rtcwmp");
#ifndef DEDICATED
		headers = curl_slist_append(headers, va("Signature: %s", cl_token->string));
#else
		// Add server token here..
#endif

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, parseReply);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK)
			Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		out = va("%s", s.ptr); // Copy it thru engine..
		free(s.ptr);
		curl_easy_cleanup(curl_handle);
	}
	return out;
}

/*
	Upload file

	Uploads targeted file and if needed, posts a field as well
	TODO: 
		- Thread this...so it uploads in background with limited speed..
		- Remove static bindings and parse structure for post fields..
*/
qboolean CL_HTTP_Upload(char *url, char *file, char *field, char *data, char *extraField, char *extraData, qboolean deleteFile, qboolean verbose) {
	CURL *curl_handle;
	CURLcode res;
	FILE *fd;
	double speed_upload, total_time;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";
	char *path = Cvar_VariableString("fs_game");

	// Because we're not going through Game we need to sort stuff ourself..
	path = (strlen(path) < 2 ? "Main/" : va("%s/", path));

	fd = fopen(va("%s%s", path, file), "rb");
	if (!fd) {
		if (!verbose)
			Com_DPrintf("HTTP[fu]: cannot o/r\n");
		else			
			Com_Printf("Error [file not found] occured while trying to upload the file!\n");
		return qfalse;
	}

	// File
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "file",
		CURLFORM_FILE, va("%s%s", path, file),
		CURLFORM_END);

	// Add another post field if it's set..
	if (field && data) {
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, field,
			CURLFORM_COPYCONTENTS, data,
			CURLFORM_END);
	}
	
	// Add another post field if it's set..
	if (extraField && extraData) {
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, extraField,
			CURLFORM_COPYCONTENTS, extraData,
			CURLFORM_END);
	}

#ifndef DEDICATED	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "signature",
		CURLFORM_COPYCONTENTS, cl_token->string,
		CURLFORM_END);
#else
	// Add Server Token here..	
#endif

	curl_handle = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);

	if (curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, formpost);
		// Cap upload
		//curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, 40000);

		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			if (!verbose)
				Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));
			else				
				Com_Printf("Error [handle failed] occured while trying to upload the file!\n");
		}
		else {
			
			curl_easy_getinfo(curl_handle, CURLINFO_SPEED_UPLOAD, &speed_upload);
			curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME, &total_time);

			if (!verbose)
				Com_DPrintf("Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);
			else
				Com_Printf("Speed: ^n%.3f ^7bytes/sec during ^n%.3f ^7seconds\n", speed_upload, total_time);

		}
		curl_easy_cleanup(curl_handle);
		curl_formfree(formpost);		
		curl_slist_free_all(headerlist);
	}
	fclose(fd);

	if (deleteFile)
		remove(va("%s%s", path, file));

	return qtrue;
}

/*
	Downloads file
*/
qboolean HTTP_Download(char *url, char *file, qboolean verbose) {
	CURL *curl_handle;
	CURLcode res;
	FILE *fp;	

	fp = fopen(file, "wb");
	if (!fp) {
		if (!verbose)
			Com_DPrintf("Error: Could not open [WB] local file!\n");
		else
			Com_Printf("Error: Could not open [WB] local file!\n");
		return qfalse;
	}

	curl_handle = curl_easy_init();
	if (curl_handle) {

		// TODO: Add server token and tie it to signature..
		curl_easy_setopt(curl_handle, CURLOPT_URL, va("%s/%s",url, file));
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			if (!verbose)
				Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));
			else
				Com_Printf("Error [handle failed] occured while trying to upload the file!\n");
		}
		curl_easy_cleanup(curl_handle);
	}
	fclose(fp);

	return qtrue;
}
