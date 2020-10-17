// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};

//PKMOD - Ergodic 10/14/00 - qsort primative for hub votes
/*
=============
SortHub

=============
*/
int QDECL SortHub( const void *a, const void *b ) {
	ghubsort_t	*ca, *cb;

	ca = (ghubsort_t *)a;
	cb = (ghubsort_t *)b;

	// then sort by score
	if ( ca->map_votes > cb->map_votes ) {
		return -1;
	}
	if ( ca->map_votes < cb->map_votes ) {
		return 1;
	}
	return 0;
}


//PKMOD - Ergodic 10/12/00 - tally and broadcast hub vote poll
void func_hubvote_think( gentity_t *self ) {
	ghubsort_t	temp_hubInfo[MAX_HUB_INDEX];
	int			indx;
	int			winner;

	gentity_t	*ent;
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			j;
	int			cnt;
	char		*s;
	int			mins, seconds, tens;
	int			msec;

	//PKMOD - Ergodic 10/12/00 - copy hub info to temp hub info
	//PKMOD - Ergodic 09/16/03 - add 4 to Hub_Index for alternate Hub Maps cvars
	for ( indx = 0; indx < ( Hub_Index + 4 ); indx++) {
		temp_hubInfo[indx].map_votes = hubInfo[indx].map_votes;
		temp_hubInfo[indx].hub_index = indx;
	}

	//10/14/00 employ qsort function
	//PKMOD - Ergodic 09/16/03 - add 4 to Hub_Index for alternate Hub Maps cvars
	qsort( temp_hubInfo, Hub_Index + 4, sizeof(temp_hubInfo[0]), SortHub );

	//PKMOD - Ergodic 10/12/00 - debug - hub temp dump inactive
//	for ( indx = 0; indx < Hub_Index; indx++) {
//		Com_Printf("func_hubvote_think - temphub: indx:%d, votes:%d\n", indx, temp_hubInfo[indx].map_votes );
//	}

	//add logic for hubinfo_overlay
	string[0] = 0;
	stringlength = 0;
	cnt = 0;

	//determine first, second, third places in the hub voting 
	//PKMOD - Ergodic 09/16/03 - add 4 to Hub_Index for alternate Hub Maps cvars
	for ( indx = 0; indx < ( Hub_Index + 4 ) && indx < 3; indx++) {
		if ( temp_hubInfo[indx].map_votes > 0 ) {
			Com_sprintf (entry, sizeof(entry), " %i %s ", temp_hubInfo[indx].map_votes, hubInfo[temp_hubInfo[indx].hub_index].map_name );
			j = strlen(entry);
			strcpy (string + stringlength, entry);
			stringlength += j;
			cnt++;
		}
	}

	//11/01/00 add HUB timelimit limit message
	if ( g_hub_timelimit.integer > 0 ) {
		msec = g_hub_timelimit.integer * 60 * 1000 - (level.time - level.startTime);
		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;
		s = va( "%i:%i%i", mins, tens, seconds );

		Com_sprintf (entry, sizeof(entry), " Time: %s ", s );
	}
	else {
		Com_sprintf (entry, sizeof(entry), " Time: N/A " );
	}
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;
	cnt++;

	//add logic for hubinfo_overlay 10/13/00
	for (indx = 0; indx < g_maxclients.integer; indx++) {
		ent = g_entities + indx;
		//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
		if ( ( ent->client->ps.persistant[PERS_PAINKILLER_COUNT] & 1 ) == 1 ) {
			//PKMOD - Ergodic 10/12/00 - debug inactive
//			Com_Printf("func_hubvote_think:>hubinfo_pka %i %s<\n", cnt, string );

			trap_SendServerCommand( ent-g_entities, va("hubinfo_pka %2i  -  %s", cnt, string) );
		}
	}

	//10/14/00 - set the nextmap to leading map
	//Determine if there are any ties
	cnt = 1;
	winner = 0; 
	if ( Hub_Index != 1 ) {
		//PKMOD - Ergodic 09/16/03 - add 4 to Hub_Index for alternate Hub Maps cvars
		//PKMOD - Ergodic 01/05/04 - upon ties only pick the standard hub maps so ...
		//				DO NOT add 4 to Hub_Index
		for ( indx = 1; indx < Hub_Index; indx++ ) {
			if ( temp_hubInfo[0].map_votes != temp_hubInfo[indx].map_votes )
				break;
			cnt++;
		}
		
		if ( cnt > 1 )
			winner = rand() % cnt;
	}

	//PKMOD - Ergodic 10/12/00 - debug inactive
//	Com_Printf("func_hubvote_think - winner:%d\n", winner );
	//PKMOD - Ergodic 12/07/00 - add return to hub functionality
	//PKMOD - Ergodic 12/08/00 - fix hub return functionality
//	trap_Cvar_Set( "nextmap", va("map %s; set nextmap vstr hub", hubInfo[temp_hubInfo[winner].hub_index].map_name ) );
	//PKMOD - Ergodic 01/04/04 - Change hub map name from "pkahub" to hub_30
	trap_Cvar_Set( "hubmap", va("map %s; set nextmap map hub_30", hubInfo[temp_hubInfo[winner].hub_index].map_name ) );
	trap_Cvar_Set( "nextmap", va("vstr hubmap" ) );

	// set time before next firing
	self->nextthink = level.time + 3000;		//every 3 seconds
}

