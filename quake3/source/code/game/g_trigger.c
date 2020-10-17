// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


void InitTrigger( gentity_t *self ) {
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	trap_SetBrushModel( self, self->model );
	self->r.contents = CONTENTS_TRIGGER;		// replaces the -1 from trap_SetBrushModel
	self->r.svFlags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger( gentity_t *ent, gentity_t *activator ) {
	ent->activator = activator;
	if ( ent->nextthink ) {
		return;		// can't retrigger until the wait is over
	}

	if ( activator->client ) {
		if ( ( ent->spawnflags & 1 ) &&
			activator->client->sess.sessionTeam != TEAM_RED ) {
			return;
		}
		if ( ( ent->spawnflags & 2 ) &&
			activator->client->sess.sessionTeam != TEAM_BLUE ) {
			return;
		}
	}

	G_UseTargets (ent, ent->activator);

	if ( ent->wait > 0 ) {
		ent->think = multi_wait;
		ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
	} else {
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = 0;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEntity;
	}
}

void Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	multi_trigger( ent, activator );
}

void Touch_Multi( gentity_t *self, gentity_t *other, trace_t *trace ) {
	if( !other->client ) {
		return;
	}
	multi_trigger( self, other );
}

/*QUAKED trigger_multiple (.5 .5 .5) ?
"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
*/
void SP_trigger_multiple( gentity_t *ent ) {
	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( ent->random >= ent->wait && ent->wait >= 0 ) {
		ent->random = ent->wait - FRAMETIME;
		G_Printf( "trigger_multiple has random >= wait\n" );
	}

	ent->touch = Touch_Multi;
	ent->use = Use_Multi;

	InitTrigger( ent );

	//PKMOD - Ergodic 08/02/01 - create a new entity type for trigger_multiple
	//								fixes the bug of dragon deploy hitting doors
	ent->s.eType = ET_TRIGGER_MULTIPLE;

	trap_LinkEntity (ent);
}



/*
==============================================================================

trigger_always

==============================================================================
*/

void trigger_always_think( gentity_t *ent ) {
	G_UseTargets(ent, ent);
	G_FreeEntity( ent );
}

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (gentity_t *ent) {
	// we must have some delay to make sure our use targets are present
	ent->nextthink = level.time + 300;
	ent->think = trigger_always_think;
}


/*
==============================================================================

trigger_push

==============================================================================
*/

void trigger_push_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {

	if ( !other->client ) {
		return;
	}

	BG_TouchJumpPad( &other->client->ps, &self->s );
}


/*
=================
AimAtTarget

Calculate origin2 so the target apogee will be hit
=================
*/
void AimAtTarget( gentity_t *self ) {
	gentity_t	*ent;
	vec3_t		origin;
	float		height, gravity, time, forward;
	float		dist;

	VectorAdd( self->r.absmin, self->r.absmax, origin );
	VectorScale ( origin, 0.5, origin );

	ent = G_PickTarget( self->target );
	if ( !ent ) {
		G_FreeEntity( self );
		return;
	}

	height = ent->s.origin[2] - origin[2];
	gravity = g_gravity.value;
	time = sqrt( height / ( .5 * gravity ) );
	if ( !time ) {
		G_FreeEntity( self );
		return;
	}

	// set s.origin2 to the push velocity
	VectorSubtract ( ent->s.origin, origin, self->s.origin2 );
	self->s.origin2[2] = 0;
	dist = VectorNormalize( self->s.origin2);

	forward = dist / time;
	VectorScale( self->s.origin2, forward, self->s.origin2 );

	self->s.origin2[2] = time * gravity;
}


/*QUAKED trigger_push (.5 .5 .5) ?
Must point at a target_position, which will be the apex of the leap.
This will be client side predicted, unlike target_push
*/
void SP_trigger_push( gentity_t *self ) {
	int		silent_flag;

	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	self->r.svFlags &= ~SVF_NOCLIENT;

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	//PKMOD - Ergodic 11/15/00 add functionality to make trigger_push silent
	//			requested by Mike Reed
	//			co-opt entity variable "count" : 1==silent, 0==normal sound
	//			to make silent use ("silent" "1") in entity definition
	G_SpawnInt( "silent", "0", &silent_flag );
	if ( silent_flag != 0 ) 
		self->s.eType = ET_QUIET_TRIGGER;
	else
		self->s.eType = ET_PUSH_TRIGGER;

	//PKMOD - Ergodic 11/15/00 - debug inactive
//	Com_Printf("SP_trigger_push - self->s.eType>%d<\n" );

	self->touch = trigger_push_touch;
	self->think = AimAtTarget;
	self->nextthink = level.time + FRAMETIME;
	trap_LinkEntity (self);
}


void Use_target_push( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( !activator->client ) {
		return;
	}

	if ( activator->client->ps.pm_type != PM_NORMAL ) {
		return;
	}
	if ( activator->client->ps.powerups[PW_FLIGHT] ) {
		return;
	}

	VectorCopy (self->s.origin2, activator->client->ps.velocity);

	// play fly sound every 1.5 seconds
	if ( activator->fly_sound_debounce_time < level.time ) {
		activator->fly_sound_debounce_time = level.time + 1500;
		G_Sound( activator, CHAN_AUTO, self->noise_index );
	}
}

