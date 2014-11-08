/*
===========================================================================
L0 - g_http.c
Basically all the core (socket) http functionality is here.

Credits: Some (most of core) of the functionality is based around S4NDMoD.

Created: 23.07 / 14
Last Updated: 26.07 / 14
===========================================================================
*/
#ifdef WIN32
	#include <winsock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#define closesocket close
	#include <unistd.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

#include "http.h"

// Easier to read..
#define _SEND(SOCK, MSG) \
	send(SOCK, MSG, (int)strlen(MSG), 0);

/*
===============
GetHostAddress

Maps URL to address
===============
*/
u_int GetHostAddress(char* host) {
	struct hostent *phe;
	char *p;

	phe = gethostbyname(host);

	if (phe == NULL) {
		return 0;
	}

	p = *phe->h_addr_list;
	return *((u_int*)p);
}

/*
===============
ValidHostChar

Validates URL string..
===============
*/
int ValidHostChar(char ch) {
	return(isalnum(ch) || ch == '-' || ch == '.' || ch == ':');
}

/*
===============
strtoup2

Transforms URL to upper case for easier handling..
===============
*/
char *strupr2(char *string) {
	char *s;

	if (string) {
		for (s = string; *s; ++s)
			*s = toupper(*s);
	}
	return string;
}

/*
===============
ParseURL

Parses URL so it's ready for sockets..
===============
*/
void ParseURL(char* url, char* protocol, int lprotocol,
	char* host, int lhost, char* request, int lrequest, int *port) {
	char *work, *ptr, *ptr2;

	*protocol = *host = *request = 0;
	*port = 80;

#ifdef WIN32
	work = _strdup(url);
#else
	work = strdup(url);
#endif

	strupr2(work);

	ptr = strchr(work, ':');
	if (ptr != NULL) {
		*(ptr++) = 0;
		strcpy(protocol, work);
	}
	else
	{
		strcpy(protocol, "HTTP");
		ptr = work;
	}

	if ((*ptr == '/') && (*(ptr + 1) == '/')) {
		ptr += 2;
	}

	ptr2 = ptr;
	while (ValidHostChar(*ptr2) && *ptr2)
		ptr2++;

	*ptr2 = 0;
	strcpy(host, ptr);

	strcpy(request, url + (ptr2 - work));

	ptr = strchr(host, ':');
	if (ptr != NULL) {
		*ptr = 0;
		*port = atoi(ptr + 1);
	}

	free(work);
}

/*
===============
Get File size

Author: http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
===============
*/
int fsize(FILE *fp) {
	int prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);

	//go back to where we were
	fseek(fp, prev, SEEK_SET);

	return sz;
}

/*
===============
http_Submit

Submits data and doesn't care about any replies and simply bails out..
===============
*/
void HTTP_SubmitData(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
	int err;
#endif
	struct  sockaddr_in sin;
	int sock;
	char protocol[20], host[256], request[1024];
	int port;
	char *header;

	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

	if (/*!g_httpIgnoreSafeSize.integer &&*/ strlen(data) > UFILE_SIZELIMIT) {		
		printf("Request is too big - Web Submitting cancelled.\n");
		printf("You can disable the limit [g_httpIgnoreSafeSize 1] at your own risk!\n");
		return;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		return;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		return;
	}

	//Connect to web sever
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		return;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	header = va(
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Signature: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %i\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n\r\n",
		request,
		STATS_VERSION,
		Q3_VERSION,
		/*g_httpToken.string*/ "blah",
		/*sv_hostname.string*/ "doh",
		strlen(data) + 8, // FYI[8]: \r\n\r\n
		host
	);

	// Header
	_SEND(sock, header);

	// Data
	_SEND(sock, data);  
	_SEND(sock, "\r\n\r\n");

	// We're done..bail out
	closesocket(sock);
	return;
}

