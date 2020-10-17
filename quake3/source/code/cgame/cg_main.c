// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

//PKMOD - Ergodic 01/17/04 - Enable HUD in PKA3.0
//#ifdef MISSIONPACK
#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;
//#endif

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		//PKMOD - Ergodic 10/31/01 - debug CG_INIT arguments (inactive)
//		Com_Printf("vmMain - 0>%d<, 1>%d<, 2>%d<, 3>%d<, 4>%d<, 5>%d<, 6>%d<, 7>%d<, 8>%d<, 9>%d<, 10>%d<, 11>%d<\n", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		//PKMOD - Ergodic 10/31/01 - debug CG_DRAW_ACTIVE_FRAME arguments (inactive)
//		Com_Printf("vmMain - 0>%d<, 1>%d<, 2>%d<, 3>%d<, 4>%d<, 5>%d<, 6>%d<, 7>%d<, 8>%d<, 9>%d<, 10>%d<, 11>%d<\n", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
//PKMOD - Ergodic 01/17/04 - enable HUD code in PKA 3.0
//#ifdef MISSIONPACK
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
//#endif
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noVoiceChats;
vmCvar_t	cg_noVoiceText;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_scorePlum;
vmCvar_t 	cg_smoothClients;
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_oldRail;
vmCvar_t	cg_oldRocket;
vmCvar_t	cg_oldPlasma;
vmCvar_t	cg_trueLightning;
//PKMOD - Ergodic 08/17/2002 - add Client error for communication to UI module
vmCvar_t	cl_pkaerror;
//PKMOD - Ergodic 08/16/03 - add cvar for PKA full weapon cycling
vmCvar_t	cg_pkafullweaponcycling;



#ifdef MISSIONPACK
vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_singlePlayer;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_recordSPDemo;
vmCvar_t	cg_recordSPDemoName;
vmCvar_t	cg_obeliskRespawnDelay;
#endif

//PKMOD - Ergodic 02/02/04 - Enable this code so that POSTGAME will show proper completion time 
vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_recordSPDemo;
vmCvar_t	cg_recordSPDemoName;


//PKMOD - Ergodic 01/17/04 - Enable Hud Code
vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;

//PKMOD - Ergodic 05/11/01 - register holdables into their own array
//		for optimization purposes
int		cg_holdable[ HI_NUM_HOLDABLE ];

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = { // bk001129
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
#ifdef MISSIONPACK
	{ &cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE },
#else
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
#endif
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo
#ifdef MISSIONPACK
	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{ &cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
	{ &cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO},
	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{ &cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE},
	{ &cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO},
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
#endif

	//PKMOD - Ergodic 02/02/04 - Enable this code so that POSTGAME will show proper completion time 
	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{ &cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE},

	//PKMOD - Ergodic 01/17/03 - Activate HUD for PKA3.0
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
	{ &cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE},
	{ &cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE},
	{ &cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE},
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
	//PKMOD - Ergodic 08/17/2002 - add Client error for communication to UI module
	{ &cl_pkaerror, "cl_pkaerror", "0", CVAR_ROM },
	//PKMOD - Ergodic 08/16/03 - add cvar for PKA full weapon cycling
	{ &cg_pkafullweaponcycling, "cg_pkafullweaponcycling", "1", CVAR_ARCHIVE },
};

static int  cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

//PKMOD - Ergodic 10/10/2000 - 25 voting images' shader assignments - set to zero
char cg_voting_shader_flag[] = { "0000000000000000000000000" };

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;

	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*																																			
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		trap_Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

#endif

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	//PKMOD - Ergodic 07/30/01 - debug "bad magic" crash (inactive)
//	int dbg1, dbg2;
//	dbg1 = 1;
//	dbg2 = 2;

	// voice commands
#ifdef MISSIONPACK
	CG_LoadVoiceChats();
#endif

	cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav", qtrue );
	cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav", qtrue );
	cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav", qtrue );
	cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav", qtrue );
	cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav", qtrue );
	cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav", qtrue );
	cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav", qtrue );
	cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav", qtrue );
	cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav", qtrue );
	cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav", qtrue );
	cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav", qtrue );
#ifdef MISSIONPACK
	cgs.media.countPrepareTeamSound = trap_S_RegisterSound( "sound/feedback/prepare_team.wav", qtrue );
#endif

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

		cgs.media.captureAwardSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav", qtrue );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav", qtrue );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav", qtrue );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav", qtrue );

		cgs.media.redScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_red_scores.wav", qtrue );
		cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", qtrue );

		cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qtrue );
		cgs.media.captureOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", qtrue );

		cgs.media.returnYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", qtrue );
		cgs.media.returnOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );

		cgs.media.takenYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", qtrue );
		cgs.media.takenOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", qtrue );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_red_returned.wav", qtrue );
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", qtrue );
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", qtrue );
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_flag.wav", qtrue );
		}

#ifdef MISSIONPACK
		if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );
			cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", qtrue );
			cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", qtrue );
		}

		if ( cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qtrue );
			cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
		}

		if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
			cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound( "sound/teamplay/voc_base_attack.wav", qtrue );
		}
#else
		cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qtrue );
		cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
		cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qtrue );
		cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", qtrue );
		cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", qtrue );
#endif
	}

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
	cgs.media.wearOffSound = trap_S_RegisterSound( "sound/items/wearoff.wav", qfalse );
	cgs.media.useNothingSound = trap_S_RegisterSound( "sound/items/use_nothing.wav", qfalse );
	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav", qfalse );
	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav", qfalse );
	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav", qfalse );
	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav", qfalse );

#ifdef MISSIONPACK
	cgs.media.useInvulnerabilitySound = trap_S_RegisterSound( "sound/items/invul_activate.wav", qfalse );
	cgs.media.invulnerabilityImpactSound1 = trap_S_RegisterSound( "sound/items/invul_impact_01.wav", qfalse );
	cgs.media.invulnerabilityImpactSound2 = trap_S_RegisterSound( "sound/items/invul_impact_02.wav", qfalse );
	cgs.media.invulnerabilityImpactSound3 = trap_S_RegisterSound( "sound/items/invul_impact_03.wav", qfalse );
	cgs.media.invulnerabilityJuicedSound = trap_S_RegisterSound( "sound/items/invul_juiced.wav", qfalse );
	cgs.media.obeliskHitSound1 = trap_S_RegisterSound( "sound/items/obelisk_hit_01.wav", qfalse );
	cgs.media.obeliskHitSound2 = trap_S_RegisterSound( "sound/items/obelisk_hit_02.wav", qfalse );
	cgs.media.obeliskHitSound3 = trap_S_RegisterSound( "sound/items/obelisk_hit_03.wav", qfalse );
	cgs.media.obeliskRespawnSound = trap_S_RegisterSound( "sound/items/obelisk_respawn.wav", qfalse );

	cgs.media.ammoregenSound = trap_S_RegisterSound("sound/items/cl_ammoregen.wav", qfalse);
	cgs.media.doublerSound = trap_S_RegisterSound("sound/items/cl_doubler.wav", qfalse);
	cgs.media.guardSound = trap_S_RegisterSound("sound/items/cl_guard.wav", qfalse);
	cgs.media.scoutSound = trap_S_RegisterSound("sound/items/cl_scout.wav", qfalse);
#endif

	cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav", qfalse );
	cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav", qfalse );
	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav", qfalse );

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav", qfalse);

	cgs.media.hitSound = trap_S_RegisterSound( "sound/feedback/hit.wav", qfalse );