//PKMOD - Ergodic 10/12/00 - spawn tally and broadcast entity
void spawn_hubvote ( void ) {
	gentity_t	*hubvote;

	hubvote = G_Spawn();
	hubvote->classname = "func_hubvote";
	hubvote->nextthink = level.time + 5000;		//first schedule is 5 seconds
	hubvote->think = func_hubvote_think;
	hubvote->r.svFlags = SVF_NOCLIENT;
//PKMOD - Ergodic 10/14/00 - add limit rules...
//		Hub detected so set the fraglimits and timelimits,
//		the cvars hub_fraglimit and hub_timelimit will override
//		the standard cvars fraglimit and timelimit
//	g_fraglimit.integer = g_hub_fraglimit.integer;
//	g_timelimit.integer = g_hub_timelimit.integer;
	//PKMOD - Ergodic 11/07/00 - use hub flag to determine whether in hub or not
	//PKMOD - Ergodic 11/07/00 - set hub flag to one
	//PKMOD - Ergodic 12/25/00 - name change "g_hub_flag" to "hub_flag"
	trap_Cvar_Set( "hub_flag", "1" );

}

	
//PKMOD - Ergodic 10/12/00 - add post vote hub deathmath spawn function
/*QUAKED info_player_postvote (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_postvote( gentity_t *ent ) {
	int			i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}

}


/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}

	//PKMOD - Ergodic 07/25/01 - store the entity into the Respawn Position structure
	Respawn_Positions[ Respawn_Position_Index ] = ent; 
	Respawn_Position_Index++;
	if ( Respawn_Position_Index > MAX_RESPAWN_POSITION_INDEX ) 
		Respawn_Position_Index = MAX_RESPAWN_POSITION_INDEX;
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client) {
			return qtrue;
		}

	}

	return qfalse;
}

//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from, int client_hub_flag ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
	if ( client_hub_flag ) {
		//if client has voted... 
		while ((spot = G_Find (spot, FOFS(classname), "info_player_postvote")) != NULL) {
			VectorSubtract( spot->s.origin, from, delta );
			dist = VectorLength( delta );
			if ( dist < nearestDist ) {
				nearestDist = dist;
				nearestSpot = spot;
			}
		}
	}
	else {
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
			VectorSubtract( spot->s.origin, from, delta );
			dist = VectorLength( delta );
			if ( dist < nearestDist ) {
				nearestDist = dist;
				nearestSpot = spot;
			}
		}
	}

	return nearestSpot;
}


//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( int client_hub_flag ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
	if ( client_hub_flag ) {
		//if client has voted... 
		while ((spot = G_Find (spot, FOFS(classname), "info_player_postvote")) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}
			spots[ count ] = spot;
			count++;
		}
	} else {
		//if client default respawn or not voted... 
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}
			spots[ count ] = spot;
			count++;
		}
	}

	if ( !count ) {	// no spots that won't telefrag
		//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
		if ( client_hub_flag ) {
			return G_Find( NULL, FOFS(classname), "info_player_postvote");
		}
		else {
			return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		}
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomDistantSpawnPoint

Chooses a player start, deathmatch start, etc

PKMOD - Ergodic 05/09/01 - add to reduce spawn fragging
============
*/
#define DISTANTSPAWNPOINTS	6
gentity_t *SelectRandomDistantSpawnPoint ( vec3_t origin ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[DISTANTSPAWNPOINTS];
	gentity_t	*list_spot[DISTANTSPAWNPOINTS];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	for (i = 0; i < DISTANTSPAWNPOINTS; i++)
		list_dist[DISTANTSPAWNPOINTS] = 0;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}

		//store the first location
		if ( !numSpots ) {
			//insert new entry into slot
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
			continue;
		}

		VectorSubtract( spot->s.origin, origin, delta );
		dist = VectorLength( delta );

		for (i = 0; i < numSpots; i++) {
			if ( dist > list_dist[i] ) {
				// found a distant spawn point
				
				//open a slot, move subordinate list down
				//m
				for (j = i + 1; j < numSpots; j++) {
					list_dist[j] = list_dist[j - 1];
					list_spot[j] = list_spot[j - 1];
				}

				//insert new entry into slot
				list_dist[i] = dist;
				list_spot[i] = spot;
			
				numSpots++;
				if (numSpots > DISTANTSPAWNPOINTS)
					numSpots = DISTANTSPAWNPOINTS;

				break; //exit from "for (i" loop
			}
		}
	}

	if (!numSpots) {
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		//PKMOD - Ergodic 05/09/01 - debug respawning in same place (inactive) 
//		Com_Printf("SelectRandomDistantSpawnPoint:!numSpots - location>%s<\n", vtos(spot->s.origin) );
		if (!spot)
			G_Error( "Couldn't find a distant spawn point" );
//		VectorCopy (spot->s.origin, origin);
//		origin[2] += 9;
//		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * numSpots;

	//PKMOD - Ergodic 05/09/01 - debug respawning in same place (inactive)
//	Com_Printf("SelectRandomDistantSpawnPoint - rnd>%d<, location>%s<\n", rnd, vtos(list_spot[rnd]->s.origin) );
//	Com_Printf("SelectRandomDistantSpawnPoint - rnd>%d<\n", rnd );

//	VectorCopy (list_spot[rnd]->s.origin, origin);
//	origin[2] += 9;
//	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectClosestSpawnPoint

Chooses a player start, deathmatch start, etc

Note:	Respawn spot will be selected regardless of telefrag potential
		Respawn spot will be selected regardles of spawn point flags (FL_NO_BOTS, FL_NO_HUMANS) 

PKMOD - Ergodic 01/10/02 - add for Prvate Bot spawninng in DM play only
============
*/
gentity_t *SelectClosestSpawnPoint ( vec3_t target_origin, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		hold_dist;
	gentity_t	*hold_spot;
	int			i;
	qboolean	firstspot = qtrue;

	for ( i = 0; i < Respawn_Position_Index; i++ ) {

		spot = Respawn_Positions[i];

		VectorSubtract( Respawn_Positions[i]->s.origin, target_origin, delta );
		dist = VectorLengthSquared( delta );
		//store the first location
		if ( firstspot ) {
			//insert new entry into slot
			hold_dist = dist;
			hold_spot = Respawn_Positions[i];
			firstspot = qfalse;
			continue;
		}

		if ( dist < hold_dist ) {
			// found a closer spawn point
			hold_dist = dist;
			hold_spot = Respawn_Positions[i];
		}
	}

	VectorCopy (hold_spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (hold_spot->s.angles, angles);

	return hold_spot;
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[64];
	gentity_t	*list_spot[64];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );
		for (i = 0; i < numSpots; i++) {
			if ( dist > list_dist[i] ) {
				if ( numSpots >= 64 )
					numSpots = 64-1;
				for (j = numSpots; j > i; j--) {
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				if (numSpots > 64)
					numSpots = 64;
				break;
			}
		}
		if (i >= numSpots && numSpots < 64) {
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if (!numSpots) {
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			G_Error( "Couldn't find a spawn point" );
		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectRandomDistantSpawnPoint2

Chooses a player start, deathmatch start, etc

PKMOD - Ergodic 07/25/01 - add to reduce spawn fragging
============
*/
#define DISTANTSPAWNPOINTS2	6
gentity_t *SelectRandomDistantSpawnPoint2 ( vec3_t origin ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[DISTANTSPAWNPOINTS2 + 1];
	gentity_t	*list_spot[DISTANTSPAWNPOINTS2 + 1];
	int			numSpots, rnd, i, j, k;

	numSpots = 0;
	spot = NULL;

	for (i = 0; i < DISTANTSPAWNPOINTS2; i++)
		list_dist[DISTANTSPAWNPOINTS2] = 0;

	for ( i = 0; i < Respawn_Position_Index; i++ ) {

		spot = Respawn_Positions[i];

		if ( SpotWouldTelefrag( Respawn_Positions[i] ) ) {
			continue;
		}

		VectorSubtract( Respawn_Positions[i]->s.origin, origin, delta );
		dist = VectorLengthSquared( delta );
		//store the first location
		if ( !numSpots ) {
			//insert new entry into slot
			list_dist[numSpots] = dist;
			list_spot[numSpots] = Respawn_Positions[i];
			numSpots++;
			continue;
		}

		for (j = 0; j < numSpots; j++) {
			if ( dist > list_dist[j] ) {
				// found a distant spawn point

				numSpots++;
				if (numSpots > DISTANTSPAWNPOINTS)
					numSpots = DISTANTSPAWNPOINTS;

				//open a slot, move subordinate list down
				//	move from end of list to start
				for (k = numSpots; k > j; k--) {
					if ( k <= numSpots ) {
						list_dist[k] = list_dist[k - 1];
						list_spot[k] = list_spot[k - 1];
					}
				}

				//insert new entry into slot
				list_dist[j] = dist;
				list_spot[j] = Respawn_Positions[i];
			

				break; //exit from "for (j" loop
			}
		}
	}

	if (!numSpots) {
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		//PKMOD - Ergodic 05/09/01 - debug respawning in same place (inactive) 
//		Com_Printf("SelectRandomDistantSpawnPoint:!numSpots - location>%s<\n", vtos(spot->s.origin) );
		if (!spot)
			G_Error( "Couldn't find a distant spawn point" );
//		VectorCopy (spot->s.origin, origin);
//		origin[2] += 9;
//		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * numSpots;

	//PKMOD - Ergodic 05/09/01 - debug respawning in same place (inactive)
//	Com_Printf("SelectRandomDistantSpawnPoint - rnd>%d<, location>%s<\n", rnd, vtos(list_spot[rnd]->s.origin) );
//	Com_Printf("SelectRandomDistantSpawnPoint - rnd>%d<\n", rnd );

//	VectorCopy (list_spot[rnd]->s.origin, origin);
//	origin[2] += 9;
//	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}


//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, int client_hub_flag ) {
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	//PKMOD - Ergodic 05/10/01 - add failsafe for initial spawns
	int			spawn_count;

	//PKMOD - Ergodic 05/10/01 - debug initial spawn logic (inactive)
//	Com_Printf("SelectSpawnPoint - avoidPoint>%s<, origin>%s<, client_hub_flag>%d<\n", vtos(avoidPoint), vtos(origin), client_hub_flag );

	//PKMOD - Ergodic 05/09/01 - add distant spawn logic
	if ( client_hub_flag ) { 
		//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
		nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint, client_hub_flag );

		//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
		spot = SelectRandomDeathmatchSpawnPoint ( client_hub_flag );
		if ( spot == nearestSpot ) {
			// roll again if it would be real close to point of death
			//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
			spot = SelectRandomDeathmatchSpawnPoint ( client_hub_flag );
			if ( spot == nearestSpot ) {
				// last try
				//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
				spot = SelectRandomDeathmatchSpawnPoint ( client_hub_flag );
			}		
		}
	}
	else {
		//PKMOD - Ergodic 07/24/01 - DEBUG the quake3 acess violation failure
		//		do not call distant respawn code
//		spot = SelectRandomDistantSpawnPoint ( avoidPoint );
		//PKMOD - Ergodic 07/25/01 - try new optimized distant respawn code
//		spot = SelectRandomDeathmatchSpawnPoint ( client_hub_flag );
		spot = SelectRandomDistantSpawnPoint2 ( avoidPoint );

		//PKMOD - Ergodic 05/10/01 - add failsafe for initial spawns
		if ( !spot ) {
			spawn_count = 3;
			while ( ( !spot ) && (spawn_count > 0) ) { 
				spot = SelectRandomDeathmatchSpawnPoint ( client_hub_flag );
				spawn_count--;
			}
		}
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "SelectSpawnPoint: Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, int client_hub_flag ) {
	gentity_t	*spot;

	spot = NULL;
	//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
	if ( client_hub_flag ) {
		//if client has voted... 
		while ((spot = G_Find (spot, FOFS(classname), "info_player_postvote")) != NULL) {
			if ( spot->spawnflags & 1 ) {
				break;
			}
		}
	} else {
		//if client default respawn or not voted... 
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
			if ( spot->spawnflags & 1 ) {
				break;
			}
		}
	}

	//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles, client_hub_flag );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;	
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
#ifdef MISSIONPACK
	gentity_t	*e;
	int i;
#endif
	gentity_t		*body;
	int			contents;

	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	//PKMOD - Ergodic 12/29/00 - debug surface flags inactive 
//	Com_Printf("CopyToBodyQue - contents>%d<\n", contents );

	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity (body);

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
#ifdef MISSIONPACK
	if ( ent->s.eFlags & EF_KAMIKAZE ) {
		body->s.eFlags |= EF_KAMIKAZE;

		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			e = &g_entities[i];
			if (!e->inuse)
				continue;
			if (e->activator != ent)
				continue;
			if (strcmp(e->classname, "kamikaze timer"))
				continue;
			e->activator = body;
			break;
		}
	}
#endif
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	gentity_t	*tent;

	CopyToBodyQue (ent);
	ClientSpawn(ent);

	// add a teleportation effect
	tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	tent->s.clientNum = ent->s.clientNum;
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			if ( level.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int		len, colorlessLen;
	char	ch;
	char	*p;
	int		spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while( 1 ) {
		ch = *in++;
		if( !ch ) {
			break;
		}

		// don't allow leading spaces
		if( !*p && ch == ' ' ) {
			continue;
		}

		// check colors
		if( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if( !*in ) {
				break;
			}

			// don't allow black in a name, period
			if( ColorIndex(*in) == 0 ) {
				in++;
				continue;
			}

			// make sure room in dest for both chars
			if( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if( ch == ' ' ) {
			spaces++;
			if( spaces > 3 ) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "UnnamedPlayer", outSize );
	}
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	int		teamTask, teamLeader, team, health;
	char	*s;
	char	model[MAX_QPATH];
	char	headModel[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	gclient_t	*client;
	char	c1[MAX_INFO_STRING];
	char	c2[MAX_INFO_STRING];
	char	redTeam[MAX_INFO_STRING];
	char	blueTeam[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];
	//PKMOD - Ergodic 01/09/02 - create a holder for private bot
	char	pb[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		strcpy (userinfo, "\\name\\badinfo");
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// set name
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname) );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, 
				client->pers.netname) );
		}
	}

	// set max health
#ifdef MISSIONPACK
	if (client->ps.powerups[PW_GUARD]) {
		client->pers.maxHealth = 200;
	} else {
		health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
		client->pers.maxHealth = health;
		if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
			client->pers.maxHealth = 100;
		}
	}
#else
	health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
#endif
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// set model
	if( g_gametype.integer >= GT_TEAM ) {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "team_model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "team_headmodel"), sizeof( headModel ) );
	} else {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "headmodel"), sizeof( headModel ) );
	}

	// bots set their team a few frames later
	if (g_gametype.integer >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT) {
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}
	}
	else {
		team = client->sess.sessionTeam;
	}