/*QUAKED target_push (.5 .5 .5) (-8 -8 -8) (8 8 8) bouncepad
Pushes the activator in the direction.of angle, or towards a target apex.
"speed"		defaults to 1000
if "bouncepad", play bounce noise instead of windfly
*/
void SP_target_push( gentity_t *self ) {
	if (!self->speed) {
		self->speed = 1000;
	}
	G_SetMovedir (self->s.angles, self->s.origin2);
	VectorScale (self->s.origin2, self->speed, self->s.origin2);

	if ( self->spawnflags & 1 ) {
		self->noise_index = G_SoundIndex("sound/world/jumppad.wav");
	} else {
		self->noise_index = G_SoundIndex("sound/misc/windfly.wav");
	}
	if ( self->target ) {
		VectorCopy( self->s.origin, self->r.absmin );
		VectorCopy( self->s.origin, self->r.absmax );
		self->think = AimAtTarget;
		self->nextthink = level.time + FRAMETIME;
	}
	self->use = Use_target_push;
}

/*
==============================================================================

trigger_teleport

==============================================================================
*/

void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	gentity_t	*dest;

	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}
	// Spectators only?
	if ( ( self->spawnflags & 1 ) && 
		other->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		return;
	}


	dest = 	G_PickTarget( self->target );
	if (!dest) {
		G_Printf ("Couldn't find teleporter destination\n");
		return;
	}

	TeleportPlayer( other, dest->s.origin, dest->s.angles );
}


/*QUAKED trigger_teleport (.5 .5 .5) ? SPECTATOR
Allows client side prediction of teleportation events.
Must point at a target_position, which will be the teleport destination.

If spectator is set, only spectators can use this teleport
Spectator teleporters are not normally placed in the editor, but are created
automatically near doors to allow spectators to move through them
*/
void SP_trigger_teleport( gentity_t *self ) {
	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	// unless is a spectator trigger
	if ( self->spawnflags & 1 ) {
		self->r.svFlags |= SVF_NOCLIENT;
	} else {
		self->r.svFlags &= ~SVF_NOCLIENT;
	}

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_TELEPORT_TRIGGER;
	self->touch = trigger_teleporter_touch;

	trap_LinkEntity (self);
}

//PKMOD - Ergodic 10/11/00 - hub vote teleport touch
//PKMOD - Ergodic 10/14/00 - update for post vote music
//PKMOD - Ergodic 10/14/00 - postvote thinking will set a delay between server and client
//PKMOD - Ergodic 11/01/00 - pass calling entity into function so that 
//			trap_SendServerCommand can be sent to specific client.
void postvote_music_think ( gentity_t *ent ) {
	char		s[MAX_STRING_CHARS];

	//PKMOD - Ergodic 10/14/00 - add post vote music
	trap_GetConfigstring( CS_POSTVOTE_MUSIC, s, sizeof( s ) );

	//PKMOD - Ergodic 10/14/00 - debug inactive
//	Com_Printf("trigger_hubvote_touch - CS_POSTVOTE_MUSIC>%s<\n", s );

//	trap_SetConfigstring( CS_MUSIC, s );
//	trap_SendServerCommand( -1, va("cs %i %s", CS_POSTVOTE_MUSIC, s) );
//	11/01/00 send to specific voting entity
	trap_SendServerCommand( ent->parent-g_entities, va("cs %i %s", CS_POSTVOTE_MUSIC, s) );

	G_FreeEntity( ent );

}

//PKMOD - Ergodic 10/14/00 - this is done to delay the trap_sendservercomand
//			so that the cgame will have the ps.persistant[PERS_HUB_FLAG] == 1
//          effectively set. 
//PKMOD - Ergodic 11/01/00 - pass calling entity into function so that 
//			trap_SendServerCommand can be sent to specific client.
void spawn_postvote_music ( gentity_t *target_client ) {
	gentity_t	*televote;

	televote = G_Spawn();
	televote->classname = "postvote_music_delay";
	televote->parent = target_client;
	televote->s.eType = ET_GENERAL;
	televote->think = postvote_music_think;
	televote->nextthink = level.time + 2000; //delay 2 second (max lag time = my connection[groan])
}

/*
==============================================================================

trigger_hubvote

==============================================================================
*/

void trigger_hubvote_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	gentity_t	*dest;
//	char		s[MAX_STRING_CHARS];

	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	dest = 	G_PickTarget( self->target );
	if (!dest) {
		G_Printf ("Couldn't find hubvote's teleporter destination\n");
		return;
	}

	//Set the client's voting status
