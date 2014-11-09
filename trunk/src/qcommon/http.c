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

		out = s.ptr;
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

		out = s.ptr;
		free(s.ptr);
		curl_easy_cleanup(curl);
	}
	return out;
}