#ifdef MISSIONPACK
	cgs.media.hitSoundHighArmor = trap_S_RegisterSound( "sound/feedback/hithi.wav", qfalse );
	cgs.media.hitSoundLowArmor = trap_S_RegisterSound( "sound/feedback/hitlo.wav", qfalse );
#endif

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav", qtrue );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav", qtrue );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav", qtrue );
	cgs.media.humiliationSound = trap_S_RegisterSound( "sound/feedback/humiliation.wav", qtrue );
	cgs.media.assistSound = trap_S_RegisterSound( "sound/feedback/assist.wav", qtrue );
	cgs.media.defendSound = trap_S_RegisterSound( "sound/feedback/defense.wav", qtrue );
#ifdef MISSIONPACK
	cgs.media.firstImpressiveSound = trap_S_RegisterSound( "sound/feedback/first_impressive.wav", qtrue );
	cgs.media.firstExcellentSound = trap_S_RegisterSound( "sound/feedback/first_excellent.wav", qtrue );
	cgs.media.firstHumiliationSound = trap_S_RegisterSound( "sound/feedback/first_gauntlet.wav", qtrue );
#endif

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav", qtrue);
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav", qtrue);
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav", qtrue);

#ifdef MISSIONPACK
	cgs.media.voteNow = trap_S_RegisterSound( "sound/feedback/vote_now.wav", qtrue);
	cgs.media.votePassed = trap_S_RegisterSound( "sound/feedback/vote_passed.wav", qtrue);
	cgs.media.voteFailed = trap_S_RegisterSound( "sound/feedback/vote_failed.wav", qtrue);
#endif

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

	cgs.media.jumpPadSound = trap_S_RegisterSound ("sound/world/jumppad.wav", qfalse );

	//PKMOD - Ergodic 08/08/00 PAINKILLER awarded after every 10 PKitem kills
	//PKMOD - Ergodic 09/03/00 updated Painkiller wav file with sounds from Mongusta
	cgs.media.painkillerSound = trap_S_RegisterSound( "sound/feedback/PainkillerAward2.wav", qfalse );


	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/boot%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mech%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/energy%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name, qfalse);
	}

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	//PKMOD - Ergodic 07/30/01 - debug "bad magic" crash (inactive)
//	Com_Printf( "CG_RegisterSounds - items>%s<\n\n", items );
//	Com_Printf( "CG_RegisterSounds - bg_numItems>%d<\n", bg_numItems );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
//		}
	}

	//PKMOD - Ergodic 07/30/01 - debug "bad magic" crash (inactive)
//	if ( dbg1 != dbg2 )
//		return;

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );

		//PKMOD - Ergodic 11/19/02 - debug mover sound (inactive)
		//Com_Printf( "CG_RegisterSounds - soundindex: %d - name >%s<\n", i, &soundName[0] ); 

	}

	// FIXME: only needed with item
	cgs.media.flightSound = trap_S_RegisterSound( "sound/items/flight.wav", qfalse );
	cgs.media.medkitSound = trap_S_RegisterSound ("sound/items/use_medkit.wav", qfalse);
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav", qfalse);
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
	cgs.media.sfx_railg = trap_S_RegisterSound ("sound/weapons/railgun/railgf1a.wav", qfalse);
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav", qfalse);
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav", qfalse);
#ifdef MISSIONPACK
	cgs.media.sfx_proxexp = trap_S_RegisterSound( "sound/weapons/proxmine/wstbexpl.wav" , qfalse);
	cgs.media.sfx_nghit = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpd.wav" , qfalse);
	cgs.media.sfx_nghitflesh = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpl.wav" , qfalse);
	cgs.media.sfx_nghitmetal = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpm.wav", qfalse );
	cgs.media.sfx_chghit = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpd.wav", qfalse );
	cgs.media.sfx_chghitflesh = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpl.wav", qfalse );
	cgs.media.sfx_chghitmetal = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpm.wav", qfalse );
	cgs.media.weaponHoverSound = trap_S_RegisterSound( "sound/weapons/weapon_hover.wav", qfalse );
	cgs.media.kamikazeExplodeSound = trap_S_RegisterSound( "sound/items/kam_explode.wav", qfalse );
	cgs.media.kamikazeImplodeSound = trap_S_RegisterSound( "sound/items/kam_implode.wav", qfalse );
	cgs.media.kamikazeFarSound = trap_S_RegisterSound( "sound/items/kam_explode_far.wav", qfalse );
	cgs.media.winnerSound = trap_S_RegisterSound( "sound/feedback/voc_youwin.wav", qfalse );
	cgs.media.loserSound = trap_S_RegisterSound( "sound/feedback/voc_youlose.wav", qfalse );
	cgs.media.youSuckSound = trap_S_RegisterSound( "sound/misc/yousuck.wav", qfalse );

	cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.wav", qfalse);
	cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.wav", qfalse);
	cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
	cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);
#endif

	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav", qfalse);
	cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.wav", qfalse);
	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse );
	cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav", qfalse);
	cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav", qfalse);

#ifdef MISSIONPACK
	trap_S_RegisterSound("sound/player/james/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/james/taunt.wav", qfalse );

	trap_S_RegisterSound("sound/player/janet/death1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/death2.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/death3.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/jump1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain25_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain75_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/pain100_1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/falling1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/gasp.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/drown.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/fall1.wav", qfalse );
	trap_S_RegisterSound("sound/player/janet/taunt.wav", qfalse );
#endif
	//PKMOD - Ergodic 12/19/00 - remove Team Arena proxmine sounds