//	other->client->ps.persistant[PERS_HUB_FLAG] = 1;
	//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
	//				Mask off lowest bit {ERROR!}
	//PKMOD - Ergodic 12/23/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
	//				set the lowest bit
	other->client->ps.persistant[PERS_PAINKILLER_COUNT] |= 1;


	//PKMOD - Ergodic 12/01/00 - don't let spectators vote
	// Spectators don't cast a vote
	if ( other->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		//Add to the votes for the particular Map 
		hubInfo[self->count].map_votes++;
	}

	TeleportPlayer( other, dest->s.origin, dest->s.angles );

	//PKMOD - Ergodic 11/01/00 - pass calling entity into function so that 
	//			trap_SendServerCommand can be sent to specific client.
	spawn_postvote_music( other );

	//PKMOD - Ergodic 10/14/00 - add post vote music
//	trap_GetConfigstring( CS_POSTVOTE_MUSIC, s, sizeof( s ) );

	//PKMOD - Ergodic 10/14/00 - debug inactive
//	Com_Printf("trigger_hubvote_touch - CS_POSTVOTE_MUSIC>%s<\n", s );

//	trap_SetConfigstring( CS_MUSIC, s );
//	trap_SendServerCommand( -1, va("cs %i %s", CS_POSTVOTE_MUSIC, s) );

}

//PKMOD - Ergodic 10/11/00 hub vote teleport
//PKMOD - Ergodic 10/12/00 add hub function entity
//PKMOD - Ergodic 03/29/01 - BETA 2.2 code - do not spawn hubvote not in advertised list
/*QUAKED trigger_hubvote (.5 .5 .5) 
Allows client side prediction of hubvote events.
Must point at a target_position, which will be the hubvote teleport destination.
"maptitle"	"enter the title of the map - (for example: Dakyne)"
"mapname"	"file name of map - (for example: pka8)"
"mapindex"	"unique index number for the map - (for example: 8)" 
*/
void SP_trigger_hubvote( gentity_t *self ) {

	char		hold_string[MAX_FILEPATH];
	char		*holdchar;
	int			vote_index;
	gentity_t	*spot;

	//PKMOD - Ergodic 12/23/00 - debug inactive
//	int			hold_hub_flag;
//	const char	*info;


		//PKMOD - Ergodic 12/23/00 - debug inactive
//	Com_Printf( "SP_trigger_hubvote - init...\n" );



	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	// unless is a spectator trigger
	if ( self->spawnflags & 1 ) {
		self->r.svFlags |= SVF_NOCLIENT;
	} else {
		self->r.svFlags &= ~SVF_NOCLIENT;
	}

	if ( Hub_Index >= MAX_HUB_INDEX ) {
		G_Error ( "trigger_hubvote: mapname %s at location:%s exceeds the maximum entity count of MAX_HUB_INDEX(25)\n", self->classname, vtos( self->s.origin ) );
	}

	holdchar = hold_string;
		//verify map name...
	if ( !G_SpawnString( "mapname", "No Map Name", &holdchar ) ) {
		G_Error ( "trigger_hubvote: %s doesn't have a mapname at location:%s\n", self->classname, vtos( self->s.origin ) );
	}

	if ( strlen (holdchar) >= MAX_HUB_NAME ) {
		G_Error ( "trigger_hubvote: mapname %s at location:%s is bigger than MAX_HUB_NAME (70) characters, please reduce filename size\n", self->classname, vtos( self->s.origin ) );
	}

	//Set the file name of the map
	strcpy( hubInfo[Hub_Index].map_name, holdchar );

	//verify map title...
	if ( !G_SpawnString( "maptitle", "No Map Title", &holdchar ) ) {
		G_Error ( "trigger_hubvote: %s doesn't have a maptitle at location:%s\n", self->classname, vtos( self->s.origin ) );
	}

	if ( strlen (holdchar) >= MAX_HUB_NAME  ) {
		G_Error ( "trigger_hubvote: maptitle %s at location:%s is bigger than MAX_HUB_NAME (70) characters, please reduce Map Title size\n", self->classname, vtos( self->s.origin ) );
	}

	//Set the title of the map
	strcpy( hubInfo[Hub_Index].map_title, holdchar );

	//verify map index...
	if ( !G_SpawnInt( "mapindex", "999", &vote_index ) ) {
		G_Error ( "trigger_hubvote: %s doesn't have a mapindex at location:%s\n", self->classname, vtos( self->s.origin ) );
	}

	if ( vote_index > MAX_HUB_INDEX ) {
		G_Error ( "trigger_hubvote: mapindex %s at location:%s exceeds the MAX_HUB_INDEX of %d,  please enter a valid shader index\n", self->classname, vtos( self->s.origin ), MAX_HUB_INDEX );
	}

	//PKMOD - Ergodic 03/29/01 - verify that the mapindex is in the Beta 2.2 "advertised list"
	//				Do not spawn the following maps. Comment out logic for maps that should
	//				be displayed.
	//		Ergodic 04/25/01 - add new maps for the 2.4 patch (3, 11, 16, 19)
	//		Ergodic 06/29/01 - enable all maps for "FULL" release
//	if ( vote_index == 0 )		//Mephisto's Hold
//		return;
//	if ( vote_index == 1 )		//The Prison
//		return;
//	if ( vote_index == 2 )		//DarkStone Temple
//		return;
//	if ( vote_index == 3 )		//Palace of Pain (added 04/25/01)
//		return;
//	if ( vote_index == 4 )		//Obscured
//		return;
//	if ( vote_index == 5 )		//Spears Point
//		return;
//	if ( vote_index == 6 )		//Suicide Staccato
//		return;
//	if ( vote_index == 7 )		//Battered Brains
//		return;
//	if ( vote_index == 8 )		//Dakyne
//		return;
//	if ( vote_index == 9 )		//The Stronghold
//		return;
//	if ( vote_index == 10 )		//Harlequin's Demise
//		return;
//	if ( vote_index == 11 )		//Derelict Point (added 04/25/01)
//		return;
//	if ( vote_index == 12 )		//The Other Side
//		return;
//	if ( vote_index == 13 )		//Techno Mountain
//		return;
//	if ( vote_index == 14 )		//N.N.S.C.
//		return;
//	if ( vote_index == 15 )		//Presence of Death
//		return;
//	if ( vote_index == 16 )		//G1ZA (added 04/25/01)
//		return;
//	if ( vote_index == 17 )		//Scorn
//		return;
//	if ( vote_index == 18 )		//Deguello
//		return;
//	if ( vote_index == 19 )		//PainKeep (added 04/25/01)
//		return;
	
	//Set the index of the map
	hubInfo[Hub_Index].map_shader_index = vote_index;

	//Set the votes of the map
	hubInfo[Hub_Index].map_votes = 0;

	//check for duplicate map indexes
	for (vote_index = 0; vote_index < Hub_Index; vote_index++ ) {
		if ( hubInfo[vote_index].map_shader_index == hubInfo[Hub_Index].map_shader_index ) {
			G_Error ( "trigger_hubvote: duplicate mapindex %d detected at location:%s\n", hubInfo[Hub_Index].map_shader_index, vtos( self->s.origin ), MAX_HUB_INDEX );
			break;
		}
	}

	//Store the hubInfo array index into the entity structure
	self->count = Hub_Index;

	Hub_Index++;		//increment "hub index"


	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_TELEPORT_TRIGGER;
	self->touch = trigger_hubvote_touch;

	trap_LinkEntity (self);


	//determine if hubvote entity is present, if not then spawn a hubvote  
	spot = NULL;
	if ( ( spot = G_Find (spot, FOFS(classname), "func_hubvote")) == NULL ) {
		//if not present, create a new hub_think entity
		//PKMOD - Ergodic 12/23/00 - debug inactive
//		Com_Printf( "SP_trigger_hubvote - calling spawn_hubvote()...\n" );
		spawn_hubvote();
	}

	//PKMOD - Ergodic 12/23/00 - debug inactive
//	hold_hub_flag = atoi( Info_ValueForKey( info, "hub_flag" ) );
//	Com_Printf("SP_trigger_hubvote - hub_flag>%d<, hub_flag.integer<%d<\n", hold_hub_flag, hub_flag.integer );

}