/*	NOTE: all client side now

	// team
	switch( team ) {
	case TEAM_RED:
		ForceClientSkin(client, model, "red");
//		ForceClientSkin(client, headModel, "red");
		break;
	case TEAM_BLUE:
		ForceClientSkin(client, model, "blue");
//		ForceClientSkin(client, headModel, "blue");
		break;
	}
	// don't ever use a default skin in teamplay, it would just waste memory
	// however bots will always join a team but they spawn in as spectator
	if ( g_gametype.integer >= GT_TEAM && team == TEAM_SPECTATOR) {
		ForceClientSkin(client, model, "red");
//		ForceClientSkin(client, headModel, "red");
	}
*/

#ifdef MISSIONPACK
	if (g_gametype.integer >= GT_TEAM) {
		client->pers.teamInfo = qtrue;
	} else {
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( ! *s || atoi( s ) != 0 ) {
			client->pers.teamInfo = qtrue;
		} else {
			client->pers.teamInfo = qfalse;
		}
	}
#else
	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if ( ! *s || atoi( s ) != 0 ) {
		client->pers.teamInfo = qtrue;
	} else {
		client->pers.teamInfo = qfalse;
	}
#endif
	/*
	s = Info_ValueForKey( userinfo, "cg_pmove_fixed" );
	if ( !*s || atoi( s ) == 0 ) {
		client->pers.pmoveFixed = qfalse;
	}
	else {
		client->pers.pmoveFixed = qtrue;
	}
	*/

	// team task (0 = none, 1 = offence, 2 = defence)
	teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	teamLeader = client->sess.teamLeader;

	// colors
	strcpy(c1, Info_ValueForKey( userinfo, "color1" ));
	strcpy(c2, Info_ValueForKey( userinfo, "color2" ));

	strcpy(redTeam, Info_ValueForKey( userinfo, "g_redteam" ));
	strcpy(blueTeam, Info_ValueForKey( userinfo, "g_blueteam" ));

	//PKMOD - Ergodic 01/09/02 - set holder for private bot
	strcpy(pb, Info_ValueForKey( userinfo, "privateBot" ));

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if ( ent->r.svFlags & SVF_BOT ) {
		//PKMOD - Ergodic 01/09/02 - update for Private Bot (pb) value
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d\\pb\\%s",
			client->pers.netname, team, model, headModel, c1, c2, 
			client->pers.maxHealth, client->sess.wins, client->sess.losses,
			Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader, pb );

			//PKMOD - Ergodic 01/10/02 - debug bot's userinfo (inactive)
