// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_bot.c

#include "g_local.h"


static int		g_numBots;
static char		*g_botInfos[MAX_BOTS];


int				g_numArenas;
static char		*g_arenaInfos[MAX_ARENAS];


#define BOT_BEGIN_DELAY_BASE		2000
#define BOT_BEGIN_DELAY_INCREMENT	1500

#define BOT_SPAWN_QUEUE_DEPTH	16

typedef struct {
	int		clientNum;
	int		spawnTime;
} botSpawnQueue_t;

//static int			botBeginDelay = 0;  // bk001206 - unused, init
static botSpawnQueue_t	botSpawnQueue[BOT_SPAWN_QUEUE_DEPTH];

vmCvar_t bot_minplayers;

extern gentity_t	*podium1;
extern gentity_t	*podium2;
extern gentity_t	*podium3;

float trap_Cvar_VariableValue( const char *var_name ) {
	char buf[128];

	trap_Cvar_VariableStringBuffer(var_name, buf, sizeof(buf));
	return atof(buf);
}



/*
===============
G_ParseInfos
===============
*/
int G_ParseInfos( char *buf, int max, char *infos[] ) {
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) {
				strcpy( token, "<NULL>" );
			}
			Info_SetValueForKey( info, key, token );
		}
		//NOTE: extra space for arena number
		infos[count] = G_Alloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);
		if (infos[count]) {
			strcpy(infos[count], info);
			count++;
		}
	}
	return count;
}

/*
===============
G_LoadArenasFromFile
===============
*/
static void G_LoadArenasFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_ARENAS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	g_numArenas += G_ParseInfos( buf, MAX_ARENAS - g_numArenas, &g_arenaInfos[g_numArenas] );
}

/*
===============
G_LoadArenas
===============
*/
static void G_LoadArenas( void ) {
	int			numdirs;
	vmCvar_t	arenasFile;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i, n;
	int			dirlen;

	g_numArenas = 0;

	trap_Cvar_Register( &arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM );
	if( *arenasFile.string ) {
		G_LoadArenasFromFile(arenasFile.string);
	}
	else {
		G_LoadArenasFromFile("scripts/arenas.txt");
	}

	// get all arenas from .arena files
	numdirs = trap_FS_GetFileList("scripts", ".arena", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadArenasFromFile(filename);
	}
	trap_Printf( va( "%i arenas parsed\n", g_numArenas ) );
	
	for( n = 0; n < g_numArenas; n++ ) {
		Info_SetValueForKey( g_arenaInfos[n], "num", va( "%i", n ) );
	}
}


/*
===============
G_GetArenaInfoByNumber
===============
*/
const char *G_GetArenaInfoByMap( const char *map ) {
	int			n;

	for( n = 0; n < g_numArenas; n++ ) {
		if( Q_stricmp( Info_ValueForKey( g_arenaInfos[n], "map" ), map ) == 0 ) {
			return g_arenaInfos[n];
		}
	}

	return NULL;
}


/*
=================
PlayerIntroSound
=================
*/
static void PlayerIntroSound( const char *modelAndSkin ) {
	char	model[MAX_QPATH];
	char	*skin;

	Q_strncpyz( model, modelAndSkin, sizeof(model) );
	skin = Q_strrchr( model, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = model;
	}

	if( Q_stricmp( skin, "default" ) == 0 ) {
		skin = model;
	}

	trap_SendConsoleCommand( EXEC_APPEND, va( "play sound/player/announce/%s.wav\n", skin ) );
}

/*
===============
G_AddRandomBot
===============
*/
void G_AddRandomBot( int team ) {
	int		i, n, num;
	float	skill;
	char	*value, netname[36], *teamstr;
	gclient_t	*cl;

	num = 0;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= g_maxclients.integer) {
			num++;
		}
	}
	num = random() * num;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= g_maxclients.integer) {
			num--;
			if (num <= 0) {
				skill = trap_Cvar_VariableValue( "g_spSkill" );
				if (team == TEAM_RED) teamstr = "red";
				else if (team == TEAM_BLUE) teamstr = "blue";
				else teamstr = "";
				strncpy(netname, value, sizeof(netname)-1);
				netname[sizeof(netname)-1] = '\0';
				Q_CleanStr(netname);

				//PKMOD - Ergodic 01/14/02 - debug bot's restart (inactive)
//				Com_Printf("G_AddRandomBot - >%s<\n", netname );

				trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f %s %i\n", netname, skill, teamstr, 0) );
				return;
			}
		}
	}
}

