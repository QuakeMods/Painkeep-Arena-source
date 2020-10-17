// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;
	int			pb;

	//PKMOD - Ergodic 01/18/02 - add a variable to hold whether respawning player is a Private Bot,
	pb = ( g_entities[client->ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) > 1;

			//	if ( g_entities[client->ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) {
//		//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//		Com_Printf("G_WriteClientSessionData - not writing session data for clientnum>%d<\n", client->ps.clientNum );
//		return;
//	}

	s = va("%i %i %i %i %i %i %i %i", 
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.teamLeader,
		//PKMOD - Ergodic 01/18/02 - add a variable to hold whether respawning player is a Private Bot,
		pb
		);

	var = va( "session%i", client - level.clients );

	//PKMOD - Ergodic 02/13/04 - debug bot's restart (inactive)
	//Com_Printf("G_WriteClientSessionData - >%s,%s<\n", var, s  );


	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;

	// bk001205 - format
	int teamLeader;
	int spectatorState;
	int sessionTeam;
	//PKMOD - Ergodic 01/18/02 - add a variable to hold whether respawning player is a Private Bot,
	int sessionPrivateBot;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i",
		&sessionTeam,                 // bk010221 - format
		&client->sess.spectatorTime,
		&spectatorState,              // bk010221 - format
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&teamLeader,                   // bk010221 - format
		//PKMOD - Ergodic 01/18/02 - add a variable to hold whether respawning player is a Private Bot,
		&sessionPrivateBot
		);

	//PKMOD - Ergodic 03/15/04 - debug bot's restart (inactive)
	//Com_Printf("G_ReadClientSessionData - >%s< - - - pb>%d<\n", s, sessionPrivateBot );

	// bk001205 - format issues
	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;
	//PKMOD - Ergodic 01/18/02 - add a variable to hold whether respawning player is a Private Bot,
	client->sess.sessionPrivateBot = (qboolean)sessionPrivateBot;
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean privatebot ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		if ( g_teamAutoJoin.integer ) {
			sess->sessionTeam = PickTeam( -1 );
			BroadcastTeamChange( client, -1 );
		} else {
			// always spawn as spectator in team games
			sess->sessionTeam = TEAM_SPECTATOR;	
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				//PKMOD - Ergodic 02/13/04 - debug adding private bot in tournament game (inactive)
				//Com_Printf("G_InitSessionData - clientnum>%d<n", client->ps.clientNum );
				// if the game is full, go into a waiting mode
				//PKMOD - Ergodic 01/21/02 - ignore adding a Private Bot to tournament game
				if ( ( level.numNonSpectatorClients >= 2 ) && !privatebot ) {
					//PKMOD - Ergodic 01/21/02 - debug adding private bot in tournament game (inactive)
//					Com_Printf("G_InitSessionData - can not player - clientNum>%d<, numNonSpectatorClients>%d<, stat[STAT_PKA_BITS]>%d<\n", client->ps.clientNum, level.numNonSpectatorClients, client->ps.stats[STAT_PKA_BITS] );
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//			Com_Printf("G_WriteSessionData\n" );
//			if ( g_entities[level.clients[i].ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) {
				//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//				Com_Printf("G_WriteSessionData - private bot detected as client>%d<\n", i );
//			}

			//PKMOD - Ergodic 02/14/04 - debug session dat for Private Bot (inactive)
			//		This will prevent Privent Bot from entering a new game
			//if ( !( g_entities[level.clients[i].ps.clientNum].r.svFlags & SVF_PRIVATEBOT ) ) { 
			//	Com_Printf("G_WriteSessionData - NON private bot detected as client>%d<\n", i );
			//}

			G_WriteClientSessionData( &level.clients[i] );

		}
	}
}