//	cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.wav", qfalse);
//	cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.wav", qfalse);
//	cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
//	cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);

	//PKMOD - Ergodic 05/22/00 add pka sounds
	//12/16/00 - add non compressed flag
	cgs.media.sfx_pkagravitylaunched = trap_S_RegisterSound ("sound/weapons2/gwell/gravity_released.wav", qfalse);
	cgs.media.sfx_pkabeartrapbreakup = trap_S_RegisterSound ("sound/weapons2/beartrap/beartrap_breakup.wav", qfalse);
	cgs.media.sfx_pkabeartrapdrop = trap_S_RegisterSound ("sound/weapons2/beartrap/beartrap_drop.wav", qfalse);
	cgs.media.sfx_pkabeartrapsnap = trap_S_RegisterSound ("sound/weapons2/beartrap/beartrap_snap.wav", qfalse);

	//PKMOD - Ergodic 03/23/01 - add team parameters
	cgs.media.pkabeartrap_red = trap_R_RegisterModel( "models/weapons2/beartrap/beartrap_red.md3" );
	cgs.media.pkabeartrap_blue = trap_R_RegisterModel( "models/weapons2/beartrap/beartrap_blue.md3" );

	//PKMOD - Ergodic 08/22/00 ChainLightning strike sounds
	//12/16/00 - add non compressed flag
	cgs.media.sfx_chainlightningstrike1 = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight spark.wav", qfalse );
	cgs.media.sfx_chainlightningstrike2 = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight spark#2.wav", qfalse );

	//PKMOD - Ergodic 08/25/00 nailgun ricochet sounds
	//12/16/00 - add non compressed flag
	cgs.media.sfx_nailrico1 = trap_S_RegisterSound ("sound/weapons2/nailgun/nailrico1.wav", qfalse);
	cgs.media.sfx_nailrico2 = trap_S_RegisterSound ("sound/weapons2/nailgun/nailrico2.wav", qfalse);
	cgs.media.sfx_nailrico3 = trap_S_RegisterSound ("sound/weapons2/nailgun/nailrico3.wav", qfalse);
	cgs.media.sfx_nailrico4 = trap_S_RegisterSound ("sound/weapons2/nailgun/nailrico4.wav", qfalse);

	//PKMOD - Ergodic 09/06/00 gravity well item suck sounds from Mongusta
	//12/16/00 - add non compressed flag
	cgs.media.sfx_pkagravitywell_suck1 = trap_S_RegisterSound ("sound/weapons2/gwell/Grav Well Suk#1.wav", qfalse);
	cgs.media.sfx_pkagravitywell_suck2 = trap_S_RegisterSound ("sound/weapons2/gwell/Grav Well Suk#2.wav", qfalse);
	cgs.media.sfx_pkagravitywell_suck3 = trap_S_RegisterSound ("sound/weapons2/gwell/Grav Well Suk#3.wav", qfalse);

	//PKMOD - Ergodic 11/22/00 autosentry sounds from Mongusta
	//12/16/00 - add non compressed flag
	cgs.media.sfx_pkasentrydrop = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentrydeploy.wav", qfalse);

	//PKMOD - Ergodic 12/26/00 add beans fart sounds
	cgs.media.sfx_pkafart1 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Blast.wav", qfalse);
	cgs.media.sfx_pkafart2 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Short.wav", qfalse);
	cgs.media.sfx_pkafart3 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Squish.wav", qfalse);
	cgs.media.sfx_pkafart4 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Whistle.wav", qfalse);
	cgs.media.sfx_pkafart5 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Whooppee.wav", qfalse);
	//PKMOD - Ergodic 06/30/01 add two more fart sounds from original Q1 PK
	cgs.media.sfx_pkafart6 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Q1PK_4.wav", qfalse);
	cgs.media.sfx_pkafart7 = trap_S_RegisterSound ("sound/weapons2/beans/Fart Q1PK_5.wav", qfalse);

	//PKMOD - Ergodic 01/13/01 - add autosentry fire sounds from mongusta
	cgs.media.sfx_pkasentry1 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryshot1.wav", qfalse);
	cgs.media.sfx_pkasentry2 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryshot2.wav", qfalse);
	cgs.media.sfx_pkasentry3 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryshot3.wav", qfalse);

	//PKMOD - Ergodic 03/26/01 - autosentry sonar ping sounds
	cgs.media.sfx_pkasentry_ping1 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryping1.wav", qfalse);
	cgs.media.sfx_pkasentry_ping2 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryping2.wav", qfalse);
	cgs.media.sfx_pkasentry_ping3 = trap_S_RegisterSound ("sound/weapons2/autosentry/autosentryping3.wav", qfalse);

	//PKMOD - Ergodic 06/20/01 - add legs model for swinging zombie
	cgs.media.pkazombie_legsModel = trap_R_RegisterModel( "models/players/biker/lower.md3" );
	cgs.media.pkazombie_legsSkin = trap_R_RegisterSkin( "models/mapobjects/zombie/lower_zombie.skin" );

	//PKMOD - Ergodic 06/21/01 - add torso model for swinging zombie
	cgs.media.pkazombie_torsoModel = trap_R_RegisterModel( "models/players/biker/upper.md3" );
	cgs.media.pkazombie_torsoSkin = trap_R_RegisterSkin( "models/mapobjects/zombie/upper_zombie.skin" );

	//PKMOD - Ergodic 06/21/01 - add head model for swinging zombie
	cgs.media.pkazombie_headModel = trap_R_RegisterModel( "models/players/biker/head.md3" );
	cgs.media.pkazombie_headSkin = trap_R_RegisterSkin( "models/mapobjects/zombie/head_zombie.skin" );

	//PKMOD - Ergodic 06/30/01 add airfist sounds for all types of situations
	cgs.media.sfx_pkaairfistfire = trap_S_RegisterSound ("sound/weapons2/airfist/af_fire.wav", qfalse);
	cgs.media.sfx_pkaairfistwaterfire = trap_S_RegisterSound ("sound/weapons2/airfist/af_wfire.wav", qfalse);
	cgs.media.sfx_pkaairfistempty = trap_S_RegisterSound ("sound/weapons2/airfist/af_empty.wav", qfalse);
	cgs.media.sfx_pkaairfistwaterempty = trap_S_RegisterSound ("sound/weapons2/airfist/af_wempty.wav", qfalse);

	//PKMOD - Ergodic 07/03/01 ChainLightning reflect sounds
	cgs.media.sfx_chainlightningreflect1 = trap_S_RegisterSound( "sound/weapons2/chainlightning/Electricute2.wav", qfalse );
	cgs.media.sfx_chainlightningreflect2 = trap_S_RegisterSound( "sound/weapons2/chainlightning/Electricute3.wav", qfalse );

	//PKMOD - Ergodic 12/03/01 - New Holdables - Private Bot pickup skins
	cgs.media.privatebot_legsSkin = trap_R_RegisterSkin( "models/players/tankjr/lower_default.skin" );
	cgs.media.privatebot_torsoSkin = trap_R_RegisterSkin( "models/players/doom/upper_phobos.skin" );
	cgs.media.privatebot_headSkin = trap_R_RegisterSkin( "models/players/visor/head_painkiller.skin" );

	//PKMOD - Ergodic 12/05/01 - Holdable: radiate sounds
	cgs.media.pkaradiatewarningSound = trap_S_RegisterSound( "sound/items/radiatewarning.wav", qfalse );
	cgs.media.pkaradiateitemSound = trap_S_RegisterSound( "sound/items/radiateitem.wav", qfalse );
	cgs.media.pkaradiateplayerSound = trap_S_RegisterSound( "sound/items/radiateplayer.wav", qfalse );
	//PKMOD - Ergodic 08/02/02 - Holdable: radiate activation sound
	cgs.media.pkaradiateactivationSound = trap_S_RegisterSound( "sound/items/radiateroar.wav", qfalse );

	//PKMOD - Ergodic 12/07/01 - Holdable: Private Bot HUD Icons
	cgs.media.pkapribot_001Icon = trap_R_RegisterShader( "icons/iconh_pribot_001" );
	cgs.media.pkapribot_010Icon = trap_R_RegisterShader( "icons/iconh_pribot_010" );
	cgs.media.pkapribot_011Icon = trap_R_RegisterShader( "icons/iconh_pribot_011" );
	cgs.media.pkapribot_100Icon = trap_R_RegisterShader( "icons/iconh_pribot_100" );
	cgs.media.pkapribot_101Icon = trap_R_RegisterShader( "icons/iconh_pribot_101" );
	cgs.media.pkapribot_110Icon = trap_R_RegisterShader( "icons/iconh_pribot_110" );
	cgs.media.pkapribot_111Icon = trap_R_RegisterShader( "icons/iconh_pribot_111" );

	//PKMOD - Ergodic 12/16/01 - add new model for repositioned deployed gauntlet blade
