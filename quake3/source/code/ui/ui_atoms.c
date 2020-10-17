// Copyright (C) 1999-2000 Id Software, Inc.
//
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

qboolean		m_entersound;		// after a frame, so caching won't disrupt the sound

// these are here so the functions in q_shared.c can link
#ifndef UI_HARD_LINKED

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	trap_Error( va("%s", text) );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( va("%s", text) );
}

#endif

qboolean newUI = qfalse;


/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
UI_StartDemoLoop
=================
*/
void UI_StartDemoLoop( void ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}


#ifndef MISSIONPACK // bk001206
static void NeedCDAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}
#endif // MISSIONPACK

#ifndef MISSIONPACK // bk001206
static void NeedCDKeyAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}
#endif // MISSIONPACK

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}



void UI_SetBestScores(postGameInfo_t *newInfo, qboolean postGame) {
	trap_Cvar_Set("ui_scoreAccuracy",     va("%i%%", newInfo->accuracy));
	trap_Cvar_Set("ui_scoreImpressives",	va("%i", newInfo->impressives));
	trap_Cvar_Set("ui_scoreExcellents", 	va("%i", newInfo->excellents));
	trap_Cvar_Set("ui_scoreDefends", 			va("%i", newInfo->defends));
	trap_Cvar_Set("ui_scoreAssists", 			va("%i", newInfo->assists));
	trap_Cvar_Set("ui_scoreGauntlets", 		va("%i", newInfo->gauntlets));
	trap_Cvar_Set("ui_scoreScore", 				va("%i", newInfo->score));
	trap_Cvar_Set("ui_scorePerfect",	 		va("%i", newInfo->perfects));
	trap_Cvar_Set("ui_scoreTeam",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
	trap_Cvar_Set("ui_scoreBase",					va("%i", newInfo->baseScore));
	trap_Cvar_Set("ui_scoreTimeBonus",		va("%i", newInfo->timeBonus));
	trap_Cvar_Set("ui_scoreSkillBonus",		va("%i", newInfo->skillBonus));
//PKMOD - Ergodic 02/02/04 - rename ui_scoreShutoutBonus to ui_scoreMedalBonus 
	trap_Cvar_Set("ui_scoreMedalBonus",	va("%i", newInfo->medalBonus));
	trap_Cvar_Set("ui_scoreTime",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));

	trap_Cvar_Set("ui_scoreCaptures",		va("%i", newInfo->captures));
	//PKMOD - Ergodic 09/17/02 - set two more cvars that are used in the menu
	trap_Cvar_Set("ui_scoreFrags", 			va("%i", newInfo->frags));
	trap_Cvar_Set("ui_scorePainkiller", 	va("%i", newInfo->painkiller));
  if (postGame) {
		trap_Cvar_Set("ui_scoreAccuracy2",     va("%i%%", newInfo->accuracy));
		trap_Cvar_Set("ui_scoreImpressives2",	va("%i", newInfo->impressives));
		trap_Cvar_Set("ui_scoreExcellents2", 	va("%i", newInfo->excellents));
		trap_Cvar_Set("ui_scoreDefends2", 			va("%i", newInfo->defends));
		trap_Cvar_Set("ui_scoreAssists2", 			va("%i", newInfo->assists));
		trap_Cvar_Set("ui_scoreGauntlets2", 		va("%i", newInfo->gauntlets));
		trap_Cvar_Set("ui_scoreScore2", 				va("%i", newInfo->score));
		trap_Cvar_Set("ui_scorePerfect2",	 		va("%i", newInfo->perfects));

		//PKMOD - Ergodic 02/02/04 - set the score to the winning scorer

		trap_Cvar_Set("ui_scoreTeam2",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
		trap_Cvar_Set("ui_scoreBase2",					va("%i", newInfo->baseScore));
		trap_Cvar_Set("ui_scoreTimeBonus2",		va("%i", newInfo->timeBonus));
		trap_Cvar_Set("ui_scoreSkillBonus2",		va("%i", newInfo->skillBonus));
		//PKMOD - Ergodic 02/02/04 - rename ui_scoreShutoutBonus to ui_scoreMedalBonus 
		trap_Cvar_Set("ui_scoreMedalBonus2",	va("%i", newInfo->medalBonus));
		trap_Cvar_Set("ui_scoreTime2",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
		trap_Cvar_Set("ui_scoreCaptures2",		va("%i", newInfo->captures));
		//PKMOD - Ergodic 09/17/02 - set two more cvars that are used in the menu
		trap_Cvar_Set("ui_scoreFrags2", 		va("%i", newInfo->frags));
		trap_Cvar_Set("ui_scorePainkiller2", 				va("%i", newInfo->painkiller));
  }
}

void UI_LoadBestScores(const char *map, int game) {
	char		fileName[MAX_QPATH];
	fileHandle_t f;
	postGameInfo_t newInfo;

	memset(&newInfo, 0, sizeof(postGameInfo_t));
	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	Com_sprintf(fileName, MAX_QPATH, "games/%s.game", map);
	if (trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
		int size = 0;
		trap_FS_Read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			trap_FS_Read(&newInfo, sizeof(postGameInfo_t), f);
		}
		trap_FS_FCloseFile(f);
	}
	UI_SetBestScores(&newInfo, qfalse);

	//PKMOD - Ergodic - 2/03/04 - debug (inactive)
//	Com_Printf( "UI_LoadBestScores map>%d<, newInfo.time>%d<, ui_scoreTime>%d<, ui_scoreTimeRaw>%d<\n", ui_currentMap.integer, newInfo.time, ui_scoreTime.integer, ui_scoreTimeRaw.integer );

	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//Com_sprintf(fileName, MAX_QPATH, "demos/%s_%d.dm_%d", map, game, (int)trap_Cvar_VariableValue("protocol"));
	Com_sprintf(fileName, MAX_QPATH, "demos/%s.dm_%d", map, (int)trap_Cvar_VariableValue("protocol"));
	uiInfo.demoAvailable = qfalse;
	if (trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
		uiInfo.demoAvailable = qtrue;
		trap_FS_FCloseFile(f);
	} 
}

/*
===============
UI_ClearScores
===============
*/
void UI_ClearScores() {
	char	gameList[4096];
	char *gameFile;
	int		i, len, count, size;
	fileHandle_t f;
	postGameInfo_t newInfo;

	count = trap_FS_GetFileList( "games", "game", gameList, sizeof(gameList) );

	size = sizeof(postGameInfo_t);
	memset(&newInfo, 0, size);

	if (count > 0) {
		gameFile = gameList;
		for ( i = 0; i < count; i++ ) {
			len = strlen(gameFile);
			if (trap_FS_FOpenFile(va("games/%s",gameFile), &f, FS_WRITE) >= 0) {
				trap_FS_Write(&size, sizeof(int), f);
				trap_FS_Write(&newInfo, size, f);
				trap_FS_FCloseFile(f);
			}
			gameFile += len + 1;
		}
	}
	
	UI_SetBestScores(&newInfo, qfalse);

}



static void	UI_Cache_f() {
	Display_CacheAll();
}

/*
=======================
UI_CalcPostGameStats
=======================
*/
static void UI_CalcPostGameStats() {
	char		map[MAX_QPATH];
	char		fileName[MAX_QPATH];
	char		info[MAX_INFO_STRING];
	fileHandle_t f;
	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//int size, game, time, adjustedTime;
	int size, time, adjustedTime;

	//PKMOD - Ergodic 02/02/04 - hold Winner Flag
	int	winner;

	//PKMOD - Ergodic 02/02/04 - debug painkiller and frags (inactive)
	//int i;

	postGameInfo_t oldInfo;
	postGameInfo_t newInfo;
	qboolean newHigh = qfalse;

	//PKMOD - Ergodic 02/03/04 - restore the ui_current map cvar/variable
	int	spindex;

	spindex = trap_Cvar_VariableValue("ui_spIndex");

	//PKMOD - Ergodic 02/04/04 - debug spindex (inactive)
	//Com_Printf("UI_CalcPostGameStats - spindex>%d<, uiInfo.mapCount>%d<\n", spindex, uiInfo.mapCount );


	if ( spindex < 0 || spindex > uiInfo.mapCount) {
		spindex = 0;
	}
	ui_currentMap.integer = spindex;
	trap_Cvar_Set("ui_currentMap", va("%i", spindex));

	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	Q_strncpyz( map, Info_ValueForKey( info, "mapname" ), sizeof(map) );
	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//game = atoi(Info_ValueForKey(info, "g_gametype"));

	// compose file name
	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	Com_sprintf(fileName, MAX_QPATH, "games/%s.game", map);
	// see if we have one already
	memset(&oldInfo, 0, sizeof(postGameInfo_t));
	if (trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
	// if so load it
		size = 0;
		trap_FS_Read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			trap_FS_Read(&oldInfo, sizeof(postGameInfo_t), f);
		}
		trap_FS_FCloseFile(f);
	}					 

	//PKMOD - Ergodic 02/02/04 - modify SP postgame argument list to sync with g_arenas.c
	newInfo.accuracy = atoi(UI_Argv(3));
	newInfo.impressives = atoi(UI_Argv(4));
	newInfo.excellents = atoi(UI_Argv(5));
	//newInfo.defends = atoi(UI_Argv(6));	//PKMOD - Ergodic 02/02/04
	//newInfo.assists = atoi(UI_Argv(7));	//PKMOD - Ergodic 02/02/04
	newInfo.gauntlets = atoi(UI_Argv(6));
	newInfo.perfects = atoi(UI_Argv(7));
	//PKMOD - Ergodic 09/19/02 - Store PainKiller count
	newInfo.painkiller = atoi(UI_Argv(8));
	newInfo.baseScore = 10 * atoi(UI_Argv(9));
	//newInfo.redScore = atoi(UI_Argv(11));	//PKMOD - Ergodic 02/02/04
	//newInfo.blueScore = atoi(UI_Argv(12));	//PKMOD - Ergodic 02/02/04
	time = atoi(UI_Argv(10));
	//newInfo.captures = atoi(UI_Argv(14));	//PKMOD - Ergodic 02/02/04
	//PKMOD - Ergodic 02/02/04 - Store Winner Flag
	winner = atoi(UI_Argv(11));
	

	//PKMOD - Ergodic 02/02/04 - debug painkiller and frags (inactive)
//	Com_Printf("UI_CalcPostGameStats" );
//	for (i= 3; i<16; i++) {
//		Com_Printf(" %d>%d<,", i, atoi(UI_Argv(i)) );
//	}
//	Com_Printf("\n" );

	newInfo.time = (time - trap_Cvar_VariableValue("ui_matchStartTime")) / 1000;
	//PKMOD - Ergodic 02/03/04 - Remove gametype from saved file name
	//adjustedTime = uiInfo.mapList[ui_currentMap.integer].timeToBeat[game];
	//adjustedTime = uiInfo.mapList[ui_currentMap.integer].timeToBeat[0];
	//PKMOD - Ergodic 02/04/04 - get adjusted time from fragcount
	adjustedTime = 60 * atoi( uiInfo.mapList[ui_currentMap.integer].fragLimit ) / AVERAGE_FRAGS_PER_MINUTE;	//time in seconds

	//PKMOD - Ergodic 02/04/04 -adjustedTime (inactive)
	//Com_Printf("UI_CalcPostGameStats - adjustedTime>%d<, newInfo.time>%d<\n", adjustedTime, newInfo.time );

	if (newInfo.time < adjustedTime) {
		//PKMOD - Ergodic 02/05/04 - reduce time bonus multiplier from 10 to 2
		//newInfo.timeBonus = (adjustedTime - newInfo.time) * 10;
		newInfo.timeBonus = (adjustedTime - newInfo.time) * 2;
	} else {
		newInfo.timeBonus = 0;
	}

//PKMOD - Ergodic 02/02/04 - don't compute Shutout for SP DM games
//	if (newInfo.redScore > newInfo.blueScore && newInfo.blueScore <= 0) {
//		newInfo.shutoutBonus = 100;
//	} else {
//		newInfo.shutoutBonus = 0;
//	}

	newInfo.skillBonus = trap_Cvar_VariableValue("g_spSkill");
	if (newInfo.skillBonus <= 0) {
		newInfo.skillBonus = 1;
	}
	//PKMOD - Ergodic 02/02/04 - add medal count into score
	//			Each medal earned is worth 10 points
	//PKMOD - Ergodic 02/04/04 - change medal multiplier from 10 to 25 points
	newInfo.medalBonus = 25 * (newInfo.impressives + newInfo.excellents + newInfo.gauntlets + newInfo.perfects + newInfo.painkiller );

	//PKMOD - Ergodic 02/02/04 - don't compute Shutout for SP DM games
	//	newInfo.score = newInfo.baseScore + newInfo.shutoutBonus + newInfo.timeBonus;
	//PKMOD - Ergodic 02/02/04 - add medal count into score
	//			Each medal earned is worth 25 points
	//PKMOD - Ergodic 02/04/04 - change base score multiplier from 1 to 10 points
	newInfo.score = newInfo.baseScore + newInfo.timeBonus + newInfo.medalBonus;

	newInfo.score *= newInfo.skillBonus;

	//PKMOD - Ergodic 02/05/04 - store high score for display on SP Postgame menu - endofgame
	trap_Cvar_Set( "ui_oldHighScore", va("%i", oldInfo.score) );

	// see if the score is higher for this one
	//newHigh = (newInfo.redScore > newInfo.blueScore && newInfo.score > oldInfo.score);
	//PKMOD - Ergodic 02/02/04 - change setting for DM games
	//				only set newhigh if you have won the game
	newHigh = winner && (newInfo.score > oldInfo.score);

	//PKMOD - Ergodic 02/02/04 - debug values (inactive)
///	Com_Printf( "UI_CalcPostGameStats - newHigh>%d<, winner>%d<, newInfo.score>%d<, oldInfo.score>%d<\n", newHigh, winner, newInfo.score, oldInfo.score );
	
	if  (newHigh) {
		// if so write out the new one
		uiInfo.newHighScoreTime = uiInfo.uiDC.realTime + 20000;
		if (trap_FS_FOpenFile(fileName, &f, FS_WRITE) >= 0) {
			size = sizeof(postGameInfo_t);
			trap_FS_Write(&size, sizeof(int), f);
			trap_FS_Write(&newInfo, sizeof(postGameInfo_t), f);
			trap_FS_FCloseFile(f);
		}
	}

	if (newInfo.time < oldInfo.time) {
		uiInfo.newBestTime = uiInfo.uiDC.realTime + 20000;
	}
 
	// put back all the ui overrides
	trap_Cvar_Set("capturelimit", UI_Cvar_VariableString("ui_saveCaptureLimit"));
	trap_Cvar_Set("fraglimit", UI_Cvar_VariableString("ui_saveFragLimit"));
	trap_Cvar_Set("cg_drawTimer", UI_Cvar_VariableString("ui_drawTimer"));
	trap_Cvar_Set("g_doWarmup", UI_Cvar_VariableString("ui_doWarmup"));
	trap_Cvar_Set("g_Warmup", UI_Cvar_VariableString("ui_Warmup"));
	trap_Cvar_Set("sv_pure", UI_Cvar_VariableString("ui_pure"));
	trap_Cvar_Set("g_friendlyFire", UI_Cvar_VariableString("ui_friendlyFire"));

	UI_SetBestScores(&newInfo, qtrue);
	UI_ShowPostGame(newHigh);


}


