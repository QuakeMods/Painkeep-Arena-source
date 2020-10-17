// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#define	GAME_VERSION		"baseq3-1"

//PKMOD - Ergodic 02/01/01 - add variable for PKARENA game version
//PKMOD - Ergodic 03/27/01 - re-version from 2.1b to 2.2
//PKMOD - Ergodic 04/21/01 - re-version from 2.2 to 2.3
//PKMOD - Ergodic 04/25/01 - re-version from 2.3 to 2.4
//PKMOD - Ergodic 06/03/01 - re-version from 2.4 to 2.5
//PKMOD - Ergodic 07/11/01 - re-version from 2.5 to 2.6
//PKMOD - Ergodic 07/14/01 - re-version from 2.6 to 2.7
//PKMOD - Ergodic 07/25/01 - re-version from 2.7 to 2.7t
//PKMOD - Ergodic 07/31/01 - re-version from 2.7t to 2.7
//PKMOD - Ergodic 08/14/01 - re-version from 2.7 to 2.7F
//PKMOD - Ergodic 08/16/01 - re-version from 2.7F to 2.7G
//PKMOD - Ergodic 08/24/01 - re-version from 2.7G to 2.7H
//PKMOD - Ergodic 08/26/01 - re-version from 2.7H to 2.8
//PKMOD - Ergodic 09/30/01 - re-version from 2.8 to 2.9a
//PKMOD - Ergodic 11/23/01 - re-version from 2.9a to 2.9b
//PKMOD - Ergodic 12/07/01 - re-version from 2.9b to 2.9c
//PKMOD - Ergodic 01/20/02 - re-version from 2.9c to 2.9d
//PKMOD - Ergodic 02/14/02 - re-version from 2.9d to 2.9e
//PKMOD - Ergodic 05/24/02 - re-version from 2.9e to 2.9f
//PKMOD - Ergodic 08/02/02 - re-version from 2.9f to 2.9g
//PKMOD - Ergodic 09/24/02 - re-version from 2.9g to 2.9h
//PKMOD - Ergodic 10/18/02 - re-version from 2.9h to 2.9i
//PKMOD - Ergodic 10/24/02 - re-version from 2.9i to 2.9j
//PKMOD - Ergodic 11/03/02 - re-version from 2.9j to 2.9k
//PKMOD - Ergodic 11/28/02 - re-version from 2.9k to 2.9L
//PKMOD - Ergodic 12/07/02 - re-version from 2.9L to 2.9m
//PKMOD - Ergodic 12/07/02 - re-version from 2.9m to 2.9n
//PKMOD - Ergodic 12/07/02 - re-version from 2.9n to 3.0
//PKMOD - Ergodic 05/15/03 - re-version from 3.0 to 3.0a
//PKMOD - Ergodic 09/23/03 - re-version from 3.0a to 3.0b
//PKMOD - Ergodic 09/25/03 - re-version from 3.0b to 3.0c
//PKMOD - Ergodic 12/13/03 - re-version from 3.0c to 3.0d
//PKMOD - Ergodic 12/18/03 - re-version from 3.0d to 3.0e
//PKMOD - Ergodic 01/03/04 - re-version from 3.0e to 3.0f
//PKMOD - Ergodic 01/03/04 - re-version from 3.0f to 3.0g
//PKMOD - Ergodic 02/06/04 - re-version from 3.0g to 3.0h
//PKMOD - Ergodic 02/11/04 - re-version from 3.0h to 3.0i
//PKMOD - Ergodic 02/24/04 - re-version from 3.0i to 3.0j
//PKMOD - Ergodic 02/27/04 - re-version from 3.0j to 3.0k
//PKMOD - Ergodic 03/15/04 - re-version from 3.0k to 3.0m (skip l)
//PKMOD - Ergodic 03/29/04 - re-version from 3.0m to 3.0n 
#define	PKARENA_VERSION		"pkarena 3.0n"

#define	DEFAULT_GRAVITY		800
#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.66

#define	MAX_ITEMS			256

#define	RANK_TIED_FLAG		0x4000

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection

#define	LIGHTNING_RANGE		768

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define	MINS_Z				-24
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC				2
#define	CS_MESSAGE				3		// from the map worldspawn's message field
#define	CS_MOTD					4		// g_motd string for server message of the day
#define	CS_WARMUP				5		// server time when the match will be restarted
#define	CS_SCORES1				6
#define	CS_SCORES2				7
#define CS_VOTE_TIME			8
#define CS_VOTE_STRING			9
#define	CS_VOTE_YES				10
#define	CS_VOTE_NO				11

#define CS_TEAMVOTE_TIME		12
#define CS_TEAMVOTE_STRING		14
#define	CS_TEAMVOTE_YES			16
#define	CS_TEAMVOTE_NO			18

#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define	CS_INTERMISSION			22		// when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS			23		// string indicating flag status in CTF
#define CS_SHADERSTATE			24
#define CS_BOTINFO				25

