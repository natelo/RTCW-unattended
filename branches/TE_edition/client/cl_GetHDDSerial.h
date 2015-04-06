#ifndef __HDD_CLIENT
#define  __HDD_CLIENT

#if WIN32
	#include <stdlib.h>
	#include <stdio.h>
	#include <stddef.h>
	#include <string.h>
	#include <windows.h>
	#include <winioctl.h>
#elif __linux__
	#include <linux/types.h>
	#include <linux/hdreg.h>
	#include <linux/fcntl.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <unistd.h> 
#elif __MACOS__ // Not supported but while adding MAC stuff..
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <net/if_types.h>
	#include <net/if_dl.h>
	#include <ifaddrs.h>
#else
	// Not supported..
#endif
#include "../game/q_shared.h"

char *getHardDriveSerial( void );
char *GetMAC( void );

#endif //  __HDD_CLIENT