/*
===============
Just wipes the file..so size doesn't kill the server on upload
===============
*/
void wipeContets(char *file) {
	FILE *fh;	

	// We call it however we do but ultimatively, choice doesn't depend of us..
	//if (!g_httpFlushFile.integer)
	//	return;

	fh = fopen(file, "w");

	if (fh)
		fclose(fh);
}

/*
===============
http_SubmitFile

Submits file to a web server
===============
*/
void HTTP_SubmitFile(char *url, char *file, qboolean wipe) {
#ifdef WIN32
	WSADATA WsaData;
	int err;
#endif
	struct  sockaddr_in sin;
	int sock;
	char protocol[20], host[256], request[1024];
	int port; 
	char *header;
	FILE *fh;
	char buf[256];
	qboolean post = qfalse;
	
	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

	fh = fopen(file, "r");
	if (!fh) {
		return;
	}

	if (/*!g_httpIgnoreSafeSize.integer &&*/ fsize(fh) > UFILE_SIZELIMIT) {
		fclose(fh);
		printf("%s file is too big - Process cancelled.\n", file);
		printf("You can disable the limit [g_httpIgnoreSafeSize 1] at your own risk!\n");
		return;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		fclose(fh);
		return;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		fclose(fh);
		return;
	}

	//Connect to web sever
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		fclose(fh);
		return;
	}

	if (!*request) {
		strcpy(request, "/");
	}
	
	header = va(
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Signature: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %i\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n\r\n",
		request,
		STATS_VERSION,
		Q3_VERSION,
		/*g_httpToken.string*/ "",
		/*sv_hostname.string*/ "",
		fsize(fh) + 9, // FYI[9]: data= \r\n
		host
	);

	// Header
	_SEND(sock, header);
	
	// Feed the troll
	while (fgets(buf, sizeof(buf), fh) != NULL) {
		if (!post) {
			// Data
			_SEND(sock, va("data=%s", buf));
			post = qtrue;
		} 
		else
			// Data
			_SEND(sock, buf);
	}

	// Bail out
	_SEND(sock, "\r\n\r\n");

	// We're done..bail out
	closesocket(sock);
	// Close file 
	fclose(fh);

	// Wipe the file
	if (wipe)
		wipeContets(file);

	return;
}

/*
===============
http_Query

Queries the web server and reads the reply..
Note: POST field is already appanded (cmd=) and accounted for.
===============
*/
char *HTTP_QueryAddres(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
	int err;
#endif
	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, done;
	char *header;
	char *out = NULL;
	
	// Parse the URL
	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

	if (strcmp(protocol, "HTTP")) {
		return out;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		return out;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return out;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		return out;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	header = va(
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Signature: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %d\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n\r\n",
		request,
		STATS_VERSION,
		Q3_VERSION,
		/*g_httpToken.string*/ "",
		/*sv_hostname.string*/ "",
		strlen(data) + 8, /* FYI[+8]:  "cmd= \r\n" */
		host
	);

	// Send Header
	_SEND(sock, header);

	// Data	
	_SEND(sock, va("cmd=%s\r\n", data));

	// Null it...
	_SEND(sock, "\r\n\r\n");

	// Receive the reply
	chars = 0;
	done = 0;
	while (!done)
	{
		l = recv(sock, buffer, 1, 0);
		if (l < 0) {
			done = 1;
		}

		switch (*buffer)
		{
		case '\r':
			break;
		case '\n':
			if (chars == 0) {
				done = 1;
			}
			chars = 0;
			break;
		default:
			chars++;
			break;
		}
	}

	do
	{
		l = recv(sock, buffer, sizeof(buffer)-1, 0);
		if (l < 0) {
			break;
		}
		*(buffer + l) = 0;

		// We do not expect large replies so it will always be the last bit
		// which ensures we don't 'eat' the whole error/timeout message..
		if (strlen(buffer) > 0)			
			out = va("%s", buffer);

	} while (l > 0);

	closesocket(sock);
	return out;
}