//	cgs.media.pkagauntlet_bladeModel = trap_R_RegisterModel( "models/weapons2/gauntlet/gauntlet_blade.md3" );

	//PKMOD - Ergodic 02/06/02 - add attack sounds for beartrap and autosentry
	cgs.media.beartrap_attackSound = trap_S_RegisterSound( "sound/feedback/attack_Trap.wav", qfalse );
	cgs.media.autosentry_attackSound = trap_S_RegisterSound( "sound/feedback/attack_Sentry.wav", qfalse );
	cgs.media.radiate_attackSound = trap_S_RegisterSound( "sound/feedback/attack_Radiate.wav", qfalse );

	//PKMOD - Ergodic 02/07/02 - add Private Bot completed sound
	cgs.media.pkapribot_complete = trap_S_RegisterSound( "sound/items/unit_complete.wav", qfalse );

	//PKMOD - Ergodic 02/10/02 - send FRAG message to Private Bot's owner
	cgs.media.pkapribot_frag1 = trap_S_RegisterSound( "sound/feedback/PB_frag_obtained.wav", qfalse );
	cgs.media.pkapribot_frag2 = trap_S_RegisterSound( "sound/feedback/PB_opposition_eliminated.wav", qfalse );

	//PKMOD - Ergodic 02/14/02 - explosive shells hit sounds
	cgs.media.sfx_expgunhit1 = trap_S_RegisterSound ("sound/weapons2/expgun/ESG_Hits1.wav", qfalse);
	cgs.media.sfx_expgunhit2 = trap_S_RegisterSound ("sound/weapons2/expgun/ESG_Hits2.wav", qfalse);
	cgs.media.sfx_expgunhit3 = trap_S_RegisterSound ("sound/weapons2/expgun/ESG_Hits3.wav", qfalse);
	//PKMOD - Ergodic 07/10/02 - add 2 more explosive shells hit sounds
	cgs.media.sfx_expgunhit4 = trap_S_RegisterSound ("sound/weapons2/expgun/ESG_Hits4.wav", qfalse);
	cgs.media.sfx_expgunhit5 = trap_S_RegisterSound ("sound/weapons2/expgun/ESG_Hits5.wav", qfalse);

	//PKMOD - Ergodic 08/02/02 - Holdable: Personal Sentry hover sound
	cgs.media.pkapersentryhoverSound = trap_S_RegisterSound( "sound/items/persentryhover.wav", qfalse );

	//PKMOD - Ergodic 08/26/02 - add Personal Sentry fire sounds from StarDagger
	cgs.media.pkapersentry_fire1 = trap_S_RegisterSound ("sound/weapons2/persentry/PersSentryshot1.wav", qfalse);
	cgs.media.pkapersentry_fire2 = trap_S_RegisterSound ("sound/weapons2/persentry/PersSentryshot2.wav", qfalse);
	cgs.media.pkapersentry_fire3 = trap_S_RegisterSound ("sound/weapons2/persentry/PersSentryshot3.wav", qfalse);

	//PKMOD - Ergodic 08/20/03 - add special shader for shooter lightning
	cgs.media.shooterlightningShader = trap_R_RegisterShader( "shooterlightningBolt" );

	//PKMOD - Ergodic 11/21/03 - Earthquake sound for Gravity Well effect on out of reach players
	//PKMOD - Ergodic 12/06/03 - update Earthquake sound for Gravity Well effect on out of reach players
	//PKMOD - Ergodic 12/07/03 - removed, code moved to global sound
	//cgs.media.pkaearthquake = trap_S_RegisterSound( "sound/weapons2/gwell/earthquake3.wav", qfalse );

	//PKMOD - Ergodic 12/08/03 - Add chargeup sound for BearTrap, Autosentry invisibility
	cgs.media.pkachargeup = trap_S_RegisterSound( "sound/weapons/lightning/lg_charge.wav", qfalse );

	//PKMOD - Ergodic 03/17/04 - add quad beans fart sounds
	cgs.media.sfx_pkaquadfart1 = trap_S_RegisterSound ("sound/weapons2/beans/QFart_Boom1.wav", qfalse);
	cgs.media.sfx_pkaquadfart2 = trap_S_RegisterSound ("sound/weapons2/beans/QFart_Duck1.wav", qfalse);
	cgs.media.sfx_pkaquadfart3 = trap_S_RegisterSound ("sound/weapons2/beans/QFart_Frog1.wav", qfalse);


}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	cgs.media.botSkillShaders[0] = trap_R_RegisterShader( "menu/art/skill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader( "menu/art/skill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader( "menu/art/skill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader( "menu/art/skill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader( "menu/art/skill5.tga" );

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "shotgunSmokePuff" );
#ifdef MISSIONPACK
	cgs.media.nailPuffShader = trap_R_RegisterShader( "nailtrail" );
	cgs.media.blueProxMine = trap_R_RegisterModel( "models/weaphits/proxmineb.md3" );
#endif
	cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1" );
	cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c", 'a'+i) );
	}

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShader( "icons/noammo" );

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad" );
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon" );
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit" );
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon" );
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility" );
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen" );
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff" );

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
#else
	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
#endif
		cgs.media.redCubeModel = trap_R_RegisterModel( "models/powerups/orb/r_orb.md3" );
		cgs.media.blueCubeModel = trap_R_RegisterModel( "models/powerups/orb/b_orb.md3" );
		cgs.media.redCubeIcon = trap_R_RegisterShader( "icons/skull_red" );
		cgs.media.blueCubeIcon = trap_R_RegisterShader( "icons/skull_blue" );
	}

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
#else
	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
#endif
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu3" );
#ifdef MISSIONPACK
		cgs.media.flagPoleModel = trap_R_RegisterModel( "models/flag2/flagpole.md3" );
		cgs.media.flagFlapModel = trap_R_RegisterModel( "models/flag2/flagflap3.md3" );

		cgs.media.redFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/red.skin" );
		cgs.media.blueFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/blue.skin" );
		cgs.media.neutralFlagFlapSkin = trap_R_RegisterSkin( "models/flag2/white.skin" );

		cgs.media.redFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/red_base.md3" );
		cgs.media.blueFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/blue_base.md3" );
		cgs.media.neutralFlagBaseModel = trap_R_RegisterModel( "models/mapobjects/flagbase/ntrl_base.md3" );
#endif
	}

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
		cgs.media.neutralFlagModel = trap_R_RegisterModel( "models/flags/n_flag.md3" );
		cgs.media.flagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral1" );
		cgs.media.flagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.flagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.flagShader[3] = trap_R_RegisterShaderNoMip( "icons/iconf_neutral3" );
	}

	if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
		cgs.media.overloadBaseModel = trap_R_RegisterModel( "models/powerups/overload_base.md3" );
		cgs.media.overloadTargetModel = trap_R_RegisterModel( "models/powerups/overload_target.md3" );
		cgs.media.overloadLightsModel = trap_R_RegisterModel( "models/powerups/overload_lights.md3" );
		cgs.media.overloadEnergyModel = trap_R_RegisterModel( "models/powerups/overload_energy.md3" );
	}

	if ( cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.harvesterModel = trap_R_RegisterModel( "models/powerups/harvester/harvester.md3" );
		cgs.media.harvesterRedSkin = trap_R_RegisterSkin( "models/powerups/harvester/red.skin" );
		cgs.media.harvesterBlueSkin = trap_R_RegisterSkin( "models/powerups/harvester/blue.skin" );
		cgs.media.harvesterNeutralModel = trap_R_RegisterModel( "models/powerups/obelisk/obelisk.md3" );
	}

	cgs.media.redKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikred" );
	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );
#endif

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
		cgs.media.redQuadShader = trap_R_RegisterShader("powerups/blueflag" );
		cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
#ifdef MISSIONPACK
		cgs.media.blueKamikazeShader = trap_R_RegisterShader( "models/weaphits/kamikblu" );