//			Com_Printf("ClientUserinfoChanged - userinfo>%s<\n", userinfo);
//			Com_Printf("ClientUserinfoChanged - s>%s<\n", s);
	} else {
		//PKMOD - Ergodic 01/09/02 - update for Private Bot (pb) value
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d\\pb\\0",
			client->pers.netname, client->sess.sessionTeam, model, headModel, redTeam, blueTeam, c1, c2, 
			client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader);
	}

	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
//	char		*areabits;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

 	// IP filtering
 	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
 	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
 	// check to see if they are on the banned IP list
	//PKMOD - Ergodic 01/09/02 - debug bot's userinfo (inactive)
//	Com_Printf("ClientConnect - userinfo>%s<\n", userinfo);

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if ( G_FilterPacket( value ) ) {
		return "You are banned from this server.";
	}

  // we don't check password for bots and local client
  // NOTE: local client <-> "ip" "localhost"
  //   this means this client is not running in our current process
	if ( !( ent->r.svFlags & SVF_BOT ) && (strcmp(value, "localhost") != 0)) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			return "Invalid password";
		}
	}

	//PKMOD - Ergodic 10/21/02 - prevent client connecting as a Private Bot
	if ( !isBot ) {
		ent->r.svFlags &= ~SVF_BOT;
		//PKMOD - Ergodic 10/27/02 - do both BOT settings, now SVF_PRIVATEBOT...
		ent->r.svFlags &= ~SVF_PRIVATEBOT;
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

//	areabits = client->areabits;

	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		//PKMOD - Ergodic 01/21/02 - modify call to pass in Private Bot status
		G_InitSessionData( client, userinfo, ent->r.svFlags & SVF_PRIVATEBOT );
	}

	G_ReadSessionData( client );

	/*
	//PKMOD - Ergodic 02/13/04 - test private bot re-entry into map
	if ( client->sess.sessionPrivateBot ) {
		//PKMOD - Ergodic 02/19/04 - no need to drop the client - just returning with text will remove Private Bot
		//		NOTE: This code is here for failsafe and will not execute since Private Bot
		//				is removed in g_main.c : G_ShutdownGame.
//PKMOD - Ergodic 03/15/04 - debug bot's restart
Com_Printf("ClientConnect - (2) returned after private bot session detected\n" );
		return "Private Bot is removed for Basic Training";		//02/20/04
	}
	*/

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	// get and distribute relevent paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
	}

	if ( g_gametype.integer >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}

	//PKMOD - Ergodic 01/24/02 - debug bot's status bits (inactive)