/*
==============================================================================

trigger_hubalternatives
PainKeep Arena dynamic hub - 08/26/03 

==============================================================================
*/

void trigger_hubalternates_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	char	altmap_info1[MAX_HUB_NAME];
	char	altmap_info2[MAX_HUB_NAME];
	char	altmap_info3[MAX_HUB_NAME];
	char	altmap_info4[MAX_HUB_NAME];
	char	buff1[MAX_HUB_NAME];
	char	buff2[MAX_HUB_NAME];

	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	//if already in the alternate hub maps voting entity then exit
	if ( other->client->ps.stats[STAT_PKA_ITEMS] & ( 1 << PKA_HUBALT_VOTING ) )
		return;

	//set the alternate hub maps voting state
	other->client->ps.stats[STAT_PKA_ITEMS] |= ( 1 << PKA_HUBALT_VOTING );

//	trap_SendServerCommand( self->client->ps.clientNum, "hubalternates\n" );
//	trap_SendServerCommand( self->client->ps.clientNum, "hubalternates\n" );
//	trap_SendConsoleCommand( self->client->ps.clientNum, "hubalternates\n" );

	//PKMOD - Ergodic 04/29/04 - Send alternate map info client as arguments
	// g_HubAltMap1 & g_hubAltTitle1
	memset(altmap_info1, 0, sizeof(altmap_info1));
	altmap_info1[0] = '\"';

	memset(buff1, 0, sizeof(buff1));
	trap_Cvar_VariableStringBuffer("g_HubAltMap1", buff1, sizeof(buff1));
	if (buff1[0] != '\0') {
		memset(buff2, 0, sizeof(buff2));
		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( buff2, MAX_HUB_NAME, " - " );

		memset(buff1, 0, sizeof(buff1));
		trap_Cvar_VariableStringBuffer("g_HubAltTitle1", buff1, sizeof(buff1));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( altmap_info1, MAX_HUB_DISPLAY_NAME - 1, buff2 );
	}

	Q_strcat( altmap_info1, MAX_HUB_DISPLAY_NAME, "\"" );

	// g_HubAltMap2 & g_hubAltTitle2
	memset(altmap_info2, 0, sizeof(altmap_info2));
	altmap_info2[0] = '\"';

	memset(buff1, 0, sizeof(buff1));
	trap_Cvar_VariableStringBuffer("g_HubAltMap2", buff1, sizeof(buff1));
	if (buff1[0] != '\0') {
		memset(buff2, 0, sizeof(buff2));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( buff2, MAX_HUB_NAME, " - " );

		memset(buff1, 0, sizeof(buff1));
		trap_Cvar_VariableStringBuffer("g_HubAltTitle2", buff1, sizeof(buff1));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( altmap_info2, MAX_HUB_DISPLAY_NAME - 1, buff2 );
	}

	Q_strcat( altmap_info2, MAX_HUB_DISPLAY_NAME, "\"" );


	// g_HubAltMap2 & g_hubAltTitle2
	memset(altmap_info3, 0, sizeof(altmap_info3));
	altmap_info3[0] = '\"';

	memset(buff1, 0, sizeof(buff1));
	trap_Cvar_VariableStringBuffer("g_HubAltMap3", buff1, sizeof(buff1));
	if (buff1[0] != '\0') {
		memset(buff2, 0, sizeof(buff2));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( buff2, MAX_HUB_NAME, " - " );

		memset(buff1, 0, sizeof(buff1));
		trap_Cvar_VariableStringBuffer("g_HubAltTitle3", buff1, sizeof(buff1));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( altmap_info3, MAX_HUB_DISPLAY_NAME - 1, buff2 );
	}

	Q_strcat( altmap_info3, MAX_HUB_DISPLAY_NAME, "\"" );


	// g_HubAltMap4 & g_hubAltTitle4
	memset(altmap_info4, 0, sizeof(altmap_info4));
	altmap_info4[0] = '\"';

	memset(buff1, 0, sizeof(buff1));
	trap_Cvar_VariableStringBuffer("g_HubAltMap4", buff1, sizeof(buff1));
	if (buff1[0] != '\0') {
		memset(buff2, 0, sizeof(buff2));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( buff2, MAX_HUB_NAME, " - " );

		memset(buff1, 0, sizeof(buff1));
		trap_Cvar_VariableStringBuffer("g_HubAltTitle4", buff1, sizeof(buff1));

		Q_strcat( buff2, MAX_HUB_NAME, buff1 );

		Q_strcat( altmap_info4, MAX_HUB_DISPLAY_NAME - 1, buff2 );
	}

	Q_strcat( altmap_info4, MAX_HUB_DISPLAY_NAME, "\"" );


	//PKMOD - Ergodic 12/09/03 - make it a server command sent to CG	
	//trap_SendConsoleCommand( EXEC_APPEND, "hubalternates\n" );
	//PKMOD - Ergodic 01/05/04 - fix bug in multiplayer mode pass in "other" and not "self"
	//trap_SendServerCommand( other->client->ps.clientNum, "cghubalternates" );
	//PKMOD - Ergodic 04/29/04 - use console command with arguments to send alternate hub choices
	trap_SendServerCommand( other->client->ps.clientNum, va( "cghubalternates %s %s %s %s\n", altmap_info1, altmap_info2, altmap_info3, altmap_info4 ) );
}