#endif
	}

	cgs.media.armorModel = trap_R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = trap_R_RegisterShaderNoMip( "icons/iconr_yellow" );

	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.shotgunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = trap_R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = trap_R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = trap_R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = trap_R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = trap_R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = trap_R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = trap_R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = trap_R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = trap_R_RegisterModel( "models/gibs/brain.md3" );

	cgs.media.smoke2 = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.balloonShader = trap_R_RegisterShader( "sprites/balloon3" );

	cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
#ifdef MISSIONPACK
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/powerups/pop.md3" );
#else
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );
#endif
#ifdef MISSIONPACK
	cgs.media.kamikazeEffectModel = trap_R_RegisterModel( "models/weaphits/kamboom2.md3" );
	cgs.media.kamikazeShockWave = trap_R_RegisterModel( "models/weaphits/kamwave.md3" );
	cgs.media.kamikazeHeadModel = trap_R_RegisterModel( "models/powerups/kamikazi.md3" );
	cgs.media.kamikazeHeadTrail = trap_R_RegisterModel( "models/powerups/trailtest.md3" );
	cgs.media.guardPowerupModel = trap_R_RegisterModel( "models/powerups/guard_player.md3" );
	cgs.media.scoutPowerupModel = trap_R_RegisterModel( "models/powerups/scout_player.md3" );
	cgs.media.doublerPowerupModel = trap_R_RegisterModel( "models/powerups/doubler_player.md3" );
	cgs.media.ammoRegenPowerupModel = trap_R_RegisterModel( "models/powerups/ammo_player.md3" );
	cgs.media.invulnerabilityImpactModel = trap_R_RegisterModel( "models/powerups/shield/impact.md3" );
	cgs.media.invulnerabilityJuicedModel = trap_R_RegisterModel( "models/powerups/shield/juicer.md3" );
	cgs.media.medkitUsageModel = trap_R_RegisterModel( "models/powerups/regen.md3" );
	cgs.media.heartShader = trap_R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );

#endif

	cgs.media.invulnerabilityPowerupModel = trap_R_RegisterModel( "models/powerups/shield/shield.md3" );
	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip( "medal_gauntlet" );
	cgs.media.medalDefend = trap_R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist = trap_R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalCapture = trap_R_RegisterShaderNoMip( "medal_capture" );

	//PKMOD - Ergodic 08/08/00 PAINKILLER awarded after every 10 PKitem kills
	cgs.media.medalPainKiller  = trap_R_RegisterShaderNoMip( "medal_painkiller" );

	//PKMOD - Ergodic 08/22/00 ChainLightning Gun player hit shader effect
	cgs.media.clgplayerhitShader = trap_R_RegisterShader("models/weaphits/clgplayerhit" );

	//PKMOD - Ergodic 11/12/00 airfist flash model shader
	cgs.media.airfistFlashShader = trap_R_RegisterShader("models/weapons2/airfist/airfist_flash" );

	//PKMOD - Ergodic 11/16/00 update airfist flash model to correspond to airfist_level
	cgs.media.airfist4FlashModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist4_flash.md3" );
	cgs.media.airfist3FlashModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist3_flash.md3" );
	cgs.media.airfist2FlashModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist2_flash.md3" );
	cgs.media.airfist1FlashModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist1_flash.md3" );
	cgs.media.airfist0FlashModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist0_flash.md3" );

	//PKMOD - Ergodic 12/27/00 add beans shader
	cgs.media.pkafartPuffShader = trap_R_RegisterShader( "pkafartPuff" );

	//PKMOD - Ergodic 12/28/00 add flash model for autosentry (same as machinegun)
	cgs.media.autosentryFlashModel  = trap_R_RegisterModel( "models/weapons2/machinegun/machinegun_flash.md3" );

	//PKMOD - Ergodic 01/16/01 - add model for exploding shells weaponhit
	cgs.media.explshellsFlashModel = trap_R_RegisterModel("models/weaphits/explboom/explboom.md3");

	//PKMOD - Ergodic 01/16/01 - add multi-shaders for exploding shells
	cgs.media.shellsExplosionShader1 = trap_R_RegisterShader("shellsexplosion1" );
	cgs.media.shellsExplosionShader2 = trap_R_RegisterShader("shellsexplosion2" );
	cgs.media.shellsExplosionShader3 = trap_R_RegisterShader("shellsexplosion3" );
	cgs.media.shellsExplosionShader4 = trap_R_RegisterShader("shellsexplosion4" );
	cgs.media.shellsExplosionShader5 = trap_R_RegisterShader("shellsexplosion5" );
	cgs.media.shellsExplosionShader6 = trap_R_RegisterShader("shellsexplosion6" );

	//PKMOD - Ergodic 01/21/01 - add coordinate model for exploding shells debug
	cgs.media.coordFlashModel = trap_R_RegisterModel("models/weapons2/explgun/coord.md3");

	//PKMOD - Ergodic 04/06/01 - add autosentry missile sprite
	cgs.media.autosentryBallShader = trap_R_RegisterShader( "sprites/autosentry/missile1" );

	//PKMOD - Ergodic 10/14/01 - add shaders for radiation holdable spark effect
	cgs.media.radiate1Shader = trap_R_RegisterShader( "radiate1Spark" );
	cgs.media.radiate2Shader = trap_R_RegisterShader( "radiate2Spark" );
	cgs.media.radiate3Shader = trap_R_RegisterShader( "radiate3Spark" );
	cgs.media.radiate4Shader = trap_R_RegisterShader( "radiate4Spark" );
	cgs.media.radiate5Shader = trap_R_RegisterShader( "radiate5Spark" );
	cgs.media.radiate6Shader = trap_R_RegisterShader( "radiate6Spark" );

	//PKMOD - Ergodic 11/17/01 - add icons for simple items that are radiatedt
	cgs.media.radiate1SimpleIcon = trap_R_RegisterShader( "radiate1SimpleIcon" );
	cgs.media.radiate2SimpleIcon = trap_R_RegisterShader( "radiate2SimpleIcon" );
	cgs.media.radiate3SimpleIcon = trap_R_RegisterShader( "radiate3SimpleIcon" );

	//PKMOD - Ergodic 11/27/01 - add radition trail for infected players
	cgs.media.radiationTrailShader = trap_R_RegisterShader( "radiationTrail" );

	//PKMOD - Ergodic 05/07/02 - add active personal sentry model
	cgs.media.persentry_active = trap_R_RegisterModel("models/powerups/holdable/persentry_active.md3");

	//PKMOD - Ergodic 06/08/02 - add personal sentry teleport model
	cgs.media.persentry_teleportEffectModel = trap_R_RegisterModel( "models/powerups/holdable/persentry_tele.md3" );

	//PKMOD - Ergodic 06/12/02 - add personal sentry missile shader
	cgs.media.personalsentryBallShader = trap_R_RegisterShader( "sprites/personalsentry/missile1" );

	//PKMOD - Ergodic 09/11/02 - add private bot field effect
	cgs.media.privatebot_CueModel = trap_R_RegisterModel( "models/powerups/holdable/pbcue.md3" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	//PKMOD - Ergodic 05/11/01 - register holdables into their own array
	//		for optimization purposes
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_HOLDABLE ) {
			if ( bg_itemlist[i].giTag < HI_NUM_HOLDABLE ) {
				cg_holdable[ bg_itemlist[i].giTag ] = i;
			}
			else {
				Com_Error( ERR_DROP, "HOLDABLE array index has been exceeded: %d at offset %d\n", bg_itemlist[i].giTag, i);
			}
		}
	}

	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
	cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
	cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark" );

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}

	//PKMOD - Ergodic 05/21/2000, register PainKeepArena models
	cgs.media.pkabeartrapgib1 = trap_R_RegisterModel( "models/weapons2/beartrap/beargib1.md3" );
	cgs.media.pkabeartrapgib2 = trap_R_RegisterModel( "models/weapons2/beartrap/beargib2.md3" );
	cgs.media.pkabeartrapgib3 = trap_R_RegisterModel( "models/weapons2/beartrap/beargib3.md3" );
	cgs.media.pkabeartrapgib4 = trap_R_RegisterModel( "models/weapons2/beartrap/beargib4.md3" );
	cgs.media.pkabeartrap = trap_R_RegisterModel( "models/weapons2/beartrap/beartrap.md3" );
	//PKMOD - Ergodic 06/11/2000, register PainKeepArena models
	cgs.media.pkabeartrapfollow = trap_R_RegisterModel( "models/weapons2/beartrap/bearfollow.md3" );
	//PKMOD - Ergodic 08/10/2000, register PainKeepArena models
	//PKMOD - Ergodic 03/18/2001, updated
	cgs.media.pkagravitywelluniverse  = trap_R_RegisterModel( "models/weapons2/gwell/gw_sphere.md3" );

	//PKMOD - Ergodic 08/01/00 set nail model
	cgs.media.nailFlashModel = trap_R_RegisterModel( "models/weapons2/nailgun/nail.md3" );
	cgs.media.nailMarkShader = trap_R_RegisterShader( "nail_mrk" );
	//PKMOD - Ergodic 08/03/00 more nail info - static nails
	cgs.media.nail1 = trap_R_RegisterModel( "models/weapons2/nailgun/nail.md3" );

	//PKMOD - Ergodic 11/22/2000, register PainKeepArena autosentry models
	//					will use deploy0 as the pickup model 
