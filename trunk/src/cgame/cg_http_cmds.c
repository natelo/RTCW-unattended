/*
===========================================================================

	Nate 'L0,
	Some basic Web wrappers
	
	ABOUT:	
			Commands itself do not do much but fire a POST packet to a 
			targeted server. Complexity solely relies on a web script
			so in essence, one can completely avoid client updates due web stuff
			and simply maintain/code/configure/tweak web site and introduce
			new features by utilizing queries and callbacks for info about new stuff.

	NOTE:	
			Technology behind website doesn't matter as game doesn't care about 
			that so python, php, java, ruby, asp..everything is a go.

			If you are planning to use your own stuff then all you need to do
			is deal with POST fields as script sends data as POST fields but note that 
			(atm) you will have to construct POST list your self albeit basic function
			(CG_cmdsToPost) for it is present.

	MODDERS:
			By default stuff is quite limited..work with GAME module to add
			complexity..like Client->Command --> Server->server replies --> Client parses & calls sys stuff
			That way you can use guid stuff like: /webstats chances <target slot> 
			and script returns win probability based upon guid (from server's module reply) comparison in back end..

===========================================================================
*/

#include "cg_local.h"

// So it's simplier
#ifndef _DEBUG
#define WEB_BANS	"http://client.rtcwmp.com/api/bans"
#define WEB_REPORTS	"http://client.rtcwmp.com/api/reports"
#define WEB_INFO	"http://client.rtcwmp.com/api/info"
#define WEB_ADMIN	"http://client.rtcwmp.com/api/admin"
#else
#define WEB_BANS	"http://127.0.0.1/stats/query/bans"
#define WEB_REPORTS	"http://127.0.0.1/stats/query/reports"
#define WEB_INFO	"http://127.0.0.1/stats/query/info"
#define WEB_ADMIN	"http://127.0.0.1/stats/query/admin"
#endif

/*
	Builds POST fields for query
	- Outputs standard POST query:	
	  count=var1&count=var2&count=var3...

	On a web side of things, you can use a loop for rtcw_ post to map fields- e.g. [PHP]:
	--------------------------------------
	...
	$arr = array();
	foreach($_POST as $key => $value) {
		// value starts with rtcw_
		if (strpos($key, 'rtcw_') === 0) {
			$arr[$key] = $value;
		}
	}

	// Do something with it..
	...
	--------------------------------------
	NOTE: A basic example...don't be naive and use sanity checks..
*/
char *CG_cmdsToPost(int firstOffset, int lastOffset) {
	int i;
	char *cList = NULL;

	for (i = 0; i <= trap_Argc(); i++) {
		if (i == firstOffset)
			cList = va("rtcw_%d=%s", i, CG_Argv(i));
		else {
			if (i > firstOffset && i <= lastOffset )
				cList = va("%s&rtcw_%d=%s", cList, i, CG_Argv(i));
		}
	}
	return cList;
}

/*
	Lists latest bans
	- Lists latest 5 or 10 bans..
*/
void CG_Web_LatestBans(char *cmd) { trap_HTTP_Post_cmd(WEB_BANS, cmd); }
/*
	Grab Reports
	- It will probably be used for something like /webreports latest cheaters
*/
void CG_Web_Reports(char *cmd) { trap_HTTP_Post_cmd(WEB_REPORTS, cmd); }
/*
	Misc info
	- No Idea what I'll do with this but probably something in sense of /webinfo latest cups
*/
void CG_Web_Info(char *cmd) { trap_HTTP_Post_cmd(WEB_INFO, cmd); }
/*
	Admin info
	- Some admin related info
*/
void CG_Web_Admin(char *cmd) { trap_HTTP_Post_cmd(WEB_ADMIN, cmd); }
