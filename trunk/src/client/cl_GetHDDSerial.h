#ifndef __HDD_CLIENT
#define  __HDD_CLIENT

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <winioctl.h>
#include "../game/q_shared.h"

char* getHardDriveSerial();
char * GetMAC();

#endif //  __HDD_CLIENT