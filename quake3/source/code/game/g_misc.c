// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_misc.c

#include "g_local.h"

//PKMOD - Ergodic 06/07/03 - debug shooter_beartrap direction
char	*alt_vtos( const vec3_t v );



/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}



/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	AngleVectorsForward( angles, player->client->ps.velocity );
	VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
	player->client->ps.pm_time = 160;		// hold time
	player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	//PKMOD - Ergodic - 05/26/2000, set the respawn values for BearTrap
	//PKMOD Ergodic - 04/07/01 - rename variable to add autosentry
	player->BearTrap_Autosentry_ImmuneTime = level.time + 2000;  // 2 seconds of immunity, quick run!

	//PKMOD - Ergodic 03/09/01 - if player is using the dragon then turn it off
	if ( player->client->hook ) {
		Weapon_HookFree( player->client->hook );
	}


	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		trap_LinkEntity (player);
	}

}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) {
	vec3_t		dir;
	gentity_t	*target;
	gentity_t	*owner;

	owner = G_PickTarget( ent->target );
	if ( !owner ) {
		G_Printf( "Couldn't find target for misc_partal_surface\n" );
		G_FreeEntity( ent );
		return;
	}
	ent->r.ownerNum = owner->s.number;

	// frame holds the rotate speed
	if ( owner->spawnflags & 1 ) {
		ent->s.frame = 25;
	} else if ( owner->spawnflags & 2 ) {
		ent->s.frame = 75;
	}

	// swing camera ?
	if ( owner->spawnflags & 4 ) {
		// set to 0 for no rotation at all
		ent->s.powerups = 0;
	}
	else {
		ent->s.powerups = 1;
	}

	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy( owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	target = G_PickTarget( owner->target );
	if ( target ) {
		VectorSubtract( target->s.origin, owner->s.origin, dir );
		VectorNormalize( dir );
	} else {
		G_SetMovedir( owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) {
	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if ( !ent->target ) {
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} else {
		ent->think = locateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/
//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t		dir;
	float		deg;
	vec3_t		up, right;
	//PKMOD - Ergodic 12/06/00 - for lightning shooter
	vec3_t		end;
	//PKMOD - Ergodic 04/16/03 - speed for beartrap shooter
	int			speedint;


	//PKMOD Ergodic 04/16/03 - debug activator (inactive)
	//Com_Printf("Use_Shooter ent->classname>%s<, other->classname>%s<, activator->classname>%s<\n", ent->classname, other->classname, activator->classname  );


	//PKMOD - Ergodic 11/21/00 - co-opt AirFist_Level for player targeting.
	//			player targeting will override other targeting
	if ( ent->AirFist_Level && activator->client ) {
		//PKMOD Ergodic 12/06/00 - debug inactive
//		Com_Printf("Use_Shooter - airfistlevel\n" );

//		VectorCopy( activator->r.currentOrigin, player_location );
//		player_location[2] = self->client->ps.viewheight * 0.75
		VectorSubtract( activator->r.currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
		//PKMOD - Ergodic 12/06/00 - for lightning shooter
		VectorCopy( activator->r.currentOrigin, end );
	}
	else {
		// see if we have a target
		if ( ent->enemy ) {
			//PKMOD Ergodic 12/06/00 - debug inactive
//			Com_Printf("Use_Shooter - ent->enemy\n" );
			VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
			VectorNormalize( dir );
			//PKMOD - Ergodic 12/06/00 - for lightning shooter
			VectorCopy( ent->enemy->r.currentOrigin, end );
		} else {
			//PKMOD Ergodic 12/06/00 - debug inactive
//			Com_Printf("Use_Shooter - NOT ent->enemy\n" );
			VectorCopy( ent->movedir, dir );
			//PKMOD - Ergodic 12/06/00 - enemy has not spawned yet, then target self
			VectorCopy( ent->r.currentOrigin, end );
		}
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) {
	case WP_GRENADE_LAUNCHER:
		fire_grenade( ent, ent->s.origin, dir );
		break;
	case WP_ROCKET_LAUNCHER:
		fire_rocket( ent, ent->s.origin, dir );
		break;
	case WP_PLASMAGUN:
		fire_plasma( ent, ent->s.origin, dir );
		break;
	//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
	case WP_SHOTGUN:
		shooter_supershotgun_fire( ent, ent->s.origin, dir );
		break;
	//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
	case WP_RAILGUN:
		shooter_railgun_fire( ent, ent->s.origin, dir );
		break;
	//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
	case WP_MACHINEGUN:
		shooter_bullet_fire( ent, ent->s.origin, dir );
		break;
	//PKMOD - Ergodic 12/05/00 - add new types of PKA shooters
	case WP_LIGHTNING:
		//PKMOD Ergodic 12/06/00 - debug inactive
//		Com_Printf("Use_Shooter - targetname>%s<\n", ent->targetname );

		shooter_Lightning_fire( ent, ent->s.origin, end );
		break;

	//PKMOD - Ergodic 04/16/03 - add beartrap shooter
	case WP_BEARTRAP:

		speedint = ((int)(ent->speed));
		//if beartrap speed is wacky then set to default;
		if ( speedint <= 0 || speedint >= 2000 )
			speedint = 400;

		//PKMOD Ergodic 06/07/03 - debug speed (inactive)
		//Com_Printf("Use_Shooter speed>%d<, dir>%s<\n", speedint, alt_vtos( dir ) );

		//PKMOD - Ergodic 06/17/03 - use flag for frag credit
		if ( ent->pka_flags == PKAEF_DOORADDFRAG )
			fire_beartrap( activator, ent->s.origin, dir, speedint );	//activator is the BearTrap Owner 
		else
			fire_beartrap( ent, ent->s.origin, dir, speedint );		//World is the BearTrap Owner

		break;

	}

	//PKMOD Ergodic 12/07/00 - don't fire lightning
	if ( ent->s.weapon != WP_LIGHTNING)
		G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
	ent->enemy = G_PickTarget( ent->target );
	ent->think = 0;
	ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
	//PKMOD - Ergodic 06/07/03 - add holder for addfrag  
	int		hold_addfrag;

	ent->use = Use_Shooter;
	ent->s.weapon = weapon;

	//PKMOD - Ergodic 11/21/00 - co-opt AirFist_Level for player targeting 
	G_SpawnInt( "enemy", "0", &ent->AirFist_Level );

	//PKMOD - Ergodic 04/16/03 - set the speed of the shooter_Beartrap
	if ( weapon == WP_BEARTRAP ) {
		G_SpawnFloat( "speed", "400", &ent->speed );

		//PKMOD - Ergodic 06/07/03 - set the appropriate value for the flag  
		G_SpawnInt( "addfrag", "0", &hold_addfrag );
		if ( hold_addfrag == 1 ) 
			ent->pka_flags = PKAEF_DOORADDFRAG;  //Note: this will be a co-opted variable
		else
			ent->pka_flags = 0;
	}

	RegisterItem( BG_FindItemForWeapon( weapon ) );

	G_SetMovedir( ent->s.angles, ent->movedir );

	if ( !ent->random ) {
		ent->random = 1.0;
	}
	ent->random = sin( M_PI * ent->random / 180 );
	// target might be a moving object, so we can't set movedir for it
	if ( ent->target ) {
		ent->think = InitShooter_Finish;
		ent->nextthink = level.time + 500;
	}
	trap_LinkEntity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) {
	InitShooter( ent, WP_ROCKET_LAUNCHER );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) {
	InitShooter( ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) {
	InitShooter( ent, WP_GRENADE_LAUNCHER);
}

#ifdef MISSIONPACK
static void PortalDie (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod) {
	G_FreeEntity( self );
	//FIXME do something more interesting
}


void DropPortalDestination( gentity_t *player ) {
	gentity_t	*ent;
	vec3_t		snapped;

	// create the portal destination
	ent = G_Spawn();
	ent->s.modelindex = G_ModelIndex( "models/powerups/teleporter/tele_exit.md3" );

	VectorCopy( player->s.pos.trBase, snapped );
	SnapVector( snapped );
	G_SetOrigin( ent, snapped );
	VectorCopy( player->r.mins, ent->r.mins );
	VectorCopy( player->r.maxs, ent->r.maxs );

	ent->classname = "hi_portal destination";
	ent->s.pos.trType = TR_STATIONARY;

	ent->r.contents = CONTENTS_CORPSE;
	ent->takedamage = qtrue;
	ent->health = 200;
	ent->die = PortalDie;

	VectorCopy( player->s.apos.trBase, ent->s.angles );

	ent->think = G_FreeEntity;
	ent->nextthink = level.time + 2 * 60 * 1000;

	trap_LinkEntity( ent );

	player->client->portalID = ++level.portalSequence;
	ent->count = player->client->portalID;

	// give the item back so they can drop the source now
	player->client->ps.stats[STAT_HOLDABLE_ITEM] = BG_FindItem( "Portal" ) - bg_itemlist;
}


static void PortalTouch( gentity_t *self, gentity_t *other, trace_t *trace) {
	gentity_t	*destination;

	// see if we will even let other try to use it
	if( other->health <= 0 ) {
		return;
	}
	if( !other->client ) {
		return;
	}
//	if( other->client->ps.persistant[PERS_TEAM] != self->spawnflags ) {
//		return;
//	}

	if ( other->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_NEUTRALFLAG ), 0 );
		other->client->ps.powerups[PW_NEUTRALFLAG] = 0;
	}
	else if ( other->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_REDFLAG ), 0 );
		other->client->ps.powerups[PW_REDFLAG] = 0;
	}
	else if ( other->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
		Drop_Item( other, BG_FindItemForPowerup( PW_BLUEFLAG ), 0 );
		other->client->ps.powerups[PW_BLUEFLAG] = 0;
	}

	// find the destination
	destination = NULL;
	while( (destination = G_Find(destination, FOFS(classname), "hi_portal destination")) != NULL ) {
		if( destination->count == self->count ) {
			break;
		}
	}

	// if there is not one, die!
	if( !destination ) {
		if( self->pos1[0] || self->pos1[1] || self->pos1[2] ) {
			TeleportPlayer( other, self->pos1, self->s.angles );
		}
		G_Damage( other, other, other, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
		return;
	}

	TeleportPlayer( other, destination->s.pos.trBase, destination->s.angles );
}


static void PortalEnable( gentity_t *self ) {
	self->touch = PortalTouch;
	self->think = G_FreeEntity;
	self->nextthink = level.time + 2 * 60 * 1000;
}


void DropPortalSource( gentity_t *player ) {
	gentity_t	*ent;
	gentity_t	*destination;
	vec3_t		snapped;

	// create the portal source
	ent = G_Spawn();
	ent->s.modelindex = G_ModelIndex( "models/powerups/teleporter/tele_enter.md3" );

	VectorCopy( player->s.pos.trBase, snapped );
	SnapVector( snapped );
	G_SetOrigin( ent, snapped );
	VectorCopy( player->r.mins, ent->r.mins );
	VectorCopy( player->r.maxs, ent->r.maxs );

	ent->classname = "hi_portal source";
	ent->s.pos.trType = TR_STATIONARY;

	ent->r.contents = CONTENTS_CORPSE | CONTENTS_TRIGGER;
	ent->takedamage = qtrue;
	ent->health = 200;
	ent->die = PortalDie;

	trap_LinkEntity( ent );

	ent->count = player->client->portalID;
	player->client->portalID = 0;

//	ent->spawnflags = player->client->ps.persistant[PERS_TEAM];

	ent->nextthink = level.time + 1000;
	ent->think = PortalEnable;

	// find the destination
	destination = NULL;
	while( (destination = G_Find(destination, FOFS(classname), "hi_portal destination")) != NULL ) {
		if( destination->count == ent->count ) {
			VectorCopy( destination->s.pos.trBase, ent->pos1 );
			break;
		}
	}

}
#endif

/*QUAKED shooter_shell (1 0 0) (-16 -16 -16) (16 16 16)
PKMOD - Ergodic 11/21/00
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target. (1.0 default)
"enemy" ("1" == target the player activator, "0" shoot without aiming)
*/
void SP_shooter_shell( gentity_t *ent ) {
	InitShooter( ent, WP_SHOTGUN );
}

/*QUAKED shooter_rail (1 0 0) (-16 -16 -16) (16 16 16)
PKMOD - Ergodic 11/21/00
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target. (1.0 default)
"enemy" ("1" == target the player activator, "0" shoot without aiming)
*/
void SP_shooter_rail( gentity_t *ent ) {
	InitShooter( ent, WP_RAILGUN );
}

/*QUAKED shooter_lightning (1 0 0) (-16 -16 -16) (16 16 16)
PKMOD - Ergodic 11/21/00
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target. (1.0 default)
"enemy" ("1" == target the player activator, "0" shoot without aiming)
*/
void SP_shooter_lightning( gentity_t *ent ) {
	InitShooter( ent, WP_LIGHTNING );
}

/*QUAKED shooter_bullet (1 0 0) (-16 -16 -16) (16 16 16)
PKMOD - Ergodic 11/21/00
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target. (1.0 default)
"enemy" ("1" == target the player activator, "0" shoot without aiming)
*/
void SP_shooter_bullet( gentity_t *ent ) {
	InitShooter( ent, WP_MACHINEGUN );
}

/*QUAKED shooter_nail (1 0 0) (-16 -16 -16) (16 16 16)
PKMOD - Ergodic 11/21/00
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target. (1.0 default)
"enemy" ("1" == target the player activator, "0" shoot without aiming)
*/
void SP_shooter_nail( gentity_t *ent ) {
	InitShooter( ent, WP_NAILGUN );
}

/*QUAKED shooter_beartrap (1 0 .5) (-8 -8 -8) (8 8 8)
This will shoot a beartrap each time it's triggered. Aiming is done by setting the "angles" key or by targeting an info_notnull or target_position entity.
-------- KEYS --------
angles: this sets the pitch and yaw aiming angles of shooter (default 0 0). The roll angle does not apply.
speed: beartrap launch speed (default 400)
targetname : activating trigger points to this.
target : this points to a target_position entity for aiming the grenades.
random : random aiming variance in degrees from the straight line to the targeted entity (default 0 - see Notes).
notfree : when set to 1, entity will not spawn in "Free for all" and "Tournament" modes.
notteam : when set to 1, entity will not spawn in "Teamplay" and "CTF" modes.
notsingle : when set to 1, entity will not spawn in Single Player mode (bot play mode).
addfrag : if set to "1" then frag credit goes to player that activates the trap
-------- NOTES --------
When the random key is set, its value is used to calculate a maximum angle deviation from the normal trajectory formed by a straight line between the shooter and the aiming entity it targets. The final trajectory will be a random value anywhere between no deviation at all (0) to maximum deviation (value of the random key).

Both the setting "angles" key or "targeting a target_position" methods can be used to aim the shooter. However, the target_position method is simpler.
*/
void SP_shooter_beartrap( gentity_t *ent ) {
	InitShooter( ent, WP_BEARTRAP );
}