#define	CS_ITEMS				27		// string of 0's and 1's that tell which items are present
//PKMOD - Ergodic 10/13/00 - add alternate music to hub
#define	CS_POSTVOTE_MUSIC		32
//PKMOD - Ergodic 02/01/01 - add variable for PKARENA game version
#define	CS_PKARENA_VERSION		33

#define	CS_MODELS				34  //PKMOD was 32
#define	CS_SOUNDS				(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS				(CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)
#define CS_PARTICLES			(CS_LOCATIONS+MAX_LOCATIONS) 

#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
	GT_FFA,				// free for all
	GT_TOURNAMENT,		// one on one tournament
	GT_SINGLE_PLAYER,	// single player ffa

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
	GT_CTF,				// capture the flag
	GT_1FCTF,
	GT_OBELISK,
	GT_HARVESTER,
	GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
	PM_NORMAL,		// can accelerate and turn
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION,	// no movement or status bar
	PM_SPINTERMISSION	// no movement or status bar
} pmtype_t;

typedef enum {
	WEAPON_READY, 
	WEAPON_RAISING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2

//PKMOD - Ergodic 05/12/01 - add flag for next_holdable button being held
#define	PMF_NEXTHOLD_HELD	4

#define	PMF_BACKWARDS_JUMP	8		// go into backwards land
#define	PMF_BACKWARDS_RUN	16		// coast down to backwards run
#define	PMF_TIME_LAND		32		// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	64		// pm_time is an air-accelerate only time
#define	PMF_TIME_WATERJUMP	256		// pm_time is waterjump
#define	PMF_RESPAWNED		512		// clear after attack and jump buttons come up
#define	PMF_USE_ITEM_HELD	1024
#define PMF_GRAPPLE_PULL	2048	// pull towards grapple location
#define PMF_FOLLOW			4096	// spectate following another player
#define PMF_SCOREBOARD		8192	// spectate as a scoreboard
#define PMF_INVULEXPAND		16384	// invulnerability sphere set to full size

#define	PMF_ALL_TIMES	(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define	MAXTOUCH	32
typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server
	qboolean	gauntletHit;		// true if a gauntlet attack would actually hit something

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;

	// for fixed msec Pmove
	int			pmove_fixed;
	int			pmove_msec;

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );

	int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================
//PKMOD - Ergodic 07/10/00 add definition for beartraps attached constant for the 
//			angles2 co-opt hack. the entitystate_t angles2[beartraps_attached] 
//			variable will link the beartrap info from the game to the client
#define  BEARTRAPS_ATTACHED		0  //this will coop the pitch of angles2

//PKMOD - Ergodic 06/18/00 add PKA items status area
// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum {
	STAT_HEALTH,
	STAT_HOLDABLE_ITEM,
#ifdef MISSIONPACK
	STAT_PERSISTANT_POWERUP,
#endif
	STAT_WEAPONS,					// 16 bit fields
	STAT_PKA_ITEMS,					// 16 bit fields
	STAT_ARMOR,				
	STAT_DEAD_YAW,					// look this direction when dead (FIXME: get rid of?)
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)
	STAT_MAX_HEALTH,				// health / armor limit, changable by handicap
	STAT_BEARTRAPS_ATTACHED,		//PKMOD - Ergodic 06/30/00, beartraps attached to player
	//PKMOD - Ergodic 01/19/02 - reuse STAT_VOTING_MODE field for other purposes
//	STAT_VOTING_MODE,				//PKMOD - Ergodic 09/24/00, add player voting mode state
	STAT_PKA_BITS,					//PKMOD - Ergodic 01/19/02, add PKA miscellaneos bits to playerstate
	STAT_AIRFIST_LEVEL,				//PKMOD - Ergodic 11/15/00, add af_lev to player state
									//		values (4,3,2,1,0)
	STAT_PRIOR_AIRFIST_LEVEL,		//PKMOD - Ergodic 11/15/00, holds af level at time of blast
									//		values (4,3,2,1,0)
	STAT_LAST_WEAPON,				//PKMOD - Ergodic 04/04/01, holds last weapon
	STAT_ACTIVE_HOLDABLE,			//PKMOD - Ergodic 05/11/01 - allow holding of more than 1 type of
									//			holdable but only 1 of each kind
	STAT_CLG_SHAFTEE_NUM			//PKMOD - Ergodic 12/16/03, hold enitynumber for prime shafted target
} statIndex_t;


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_ATTACKEE_ARMOR,			// health/armor of last person we attacked
	PERS_KILLED,					// count of the number of times you died
	// player awards tracking
	PERS_IMPRESSIVE_COUNT,			// two railgun hits in a row
	PERS_EXCELLENT_COUNT,			// two successive kills in a short amount of time
	PERS_DEFEND_COUNT,				// defend awards
	PERS_ASSIST_COUNT,				// assist awards
	PERS_GAUNTLET_FRAG_COUNT,		// kills with the guantlet
	PERS_CAPTURES,					// captures
//PKMOD - Ergodic 08/08/00 add PKA Medal - packed variable
	PERS_PAINKILLER_COUNT
//PKMOD - Ergodic 10/11/00 add PKA HUB Flag - packed variable
	//PKMOD - Ergodic 12/16/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
	//		
} persEnum_t;


