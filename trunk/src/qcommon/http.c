/*
===========================================================================
	L0 - g_http.c

	HTTP related functionality
===========================================================================
*/
#include "http.h"
#include <curl/curl.h>
#include <curl/easy.h>

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
		Com_Printf("HTTP[i_s]: malloc() failed\n");
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
		Com_Printf("HTTP[write]: realloc() failed\n");
		return 0;
	}

	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

/*
	Just Submits data 

	Used when we want to send specific data but do not care about reply..
*/
void HTTP_Post(char *url, char *data) {
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {		
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: one way street");
		headers = curl_slist_append(headers, "Client: rtcwmp");
		headers = curl_slist_append(headers, va("Signature: BLA BLA"));

		curl_easy_setopt(curl, CURLOPT_URL, url);	
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			Com_Printf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return;
}

/*
	Submits data and waits for reply..

	Sends data and expects reply so rest of depended functions can do their thing
*/
char *HTTP_PostQuery(char *url, char *data) {
	CURL *curl;
	CURLcode res;
	char *out;

	curl = curl_easy_init();
	if (curl) {
		struct HTTPreply s;
		init_string(&s);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: two way street");
		headers = curl_slist_append(headers, "Client: rtcwmp");
		headers = curl_slist_append(headers, va("Signature: BLA BLA"));

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parseReply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			Com_Printf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		out = va("%s", s.ptr); // Copy it thru engine..
		free(s.ptr);
		curl_easy_cleanup(curl);
	}
	return out;
}

/*
	Queries server

	Connects to address and processes any data it gets..
*/
char *HTTP_Query(char *url) {
	CURL *curl;
	CURLcode res;
	char *out;

	curl = curl_easy_init();
	if (curl) {
		struct HTTPreply s;
		init_string(&s);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Intention: friends with benefits");
		headers = curl_slist_append(headers, "Client: rtcwmp");
		headers = curl_slist_append(headers, va("Signature: BLA BLA"));

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parseReply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			Com_Printf("HTTP[res] failed: %s\n", curl_easy_strerror(res));

		out = va("%s", s.ptr); // Copy it thru engine..
		free(s.ptr);
		curl_easy_cleanup(curl);
	}
	return out;
}

/*
	Get File size

	Author: http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
*/
int fsize(FILE *fp) {
	int prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);

	//go back to where we were
	fseek(fp, prev, SEEK_SET);

	return sz;
}

struct WriteThis {
	const char *readptr;
	long sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct WriteThis *pooh = (struct WriteThis *)userp;

	if (size*nmemb < 1)
		return 0;

	if (pooh->sizeleft) {
		*(char *)ptr = pooh->readptr[0]; /* copy one single byte */
		pooh->readptr++;                 /* advance pointer */
		pooh->sizeleft--;                /* less data left */
		return 1;                        /* we return 1 byte at a time! */
	}

	return 0;                          /* no more data left to deliver */
}

#include <sys/stat.h>
qboolean HTTP_Upload(char *url, char *file) {
	CURL *curl;
	CURLcode res;
	struct stat file_info;
	double speed_upload, total_time;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	FILE *fd;
	static const char buf[] = "Expect:";

	fd = fopen(va("Main/%s", file), "rb");
	if (!fd) {
		Com_Printf("HTTP[fu]: cannot o/r\n");
		return qfalse;
	}

	if (fstat(fileno(fd), &file_info) != 0) {
		Com_Printf("HTTP[fs]: cannot o/r\n");
		return qfalse; 
	}

	/* Fill in the file upload field */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "file",
		CURLFORM_FILE, va("Main/%s", file),
		CURLFORM_END);

	/* Fill in the filename field */
	/*curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "text",
		CURLFORM_COPYCONTENTS, file,
		CURLFORM_END);
	*/
	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);

	if (curl) {		
		//struct curl_slist *headers = NULL;
		//headers = curl_slist_append(headers, "Content-Type: image/jpeg");

		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/stats/query/upload");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		//curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		//curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		//curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);		
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		res = curl_easy_perform(curl);		
		if (res != CURLE_OK) {
			Com_Printf("HTTP[res] failed: %s\n", curl_easy_strerror(res));
		}
		else {
			
			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

			Com_Printf("Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);

		}
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(formpost);
		/* free slist */
		curl_slist_free_all(headerlist);
	}

	//fclose(fd);
	//remove(va("Main/%s", file));

	return qtrue;
}