//	Com_Printf("ClientConnect - before CalculateRanks\n" );

	// count current clients and rank for scoreboard
	CalculateRanks();

	//PKMOD - Ergodic 01/24/02 - debug bot's status bits (inactive)
//	Com_Printf("ClientConnect - after CalculateRanks\n" );

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

	return NULL;
}

//PKMOD - Ergodic 10/11/00 - update client PERS with voting information (set to zero)
/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	gentity_t	*tent;
	int			flags;

	//PKMOD - Ergodic 02/19/04 - debug bot's restart (inactive)
	//Com_Printf("ClientBegin - client>%d<, svFlags>%d<, PrivateBot?>%d<\n", clientNum, g_entities[clientNum].r.svFlags, g_entities[clientNum].r.svFlags & SVF_PRIVATEBOT );


	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	//PKMOD - Ergodic 01/15/02 - debug bot's restart (inactive)
//	Com_Printf("ClientBegin - persistant[PERS_KILLED]>%d<\n", client->ps.persistant[PERS_KILLED] );

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// locate ent at a spawn point
	ClientSpawn( ent );

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		// send event
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;

		if ( g_gametype.integer != GT_TOURNAMENT  ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

	//PKMOD - Ergodic 10/11/00 - set the hub voting flag
	//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
	//				Mask off lowest bit
	client->ps.persistant[PERS_PAINKILLER_COUNT] &= 0x7FFFFFFE;

	//PKMOD - Ergodic 01/18/02 - drop Private Bot upon entry
	//PKMOD - Ergodic 12/19/02 - emove this code - code will cause PB to not initialize
	//			properly in Online game.
	//			This removal code will be executed int he shutdown function
	//if ( client->sess.sessionPrivateBot ) {
		//PKMOD - Ergodic 01/09/02 - debug bot's userinfo (inactive)
//		Com_Printf("ClientBegin - dropping client>%d<\n", clientNum );
	//	trap_DropClient( clientNum, "Private Bot returning to base" );
	//}

}