//===== PainKeep Arena ========================================================

/*QUAKED trigger_hubalternates (.5 .5 .5) ?
Voting entity for alternate PKA maps that the Server Admin Hosts
Allows client side prediction of teleportation events.
Must point at a target_position, which will be the teleport destination.
*/
void SP_trigger_hubalternates( gentity_t *self ) {

	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	// unless is a spectator trigger
	if ( self->spawnflags & 1 ) {
		self->r.svFlags |= SVF_NOCLIENT;
	} else {
		self->r.svFlags &= ~SVF_NOCLIENT;
	}

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	//PKMOD - Ergodic 09/17/03 - change type from teleport to general
	//self->s.eType = ET_TELEPORT_TRIGGER;
	self->s.eType = ET_GENERAL;
	self->touch = trigger_hubalternates_touch;

	trap_LinkEntity (self);
}


/*
===============
Svcmd_hubaltvoting_f

PKMOD - Ergodic 09/14/03 - enact voting procedure after close of ui selection

===============
*/
void Svcmd_hubaltvoting_f( void ) {
	int				clientnumber;
	int				vote;
	char			string[MAX_TOKEN_CHARS];
	gentity_t		*ent;
	gentity_t		*spawnPoint;
	vec3_t			spawn_origin, spawn_angles;
	char			map_name[MAX_HUB_NAME];
	char			map_title[MAX_HUB_NAME];
	char			*name;
	char			*title;

	int				indx;

	// client number
	trap_Argv( 1, string, sizeof( string ) );
	if ( !string[0] ) {
		trap_Printf( "Usage: hubaltvote <clientnumber> <vote>\n" );
		return;
	}

	clientnumber = atoi( string );

	// get vote from UI selection
	trap_Argv( 2, string, sizeof( string ) );
	if ( !string[0] ) {
		trap_Printf( "Usage: hubaltvote <clientnumber> <vote>\n" );
		return;
	}

	vote = atoi( string );


	//validate vote
	if ( ( vote < -1 )  || ( clientnumber > 3 ) ) {
		trap_Printf( va( "hubaltvote invalid vote >%i<\n", vote ) );
		return;
	}

	//validate clientnumber
	if ( ( clientnumber < 0 )  || ( clientnumber >= level.maxclients ) ) {
		trap_Printf( va( "hubaltvote invalid client number >%i<\n", clientnumber ) );
		return;
	}

	ent = g_entities + clientnumber;

	//validate voter's state - must be in 
	//if already in the alternate hub maps voting entity then exit
	if ( !( ent->client->ps.stats[STAT_PKA_ITEMS] & ( 1 << PKA_HUBALT_VOTING ) ) )
		return;

	//clear the alternate alternate hub maps voting state
	ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_HUBALT_VOTING );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		//Spectators to teleport in postvote area
		spawnPoint = SelectSpawnPoint( ent->client->ps.origin, spawn_origin, spawn_angles, 1 );
	}
	else if ( vote < 0 ) {
		//"no-vote" clients to teleport back to start area
		spawnPoint = SelectSpawnPoint( ent->client->ps.origin, spawn_origin, spawn_angles, 0 );
	}
	else {
		//"voting" clients to teleport in postvote area
		spawnPoint = SelectSpawnPoint( ent->client->ps.origin, spawn_origin, spawn_angles, 1 );
		//Set the client's voting status
		//PKMOD - Ergodic 12/23/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
		//				set the lowest bit
		ent->client->ps.persistant[PERS_PAINKILLER_COUNT] |= 1;

		//Set the map name and map title variables
		name = map_name;
		title = map_title;
		switch (vote) {
			case 0:
				trap_Cvar_VariableStringBuffer("g_HubAltMap1", name, sizeof(map_name));
				trap_Cvar_VariableStringBuffer("g_HubAltTitle1", title, sizeof(map_title));
				break;
			case 1:
				trap_Cvar_VariableStringBuffer("g_HubAltMap2", name, sizeof(map_name));
				trap_Cvar_VariableStringBuffer("g_HubAltTitle2", title, sizeof(map_title));
				break;
			case 2:
				trap_Cvar_VariableStringBuffer("g_HubAltMap3", name, sizeof(map_name));
				trap_Cvar_VariableStringBuffer("g_HubAltTitle3", title, sizeof(map_title));
				break;
			default:
				trap_Cvar_VariableStringBuffer("g_HubAltMap4", name, sizeof(map_name));
				trap_Cvar_VariableStringBuffer("g_HubAltTitle4", title, sizeof(map_title));
		}

		//PKMOD - Ergodic 01/04/04 - determine if map is already a HUB map
		for ( indx = 0; indx < Hub_Index; indx++ ) {
			if ( !strcmp( hubInfo[indx].map_name, name ) ) {
				break;	//found it! Keep value in "indx"
			}
		}

		if ( indx < Hub_Index ) {
			//Here if we found a match within the STANDARD Hub maps
			hubInfo[indx].map_votes++;
			//debug values (inactive)
			//Com_Printf( "Svcmd_hubaltvoting_f - standard Hub map detected,  - vote>%d<, indx>%d<, map>%s<\n", vote, indx, name );
		} else {
			//Here if alternate map is not a STANDARD Hub map

			//debug values (inactive)
			//Com_Printf( "Svcmd_hubaltvoting_f - New Hub map detected,  - vote>%d<, map>%s<\n", vote, name );


			//PKMOD - Ergodic 09/15/03 - store the map name and title if it was not already set
			if ( hubInfo[Hub_Index + vote].map_name[0] == '\0') {
				Q_strncpyz(hubInfo[Hub_Index + vote].map_name, name, MAX_HUB_NAME);
				Q_strncpyz(hubInfo[Hub_Index + vote].map_title, title, MAX_HUB_NAME);
			}

			hubInfo[Hub_Index + vote].map_votes++;
		}

	}

	TeleportPlayer( ent, spawn_origin, spawn_angles );

	//PKMOD - Ergodic 11/01/00 - pass calling entity into function so that 
	//			trap_SendServerCommand can be sent to specific client.
	spawn_postvote_music( ent );

}