// entityState_t->eFlags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#ifdef MISSIONPACK
#define EF_TICKING			0x00000002		// used to make players play the prox mine ticking sound
#endif
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define EF_PLAYER_EVENT		0x00000010
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
//PKMOD Ergodic - 07/08/01, remove kamizaki and add lightningstrike to EF_ area
//#define	EF_KAMIKAZE			0x00000200
#define EF_LIGHTNINGSTRIKE	0x00000200		//player shooting the clg has struck an enemy

#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define EF_AWARD_CAP		0x00000800		// draw the capture sprite
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote
//PKMOD - Ergodic 08/08/00 - PK Medals
//PKMOD - Ergodic 12/16/00 - was EF_AWARD_PAINKILLER	0x00000800
#define	EF_AWARD_PAINKILLER	0x00100000		// draw a painkiller sprite

//PKMOD Ergodic - 12/28/00, add general PainKeepArena entity flags
//PKMOD Ergodic - 07/08/01, move lightningstrike to EF_ area
//#define PKAEF_LIGHTNINGSTRIKE	0x00000001		//player shooting the clg has struck an enemy
#define PKAEF_AUTOSENTRYFIRING	0x00000002		//autosentry is firing the guns
//PKMOD Ergodic - 01/11/01, add flag for trap door that causes death will credit
//							the activating player
#define PKAEF_DOORADDFRAG		0x00000004		//Activating player will be credited the frag(s)
//PKMOD Ergodic - 01/23/01, add flag for setting the turret portion of the autosentry to die
#define PKAEF_AUTOSENTRYDEATH	0x00000008		//autosentry is set to die
//PKMOD Ergodic - 01/30/01, add flag for setting the turret portion of the autosentry to Free
#define PKAEF_AUTOSENTRYFREE	0x00000010		//autosentry is set to Free (don't die twice)
//PKMOD Ergodic - 08/05/01, add flag for setting the BearTrap to die (used in g_mover)
#define PKAEF_BEARTRAPDIE		0x00000020		//Beartrap is set to die (don't die twice)
//PKMOD Ergodic - 10/25/01, add flag for setting the entity irradiated
#define PKAEF_IRRADIATED		0x00000040		//Entity is irradiated
//PKMOD Ergodic - 11/21/03, add flag for Gravity Well Earthquake sound
#define PKAEF_EARTHQUAKE		0x00000080		//Set when Earthquake Sound is issued
//PKMOD Ergodic - 01/05/04, add flag for beanstoot ent, when it is quaded
#define PKAEF_QUADFART			0x00000100		//Set when Quad Farting is enabled

//PKMOD - Ergodic 01/19/02 - STAT_PKA_BITS
//PKMOD - Ergodic 02/05/02 - change STAT_PKA_BITS settings from enum type to definition
#define PKA_BITS_PRIVATEBOT		0x00000001		//1 - flag for private bot
//PKMOD - Ergodic 02/05/02 - Add ATTACK Bits for hit sounds
#define PKA_BITS_DEFAULTATTACK	0x00000000		//0 - regular hit sound, 2 - beartrap hit sound, 3 - autosentry hit sound
#define PKA_BITS_BEARTRAPATTACK	0x00000002		//2 - beartrap hit sound
#define PKA_BITS_RADIATEATTACK	0x00000004		//4 - radiate hit sound
#define PKA_BITS_SENTRYATTACK	0x0000000E		//E - autosentry hit sound [NOTE: this bitfield should be a complete mask]


// NOTE: may not have more than 16
typedef enum {
	PW_NONE,			//0  - powerup

	PW_QUAD,			//1  - powerup
	PW_BATTLESUIT,		//2  - powerup
	PW_HASTE,			//3  - powerup
	PW_INVIS,			//4  - powerup
	PW_REGEN,			//5  - powerup
	PW_FLIGHT,			//6  - powerup

	PW_REDFLAG,			//7  - powerup
	PW_BLUEFLAG,		//8  - powerup
	PW_NEUTRALFLAG,		//9  - powerup

//PKMOD - Ergodic 10/13/01 - add powerup timer for radiate effect
//			This will redefine the PW_SCOUT area
	PW_RADIATE,			//10 - powerup
//	PW_SCOUT,			//10 - powerup
//PKMOD - Ergodic 05/07/02 - add powerup timer for Active Personal Sentry
//			This will redefine the PW_GUARD area
	PW_PERSENTRY,		//11 - powerup
//	PW_GUARD,			//11 - powerup
	PW_DOUBLER,			//12 - powerup
//	PW_AMMOREGEN,
//PKMOD - Ergodic 06/03/01 - add bean powerup timer for armor countdown immunity
	PW_BEANS,			//13 - powerup
	PW_INVULNERABILITY,	//14 - powerup
//PKMOD - Ergodic 08/22/00 - use powerup as vehicle to communicate clg shaft player hit info to cgame
	PW_CLGPLAYERHIT,	//15 - powerup
//PKMOD - Ergodic 09/20/00 - voting item
//PKMOD - Ergodic 12/16/00 - removed and need to NULL bg_misc entry. field is unused
//	PW_VOTING,

	PW_NUM_POWERUPS

} powerup_t;