//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
//	int		savedEvents[MAX_PS_EVENTS];		//PKMOD -Ergodic 12/19/02 - unsed variable - Unlagged?
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = ent - g_entities;
	client = ent->client;

	//PKMOD - Ergodic 01/17/00 - if Bot in HUB then force postvote respawn
	//			Bots do not navigate the hub very well, bots usually fight instead of vote
	//			we will just place them in the postvote area as a default and leave the voting
	//			to the human players
	if ( ( ent->r.svFlags & SVF_BOT ) && ( hub_flag.integer == 1 ) ) {
		client->ps.persistant[PERS_PAINKILLER_COUNT] |= 1;
	}

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
			spawn_origin, spawn_angles);
	} else if (g_gametype.integer == GT_CTF) {
		//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
		//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
		spawnPoint = SelectCTFSpawnPoint ( 
			client->sess.sessionTeam, 
			client->pers.teamState.state, 
			spawn_origin, spawn_angles, ( client->ps.persistant[PERS_PAINKILLER_COUNT] & 1 ) );
	} else {
		do {
			//PKMOD - Ergodic 01/10/02 - spawn Private Bot close to owner (DM only feature)
			if ( ent->r.svFlags & SVF_PRIVATEBOT ) {
				spawnPoint = SelectClosestSpawnPoint( ent->parent->r.currentOrigin, spawn_origin, spawn_angles );
				break;
			}
			else {
				// the first spawn should be at a good looking spot
				if ( !client->pers.initialSpawn && client->pers.localClient ) {
					client->pers.initialSpawn = qtrue;
					//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
					//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles, ( client->ps.persistant[PERS_PAINKILLER_COUNT] & 1 ) );
				} else {
					// don't spawn near existing origin if possible
					//PKMOD - Ergodic 10/12/00 - add "client_hub_flag" argument - 1:voted, 0:default/not-voted
					//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
					spawnPoint = SelectSpawnPoint ( client->ps.origin, spawn_origin, spawn_angles, ( client->ps.persistant[PERS_PAINKILLER_COUNT] & 1 ) );
				}
			}
			// Tim needs to prevent bots from spawning at the initial point
			// on q3dm0...
			if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}
			// just to be symetric, we have a nohumans option...
			if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}

			break;

		} while ( 1 );
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	//PKMOD Ergodic - 07/08/01, remove kamizaki entity flag
	// always clear the kamikaze flag