//PKMOD - Ergodic 10/11/00 - add custom "message" to trigger_hurt
//			note - nothing needs to be done here to add the message
//			DOES NOT WORK - need to create a server command (ouchies)
/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF - SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.
It does dmg points of damage each server frame
Targeting the trigger will toggle its on / off state.

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( self->r.linked ) {
		trap_UnlinkEntity( self );
	} else {
		trap_LinkEntity( self );
	}
}

void hurt_touch( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int		dflags;

	if ( !other->takedamage ) {
		return;
	}

	if ( self->timestamp > level.time ) {
		return;
	}

	if ( self->spawnflags & 16 ) {
		self->timestamp = level.time + 1000;
	} else {
		self->timestamp = level.time + FRAMETIME;
	}

	// play sound
	if ( !(self->spawnflags & 4) ) {
		G_Sound( other, CHAN_AUTO, self->noise_index );
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	G_Damage (other, self, self, NULL, NULL, self->damage, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt( gentity_t *self ) {
	InitTrigger (self);

	self->noise_index = G_SoundIndex( "sound/world/electro.wav" );
	self->touch = hurt_touch;

	if ( !self->damage ) {
		self->damage = 5;
	}

	self->r.contents = CONTENTS_TRIGGER;

	if ( self->spawnflags & 2 ) {
		self->use = hurt_use;
	}

	// link in to the world if starting active
	if ( ! (self->spawnflags & 1) ) {
		trap_LinkEntity (self);
	}
}


/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
This should be renamed trigger_timer...
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

*/
void func_timer_think( gentity_t *self ) {
	G_UseTargets (self, self->activator);
	// set time before next firing
	self->nextthink = level.time + 1000 * ( self->wait + crandom() * self->random );
}

void func_timer_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	self->activator = activator;

	// if on, turn it off
	if ( self->nextthink ) {
		self->nextthink = 0;
		return;
	}

	// turn it on
	func_timer_think (self);
}

void SP_func_timer( gentity_t *self ) {
	G_SpawnFloat( "random", "1", &self->random);
	G_SpawnFloat( "wait", "1", &self->wait );

	self->use = func_timer_use;
	self->think = func_timer_think;

	if ( self->random >= self->wait ) {
		self->random = self->wait - FRAMETIME;
		G_Printf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ) );
	}

	if ( self->spawnflags & 1 ) {
		self->nextthink = level.time + FRAMETIME;
		self->activator = self;
	}

	self->r.svFlags = SVF_NOCLIENT;
}