typedef enum {
	HI_NONE,

	HI_TELEPORTER,
	HI_MEDKIT,
	//PKMOD - Ergodic 10/06/01 - add new holdable
	HI_RADIATE,
	//PKMOD - Ergodic 11/23/01 - add new holdable
	HI_PERSENTRY,
	//PKMOD - Ergodic 12/01/01 - add new Private Bot holdables
	HI_BOTLEGS,
	HI_BOTTORSO,
	HI_BOTHEAD,
	HI_KAMIKAZE,
	HI_PORTAL,
	HI_INVULNERABILITY,

	HI_NUM_HOLDABLE
} holdable_t;

//PKMOD - Ergodic 05/14/00 - modify for PK weapons order (later: need to fix weapon 1 cycling)
//PKMOD - Ergodic 06/08/00 - modify for PK weapons order using 2 byte STAT_WEAPONS
//PKMOD - Ergodic 06/09/00 - Currently only 15 weapons means may not need to add expansion byte, just yet
typedef enum {
	WP_NONE,

	WP_GAUNTLET,			//1
	WP_MACHINEGUN,			//2
	WP_SHOTGUN,				//3
	WP_AIRFIST,				//4 PKMOD -Add Weapons.
	WP_NAILGUN,				//5
	WP_GRENADE_LAUNCHER,	//6
	WP_ROCKET_LAUNCHER,		//7
	WP_LIGHTNING,			//8
	WP_RAILGUN,				//9
	WP_GRAPPLING_HOOK,		//10 - this is the last weapon before special PK items
	WP_GRAVITY,				//11
	WP_SENTRY,				//12
	WP_BEARTRAP,			//13  PKMOD - Ergodic 06/08/00 Weapon enum #16
	WP_BEANS,				//14  - this is the last special PK item
	WP_EXPLODING_SHELLS,	//15 - 2002-Aug-18: this is the last weapon due to cg_weapons.c weapon cycling rules

	WP_NUM_WEAPONS,
	WP_CHAINLG,				//never select the chainlg
	WP_PLASMAGUN,			//never select the plasmagun
	WP_BFG					//never select the bgf

} weapon_t;

//PKMOD - Ergodic 06/18/00 - PKA Items Status
typedef enum {
	PKA_NONE,
	PKA_BEANS,			//1 - flag for active beans
	PKA_SHAFTED,		//2 - flag for client currently being shafted
	PKA_IRRADIATED,		//3 - flag for radiated "infected" client
	//PKMOD - Ergodic 02/05/02 - Add Bits for activated Personal Sentry
	PKA_PERSENTRY_ACTIVE,	//4 - Personal Sentry is active on player
	//PKMOD - Ergodic 09/14/03 - Add Bits for state when clint is voting for the alternate hub maps
	PKA_HUBALT_VOTING	//5 - Personal Sentry is active on player
} pka_items_t;

// reward sounds
typedef enum {
	REWARD_BAD,

	REWARD_IMPRESSIVE,
	REWARD_EXCELLENT,
	REWARD_DENIED,
	REWARD_GAUNTLET,
	REWARD_PAINKILLER
} reward_t;

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

#define	EVENT_VALID_MSEC	300

