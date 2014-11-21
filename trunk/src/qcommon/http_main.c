/*
===========================================================================
	L0 - http_main.c

	Main HTTP related functionality.
===========================================================================
*/
#include "http.h"

#ifdef __linux__
#include <gnu/lib-names.h>
#elif defined( __MACOS__ )
#define LIBPTHREAD_SO "/usr/lib/libpthread.dylib"
#elif defined( __APPLE__ )
#define LIBPTHREAD_SO "/usr/lib/libpthread.dylib"
#endif

#ifndef WIN32
#include <pthread.h>
#include <dlfcn.h>


// tjw: handle for libpthread.so
void *g_pthreads = NULL;

// tjw: pointer to pthread_create() from libpthread.so
static int(*g_pthread_create)
(pthread_t  *,
__const pthread_attr_t *,
void * (*)(void *),
void *) = NULL;


void CL_HTTP_InitThreads(void)
{
	if (g_pthreads != NULL) {
		Com_DPrintf("pthreads already loaded\n");
		return;
	}
	g_pthreads = dlopen(LIBPTHREAD_SO, RTLD_NOW);
	if (g_pthreads == NULL) {
		Com_DPrintf("could not load libpthread\n%s\n", dlerror());
		return;
	}
	Com_DPrintf("loaded libpthread\n");
	g_pthread_create = dlsym(g_pthreads, "pthread_create");
	if (g_pthread_create == NULL) {
		Com_DPrintf("could not locate pthread_create\n%s\n", dlerror());
		return;
	}
	Com_Printf("--- Threading initialized ---\n");
}

/*
	Create a thread
*/
int http_create_thread(void *(*thread_function)(void *), void *arguments) {
	pthread_t thread_id;

	if (g_pthread_create == NULL) {
		// tjw: pthread_create() returns non 0 for failure
		//      but I don't know what's proper here.
		return -1;
	}

	Com_DPrintf("Thread created.\n");

	return g_pthread_create(&thread_id, NULL, thread_function, arguments);
}

#else //WIN32
#include <process.h>

void CL_HTTP_InitThreads(void)
{
	// forty - we can have thread support in win32 we need to link with the MT runtime and use _beginthread
	Com_Printf("--- Threading initialized ---\n");
}

/*
	Create a thread
*/
int http_create_thread(void *(*thread_function)(void *), void *arguments) {
	void *(*func)(void *) = thread_function;

	Com_DPrintf("Thread created.\n");

	//Yay - no complaining
	_beginthread((void(*)(void *))func, 0, arguments);
	return 0;
}
#endif

/*
	Process query 
*/
void *HTTP_process_PostQuery(void *args) {
	HTTP_postCmd_t *cmd = (HTTP_postCmd_t *)args;
	char *reply = NULL;
	
	// Spinlock as I really do not want to deal with sync across the threads..
	while (HTTPreplyPtr != NULL);

	HTTPreplyPtr = HTTP_PostQuery(cmd->url, cmd->data);

	free(cmd);
	return 0;
}

/*
	Wrapper for HTTP_PostQuery
*/
char *CL_HTTP_PostQuery(char *url, char *data) {
	HTTP_postCmd_t *post_cmd = (HTTP_postCmd_t*)malloc(sizeof(HTTP_postCmd_t));
	char *out = NULL;

	// Fill the structure
	post_cmd->url = url;
	post_cmd->headerToken = "signature"; // FIXME - Add token for cheap request signing..
	post_cmd->data = data;

	http_create_thread(HTTP_process_PostQuery, (void*)post_cmd);

	out = va("%s", HTTPreplyPtr);
	HTTPreplyPtr = NULL;

	Com_DPrintf("Thread destroyed.\n");
	return out;
}
