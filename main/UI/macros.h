/*
===========================================================================
wolfX / GPLv3

Got tired of dealing with menus..we need to port css/html support or 
smth but until then...i made this as it's getting ridiculous.

NOTE: This currently only serves options->menu ...for large deployment
	  it would need to be standarized and made dynamic as it's static atm.

Author: Nate 'L0
Created: 3 April / 2013
Updated: 4 April / 2013
===========================================================================
*/

// ITEM_TYPE_YESNO
#define ROWYES(NAME, GROUP, TEXT, CVAR, RECT)	\
	itemDef {									\
			name ""##NAME						\
			group ""##GROUP						\
			type ITEM_TYPE_YESNO				\
			text ""##TEXT":  "					\
			cvar ""##CVAR""						\
			rect 22 $evalfloat(RECT) 400 12		\
			textalign ITEM_ALIGN_RIGHT			\
			textalignx 205						\
			textaligny 10						\
			textfont UI_FONT_NORMAL				\
			textscale .255						\
			textfont UI_FONT_NORMAL				\
			style WINDOW_STYLE_FILLED			\
			backcolor 1 1 1 .07					\
			forecolor 1 1 1 1					\
			visible 0							\
			mouseenter { show floater_message } \
			mouseexit { hide floater_message }	\
    	}

// ITEM_TYPE_MULTI
#define ROWMULTIF(NAME, GROUP, TEXT, CVAR, OPTIONS, RECT)	\
	itemDef {												\
			name ""##NAME									\
			group ""##GROUP									\
			type ITEM_TYPE_MULTI							\
			text ""##TEXT":  "								\
			cvar ""##CVAR""									\
			cvarFloatList { ""##OPTIONS }					\
			rect 22 $evalfloat(RECT) 400 12					\
			textalign ITEM_ALIGN_RIGHT						\
			textalignx 205									\
			textaligny 10									\
			textfont UI_FONT_NORMAL							\
			textscale .255									\
			textfont UI_FONT_NORMAL							\
			style WINDOW_STYLE_FILLED						\
			backcolor 1 1 1 .07								\
			forecolor 1 1 1 1								\
			visible 0										\
			mouseenter { show floater_message }				\
			mouseexit { hide floater_message }				\
    	}
/*
// ITEM_TYPE_MULTI (strings)
#define ROWMULTIS(NAME, GROUP, TEXT, CVAR, OPTIONS, RECT)	\
	itemDef {												\
			name ##NAME										\
			group ##GROUP									\
			type ITEM_TYPE_MULTI							\
			text ##TEXT":  "								\
			cvar ##CVAR										\
			cvarStrList { ##OPTIONS }						\
			rect ##RECT										\
			textalign ITEM_ALIGN_RIGHT						\
			textalignx 128									\
			textaligny 10									\
			textfont UI_FONT_NORMAL							\
			textscale .255									\
			textfont UI_FONT_NORMAL							\
			style WINDOW_STYLE_FILLED						\
			backcolor 1 1 1 .07								\
			forecolor 1 1 1 1								\
			visible 0										\
			mouseenter { show floater_message }				\
			mouseexit { hide floater_message }				\
    	}

// ITEM_TYPE_SLIDER
#define ROWSLIDER(NAME, GROUP, TEXT, CVAR, DEFAULT, FROM, TO, RECT)	\
	itemDef {														\
			name ##NAME												\
			group ##GROUP											\
      		type ITEM_TYPE_SLIDER									\
			text ##TEXT":  "										\
			cvarfloat ##CVAR ##DEFAULT ##FROM ##TO					\
			rect ##RECT												\
    	  	textalign ITEM_ALIGN_RIGHT								\
      		textalignx 128											\
     	 	textaligny 10											\
			textfont UI_FONT_NORMAL									\
			textscale .25											\
			style WINDOW_STYLE_FILLED								\
			backcolor 1 1 1 .07										\ 
    	  	forecolor 1 1 1 1										\
   	   		visible 0												\
			mouseenter { show floater_message}						\
			mouseexit { hide floater_message }						\
    	}
*/
// ITEM_TYPE_BIND
#define ROWBIND(NAME, GROUP, TEXT, CVAR, RECT)	\
	itemDef {									\
			name ""##NAME						\
			group ""##GROUP						\
			type ITEM_TYPE_BIND					\
			text ""##TEXT":  "					\
			cvar ""##CVAR""						\			
			rect 22 $evalfloat(RECT) 400 12		\
			textalign ITEM_ALIGN_RIGHT			\
			textalignx 205						\
			textaligny 10						\
			textfont UI_FONT_NORMAL				\
			textscale .25						\
			forecolor 1 1 1 1					\
			style WINDOW_STYLE_FILLED			\
			backcolor 1 1 1 .07					\
			visible 0							\
			mouseenter { show keyBindStatus }	\
			mouseexit { hide keyBindStatus }	\
		}

// Title inside menu
#define ROWTITLE(NAME, GROUP, TEXT, RECT )	\
	itemDef {								\
			name ##NAME						\
			group ##GROUP					\
			text ##TEXT						\
 			type 1							\
			rect 197 205 60 12				\
			textalign ##RECT				\
			textfont UI_FONT_NORMAL			\
			textscale .255					\
			textalignx 30					\
			textfont UI_FONT_NORMAL			\
			textaligny 12					\
			forecolor 1 0 0 1				\
			visible 0						\
			decoration						\	
		}	