typedef enum {
	EV_NONE,

	EV_FOOTSTEP,
	EV_FOOTSTEP_METAL,
	EV_FOOTSPLASH,
	EV_FOOTWADE,
	EV_SWIM,

	EV_STEP_4,
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,

	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,

	EV_JUMP_PAD,			// boing sound at origin, jump sound on player

	EV_JUMP,
	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LEAVE,	// foot leaves
	EV_WATER_UNDER,	// head touches
	EV_WATER_CLEAR,	// head leaves

	EV_ITEM_PICKUP,			// normal item pickups are predictable
	EV_GLOBAL_ITEM_PICKUP,	// powerup / team sounds are broadcast to everyone

	EV_NOAMMO,
	EV_CHANGE_WEAPON,
	EV_FIRE_WEAPON,

	EV_USE_ITEM0,
	EV_USE_ITEM1,
	EV_USE_ITEM2,
	EV_USE_ITEM3,
	EV_USE_ITEM4,
	EV_USE_ITEM5,
	EV_USE_ITEM6,
	EV_USE_ITEM7,
	EV_USE_ITEM8,
	EV_USE_ITEM9,
	EV_USE_ITEM10,
	EV_USE_ITEM11,
	EV_USE_ITEM12,
	EV_USE_ITEM13,
	EV_USE_ITEM14,
	EV_USE_ITEM15,

	EV_ITEM_RESPAWN,
	EV_ITEM_POP,
	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,

	EV_GRENADE_BOUNCE,		// eventParm will be the soundindex

	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// no attenuation
	EV_GLOBAL_TEAM_SOUND,

	EV_BULLET_HIT_FLESH,
	EV_BULLET_HIT_WALL,

	EV_MISSILE_HIT,
	EV_MISSILE_MISS,
	EV_MISSILE_MISS_METAL,
	EV_RAILTRAIL,
	EV_SHOTGUN,
	EV_BULLET,				// otherEntity is the shooter

	EV_PAIN,
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,
	EV_OBITUARY,

	EV_POWERUP_QUAD,
	EV_POWERUP_BATTLESUIT,
	EV_POWERUP_REGEN,

	EV_GIB_PLAYER,			// gib a previously living player
	EV_SCOREPLUM,			// score plum

//#ifdef MISSIONPACK
	EV_PROXIMITY_MINE_STICK,
	EV_PROXIMITY_MINE_TRIGGER,
	EV_KAMIKAZE,			// kamikaze explodes
	EV_OBELISKEXPLODE,		// obelisk explodes
	EV_OBELISKPAIN,			// obelisk is in pain
	EV_INVUL_IMPACT,		// invulnerability sphere impact
	EV_JUICED,				// invulnerability juiced effect
	EV_LIGHTNINGBOLT,		// lightning bolt bounced of invulnerability sphere
//#endif

	EV_DEBUG_LINE,
	EV_STOPLOOPINGSOUND,
	EV_TAUNT,
	EV_TAUNT_YES,
	EV_TAUNT_NO,
	EV_TAUNT_FOLLOWME,
	EV_TAUNT_GETFLAG,
	EV_TAUNT_GUARDBASE,
	EV_TAUNT_PATROL,
	//
	// PKMOD - Ergodic 05/22/00 Events
	//
	EV_BEARTRAP_DIE,
	EV_BEARTRAP_SNAP, //PKMOD - Ergodic 07/01/00
	EV_CHAINLIGHTNING_STRIKE, //PKMOD - Ergodic 08/22/00
	EV_GRAVITY_RELEASED, //PKMOD - Ergodic 07/11/00
	EV_LIGHTNING_FX, //PKMOD - Ergodic 07/19/00
	EV_BEARTRAP_DROP,	//PKMOD - Ergodic 08/07/00
	EV_GRAVITYWELL_SUCK,	//PKMOD - Ergodic 09/06/00 gravity well item suck sounds from Mongusta

	EV_PKA_NOAMMO, //PKMOD - Ergodic 05/30/00
	EV_ITEM_REMOVE, //PKMOD - Ergodic 11/16/00 - add target_remove code
	EV_GLOBAL_ITEM_REMOVE, //PKMOD - Ergodic 11/20/00 - add target_remove code (powerups)

	EV_AUTOSENTRY_DROP,	//PKMOD - Ergodic 11/22/00
	EV_AUTOSENTRY_DIE, //PKMOD - Ergodic 11/22/00
	EV_SHOOTER_LIGHTNING, 	//PKMOD - Ergodic 12/06/00 - special lightning shooter event

	//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
	EV_AUTOSENTRY_HIT_WALL,
	EV_AUTOSENTRY_HIT_FLESH,

	//PKMOD - Ergodic 12/26/00 add Beans fart noises from Mongusta
	EV_BEANS_TOOT,
	//PKMOD - Ergodic 01/13/01 - add autosentry fire sounds from mongusta
	EV_AUTOSENTRY_FIRE,
	//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
	EV_LIGHTNING_WATER_DISCHARGE,
	//PKMOD - Ergodic 01/21/01 - exploding shells debug model
	EV_COORD,
	//PKMOD - Ergodic 03/26/01 - add autosentry ping sound
	EV_AUTOSENTRY_PING,
	//PKMOD - Ergodic 07/03/01 ChainLightning reflect sounds
	EV_CHAINLIGHTNING_REFLECT,
	//PKMOD - Ergodic 12/05/01 - Radiate Item & Radiate Player events
	EV_ITEM_RADIATE,
	EV_PLAYER_RADIATE,
	//PKMOD - Ergodic 01/07/02 - send message to client if not all Private Bot parts are held
	EV_INCOMPLETE_PRIVATEBOT,
	//PKMOD - Ergodic 02/07/02 - send message to client if Private Bot is completed
	EV_COMPLETED_PRIVATEBOT,
	//PKMOD - Ergodic 02/10/02 - send FRAG message to Private Bot's owner
	EV_PRIVATEBOT_FRAG,
	//PKMOD - Ergodic 03/18/02 - send message to client that no more Private Bots are available
	EV_NOAVAILABLE_PRIVATEBOTS,
	//PKMOD - Ergodic 06/09/02 - create the teleport flash for the personal sentry (teleport in sound)
	EV_TELE_IN_PERSONALSENTRY,
	//PKMOD - Ergodic 06/12/02 - generate sound effect for firing Personal Sentry
	EV_FIRE_PERSONALSENTRY,
	//PKMOD - Ergodic 08/03/02 - create the teleport flash for the personal sentry (teleport out sound)
	EV_TELE_OUT_PERSONALSENTRY,
	//PKMOD - Ergodic 11/20/03 - create earthquake sound for out of range gravity well
	//EV_EARTHQUAKE,
	//PKMOD - Ergodic 12/06/03 - create charge up sound for CLG striking Beartrap or Autosentry
	EV_CHAINLIGHTNING_CHARGE_UP,
	//PKMOD - Ergodic 01/05/04 - add quad farting logic for differing CG graphic sequence
	EV_QUADBEANS_TOOT
} entity_event_t;