//PKMOD - Ergodic 07/16/00 special spawn functions 
/*
	SP_func_lightning_fx
	carried over from Quake 1 PainKeep

	"classname"			- "func_lightning_fx"
	"origin" "x x x"	- Origin of the entity
	"light" "#"			- Brightness of the lightning (100 is pretty good)
	"color"	"#"			- constantLight color default white
	"mean" "#"			- Mean time in seconds of next event (default 15)
	"stdev" "#"			- Variation around the mean time (default 1)
*/

//PKMOD - Ergodic 07/18/00 - add step logic
void func_lightning_fx_think( gentity_t *self ) {
	int		cl;
	int		intensity;
	gentity_t *tent;

	cl = self->AirFist_ResetTime;

//PKMOD Ergodic debug 09/30/01 (inactive)
//Com_Printf("func_lightning_fx_think - level>%d<,  color>%d<\n", self->AirFist_Level, cl );


	//make lightning flash by building up in steps and then stop and then abrupt flash
	switch ( self->AirFist_Level ) {
		case 0:			 //low light
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
//			intensity = ( ( cl >> 24 ) & 255 ) / 3;
			intensity = 255 / 3;
			self->s.constantLight = ( cl & 0xFFFFFF ) | ( intensity << 24 );
			self->AirFist_Level = 1;
			self->nextthink = level.time + 100;
			break;
		case 1:			//medium light
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
//			intensity = 2 * ( ( cl >> 24 ) & 255 ) / 3;
			intensity = 2 * 255 / 3;
			self->s.constantLight = ( cl & 0xFFFFFF ) | ( intensity << 24 );
			self->AirFist_Level = 2;
			self->nextthink = level.time + 100;
			break;
		case 2:			//no light
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
			intensity = 0;
			self->s.constantLight = 0;
			self->AirFist_Level = 3;
			self->nextthink = level.time + 100;
			break;
		case 3:			//full light
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
			intensity = 255;
			self->s.constantLight = cl;
			self->AirFist_Level = 4;
			self->nextthink = level.time + 200;  //more time on full light
			break;
		case 4:			//medium light
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
//			intensity = 2 * ( ( cl >> 24 ) & 255 ) / 3;
			intensity = 2 * 255 / 3;
			self->s.constantLight = ( cl & 0xFFFFFF ) | ( intensity << 24 );
			self->AirFist_Level = 5;  //this will schedule next lightning strike
			self->nextthink = level.time + 100;
			break;
		default:			//no light, schedule next lightning strike
			//PKMOD - Ergodic 09/30/01 - set intensity to 1 byte values
			intensity = 0;
			self->s.constantLight = 0;
			self->AirFist_Level = 0;
			self->nextthink = level.time + 1000 * ( self->wait + ( crandom() * self->random ) );
			break;
	}
	
//PKMOD - Ergodic  09/30/01 - debug (inactive)
//	Com_Printf("func_lightning_fx_think adding event with constantLight>%d<\n", self->s.constantLight );

//PKMOD - Ergodic  09/30/01 - change logic from G_AddEvent to G_TempEntity
//	G_AddEvent( self, EV_LIGHTNING_FX, self->s.constantLight );
	tent = G_TempEntity ( self->s.origin, EV_LIGHTNING_FX );
	tent->s.eventParm = intensity;

	trap_LinkEntity (self);
	
}

