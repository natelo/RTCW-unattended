/*
---------------------------
Nate 'L0 - g_threads.h
Created: 23.07/14
---------------------------
*/
#ifndef _G_THREADS_H
#define _G_THREADS_H

void G_InitThreads(void);
int create_thread(void *(*thread_function)(void *),void *arguments);

#endif // _G_THREADS_H