/*
===============
G_RemoveRandomBot
===============
*/
int G_RemoveRandomBot( int team ) {
	int i;
	char netname[36];
	gclient_t	*cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
			continue;
		}
		if ( team >= 0 && cl->sess.sessionTeam != team ) {
			continue;
		}

		//PKMOD - Ergodic 12/16/02 - Never remove a Private Bot
		//		02/29/04 - NOTE: this works for SP games but not Client/Server games
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) {
			continue;
		}

		strcpy(netname, cl->pers.netname);
		Q_CleanStr(netname);

		//PKMOD - Ergodic 02/29/04 - debug PB removal on client/server games (inactive)
		//Com_Printf( "G_RemoveRandomBot - removing i>%d<, i_flags>%d<, cl->ps.clientNum>%d<, clientNum_flags>%d<, netname>%s<\n", i, g_entities[i].r.svFlags, cl->ps.clientNum, g_entities[cl->ps.clientNum].r.svFlags, netname );
		//G_LogPrintf( "G_RemoveRandomBot - removing i>%d<, i_flags>%d<, cl->ps.clientNum>%d<, clientNum_flags>%d<, netname>%s<\n", i, g_entities[i].r.svFlags, cl->ps.clientNum, g_entities[cl->ps.clientNum].r.svFlags, netname );

		trap_SendConsoleCommand( EXEC_INSERT, va("kick %s\n", netname) );
		return qtrue;
	}
	return qfalse;
}

/*
===============
G_CountHumanPlayers
===============
*/
int G_CountHumanPlayers( int team ) {
	int i, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}
		if ( team >= 0 && cl->sess.sessionTeam != team ) {
			continue;
		}
		num++;
	}
	return num;
}

/*
===============
G_CountBotPlayers
===============
*/
int G_CountBotPlayers( int team ) {
	int i, n, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
			continue;
		}
		if ( team >= 0 && cl->sess.sessionTeam != team ) {
			continue;
		}
		//PKMOD - Ergodic 01/21/02 - don't count Private Bots (inactive)
		//PKMOD - Ergodic 07/21/02 - don't count Private Bots, now active
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) {
			continue;
		}
		num++;
	}
	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) {
			continue;
		}
		num++;
	}

	return num;
}

/*
===============
G_CheckMinimumPlayers
===============
*/
void G_CheckMinimumPlayers( void ) {
	int minplayers;
	int humanplayers, botplayers;
	static int checkminimumplayers_time;

	if (level.intermissiontime) return;
	//only check once each 10 seconds
	if (checkminimumplayers_time > level.time - 10000) {
		return;
	}
	checkminimumplayers_time = level.time;
	trap_Cvar_Update(&bot_minplayers);
	minplayers = bot_minplayers.integer;
	if (minplayers <= 0) return;

	if (g_gametype.integer >= GT_TEAM) {
		if (minplayers >= g_maxclients.integer / 2) {
			minplayers = (g_maxclients.integer / 2) -1;
		}

		humanplayers = G_CountHumanPlayers( TEAM_RED );
		botplayers = G_CountBotPlayers(	TEAM_RED );
		//
		if (humanplayers + botplayers < minplayers) {
			G_AddRandomBot( TEAM_RED );
		} else if (humanplayers + botplayers > minplayers && botplayers) {
			G_RemoveRandomBot( TEAM_RED );
		}
		//
		humanplayers = G_CountHumanPlayers( TEAM_BLUE );
		botplayers = G_CountBotPlayers( TEAM_BLUE );
		//
		if (humanplayers + botplayers < minplayers) {
			G_AddRandomBot( TEAM_BLUE );
		} else if (humanplayers + botplayers > minplayers && botplayers) {
			G_RemoveRandomBot( TEAM_BLUE );
		}
	}
	else if (g_gametype.integer == GT_TOURNAMENT ) {
		if (minplayers >= g_maxclients.integer) {
			minplayers = g_maxclients.integer-1;
		}
		humanplayers = G_CountHumanPlayers( -1 );
		botplayers = G_CountBotPlayers( -1 );
		//
		if (humanplayers + botplayers < minplayers) {
			G_AddRandomBot( TEAM_FREE );
		} else if (humanplayers + botplayers > minplayers && botplayers) {
			// try to remove spectators first
			if (!G_RemoveRandomBot( TEAM_SPECTATOR )) {
				// just remove the bot that is playing
				G_RemoveRandomBot( -1 );
			}
		}
	}
	else if (g_gametype.integer == GT_FFA) {
		if (minplayers >= g_maxclients.integer) {
			minplayers = g_maxclients.integer-1;
		}
		humanplayers = G_CountHumanPlayers( TEAM_FREE );
		botplayers = G_CountBotPlayers( TEAM_FREE );
		//
		if (humanplayers + botplayers < minplayers) {
			G_AddRandomBot( TEAM_FREE );
		} else if (humanplayers + botplayers > minplayers && botplayers) {
			G_RemoveRandomBot( TEAM_FREE );
		}
	}
}