//PKMOD - Ergodic 07/16/00 - co-opted variables
//			lightning_state: "AirFist_Level", hold light settings: "AirFist_ResetTime"
//			mean time to next flash: "wait", standard deviation: "random"
//			updated 07/18/00
void SP_func_lightning_fx (gentity_t *ent) {

	float		light;
	vec3_t		color;
	qboolean	lightSet, colorSet;
	int		r, g, b, i;


	//	next lightning fx is mean + crandom() * stdev;
	G_SpawnFloat( "mean", "15", &ent->wait );
	G_SpawnFloat( "stdev", "1", &ent->random );

	//PKMOD - Ergodic 07/17/00 set initial light 
	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );

	//lightning state
	ent->AirFist_Level = 0;

	r = color[0] * 255;
	if ( r > 255 ) {
		r = 255;
	}

	g = color[1] * 255;
	if ( g > 255 ) {
		g = 255;
	}

	b = color[2] * 255;
	if ( b > 255 ) {
		b = 255;
	}

	i = light / 4;
	if ( i > 255 ) {
		i = 255;
	}

	//lightniung flash effects
	ent->s.constantLight = 0;
	ent->AirFist_ResetTime = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );

	ent->think = func_lightning_fx_think;

	ent->nextthink = level.time + FRAMETIME;

	//PKMOD Ergodic debug 09/30/01
Com_Printf("SP_func_lightning_fx - ent->s.origin>%s<\n", vtos(ent->s.origin) );


	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	VectorCopy (ent->s.origin, ent->r.currentOrigin);
	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	ent->r.svFlags = SVF_NOCLIENT;
	ent->s.eType = ET_LIGHTNING_FX;
	trap_LinkEntity (ent);

}

#define GRAVITY_TOGGLE		1
#define GRAVITY_START_OFF	2

//PKMOD - Ergodic 04/20/01 - add use function for toggled gravity
//toggle on/off local gravity only if local gravity is a TOGGLE entity
void Use_Gravity( gentity_t *ent, gentity_t *other, gentity_t *activator ) {

	//PKMOD Ergodic debug 04/20/01 - debug use (inactive)
//	Com_Printf("Use_Gravity -  In : spawnflags>%d<\n", ent->spawnflags );

	if ( ent->spawnflags & GRAVITY_TOGGLE ) {
		if ( ent->spawnflags & GRAVITY_START_OFF ) {
			//if ON the turn OFF
			ent->spawnflags &= ~GRAVITY_START_OFF;
		} else {
			//if OFF then turn ON
			ent->spawnflags |= GRAVITY_START_OFF;
		}
	}
	//PKMOD Ergodic debug 04/20/01 - debug use (inactive)
//	Com_Printf("Use_Gravity -  Out : spawnflags>%d<\n", ent->spawnflags );
}

//PKMOD - Ergodic 04/15/01 - add gravity touch function
void Touch_Gravity (gentity_t *self, gentity_t *other, trace_t *trace ) {

	//PKMOD Ergodic debug 04/20/01 - debug touch (inactive)
//	if ( rand() % 100 > 92 )
//		Com_Printf("Touch_Gravity -  other>%s<, spawnflags>%d<\n", other->classname, self->spawnflags );

//	if ( !other->client ) {
//		return;
//	}

	//PKMOD - Ergodic 04/20/01 - add trigger on/off logic
	if ( !( self->spawnflags & GRAVITY_START_OFF ) ) {
		other->pka_gravity = self->wait;
		other->pka_gravity_time = level.time + 500;
		//PKMOD - Ergodic 04/17/01 - add local gravity
		other->s.pos.trType = TR_LOCAL_GRAVITY;
		other->s.pos.trDuration = self->wait;
	}
}

/*QUAKED func_local_gravity (0 .5 .8) ? TOGGLE START_OFF
//PKMOD - Ergodic 04/15/01 - add PKA custom entity to control gravity
TOGGLE		wait in both the start and end states for a trigger event.
START_OFF	if local gravity is toggled, then start in the off state (default on).
"gravity"		defaults to 800 limits:(0,3000)
*/
void SP_func_local_gravity( gentity_t *self ) {

	InitTrigger (self);

	//PKMOD - Ergodic 04/15/01 - co-opt wait variable
	G_SpawnFloat( "gravity", "800", &self->wait);

	//check limits
	if ( self->wait > 3000 )
		self->wait = 3000;

	if ( self->wait < 0 )
		self->wait = 0;

	//PKMOD Ergodic debug 04/20/01 - debug initialization (inactive)
//	Com_Printf("SP_func_local_gravity - in: spawnflags>%d<\n", self->spawnflags );

	//PKMOD - Ergodic 04/20/01 - add trigger on/off logic
	if (self->spawnflags & GRAVITY_TOGGLE) {
		self->use = Use_Gravity;
	}
	else {
		//turn on gravity always
		self->spawnflags &= ~GRAVITY_START_OFF;
	}

	//PKMOD Ergodic debug 04/20/01 - debug initialization (inactive)
//	Com_Printf("SP_func_local_gravity - out: spawnflags>%d<\n", self->spawnflags );

	self->touch = Touch_Gravity;

	trap_LinkEntity (self);

}
