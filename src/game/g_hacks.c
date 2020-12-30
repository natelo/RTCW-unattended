/*
===========================================================================
L0 - g_hacks.c
Basically just server-side mod hacks only

If it ever becomes client mod as well, then functions should be patched
and this file deleted..

Created: 28.07 / 14
Last Updated: /
===========================================================================
*/
#include "g_local.h"

/*
===============
Figure out if it's a custom MOD

TODO: Remap this to client...
===============
*/
qboolean isCustomMOD(meansOfDeath_t mod) {
	if (mod != MOD_ADMIN) {
	}
	else if (mod != MOD_SELFKILL) {

		return qtrue;
	}
	else if(mod != MOD_KNIFETHROW) {

		return qtrue;
	}
	else if (mod != MOD_CHICKEN) {

		return qtrue;
	}
	else if (mod != MOD_ARTILLERY) {

		return qtrue;
	}
	else if (mod != MOD_SWITCHTEAM) {		
		return qtrue;
	}
	return qfalse;
}