//	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	memset (client, 0, sizeof(*client)); // bk FIXME: Com_Memset?

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );

	//PKMOD - Ergodic 01/09/02 - debug bot's userinfo (inactive)
//	Com_Printf("ClientSpawn - userinfo>%s<\n", userinfo);

	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	//PKMOD - Ergodic - 03/29/2000, set the respawn values for AirFist
//	ent->AirFist_Level = 100;
	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	ent->client->ps.stats[STAT_AIRFIST_LEVEL] = 4;

	//PKMOD - Ergodic - 09/09/2000, set the respawn values for Lightning_Strike
	//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
	//			set all flags to zero
//	ent->Lightning_Strike = 0;
	ent->pka_flags = 0;

	//PKMOD - Ergodic - 05/26/2000, set the respawn values for BearTrap
	//PKMOD Ergodic - 04/07/01 - rename variable to add autosentry
	ent->BearTrap_Autosentry_ImmuneTime = level.time + 2000;  // 2 seconds of immunity, quick run!

	//PKMOD - Ergodic 06/19/00 set the pka_items flag to 0
	ent->client->ps.stats[STAT_PKA_ITEMS] = 0;

	//PKMOD - Ergodic 06/30/00 - reset counter variable for number of beartraps attached to the player
	ent->client->ps.stats[STAT_BEARTRAPS_ATTACHED] = 0;

	//PKMOD - Ergodic 03/13/01 - clear the dragon deploy variable
	ent->client->ps.generic1 = 0;

	//PKMOD - Ergodic 04/16/01 - reset special gravity
	ent->pka_gravity_time = 0;
	ent->pka_gravity = g_gravity.value;

	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;

	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_MACHINEGUN );

	//PKMOD - Ergodic 12/02/01 - test bots using grapple - TO BE REMOVED LATER
	//PKMOD - Ergodic 08/06/02 - remove automatic Dragon from strtup inventory