/*
===============
G_CheckBotSpawn
===============
*/
void G_CheckBotSpawn( void ) {
	int		n;
	char	userinfo[MAX_INFO_VALUE];

	G_CheckMinimumPlayers();

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) {
			continue;
		}
		//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//		Com_Printf("G_CheckBotSpawn - before client begin\n" );

		ClientBegin( botSpawnQueue[n].clientNum );

		//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//		Com_Printf("G_CheckBotSpawn - after client begin\n" );

		botSpawnQueue[n].spawnTime = 0;

		if( g_gametype.integer == GT_SINGLE_PLAYER ) {
			trap_GetUserinfo( botSpawnQueue[n].clientNum, userinfo, sizeof(userinfo) );
			PlayerIntroSound( Info_ValueForKey (userinfo, "model") );
		}
	}
}


/*
===============
AddBotToSpawnQueue
===============
*/
static void AddBotToSpawnQueue( int clientNum, int delay ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			botSpawnQueue[n].spawnTime = level.time + delay;
			botSpawnQueue[n].clientNum = clientNum;
			return;
		}
	}

	G_Printf( S_COLOR_YELLOW "Unable to delay spawn\n" );
	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("AddBotToSpawnQueue - before client begin\n" );

	ClientBegin( clientNum );

	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("AddBotToSpawnQueue - after client begin\n" );

}


/*
===============
G_RemoveQueuedBotBegin

Called on client disconnect to make sure the delayed spawn
doesn't happen on a freed index
===============
*/
void G_RemoveQueuedBotBegin( int clientNum ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( botSpawnQueue[n].clientNum == clientNum ) {
			botSpawnQueue[n].spawnTime = 0;
			return;
		}
	}
}


/*
===============
G_BotConnect
===============
*/
qboolean G_BotConnect( int clientNum, qboolean restart ) {
	bot_settings_t	settings;
	char			userinfo[MAX_INFO_STRING];

	trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );

	Q_strncpyz( settings.characterfile, Info_ValueForKey( userinfo, "characterfile" ), sizeof(settings.characterfile) );
	settings.skill = atof( Info_ValueForKey( userinfo, "skill" ) );
	Q_strncpyz( settings.team, Info_ValueForKey( userinfo, "team" ), sizeof(settings.team) );

	if (!BotAISetupClient( clientNum, &settings, restart )) {
		trap_DropClient( clientNum, "BotAISetupClient failed" );
		return qfalse;
	}

	return qtrue;
}


