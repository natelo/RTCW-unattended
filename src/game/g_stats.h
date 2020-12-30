/*
---------------------------
Nate 'L0 - g_stats.h
Created: 29.07/14
Updated: 31.07/14

Holds stuff related to stats.
---------------------------
*/
#ifndef _STATS
#define _STATS

#define PB_GUID_LENGTH 33 // PB : 32 + trailing zero
#define WEBSTATS_LOG "webStats.log"	// Log file

/*
============
Linked list
============
*/
struct statEntry {
	char info[4096]; 
	struct statEntry *next; 
};

struct statEntry *statHead, *statTail;
int statNumber;

/*
============
MOD and WP_ Conversions
============
*/
typedef enum {
	STATS_MP40,
	STATS_THOMPSON,
	STATS_STEN,
	STATS_MAUSER,
	STATS_SNIPERRIFLE,
	STATS_FLAMETHROWER,
	STATS_PANZERFRAUST,
	STATS_VENOM,
	STATS_GRENADE,
	STATS_LUGER,
	STATS_COLT,
	STATS_DYNAMITE,
	STATS_MG42,
	STATS_KNIFE,
	STATS_KNIFESTEALTH,
	STATS_KNIFETHROW,
	STATS_AIRSTRIKE,
	STATS_ARTILLERY,
	STATS_POISON,
	STATS_GOOMBA,
	STATS_FALLING,
	STATS_MORTAR,
	STATS_SUICIDE,
	STATS_CHICKEN,
	STATS_DROWN,
	STATS_WORLD,
	STATS_ADMIN,
	STATS_MAX
} statsMODs;

//
// MODs
//
typedef struct {
	int count;
} web_MODs_s;

//
// Hit List
//
typedef struct {	
	int count;
	char guid[PB_GUID_LENGTH];
} web_deathList_s;

//
// Class time
//
typedef struct {
	int count;
	int time;
} web_classTime_s;

//
// Stats Time
//
typedef struct {
	int alivePeak;
	int time;
	int overall;
	int axisTeam;
	int alliedTeam;
} web_statsTimers_s;

//
// Stats
//
typedef struct {
	int		kills;
	int		deaths;
	int		headshots;
	int		teamKills;	
	int		poison;
	int		revives;
	int		revivesRec;
	int		ammoGiv;	
	int		medGiv;		
	int		medRec;
	int		ammoRec;
	int		gibs;
	int		suicides;
	int		goomba;
	int		knife;
	int		knifeStealth;
	int		knifeThrow;
	int		killPeak;
	int		deathPeak;
	int		shotsFired;
	int		shotsHit;
	int		dynoPlanted;
	int		dynoDisarmed;
	int		mgsRepaired;
	int		ASCalled;
	int		ASThrown;
	int		ASBlocked;
	int		chickenRun;
	int		dmgTeam;
	int		dmgGiv;
	int		dmgRec;
	int		objSteals;
	int		flagCapture;
	int		flagReclaim;

	// These are mapped on their own
	int wShotsFired[STATS_MAX];	
	int wShotsHit[STATS_MAX];
	int wShotsRec[STATS_MAX];
	int wHeadshots[STATS_MAX];
	int wHeadshotsRec[STATS_MAX];
	int wGibs[STATS_MAX];
	int wGibsBy[STATS_MAX];
	int wDmgGvn[STATS_MAX];
	int wDmgRcv[STATS_MAX];
	int wTDmgGvn[STATS_MAX];
	int wTDmgRcv[STATS_MAX];
} statsClientData_t;

/**** Killing Sprees ****/
typedef struct {
	char *msg;
	char *snd;
} killing_sprees_t;

static const killing_sprees_t killingSprees[] = {
	{ "MULTI KILL!", "multikill.wav" },
	{ "MEGA KILL!", "megakill.wav" },
	{ "RAMPAGE!", "rampage.wav" },
	{ "GUNSLINGER!", "gunslinger.wav" },
	{ "ULTRA KILL!", "ultrakill.wav" },
	{ "MANIAC!", "maniac.wav" },
	{ "SLAUGHTER!", "slaughter.wav" },
	{ "MASSACRE!", "massacre.wav" },
	{ "IMPRESSIVE!", "impressive.wav" },
	{ "DOMINATING!", "dominating.wav" },
	{ "BLOODBATH!", "bloodbath.wav" },
	{ "KILLING MACHINE!", "killingmachine.wav" },
	{ "MONSTER KILL!", "monsterkill.wav" },
	{ "LUDICROUS KILL!", "ludicrouskill.wav" },
	{ "UNSTOPPABLE!", "unstoppable.wav" },
	{ "UNREAL!", "unreal.wav" },
	{ "OUTSTANDING!", "outstanding.wav" },
	{ "WICKED SICK!", "wickedsick.wav" },
	{ "HOLY SHIT!", "holyshit.wav" },
	{ "BLAZE OF GLORY!!", "blazeofglory.wav" },
	{ NULL, NULL }
};