//	cgs.media.pkasentry_pickup = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_pickup.md3" );
	//PKMOD - Ergodic 11/25/00 add autosentry deploy models
	//PKMOD - Ergodic 05/31/02 - use autosentry animation model
	/*
	cgs.media.pkasentry_deploy0 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy0.md3" );
	cgs.media.pkasentry_deploy1 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy1.md3" );
	cgs.media.pkasentry_deploy2 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy2.md3" );
	cgs.media.pkasentry_deploy3 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy3.md3" );
	cgs.media.pkasentry_deploy4 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy4.md3" );
	cgs.media.pkasentry_deploy5 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy5.md3" );
	cgs.media.pkasentry_deploy6 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy6.md3" );
	cgs.media.pkasentry_deploy7 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy7.md3" );
	cgs.media.pkasentry_deploy8 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy8.md3" );
	cgs.media.pkasentry_deploy9 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy9.md3" );
	cgs.media.pkasentry_deploy10 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy10.md3" );
	*/
	//PKMOD - Ergodic 03/20/01 - add team parameters
	//PKMOD - Ergodic 05/31/02 - use autosentry animation model
	cgs.media.pkasentry_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_red.md3" );
	cgs.media.pkasentry_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_blue.md3" );
	//PKMOD - Ergodic 06/01/02 - add default autosentry model for dragon deploy
	cgs.media.pkasentry = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry.md3" );
	/*
	cgs.media.pkasentry_deploy0_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy0_red.md3" );
	cgs.media.pkasentry_deploy1_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy1_red.md3" );
	cgs.media.pkasentry_deploy2_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy2_red.md3" );
	cgs.media.pkasentry_deploy3_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy3_red.md3" );
	cgs.media.pkasentry_deploy4_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy4_red.md3" );
	cgs.media.pkasentry_deploy5_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy5_red.md3" );
	cgs.media.pkasentry_deploy6_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy6_red.md3" );
	cgs.media.pkasentry_deploy7_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy7_red.md3" );
	cgs.media.pkasentry_deploy8_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy8_red.md3" );
	cgs.media.pkasentry_deploy9_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy9_red.md3" );
	cgs.media.pkasentry_deploy10_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy10_red.md3" );
	cgs.media.pkasentry_deploy0_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy0_blue.md3" );
	cgs.media.pkasentry_deploy1_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy1_blue.md3" );
	cgs.media.pkasentry_deploy2_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy2_blue.md3" );
	cgs.media.pkasentry_deploy3_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy3_blue.md3" );
	cgs.media.pkasentry_deploy4_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy4_blue.md3" );
	cgs.media.pkasentry_deploy5_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy5_blue.md3" );
	cgs.media.pkasentry_deploy6_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy6_blue.md3" );
	cgs.media.pkasentry_deploy7_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy7_blue.md3" );
	cgs.media.pkasentry_deploy8_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy8_blue.md3" );
	cgs.media.pkasentry_deploy9_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy9_blue.md3" );
	cgs.media.pkasentry_deploy10_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_deploy10_blue.md3" );
	*/

	//PKMOD - Ergodic 12/02/00 add split autosentry models
	//PKMOD - Ergodic 03/20/01 - add team parameters
	cgs.media.pkasentry_base = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_base.md3" );
	cgs.media.pkasentry_base_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_base_red.md3" );
	cgs.media.pkasentry_base_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_base_blue.md3" );

	//PKMOD - Ergodic 03/20/01 - add team parameters
	cgs.media.pkasentry_turret = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_turret.md3" );
	cgs.media.pkasentry_turret_red = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_turret_red.md3" );
	cgs.media.pkasentry_turret_blue = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_turret_blue.md3" );

	//PKMOD - Ergodic 12/14/00 - add autosentry gib models
	cgs.media.pkasentry_gib1 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib1.md3" );
	cgs.media.pkasentry_gib2 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib2.md3" );
	cgs.media.pkasentry_gib3 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib3.md3" );
	cgs.media.pkasentry_gib4 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib4.md3" );
	cgs.media.pkasentry_gib5 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib5.md3" );
	cgs.media.pkasentry_gib6 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib6.md3" );
	cgs.media.pkasentry_gib7 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib7.md3" );
	cgs.media.pkasentry_gib8 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib8.md3" );
	cgs.media.pkasentry_gib9 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib9.md3" );
	cgs.media.pkasentry_gib10 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib10.md3" );
	cgs.media.pkasentry_gib11 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib11.md3" );
	cgs.media.pkasentry_gib12 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib12.md3" );
	cgs.media.pkasentry_gib13 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib13.md3" );
	cgs.media.pkasentry_gib14 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib14.md3" );
	cgs.media.pkasentry_gib15 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib15.md3" );
	cgs.media.pkasentry_gib16 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib16.md3" );
	cgs.media.pkasentry_gib17 = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_gib17.md3" );

	//PKMOD - Ergodic 10/14/02 - Add the gravity well expanding wave
	cgs.media.pkagravitywellwave = trap_R_RegisterModel( "models/weapons2/gwell/gw_wave.md3" );
	//PKMOD - Ergodic 10/18/02 - Add the gravity well spark
	cgs.media.pkagravitywellspark = trap_R_RegisterShader( "gravitywell_spark" );

	//PKMOD - Ergodic 07/18/03 - add invisible Beartrap Shaders
	cgs.media.pkainvisbeartrap1 = trap_R_RegisterShader( "beartrap1invis" );
	cgs.media.pkainvisbeartrap2 = trap_R_RegisterShader( "beartrap2invis" );
	cgs.media.pkainvisbeartrap3 = trap_R_RegisterShader( "beartrap3invis" );
	cgs.media.pkainvisbeartrap4 = trap_R_RegisterShader( "beartrap4invis" );
	cgs.media.pkainvisbeartrap5 = trap_R_RegisterShader( "beartrap5invis" );
	cgs.media.pkainvisbeartrap6 = trap_R_RegisterShader( "beartrap6invis" );
	cgs.media.pkainvisbeartrap7 = trap_R_RegisterShader( "beartrap7invis" );
	cgs.media.pkainvisbeartrap8 = trap_R_RegisterShader( "beartrap8invis" );
	cgs.media.pkainvisbeartrap9 = trap_R_RegisterShader( "beartrap9invis" );
	cgs.media.pkainvisbeartrap10 = trap_R_RegisterShader( "beartrap10invis" );
	cgs.media.pkainvisbeartrap11 = trap_R_RegisterShader( "beartrap11invis" );
	cgs.media.pkainvisbeartrap12 = trap_R_RegisterShader( "beartrap12invis" );
	cgs.media.pkainvisbeartrap13 = trap_R_RegisterShader( "beartrap13invis" );
	cgs.media.pkainvisbeartrap14 = trap_R_RegisterShader( "beartrap14invis" );
	cgs.media.pkainvisbeartrap15 = trap_R_RegisterShader( "beartrap15invis" );
	cgs.media.pkainvisbeartrap16 = trap_R_RegisterShader( "beartrap16invis" );
	cgs.media.pkainvisbeartrap17 = trap_R_RegisterShader( "beartrap17invis" );
	cgs.media.pkainvisbeartrap18 = trap_R_RegisterShader( "beartrap18invis" );
	cgs.media.pkainvisbeartrap19 = trap_R_RegisterShader( "beartrap19invis" );
	cgs.media.pkainvisbeartrap20 = trap_R_RegisterShader( "beartrap20invis" );

	//PKMOD - Ergodic 09/18/03 - Beartrap Invisibility Spark sprite shaders
	cgs.media.pkabeartrapspark1Shader = trap_R_RegisterShader( "beartrap1Spark" );
	cgs.media.pkabeartrapspark2Shader = trap_R_RegisterShader( "beartrap2Spark" );
	cgs.media.pkabeartrapspark3Shader = trap_R_RegisterShader( "beartrap3Spark" );
	cgs.media.pkabeartrapspark4Shader = trap_R_RegisterShader( "beartrap4Spark" );
	cgs.media.pkabeartrapspark5Shader = trap_R_RegisterShader( "beartrap5Spark" );

	//PKMOD - Ergodic 12/13/03 - add invisible Autosentry Shaders
	cgs.media.pkainvisautosentry1 = trap_R_RegisterShader( "autosentry1invis" );
	cgs.media.pkainvisautosentry2 = trap_R_RegisterShader( "autosentry2invis" );
	cgs.media.pkainvisautosentry3 = trap_R_RegisterShader( "autosentry3invis" );
	cgs.media.pkainvisautosentry4 = trap_R_RegisterShader( "autosentry4invis" );
	cgs.media.pkainvisautosentry5 = trap_R_RegisterShader( "autosentry5invis" );
	cgs.media.pkainvisautosentry6 = trap_R_RegisterShader( "autosentry6invis" );
	cgs.media.pkainvisautosentry7 = trap_R_RegisterShader( "autosentry7invis" );
	cgs.media.pkainvisautosentry8 = trap_R_RegisterShader( "autosentry8invis" );
	cgs.media.pkainvisautosentry9 = trap_R_RegisterShader( "autosentry9invis" );
	cgs.media.pkainvisautosentry10 = trap_R_RegisterShader( "autosentry10invis" );
	cgs.media.pkainvisautosentry11 = trap_R_RegisterShader( "autosentry11invis" );
	cgs.media.pkainvisautosentry12 = trap_R_RegisterShader( "autosentry12invis" );
	cgs.media.pkainvisautosentry13 = trap_R_RegisterShader( "autosentry13invis" );
	cgs.media.pkainvisautosentry14 = trap_R_RegisterShader( "autosentry14invis" );
	cgs.media.pkainvisautosentry15 = trap_R_RegisterShader( "autosentry15invis" );
	cgs.media.pkainvisautosentry16 = trap_R_RegisterShader( "autosentry16invis" );
	cgs.media.pkainvisautosentry17 = trap_R_RegisterShader( "autosentry17invis" );
	cgs.media.pkainvisautosentry18 = trap_R_RegisterShader( "autosentry18invis" );
	cgs.media.pkainvisautosentry19 = trap_R_RegisterShader( "autosentry19invis" );
	cgs.media.pkainvisautosentry20 = trap_R_RegisterShader( "beartrap20invis" );

	//PKMOD - Ergodic 01/07/04 - add quad farting logic for differing CG graphic sequence
	cgs.media.pkaquadbeansShader = trap_R_RegisterShader( "QuadBeansExplosion" );
	cgs.media.pkaquadbeansModel = trap_R_RegisterModel("models/weaphits/quadbeans/quadbeans.md3");