typedef enum {
	GTS_RED_CAPTURE,
	GTS_BLUE_CAPTURE,
	GTS_RED_RETURN,
	GTS_BLUE_RETURN,
	GTS_RED_TAKEN,
	GTS_BLUE_TAKEN,
	GTS_REDOBELISK_ATTACKED,
	GTS_BLUEOBELISK_ATTACKED,
	GTS_REDTEAM_SCORED,
	GTS_BLUETEAM_SCORED,
	GTS_REDTEAM_TOOK_LEAD,
	GTS_BLUETEAM_TOOK_LEAD,
	GTS_TEAMS_ARE_TIED,
	GTS_KAMIKAZE
} global_team_sound_t;

// animations
typedef enum {
	BOTH_DEATH1,
	BOTH_DEAD1,
	BOTH_DEATH2,
	BOTH_DEAD2,
	BOTH_DEATH3,
	BOTH_DEAD3,

	TORSO_GESTURE,

	TORSO_ATTACK,
	TORSO_ATTACK2,

	TORSO_DROP,
	TORSO_RAISE,

	TORSO_STAND,
	TORSO_STAND2,

	LEGS_WALKCR,
	LEGS_WALK,
	LEGS_RUN,
	LEGS_BACK,
	LEGS_SWIM,

	LEGS_JUMP,
	LEGS_LAND,

	LEGS_JUMPB,
	LEGS_LANDB,

	LEGS_IDLE,
	LEGS_IDLECR,

	LEGS_TURN,

	TORSO_GETFLAG,
	TORSO_GUARDBASE,
	TORSO_PATROL,
	TORSO_FOLLOWME,
	TORSO_AFFIRMATIVE,
	TORSO_NEGATIVE,

	MAX_ANIMATIONS,

	LEGS_BACKCR,
	LEGS_BACKWALK,
	FLAG_RUN,
	FLAG_STAND,
	FLAG_STAND2RUN,

	MAX_TOTALANIMATIONS
} animNumber_t;


typedef struct animation_s {
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
	int		reversed;			// true if animation is reversed
	int		flipflop;			// true if animation should flipflop back to base
} animation_t;


// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		128


typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS
} team_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME		1000

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

//team task
typedef enum {
	TEAMTASK_NONE,
	TEAMTASK_OFFENSE, 
	TEAMTASK_DEFENSE,
	TEAMTASK_PATROL,
	TEAMTASK_FOLLOW,
	TEAMTASK_RETRIEVE,
	TEAMTASK_ESCORT,
	TEAMTASK_CAMP
} teamtask_t;

//PKMOD - Ergodic 06/05/03 - list order must match modNames in g_combat.c
/*PKMOD - Ergodic 06/05/03 - removed:
	MOD_A2K,
	MOD_EMPNUKE,
	MOD_HARPOON,
	MOD_CHAINLG,
*/	
// means of death
typedef enum {
	MOD_UNKNOWN,
	MOD_SHOTGUN,
	MOD_GAUNTLET,
	MOD_MACHINEGUN,
	MOD_GRENADE,
	MOD_GRENADE_SPLASH,
	MOD_ROCKET,
	MOD_ROCKET_SPLASH,
	MOD_PLASMA,
	MOD_PLASMA_SPLASH,
	MOD_RAILGUN,
	MOD_LIGHTNING,
	MOD_BFG,
	MOD_BFG_SPLASH,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
	MOD_GRAPPLE,
	//PKMOD -Add Weapons. 
	MOD_GRAVITY,
	MOD_SENTRY,
	MOD_BEARTRAP,
	MOD_AIRFIST,
	MOD_NAILGUN,
	MOD_NAIL,  //PKMOD - Ergodic 08/01/00
	MOD_EXPLODING_SHELLS,
	MOD_EXPLODING_SHELLS_SPLASH,
	//PKMOD - Ergodic 12/05/00 - add new types of PKA shooters
	MOD_SHOOTER_LIGHTNING,
	//PKMOD - Ergodic 01/11/01 - give frag credit to activator of CRUSH Death
	MOD_CRUSH_CREDIT,
	//PKMOD - Ergodic 01/13/01 - exploding autosentry will produce splash damage
	MOD_SENTRY_SPLASH,
	//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
	MOD_LIGHTNING_WATER_DISCHARGE,
	//PKMOD - Ergodic 02/01/01 - add can of beans toot damage
	MOD_BEANS_BLAST,
	//PKMOD - Ergodic 07/02/01 - add reverse damage on lightning from autosentry
	MOD_REVERSE_LIGHTNING,
	//PKMOD - Ergodic 10/29/01 - add holdable radiation death
	MOD_RADIATION,
	//PKMOD - Ergodic 06/08/02 - add personal sentry death
	MOD_PERSONALSENTRY,
	//PKMOD - Ergodic 10/23/02 - Create new Means of Death for the Dragon Blade
	MOD_DRAGONBLADE,
	//PKMOD - Ergodic 06/06/03 - Create new Means of Death for the Private Bot for logging use in games.log
	MOD_PRIVATEBOT,
	//PKMOD - Ergodic 01/05/04 - add QUAD FART beans toot damage
	MOD_QUADBEANS_BLAST
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
	IT_PERSISTANT_POWERUP,
	IT_TEAM,
	IT_VOTING,				//PKMOD - Ergodic 09/20/00 - Voting Entity 
	IT_PKARENA_ACTIVE		// EFX: No Rotation
} itemType_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];

	char		*icon;
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
gitem_t	*BG_FindItemForWeapon( weapon_t weapon );
gitem_t	*BG_FindItemForPowerup( powerup_t pw );
gitem_t	*BG_FindItemForHoldable( holdable_t pw );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );


// g_dmflags->integer flags
#define	DF_NO_FALLING			8
#define DF_FIXED_FOV			16
#define	DF_NO_FOOTSTEPS			32

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)


//
// entityState_t->eType
//
typedef enum {
	ET_GENERAL,				//  0
	ET_PLAYER,				//  1
	ET_ITEM,				//  2
	ET_MISSILE,				//  3
	ET_MOVER,				//  4
	ET_BEAM,				//  5
	ET_PORTAL,				//  6
	ET_SPEAKER,				//  7
	ET_PUSH_TRIGGER,		//  8
	ET_TELEPORT_TRIGGER,	//  9
	ET_INVISIBLE,			// 10
	ET_GRAPPLE,				// 11 - grapple hooked on wall
	ET_TEAM,				// 12

	//PKMOD Ergodic - 05/29/2000, add PKMOD's event type
	ET_BEARTRAP,			// 13
	//PKMOD Ergodic - 06/11/2000, add PKMOD's event type
	ET_BEARTRAP_FOLLOW,		// 14
	//PKMOD Ergodic - 07/01/2000, add PKMOD's event type
	ET_GRAVITY_WELL,		// 15
	//PKMOD Ergodic - 07/12/2000, add chain lightning event type
	ET_CHAIN_LIGHTNING,		// 16
	//PKMOD - Ergodic 07/16/00 special spawn functions (lightning_fx) 
	ET_LIGHTNING_FX,		// 17
	//PKMOD - Ergodic 08/03/00 event type for nail entities 
	ET_NAIL,				// 18
	//PKMOD - Ergodic 11/15/00 add functionality to make trigger_push silent
	ET_QUIET_TRIGGER,		// 19
	//PKMOD - Ergodic 01/29/01 autosentry entries must be contiguous and in order
	//			Do not change the order of ET_AUTOSENTRY, ET_AUTOSENTRY_DEPLOY, ET_AUTOSENTRY_DEPLOY,
	//			and ET_AUTOSENTRY_TURRET this will effect missile and hitscan weapons
	//			//g_weapons: shotgun, railgun, machinegun, and bullets
	//PKMOD - Ergodic 11/22/00 add autosentry launch event type
	ET_AUTOSENTRY_LAUNCH,	// 20
	//PKMOD Ergodic - 11/26/2000, add deploy autosentry entity type
	ET_AUTOSENTRY_DEPLOY,	// 21
	//PKMOD Ergodic - 12/02/2000, add split autosentry entity types
	ET_AUTOSENTRY_BASE,		// 22
	ET_AUTOSENTRY_TURRET,	// 23
	//PKMOD - Ergodic 03/14/01 - add dragon deployable weapon fire
	ET_DRAGON_DEPLOY,		// 24
	//PKMOD - Ergodic 06/07/01 - add ZOMBIE entity type
	ET_ZOMBIE,				// 25
	//PKMOD - Ergodic 08/02/01 - create a new entity type for door_trigger
	//								fixes the bug of dragon deploy hitting doors
	ET_DOOR_TRIGGER,		// 26
	//PKMOD - Ergodic 08/02/01 - create a new entity type for trigger_multiple
	//								fixes the bug of dragon deploy hitting doors
	ET_TRIGGER_MULTIPLE,	// 27
	//PKMOD - Ergodic 06/09/02 - create a new entity type for the active personal sentry
	ET_PERSONALSENTRY,		//28
	//PKMOD - Ergodic 08/20/03 - After a new shader beam was added to the CLG, we want to
	//				differentiate between shooter_Lightning(green) and CLG lightning(red)
	ET_SHOOTER_LIGHTNING,	//29


	ET_EVENTS,				// 30 - any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
	//PKMOD Ergodic - 03/29/2000, add airfist event type
	ET_AIRFIST				// 31

} entityType_t;



void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void	BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );


#define ARENAS_PER_TIER		4
#define MAX_ARENAS			1024
#define	MAX_ARENAS_TEXT		8192

#define MAX_BOTS			1024
#define MAX_BOTS_TEXT		8192

//PKMOD - Ergodic 03/18/01 - Gravity Well visual paramters
//PKMOD - Ergodic 03/25/01 - add Gravity Well micro contractions
	//++++++++++++++++++++++
	//	First Expansion
	//++++++++++++++++++++++