/**** Killer Sprees (resets when player dies) ****/
typedef struct {
	char *msg;
	char *snd;
} killer_sprees_t;

static const killer_sprees_t killerSprees[] = {
	{ "MULTI KILL!", "multikill.wav" },
	{ "KILLING SPREE!", "killingspree.wav" },
	{ "RAMPAGE!", "rampage.wav" },
	{ "ULTRA KILL!", "ultraKill.wav" },
	{ "MONSTER KILL!", "monsterkill.wav" },
	{ "LUDICROUS KILL!", "ludicrouskill.wav" },
	{ "DOMINATING!", "dominating.wav" },
	{ "GODLIKE!", "godlike.wav" },
	{ "UNSTOPPABLE!", "unstoppable.wav" },
	{ "WICKED SICK!", "wickedsick.wav" },
	{ "HOLY SHIT!!", "holyshit.wav" },
	{ NULL, NULL }
};

/**** Map Stats ****/
#define MAP_KILLER			1
#define MAP_KILLING_SPREE   2
#define MAP_VICTIM			3
#define MAP_DEATH_SPREE		4
#define MAP_REVIVES			5
#define MAP_HEADSHOTS		6

/**** RMS (Round Match Stats) ****/
#define ROUND_PRINT			1	// Should always be First
#define ROUND_KILLS			2
#define ROUND_DEATHS		3
#define ROUND_HEADSHOTS		4
#define ROUND_TEAMKILLS		5
#define ROUND_TEAMBLEED		6
#define ROUND_POISON		7
#define ROUND_REVIVES		8
#define ROUND_AMMOGIVEN		9
#define ROUND_MEDGIVEN		10
#define ROUND_GIBS			11
#define ROUND_SUICIDES		12
#define ROUND_GOOMBAS		13
#define ROUND_KNIFETHROW	14
#define ROUND_FASTSTABS		15
#define ROUND_CHICKEN		16
#define ROUND_KILLPEAK		17
#define ROUND_DEATHPEAK		18
#define ROUND_ACC			19
#define ROUND_KR			20
#define ROUND_EFF			21
#define ROUND_LIMIT			22	// Should always be Last!

typedef struct {
	char *reward;
	char *snd;
} round_stats_t;

static const round_stats_t rSM[] = {
	{ "Match Results", "xmod/sound/scenaric/achievers/intro.wav" },
	{ "Most Kills", "xmod/sound/scenaric/achievers/blazeofglory.wav" },
	{ "Most Deaths", "xmod/sound/scenaric/achievers/yousuck.wav" },
	{ "Most Headshots", "xmod/sound/scenaric/achievers/headhunter.wav" },
	{ "Most Team Kills", "xmod/sound/scenaric/achievers/teamkiller.wav" },
	{ "Most Team Bleeding", "xmod/sound/scenaric/achievers/teambleeder.wav" },
	{ "Most Poisons", "xmod/sound/scenaric/achievers/toxic.wav" },
	{ "Most Revives", "xmod/sound/scenaric/achievers/excellent.wav" },
	{ "Most Ammo Given", "xmod/sound/scenaric/achievers/ammo.wav" },
	{ "Most Med Given", "xmod/sound/scenaric/achievers/med.wav" },
	{ "Most Gibs", "xmod/sound/scenaric/achievers/ownage.wav" },
	{ "Most Suicides", "xmod/sound/scenaric/achievers/suicides.wav" },
	{ "Most Goomba Kills", "sound/player/gibsplt1.wav" },
	{ "Knife Juggler", "xmod/sound/scenaric/achievers/knife_juggler.wav" },
	{ "Stealth Kills", "xmod/sound/scenaric/achievers/assasin.wav" },
	{ "Coward", "xmod/sound/scenaric/achievers/chicken.wav" },
	{ "Highest Kill Spree", "xmod/sound/scenaric/achievers/killingmachine.wav" },
	{ "Highest Death Spree", "xmod/sound/scenaric/achievers/slaughter.wav" },
	{ "Highest Accuracy", "xmod/sound/scenaric/achievers/accuracy.wav" },
	{ "Highest Kill Ratio", "xmod/sound/scenaric/achievers/outstanding.wav" },
	{ "Most Efficient", "xmod/sound/scenaric/achievers/impressive.wav" },
	{ NULL, NULL }
};

struct round_stats_structure_s {
	unsigned int stats;				// Label (see defines above)
	unsigned int score;				// Most of stats
	char player[MAX_TOKEN_CHARS];	// Players that made the list
	char out[MAX_TOKEN_CHARS];		// For output
};

typedef struct round_stats_structure_s roundStruct;
extern roundStruct roundStats[];

#endif // _STATS