//	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_MACHINEGUN ) | ( 1 << WP_GRAPPLING_HOOK );

	if ( g_gametype.integer == GT_TEAM ) {
		client->ps.ammo[WP_MACHINEGUN] = 50;
	} else {
		client->ps.ammo[WP_MACHINEGUN] = 100;
	}

	client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET );

//PKMOD -Enable Grapple. This version automatically gives you the grapple (disabled 07/05/00)
//	client->ps.stats[STAT_WEAPONS] |=	( 1 << WP_GRAPPLING_HOOK);
//PKMOD - Ergodic 05/17/00 - remove automatic gravity well (disabled 07/05/00)
//	client->ps.stats[STAT_WEAPONS] |=	( 1 << WP_GRAVITY);
	//PKMOD

	client->ps.ammo[WP_GAUNTLET] = -1;
	client->ps.ammo[WP_GRAPPLING_HOOK] = -1;

	// health will count down towards max_health
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;

	//PKMOD - Ergodic 01/07/02 - Private Bot will be enhanced
	if ( ent->r.svFlags & SVF_PRIVATEBOT ) {
		//max out health
		ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 2;
		//max out armor
		client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_HEALTH] * 2;
		//give some weapon(s)
		//PKMOD - Ergodic 01/24/02 - add to inventory with |= not set with =
		client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GRAPPLING_HOOK );

		//give 1 high-powered weapon
		//PKMOD - Ergodic 01/05/03 - remove rail from PB selection
		//			Scale selection so that HIGH powered weapons are less frequently
		//			obtained than lower powered weapons
		//	Nailgun ~50%
		//  Rocket Launcher ~30%
		//  Lightning Gun ~20%
		i = rand() % 100; //0,1,2...97,98,99
		if ( i < 50 ) {
			//Give a Nail Gun
			client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_NAILGUN );
			client->ps.ammo[WP_NAILGUN] = 135;
		} else if ( i < 80 ) {
				//Give a Rocket Launcher
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_ROCKET_LAUNCHER );
				client->ps.ammo[WP_ROCKET_LAUNCHER] = 25;
		} else {
				//Give a Lightning Gun
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_LIGHTNING );
				client->ps.ammo[WP_LIGHTNING] = 180;
		}
	}

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {

	} else {
		G_KillBox( ent );
		trap_LinkEntity (ent);

		// force the base weapon up
		client->ps.weapon = WP_MACHINEGUN;
		client->ps.weaponstate = WEAPON_READY;

	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
	client->ps.torsoAnim = TORSO_STAND;
	client->ps.legsAnim = LEGS_IDLE;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		G_UseTargets( spawnPoint, ent );

		// select the highest weapon number available, after any
		// spawn given items have fired
		client->ps.weapon = 1;
		//PKMOD -Enable Grapple. This version automatically gives you the grapple
		// -1 changed to -2 in "for ( i = WP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {"
		for ( i = WP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {
			if ( client->ps.stats[STAT_WEAPONS] & ( 1 << i ) ) {
				client->ps.weapon = i;
				break;
			}
		}
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
#ifdef MISSIONPACK
		TossClientPersistantPowerups( ent );
		if( g_gametype.integer == GT_HARVESTER ) {
			TossClientCubes( ent );
		}
#endif

	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	//PKMOD - Ergodic 01/22/02 - debug disconnecting private bot in tournament game (inactive)
//	Com_Printf( "ClientDisconnect: clientNum>%d<, svFlags>%d<\n", clientNum, ent->r.svFlags );
//	if ( ent->r.svFlags & SVF_PRIVATEBOT )
//		Com_Printf( "ClientDisconnect: private bot disconnecting clientNum>%d<\n", clientNum );
//	else
//		Com_Printf( "ClientDisconnect: NOT private bot disconnecting clientNum>%d<\n", clientNum );



	//PKMOD - Ergodic 02/11/04 - if cvar is set then do not count leaving the game as a win for the
	//		other player
	if ( g_pkatourneyrules.integer == 0 ) {
		// if we are playing in tourney mode and losing, give a win to the other player
		if ( (g_gametype.integer == GT_TOURNAMENT )
			&& !level.intermissiontime
			&& !level.warmupTime && ( level.sortedClients[1] == clientNum ) &&
			//PKMOD - Ergodic 01/22/02 - disregard Private Bot disconnects in tournament play
			!( ent->r.svFlags & SVF_PRIVATEBOT ) ) {
			level.clients[ level.sortedClients[0] ].sess.wins++;
			ClientUserinfoChanged( level.sortedClients[0] );
		}
	}

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}
}