/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realTime;

	cmd = UI_Argv( 0 );

	// ensure minimum menu data is available
	//Menu_Cache();

	if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		UI_ShowPostGame(qtrue);
	}

	if ( Q_stricmp (cmd, "ui_report") == 0 ) {
		UI_Report();
		return qtrue;
	}
	
	if ( Q_stricmp (cmd, "ui_load") == 0 ) {
		UI_Load();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (trap_Argc() == 4) {
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			Q_strncpyz(shader1, UI_Argv(1), sizeof(shader1));
			Q_strncpyz(shader2, UI_Argv(2), sizeof(shader2));
			trap_R_RemapShader(shader1, shader2, UI_Argv(3));
			return qtrue;
		}
	}

	if ( Q_stricmp (cmd, "postgame") == 0 ) {
		//PKMOD - Ergodic 02/04/04 - Call loadarena before you calculate postgame stats
		UI_LoadArenas();

		UI_CalcPostGameStats();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cache") == 0 ) {
		UI_Cache_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_teamOrders") == 0 ) {
		//UI_TeamOrdersMenu_f();
		return qtrue;
	}


	if ( Q_stricmp (cmd, "ui_cdkey") == 0 ) {
		//UI_CDKeyMenu_f();
		return qtrue;
	}

//PKMOD - Ergodic 08/26/03 - display hub alternates menu
//PKMOD - Ergodic 09/12/03 - modify code to make it more closer to production
	if ( Q_stricmp (cmd, "hubalternates") == 0 ) {
//		trap_Cvar_Set ("cg_cameraOrbit", "0");
//		trap_Cvar_Set("cg_thirdPerson", "0");
//		//trap_Cvar_Set( "sv_killserver", "1" );
		_UI_SetActiveMenu(UIMENU_HUBALTERNATES);
		return qtrue;
	}


	return qfalse;
}

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
#if 0
	*x = *x * uiInfo.uiDC.scale + uiInfo.uiDC.bias;
	*y *= uiInfo.uiDC.scale;
	*w *= uiInfo.uiDC.scale;
	*h *= uiInfo.uiDC.scale;
#endif

	*x *= uiInfo.uiDC.xscale;
	*y *= uiInfo.uiDC.yscale;
	*w *= uiInfo.uiDC.xscale;
	*h *= uiInfo.uiDC.yscale;

}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = trap_R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );

	trap_R_SetColor( NULL );
}

void UI_DrawSides(float x, float y, float w, float h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void UI_DrawTopBottom(float x, float y, float w, float h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

  UI_DrawTopBottom(x, y, width, height);
  UI_DrawSides(x, y, width, height);

	trap_R_SetColor( NULL );
}

void UI_SetColor( const float *rgba ) {
	trap_R_SetColor( rgba );
}

void UI_UpdateScreen( void ) {
	trap_UpdateScreen();
}


void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	UI_DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uiInfo.uiDC.cursorx < x ||
		uiInfo.uiDC.cursory < y ||
		uiInfo.uiDC.cursorx > x+width ||
		uiInfo.uiDC.cursory > y+height)
		return qfalse;

	return qtrue;
}