/*
===============
G_AddBot
===============
*/
static void G_AddBot( const char *name, float skill, const char *team, int delay, char *altname) {
	int				clientNum;
	char			*botinfo;
	gentity_t		*bot;
	char			*key;
	char			*s;
	char			*botname;
	char			*model;
	char			*headmodel;
	char			userinfo[MAX_INFO_STRING];

	//PKMOD - Ergodic 02/19/04 - debug bot's userinfo (inactive)
	//Com_Printf("G_AddBot - >%s<\n", name );

	// get the botinfo from bots.txt
	botinfo = G_GetBotInfoByName( name );
	if ( !botinfo ) {
		G_Printf( S_COLOR_RED "Error: Bot '%s' not defined\n", name );
		return;
	}

	// create the bot's userinfo
	userinfo[0] = '\0';

	botname = Info_ValueForKey( botinfo, "funname" );
	if( !botname[0] ) {
		botname = Info_ValueForKey( botinfo, "name" );
	}
	// check for an alternative name
	if (altname && altname[0]) {
		botname = altname;
	}
	Info_SetValueForKey( userinfo, "name", botname );
	Info_SetValueForKey( userinfo, "rate", "25000" );
	Info_SetValueForKey( userinfo, "snaps", "20" );
	Info_SetValueForKey( userinfo, "skill", va("%1.2f", skill) );

	if ( skill >= 1 && skill < 2 ) {
		Info_SetValueForKey( userinfo, "handicap", "50" );
	}
	else if ( skill >= 2 && skill < 3 ) {
		Info_SetValueForKey( userinfo, "handicap", "70" );
	}
	else if ( skill >= 3 && skill < 4 ) {
		Info_SetValueForKey( userinfo, "handicap", "90" );
	}

	key = "model";
	model = Info_ValueForKey( botinfo, key );
	if ( !*model ) {
		model = "visor/default";
	}
	Info_SetValueForKey( userinfo, key, model );
	key = "team_model";
	Info_SetValueForKey( userinfo, key, model );

	key = "headmodel";
	headmodel = Info_ValueForKey( botinfo, key );
	if ( !*headmodel ) {
		headmodel = model;
	}
	Info_SetValueForKey( userinfo, key, headmodel );
	key = "team_headmodel";
	Info_SetValueForKey( userinfo, key, headmodel );

	key = "gender";
	s = Info_ValueForKey( botinfo, key );
	if ( !*s ) {
		s = "male";
	}
	Info_SetValueForKey( userinfo, "sex", s );

	key = "color1";
	s = Info_ValueForKey( botinfo, key );
	if ( !*s ) {
		s = "4";
	}
	Info_SetValueForKey( userinfo, key, s );

	key = "color2";
	s = Info_ValueForKey( botinfo, key );
	if ( !*s ) {
		s = "5";
	}
	Info_SetValueForKey( userinfo, key, s );

	s = Info_ValueForKey(botinfo, "aifile");
	if (!*s ) {
		trap_Printf( S_COLOR_RED "Error: bot has no aifile specified\n" );
		return;
	}

	// have the server allocate a client slot
	clientNum = trap_BotAllocateClient();
	if ( clientNum == -1 ) {
		G_Printf( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
		G_Printf( S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n" );
		return;
	}

	// initialize the bot settings
	if( !team || !*team ) {
		if( g_gametype.integer >= GT_TEAM ) {
			if( PickTeam(clientNum) == TEAM_RED) {
				team = "red";
			}
			else {
				team = "blue";
			}
		}
		else {
			team = "red";
		}
	}
	Info_SetValueForKey( userinfo, "characterfile", Info_ValueForKey( botinfo, "aifile" ) );
	Info_SetValueForKey( userinfo, "skill", va( "%5.2f", skill ) );
	Info_SetValueForKey( userinfo, "team", team );

	bot = &g_entities[ clientNum ];
	bot->r.svFlags |= SVF_BOT;
	//PKMOD - Ergodic 03/07/02 - clear Private Bot flag
	bot->r.svFlags &= ~SVF_PRIVATEBOT;

	bot->inuse = qtrue;

	//PKMOD - Ergodic 01/06/02 - debug bot's userinfo
//	Com_Printf("G_AddBot - clientNum is>%d<\n", clientNum );
//	Com_Printf("G_AddBot - g_PrivateBotSkill.integer>%d<\n", g_PrivateBotSkill.integer );
//	Com_Printf("G_AddBot - g_PrivateBotSkill.value>%1.2f<\n", g_PrivateBotSkill.value );
//	Com_Printf("G_AddBot - aifile>%s<\n", Info_ValueForKey( botinfo, "aifile" ));
//	Com_Printf("G_AddBot - userinfo>%s<\n", userinfo);
//	Info_SetValueForKey( userinfo, "legsmodel", "hunter/default" );
//	Info_SetValueForKey( userinfo, "headmodel", "visor/painkiller" );
//	Com_Printf("G_AddBot - userinfo>%s<\n", userinfo);

	// register the userinfo
	trap_SetUserinfo( clientNum, userinfo );

	// have it connect to the game as a normal client
	if ( ClientConnect( clientNum, qtrue, qtrue ) ) {
		return;
	}

	if( delay == 0 ) {
	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("G_AddBot - before client begin\n" );

		ClientBegin( clientNum );

	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("G_AddBot - after client begin\n" );

		return;
	}

	AddBotToSpawnQueue( clientNum, delay );
}


/*
===============
Svcmd_AddBot_f
===============
*/
void Svcmd_AddBot_f( void ) {
	float			skill;
	int				delay;
	char			name[MAX_TOKEN_CHARS];
	char			altname[MAX_TOKEN_CHARS];
	char			string[MAX_TOKEN_CHARS];
	char			team[MAX_TOKEN_CHARS];

	// are bots enabled?
	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	// name
	trap_Argv( 1, name, sizeof( name ) );
	if ( !name[0] ) {
		trap_Printf( "Usage: Addbot <botname> [skill 1-5] [team] [msec delay] [altname]\n" );
		return;
	}

	// skill
	trap_Argv( 2, string, sizeof( string ) );
	if ( !string[0] ) {
		skill = 4;
	}
	else {
		skill = atof( string );
	}

	// team
	trap_Argv( 3, team, sizeof( team ) );

	// delay
	trap_Argv( 4, string, sizeof( string ) );
	if ( !string[0] ) {
		delay = 0;
	}
	else {
		delay = atoi( string );
	}

	// alternative name
	trap_Argv( 5, altname, sizeof( altname ) );

	G_AddBot( name, skill, team, delay, altname );

	// if this was issued during gameplay and we are playing locally,
	// go ahead and load the bot's media immediately
	if ( level.time - level.startTime > 1000 &&
		trap_Cvar_VariableIntegerValue( "cl_running" ) ) {
		trap_SendServerCommand( -1, "loaddefered\n" );	// FIXME: spelled wrong, but not changing for demo
	}
}

/*
======================================================================

PKMOD - Ergodic 01/13/02 - Private Bot Driver entity
	- verifys the status of client owner
	- extinguish private bot at end of duration

Note:	ent->parent = Private Bot
		ent->parent->parent = owner of Private Bot

======================================================================
*/
void botdriverThink (gentity_t *ent) {

	//remove driver entity if Private Bot is disconnected 
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		G_FreeEntity( ent );
		return;
	}

	//remove Private Bot if owner has disconnected
	if (!strcmp( ent->parent->parent->classname,"disconnected" ) || ( ent->parent->parent->client->pers.connected == CON_DISCONNECTED )) {
//		trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", ent->parent->client->ps.clientNum ) );

		//PKMOD - Ergodic 02/27/04 - Reset the Private Bot flag
		//		This flag may cause next clients that start in the same area to be unable to join
		//		in the server
		ent->parent->r.svFlags &=  ~SVF_PRIVATEBOT;

		//PKMOD - Ergodic 01/19/02 - use trap_DropClient instead of clientkick 
		trap_DropClient( ent->parent->client->ps.clientNum, "has lost faith in his Leader" );
		//PKMOD - Ergodic 03/18/02 - decrement the Private Bot Counter 
		active_private_bots--;	
		G_FreeEntity( ent );
		return;
	}

	//remove Private Bot if owner's status has changed
	if ( ent->s.modelindex != ent->parent->parent->client->sess.sessionTeam ) {
//		trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", ent->parent->client->ps.clientNum ) );

		//PKMOD - Ergodic 02/27/04 - Reset the Private Bot flag
		//		This flag may cause next clients that start in the same area to be unable to join
		//		in the server
		ent->parent->r.svFlags &=  ~SVF_PRIVATEBOT;

		//PKMOD - Ergodic 01/19/02 - use trap_DropClient instead of clientkick 
		trap_DropClient( ent->parent->client->ps.clientNum, "was Betrayed by his Leader" );
		//PKMOD - Ergodic 03/18/02 - decrement the Private Bot Counter 
		active_private_bots--;	
		G_FreeEntity( ent );
		return;
	}

	if ( ent->wait > level.time ) {
		ent->eventTime = level.time;
		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	}
	else {
//		trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", ent->parent->client->ps.clientNum ) );

		//PKMOD - Ergodic 02/27/04 - Reset the Private Bot flag
		//		This flag may cause next clients that start in the same area to be unable to join
		//		in the server
		ent->parent->r.svFlags &=  ~SVF_PRIVATEBOT;

		//PKMOD - Ergodic 01/19/02 - use trap_DropClient instead of clientkick 
		trap_DropClient( ent->parent->client->ps.clientNum, "has completed his mission!" );
		//PKMOD - Ergodic 03/18/02 - decrement the Private Bot Counter 
		active_private_bots--;	
		G_FreeEntity( ent );
	}
}

#define	PRIVATE_BOT_DURATION 40000		//40 seconds

/*
===============
G_AddPrivateBot

PKMOD - Ergodic 01/06/02 - add "Private Bot" to the game  
===============
*/
void G_AddPrivateBot( gentity_t *owner ) {
	int				clientNum;
//	char			*botinfo;
	gentity_t		*bot;
	char			userinfo[MAX_INFO_STRING];
	float			skill;
	char			*team;
	char			model[MAX_INFO_STRING];
	char			headmodel[MAX_INFO_STRING];
	char			sex[MAX_INFO_STRING];
	char			ownerinfo[MAX_INFO_STRING];
	//PKMOD - Ergodic 01/13/02 - add driver entity for "Private Bot"
	gentity_t		*bot_driver;
	//PKMOD - Ergodic 04/02/02 - owner's cells will add to Private Bot's duration
	int				cells;

	// create the bot's userinfo
	userinfo[0] = '\0';

	Info_SetValueForKey( userinfo, "name", "PrivateBot" );
	Info_SetValueForKey( userinfo, "rate", "25000" );
	Info_SetValueForKey( userinfo, "snaps", "20" );

	skill = g_PrivateBotSkill.value;

	//PKMOD - Ergodic 01/10/02 - debug bot's skill (inactive)
//	Com_Printf("G_AddPrivateBot - skill>%5.2f<\n", skill );

	Info_SetValueForKey( userinfo, "skill", va("%1.2f", skill) );

	if ( skill >= 1 && skill < 2 ) {
		Info_SetValueForKey( userinfo, "handicap", "50" );
	}
	else if ( skill >= 2 && skill < 3 ) {
		Info_SetValueForKey( userinfo, "handicap", "70" );
	}
	else if ( skill >= 3 && skill < 4 ) {
		Info_SetValueForKey( userinfo, "handicap", "90" );
	}

	//PKMOD - Ergodic 03/06/02 - get owner info
	trap_GetConfigstring(CS_PLAYERS+owner->client->ps.clientNum, ownerinfo, sizeof(ownerinfo));

//	Info_SetValueForKey( userinfo, "model", "visor/painkeep" );
	//PKMOD - Ergodic 03/06/02 - set Private Bot's model and skin to mimic owners
	//model
	strcpy(model, Info_ValueForKey( ownerinfo, "model" ));
	if ( !*model ) {
		strcpy(model, "visor/default");
	}
	Info_SetValueForKey( userinfo, "model", model );

//	Info_SetValueForKey( userinfo, "headmodel", "visor/painkeep" );
	//PKMOD - Ergodic 03/07/02 - set Private Bot's model and skin to mimic owners
	//headmodel
	strcpy(headmodel, Info_ValueForKey( ownerinfo, "headmodel" ));
	if ( !*headmodel ) {	//if headmodel is not set then default to model
		strcpy(headmodel, model);
	}
	Info_SetValueForKey( userinfo, "headmodel", headmodel );

	//PKMOD - Ergodic 03/07/02 - set Private Bot's model and skin to mimic owners
	//headmodel
//	Info_SetValueForKey( userinfo, "sex", "male" );
	strcpy(sex, Info_ValueForKey( ownerinfo, "sex" ));
	if ( !*sex ) {	//if sex is not set then default to male (Biased?... hmmmm)
		strcpy(sex, "male");
	}
	Info_SetValueForKey( userinfo, "sex", sex );

	Info_SetValueForKey( userinfo, "color1", "3" );
	Info_SetValueForKey( userinfo, "color2", "3" );

	//PKMOD - Ergodic 01/09/02 - add info to structure so Private Bot will not appear in scoreboard 
	Info_SetValueForKey( userinfo, "privateBot", "1" );

	// have the server allocate a client slot
	clientNum = trap_BotAllocateClient();
	if ( clientNum == -1 ) {
		G_Printf( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
		G_Printf( S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n" );
		return;
	}

	// initialize the bot team settings
	if( g_gametype.integer >= GT_TEAM ) {
		// same team, if the flag at base, check to he has the enemy flag
		if (owner->client->sess.sessionTeam == TEAM_RED)
			team = "red";
		else 
			team = "blue";
	}

	Info_SetValueForKey( userinfo, "characterfile", "bots/privatebot_c.c" );
	Info_SetValueForKey( userinfo, "team", team );

	bot = &g_entities[ clientNum ];
	bot->r.svFlags |= SVF_BOT;
	bot->r.svFlags |= SVF_PRIVATEBOT;
	bot->inuse = qtrue;

	//set the owner
	//PKMOD - Ergodic 01/10/02 - if owner is a Private Bot then set owner to the originating player
	if ( owner->r.svFlags & SVF_PRIVATEBOT ) 
		bot->parent = owner->parent;
	else
		bot->parent = owner;

	//PKMOD - Ergodic 01/09/02 - debug bot's userinfo (inactive)
//	Com_Printf("G_AddPrivateBot - userinfo>%s<\n", userinfo);

	// register the userinfo
	trap_SetUserinfo( clientNum, userinfo );

	//PKMOD - Ergodic 04/02/02 - calculate cells
	cells = owner->client->ps.ammo[ WP_LIGHTNING ] / 10;
	//update lightning ammo
	owner->client->ps.ammo[ WP_LIGHTNING ] -= cells * 10;
	
	//PKMOD - Ergodic 01/13/02 - establish driver entity parameters for "Private Bot"
	bot_driver = G_Spawn();
	bot_driver->classname = "PrivateBotDriver";
	bot_driver->s.eType = ET_GENERAL;
	bot_driver->r.ownerNum = bot->s.number;
	bot_driver->parent = bot;
	bot_driver->s.modelindex = bot->parent->client->sess.sessionTeam;
	bot_driver->think = botdriverThink;
	//PKMOD - Ergodic 04/03/02 - add "cell" time to Private Bot duration
	bot_driver->wait = level.time + PRIVATE_BOT_DURATION + 4000 * random() + 1000 * cells; 
	bot_driver->nextthink = level.time + 300;
	bot_driver->eventTime = level.time;

	//PKMOD - Ergodic 03/15/04 - debug bot's restart (inactive)
//	Com_Printf("G_AddPrivateBot - before client connect\n" );
//	G_LogPrintf( "G_AddPrivateBot - before client connect\n" );

	// have it connect to the game as a normal client
	if ( ClientConnect( clientNum, qtrue, qtrue ) ) {
		//PKMOD - Ergodic 03/15/04 - debug bot's restart (inactive)
//		Com_Printf("G_AddPrivateBot - premature leaving of procedure\n" );
//		G_LogPrintf("G_AddPrivateBot - premature leaving of procedure\n" );
		return;
	}

	//PKMOD - Ergodic 03/15/04 - debug bot's restart (inactive)
//	Com_Printf("G_AddPrivateBot - before client begin\n" );
//	G_LogPrintf("G_AddPrivateBot - before client begin\n" );

	ClientBegin( clientNum );

	//PKMOD - Ergodic 01/19/02 - set bot's playerstate for use on cgame side
	//PKMOD - Ergodic 02/05/02 - change STAT_PKA_BITS settings from enum type to definition
	bot->client->ps.stats[STAT_PKA_BITS] = PKA_BITS_PRIVATEBOT;

	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("G_AddPrivateBot - after client begin\n" );

}


/*
===============
Svcmd_BotList_f
===============
*/
void Svcmd_BotList_f( void ) {
	int i;
	char name[MAX_TOKEN_CHARS];
	char funname[MAX_TOKEN_CHARS];
	char model[MAX_TOKEN_CHARS];
	char aifile[MAX_TOKEN_CHARS];

	trap_Printf("^1name             model            aifile              funname\n");
	for (i = 0; i < g_numBots; i++) {
		strcpy(name, Info_ValueForKey( g_botInfos[i], "name" ));
		if ( !*name ) {
			strcpy(name, "UnnamedPlayer");
		}
		strcpy(funname, Info_ValueForKey( g_botInfos[i], "funname" ));
		if ( !*funname ) {
			strcpy(funname, "");
		}
		strcpy(model, Info_ValueForKey( g_botInfos[i], "model" ));
		if ( !*model ) {
			strcpy(model, "visor/default");
		}
		strcpy(aifile, Info_ValueForKey( g_botInfos[i], "aifile"));
		if (!*aifile ) {
			strcpy(aifile, "bots/default_c.c");
		}
		trap_Printf(va("%-16s %-16s %-20s %-20s\n", name, model, aifile, funname));
	}
}


/*
===============
G_SpawnBots
===============
*/
static void G_SpawnBots( char *botList, int baseDelay ) {
	char		*bot;
	char		*p;
	float		skill;
	int			delay;
	char		bots[MAX_INFO_VALUE];

	podium1 = NULL;
	podium2 = NULL;
	podium3 = NULL;

	skill = trap_Cvar_VariableValue( "g_spSkill" );
	if( skill < 1 ) {
		trap_Cvar_Set( "g_spSkill", "1" );
		skill = 1;
	}
	else if ( skill > 5 ) {
		trap_Cvar_Set( "g_spSkill", "5" );
		skill = 5;
	}

	Q_strncpyz( bots, botList, sizeof(bots) );
	p = &bots[0];
	delay = baseDelay;
	while( *p ) {
		//skip spaces
		while( *p && *p == ' ' ) {
			p++;
		}
		if( !p ) {
			break;
		}

		// mark start of bot name
		bot = p;

		// skip until space of null
		while( *p && *p != ' ' ) {
			p++;
		}
		if( *p ) {
			*p++ = 0;
		}

		// we must add the bot this way, calling G_AddBot directly at this stage
		// does "Bad Things"

		//PKMOD - Ergodic 01/14/02 - debug bot's restart (inactive)
//		Com_Printf("G_SpawnBots - >%s<\n", bot );

		trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f free %i\n", bot, skill, delay) );

		delay += BOT_BEGIN_DELAY_INCREMENT;
	}
}


/*
===============
G_LoadBotsFromFile
===============
*/
static void G_LoadBotsFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_BOTS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_BOTS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_BOTS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	g_numBots += G_ParseInfos( buf, MAX_BOTS - g_numBots, &g_botInfos[g_numBots] );
}

/*
===============
G_LoadBots
===============
*/
static void G_LoadBots( void ) {
	vmCvar_t	botsFile;
	int			numdirs;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i;
	int			dirlen;

	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	g_numBots = 0;

	trap_Cvar_Register( &botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM );
	if( *botsFile.string ) {
		//PKMOD - Ergodic 01/14/02 - debug bot's restart (inactive)
//		Com_Printf("G_LoadBots - botsFile>%s<\n", botsFile.string );

		G_LoadBotsFromFile(botsFile.string);
	}
	else {
		G_LoadBotsFromFile("scripts/bots.txt");
	}

	// get all bots from .bot files
	numdirs = trap_FS_GetFileList("scripts", ".bot", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadBotsFromFile(filename);
	}
	trap_Printf( va( "%i bots parsed\n", g_numBots ) );
}



/*
===============
G_GetBotInfoByNumber
===============
*/
char *G_GetBotInfoByNumber( int num ) {
	if( num < 0 || num >= g_numBots ) {
		trap_Printf( va( S_COLOR_RED "Invalid bot number: %i\n", num ) );
		return NULL;
	}
	return g_botInfos[num];
}


/*
===============
G_GetBotInfoByName
===============
*/
char *G_GetBotInfoByName( const char *name ) {
	int		n;
	char	*value;

	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		if ( !Q_stricmp( value, name ) ) {
			return g_botInfos[n];
		}
	}

	return NULL;
}

/*
===============
G_InitBots
===============
*/
void G_InitBots( qboolean restart ) {
	int			fragLimit;
	int			timeLimit;
	const char	*arenainfo;
	char		*strValue;
		//PKMOD - Ergodic 11/07/00 - add logic to enable hub limits
	char		*hub_flagValue;
	int			basedelay;
	char		map[MAX_QPATH];
	char		serverinfo[MAX_INFO_STRING];

	G_LoadBots();
	G_LoadArenas();

	trap_Cvar_Register( &bot_minplayers, "bot_minplayers", "0", CVAR_SERVERINFO );

	if( g_gametype.integer == GT_SINGLE_PLAYER ) {
		trap_GetServerinfo( serverinfo, sizeof(serverinfo) );
		Q_strncpyz( map, Info_ValueForKey( serverinfo, "mapname" ), sizeof(map) );
		arenainfo = G_GetArenaInfoByMap( map );
		if ( !arenainfo ) {
			return;
		}

		//PKMOD - Ergodic 11/07/00 - add logic to enable hub limits
		hub_flagValue = Info_ValueForKey( arenainfo, "hub_flag" );

		if ( ! strcmp( hub_flagValue, "1" ) ) {
			strValue = Info_ValueForKey( arenainfo, "hub_fraglimit" );
		}
		else {
			strValue = Info_ValueForKey( arenainfo, "fraglimit" );
		}

		fragLimit = atoi( strValue );
		if ( fragLimit ) {
			trap_Cvar_Set( "fraglimit", strValue );
		}
		else {
			trap_Cvar_Set( "fraglimit", "0" );
		}


		//PKMOD - Ergodic 11/07/00 - add logic to enable hub limits
		if ( ! strcmp( hub_flagValue, "1" ) ) {
			strValue = Info_ValueForKey( arenainfo, "hub_timelimit" );
		}
		else {
			strValue = Info_ValueForKey( arenainfo, "timelimit" );
		}

		timeLimit = atoi( strValue );
		if ( timeLimit ) {
			trap_Cvar_Set( "timelimit", strValue );
		}
		else {
			trap_Cvar_Set( "timelimit", "0" );
		}

		if ( !fragLimit && !timeLimit ) {
			trap_Cvar_Set( "fraglimit", "10" );
			trap_Cvar_Set( "timelimit", "0" );
		}

		basedelay = BOT_BEGIN_DELAY_BASE;
		strValue = Info_ValueForKey( arenainfo, "special" );
		if( Q_stricmp( strValue, "training" ) == 0 ) {
			basedelay += 10000;
		}

		//PKMOD - Ergodic 01/14/02 - debug bot's restart (inactive)
//		Com_Printf("G_InitBots - spawning bots>%s<\n", Info_ValueForKey( arenainfo, "bots" ) );

		if( !restart ) {
			G_SpawnBots( Info_ValueForKey( arenainfo, "bots" ), basedelay );
		}
	}
}