#define GWELL_EXPAND_STARTTIME_1			500
#define GWELL_EXPAND_ENDTIME_1				2000
#define GWELL_EXPAND_FADETIME_1				1500
#define GWELL_ORB_RADIUS_1					6

	//++++++++++++++++++++++
	//	First Contraction
	//++++++++++++++++++++++
#define GWELL_CONTRACTION_STARTTIME_1		2000
#define GWELL_CONTRACTION_ENDTIME_1			2500
#define GWELL_CONTRACTION_FADETIME_1		2250

	//++++++++++++++++++++++
	//	Second Expansion
	//++++++++++++++++++++++
#define GWELL_EXPAND_STARTTIME_2			2500
#define GWELL_EXPAND_ENDTIME_2				4000
#define GWELL_EXPAND_FADETIME_2				3500
#define GWELL_ORB_RADIUS_2					6

	//++++++++++++++++++++++
	//	Second Contraction
	//++++++++++++++++++++++
#define GWELL_CONTRACTION_STARTTIME_2		4000
#define GWELL_CONTRACTION_ENDTIME_2			4500
#define GWELL_CONTRACTION_FADETIME_2		4250

	//++++++++++++++++++++++
	//	Third Expansion
	//++++++++++++++++++++++
#define GWELL_EXPAND_STARTTIME_3			4500
#define GWELL_EXPAND_ENDTIME_3				6000
#define GWELL_EXPAND_FADETIME_3				5500
#define GWELL_ORB_RADIUS_3					6

	//++++++++++++++++++++++
	//	Third Contraction
	//++++++++++++++++++++++
#define GWELL_CONTRACTION_STARTTIME_3		6000
#define GWELL_CONTRACTION_ENDTIME_3			8000
#define GWELL_CONTRACTION_FADETIME_3		7500

#define GWELL_SHOCKWAVE_MAXRADIUS			1000
#define GWELL_SHOCKWAVE2_MAXRADIUS			1000

// Kamikaze

// 1st shockwave times
#define KAMI_SHOCKWAVE_STARTTIME		0
#define KAMI_SHOCKWAVEFADE_STARTTIME	1500
#define KAMI_SHOCKWAVE_ENDTIME			2000
// explosion/implosion times
#define KAMI_EXPLODE_STARTTIME			250
#define KAMI_IMPLODE_STARTTIME			2000
#define KAMI_IMPLODE_ENDTIME			2250
// 2nd shockwave times
#define KAMI_SHOCKWAVE2_STARTTIME		2000
#define KAMI_SHOCKWAVE2FADE_STARTTIME	2500
#define KAMI_SHOCKWAVE2_ENDTIME			3000
// radius of the models without scaling
#define KAMI_SHOCKWAVEMODEL_RADIUS		88
#define KAMI_BOOMSPHEREMODEL_RADIUS		72
// maximum radius of the models during the effect
#define KAMI_SHOCKWAVE_MAXRADIUS		1320
#define KAMI_BOOMSPHERE_MAXRADIUS		720
#define KAMI_SHOCKWAVE2_MAXRADIUS		704

//PKMOD - Ergodic 09/26/2000, hub voting entities
//PKMOD - Ergodic 09/15/2003, increase Index from 25 - 35, and increase name NAME from 50 to 70  
#define	MAX_HUB_INDEX		35		//maximum voting_images in the HUB
#define	MAX_HUB_NAME		70		//maximum size of mapname and maptitle

#define MAX_HUB_DISPLAY_NAME	40 //03/29/04 - maximum name size that is displayed in voting UI menu

typedef struct ghubInfo_s {
	int			map_votes;
	char		map_name[MAX_HUB_NAME];
	char		map_title[MAX_HUB_NAME];
	int			map_shader_index;
} ghubInfo_t;

//PKMOD - Ergodic 09/27/2000, included in both the game dll and the client
extern	int			Hub_Index;					//points to last Hub element 
extern	ghubInfo_t	hubInfo[];

//PKMOD - Ergodic 09/27/2000, included in both the game dll and the client
typedef struct	ghubsort_s {
	int			map_votes;
	int			hub_index;
} ghubsort_t;

//PKMOD - Ergodic 10/13/01 - define constants for new holdables
#define			PLAYER_RADIATE_TIME		25

//PKMOD - Ergodic 05/07/02 - define constants for new holdables
#define			PLAYER_PERSENTRY_TIME		25

//PKMOD - Ergodic 12/12/01 - PRIVATE BOT definitions
#define			MAX_PRIVATE_BOTS		5			//augment sv_maxclients
//PKMOD - Ergodic 01/13/02 - PRIVATE BOT definitions
extern	int		active_private_bots;			//number of active Private Bots

//PKMOD - Ergodic 10/10/03 - Maximum charge to the Beartrap or Autosentry to make it invisible
//PKMOD - Ergodic 12/13/03 - change value from 70 to 50
//PKMOD - Ergodic 03/17/04 - change value from 50 to 25
#define			MAX_INVISIBILITY_CHARGE		25


