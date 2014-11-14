/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of wolfX source code.  

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
 * name:			cg_window.c
 *
 * desc:			OSP port of cgame window handling
 * Author:			Nate 'L0 (original Author: rhea@OrangeSmoothie.org)
 * created:			14 Jan / 2013
 * Last updated:	31 Jan / 2013
 * Notes:			I've added missing effects / added stats window handling / 
 *					truetype effect font is set to fixed site (for now) / few tweaks up and there.
 */

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern pmove_t cg_pmove;        // cg_predict.c
extern displayContextDef_t cgDC;// L0 - Makes more sense here..	

/*
	Basic info and not even a window..

	NOTE: Ugly inlines :|
*/
void CG_demoView(void) {

	if (cg.demoPlayback && demo_infoWindow.integer) {
		vec4_t colorGeneralFill = { 0.1f, 0.1f, 0.1f, 0.4f };
		vec4_t colorBorderFill = { 0.1f, 0.1f, 0.1f, 0.8f };
		char *s = va("^nWallhack: ^7%s ^n| Timescale: ^7%.1f", (demo_wallhack.integer ? "On" : "Off"), cg_timescale.value);
		char *ts = (cg_timescale.value != 1.0 ? "Space: Default" : "Fst/Slw: Scroll");
		int w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
		char *s2 = (demo_wallhack.integer ? va("^nToggle: F1     | %s", ts) : va("^nToggle: F1      | %s", ts));
		int w2 = CG_DrawStrlen(s) * (TINYCHAR_WIDTH - 1);

		CG_FillRect(42 - 2, 400, w + 5, SMALLCHAR_HEIGHT + 3, colorGeneralFill);
		CG_DrawRect(42 - 2, 400, w + 5, SMALLCHAR_HEIGHT + 3, 1, colorBorderFill);

		CG_DrawStringExt(42, 400, s, colorWhite, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
		CG_DrawStringExt(42, 420, s2, colorWhite, qfalse, qtrue, TINYCHAR_WIDTH - 1, TINYCHAR_HEIGHT - 1, 0);
	}
}

//////////////////////////////////////////////
//////////////////////////////////////////////
//
//      WINDOW HANDLING AND PRIMITIVES
//
//////////////////////////////////////////////
//////////////////////////////////////////////


// Windowing system setup
void CG_windowInit( void ) {
	int i;

	cg.winHandler.numActiveWindows = 0;
	for ( i = 0; i < MAX_WINDOW_COUNT; i++ ) {
		cg.winHandler.window[i].inuse = qfalse;
	}
	
	cg.controlsWindow = NULL;
}


// Window stuct "constructor" with some common defaults
void CG_windowReset( cg_window_t *w, int fx, int startupLength ) {
	// L0 - Made it more darker 
	vec4_t colorGeneralBorder = { 0.4f, 0.4f, 0.4f, 0.5f };
	vec4_t colorGeneralFill   = { 0.1f, 0.1f, 0.1f, 0.8f };

	w->effects = fx;
	w->fontScaleX = 0.25;
	w->fontScaleY = 0.25;
	w->flashPeriod = 1000;
	w->flashMidpoint = w->flashPeriod / 2;
	w->id = WID_NONE;
	w->inuse = qtrue;
	w->lineCount = 0;
	w->state = ( fx >= WFX_FADEIN ) ? WSTATE_START : WSTATE_COMPLETE;
	w->targetTime = ( startupLength > 0 ) ? startupLength : 0;
	w->time = trap_Milliseconds();
	w->x = 0;
	w->y = 0;

	memcpy( &w->colorBorder, &colorGeneralBorder, sizeof( vec4_t ) );
	memcpy( &w->colorBackground, &colorGeneralFill, sizeof( vec4_t ) );
}

// Reserve a window
cg_window_t *CG_windowAlloc( int fx, int startupLength ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	if ( wh->numActiveWindows == MAX_WINDOW_COUNT ) {
		return( NULL );
	}

	for ( i = 0; i < MAX_WINDOW_COUNT; i++ ) {
		w = &wh->window[i];
		if ( w->inuse == qfalse ) {
			CG_windowReset( w, fx, startupLength );
			wh->activeWindows[wh->numActiveWindows++] = i;
			return( w );
		}
	}

	// Fail if we're a full airplane
	return( NULL );
}


// Free up a window reservation
void CG_windowFree( cg_window_t *w ) {
	int i, j;
	cg_windowHandler_t *wh = &cg.winHandler;

	if ( w == NULL ) {
		return;
	}

	if ( w->effects >= WFX_FADEIN && w->state != WSTATE_OFF && w->inuse == qtrue ) {
		w->state = WSTATE_SHUTDOWN;
		w->time = trap_Milliseconds();
		return;
	}

	for ( i = 0; i < wh->numActiveWindows; i++ ) {
		if ( w == &wh->window[wh->activeWindows[i]] ) {
			for ( j = i; j < wh->numActiveWindows; j++ ) {
				if ( j + 1 < wh->numActiveWindows ) {
					wh->activeWindows[j] = wh->activeWindows[j + 1];
				}
			}

			w->id = WID_NONE;
			w->inuse = qfalse;
			w->state = WSTATE_OFF;

			CG_removeStrings( w );

			wh->numActiveWindows--;

			break;
		}
	}
}

void CG_windowCleanup( void ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	for ( i = 0; i < wh->numActiveWindows; i++ ) {
		w = &wh->window[wh->activeWindows[i]];
		if ( !w->inuse || w->state == WSTATE_OFF ) {
			CG_windowFree( w );
			i--;
		}
	}
}

// Main window-drawing handler
void CG_windowDraw( void ) {
	int h, x, y, i, j, milli, t_offset, tmp;
	cg_window_t *w;
	qboolean fCleanup = qfalse;
	vec4_t *bg;
	vec4_t textColor, borderColor, bgColor;

	CG_demoView();

	if ( cg.winHandler.numActiveWindows == 0 ) {		
		if (demo_controlsWindow.integer) {
			cgs.demoControlInfo.show = SHOW_ON;
			CG_createControlsWindow();
		}
		return;
	}

	milli = trap_Milliseconds();
	memcpy( textColor, colorWhite, sizeof( vec4_t ) );

	for ( i = 0; i < cg.winHandler.numActiveWindows; i++ ) {
		w = &cg.winHandler.window[cg.winHandler.activeWindows[i]];

		if ( !w->inuse || w->state == WSTATE_OFF ) {
			fCleanup = qtrue;
			continue;
		}

		if ( w->effects & WFX_TEXTSIZING ) {
			CG_windowNormalizeOnText( w );
			w->effects &= ~WFX_TEXTSIZING;
		}

		bg = ( ( w->effects & WFX_FLASH ) && ( milli % w->flashPeriod ) > w->flashMidpoint ) ? &w->colorBackground2 : &w->colorBackground;

		h = w->h;
		x = w->x;
		y = w->y;
		t_offset = milli - w->time;
		textColor[3] = 1.0f;
		memcpy( &borderColor, w->colorBorder, sizeof( vec4_t ) );
		memcpy( &bgColor, bg, sizeof( vec4_t ) );
		
		if ( w->state == WSTATE_START ) {
			tmp = w->targetTime - t_offset;
			if ( w->effects & WFX_SCROLLUP ) {
				if ( tmp > 0 ) {
					y += ( 480 - y ) * tmp / w->targetTime; 
				} else {
					w->state = WSTATE_COMPLETE;
				}

				w->curY = y;
			}
			// L0 - New ones (unsued)
			// NOTE -> Scroll right = (start) right and animate to (end) left and then back..
			//         Side indicates from which corner it pops in and pops out.
			if ( w->effects & WFX_SCROLLRIGHT ) {
				if ( tmp > 0 ) {
					x += ( 680 - x ) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				w->curX = x;
			}
			if ( w->effects & WFX_SCROLLLEFT ) {
				if ( tmp > 0 ) {
					x -= ( 680 + x ) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				w->curX = x;
			} // End
			if ( w->effects & WFX_FADEIN ) {
				if ( tmp > 0 ) {
					textColor[3] = (float)( (float)t_offset / (float)w->targetTime );
				} else { w->state = WSTATE_COMPLETE;}
			}
		} else if ( w->state == WSTATE_SHUTDOWN ) {
			tmp = w->targetTime - t_offset;
			if ( w->effects & WFX_SCROLLUP ) {
				if ( tmp > 0 ) {
					y = w->curY + ( 480 - w->y ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || y >= 480 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}	
			// L0 - New effects
			if ( w->effects & WFX_SCROLLRIGHT ) {
				if ( tmp > 0 ) {
					x = w->curX + ( 680 - w->x ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || x >= 680 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			if ( w->effects & WFX_SCROLLLEFT ) {
				if ( tmp > 0 ) {
					x = w->curX - ( 680 + w->x ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || x >= 680 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			} // End
			if ( w->effects & WFX_FADEIN ) {
				if ( tmp > 0 ) {
					textColor[3] -= (float)( (float)t_offset / (float)w->targetTime );
				} else {
					textColor[3] = 0.0f;
					w->state = WSTATE_OFF;
				}
			}
		}

		borderColor[3] *= textColor[3];
		bgColor[3] *= textColor[3];

		CG_FillRect( x, y, w->w, h, bgColor );
		CG_DrawRect( x, y, w->w, h, 1, borderColor );

		x += 5;
		y -= ( w->effects & WFX_TRUETYPE ) ? 3 : 0;

		for ( j = w->lineCount - 1; j >= 0; j-- ) {
			if ( w->effects & WFX_TRUETYPE ) {
				// L0 - If i'll ever port the font..this can be restored..
				//CG_Text_Paint_Ext( x, y + h, w->fontScaleX, w->fontScaleY, textColor,
				//				   (char*)w->lineText[j], 0.0f, 0, 0, &cgDC.Assets.textFont );

				// Note that size is fixed (for now)..
				CG_DrawStringExt(x, y - 7 + h, (char*)w->lineText[j], textColor, qfalse, qfalse,5, 10, 70);		
			}

			h -= ( w->lineHeight[j] + 3 );

			if ( !( w->effects & WFX_TRUETYPE ) ) {
				CG_DrawStringExt2( x, y + h, (char*)w->lineText[j], textColor,
								   qfalse, qtrue, w->fontWidth, w->fontHeight, 0 );
			}
		}
	}

	if ( fCleanup ) {
		CG_windowCleanup();
	}
}

// Set the window width and height based on the windows text/font parameters
void CG_windowNormalizeOnText( cg_window_t *w ) {
	int i, tmp;

	if ( w == NULL ) {
		return;
	}

	w->w = 0;
	w->h = 0;

	if ( !( w->effects & WFX_TRUETYPE ) ) {
		w->fontWidth = w->fontScaleX * WINDOW_FONTWIDTH;
		w->fontHeight = w->fontScaleY * WINDOW_FONTHEIGHT;
	}

	for ( i = 0; i < w->lineCount; i++ ) {
		if ( w->effects & WFX_TRUETYPE ) {
			//tmp = CG_Text_Width_Ext( (char*)w->lineText[i], w->fontScaleX, 0, &cgs.media.limboFont2 );			
			tmp = CG_Text_Width_Ext( (char*)w->lineText[i], w->fontScaleX, 0, &cgDC.Assets.textFont );
		} else {
			tmp = CG_DrawStrlen( (char*)w->lineText[i] ) * w->fontWidth;
		}

		if ( tmp > w->w ) {
			w->w = tmp;
		}
	}

	for ( i = 0; i < w->lineCount; i++ ) {
		if ( w->effects & WFX_TRUETYPE ) {				
			w->lineHeight[i] = CG_Text_Height_Ext( (char*)w->lineText[i], w->fontScaleY, 0, &cgDC.Assets.textFont );
		} else {
			w->lineHeight[i] = w->fontHeight;
		}

		w->h += w->lineHeight[i] + 3;
	}

	// Border + margins
	w->w += 10;
	w->h += 3;

	// Set up bottom alignment
	if ( w->x < 0 ) {
		w->x += 640 - w->w;
	}
	if ( w->y < 0 ) {
		w->y += 480 - w->h;
	}
}

void CG_printWindow( char *str ) {
	int pos = 0, pos2 = 0;
	char buf[MAX_STRING_CHARS];
	cg_window_t *w = cg.windowCurrent;

	if ( w == NULL ) {
		return;
	}

	// Silly logic for a strict format
	Q_strncpyz( buf, str, MAX_STRING_CHARS );
	while ( buf[pos] > 0 && w->lineCount < MAX_WINDOW_LINES ) {
		if ( buf[pos] == '\n' ) {
			if ( pos2 == pos ) {
				if ( !CG_addString( w, " " ) ) {
					return;
				}
			} else {
				buf[pos] = 0;
				if ( !CG_addString( w, buf + pos2 ) ) {
					return;
				}
			}
			pos2 = ++pos;
			continue;
		}
		pos++;
	}

	if ( pos2 < pos ) {
		CG_addString( w, buf + pos2 );
	}
}

//
// String buffer handling
//
void CG_initStrings( void ) {
	int i;

	for ( i = 0; i < MAX_STRINGS; i++ ) {
		cg.aStringPool[i].fActive = qfalse;
		cg.aStringPool[i].str[0] = 0;
	}
}

qboolean CG_addString( cg_window_t *w, char *buf ) {
	int i;

	// Check if we're reusing the current buf
	if ( w->lineText[w->lineCount] != NULL ) {
		for ( i = 0; i < MAX_STRINGS; i++ ) {
			if ( !cg.aStringPool[i].fActive ) {
				continue;
			}

			if ( w->lineText[w->lineCount] == (char *)&cg.aStringPool[i].str ) {
				w->lineCount++;
				cg.aStringPool[i].fActive = qtrue;
				strcpy( cg.aStringPool[i].str, buf );

				return( qtrue );
			}
		}
	}

	for ( i = 0; i < MAX_STRINGS; i++ ) {
		if ( !cg.aStringPool[i].fActive ) {
			cg.aStringPool[i].fActive = qtrue;
			strcpy( cg.aStringPool[i].str, buf );
			w->lineText[w->lineCount++] = (char *)&cg.aStringPool[i].str;

			return( qtrue );
		}
	}

	return( qfalse );
}

void CG_removeStrings( cg_window_t *w ) {
	int i, j;

	for ( i = 0; i < w->lineCount; i++ ) {
		char *ref = w->lineText[i];

		for ( j = 0; j < MAX_STRINGS; j++ ) {
			if ( !cg.aStringPool[j].fActive ) {
				continue;
			}

			if ( ref == (char *)&cg.aStringPool[j].str ) {
				w->lineText[i] = NULL;
				cg.aStringPool[j].fActive = qfalse;
				cg.aStringPool[j].str[0] = 0;

				break;
			}
		}
	}
}