#ifdef MISSIONPACK
	// new stuff
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
	cgs.media.cursor = trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = trap_R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = trap_R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	trap_R_RegisterModel( "models/players/james/lower.md3" );
	trap_R_RegisterModel( "models/players/james/upper.md3" );
	trap_R_RegisterModel( "models/players/heads/james/james.md3" );

	trap_R_RegisterModel( "models/players/janet/lower.md3" );
	trap_R_RegisterModel( "models/players/janet/upper.md3" );
	trap_R_RegisterModel( "models/players/heads/janet/janet.md3" );

#endif

	//PKMOD - Ergodic 01/28/04 - Dynamic HUD activation: move flagShaders & heartShader to active code
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");
	cgs.media.heartShader = trap_R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );

	//PKMOD - Ergodic 01/30/04 - Dynamic HUD activation: move the following Shaders to active code
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");

	CG_ClearParticles ();
/*
	for (i=1; i<MAX_PARTICLES_AREAS; i++)
	{
		{
			int rval;

			rval = CG_NewParticleArea ( CS_PARTICLES + i);
			if (!rval)
				break;
		}
	}
*/
}



/*																																			
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString() {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*																																			
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

//PKMOD - Ergodic 10/13/00 - add alternate music to hub
/*
======================
CG_StartPostVoteMusic

======================
*/
void CG_StartPostVoteMusic( const char *postvotemusic ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	//PKMOD - Ergodic 10/14/00 - debug inactive
//	Com_Printf( "CG_StartPostVoteMusic - initiating\n" );


	//PKMOD - Ergodic 10/14/00 - only do alternate music if player has voted
//	if ( cg.snap->ps.persistant[PERS_HUB_FLAG] != 1 )
//		return;
	//PKMOD - Ergodic 12/16/00 - change PERS_HUB_FLAG to be first bit of PERS_PAINKILLER_COUNT
	if ( !( cg.snap->ps.persistant[PERS_PAINKILLER_COUNT] & 1 ) ) {
		return;
	}


	//PKMOD - Ergodic 10/14/00 - debug inactive
//	Com_Printf( "CG_StartPostVoteMusic - setting music\n" );

	// start the background music
//	s = (char *)CG_ConfigString( CS_POSTVOTE_MUSIC );
//	s = (char *)CG_ConfigString( CS_MUSIC );

	s = (char *)postvotemusic;

	//PKMOD - Ergodic 10/14/00 - debug inactive
//	Com_Printf( "CG_StartPostVoteMusic - setting music>%s<\n", s );

	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}
