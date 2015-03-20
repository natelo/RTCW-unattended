#ifndef __HDD_CLIENT
#define  __HDD_CLIENT

#if WIN32
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <winioctl.h>
#else
// For linux..
#endif
#include "../game/q_shared.h"

char* getHardDriveSerial();
char * GetMAC();


#endif //  __HDD_CLIENT