//PKMOD - Ergodic 01/17/04 - enable HUD code in PKA 3.0
//#ifdef MISSIONPACK
char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse; // bk001204 - why not?
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
		handle = trap_PC_LoadSource("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token); 
	}
	return qfalse;
}



void CG_LoadMenus(const char *menuFile) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
		len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			trap_Error( va( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	
	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	return qfalse;
}


static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_RED) {
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_BLUE) {
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
		return cg.numScores;
	}
	return count;
}


void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
	gitem_t *item;
	int scoreIndex = 0;
	clientInfo_t *info = NULL;
	int team = -1;
	score_t *sp = NULL;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	if (info && info->infoValid) {
		switch (column) {
			case 0:
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else {
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
				}
			break;
			case 1:
				if (team == -1) {
					return "";
				} else {
					*handle = CG_StatusHandle(info->teamTask);
				}
		  break;
			case 2:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
				if (team == -1) {
					if (cgs.gametype == GT_TOURNAMENT) {
						return va("%i/%i", info->wins, info->losses);
					} else if (info->infoValid && info->team == TEAM_SPECTATOR ) {
						return "Spectator";
					} else {
						return "";
					}
				} else {
					if (info->teamLeader) {
						return "Leader";
					}
				}
			break;
			case 3:
				return info->name;
			break;
			case 4:
				return va("%i", info->score);
			break;
			case 5:
				return va("%4i", sp->time);
			break;
			case 6:
				if ( sp->ping == -1 ) {
					return "connecting";
				} 
				return va("%4i", sp->ping);
			break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
	return 0;
}

static void CG_FeederSelection(float feederID, int index) {
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}
//PKMOD - Ergodic 01/17/04 - enable HUD code in PKA 3.0
//#endif

//PKMOD - Ergodic 01/17/04 - Enable HUD in PKA3.0
//#ifdef MISSIONPACK // bk001204 - only needed there
static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}
//#endif

//PKMOD - Ergodic 01/17/04 - Enable the mission pack code so that HUD code will work
//#ifdef MISSIONPACK
void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style) {
	CG_Text_Paint(x, y, scale, color, text, 0, limit, style);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0);
	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
			break;
	  case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, 0);
			break;
	  case CG_RED_NAME:
			return CG_Text_Width(cg_redTeamName.string, scale, 0);
			break;
	  case CG_BLUE_NAME:
			return CG_Text_Width(cg_blueTeamName.string, scale, 0);
			break;


	}
	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu() {
	char buff[1024];
	const char *hudSet;

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;   
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &trap_S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error; 
	cgDC.Print = &Com_Printf; 
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	
	Init_Display(&cgDC);

	Menu_Reset();
	
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet);
}

void CG_AssetCache() {
	//if (Assets.textFont == NULL) {
	//  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}

//PKMOD - Ergodic 01/17/04 - Enable the mission pack code so that HUD code will work
//#endif

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;
	int			version_test;	//PKMOD - Ergodic 08/19/02 - Hold PKA version tests
	
	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	//PKMOD - Ergodic 08/19/02 - give greater information when version tests fail
	//< 0	string1 less than string2
	//= 0	string1 identical to string2
	//> 0	string1 greater than string2
 

	version_test = strcmp( s, GAME_VERSION );

	if ( version_test ) {
		if ( version_test < 0) {
			//PKMOD - Ergodic 08/19/02 - set the CVAR
			//Server version is less than Client Version 
			trap_Cvar_Set( "cl_pkaerror", va("QUAKE3 Client(%s)/Server(%s) game version mismatch!\n Client at a Higher Version than Server.\n Please try another PKA server", GAME_VERSION, s ) );		
			CG_Error( "QUAKE3 Client(%s)/Server(%s) game version mismatch!\n Client at a Higher Version than Server.\n Please try another PKA server", GAME_VERSION, s );
		}
		else {
			//PKMOD - Ergodic 08/19/02 - set the CVAR
			//Client version is less than Server Version 
			trap_Cvar_Set( "cl_pkaerror", va("QUAKE3 Client(%s)/Server(%s) game version mismatch!\n  Server at a Higher Version than Client.\n Please upgrade at www.idsoftware.com", GAME_VERSION, s ) );		
			CG_Error( "QUAKE3 Client(%s)/Server(%s) game mismatch!\n Server at a Higher Version than Client.\n Please upgrade at www.idsoftware.com", GAME_VERSION, s );
		}
	}

	//PKMOD - Ergodic 02/01/01 - check the PKARENA game version
	s = CG_ConfigString( CS_PKARENA_VERSION );

	//PKMOD - Ergodic 08/19/02 - give greater information when version tests fail
	version_test = strcmp( s, PKARENA_VERSION );
	if ( version_test ) {
		if ( version_test < 0) {
			//PKMOD - Ergodic 08/19/02 - set the CVAR
			//Server version is less than Client Version 
			trap_Cvar_Set( "cl_pkaerror", va("PKARENA Client(%s)/Server(%s) game version mismatch!\n Client at a Higher Version than Server.\n Please try another PKA server", PKARENA_VERSION, s ) );		
			CG_Error( "PKARENA Client(%s)/Server(%s) game version mismatch!\n Client at a Higher Version than Server.\n Please try another PKA server", PKARENA_VERSION, s );
		}
		else {
			//PKMOD - Ergodic 08/19/02 - set the CVAR
			//Client version is less than Server Version 
			trap_Cvar_Set( "cl_pkaerror", va("PKARENA Client(%s)/Server(%s) game version mismatch!\n Server at a Higher Version than Client.\n Please upgrade at www.team-evolve.com", PKARENA_VERSION, s ) );		
			CG_Error( "PKARENA Client(%s)/Server(%s) game version mismatch!\n Server at a Higher Version than Client.\n Please upgrade at www.team-evolve.com", PKARENA_VERSION, s );
		}
	}
	else {
		//PKMOD - Ergodic 08/17/02 - clear the error CVAR
		trap_Cvar_Set( "cl_pkaerror", "0" );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

//PKMOD - Ergodic 01/31/03 - Activate HUD for PKA3.0
//#ifdef MISSIONPACK
	String_Init();
//#endif

	cg.loading = qtrue;		// force players to load instead of defer

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

//PKMOD - Ergodic 03/12/04 - remove this to enable scollbars in scoreboard
//#ifdef MISSIONPACK
	CG_AssetCache();
//PKMOD - Ergodic 03/12/04 - remove this to enable scollbars in scoreboard
//#endif
	//PKMOD - Ergodic 01/17/03 - Activate HUD for PKA3.0
	CG_LoadHudMenu();      // load new hud stuff

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

#ifdef MISSIONPACK
	CG_InitTeamChat();
#endif

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds( qtrue );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
//PKMOD - Ergodic 01/30/04 - Activate HUD - These functions will use TA definitions found in UI module

/* PKMOD++++++++++
#ifndef MISSIONPACK
void CG_EventHandling(int type) {
}



void CG_KeyEvent(int key, qboolean down) {
}

void CG_MouseEvent(int x, int y) {
}
#endif
*/ //PKMOD---------