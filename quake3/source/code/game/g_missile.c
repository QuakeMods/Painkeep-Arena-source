// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50

//PKMOD - Ergodic 08/03/00 add nail state definitions
void	NailTouch ( gentity_t *ent, gentity_t *other, trace_t *trace );
void	NailDeath ( gentity_t *ent );

//PKMOD - Ergodic 10/05/00 - define touch function for grapple touching items
void DragonTouch ( gentity_t *ent, gentity_t *other, trace_t *trace );

/*
=============
AltVectorToString (greater precision)

  vector * 1000

PKMOD - Ergodic (debugging helper)

This is just a convenience function
for printing vectors
=============
*/
char	*alt_vtos( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)(v[0]*1000), (int)(v[1]*1000), (int)(v[2]*1000));

	return s;
}




/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	//PKMOD - Ergodic 03/07/01 - debug autosentry multi-model bounce (inactive)
//	gentity_t		*other;

//Ergodic debug - disabled
//Com_Printf("G_BounceMissile - mins>%s<, maxs.%s<\n", vtos(ent->r.mins), vtos(ent->r.maxs));

	//PKMOD - Ergodic 03/07/01 - debug autosentry multi-model bounce (inactive)
//	if ( ent->s.eType == ET_AUTOSENTRY_BASE ) {
//		other = &g_entities[trace->entityNum];
//		Com_Printf("G_BounceMissile - ent->s.weapon>%d<, ent->s.pos.trDelta>%s<\n", ent->s.weapon, alt_vtos(ent->s.pos.trDelta) );
//	}

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );

	//PKMOD - Ergodic 02/23/04 - debug beartrap multi bounce (inactive)
//	if ( ent->s.weapon == WP_BEARTRAP ) {
//		int diff_i;
//		int fraction_i;
//		diff_i = hitTime - ent->s.pos.trTime;
//		fraction_i = 1000 * trace->fraction;
//		Com_Printf("G_BounceMissile - trType>%d<, hitTime>%d<, trTime>%d<, diff_i>%d<, fraction*1000>%d<, velocity*100>%s<\n", ent->s.pos.trType, hitTime, ent->s.pos.trTime, diff_i, fraction_i, alt_vtos( velocity) );
//	}

	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		//PKMOD - Ergodic 08/04/00 less bouncy gravity well
		//PKMOD - Ergodic 08/07/00 change value from 0.25 to 0.30 
		//PKMOD - Ergodic 08/07/00 less bouncy beartrap
		//PKMOD - Ergodic 11/22/00 less bouncy autosentry
		//PKMOD - Ergodic 07/02/01 make gravity well as bouncy as the grenade
//		if ( ( ent->s.weapon == WP_GRAVITY ) || ( ent->s.weapon == WP_BEARTRAP ) || ( ent->s.weapon == WP_SENTRY ) )
		if ( ( ent->s.weapon == WP_BEARTRAP ) || ( ent->s.weapon == WP_SENTRY ) )
			VectorScale( ent->s.pos.trDelta, 0.30, ent->s.pos.trDelta );
		else
			VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );

		//PKMOD - Ergodic 03/07/01 - debug autosentry multi-model bounce (inactive)
//		Com_Printf("G_BounceMissile - check4Stop: trace->plane.normal[2]>%f<, ent->s.pos.trDelta>%s<\n", trace->plane.normal[2] * 1000, alt_vtos(ent->s.pos.trDelta) );

	//PKMOD - Ergodic 03/07/01 - debug autosentry multi-model bounce (inactive)
//	Com_Printf("G_BounceMissile - ent->s.weapon>%d<, ent->s.pos.trDelta>%s<\n", ent->s.weapon, alt_vtos(ent->s.pos.trDelta) );


		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			//PKMOD - Ergodic 07/05/01 - fix beartrap bounce run-on
			//			if velocity is less than 20 then set it to zero...
			G_SetOrigin( ent, trace->endpos );
			//PKMOD - Ergodic 11/22/00 change to switch logic
			switch ( ent->s.weapon ) {
				case WP_BEARTRAP:

					//PKMOD - Ergodic 02/21/04 - ensure size is set
					//		execute only once
					if ( ent->r.maxs[0] != 28 ) { 
						//PKMOD - Ergodic 08/01/01 - allow beartraps to ride movers
						ent->s.groundEntityNum = trace->entityNum;

						//PKMOD - Ergodic 10/22/04 - set size of beartrap so it will interact with CLG and
						//			other damaging devices
						//PKMOD - Ergodic 10/13/03 - set mins size to 0 
						//VectorSet (bolt->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
						VectorSet (ent->r.mins, 0, 0, 0);
						//PKMOD - Ergodic 10/16/03 - set max from 15 to 24
						//VectorSet (bolt->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
						//VectorSet (bolt->r.maxs, 24, 24, 24);
						//PKMOD - Ergodic 02/01/04 - set max from 24 to 28
						VectorSet (ent->r.maxs, 28, 28, 28);
					
						//PKMOD - Ergodic 10/08/03 - set missile "absolute" size
						VectorCopy(ent->r.mins, ent->r.absmin);
						VectorCopy(ent->r.maxs, ent->r.absmax);
					}

					//PKMOD - Ergodic 02/21/04 - elminate beartrap multi-bounce 
					if ( VectorLength( ent->s.pos.trDelta ) > 1 ) {

						//PKMOD - Ergodic 02/23/04 - debug beartrap multi-bounce 
						Com_Printf("G_BounceMissile(1) - trace->plane.normal>%s<, s.pos.trDelta>%s<\n", alt_vtos( trace->plane.normal ), alt_vtos( ent->s.pos.trDelta ) );

						//PKMOD - Ergodic 08/07/2000 if beartrap then set beartrap drop sound
						G_AddEvent( ent, EV_BEARTRAP_DROP, 0 );
					}

					//PKMOD - Ergodic 07/05/01 - fix beartrap bounce run-on
					//			set velocity to zero...
					VectorClear( ent->s.pos.trDelta );

					break;
				case WP_SENTRY:
					//PKMOD - Ergodic 11/22/2000 if autosentry the beartrap then set beartrap drop sound
					//PKMOD Ergodic - 11/26/2000, once on ground, set autosentry
					//			entity type to "deploy"
					//PKMOD Ergodic - 01/31/01, only deploy if in launch state

					//PKMOD - Ergodic 08/02/01 - allow autosentrys to ride movers
					ent->s.groundEntityNum = trace->entityNum;

					if ( ent->s.eType == ET_AUTOSENTRY_LAUNCH ) {
						ent->s.eType = ET_AUTOSENTRY_DEPLOY;
						G_AddEvent( ent, EV_AUTOSENTRY_DROP, 0 );
					}
					break;
				default: 
					//PKMOD - Ergodic 08/08/00 move bounce event to G_BounceMissile because pkitems will 
					//			demonstrate different sounds when stopped.
					G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
			}
			//PKMOD - Ergodic 06/05/2000 if beartrap then set missile type
			//set eType so missile logic will no longer apply
//			if ( ent->s.eType = ET_BEARTRAP );
//				ent->s.eType = ET_GENERAL;
			//PKMOD - Ergodic 08/07/00 - debug inactive
//			Com_Printf("G_BounceMissile - stopped>%s<\n", vtos(ent->r.currentOrigin));
			return;
		}
		else {
			//PKMOD - Ergodic 07/06/01 - fix #2 for beartrap bounce run-on
			//			set velocity to zero...
			// This case is for beartrap that is in tight to a wall
			if ( ( ent->s.weapon == WP_BEARTRAP ) && ( VectorLength( ent->s.pos.trDelta ) < 40 ) ) {


				//PKMOD - Ergodic 02/25/04 - set the origin and turn entity into TR_STATIONARY
				G_SetOrigin( ent, trace->endpos );


				//PKMOD - Ergodic 02/21/04 - ensure size is set
				//		execute only once
				if ( ent->r.maxs[0] != 28 ) { 
					//PKMOD - Ergodic 08/01/01 - allow beartraps to ride movers
					ent->s.groundEntityNum = trace->entityNum;
					//PKMOD - Ergodic 10/22/04 - set size of beartrap so it will interact with CLG and
					//			other damaging devices
					//PKMOD - Ergodic 10/13/03 - set mins size to 0 
					//VectorSet (bolt->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
					VectorSet (ent->r.mins, 0, 0, 0);
					//PKMOD - Ergodic 10/16/03 - set max from 15 to 24
					//VectorSet (bolt->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
					//VectorSet (bolt->r.maxs, 24, 24, 24);
					//PKMOD - Ergodic 02/01/04 - set max from 24 to 28
					VectorSet (ent->r.maxs, 28, 28, 28);
				
					//PKMOD - Ergodic 10/08/03 - set missile "absolute" size
					VectorCopy(ent->r.mins, ent->r.absmin);
					VectorCopy(ent->r.maxs, ent->r.absmax);
				}

				//PKMOD - Ergodic 02/21/04 - elminate beartrap multi-bounce 
				if ( VectorLength( ent->s.pos.trDelta ) > 1 ) {
					float holdlength;
					int hold100;
					holdlength = VectorLength( ent->s.pos.trDelta );
					hold100 = 100 * holdlength;
					//PKMOD - Ergodic 02/23/04 - debug beartrap multi-bounce (inactive) 
					//Com_Printf("G_BounceMissile(2) - ent->s.pos.trDelta>%s<, veclength*100>%d<\n", alt_vtos( ent->s.pos.trDelta ), hold100 );
					//PKMOD - Ergodic 08/07/2000 if beartrap then set beartrap drop sound
					G_AddEvent( ent, EV_BEARTRAP_DROP, 0 );
				}

				//PKMOD - Ergodic 07/05/01 - fix beartrap bounce run-on
				//			set velocity to zero...
				VectorClear( ent->s.pos.trDelta );

				return;
			}
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
//Ergodic debug - disabled
//Com_Printf("G_BounceMissile - moving>%s<\n", vtos(ent->r.currentOrigin));
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
	//PKMOD - Ergodic 08/08/00 move bounce event to G_BounceMissile because pkitems will 
	//			demonstrate different sounds when stopped.
	G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );

}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, NULL
			, ent->splashMethodOfDeath ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity( ent );
}

#ifdef MISSIONPACK
/*
================
ProximityMine_Explode
================
*/
static void ProximityMine_Explode( gentity_t *mine ) {
	G_ExplodeMissile( mine );
	// if the prox mine has a trigger free it
	if (mine->activator) {
		G_FreeEntity(mine->activator);
		mine->activator = NULL;
	}
}

/*
================
ProximityMine_Die
================
*/
static void ProximityMine_Die( gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {
	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + 1;
}

/*
================
ProximityMine_Trigger
================
*/
void ProximityMine_Trigger( gentity_t *trigger, gentity_t *other, trace_t *trace ) {
	vec3_t		v;
	gentity_t	*mine;

	if( !other->client ) {
		return;
	}

	// trigger is a cube, do a distance test now to act as if it's a sphere
	VectorSubtract( trigger->s.pos.trBase, other->s.pos.trBase, v );
	if( VectorLength( v ) > trigger->parent->splashRadius ) {
		return;
	}


	if ( g_gametype.integer >= GT_TEAM ) {
		// don't trigger same team mines
		if (trigger->parent->s.generic1 == other->client->sess.sessionTeam) {
			return;
		}
	}

	// ok, now check for ability to damage so we don't get triggered thru walls, closed doors, etc...
	if( !CanDamage( other, trigger->s.pos.trBase ) ) {
		return;
	}

	// trigger the mine!
	mine = trigger->parent;
	mine->s.loopSound = 0;
	G_AddEvent( mine, EV_PROXIMITY_MINE_TRIGGER, 0 );
	mine->nextthink = level.time + 500;

	G_FreeEntity( trigger );
}

/*
================
ProximityMine_Activate
================
*/
static void ProximityMine_Activate( gentity_t *ent ) {
	gentity_t	*trigger;
	float		r;

	ent->think = ProximityMine_Explode;
	ent->nextthink = level.time + g_proxMineTimeout.integer;

	ent->takedamage = qtrue;
	ent->health = 1;
	ent->die = ProximityMine_Die;

	ent->s.loopSound = G_SoundIndex( "sound/weapons/proxmine/wstbtick.wav" );

	// build the proximity trigger
	trigger = G_Spawn ();

	trigger->classname = "proxmine_trigger";

	r = ent->splashRadius;
	VectorSet( trigger->r.mins, -r, -r, -r );
	VectorSet( trigger->r.maxs, r, r, r );

	G_SetOrigin( trigger, ent->s.pos.trBase );

	trigger->parent = ent;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->touch = ProximityMine_Trigger;

	trap_LinkEntity (trigger);

	// set pointer to trigger so the entity can be freed when the mine explodes
	ent->activator = trigger;
}

/*
================
ProximityMine_ExplodeOnPlayer
================
*/
static void ProximityMine_ExplodeOnPlayer( gentity_t *mine ) {
	gentity_t	*player;

	player = mine->enemy;
	player->client->ps.eFlags &= ~EF_TICKING;

	if ( player->client->invulnerabilityTime > level.time ) {
		G_Damage( player, mine->parent, mine->parent, vec3_origin, mine->s.origin, 1000, DAMAGE_NO_KNOCKBACK, MOD_JUICED );
		player->client->invulnerabilityTime = 0;
		G_TempEntity( player->client->ps.origin, EV_JUICED );
	}
	else {
		G_SetOrigin( mine, player->s.pos.trBase );
		// make sure the explosion gets to the client
		mine->r.svFlags &= ~SVF_NOCLIENT;
		mine->splashMethodOfDeath = MOD_PROXIMITY_MINE;
		G_ExplodeMissile( mine );
	}
}

/*
================
ProximityMine_Player
================
*/
static void ProximityMine_Player( gentity_t *mine, gentity_t *player ) {
	if( mine->s.eFlags & EF_NODRAW ) {
		return;
	}

	G_AddEvent( mine, EV_PROXIMITY_MINE_STICK, 0 );

	if( player->s.eFlags & EF_TICKING ) {
		player->activator->splashDamage += mine->splashDamage;
		player->activator->splashRadius *= 1.50;
		mine->think = G_FreeEntity;
		mine->nextthink = level.time;
		return;
	}

	player->client->ps.eFlags |= EF_TICKING;
	player->activator = mine;

	mine->s.eFlags |= EF_NODRAW;
	mine->r.svFlags |= SVF_NOCLIENT;
	mine->s.pos.trType = TR_LINEAR;
	VectorClear( mine->s.pos.trDelta );

	mine->enemy = player;
	mine->think = ProximityMine_ExplodeOnPlayer;
	if ( player->client->invulnerabilityTime > level.time ) {
		mine->nextthink = level.time + 2 * 1000;
	}
	else {
		mine->nextthink = level.time + 10 * 1000;
	}
}
#endif

/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;
	other = &g_entities[trace->entityNum];
#ifdef MISSIONPACK
	vec3_t			forward, impactpoint, bouncedir;
	int				eFlags;
#endif

	//PKMOD - Ergodic 01/25/01 - debug missile touching the sentry (inactive)
//	Com_Printf("G_MissileImpact - other->classname>%s<\n", other->classname);


//PKMOD - Ergodic 10/31/00 - debug (inactive)
//Com_Printf("G_MissileImpact - ent->classname>%s<, other->classname>%s<\n", ent->classname, other->classname);
//if ( !strcmp(other->classname, "bodyque" ) ) {
//	VectorScale( ent->s.pos.trDelta, 3.5, ent->s.pos.trDelta );
//	ent->s.pos.trDelta[2] *= 3.4;

//	if ( other->takedamage )
//		Com_Printf("G_MissileImpact - bodyque takes damage\n" );
//	else
//		Com_Printf("G_MissileImpact - bodyque does not take damage\n" );
//}
	//PKMOD - Ergodic 07/11/00 Thrown Gravity well - just bounce
//	if (!strcmp(ent->classname, "Gravity Well")) {
	//PKMOD - Ergodic 01/30/01 Make the code more efficient
	if ( ent->s.weapon == WP_GRAVITY ) {
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(0) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		G_BounceMissile( ent, trace );
	//PKMOD - Ergodic 08/08/00 move bounce event to G_BounceMissile because pkitems will 
	//			demonstrate different sounds when stopped.
//		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	//PKMOD - Ergodic 01/30/01 add bounce for autosentry
	//PKMOD - Ergodic 04/06/01 add case of NOT autosentry missile
	if ( ( ent->s.weapon == WP_SENTRY ) && ( ent->s.eType != ET_MISSILE ) ) {
	//PKMOD - Ergodic 03/08/01 debug multi bounce (inactive)
//		Com_Printf("G_MissileImpact - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(1) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		G_BounceMissile( ent, trace );
		return;
	}

//	if ( other->takedamage && other->client && !strcmp(ent->classname, "beartrap") ) {
		//PKMOD - Ergodic 10/30/00 - debug
//		Com_Printf("G_MissileImpact - beartrap hit on client\n" );
//	}

		// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
	//PKMOD - Ergodic 04/23/01 debug nails on autosentry bug (inactive)
//		Com_Printf("G_MissileImpact - [1]ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(2) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		G_BounceMissile( ent, trace );
	//PKMOD - Ergodic 08/08/00 move bounce event to G_BounceMissile because pkitems will 
	//			demonstrate different sounds when stopped.
//		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

//PKMOD - Ergodic 06/05/00 Beatrap Missile logic
		//PKMOD - Ergodic 03/08/01 optimize (don't do strcmp - sheesh)
//	if (!strcmp(ent->classname, "beartrap")) {
		//PKMOD - Ergodic 10/31/00 modify for beartrap hitting a corpse
		//PKMOD - Ergodic 03/08/01 fix beartrap disappearing on the autosentry
	if ( ent->s.weapon == WP_BEARTRAP ) {
	//PKMOD - Ergodic 03/08/01 debug multi bounce (inactive)
//		Com_Printf("G_MissileImpact - [2]ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
		//PKMOD - Ergodic 03/09/01 - separate bodyque from sentry logic

		//PKMOD - Ergodic 08/05/01 - Debug beartraps hitting doors and then disappearing (inactive)
//		Com_Printf( "G_MissileImpact: Beartrap: >%s<, ent->s.eType>%d<, other->s.eType>%d<,  other->s.eFlags>%d<, other->classname>%s<\n", ent->classname, ent->s.eType, other->s.eType, other->s.eFlags, other->classname );

		//PKMOD - Ergodic 03/10/01 - new net and bot corpse logic
		if ( other->s.eType == ET_PLAYER ) {
			if ( !strcmp(other->classname, "bodyque" ) || ( other->s.eFlags & EF_DEAD ) ) {
 				VectorScale( ent->s.pos.trDelta, 3.5, ent->s.pos.trDelta ); //give a little umph
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(3) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
				G_BounceMissile( ent, trace );
			}
		}
		else {
			if ( ( other->s.weapon == WP_SENTRY ) ){
				//PKMOD - Ergodic 03/08/01 - if beartrap is slow moving then do not bounce
				if ( VectorLength( ent->s.pos.trDelta ) > 30 ) {
					VectorScale( ent->s.pos.trDelta, 3.5, ent->s.pos.trDelta ); //give a little umph
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(4) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
					G_BounceMissile( ent, trace );
				}
			}
			else {
				//PKMOD - Ergodic 08/05/01 - beartraps should bounce against doors
				if ( other->s.eType == ET_MOVER ) {
					if ( !strcmp(other->classname, "func_door" ) ) {
	 					VectorScale( ent->s.pos.trDelta, 3.0, ent->s.pos.trDelta ); //give a little umph
		//PKMOD - Ergodic 02/21/04 debug multi bounce beartrap (inactive)
		//Com_Printf("G_MissileImpact(5) - ent->s.eType>%d<, other->s.eType>%d<\n", ent->s.eType, other->s.eType );
						G_BounceMissile( ent, trace );
					}
				}
			}
		}

		return;
	}

#ifdef MISSIONPACK
	if ( other->takedamage ) {
		if ( ent->s.weapon != WP_PROX_LAUNCHER ) {
			if ( other->client && other->client->invulnerabilityTime > level.time ) {
				//
				VectorCopy( ent->s.pos.trDelta, forward );
				VectorNormalize( forward );
				if (G_InvulnerabilityEffect( other, forward, ent->s.pos.trBase, impactpoint, bouncedir )) {
					VectorCopy( bouncedir, trace->plane.normal );
					eFlags = ent->s.eFlags & EF_BOUNCE_HALF;
					ent->s.eFlags &= ~EF_BOUNCE_HALF;
					G_BounceMissile( ent, trace );
					ent->s.eFlags |= eFlags;
				}
				ent->target_ent = other;
				return;
			}
		}
	}
#endif


	// impact damage
	if (other->takedamage) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			//PKMOD - Ergodic 04/23/01 - Debug nails on autosentry (inactive)
//			Com_Printf( "G_MissileImpact: wrong way damage\n" );

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage, 
				0, ent->methodOfDeath);
		}
	}

#ifdef MISSIONPACK
	if( ent->s.weapon == WP_PROX_LAUNCHER ) {
		if( ent->s.pos.trType != TR_GRAVITY ) {
			return;
		}

		// if it's a player, stick it on to them (flag them and remove this entity)
		if( other->s.eType == ET_PLAYER && other->health > 0 ) {
			ProximityMine_Player( ent, other );
			return;
		}

		SnapVectorTowards( trace->endpos, ent->s.pos.trBase );
		G_SetOrigin( ent, trace->endpos );
		ent->s.pos.trType = TR_STATIONARY;
		VectorClear( ent->s.pos.trDelta );

		G_AddEvent( ent, EV_PROXIMITY_MINE_STICK, trace->surfaceFlags );

		ent->think = ProximityMine_Activate;
		ent->nextthink = level.time + 2000;

		vectoangles( trace->plane.normal, ent->s.angles );
		ent->s.angles[0] += 90;

		// link the prox mine to the other entity
		ent->enemy = other;
		ent->die = ProximityMine_Die;
		VectorCopy(trace->plane.normal, ent->movedir);
		VectorSet(ent->r.mins, -4, -4, -4);
		VectorSet(ent->r.maxs, 4, 4, 4);
		trap_LinkEntity(ent);

		return;
	}
#endif

	if (!strcmp(ent->classname, "hook")) {
		gentity_t *nent;
		vec3_t v;

		//PKMOD - Ergodic 01/26/01 - Debug (inactive)
//		Com_Printf("G_MissileImpact - Hook Other's: classname>%s<, contents>%d<, clipmask>%d<\n", other->classname, other->r.contents, other->clipmask );

		nent = G_Spawn();
		if ( other->takedamage && other->client ) {

			G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth
		} else {
			VectorCopy(trace->endpos, v);
			G_AddEvent( nent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
			ent->enemy = NULL;
		}

		SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType = ET_GRAPPLE;

		//PKMOD - Ergodic 08/03/01 - allow dragon to ride movers
		ent->s.groundEntityNum = trace->entityNum;

		G_SetOrigin( ent, v );
		G_SetOrigin( nent, v );

		ent->think = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity( ent );
		trap_LinkEntity( nent );

		return;
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	//PKMOD - Ergodic 08/06/00 logic change: so nails hitting clients will be removed immediately
	//PKMOD - Ergodic 01/29/01 - add hit on autosentry an autosentry with this specific comparison...
	if ( other->takedamage && ( other->client || ( ( other->s.eType <= ET_AUTOSENTRY_TURRET ) && ( other->s.eType >= ET_AUTOSENTRY_LAUNCH ) ) ) ) {
		//PKMOD - Ergodic 04/23/01 - Debug nails on sentry (inactive)
//		Com_Printf( "G_MissileImpact: here we are - other->s.eType>%d<\n", other->s.eType );

		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
		//PKMOD - Ergodic 08/03/00 logic change: bundle else clause with ET_GENERAL
		ent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		ent->s.eType = ET_GENERAL;
	}
	else {
		
		//PKMOD - Ergodic 04/23/01 - Debug nails on sentry (inactive)
//		Com_Printf( "G_MissileImpact: we are in else - ent->s.weapon>%d<, other->s.eType>%d<\n", ent->s.weapon, other->s.eType );

		if( trace->surfaceFlags & SURF_METALSTEPS )
			G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
		else
			G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );

		//PKMOD - Ergodic 08/01/00 nailgun persistence code
		//PKMOD - Ergodic 08/03/00 updated with more values
		if ( ent->s.weapon == WP_NAILGUN ) {
			//PKMOD - Ergodic 10/14/00 remove immediately if hitting a door
			//PKMOD - Ergodic 10/15/00 remove immediately if hitting a bobbing
			//PKMOD - Ergodic 10/15/00 remove immediately if hitting a pendulum
			//PKMOD - Ergodic 10/15/00 remove immediately if hitting a train
			//PKMOD - Ergodic 10/15/00 remove immediately if hitting a plat
			//PKMOD - Ergodic 10/15/00 remove immediately if hitting a rotating

//			if ( !strcmp( other->classname, "func_door" ) ||
//				!strcmp( other->classname, "func_bobbing" ) ||
//				!strcmp( other->classname, "func_pendulum" ) ||
//				!strcmp( other->classname, "func_train" ) || 
//				!strcmp( other->classname, "func_plat" ) || 
//				!strcmp( other->classname, "func_rotating" ) ) {
			//PKMOD - Ergodic 01/29/01 - optimize the nail impact with func_*
			//			assumming the follwing two types:
//				!strcmp( other->classname, "func_button" ) || 
//				!strcmp( other->classname, "func_static" ) || 

			//PKMOD - Ergodic 04/23/01 - debug nails on sentrys (inactive)
//			Com_Printf( "G_MissileImpact: ent->s.eType>%d<, r.mins>%s<, r.maxs>%s<\n", ent->s.eType, vtos(ent->r.mins), vtos(ent->r.maxs) );
//			Com_Printf( "G_MissileImpact: other->s.eType>%d<, r.mins>%s<, r.maxs>%s<\n", other->s.eType, vtos(other->r.mins), vtos(other->r.maxs) );
			
			if ( other->s.eType == ET_MOVER ) {
				G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
				ent->s.otherEntityNum = other->s.number;
				//PKMOD - Ergodic 08/03/00 logic change: bundle else clause with ET_GENERAL
				ent->freeAfterEvent = qtrue;
				// change over to a normal entity right at the point of impact
				ent->s.eType = ET_GENERAL;
			}
			else {
				ent->touch = NailTouch;
				//PKMOD - Ergodic 01/29/01 - add random time on nails
				ent->nextthink = level.time + 1500 + rand() % 500;
				ent->think = NailDeath;

				VectorSet( ent->s.pos.trDelta, 0, 0, 0 );
				ent->r.contents = CONTENTS_TRIGGER;
				//ITEM_RADIUS = 15
				VectorSet (ent->r.mins, -15, -15, -15);
				VectorSet (ent->r.maxs, 15, 15, 15);

				ent->s.eType = ET_NAIL;
				//10/28/00 set punctuation mechanism using the "wait" field
				ent->wait = level.time;
			}
		}
		else {
			//PKMOD - Ergodic 08/03/00 logic change: bundle else clause with ET_GENERAL
			ent->freeAfterEvent = qtrue;
			// change over to a normal entity right at the point of impact
			ent->s.eType = ET_GENERAL;
		}
	}

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}


/*
================
G_RunMissile

================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			passent;
	gentity_t	*traceEnt;
	vec3_t		dragonorigin; //PKMOD - Ergodic 01/11/01 - Fix grappling through a doorway
	vec3_t		turretorigin; //PKMOD - Ergodic  03/07/01 - set turret's location from base

	//PKMOD - Ergodic 01/11/01 - Fix grappling through a doorway (backup the currentOrigin)
	if ( ent->s.weapon == WP_GRAPPLING_HOOK ) {
		VectorCopy( ent->r.currentOrigin, dragonorigin );
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
#ifdef MISSIONPACK
	// prox mines that left the owner bbox will attach to anything, even the owner
	else if (ent->s.weapon == WP_PROX_LAUNCHER && ent->count) {
		passent = ENTITYNUM_NONE;
	}
#endif
	}
	else {
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}

	//PKMOD - Ergodic 03/06/01 - autosentry base should not bounce off the autosentry turret
	if ( ent->s.eType == ET_AUTOSENTRY_BASE ) {
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->nextTrain->s.number, ent->clipmask );
		traceEnt = &g_entities[ tr.entityNum ];
		//PKMOD - Ergodic 03/08/01 - update the attached turret's location
		if ( ent->nextTrain->inuse ) {
			VectorCopy(  ent->r.currentOrigin, turretorigin );
			turretorigin[2] += 16;		//offest turret up by 16 units
			G_SetOrigin( ent->nextTrain, turretorigin );
		}

	}
	else {
		//PKMOD - Ergodic 04/09/01 - if an autosentry missile
		//			This case will ignore sentry missiles from hitting
		//			the launching turret
		//			Launching turret entity number is stored in splashRadius (hack)
		if ( ( ent->s.eType == ET_MISSILE )  && ( ent->s.weapon == WP_SENTRY ) ) {
			//PKMOD - Ergodic 06/14/02 - make sure Personal Sentry missiles don't hit player
			//PKMOD - Ergodic 07/31/02 - time2==251 -> persentry determination
			if ( ent->s.time2 == 251 ) {//co-opt the time2 variable for communication to cgame
				//PKMOD - Ergodic 08/02/02 - Debug (inactive)
//				Com_Printf( "G_RunMissile: Personal Sentry Missile detected\n" );
				trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );
			}
			else {
				//PKMOD - Ergodic 10/23/02 - add (turret) ent info so that it can be ignored by autosentry missile
				trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->prevTrain->s.number, ent->clipmask );
			}
			traceEnt = &g_entities[ tr.entityNum ];
		}
		else {
			//PKMOD - Ergodic 01/29/01 - send a trace to detect an autosentry
			trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ENTITYNUM_NONE, ent->clipmask );
			traceEnt = &g_entities[ tr.entityNum ];
		}
		//PKMOD - Ergodic 04/23/01 - Debug (inactive)
//		Com_Printf( "G_RunMissile: traceEnt->classname>%s<, traceEnt->s.eType>%d<\n", traceEnt->classname, traceEnt->s.eType );
	}

	//PKMOD - Ergodic 04/07/01 - if NOT an autosentry missile
	//			This case will allow sentry missiles to hit players
	if ( ( ent->s.eType != ET_MISSILE )  || ( ent->s.weapon != WP_SENTRY ) ) {
		//PKMOD - Ergodic 01/29/01 - if NOT detect an autosentry with this specific comparison...
		if ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) {
			//We did not hit an autosentry so send trace to detect a passent entity
			// trace a line from the previous position to the current position
			trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );
			traceEnt = &g_entities[ tr.entityNum ];
			//PKMOD - Ergodic 03/07/01 - Debug (inactvive) 
//			if ( ent->s.eType != ET_AUTOSENTRY_BASE )
//				Com_Printf( "G_RunMissile: [3]: >%s<, ent->s.eType:>%d<, traceEnt->s.eType:>%d<\n", ent->classname, ent->s.eType, traceEnt->s.eType );
		}
	}

	//PKMOD - Ergodic 03/07/01 - Debug (inactive)
//	Com_Printf( "G_RunMissile: [1]solid hit: >%s<, ent->s.eType:>%d<, traceEnt->s.eType:>%d<\n", ent->classname, ent->s.eType, traceEnt->s.eType );


	if ( tr.startsolid || tr.allsolid ) {
		//PKMOD - Ergodic 01/26/01 - debug dragon hit (inactive)
//		traceEnt = &g_entities[ tr.entityNum ];
		//PKMOD - Ergodic 04/23/01 - Debug nails on sentry (inactive)
//		Com_Printf( "G_RunMissile: [2]solid hit: >%s<, ent->s.eType:>%d<, traceEnt->s.eType:>%d<\n", ent->classname, ent->s.eType, traceEnt->s.eType );
		if ( ( ent->s.eType != ET_MISSILE ) || ( traceEnt->s.eType != ET_AUTOSENTRY_TURRET ) ) {
		//PKMOD - Ergodic 04/23/01 - Debug upclose nails on sentry

			//PKMOD - Ergodic 03/09/01 - ignore turret on beartrap (negative logic)
			if ( ( ent->s.eType != ET_BEARTRAP ) || ( traceEnt->s.eType != ET_AUTOSENTRY_TURRET ) ) {
				//PKMOD - Ergodic 01/26/01 - Debug (inactive)
//				Com_Printf(" G_RunMissile: [2]solid: classname>%s<, contents>%d<, clipmask>%d<\n", traceEnt->classname, traceEnt->r.contents, traceEnt->clipmask );

				// make sure the tr.entityNum is set to the entity we're stuck in
				trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
				tr.fraction = 0;
				traceEnt = &g_entities[ tr.entityNum ];
			}
		}
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	trap_LinkEntity( ent );

	if ( tr.fraction != 1 ) {

		//PKMOD - Ergodic 10/04/00 grappling code
		if ( ent->s.weapon == WP_GRAPPLING_HOOK ) {

			//PKMOD - Ergodic 08/02/01 - debug dragon deploy hitting doors (inactive)
//			Com_Printf( "G_RunMissile: dragon/dragondeploy hit: >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );
			//PKMOD - Ergodic 03/14/01 - add dragon deployable weapon fire
			//PKMOD - Ergodic 03/28/01 - remove this code (for Beta 2.2)
			//PKMOD - Ergodic 05/25/01 - re-add code for testing
			if ( ent->s.generic1 != 0 ) {
				//PKMOD - Ergodic 08/02/01 - create a new entity type for door_trigger and trigger_multiple
				//								fixes the bug of dragon deploy hitting doors
				if ( traceEnt->s.eType == ET_DOOR_TRIGGER || traceEnt->s.eType == ET_TRIGGER_MULTIPLE ) {
					//PKMOD - Ergodic 08/02/01 - debug dragon deploy hitting doors (inactive)
//					Com_Printf( "G_RunMissile: dragon deploy hit: >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );

					//PKMOD - Ergodic 08/05/01 - check if door is opening or opened
					if ( traceEnt->parent->moverState == MOVER_1TO2 || traceEnt->parent->moverState == MOVER_POS2) {
						trap_Trace( &tr, dragonorigin, ent->r.mins, ent->r.maxs, origin, passent, MASK_SHOT );
						VectorCopy( tr.endpos, ent->r.currentOrigin );
						return;
					}
				}
					//PKMOD - Ergodic 08/02/01 - debug dragon deploy hitting doors (inactive)
//					Com_Printf( "G_RunMissile: dragon deploy hit: >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );

				//PKMOD - Ergodic 08/24/01 - dragon deploy touching a flag - ignore flag
				if ( ( traceEnt->s.eType == ET_ITEM ) && ( traceEnt->item->giType == IT_TEAM ) && ( ( traceEnt->item->giTag == PW_BLUEFLAG ) || ( traceEnt->item->giTag == PW_REDFLAG ) ) ) {
					trap_Trace( &tr, dragonorigin, ent->r.mins, ent->r.maxs, origin, passent, MASK_SHOT );
					VectorCopy( tr.endpos, ent->r.currentOrigin );
					return;
				}

				//PKMOD - Ergodic 12/20/01 - modify activate_dragon_deploy call to add struck entity
				//PKMOD - Ergodic 01/26/02 - modify activate_dragon_deploy call to add bytedir for gauntlet
				activate_dragon_deploy ( ent, traceEnt, DirToByte( tr.plane.normal ) );
				//PKMOD - Ergodic 03/15/01 - remove hook & exit
				Weapon_HookFree( ent );
				return;
			}
			//			traceEnt = &g_entities[ tr.entityNum ];
			//PKMOD - Ergodic 03/14/01 - debug dragon hit (inactive)
//			Com_Printf( "G_RunMissile: [1]WP_GRAPPLING_HOOK hit: >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );
			//PKMOD - Ergodic 01/26/01 - Debug (inactive)
//			Com_Printf(" G_RunMissile: [2]WP_GRAPPLING_HOOK hit: classname>%s<, contents>%d<, clipmask>%d<\n", traceEnt->classname, traceEnt->r.contents, traceEnt->clipmask );

			//PKMOD - Ergodic 01/11/01 - Fix grappling through a doorway
			//PKMOD - Ergodic 01/29/01 - If last trace did not detect an autosenty nor
			//			an ET_ITEM then use MASK_SHOT
			if ( (traceEnt->s.eType != ET_ITEM ) && ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) ) {
//			if ( !strcmp(traceEnt->classname, "door_trigger" ) ) {  //found a door
				//if hit door then recalculate trace with a mask_shot clip mask
				//PKMOD - Ergodic 01/11/01 - debug inactive
//				Com_Printf( "G_RunMissile: hit a door - recalculate trap trace\n" );
				// trace a line from the previous position to the current position
				trap_Trace( &tr, dragonorigin, ent->r.mins, ent->r.maxs, origin, passent, MASK_SHOT );
				traceEnt = &g_entities[ tr.entityNum ];
				//PKMOD - Ergodic 10/05/00 - debug dragon hit (inactive)
//				Com_Printf( "G_RunMissile: [3]WP_GRAPPLING_HOOK hit >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );
	
				if ( tr.startsolid || tr.allsolid ) {
					// make sure the tr.entityNum is set to the entity we're stuck in
					trap_Trace( &tr, dragonorigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, MASK_SHOT );
					tr.fraction = 0;
				}
				else {
					VectorCopy( tr.endpos, ent->r.currentOrigin );
				}

				//exit if did not hit anything after recalculation
				if ( tr.fraction == 1 ) {
					G_RunThink( ent );
					return;
				}
			}

			//PKMOD - Ergodic 08/24/01 - dragon touching a flag - ignore flag
			if ( ( traceEnt->s.eType == ET_ITEM ) && ( traceEnt->item->giType == IT_TEAM ) && ( ( traceEnt->item->giTag == PW_BLUEFLAG ) || ( traceEnt->item->giTag == PW_REDFLAG ) ) ) {
				//PKMOD - Ergodic 08/24/01 - debug other (inactive)
//				Com_Printf( "G_RunMissile[0] - traceEnt: classname>%s<, eType>%d<, eTag>%d<\n", traceEnt->classname, traceEnt->s.eType, traceEnt->item->giTag );

				trap_Trace( &tr, dragonorigin, ent->r.mins, ent->r.maxs, origin, passent, MASK_SHOT );
				traceEnt = &g_entities[ tr.entityNum ];
				//PKMOD - Ergodic 10/05/00 - debug dragon hit (inactive)
//				Com_Printf( "G_RunMissile: [3]WP_GRAPPLING_HOOK hit >%s<, traceEnt->s.eType:>%d<\n", traceEnt->classname, traceEnt->s.eType );
				//PKMOD - Ergodic 08/24/01 - debug other (inactive)
//				Com_Printf( "G_RunMissile[1] - traceEnt: classname>%s<, eType>%d<, eTag>%d<\n", traceEnt->classname, traceEnt->s.eType, traceEnt->item->giTag );

				VectorCopy( tr.endpos, ent->r.currentOrigin );
				return;
			}



			if ( ent->AirFist_ResetTime == 1 ) { //if dragon is already holding an item
				//PKMOD - Ergodic 10/05/00 - debug hit inactive
//				Com_Printf( "G_RunMissile: ent->AirFist_ResetTime == 1\n" );
				G_RunThink( ent );
				return;
			}

			if ( traceEnt->s.eType  == ET_ITEM ) {  //found an item
				//PKMOD - Ergodic 08/25/01 - debug other (inactive)
//				Com_Printf( "G_RunMissile[3] - traceEnt: classname>%s<, eType>%d<, giType>%d<, giTag>%d<\n", traceEnt->classname, traceEnt->s.eType, traceEnt->item->giType, traceEnt->item->giTag );

				//PKMOD - Ergodic 10/06/00 - don't grapple a voting entity
				if ( traceEnt->item->giType != IT_VOTING ) {
					DragonTouch ( ent, traceEnt, &tr );
					G_RunThink( ent );
					return;
				}
			}


		}


		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {

			//PKMOD - Ergodic 04/19/01 - autosentry in teleport (exploit)
			if ( ent->s.eType == ET_AUTOSENTRY_BASE ) {
				if ( ent->nextTrain->inuse ) {
					//PKMOD Ergodic - 01/23/01, set flag for the turret portion of the autosentry to die
					ent->nextTrain->pka_flags |= PKAEF_AUTOSENTRYDEATH;
				}
				G_FreeEntity( ent );
				return;
			}


			/*
			//PKMOD - Ergodic 04/19/01 - autosentry in teleport (exploit)
			if ( ent->s.eType == ET_AUTOSENTRY_BASE ) {
				//schedule autosentry death
				//(1) did we try to remove this before?
				if ( !( ent->pka_flags & PKAEF_AUTOSENTRYDEATH ) ) {
					ent->pka_flags |= PKAEF_AUTOSENTRYDEATH;
					//(2) Remove the Autosentry entity (base portion)
					ent->nextthink = level.time + 200; 
					ent->think = AutoSentry_BaseDeath;
					//PKMOD - Ergodic 01/30/01 - add check for turret status (inuse == qtrue) 
					if ( ent->nextTrain->inuse ) {
						//PKMOD Ergodic - 01/23/01, set flag for the turret portion of the autosentry to die
						ent->nextTrain->pka_flags |= PKAEF_AUTOSENTRYDEATH;
					}
				}
				return;
			}
			*/

			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent) {
				ent->parent->client->hook = NULL;
			}

			G_FreeEntity( ent );

			return;
		}

		G_MissileImpact( ent, &tr );


//		if ( ent->s.eType != ET_MISSILE && ent->s.eType != ET_BEARTRAP ) {
		//PKMOD - Ergodic 01/31/01 - add check for not an autosentry
		//PKMOD - Ergodic 01/31/01 - ignore the turret sentry
//		if ( ent->s.eType != ET_MISSILE && ent->s.eType != ET_BEARTRAP && ( ( ent->s.eType > ET_AUTOSENTRY_TURRET ) || ( ent->s.eType < ET_AUTOSENTRY_LAUNCH ) ) ) {
		if ( ent->s.eType != ET_MISSILE && ent->s.eType != ET_BEARTRAP && ( ( ent->s.eType >= ET_AUTOSENTRY_TURRET ) || ( ent->s.eType < ET_AUTOSENTRY_LAUNCH ) ) ) {
			return;		// exploded
		}
	}
#ifdef MISSIONPACK
	// if the prox mine wasn't yet outside the player body
	if (ent->s.weapon == WP_PROX_LAUNCHER && !ent->count) {
		// check if the prox mine is outside the owner bbox
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, ENTITYNUM_NONE, ent->clipmask );
		if (!tr.startsolid || tr.entityNum != ent->r.ownerNum) {
			ent->count = 1;
		}
	}
#endif

	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================

//PKMOD
/*PKMOD -Add Weapons. 
	WP_HARPOON,
	WP_GRAVITY,
	WP_SENTRY,
	WP_BEARTRAP,
	WP_CHAINLG,
	WP_A2K,
	WP_EMPNUKE,
	WP_AIRFIST,
	WP_NAILGUN,
	PKMOD -Add Weapons. */
/*
=================
AutoSentry Definitions
=================
*/
#define AUTOSENTRY_RANGE 1500		//maximum range of autosentry target detection 
#define AUTOSENTRY_SWEEP 12			//degrees the autosentry will rotate in one movement 
#define AUTOSENTRY_DETERMINATION 5	//how many times to keep tracking an obscured target 
#define AUTOSENTRY_SPREAD	300
#define	AUTOSENTRY_DAMAGE	9
#define AUTOSENTRY_INFRONT 0.86		//Target is infront of autosentry if higher than this value
#define AUTOSENTRY_BIG_SWEEP 30		//03/22/01 - big_sweep in vertical if needed 
#define AUTOSENTRY_PING_TIME 1500	//03/26/01 - time in between pings 

//#define AUTOSENTRY_ROTATE_VELOCITY 60		//01/31/02 - rotation velocity for the autosentry
//#define AUTOSENTRY_ROTATE_VELOCITY 70		//02/17/02 - ~15% faster rotation
#define AUTOSENTRY_ROTATE_VELOCITY 90		//10/27/02 - ~50% faster rotation
//#define AUTOSENTRY_VERTICAL_VELOCITY 5		//05/23/02 - add velocity for resetting the vertical position
#define AUTOSENTRY_VERTICAL_VELOCITY 8		//12/14/02 - add velocity for resetting the vertical position
#define AUTOSENTRY_MAX_PITCH 80				//12/14/02 - max value of the ABS (pitch)

void AutoSentryTurret_Think ( gentity_t *ent );
void Bullet_Fire (gentity_t *ent, float spread, int damage );

/*
=================
AutoSentry_BaseDeath
=================
*/
void AutoSentry_BaseDeath ( gentity_t *ent ) {

	ent->nextthink = level.time + 200; 
	ent->think = G_FreeEntity;

}

/*
=================
AutoSentry_TurretDeath
=================
*/
void AutoSentry_TurretDeath ( gentity_t *ent ) {

	int			splashdamage;
	int			splashradius;
	qboolean	dummy;

	//PKMOD Ergodic - 01/30/01, add flag for setting the turret portion of the autosentry to Free
	//			This will prevent the sentry from dying twice
	if ( ent->pka_flags & PKAEF_AUTOSENTRYFREE ) //if free then already dead so return
		return;

	//PKMOD Ergodic - 01/30/01, set the free flag
	ent->pka_flags |= PKAEF_AUTOSENTRYFREE;

	ent->touch = 0; //PKMOD - Ergodic 08/06/00 change NULL to 0 because of compiler warnings
	G_AddEvent( ent, EV_AUTOSENTRY_DIE, 0 );

	//if the autosentry has a turret then remove the turret
//	if ( ent->nextTrain != NULL ) {
//		if ( ent->nextTrain->inuse ) {
//			ent->nextTrain->nextthink = level.time + 200; 
//			ent->nextTrain->think = G_FreeEntity;
//		}
//	}

	//PKMOD - Ergodic 01/13/01 - cause explosion
	splashdamage = 45 + rand() % 20;
	splashradius = 100 + rand() % 40;

//Ergodic - this will cause a crash...
	//PKMOD - Ergodic 01/23/01 - lucky guess - don't damage self
	dummy = G_RadiusDamage( ent->r.currentOrigin, ent->parent, splashdamage, splashradius, ent, MOD_SENTRY_SPLASH ); 

	ent->nextthink = level.time + 200; 
	ent->think = G_FreeEntity;

//	trap_LinkEntity( ent );

}

//PKMOD - Ergodic 01/22/01 - add new die function for autosentry base
void AutoSentry_BaseDie ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	AutoSentry_BaseDeath ( self );
}

//PKMOD - Ergodic 01/22/01 - rename the die function to TurretDie (make it more specific)
void AutoSentry_TurretDie ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	AutoSentry_TurretDeath ( self );
}

/*
=================
AutoSentry_Touch
=================
*/
void AutoSentry_Touch ( gentity_t *ent, gentity_t *other, trace_t *trace ) {

	//PKMOD - Ergodic 04/18/01 - debug (inactive)
//	Com_Printf("AutoSentry_Touch - other->classname>%s<\n", other->classname );

	//operate on clients only
	if ( !other->client )
		return;

	//owner is immune to the autosentry for a moment
	if ( (other == ent->parent) && (ent->wait > level.time) ) {
		return;
	}

}

/*
=================
AutoSentryBase_Think
=================
*/
void AutoSentryBase_Think ( gentity_t *ent ) {
//	vec3_t		hold_vec;
	//PKMOD - Ergodic 04/18/01 - debug wait state (inactive)
//	int			diff;

	//PKMOD - Ergodic 04/18/01 - debug wait state (inactive)
//	diff = (int)ent->wait - level.time;
//	Com_Printf("AutoSentryBase_Think - wait>%f<, level.time>%d<, diff>%d<\n", ent->wait, level.time, diff);

	//PKMOD - Ergodic 01/24/01 - debug (inactive) zlocations
//	int		player_z, base_z, turret_z;

	//PKMOD - Ergodic 01/24/01 - debug (inactive) zlocations
//	player_z = ent->parent->r.currentOrigin[2];
//	base_z = ent->r.currentOrigin[2];
//	turret_z = ent->nextTrain->r.currentOrigin[2];
//	Com_Printf("AutoSentryBase_Think - player_z>%d<, base_z>%d<, turret_z>%d<\n", player_z, base_z, turret_z );

	//PKMOD - Ergodic 01/24/01 - debug moving the turret's location (inactive) does not work
//	ent->nextTrain->r.currentOrigin[2] += 1;

	//PKMOD - Ergodic 03/23/01 - remove autosentry if the owner player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		G_FreeEntity( ent->nextTrain );
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 03/23/01 - remove sentry if owner's status has changed
	if ( ent->s.modelindex != ent->parent->client->sess.sessionTeam ) {
		G_FreeEntity( ent->nextTrain );
		G_FreeEntity( ent );
		return;
	}
	
	//PKMOD - Ergodic 01/30/01 - add check for turret status (inuse == qtrue)
	if (ent->wait > level.time && ent->nextTrain->inuse ) {

		//03/06/01 check for movement
		//set the location and vertical offset 
//		VectorCopy ( ent->r.currentOrigin, hold_vec );
//		hold_vec[2] += 16;
		//03/07/01 turn off
//		G_SetOrigin ( ent->nextTrain, hold_vec );

		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	}
	else {
		//Remove the Autosentry entity (base portion)
		ent->nextthink = level.time + 200; 
		ent->think = AutoSentry_BaseDeath;
		//PKMOD - Ergodic 01/30/01 - add check for turret status (inuse == qtrue) 
		if ( ent->nextTrain->inuse ) {
			//PKMOD Ergodic - 01/23/01, set flag for the turret portion of the autosentry to die
			ent->nextTrain->pka_flags |= PKAEF_AUTOSENTRYDEATH;
		}
	}
}

/*
=================
AutoSentryAimed
=================
*/
qboolean AutoSentryAimed ( gentity_t *ent, gentity_t *enemy_target ) {
	vec3_t		hold_vec;
	float		dot;
	vec3_t		sentry_forward;
//	vec3_t		hold_angles;
//	vec3_t		sentry_forward_2; 	//PKMOD - Ergodic 01/13/01 - remove this code 
	//PKMOD - Ergodic 06/14/02 - debug Personal Sentry
	int			dot_int;


	//PKMOD - Ergodic 12/11/00 - can't aim at invisible clients
	//		Code is here if autosentry is currently firing at a target
	//		that becomes invisible, then shooting will stop
	//PKMOD - Ergodic 04/06/01 - only check for invisible clients
	if ( enemy_target->client ) {
		if ( enemy_target->client->ps.powerups[PW_INVIS] ) {
			//PKMOD - Ergodic 01/13/01 - debug (inactive)
//			Com_Printf( "AutoSentryAimed - invis\n" );
			return qfalse;
		}
	}

	if ( ent->s.eType == ET_AUTOSENTRY_TURRET ) {
		VectorSubtract( enemy_target->r.currentOrigin, ent->r.currentOrigin, hold_vec );
		//PKMOD - Ergodic 01/13/01 - Turret is 8 units above of the floor, remove the difference
		hold_vec[2] -= 8;
	}
	else {
		VectorSubtract( enemy_target->r.currentOrigin, ent->r.currentOrigin, hold_vec );
	}


	//PKMOD - Ergodic 01/13/01 - debug inactive
//	VectorCopy ( hold_vec, debug_vec );

	//PKMOD - Ergodic 01/13/01 - remove this code 
//	vectoangles ( hold_vec, hold_angles );
//	AngleVectors ( hold_angles, sentry_forward_2, NULL, NULL );

	//PKMOD - Ergodic 05/19/02 - remove this code and add anglevectors 
//	vectoangles( ent->s.angles, hold_angles );

	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors ( hold_angles, sentry_forward, NULL, NULL );
	//PKMOD - Ergodic 05/19/02 - remove this code and add anglevectors 
//	AngleVectorsForward( hold_angles, sentry_forward );
	AngleVectorsForward( ent->s.angles, sentry_forward );
	
	VectorNormalize ( hold_vec );
	dot = DotProduct (hold_vec, sentry_forward);

//	AngleVectors ( hold_vec, sentry_forward_o, NULL, NULL );


//	VectorCopy( ent->s.angles, hold_angles2 );
//	VectorNormalize ( hold_angles2 );
//	vectoangles( hold_angles2, hold_angles );
//	AngleVectors ( hold_angles, sentry_forward, NULL, NULL );
	//PKMOD - Ergodic 12/04/00 - debug inactive
//	Com_Printf("AutoSentryFindTarget - forward_2>%s<, forward>%s<\n", alt_vtos(sentry_forward_2), alt_vtos(sentry_forward) );
	
//	dot = DotProduct (hold_vec, ent->s.angles);

	//PKMOD - Ergodic 06/14/02 - debug Personal Sentry
	dot_int = dot * 100;
	
	if ( dot > AUTOSENTRY_INFRONT ) {
		//PKMOD - Ergodic 09/28/02 - debug Personal Sentry (inactive)
//		Com_Printf("AutoSentryFindTarget Infront - target>%s<, dot_int>%d<\n", enemy_target->client->pers.netname, dot_int );
//		Com_Printf("AutoSentryFindTarget Infront - health>%d<, forward>%s<, s.angles>%s<\n", ent->health, alt_vtos(sentry_forward), alt_vtos(ent->s.angles) );
		return qtrue;
	}
	else {
		//PKMOD - Ergodic 09/28/02 - debug Personal Sentry (inactive)
//		Com_Printf("Behind - dot_int>%d<, hold_vec>%s<\n", dot_int, alt_vtos(hold_vec) );
//		Com_Printf("Behind - forward>%s<, s.angles>%s<\n", alt_vtos(sentry_forward), alt_vtos(ent->s.angles) );
		return qfalse;
	}
}


/*
=================
AutoSentry_Fire
=================
*/
//PKMOD - Ergodic 12/03/00 - modify routine so damage is externally quaded
void AutoSentry_Fire (gentity_t *ent, vec3_t target_direction ) {
	vec3_t		muzzlePoint;
	vec3_t		sentry_forward;
	vec3_t		sentry_right;
	vec3_t		sentry_up;
	gentity_t	*bolt;

	//PKMOD - Ergodic 01/23/01 - debug (inactive) muzzle location on model 
//	gentity_t		*debug_tent;

	//PKMOD - Ergodic 12/28/00 - Turn ON the autosentry firing flag
	ent->pka_flags |= PKAEF_AUTOSENTRYFIRING;
	//PKMOD - Ergodic 04/12/01 - modify so that time2 will be a packed variable
	ent->s.time2 |= 1; //co-opt the time2 variable for communication to cgame

	AngleVectors ( target_direction, sentry_forward, sentry_right, sentry_up);

	//PKMOD Ergodic 04/09/01 - debug (inactive)
//	Com_Printf("AutoSentry_Fire - Before t-d:%s , f: %s\n", alt_vtos( target_direction ), alt_vtos( sentry_forward ) );

	VectorNormalize2( target_direction, sentry_forward );

	//PKMOD Ergodic 04/09/01 - debug (inactive)
//	Com_Printf("AutoSentry_Fire - After t-d:%s , f: %s\n", alt_vtos( target_direction ), alt_vtos( sentry_forward ) );

	//PKMOD - Ergodic 01/23/01 - calculate the muzzle points 
	VectorCopy( ent->r.currentOrigin, muzzlePoint );
//	VectorMA( muzzlePoint, 12, sentry_up, muzzlePoint ); 	//04/06/01 - was 6
	muzzlePoint[2] += 10;

	VectorMA( muzzlePoint, 30, sentry_forward, muzzlePoint );	//04/06/01 - was 4

	//PKMOD Ergodic 04/09/01 - debug (inactive)
//	Com_Printf("AutoSentry_Fire - before s_r muzzlePoint:%s \n", alt_vtos( muzzlePoint ) );
	
	if ( rand() % 2 )		//random value {0, 1}
		VectorMA( muzzlePoint, 11, sentry_right, muzzlePoint );
	else
		VectorMA( muzzlePoint, -11, sentry_right, muzzlePoint );

	//PKMOD Ergodic 04/09/01 - debug (inactive)
//	Com_Printf("AutoSentry_Fire - after s_r muzzlepoint:%s , r: %s\n", alt_vtos( muzzlePoint ), alt_vtos( sentry_right ) );


/*
//	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	VectorCopy( ent->r.currentOrigin, muzzlePoint );
	muzzlePoint[2] += 5;	//was 30, Ergodic 01/13/01 
	VectorMA( muzzlePoint, 14, sentry_forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
*/
	
	//PKMOD - Ergodic 01/23/01 - debug (inactive) muzzle location on model 
//	debug_tent = G_TempEntity( muzzlePoint, EV_COORD );
//	debug_tent->s.weapon = WP_SENTRY;
//	debug_tent->s.otherEntityNum = ent->parent->s.number;


	//PKMOD - Ergodic 01/13/01 - add autosentry fire sounds
	G_AddEvent( ent, EV_AUTOSENTRY_FIRE, 0 );

//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
//	damage *= s_quadFactor;

	bolt = G_Spawn();
	bolt->classname = "autosentry_missile";
	bolt->nextthink = level.time + 5000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_SENTRY;
	bolt->r.ownerNum = ent->parent->s.number;
	bolt->parent = ent->parent;

	//PKMOD - Ergodic 10/23/02 - add (turret) ent info so that it can be ignored by autosentry missile
	bolt->prevTrain = ent;

	//PKMOD - Ergodic 04/12/01 add quad affects
	if ( ent->s.time2 & 2 ) {
		bolt->damage = ent->damage * g_quadfactor.value;
	}
	else {
		bolt->damage = ent->damage;
	}

//	bolt->splashDamage = 15;		//PKMOD - no splash damage
//	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_SENTRY;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( muzzlePoint, bolt->s.pos.trBase );
//	VectorScale( sentry_forward, 200, bolt->s.pos.trDelta );	//debug speed (200)
	VectorScale( sentry_forward, 1500, bolt->s.pos.trDelta );	//debug speed (200)
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (muzzlePoint, bolt->r.currentOrigin);

	//PKMOD - Ergodic 04/09/01 - store the "self" of the turret in "splashRadius"
	//			Launching turret entity number is stored in splashRadius (hack)
	bolt->splashRadius = ent->s.number;

}

/*
=================
AutoSentryAttack_Think
=================
*/
void AutoSentryAttack_Think ( gentity_t *ent ) {
	vec3_t		hold_vec;
	float		dist;
	qboolean	fire_sentry;
	//PKMOD Ergodic - 10/22/02 - hold target origin (raise it higher so turret will fire level)
	vec3_t		target_origin;

	//PKMOD Ergodic - 01/23/01, check if sentry should die
	if ( ent->pka_flags & PKAEF_AUTOSENTRYDEATH ) {
		ent->think = AutoSentry_TurretDeath;
		ent->nextthink = level.time + 200;  //was 200
		trap_LinkEntity( ent );
		return;
	}

	//PKMOD - Ergodic 04/12/01 add quad affects
	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		ent->s.time2 |= 2;	//for quad effects on the cgame side
	}
	else {
		ent->s.time2 &= ~2;	//turn off quad effects on the cgame side
	}

	//default logic: set to true and attempt to prove false
	fire_sentry = qtrue;

	//is the target still visible?
	if( !CanDamage (ent->target_ent, ent->r.currentOrigin) )  
		fire_sentry = qfalse;

	//check target's game status
	if (( ent->target_ent->client->ps.pm_type == PM_DEAD ) || ( ent->target_ent->s.eFlags == EF_DEAD )) 
		fire_sentry = qfalse;

	//PKMOD Ergodic 12/04/00 - debug inactive 
//	if ( ( rand() % 1000) > 800 ) {
//		Com_Printf("AutoSentryAttack_Think - STAT_HEALTH>%d<, health>%d<\n", ent->target_ent->client->ps.stats[STAT_HEALTH], ent->target_ent->health );
//	}

	//check target's health status
	if (( ent->target_ent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ent->target_ent->health <= 0 ))
		fire_sentry = qfalse;


	//PKMOD Ergodic - 10/22/02 - hold target origin (raise it higher so turret will fire level)
	VectorCopy( ent->target_ent->r.currentOrigin, target_origin );
	target_origin[2] += 10;		//raise target origin up a little

	//is the target still in range?
	VectorSubtract( target_origin, ent->r.currentOrigin, hold_vec );
	dist = VectorLength( hold_vec );
	if ( dist >= AUTOSENTRY_RANGE )		//If too far then forget it 
		fire_sentry = qfalse;

	//is target infront of the autosenty
	if ( !AutoSentryAimed( ent, ent->target_ent ) )
		fire_sentry = qfalse;

	if ( fire_sentry ) {
		//set new aim at target	
		//PKMOD - Ergodic 10/24/02 - set angles to where we are pointing
		vectoangles ( hold_vec, ent->s.angles );

		//PKMOD - Ergodic 12/14/02 - put a max PITCH on the tracking AUTOSENTRY
		if ( ent->s.angles[0] < -180 ) {	//When AutoSenty is pointed DOWNWARDS
			if ( ent->s.angles[0] > ( AUTOSENTRY_MAX_PITCH - 360 ) ) {	//oversized PITCH is Truncated
				ent->s.angles[0] = ( AUTOSENTRY_MAX_PITCH - 360 );
			}
		}
		else { //When AutoSenty is pointed UPWARDS
			if ( ent->s.angles[0] < - AUTOSENTRY_MAX_PITCH ) {	//oversized PITCH is Truncated
				ent->s.angles[0] = - AUTOSENTRY_MAX_PITCH;
			}
		}

		//Fire at target at the hold_vec direction
		AutoSentry_Fire ( ent, hold_vec );

		//PKMOD - Ergodic 04/06/01 - debug new missile effects
		ent->nextthink = level.time + 200;
//		ent->nextthink = level.time + 800; //debug was 100
	}
	else {
		//PKMOD Ergodic 05/24/02 - debug (inactive)
//		Com_Printf("AutoSentryAttack_Think - no fire, ent->s.angles>%s<\n\n", alt_vtos(ent->s.angles) );

		//PKMOD - Ergodic 12/28/00 - Turn OFF the autosentry firing flag
		ent->pka_flags &= ~( PKAEF_AUTOSENTRYFIRING );
		//PKMOD - Ergodic 04/12/01 - modify so that time2 will be a packed variable
		ent->s.time2 &= ~1; //co-opt the time2 variable for communication to cgame

		//PKMOD - Ergodic 02/05/02 - Reset the base angle and time for auto-rotation
//		AngleVectors ( ent->s.angles, hold_autosentry_forward, NULL, NULL );
//		VectorCopy( hold_autosentry_forward, ent->s.apos.trBase );
		//PKMOD - Ergodic 05/21/02 - Reset the base angle and time for auto-rotation
//		vectoangles( ent->s.angles, ent->s.apos.trBase );
//		AngleVectors ( ent->s.angles, hold_autosentry_forward, NULL, NULL );
		VectorCopy( ent->s.angles, ent->s.apos.trBase );
		ent->s.apos.trTime = level.time;

		//linger for a moment
		ent->count++;
		if ( ent->count > AUTOSENTRY_DETERMINATION ) {
			ent->target_ent = NULL;
			ent->nextthink = level.time + 200; //was 100

			ent->think = AutoSentryTurret_Think;
		}
		else
			ent->nextthink = level.time + 200;  //was 200
		
	}

	trap_LinkEntity( ent );

}


/*
=============
Find_Sentry

PKMOD - Ergodic 04/06/01 - Searches all active entities for the 
		next one that is an autosentry turret
		from->s.eType == ET_AUTOSENTRY_TURRET

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
gentity_t *Find_Sentry (gentity_t *from ) {

	if (!from)
		from = g_entities;
	else
		from++;

	for ( ; from < &g_entities[level.num_entities] ; from++)
	{
		if (!from->inuse)
			continue;

		if ( from->s.eType != ET_AUTOSENTRY_TURRET )
			continue;

		return from;
	}

	return NULL;
}


/*
=================
AutoSentryFindTarget_Think
=================
*/
qboolean AutoSentryFindTarget ( gentity_t *ent ) {
	gentity_t	*ext_ent;
	float		dist;
//	vec3_t		mins, maxs;
	vec3_t		dist_v;
//	int			i;
	int			indx;
	gentity_t	*enemy_target;
	int			enemy_distance;

	//Ergodic debug inactive
//Com_Printf("AutoSentryFindTarget - origin:%s, angles:%s\n", vtos(ent->r.currentOrigin),vtos(ent->r.currentAngles));

	enemy_distance = AUTOSENTRY_RANGE;
	enemy_target = NULL;

//PKMOD - Ergodic 01/13/01 - we don't need to do this detail
//	for ( i = 0 ; i < 3 ; i++ ) {
//		mins[i] = ent->r.currentOrigin[i] - AUTOSENTRY_RANGE;
//		maxs[i] = ent->r.currentOrigin[i] + AUTOSENTRY_RANGE;
//	}

	//PKMOD - Ergodic 01/10/01 - Change logic to just look for clients
	//				all clients are at the beginning of the entity list
	ext_ent = &g_entities[0];
	for ( indx =0 ; indx < MAX_CLIENTS  ; indx++, ext_ent++) {
		if ( !ext_ent->inuse ) {
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] not in use\n", indx );
			continue;
		}

		//PKMOD - Ergodic 02/18/01 - if team game, don't attack teammate
		//				note: OnSameTeam insures g_gametype.integer >= GT_TEAM 
		if ( OnSameTeam (ext_ent, ent->parent ) )
			continue;

		//clients are immune to autosentry upon respawning or teleporting
		//PKMOD Ergodic - 04/07/01 - rename variable to add autosentry
		//PKMOD Ergodic - 06/10/02 - Personal Sentry does not use immune time
		if ( ent->s.eType == ET_AUTOSENTRY_TURRET )
			if ( ext_ent->BearTrap_Autosentry_ImmuneTime > level.time ) {
				//PKMOD Ergodic - 12/18/03 - immune time is negated when owning player has HASTE powerup
				if ( ent->parent->client->ps.powerups[PW_HASTE] == 0 )
					continue;
			}

//PKMOD - Ergodic 01/13/01 - we don't need to do this detail
		// find the distance from the edge of the bounding box
//		for ( i = 0 ; i < 3 ; i++ ) {
//			if ( ent->r.currentOrigin[i] < ext_ent->r.absmin[i] ) {
//				v[i] = ext_ent->r.absmin[i] - ent->r.currentOrigin[i];
//			} else if ( ent->r.currentOrigin[i] > ext_ent->r.absmax[i] ) {
//				v[i] = ent->r.currentOrigin[i] - ext_ent->r.absmax[i];
//			} else {
//				v[i] = 0;
//			}
//		}

		VectorSubtract( ent->r.currentOrigin, ext_ent->r.currentOrigin, dist_v );
		dist = VectorLength( dist_v );

		//target the closest enemy
		if ( dist >= enemy_distance ) {		//test target distance against closest enemy
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] too distant\n", indx );
			continue;
		}

		if( !CanDamage (ext_ent, ent->r.currentOrigin) ) { //If not visible then forget it
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] can not damage\n", indx );
			continue;
		}

		//check target's game status
		if (( ext_ent->client->ps.pm_type == PM_DEAD ) || ( ext_ent->s.eFlags == EF_DEAD )) {
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] is dead\n", indx );
			continue;
		}

	//PKMOD Ergodic 12/04/00 - debug inactive
//		if ( (rand() % 1000) > 8 ) {
//			Com_Printf("AutoSentryFindTarget - STAT_HEALTH>%d<, health>%d<\n", ext_ent->client->ps.stats[STAT_HEALTH], ext_ent->health );
//		}

		//check target's health status
		if (( ext_ent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ext_ent->health <= 0 )) {
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] has no health\n", indx );
			continue;
		}

		//PKMOD - Ergodic 07/02/01 - don't fire on spectators
		//check clint's session type
		if ( ext_ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
 			//PKMOD - Ergodic 07/02/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] is a spectator\n", indx );
			continue;
		}


//>>>>>>>>>>>>>>>>
//	vectoangles( cent->lerpAngles, hold_dir);

		//PKMOD Ergodic 12/02/00 - debug inactive
//	if ( (rand() % 1000) > 975 ) {
//		debug_yaw = hold_dir[1];
//		Com_Printf("CG_AutoSentry_Turret - debug_yaw>%d<\n", debug_yaw );
//	}

//	hold_dir[0] = 0;
//	hold_dir[1] -= 90;  //offset
//	hold_dir[2] = 0;
//	AnglesToAxis( hold_dir, ent.axis );

//>>>>>>>>>>>>>>>>
		//is target infront of the autosenty
		if ( !AutoSentryAimed( ent, ext_ent ) ) {
 			//PKMOD - Ergodic 06/13/02 - debug (inactive)
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] is not aimed upon\n", indx );
			continue;
		}

		//closest enemy has been detected, now store the enemy information
		enemy_distance = dist;
		enemy_target = ext_ent;

	}

	//did autosentry detect a target?
	if ( enemy_distance < AUTOSENTRY_RANGE ) {
		ent->target_ent = enemy_target;
		ent->count = 0; //linger counter

		//PKMOD - Ergodic 01/13/01 - debug inactive
//		Com_Printf("AutoSentryFindTarget - target>%s<\n", enemy_target->client->pers.netname );

		//PKMOD Ergodic - 06/10/02 - Personal Sentry will use a different attack_think
		if ( ent->s.eType == ET_AUTOSENTRY_TURRET )
			ent->think = AutoSentryAttack_Think;
		else {
 			//PKMOD - Ergodic 06/14/02 - debug personal sentry firing (inactive)
//			Com_Printf( "AutoSentryFindTarget - calling PersonalSentryAttack_Think\n" );
			ent->think = PersonalSentryAttack_Think;
		}
		return qtrue;
	}

	//+++
	//
	// HERE IF NO CLIENT DETECTED
	//
	//+++

	//PKMOD - Ergodic 04/06/01 - here we have not found a client target,
	//			search for an autosentry target

	enemy_distance = AUTOSENTRY_RANGE;
	enemy_target = NULL;

	ext_ent = NULL;
	while ( ( ext_ent = Find_Sentry( ext_ent ) ) != NULL ) {
		if ( !ext_ent->inuse ) {
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent not in use\n" );
			continue;
		}

		//test same owner
		if ( ext_ent->parent == ent->parent )
			continue;

		//PKMOD - Ergodic 02/18/01 - if team game, don't attack teammate
		//				note: OnSameTeam insures g_gametype.integer >= GT_TEAM 
		if ( OnSameTeam (ext_ent->parent, ent->parent ) )
			continue;

		VectorSubtract( ent->r.currentOrigin, ext_ent->r.currentOrigin, dist_v );
		dist = VectorLength( dist_v );

		//target the closest enemy
		if ( dist >= enemy_distance ) {		//test target distance against closest enemy
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent too distant\n" );
			continue;
		}

		if( !CanDamage (ext_ent, ent->r.currentOrigin) ) { //If not visible then forget it
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent[%d] can not damage\n", indx );
			continue;
		}

		if ( !AutoSentryAimed( ent, ext_ent ) ) {
 			//PKMOD - Ergodic 01/13/01 - debug inactive
//			Com_Printf( "AutoSentryFindTarget - ext_ent is not aimed upon a sentry\n" );
			continue;
		}

		//closest enemy has been detected, now store the enemy information
		enemy_distance = dist;
		enemy_target = ext_ent;

	}


		//did autosentry detect a target?
	if ( enemy_distance < AUTOSENTRY_RANGE ) {
		ent->target_ent = enemy_target;
		ent->count = 0; //linger counter

		//PKMOD - Ergodic 01/13/01 - debug inactive
//		Com_Printf("AutoSentryFindTarget - target>%s<\n", enemy_target->client->pers.netname );

		//PKMOD Ergodic - 06/10/02 - Personal Sentry will use a different attack_think
		if ( ent->s.eType == ET_AUTOSENTRY_TURRET )
			ent->think = AutoSentryAttack_Think;
		else
			ent->think = PersonalSentryAttack_Think;
		return qtrue;
	}

	//PKMOD - Ergodic 01/13/01 - debug inactive
//	Com_Printf("AutoSentryFindTarget - no target\n" );

	return qfalse;

}

/*
=================
AutoSentryTurret_Rotate
//PKMOD - Ergodic 02/01/02 - New rotation code
=================
*/
void AutoSentryTurret_Rotate ( gentity_t *ent ) {

//	int		hold_yaw;
	vec3_t	hold_angles;
	int		hold_pitch;

	//PKMOD - Ergodic 05/22/02 - debug vars (inactive)
//	vec3_t	hold_in;
//	vec3_t	hold_out;
//	int		mod_type;	//0 At zero, No change,
						//1 Less than zero force to zero,
						//2 Less than zero add 0.1,
						//3 Greater than zero force to zero,
						//4 Greater than zero subtract 0.1

//	vec3_t	hold_forward;
//	vec3_t	hold_right;
//	vec3_t	hold_up;
//	int		hold_pitch;
//	vec3_t	amove;
//	vec3_t	hold_vector1;
//	vec3_t	hold_vector2;
//	vec3_t	hold_vector3;
//	vec3_t	hold_vector4;
//	vec3_t	hold_vector5;

	//exit is autosentry is currently firing
	if ( ent->pka_flags & PKAEF_AUTOSENTRYFIRING )
		return;

	//PKMOD - Ergodic 05/23/02 - check vertical aiming
	//PKMOD - Ergodic 05/24/02 - use s.angles instead of s.apos.trBase (silly me)
	//PKMOD - Ergodic 12/14/02 - [NOTE: AutoSentry PITCH is always negative! 
	if ( ent->s.angles[0] ) {
		hold_pitch = ent->s.angles[0];

		//PKMOD - Ergodic 12/14/02 - debug (inactive)
		//Com_Printf("AutoSentryTurret_Rotate - hold_pitch>%d<, s.angles>%s<\n", hold_pitch, alt_vtos( ent->s.angles ) );


		//PKMOD - Ergodic 12/14/02 - normalize the PITCH 
		if ( hold_pitch < -180 ) {	//When AutoSenty is pointed DOWNWARDS
			ent->s.apos.trDelta[0] = - AUTOSENTRY_VERTICAL_VELOCITY;
			if ( hold_pitch > ( AUTOSENTRY_MAX_PITCH - 360 ) ) {	//oversized PITCH is Truncated
				ent->s.apos.trBase[0] = ( AUTOSENTRY_MAX_PITCH - 360 );
			}
			else if ( hold_pitch < ( AUTOSENTRY_VERTICAL_VELOCITY - 360 ) ) {		//small PITCH is ZERO
				ent->s.apos.trBase[0] = 0;
				ent->s.apos.trDelta[0] = 0;
				ent->s.angles[0] = 0;
			}
		}
		else { //When AutoSenty is pointed UPWARDS
			ent->s.apos.trDelta[0] = - AUTOSENTRY_VERTICAL_VELOCITY;
			if ( hold_pitch < - AUTOSENTRY_MAX_PITCH ) {	//oversized PITCH is Truncated
				ent->s.apos.trBase[0] = - AUTOSENTRY_MAX_PITCH;
			}
			else if ( hold_pitch < - AUTOSENTRY_VERTICAL_VELOCITY ) {		//small PITCH is ZERO
				ent->s.apos.trBase[0] = 0;
				ent->s.apos.trDelta[0] = 0;
				ent->s.angles[0] = 0;
			}
		}

	}
	else { 
		ent->s.apos.trDelta[0] = 0;
	}

	//PKMOD - Ergodic 02/01/02 - New rotation code
	BG_EvaluateTrajectory( &ent->s.apos, level.time, hold_angles );
	hold_angles[2] = 0;		//roll "tilt" should always be 0
	//PKMOD - Ergodic 05/21/02 - set the angle that sentry is facing
	VectorCopy( hold_angles, ent->s.angles );

	//PKMOD - Ergodic 02/09/02 - debug pitch rotation set the rotation velocity
//	AngleVectors (hold_angles, hold_forward, hold_right, hold_up);

	//PKMOD - Ergodic 02/03/02 - debug current angles (inactive)
//	if ( ent->count < 20 ) {
//		VectorScale( ent->s.apos.trBase, 1000, hold_vector1 );
//		VectorScale( ent->s.angles, 1000, hold_vector2 );
//		VectorScale( hold_angles, 1000, hold_vector3 );
//		VectorScale( hold_forward, 1000, hold_vector4 );
//		AngleVectors (ent->s.angles, hold_vector5, hold_right, hold_up);
//		VectorScale( hold_vector5, 1000, hold_vector5 );
//		Com_Printf("AutoSentryTurret_Rotate: 1000*trBase>%s<, 1000*s.angles>%s<, 1000*hold_angles>%s<, 1000*hold_forward>%s<, 1000*s.angles_forward>%s<\n", vtos( hold_vector1 ), vtos( hold_vector2 ), vtos( hold_vector3 ), vtos( hold_vector4 ),  vtos( hold_vector5 ) );
//		ent->count += 1;
//	}

//	}

//	AngleVectors (hold_angles, hold_forward, hold_right, hold_up);

	//PKMOD - Ergodic 02/03/02 - Add delta to base angles
//	VectorAdd( hold_forward, ent->s.angles, ent->s.angles );

	//PKMOD - Ergodic 02/07/02 - debug pitch rotation set the rotation velocity (inactive)
//	VectorAdd( hold_forward, hold_right, hold_forward );


	//PKMOD - Ergodic 02/08/02 - set the angle that sentry is facing
//	VectorCopy( hold_forward, ent->s.angles );

//	VectorCopy( hold_forward, ent->s.angles );

/*
	//PKMOD - Ergodic 05/22/02 - debug pitch problems
	VectorScale( hold_angles, 100, hold_in );
	mod_type = 0;	//DEBUG: 0 At zero, No change,

	//PKMOD - Ergodic 05/21/02 - set the angle that sentry is facing
	if ( hold_angles[0] < 0 ) {
		if ( hold_angles[0] > -0.1 ) {
			mod_type = 1;	//DEBUG: 1 Less than zero force to zero
			hold_angles[0] = 0;
		} else {
			mod_type = 2;	//DEBUG: 2 Less than zero add 0.1
			hold_angles[0] = hold_angles[0] + 0.1;
		}
	} else {
		if ( hold_angles[0] > 0 ) {
			if ( hold_angles[0] < 0.1 ) {
				mod_type = 3;	//DEBUG: 3 Greater than zero force to zero
				hold_angles[0] = 0;
			} else {
				mod_type = 4;	//DEBUG: 3 Greater than zero force to zero
				hold_angles[0] = hold_angles[0] - 0.1;
			}
		}
	}
*/

	//PKMOD - Ergodic 05/22/02 - debug pitch problems (inactive)
//	VectorScale( hold_angles, 100, hold_out );

	//PKMOD - Ergodic 05/22/02 - debug pitch problems (inactive)
//	if ( (rand() % 1000) > 900 ) {
//		Com_Printf("AutoSentryTurret_Rotate - type>%d<, in>%s<, out>%s<\n", mod_type, vtos( hold_in ), vtos( hold_out ) );
//	}

//	if (hold_angles[0] != 9999) //nearly always true
//		return;

	//rotate turret
//	vectoangles( ent->s.angles, hold_angles );

//	hold_yaw = hold_angles[YAW] + AUTOSENTRY_SWEEP;

//	if ( hold_yaw > 359 )
//		hold_yaw -= 360;

//	hold_angles[YAW] = hold_yaw;

	//PKMOD - Ergodic 01/24/01 - move the direction's "altimeter" to zero
	//PKMOD - Ergodic 03/22/01 - BUG FIX - use PITCH and not ROLL (duh)
//	hold_pitch = hold_angles[PITCH];

	//PKMOD - Ergodic 03/22/01 debug pitch (inactive)
//	Com_Printf("Turret_Think - pitch in:>%d<, ", hold_pitch );


//	if ( hold_pitch != 0 ) {	//if roll is not yet at zero then angle sentry to zero
		//PKMOD - Ergodic 03/22/01 - create big sweep if needed
//		if ( abs( hold_pitch ) > AUTOSENTRY_BIG_SWEEP )
//			hold_pitch = 0;
//		else {
//			if ( hold_pitch > 0 ) { //pitch greater than zero
//				hold_pitch -= AUTOSENTRY_SWEEP;
//				if ( hold_pitch < 0 )	//did we overcompensate
//					hold_pitch = 0;
//			}
//			else {	//pitch less than zero
//				hold_pitch += AUTOSENTRY_SWEEP;
//				if ( hold_pitch > 0 )	//did we overcompensate
//					hold_pitch = 0;
//			}
//		}
//		hold_angles[PITCH] = hold_pitch;
//	}

	//PKMOD - Ergodic 03/22/01 debug pitch (inactive)
//	Com_Printf("Turret_Think - pitch out:>%d<\n", hold_pitch );

//	AngleVectors (hold_angles, hold_forward, hold_right, hold_up);

//	VectorCopy( hold_forward, ent->s.angles );
}


/*
=================
AutoSentryTurret_Think
=================
*/
void AutoSentryTurret_Think ( gentity_t *ent ) {

//	int		hold_yaw;
//	vec3_t	hold_angles;
//	vec3_t	hold_forward;
//	vec3_t	hold_right;
//	vec3_t	hold_up;
//	int		hold_pitch;
	//PKMOD - Ergodic 01/31/01 - test moving the sentry
//	vec3_t	hold_origin;

	//owner is immune to the autosentry for a moment
//	if ( (ent == ent->parent) && (ent->wait > level.time) ) {
//		return;
//	}

	//PKMOD - Ergodic 03/26/01 - add autosentry ping sound
	if (ent->wait < level.time) {
		G_AddEvent( ent, EV_AUTOSENTRY_PING, 0 );
		ent->wait = level.time + AUTOSENTRY_PING_TIME + rand() % 200;
	}

	//PKMOD - Ergodic 04/12/01 add quad affects
	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		ent->s.time2 |= 2;	//for quad effects on the cgame side
	}
	else {
		ent->s.time2 &= ~2;	//turn off quad effects on the cgame side
	}

	//PKMOD - Ergodic 01/31/02 - move this logic to its own function
/*
	//rotate turret
	vectoangles( ent->s.angles, hold_angles );

	hold_yaw = hold_angles[YAW] + AUTOSENTRY_SWEEP;

	if ( hold_yaw > 359 )
		hold_yaw -= 360;

	hold_angles[YAW] = hold_yaw;

	//PKMOD - Ergodic 01/24/01 - move the direction's "altimeter" to zero
	//PKMOD - Ergodic 03/22/01 - BUG FIX - use PITCH and not ROLL (duh)
	hold_pitch = hold_angles[PITCH];

	//PKMOD - Ergodic 03/22/01 debug pitch (inactive)
//	Com_Printf("Turret_Think - pitch in:>%d<, ", hold_pitch );


	if ( hold_pitch != 0 ) {	//if roll is not yet at zero then angle sentry to zero
		//PKMOD - Ergodic 03/22/01 - create big sweep if needed
		if ( abs( hold_pitch ) > AUTOSENTRY_BIG_SWEEP )
			hold_pitch = 0;
		else {
			if ( hold_pitch > 0 ) { //pitch greater than zero
				hold_pitch -= AUTOSENTRY_SWEEP;
				if ( hold_pitch < 0 )	//did we overcompensate
					hold_pitch = 0;
			}
			else {	//pitch less than zero
				hold_pitch += AUTOSENTRY_SWEEP;
				if ( hold_pitch > 0 )	//did we overcompensate
					hold_pitch = 0;
			}
		}
		hold_angles[PITCH] = hold_pitch;
	}

	//PKMOD - Ergodic 03/22/01 debug pitch (inactive)
//	Com_Printf("Turret_Think - pitch out:>%d<\n", hold_pitch );

	AngleVectors (hold_angles, hold_forward, hold_right, hold_up);

	VectorCopy( hold_forward, ent->s.angles );
*/

	//PKMOD Ergodic - 01/23/01, check if sentry should die
	if ( ent->pka_flags & PKAEF_AUTOSENTRYDEATH ) {
		ent->think = AutoSentry_TurretDeath;
		ent->nextthink = level.time + 200;  //was 200
	}
	else {
		//reset the think function if target is detected 
		if ( AutoSentryFindTarget( ent ) )
			ent->nextthink = level.time + 200;  //was 200
		else
			ent->nextthink = level.time + 200;  //was 200
	}

	trap_LinkEntity( ent );

}

/*
=================
AutoSentryEnable_Think
split entity into two entities 1) base 2) turret
	turret location will be driven by base 
=================
*/
void AutoSentryEnable_Think ( gentity_t *ent ) {
	gentity_t	*bolt;
	vec3_t		hold_location;

	//change entity type to be only the base
	ent->s.eType = ET_AUTOSENTRY_BASE;

	//create the turret
	bolt = G_Spawn();
	bolt->classname = "autosentry";
	bolt->nextthink = level.time + 300; //schedule turret think 
	//PKMOD Change think function to match sentry behavior
	bolt->think = AutoSentryTurret_Think;
	bolt->nextTrain = NULL;

	//PKMOD - Ergodic 11/26/00 - set the direction
	VectorCopy ( ent->s.angles, bolt->s.angles );
	//PKMOD - Ergodic 01/31/02 - set the rotation velocity
	bolt->s.apos.trDelta[0] = 0;

	bolt->s.apos.trDelta[1] = AUTOSENTRY_ROTATE_VELOCITY;
	//PKMOD - Ergodic 02/07/02 - debug pitch rotation set the rotation velocity (inactive)
//	bolt->s.apos.trDelta[2] = 5;
	bolt->s.apos.trDelta[2] = 0;

	//PKMOD - Ergodic 02/03/02 - set the rotation base angle
//	VectorCopy ( ent->s.angles, bolt->s.apos.trBase );
	//PKMOD - Ergodic 02/04/02 - set the rotation base angle
	vectoangles( ent->s.angles, bolt->s.apos.trBase );

	bolt->s.eType = ET_AUTOSENTRY_TURRET;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_SENTRY;
	bolt->r.ownerNum = ent->r.ownerNum; //set this to the client owner
	bolt->parent = ent->parent;
	bolt->damage = ent->damage;
	bolt->methodOfDeath = MOD_SENTRY;
//	bolt->clipmask = MASK_SHOT;
	//PKMOD - Ergodic 12/09/00 - make it solid
//	bolt->r.contents = CONTENTS_BODY;
//	bolt->clipmask = MASK_PLAYERSOLID;
//	bolt->clipmask = MASK_PLAYERSOLID;
//	bolt->r.contents = CONTENTS_PLAYERCLIP;
	//PKMOD - Ergodic 01/25/01 - another try
//	bolt->r.contents = CONTENTS_BODY;
//	bolt->clipmask = MASK_PLAYERSOLID; //was MASK_SHOT;
//	bolt->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
//	bolt->clipmask = MASK_PLAYERSOLID;

//	bolt->clipmask = CONTENTS_BODY | MASK_SHOT | CONTENTS_TRIGGER; 

	//PKMOD - Ergodic 01/31/01 - test contents trigger after blowing sentry
//	bolt->r.contents = CONTENTS_TRIGGER;

	bolt->r.contents = CONTENTS_BODY;
	bolt->clipmask = MASK_SHOT; 

	//PKMOD - Ergodic 03/23/01 - add team parameters
	bolt->s.modelindex = ent->parent->client->sess.sessionTeam;

	//PKMOD - Ergodic 12/13/03 - set invisibility factor
	bolt->s.generic1 = 0;

	//PKMOD - Ergodic 12/10/00 - set missile size
	VectorSet (bolt->r.mins, -16, -16, -24);
	VectorSet (bolt->r.maxs, 16, 16, 32);

	
	//PKMOD - Ergodic 01/26/00 - set missile "absolute" size
	VectorCopy(bolt->r.mins, bolt->r.absmin);
	VectorCopy(bolt->r.maxs, bolt->r.absmax);

	//PKMOD - Ergodic 12/09/00 - debug size inactive
//	Com_Printf("AutoSentryEnable_Think - mins>%s<, maxs.%s<\n", vtos(bolt->r.mins), vtos(bolt->r.maxs));

	//PKMOD - Ergodic 12/28/00 - set the autosentry firing flag to zero
	//PKMOD Ergodic - 01/23/01, add flag for setting the turret portion of the autosentry to die (Set to off)
	//PKMOD Ergodic - 01/30/01, add flag for setting the turret portion of the autosentry to Free (Set to off)
	bolt->pka_flags &= ~( PKAEF_AUTOSENTRYFIRING | PKAEF_AUTOSENTRYDEATH | PKAEF_AUTOSENTRYFREE );
	bolt->s.time2 = 0; //co-opt the time2 variable for communication to cgame

	bolt->touch = AutoSentry_Touch;
	//PKMOD - Ergodic 01/22/01 - rename the die function for the turret to make it more specific
	bolt->die = AutoSentry_TurretDie;

	bolt->takedamage = qtrue;
	bolt->health = ent->health;  //03/08/01 set to base's health

	//PKMOD - Ergodic 01/26/00 - set move type
//	bolt->s.pos.trType = TR_STATIONARY;
	bolt->s.pos.trType = TR_LINEAR;
	//PKMOD - Ergodic 02/01/02 - set rotation
	bolt->s.apos.trType = TR_LINEAR;
	bolt->s.apos.trTime = level.time;

	//PKMOD - Ergodic 03/26/01 - set next time to ping
	bolt->wait = level.time + AUTOSENTRY_PING_TIME * 1.25;

	//PKMOD - Ergodic 02/03/02 - debug rotation (inactive)
//	bolt->count = 0;


	//PKMOD - Ergodic 01/26/00 - set the velocity
	VectorSet ( bolt->s.pos.trDelta, 0, 0, 0 );

	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame


	//set the location and vertical offset 
	VectorCopy ( ent->r.currentOrigin, hold_location );
	hold_location[2] += 16;
	//PKMOD - Ergodic 03/07/01 - debug sentry bounce bug (inactive)
//	hold_location[1] += 100;

	VectorCopy ( hold_location, bolt->r.currentOrigin );
	VectorCopy( hold_location, bolt->s.pos.trBase );

	//PKMOD - Ergodic 12/13/03 - Invisibility code - add base link to turret for
	bolt->prevTrain = ent;

	trap_LinkEntity (bolt);

	//set the link from autosentry base to autosentry turret
	ent->nextTrain = bolt;
	ent->nextthink = level.time + 300;
	ent->think = AutoSentryBase_Think;
	trap_LinkEntity( ent );

}

/*
=================
AutoSentryDeploy_Think
wait for sentry to deploy then enable tracking mechanism 
=================
*/
void AutoSentryDeploy_Think ( gentity_t *ent ) {

	if (ent->wait > level.time) {
		//deploy tracking in 3 seconds
		if ( ent->s.eType == ET_AUTOSENTRY_DEPLOY ) {
			ent->think = AutoSentryEnable_Think;
			ent->nextthink = level.time + 2000;
		}
		else {
			ent->nextthink = level.time + 300;
		}

		trap_LinkEntity( ent );
	}
	else {
		//Remove the Autosentry entity
		ent->nextthink = level.time + 100; 
		ent->think = AutoSentry_TurretDeath;
	}

}

/*
=================
fire_sentry

	Co OP variables:
		generic1: Autosentry invisibility factor 10/10/03

=================
*/
gentity_t *fire_autosentry (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "autosentry";
	bolt->nextthink = level.time + 500; //call Autosentry deploy
	//PKMOD Change think function to match sentry behavior
	bolt->think = AutoSentryDeploy_Think;

	//PKMOD - Ergodic 11/26/00 - set the direction
	VectorCopy ( dir, bolt->s.angles );

	//PKMOD - Ergodic 01/24/01 - Set the directon's "altimeter" to zero
	bolt->s.angles[2] = 0;

	//PKMOD - Ergodic 01/24/01 - debug s.angles inactive
//	Com_Printf("fire_autosentry - dir*1000>%s<\n", alt_vtos( dir ) );


	bolt->s.eType = ET_AUTOSENTRY_LAUNCH;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_SENTRY;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = AUTOSENTRY_DAMAGE;
	bolt->methodOfDeath = MOD_SENTRY;
	bolt->clipmask = MASK_SHOT;

	bolt->touch = AutoSentry_Touch;
	//PKMOD - Ergodic 01/22/01 - add new die function for autosentry base
	bolt->die = AutoSentry_BaseDie;

	bolt->takedamage = qtrue;
	bolt->health = 300 + rand() % 50;	//03/08/01 - set health to 325 + random amount

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	//PKMOD - Ergodic 08/30/01 - set the team fields so bots will not shoot their own team's autosentries
	//PKMOD - Ergodic 10/10/03 - BOT AI will use modelindex instead of generic1 in order to free generic1 for "cell damages"
	//	bolt->s.generic1 = self->client->sess.sessionTeam;
	//PKMOD - Ergodic 03/20/01 - add team parameters
	bolt->s.modelindex = self->client->sess.sessionTeam;

	//PKMOD - Ergodic 10/10/03 - set invisibility factor
	bolt->s.generic1 = 0;

	VectorNormalize (dir);

	VectorScale( dir, 400, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	//autosentry is standalone at the start and will split into two
	//		pieces after it is fully deployed
	bolt->nextTrain = NULL;

	bolt->wait = level.time + 45000 + 3000 * random(); //stay alive for 45 seconds + 3 random seconds

	trap_LinkEntity (bolt);

	return bolt;
}
//PKMOD

//PKMOD - Ergodic 12/16/02 - change Personal Sentry damage from 6 to 10
#define	PERSONALSENTRY_DAMAGE	10

/*
=================
PersonalSentry_Exit
=================
*/
void PersonalSentry_Exit ( gentity_t *ent ) {
	gentity_t	*teleport_tent;
	vec3_t		muzzlePoint;
	vec3_t		persentry_forward;
	vec3_t		persentry_right;
	vec3_t		persentry_up;

	// set aiming directions
	AngleVectors ( ent->parent->client->ps.viewangles, persentry_forward, persentry_right, persentry_up);

	//PKMOD - Ergodic 06/07/02 - calculate the muzzle points 
	VectorCopy( ent->parent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->parent->client->ps.viewheight;
	VectorMA( muzzlePoint, -1, persentry_forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
//	SnapVector( muzzlePoint );
	VectorMA( muzzlePoint, 15, persentry_up, muzzlePoint );
	VectorMA( muzzlePoint, -16, persentry_right, muzzlePoint );

	//create the teleport flash
	teleport_tent = G_TempEntity( muzzlePoint, EV_TELE_OUT_PERSONALSENTRY );
	teleport_tent->s.otherEntityNum = ent->s.number;

	ent->timestamp = 1;
}


/*
=================
PersonalSentry_Fire
=================
*/
void PersonalSentry_Fire (gentity_t *ent, vec3_t target_direction ) {
	vec3_t		muzzlePoint;
	gentity_t	*bolt;
	vec3_t		sentry_forward;

	//PKMOD - Ergodic 01/23/01 - debug (inactive) muzzle location on model 
//	gentity_t		*debug_tent;

	//PKMOD - Ergodic 06/14/02 - create forward displacement from gun
	VectorCopy( ent->r.currentOrigin, muzzlePoint );
	VectorNormalize2( target_direction, sentry_forward );
	//PKMOD - Ergodic 08/02/02 - previous scale factor was 16
	VectorMA( muzzlePoint, 14, sentry_forward, muzzlePoint );

	VectorNormalize2( target_direction, sentry_forward );

	//PKMOD - Ergodic 12/28/00 - Turn ON the autosentry firing flag
	ent->pka_flags |= PKAEF_AUTOSENTRYFIRING;
	//PKMOD - Ergodic 04/12/01 - modify so that time2 will be a packed variable
	ent->s.time2 = 251; //co-opt the time2 variable for communication to cgame

	//PKMOD Ergodic 04/09/01 - debug (inactive)
//	Com_Printf("AutoSentry_Fire - After t-d:%s , f: %s\n", alt_vtos( target_direction ), alt_vtos( sentry_forward ) );

	//PKMOD - Ergodic 01/13/01 - add Personal Sentry fire sounds
	G_AddEvent( ent, EV_FIRE_PERSONALSENTRY, 0 );

//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
//	damage *= s_quadFactor;

	bolt = G_Spawn();
	bolt->classname = "personalsentry_missile";
	bolt->nextthink = level.time + 5000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_SENTRY;			//weapon number and time2==251 determine type of missile
	bolt->r.ownerNum = ent->parent->s.number;
	bolt->parent = ent->parent;
	//PKMOD - Ergodic 04/12/02 - modify time2 to store autosentry/persentry determination
	bolt->s.time2 = 251; //co-opt the time2 variable for communication to cgame


	//PKMOD - Ergodic 06/12/02 add quad affects
 	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		bolt->damage = PERSONALSENTRY_DAMAGE * g_quadfactor.value;
	}
	else {
		bolt->damage = PERSONALSENTRY_DAMAGE;
	}

//	bolt->splashDamage = 15;		//PKMOD - no splash damage
//	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PERSONALSENTRY;	//08/02/02, set the correct mod
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( muzzlePoint, bolt->s.pos.trBase );
//	VectorScale( sentry_forward, 200, bolt->s.pos.trDelta );	//debug speed (200)
	VectorScale( sentry_forward, 1600, bolt->s.pos.trDelta );	//debug speed (200)
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (muzzlePoint, bolt->r.currentOrigin);

	//PKMOD - Ergodic 04/09/01 - store the "self" of the turret in "splashRadius"
	//			Launching turret entity number is stored in splashRadius (hack)
	bolt->splashRadius = ent->s.number;

}

/*
=================
PersonalSentryAttack_Think
=================
*/
void PersonalSentryAttack_Think ( gentity_t *ent ) {
	vec3_t		hold_vec;
	float		dist;
	qboolean	fire_sentry;
	vec3_t		muzzlePoint;
	vec3_t		persentry_forward;
	vec3_t		persentry_right;
	vec3_t		persentry_up;

	//PKMOD - Ergodic 08/02/02 - remove PS_THINK if player no longer has an active Personal Sentry powerup
	if ( ent->parent->client->ps.powerups[ PW_PERSENTRY ] <= 1 ) {
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 08/03/02 - If near end of life play the teleport exit
	if ( ( ent->parent->client->ps.powerups[ PW_PERSENTRY ] - level.time) < 300 ) {
		if (! ent->timestamp )
			PersonalSentry_Exit ( ent );
	}

	//set the origin and angles
	// set aiming directions
	VectorCopy ( ent->parent->client->ps.viewangles, ent->s.angles );

	AngleVectors ( ent->parent->client->ps.viewangles, persentry_forward, persentry_right, persentry_up );

	//PKMOD - Ergodic 06/07/02 - calculate the muzzle points 
	VectorCopy( ent->parent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, -1, persentry_forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
//	SnapVector( muzzlePoint );
	VectorMA( muzzlePoint, 5, persentry_up, muzzlePoint );
	VectorMA( muzzlePoint, -16, persentry_right, muzzlePoint );

	VectorCopy (muzzlePoint, ent->r.currentOrigin);

	//default logic: set to true and attempt to prove false
	fire_sentry = qtrue;

	//is the target still visible?
	if( !CanDamage (ent->target_ent, ent->r.currentOrigin) )  
		fire_sentry = qfalse;

	//check target's game status
	if (( ent->target_ent->client->ps.pm_type == PM_DEAD ) || ( ent->target_ent->s.eFlags == EF_DEAD )) 
		fire_sentry = qfalse;

	//check target's health status
	if (( ent->target_ent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ent->target_ent->health <= 0 ))
		fire_sentry = qfalse;

	//is the target still in range?
	VectorSubtract( ent->target_ent->r.currentOrigin, ent->r.currentOrigin, hold_vec );
	dist = VectorLength( hold_vec );
	if ( dist >= AUTOSENTRY_RANGE )		//If too far then forget it 
		fire_sentry = qfalse;

	//is target infront of the autosenty
	if ( !AutoSentryAimed( ent, ent->target_ent ) )
		fire_sentry = qfalse;

	if ( fire_sentry ) {

		//PKMOD - Ergodic 06/14/02 - debug personal sentry firing (inactive)
//		Com_Printf("PersonalSentryAttack_Think - calling PersonalSentry_Fire\n" );

		//Fire at target at the hold_vec direction
		PersonalSentry_Fire ( ent, hold_vec );

		ent->nextthink = level.time + 200;
	}
	else {

		//PKMOD - Ergodic 12/28/00 - Turn OFF the autosentry firing flag
		ent->pka_flags &= ~( PKAEF_AUTOSENTRYFIRING );

		//linger for a moment
		ent->count++;
		if ( ent->count > AUTOSENTRY_DETERMINATION ) {
			ent->target_ent = NULL;
			ent->nextthink = level.time + 200; //was 100

			ent->think = PersonalSentry_Think;
		}
		else
			ent->nextthink = level.time + 200;  //was 200
		
	}

	trap_LinkEntity( ent );

}

/*
=================
PersonalSentry_Think
PKMOD - Ergodic 06/09/02 - main think loop
=================
*/
void PersonalSentry_Think ( gentity_t *ent ) {

	vec3_t		muzzlePoint;
	vec3_t		persentry_forward;
	vec3_t		persentry_right;
	vec3_t		persentry_up;

	//PKMOD - Ergodic 06/13/02 - debug testing...(inactive)
//	Com_Printf( "PersonalSentry_Think\n" );
	//remove PS_THINK if player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		G_FreeEntity( ent );
		return;
	}

	//remove PS_THINK if player has died
	if (( ent->parent->client->ps.pm_type == PM_DEAD ) || ( ent->parent->s.eFlags == EF_DEAD )) {
		G_FreeEntity( ent );
		return;
	}

	//remove PS_THINK if player's health drops to, or below, zero 
	if (( ent->parent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ent->parent->health <= 0 ) ) {
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 08/03/02 - If near end of life play the teleport exit
	if ( ( ent->parent->client->ps.powerups[ PW_PERSENTRY ] - level.time) < 300 ) {
		if (! ent->timestamp )
			PersonalSentry_Exit ( ent );
	}

	//remove PS_THINK if player no longer has an active Personal Sentry powerup
	if ( ent->parent->client->ps.powerups[ PW_PERSENTRY ] <= 1 ) {
		G_FreeEntity( ent );
		return;
	}

	//set the origin and angles
	// set aiming directions
	VectorCopy ( ent->parent->client->ps.viewangles, ent->s.angles );

	AngleVectors ( ent->parent->client->ps.viewangles, persentry_forward, persentry_right, persentry_up );

	//PKMOD - Ergodic 06/07/02 - calculate the muzzle points 
	VectorCopy( ent->parent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, -1, persentry_forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
//	SnapVector( muzzlePoint );
	VectorMA( muzzlePoint, 5, persentry_up, muzzlePoint );
	VectorMA( muzzlePoint, -16, persentry_right, muzzlePoint );

	VectorCopy (muzzlePoint, ent->r.currentOrigin);

	//now try and find a target client
	//reset the think function if target is detected 
	if ( AutoSentryFindTarget( ent ) )
		ent->nextthink = level.time + 200;  //was 200
	else
		ent->nextthink = level.time + 200;  //was 200

	trap_LinkEntity( ent );


}

/*
=================
G_AddPersonalSentry
PKMOD - Ergodic 06/08/02 - setup personal sentry driver
=================
*/
void G_AddPersonalSentry( gentity_t *ent ) {
	gentity_t	*teleport_tent;
	vec3_t		muzzlePoint;
	vec3_t		persentry_forward;
	vec3_t		persentry_right;
	vec3_t		persentry_up;
	gentity_t	*personalSentry;


	// set aiming directions
	AngleVectors ( ent->client->ps.viewangles, persentry_forward, persentry_right, persentry_up);

	//PKMOD - Ergodic 06/07/02 - calculate the muzzle points 
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, -1, persentry_forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
//	SnapVector( muzzlePoint );
	VectorMA( muzzlePoint, 15, persentry_up, muzzlePoint );
	VectorMA( muzzlePoint, -16, persentry_right, muzzlePoint );

	//create the teleport flash
	teleport_tent = G_TempEntity( muzzlePoint, EV_TELE_IN_PERSONALSENTRY );
	teleport_tent->s.otherEntityNum = ent->s.number;

	//create the Personal Sentry entity
	personalSentry = G_Spawn();
	personalSentry->classname = "personalsentry";

	//set the angles
	VectorCopy ( ent->client->ps.viewangles, ent->s.angles );

	personalSentry->s.eType = ET_PERSONALSENTRY;
	personalSentry->clipmask = MASK_SHOT;
	personalSentry->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	personalSentry->s.weapon = WP_SENTRY;
	personalSentry->s.eFlags = EF_BOUNCE_HALF;
	personalSentry->r.ownerNum = ent->s.number;
	personalSentry->parent = ent;

	//PKMOD - Ergodic 03/23/01 - add team parameters
	personalSentry->s.modelindex = ent->client->sess.sessionTeam;

	//PKMOD - Ergodic 08/03/02 - add parameter to control exit teleport
	personalSentry->timestamp = 0;

	personalSentry->r.contents = CONTENTS_TRIGGER;
	personalSentry->parent = ent;
//	personalSentry->takedamage = qtrue;

	//PKMOD - Ergodic 03/11/01 - set the health for the personal sentry
	//personalSentry->health = 100 + rand() % 50;	//06/09/02 - set health to 100 + random amount
	
	personalSentry->methodOfDeath = MOD_PERSONALSENTRY;
//	personalSentry->splashMethodOfDeath = MOD_PERSONALSENTRY;
	personalSentry->clipmask = MASK_SHOT;
//	personalSentry->touch = PersonalSentryTouch;
//	personalSentry->die = PersonalSentryKill;
	personalSentry->target_ent = NULL;

	//PKMOD - Ergodic 08/05/01 - clear the PKA entity flags
	personalSentry->pka_flags = 0;

	//PKMOD - Ergodic 08/30/01 - set the team fields so bots will not shoot their own team's beartraps
	personalSentry->s.generic1 = ent->client->sess.sessionTeam;

	personalSentry->s.pos.trType = TR_LINEAR;
	//PKMOD - Ergodic 06/10/02 - use calculated muzzlepoint and not ent->s.pos.trBase
//	VectorCopy( ent->s.pos.trBase, personalSentry->s.pos.trBase );
	VectorCopy( muzzlePoint, personalSentry->s.pos.trBase );
	VectorClear( personalSentry->s.pos.trDelta );

	//PKMOD - Ergodic 06/10/02 - use calculated muzzlepoint and not ent->s.pos.trBase
//	VectorCopy (ent->s.pos.trBase, personalSentry->r.currentOrigin);
	VectorCopy( muzzlePoint, personalSentry->s.pos.trBase );

	//set next event
	personalSentry->nextthink = level.time + 200; //check for all parent status
	personalSentry->think = PersonalSentry_Think;
	//PKMOD - Ergodic 06/13/02 - debug testing...
//	personalSentry->nextthink = level.time + 3000; //in three seconds...
//	personalSentry->think = G_FreeEntity;

	trap_LinkEntity (personalSentry);
}



/*
=================
fire_chainlg
=================
*/
/*PKMOD - Ergodic 06/05/03 - remove dead code for CLG...
//PKMOD Move this 
gentity_t *fire_chainlg (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "flame";
	bolt->nextthink = level.time + 1500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_CHAINLG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 30;
	bolt->splashDamage = 25;
	bolt->splashRadius = 45;
	bolt->methodOfDeath = MOD_CHAINLG;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

return bolt;
}
end 06/05/03 remove section*/



//PKMOD - Ergodic 06/26/00 
/*
=================
ignore_entity 
logic used by airfist and gravity well for selection
=================
*/
qboolean ignore_entity ( gentity_t *ent ) {

	//Not IT_TEAM nor IT_PKARENA_ACTIVE

	//PKMOD - Ergodic 05/23/01 - first check if item is "visible" in the map
	//PKMOD - Ergodic 07/30/00 Move item back to original map location
	if ( ent->r.svFlags & SVF_NOCLIENT ) 
		return qtrue;

	//PKMOD - Ergodic 05/23/01 - first check if item is "visible" in the map
	//PKMOD - Ergodic 07/30/00 Move item back to original map location
	if ( ent->s.eFlags & EF_NODRAW )
		return qtrue;


	//PKMOD - Ergodic 02/17/01 - optimize (do not use strcmp)
//	if (!strcmp(ent->classname,"airfist_blast"))
	if ( ent->s.eType == ET_AIRFIST )		//07/12/00
		return qtrue;

	//PKMOD - Ergodic 02/17/01 - optimize (do not use strcmp)
//	if (!strcmp(ent->classname,"chainlightning"))
	if ( ent->s.eType == ET_CHAIN_LIGHTNING )		//07/12/00
		return qtrue;

	if ( (ent->item->giType >= IT_WEAPON) && (ent->item->giType <= IT_HOLDABLE)) 
		return qfalse;

	//PKMOD - Ergodic 01/39/01 - if autosentry then don't ignore
	if ( ( ent->s.eType <= ET_AUTOSENTRY_TURRET ) && ( ent->s.eType >= ET_AUTOSENTRY_LAUNCH ) )
		return qfalse;


	//PKMOD - Ergodic 10/19/00 - don't ignore the beartrap... 
	if ( ent->s.eType == ET_BEARTRAP ) 
		return qfalse;

	//PKMOD - Ergodic 10/19/00 - don't ignore the following missiles... 
	if ( ent->s.eType == ET_MISSILE ) {
		switch ( ent->s.weapon ) { 
		case WP_ROCKET_LAUNCHER:
		case WP_GRENADE_LAUNCHER:
		case WP_GRAVITY:  //up until demon is released
			return qfalse;
		default:
			return qtrue;
		}
	}

	if ( ent->client )
		return qfalse;

	return qtrue;
}

//Ergodic - 02/17/01, AirFist infront defintion
//Ergodic - 04/25/01, update AirFist infront from 0.75 to 0.66
#define AIRFIST_INFRONT		0.66		//Target is infront of airfist if higher than this value
#define AF_MAX_RADIUS		800.0
#define AF_MAX_RADIUS_75	600.0		// 04/26/01 - 75% of AF_MAX_RADIUS
#define AF_MAX_RADIUS_25	200.0		// 04/26/01 - 25% of AF_MAX_RADIUS
#define AF_MAX_DAMAGE		20.0
#define AF_MAX_VELOCITY		800.0
#define AF_UP_VELOCITY		200.0

//Ergodic - 03/28/2000, AirFist thinking
/*
================
G_airfistThink

manifest AirFist impact and damage on entities
generate damage only on first blast
================
*/
void G_airfistThink ( gentity_t *ent ) {
	gentity_t	*ext_ent;
	vec3_t		hold_vec;
	float		dot;
	float		dist;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e;
	qboolean	hitClient = qfalse;
	float		hold_velocity;
	float		hold_damage;
	vec3_t		forward;
	float		missile_speed;
	float		af_effect;
	float		dist_factor;
	float		effect_dist;		// 04/26/01 
//	int			tempint;

/*Constants...
	AF_MAX_RADIUS
	AF_MAX_DAMAGE
	AF_MAX_VELOCITY
	MOD_AIRFIST
	AF_UP_VELOCITY
*/
//Ergodic debug inactive
//Com_Printf("G_airfistThink- origin:%s, angles:%s\n", vtos(ent->r.currentOrigin),vtos(ent->r.currentAngles));
//Com_Printf("G_airfistThink- level.time:%d, nextthink:%d, eventTime:%d, timestamp:%d\n", level.time, ent->nextthink, ent->eventTime, ent->timestamp);
//Com_Printf("G_airfistThink- ent->damage>%d<\n", ent->damage);

	af_effect = ent->damage / 100.0;

	//PKMOD - Ergodic 04/26/01 - calculate effect distance
	//			always insure a 75% radius of AF_MAX_RADIUS for non zero AirFist
	if ( ent->damage > 0 )
		effect_dist = AF_MAX_RADIUS_75 + AF_MAX_RADIUS_25 * af_effect;
	else 
		effect_dist = 0;

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = ent->r.currentOrigin[i] - AF_MAX_RADIUS * af_effect;
		maxs[i] = ent->r.currentOrigin[i] + AF_MAX_RADIUS * af_effect;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ext_ent = &g_entities[entityList[ e ]];

		/* removed 03/29/01
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( ent->r.currentOrigin[i] < ext_ent->r.absmin[i] ) {
				v[i] = ext_ent->r.absmin[i] - ent->r.currentOrigin[i];
			} else if ( ent->r.currentOrigin[i] > ext_ent->r.absmax[i] ) {
				v[i] = ent->r.currentOrigin[i] - ext_ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
		*/

		VectorSubtract( ent->r.currentOrigin, ext_ent->r.currentOrigin, v );
		dist = VectorLength( v );

		//PKMOD - Ergodic 04/26/01 - calculate effect distance
//		if ( dist >= AF_MAX_RADIUS * af_effect)
		if ( dist >= effect_dist)
			continue;

		//PKMOD - Ergodic 06/22/01 - debug zombie (inactive)
//	if ( ext_ent->s.eType == ET_ZOMBIE ) {
//		//PKMOD - ergodic 06/22/01 - debug zombie origin (inactive)
//		Com_Printf("G_airfistThink - classname>%s<, loc: current>%s<\n", ext_ent->classname, vtos(ext_ent->r.currentOrigin) );
//	}


		//PKMOD - Ergodic 10/19/00 debug inactive
//		if ( !strcmp(ext_ent->classname,"grenade") ) { 
//			Com_Printf("G_airfistThink - preprocess: grenade detected\n" );
//		}


		//PKMOD - Ergodic 10/19/00 debug inactive
//		if ( !strcmp(ext_ent->classname,"grenade") ) {
//			Com_Printf("G_airfistThink - can damage grenade\n" );
//		}

		//PKMOD - Ergodic 02/18/01 - check if entity is infront of blast
		//PKMOD - Ergodic 02/19/01 - fixed direction
		VectorSubtract(ext_ent->r.currentOrigin, ent->r.currentOrigin, hold_vec);
		VectorNormalize(hold_vec);
		dot = DotProduct ( hold_vec, ent->r.currentAngles );

		//PKMOD - Ergodic 03/28/01 - Cap Max the dot
		if ( dot > 1 ) 
			dot = 1.0;

		//PKMOD - Ergodic 02/19/01 - debug "infront" logic (inactive)
//		if ( !strcmp(ext_ent->classname,"weapon_rocketlauncher") ) {
//			Com_Printf("G_airfistThink- dot:%f, hold_vec:%s, angles:%s\n", dot, alt_vtos(hold_vec), alt_vtos(ent->r.currentAngles ));
//		}

		if ( dot < AIRFIST_INFRONT )
			continue;

		if( !CanDamage (ext_ent, ent->r.currentOrigin) )  //is candidate target visible?
			continue;

		if ( ignore_entity(ext_ent) )
			continue;

		//PKMOD - Ergodic 10/19/00 debug inactive
//		if ( !strcmp(ext_ent->classname,"grenade") ) {
//			Com_Printf("G_airfistThink - do not ignore grenade\n" );
//		}

		if (ext_ent == ent)  //don't pulse yourself
			continue;

		if (ext_ent == ent->parent)  //don't pulse owner
			continue;

		//PKMOD - Ergodic 04/27/01 - always make positive, trust no one
//		dist_factor =  af_effect - (dist / AF_MAX_RADIUS);
		if ( effect_dist > 0.1 )	//effectively 0
			dist_factor =  af_effect - (dist / effect_dist);
		else
			dist_factor =  af_effect - (dist / AF_MAX_RADIUS);

		if 	( dist_factor < 0 )
			dist_factor = - dist_factor;		//dont go negative

		//move non-clients 
		if ( !ext_ent->client ) {
			//PKMOD - Ergodic, 07/30/00 item move logic
			if ( ext_ent->s.eType == ET_ITEM ) {
				//PKMOD - Ergodic 10/03/00. fixed entity bug (pointed to wrong entity)
				if (( ext_ent->AirFist_Level == 0 ) && !( ext_ent->flags & FL_DROPPED_ITEM )) {
//				if (( ext_ent->AirFist_Level == 0 ) && !( ent->flags & FL_DROPPED_ITEM )) {
					//PKMOD - Ergodic 07/30/00 debug inactive
//					if (ext_ent->item->giTag == WP_LIGHTNING ) {
//						Com_Printf("G_airfistThink - moving %s : %s TrType:%d\n", ext_ent->classname, vtos(ext_ent->s.origin), ext_ent->s.pos.trType);
//						Com_Printf("G_airfistThink - eflags>%d<,  svFlags>%d<,  clipmask>%d<,  groundEntityNum>%d<,  spawnflags>%d<\n", ext_ent->s.eFlags, ext_ent->r.svFlags, ext_ent->clipmask, ext_ent->s.groundEntityNum, ext_ent->spawnflags);
//					}
					VectorCopy(ext_ent->s.origin, ext_ent->pka_originalposition);
					ext_ent->AirFist_Level = 1;

					ext_ent->nextthink = level.time + (25 + ( rand() % 11 ) ) * 1000; //restore in ~30 seconds
					ext_ent->think = PKA_RestoreItem;

				}
			}
//			AngleVectors (ent->r.currentOrigin, forward, right, up);
			//PKMOD - Ergodic 10/19/00 - use af blast entity's trbase instead of currentorigin
//			VectorSubtract(ext_ent->s.pos.trBase, ent->r.currentOrigin, hold_vec);
//			VectorSubtract(ext_ent->s.pos.trBase, ent->s.pos.trBase, hold_vec);
			//PKMOD - Ergodic 10/20/00 - return to r.currentorigin
//			VectorSubtract(ext_ent->r.currentOrigin, ent->r.currentOrigin, hold_vec);
//			VectorNormalize(hold_vec);

			//PKMOD - Ergodic 07/13/00 bump up 1 unit
			//if not rocket
			//10/20/00 give upward velocity on first blast only
			if (ent->health == 100) {
				if ( strcmp(ext_ent->classname,"rocket") ) {
			//10/20/00 test nobump
//					ext_ent->s.pos.trBase[2] += 1;
					ext_ent->s.pos.trType = TR_GRAVITY;
					ext_ent->s.eFlags |= EF_BOUNCE_HALF;
				}
			//PKMOD - Ergodic 10/20/00 debug
			//Com_Printf("G_airfistThink -  %s: mins>%s<, maxs.%s<\n",ext_ent->classname, vtos(ext_ent->r.mins), vtos(ext_ent->r.maxs));

			}

			missile_speed = VectorLength(ext_ent->s.pos.trDelta);
			//PKMOD - Ergodic 10/20/00 debug inactive
//			if ( !strcmp(ext_ent->classname,"rocket") )
//				Com_Printf("G_airfistThink - missile speed>%f<\n", missile_speed );

			//give non-moving / slow-moving added velocity to the grenade
			// Ergodic - 10/19/00 specify the grenade specifically
//			if (!((!strcmp(ext_ent->classname,"rocket")) || (!strcmp(ext_ent->classname,"plasma")))) 
			if ( !strcmp(ext_ent->classname,"grenade") ||
					!strcmp(ext_ent->classname,"Gravity Well") ||
					!strcmp(ext_ent->classname,"BearTrap") ) {
				missile_speed += AF_MAX_VELOCITY * dist_factor;
//				missile_speed = AF_MAX_VELOCITY * dist_factor;
			}
//			if ( missile_speed > AF_MAX_VELOCITY ) {
				//PKMOD - Ergodic 10/20/00 debug
//				tempint = dist_factor * 100;
//				Com_Printf("G_airfistThink - missile_speed>%f<, dist_factor>%d<, setting max velocity for %s\n", missile_speed, tempint, ext_ent->classname );
//				missile_speed = AF_MAX_VELOCITY;
//			}

			VectorScale(hold_vec, missile_speed, hold_vec);
			//bias it upwards
			//PKMOD - Ergodic 10/20/00 - new rocket deflection
			//10/20/00 give upward velocity on first blast only
			if (ent->health == 100) {
				//10/20/00 reset the level time
				ext_ent->s.pos.trTime = level.time;
				//10/20/00 if rocket then reset starting location
				if ( !strcmp(ext_ent->classname,"rocket") ) 
					VectorCopy( ext_ent->r.currentOrigin, ext_ent->s.pos.trBase );
			}

			if ( !strcmp(ext_ent->classname,"rocket") )
				hold_vec[2] += AF_UP_VELOCITY * dist_factor;
			else
				hold_vec[2] += 2 * AF_UP_VELOCITY * dist_factor;
//				if ( strcmp(ext_ent->classname,"rocket") ) //if not rocket
//					if ( hold_vec[2] > AF_UP_VELOCITY )
//						hold_vec[2] = AF_UP_VELOCITY;
//			}

			VectorCopy(hold_vec, ext_ent->s.pos.trDelta);
			continue;
		}

		//-------------------
		//here if client type
		//-------------------
		//position of target with respect to airfist origin
		//bias weapon effects to depend on degree of "AIM"
//		VectorCopy (ent->r.currentAngles, forward);

//		VectorSubtract(ext_ent->r.currentOrigin, ent->r.currentOrigin, hold_vec);
//		VectorNormalize(hold_vec);
//		dot = DotProduct( hold_vec, forward );
//Ergodic debug - force "dot" to 1 to prevent negative damage (need to fix later)
//		dot = 1;


//set the horizontal pulse speed
		//PKMOD - Ergodic 03/29/01 - tweak the upward displacement
//		hold_velocity = AF_MAX_VELOCITY * dot * dist_factor / 4;
		hold_velocity = AF_MAX_VELOCITY * dot * dist_factor;

//calculate the horizontal pulse velocity
		VectorScale(hold_vec, hold_velocity, hold_vec);
//calculate the vertical pulse velocity
		//PKMOD - Ergodic 03/12/01 - tweak the upward displacement
		//PKMOD - Ergodic 03/22/01 - tweak the upward displacement
		//PKMOD - Ergodic 03/28/01 - tweak the upward displacement
		//PKMOD - Ergodic 03/29/01 - tweak the upward displacement
//		if (ent->health == 100)
//			hold_vec[2] += 1.9 * AF_UP_VELOCITY * dist_factor / 10;
//		else
//			hold_vec[2] += 0.55 * AF_UP_VELOCITY * dist_factor / 10;
//
		if (ent->health == 100)
			hold_vec[2] += AF_UP_VELOCITY * hold_velocity / AF_MAX_VELOCITY;
		else
			hold_vec[2] += 0.5 * AF_UP_VELOCITY * hold_velocity / AF_MAX_VELOCITY;

//		hold_vec[2] /= 3;
		//PKMOD - Ergodic 03/12/01 - debug the upward jump velocity
//		VectorSet (hold_vec, 0,0, (int)(200 + rand() % 200) * dist_factor); 	//jump velocity = 220 + (0-200)

		//PKMOD - Ergodic 03/11/01 - debug (inactive)
//		Com_Printf( "hold_vec>%s<, ps.velocity>%s<, dist_factor*100>%f<, velocity*100>%f<\n", vtos( hold_vec ), vtos( ext_ent->client->ps.velocity ), 100 * dist_factor, 100 * hold_velocity );

		VectorAdd(hold_vec, ext_ent->client->ps.velocity, ext_ent->client->ps.velocity);
		SnapVector( ext_ent->client->ps.velocity );			// save net bandwidth

//Ergodic Debug
//Com_Printf("AirFist_Level=%d,health=%d,hold_vec:%s,dist*100=%f,hold_velocity*100=%f,dist_factor*100=%f\n",ent->AirFist_Level, ent->health, dist*100, vtos(hold_vec), hold_velocity*100, dist_factor*100);
//Com_Printf("AF_velocity = %1.1f %1.1f %1.1f\n", ext_ent->client->ps.velocity[0], ext_ent->client->ps.velocity[1], ext_ent->client->ps.velocity[2]);

		//if first blast (health == 1) then give damage
		if (ent->health == 100) {
//			Com_Printf("AirFist_Health == 1\n");

			if ( ext_ent->takedamage) {
//				Com_Printf("ext_ent->takedamage\n");
				//give damage to enities within a percentage of max_radius
				if (dist < (af_effect * AF_MAX_RADIUS * 0.80)) {
//					hold_damage = AF_MAX_DAMAGE  * dot * af_effect * ( 1.0 - dist / (AF_MAX_RADIUS * 0.80) );
					hold_damage = AF_MAX_DAMAGE  * dot * dist_factor;
//Ergodic Debug
//Com_Printf("hold_damage=%f\n",hold_damage);
//Ergodic Debug
					if (hold_damage > 0) {
						G_Damage( ext_ent, ent, ent->parent, forward, NULL,hold_damage, 0, MOD_AIRFIST);
					}
				}
			}
		}
	} //end for entity loop

	ent->health = 50;	//set first blast flag to 50 => false
						//subsequent blasts will not cause damage for this particular entity  

	if (ent->timestamp > level.time) {
		ent->nextthink = level.time + 200;
		ent->eventTime = level.time;
		trap_LinkEntity( ent );
	}
	else {
	//	Com_Printf("G_airfistThink: Removing AirFist_Level:%d\n", ent->AirFist_Level);  //Ergodic debug
		G_FreeEntity( ent );
	}

}


//PKMOD - Ergodic - 03/28/2000, create airfist blast as an entity
/*
=================
fire_airfist
co-opted entity fields...
	timestamp : holds the duration of the AirFist blast
	health : holds first blast information (only damage on first blast of entity's life)
=================
*/
gentity_t *fire_airfist (gentity_t *self, vec3_t start, vec3_t dir, float mc_quadFactor) {
	gentity_t	*bolt;
	vec3_t		newstart;
	vec3_t		af_backward;
	int			hold_af_effect;

	//PKMOD - Ergodic 10/20/00 debug inactive
//	Com_Printf("fire_airfist\n" );


	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "airfist_blast";
	//PKMOD - Ergodic 10/19/00 - vector "start" is located at end of muzzle
	//			newstart will be placed back at players origin(x,y) and muzzle's height(z).
	//			The code is a reverse calcmuzzlepoint
	//			This will address missles firing backward after airfist blast
//	VectorCopy ( self->r.currentOrigin, newstart );
//	newstart[2] = start[2];
	VectorCopy( self->s.pos.trBase, newstart );
	newstart[2] += self->client->ps.viewheight;
	VectorNegate( dir, af_backward );

	//PKMOD - Ergodic 10/19/00 - Debug 
//	Com_Printf("fire_airfist: dir>%s<, af_backward>%s<\n", vtos( dir ), vtos( af_backward ) );

	VectorMA( newstart, 28, af_backward, newstart );  //was 14

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( newstart );			// save net bandwidth
	VectorCopy ( newstart, bolt->s.pos.trBase );
	VectorCopy ( newstart, bolt->r.currentOrigin );

	SnapVector( bolt->r.currentOrigin );			// save net bandwidth
	VectorCopy (dir, bolt->r.currentAngles);
	bolt->health = 100;
	bolt->s.eType = ET_AIRFIST;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_AIRFIST;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	//PKMOD - Ergodic 06/24/00 fix airfist water bug
	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	//PKMOD - Ergodic 02/02/04, change scale from (100, 90, 70, 50, 5) 
	//				to (100, 93, 82, 65, 15)
	switch ( self->client->ps.stats[STAT_AIRFIST_LEVEL] ) {
	case 4:
		hold_af_effect = 100;
		break;
	case 3:
		hold_af_effect = 93;
		break;
	case 2:
		hold_af_effect = 82;
		break;
	case 1:
		hold_af_effect = 65;
		break;
	default: 
		hold_af_effect = 15;
		break;
	}

	if (self->waterlevel > 2)
		bolt->damage = hold_af_effect * mc_quadFactor * 0.4;
	else
		bolt->damage = hold_af_effect * mc_quadFactor;

	bolt->methodOfDeath = MOD_AIRFIST;
	bolt->clipmask = MASK_SHOT;

	bolt->think = G_airfistThink;
	bolt->nextthink = level.time + 100;
	bolt->eventTime = level.time;
	bolt->timestamp = level.time + AIRFIST_DURATION;

	self->AirFist_ResetTime = level.time + AIRFIST_RESET_WAIT;  //set time when Airfist will rebuild

	return bolt;
}

/*
=================
NailTouch
Ergodic 08/01/00
=================
*/
void NailTouch ( gentity_t *ent, gentity_t *other, trace_t *trace ) {

	if ( other->client ) {
		//PKMOD - Ergodic 10/28/00 - punctuate damage 
		if  ( ent->wait < level.time ) {
			//PKMOD - Ergodic 04/21/01 - increase damage from (4, 6) to (5, 9)
			G_Damage( other, ent, ent->parent, NULL, NULL, 5 + ( rand() % 5 ), 0, MOD_NAIL);
			//PKMOD - Ergodic 04/21/01 - punctuate damage to every 1/10 second (was 1/5) 
			//PKMOD - Ergodic 05/11/01 - punctuate damage to every 0.08 second (was 1/10) 
			ent->wait = level.time + 80;  // punctuate damage every 0.08 second

		}
	}
}

/*
=================
NailDeath
Ergodic 08/01/00
=================
*/
void NailDeath ( gentity_t *ent ) {

	ent->touch = 0; //PKMOD - Ergodic 08/06/00 change NULL to 0 because of compiler warnings
	//change model to dull color
//	G_AddEvent( ent, EV_NAIL_DIE, 0 );

	//Ergodic debug - 08/03/00 inactive
//	Com_Printf("NailDeath - mins>%s<, maxs.%s<\n", vtos(ent->r.mins), vtos(ent->r.maxs));

	ent->nextthink = level.time + 300; 
	ent->think = G_FreeEntity;
}

/*
=================
fire_nailgun
=================
*/
//PKMOD Ergodic 07/28/00 
gentity_t *fire_nailgun (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "nail";
	//PKMOD - Ergodic 08/28/02 - Jim Kramer bug found on skymap - increase lifetime from 1500 to 10000
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAILGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
//	bolt->splashDamage = 25;
//	bolt->splashRadius = 45;
	bolt->methodOfDeath = MOD_NAILGUN;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;

	VectorCopy( start, bolt->s.pos.trBase );
//	VectorScale( dir, 200, bolt->s.pos.trDelta );  //trace code only
	VectorScale( dir, 1500, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);
	//PKMOD Ergodic 08/03/00 set the angle of the nail 
	VectorCopy( dir, bolt->s.apos.trBase );


return bolt;
}

/*
=================
Gravity Well
PKMOD - Ergodic 02/17/01 - make setting as definitions instead of int declarations
=================
*/
	//Orginal Q1 values were Radius:960, Velocity:512
#define			GW_MAX_RADIUS		1200	//was 1000 - Ergodic 10/19/00
#define			GW_MAX_ITEM_RADIUS	700 
#define			GW_MAX_VELOCITY		1000	//was 2000 - Ergodic 02/17/01
#define			GW_UP_VELOCITY		250		//was 750 - Ergodic 02/17/01
	//PKMOD - Ergodic 02/16/01 - optimizations
#define			GW_RADIUS_EFFECT	0.833		//(float) GW_MAX_VELOCITY / (float) GW_MAX_RADIUS; 
	//PKMOD - Ergodic 05/24/00 - GW tweaking
#define			GW_RATE				5 //rate of force applied per second
	//PKMOD - Ergodic 07/02/00 - GW Think will perform the killing too
#define			GW_KILL_RADIUS		180	//if within kill radius then die 
										//03/28/01 kill radius updated was (80)		
										//06/06/02 - kill radius updated was (160)		
#define			GW_MAX_ENTITIES		500 //limit of entities to be found with the box
#define			GW_MAX_ENT_ARRAY	GW_MAX_ENTITIES + 64 //limit of array entities (GW_MAX_ENTITIES + 64)
#define			GW_MAX_ENTITIES_CAP	150 //limit of entities to be processed
#define			GW_DURATION			8000 //06/06/02 - duration of active gravity well
#define			GW_DELAY			2000 //06/06/02 - call GravityThink in 2 seconds
#define			GW_MIDPOINT			((GW_DURATION - GW_DELAY)/2) //06/06/02 - call GravityThink in 2 seconds
#define			GW_MAXVERTOFFSET	100 //06/06/02 - max height of gravity attraction offset
#define			GW_EARTHQUAKE_SOUND_DELAY 50 //11/21/03 - Delay of Earthquake sound from start
//PKMOD - Ergodic 12/18/03 - Change earthquake sound delay from 1000 to 100
//PKMOD - Ergodic 01/04/04 - change name from GW_EARTHQUAKE_DELAY to GW_EARTHQUAKE_SOUND_DELAY
#define			GW_EARTHQUAKE_MOTION_DELAY 400 //01/01/04 - Delay of Earthquake motion from start
#define			GW_MAX_HORIZONTAL_SPEED 350 //12/18/03 - add horizontal governor
#define			GW_MAX_VERTICAL_SPEED 300 //12/18/03 - add vertical governor

/*
=================
GravityTouch
=================
*/
void GravityTouch ( gentity_t *ent, gentity_t *other, trace_t *trace ) {

	if ( other->client ) {
		G_Damage( other, ent, ent->parent, NULL, NULL, ent->damage, 0, MOD_GRAVITY);
//		G_Printf( "GravityTouch Detected!\n" );
	}
}

/*
=================
GravityThink
=================
*/
void GravityThink( gentity_t *ent ) {
	gentity_t	*ext_ent;
	vec3_t		hold_vec;
	float		dist;
	int			entityList[GW_MAX_ENT_ARRAY];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e, clientIndex;
	qboolean	hitClient = qfalse;
	float		gw_effect;
	float		missile_speed;
	//PKMOD - Ergodic 06/06/02 - add variable origin for clipped players 
	vec3_t		gw_attractionPoint;

	//PKMOD - Ergodic 02/17/01 - Change logic to look for clients first 
	//				and then look for items. Item effect radius will be
	//				drastically lower than client in order to prevent
	//				game latency 
	ext_ent = &g_entities[0];
	clientIndex = 0;

	//PKMOD - Ergodic 11/21/03 - Play earthquake sound if it is time
	if ( ! ( ent->pka_flags & PKAEF_EARTHQUAKE ) ) {
		//PKMOD - Ergodic 11/05/00 - debug earthquake (1) - (Inactive)
		//Com_Printf( "GravityThink (1) - flag not set, level.time>%d<, time2>%d<...\n", level.time, ent->s.time2 );
		if ( ( ent->s.time2 + GW_DELAY + GW_EARTHQUAKE_SOUND_DELAY ) < level.time ) {
			//PKMOD - Ergodic 11/05/00 - debug earthquake (2) - (Inactive)
			//Com_Printf( "GravityThink (2) - Time for sound...\n" );
			//G_TempEntity( ext_ent->r.currentOrigin, EV_EARTHQUAKE );
			//PKMOD - Ergodic 12/07/03 - Make earthquake a WORLD SOUND
			gentity_t	*te;

			te = G_TempEntity( ent->r.currentOrigin, EV_GLOBAL_SOUND );
			te->s.eventParm = G_SoundIndex( "sound/weapons2/gwell/earthquake3.wav" );
			te->r.svFlags |= SVF_BROADCAST;

			ent->pka_flags |= PKAEF_EARTHQUAKE;

		}
	}

	for ( i = 0 ; i < MAX_CLIENTS  ; i++, ext_ent++) {
		if ( ext_ent->inuse ) {
			//Found an inuse client so build the numListedEntities array
			entityList[ clientIndex ] = i;
			clientIndex++;
		}
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = ent->r.currentOrigin[i] - GW_MAX_ITEM_RADIUS;
		maxs[i] = ent->r.currentOrigin[i] + GW_MAX_ITEM_RADIUS;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, &entityList[clientIndex], GW_MAX_ENTITIES );
	numListedEntities += clientIndex;

	//PKMOD - Ergodic 02/18/01 - after building list, assign a max capacity to the size
	if ( numListedEntities > GW_MAX_ENTITIES_CAP )
		numListedEntities = GW_MAX_ENTITIES_CAP;

	//PKMOD - Ergodic 03/27/01 - debug inactive
//	Com_Printf("GravityThink - numListedEntities<%d<\n", numListedEntities );

	for ( i = 0, e = 0 ; e < numListedEntities ; i++, e++ ) {
		ext_ent = &g_entities[entityList[ e ]];

/*
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( ent->r.currentOrigin[i] < ext_ent->r.absmin[i] ) {
				v[i] = ext_ent->r.absmin[i] - ent->r.currentOrigin[i];
			} else if ( ent->r.currentOrigin[i] > ext_ent->r.absmax[i] ) {
				v[i] = ent->r.currentOrigin[i] - ext_ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
*/
		VectorSubtract( ext_ent->r.currentOrigin, ent->r.currentOrigin, v );
		dist = VectorLength( v );

		if ( dist > GW_MAX_RADIUS) {
		//PKMOD - Ergodic 11/19/03 - Players beyond GW_MAX_RADIUS will experience earthquake
			if (ext_ent->client) {
				if (ext_ent->inuse) {
					//PKMOD - Ergodic 01/01/04 - add motion delay 
					if ( ( ent->s.time2 + GW_DELAY + GW_EARTHQUAKE_MOTION_DELAY ) < level.time ) {

						//PKMOD - Ergodic 12/19/03 - don't shake spectators
						if ( ext_ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
							hold_vec[0] = crandom() * 250;
							hold_vec[1] = crandom() * 250;
							hold_vec[2] = crandom() * 200;
							VectorAdd(hold_vec, ext_ent->client->ps.velocity, hold_vec);
							//PKMOD - Ergodic 12/18/03 - Add governor for max speeds
							if ( hold_vec[0] > GW_MAX_HORIZONTAL_SPEED )
								hold_vec[0] = GW_MAX_HORIZONTAL_SPEED;
							else if (hold_vec[0] < - GW_MAX_HORIZONTAL_SPEED )
								hold_vec[0] = - GW_MAX_HORIZONTAL_SPEED;

							if ( hold_vec[1] > GW_MAX_HORIZONTAL_SPEED )
								hold_vec[1] = GW_MAX_HORIZONTAL_SPEED;
							else if (hold_vec[1] < - GW_MAX_HORIZONTAL_SPEED )
								hold_vec[1] = - GW_MAX_HORIZONTAL_SPEED;

							if ( hold_vec[2] > GW_MAX_VERTICAL_SPEED )
								hold_vec[2] = GW_MAX_VERTICAL_SPEED;
							else if (hold_vec[2] < - GW_MAX_VERTICAL_SPEED )
								hold_vec[2] = - GW_MAX_VERTICAL_SPEED;

							VectorCopy(hold_vec, ext_ent->client->ps.velocity);
						}
					}
				}
			}
			continue;
		}

		//PKMOD - Ergodic 06/06/02 - use attraction point for clients
		//PKMOD - Ergodic 06/06/02 - set attraction point 
		VectorCopy( ent->r.currentOrigin, gw_attractionPoint );

		//PKMOD - Ergodic 12/14/02 - Attempt Primary Attraction at 'Real' Origin
		if( !CanDamage (ext_ent, gw_attractionPoint) ) {	//is candidate target visible?
			//Here if Primary Attraction Failed
			//PKMOD - Ergodic 06/06/02 - displace the vertical location of the attraction point
			if ( abs(level.time - ent->count) <= GW_MIDPOINT )
				gw_attractionPoint[2] += GW_MAXVERTOFFSET * (1 - abs(level.time - ent->count) / GW_MIDPOINT);

			//Attempt Secondary Attraction
			if( !CanDamage (ext_ent, gw_attractionPoint) )  { //is candidate target visible?
				//PKMOD - Ergodic 11/20/03 - Players beyond GW_MAX_RADIUS will experience earthquake
				if (ext_ent->client) {
					if (ext_ent->inuse) {
						hold_vec[0] = crandom() * 250;
						hold_vec[1] = crandom() * 250;
						hold_vec[2] = crandom() * 200;
						VectorAdd(hold_vec,ext_ent->client->ps.velocity,ext_ent->client->ps.velocity);
					}
				}
				continue;
			}
		}

		if ( ignore_entity(ext_ent) )
			continue;

		//PKMOD - Ergodic 02/16/01 - optimizations
//		gw_effect = GW_MAX_VELOCITY * (1 - (dist / GW_MAX_RADIUS)) / GW_RATE;
		gw_effect = GW_MAX_VELOCITY - dist * GW_RADIUS_EFFECT;

		//move non-clients 
		if ( !ext_ent->client ) {
			//PKMOD - Ergodic, 07/30/00 item move logic
			if ( ext_ent->s.eType == ET_ITEM ) {
				if (( ext_ent->AirFist_Level == 0 ) && !( ext_ent->flags & FL_DROPPED_ITEM )) {
					//PKMOD - Ergodic 07/30/00 debug inactive
//					Com_Printf("GravityThink - moving %s : %s\n", ext_ent->classname, vtos(ext_ent->s.origin));

					VectorCopy(ext_ent->s.origin, ext_ent->pka_originalposition);
					ext_ent->AirFist_Level = 1;

					ext_ent->nextthink = level.time + (25 + ( rand() % 11 ) ) * 1000; //restore in ~30 seconds
					ext_ent->think = PKA_RestoreItem;

				}
				//PKMOD - Ergodic, 07/30/00 item move logic - gw will remove items
				if ( dist < GW_KILL_RADIUS ) {
					//turn the item "off"
					ext_ent->r.svFlags |= SVF_NOCLIENT;
					ext_ent->s.eFlags |= EF_NODRAW;
					ext_ent->r.contents = 0;
					//PKMOD - Ergodic 09/06/00 gravity well item suck sounds from Mongusta
					G_AddEvent( ent, EV_GRAVITYWELL_SUCK, 0 );
					continue;
				}

			}

			//PKMOD - Ergodic 09/20/00 - add logic to remove activated beartraps
			if ( ext_ent->s.eType == ET_BEARTRAP ) {
				if ( dist < GW_KILL_RADIUS ) {
					G_AddEvent( ent, EV_GRAVITYWELL_SUCK, 0 );
					G_FreeEntity( ext_ent );
					continue;
				}
			}

//			AngleVectors (ent->r.currentOrigin, forward, right, up);
			VectorSubtract( ent->r.currentOrigin, ext_ent->s.pos.trBase, hold_vec);
			VectorNormalize(hold_vec);

			//PKMOD - Ergodic 07/13/00 bump up 1 unit
			ext_ent->s.pos.trBase[2] += 1;
			ext_ent->s.pos.trType = TR_GRAVITY;
//PKMOD - Ergodic 11/27/01 - debug level time
//			ext_ent->s.pos.trTime = level.time;
			ext_ent->s.eFlags |= EF_BOUNCE_HALF;

			missile_speed = VectorLength(ext_ent->s.pos.trDelta);
			//give non-moving / slow-moving added velocity to the grenade
			if (!((!strcmp(ext_ent->classname,"rocket")) || (!strcmp(ext_ent->classname,"plasma")))) 
				missile_speed += gw_effect;
			VectorScale(hold_vec, missile_speed, hold_vec);
			//bias it upwards
			hold_vec[2] += 2 * GW_UP_VELOCITY * gw_effect / GW_MAX_VELOCITY;
			VectorCopy(hold_vec, ext_ent->s.pos.trDelta);

			continue;
		}

		//here if client type
		//position of target with respect to airfist origin
		//bias weapon effects to depend on degree of "AIM"

		if (ext_ent == ent)  //don't effect yourself
			continue;

		//PKMOD - Ergodic 02/27/01 - Debug graphics (don't suck owner)
		//				remove this code later
//		if ( ext_ent == ent->parent )
//			continue;

		if ( i >= clientIndex )		//don't double hit client
			continue; 

		//PKMOD - Ergodic 02/17/01 - debug inactive
//		Com_Printf("GravityThink - client:%s, dist:%f, GW_KILL_RADIUS:%d \n", ext_ent->client->pers.netname, dist, GW_KILL_RADIUS );
		//PKMOD - Ergodic 02/18/01 - gravity well overpowers hook
		//if client has hook then remove hook - grin: poor bastard
		if ( ext_ent->client->hook ) {
//			Com_Printf( "GravityThink - client has hook, numListedEntities:%d\n", numListedEntities );
			Weapon_HookFree(ext_ent->client->hook);
		}

		//PKMOD - Here if a Client
		if ( dist < GW_KILL_RADIUS ) {
			G_Damage( ext_ent, ent, ent->parent, NULL, NULL, ent->damage, 0, MOD_GRAVITY);
			continue;
		}

//		VectorSubtract(ent->r.currentOrigin, ext_ent->r.currentOrigin, hold_vec);
		//PKMOD - Ergodic 06/06/02 - use attraction point for clients  
		VectorSubtract(gw_attractionPoint, ext_ent->r.currentOrigin, hold_vec);
		VectorNormalize(hold_vec);

		//PKMOD - Ergodic 05/24/00, gw tweak set the player's movement direction
		VectorCopy(hold_vec, ext_ent->movedir);

		//calculate the horizontal velocity
		VectorScale(hold_vec, gw_effect, hold_vec);
		//bias the the vertical velocity
		hold_vec[2] += GW_UP_VELOCITY * gw_effect / GW_MAX_VELOCITY;
		VectorAdd(hold_vec,ext_ent->client->ps.velocity,ext_ent->client->ps.velocity);



		//PKMOD - Ergodic 05/24/00, gw tweak set the player's movement timer
		if (!ext_ent->client->ps.pm_time) {
			ext_ent->client->ps.pm_time = GW_RATE - 1;
			ext_ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}

	}

	if (ent->timestamp > level.time) {
		ent->eventTime = level.time;
		ent->nextthink = level.time + 200;
		trap_LinkEntity( ent );
	}
	else {
		G_FreeEntity( ent );
	}

}
/*
=================
Release_Gravity
=================
*/
void Release_Gravity( gentity_t *ent ) {
	gentity_t	*bolt;



	bolt = G_Spawn();
	bolt->classname = "Gravity Well";
	//PKMOD TEMP
	bolt->s.eType = ET_GRAVITY_WELL;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;

	//PKMOD - Ergodic 11/21/03 - clear the PKA entity flags (now using an earthquake flag)
	bolt->pka_flags = 0;


//PKMOD Ergodic 05/17/00 multi-model hack
//PKMOD Ergodic 06/09/00 reset model back to original name, multi-model controlled elsewhere
	bolt->s.weapon = WP_GRAVITY;
	bolt->r.ownerNum = ent->r.ownerNum;
	bolt->parent = ent->parent;
	bolt->damage = ent->damage;
//	bolt->splashDamage = 1000;
//	bolt->splashRadius = 1500;
	bolt->methodOfDeath = MOD_GRAVITY;
	bolt->splashMethodOfDeath = MOD_GRAVITY;
	bolt->clipmask = MASK_SHOT;
	bolt->touch = GravityTouch;

	VectorSet (bolt->r.mins, -1 * ITEM_RADIUS, -1 * ITEM_RADIUS, -1 * ITEM_RADIUS);
	VectorSet (bolt->r.maxs, 1 * ITEM_RADIUS, 1 * ITEM_RADIUS, 1 * ITEM_RADIUS);

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	//10/20/00 don't copy the s.pos location(trBase) or velocity (trDelta)
//	VectorCopy( ent->s.pos.trBase, bolt->s.pos.trBase );
//	VectorCopy( ent->s.pos.trDelta, bolt->s.pos.trDelta );
//	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	//PKMOD - Ergodic 03/28/01 - use trBase as location and not r.currentOrigin
	VectorCopy (ent->r.currentOrigin, bolt->s.pos.trBase);
	VectorCopy (ent->r.currentOrigin, bolt->r.currentOrigin);

	bolt->eventTime = level.time;

	//PKMOD - Ergodic 11/03/02 - store the strat time of the Gravity Well
	bolt->s.time2 = level.time;
	//PKMOD - Ergodic 11/03/00 - debug start time (inactive)
//	Com_Printf("Release_Gravity - level.time>%d<\n", level.time );

	//PKMOD - Ergodic 03/27/01 - increase duration from 6500 to 8000
	bolt->timestamp = level.time + GW_DURATION; //duration
	bolt->nextthink = level.time + GW_DELAY; // call GravityThink in 2 seconds
	bolt->count = bolt->timestamp - GW_MIDPOINT; // set the max X midpoint
	bolt->think = GravityThink;
	trap_LinkEntity( bolt );

	//PKMOD Ergodic 07/11/00 add event to handle glass break sounds
	G_AddEvent( bolt, EV_GRAVITY_RELEASED, 0 );



	//Remove the original entity - Gravity Well Container
	ent->nextthink = level.time + 500;
	ent->think = G_FreeEntity;

}

/*
=================
fire_gravity
=================
*/
gentity_t *fire_gravity (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;
	int GW_MAX_DAMAGE = 5000; 

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "Gravity Well";

//PKMOD Ergodic 07/11/00 set release delay was (1500)
//PKMOD Ergodic 10/07/00 set release delay was (4000)
//PKMOD Ergodic 03/28/01 set release delay was (1250)
	bolt->nextthink = level.time + 1800;
	bolt->think = Release_Gravity;
//	bolt->wait = level.time + 2000; // GWlifetime.
	//PKMOD TEMP
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
//PKMOD Ergodic 05/17/00 multi-model hack
	bolt->s.weapon = WP_GRAVITY;
	bolt->s.eFlags = EF_BOUNCE_HALF;

//PKMOD Ergodic 05/17/00 multi-model hack
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = GW_MAX_DAMAGE;
//PKMOD Ergodic 07/01/00 remove splash damage
//	bolt->splashDamage = 1000;
//	bolt->splashRadius = 1500;
	bolt->methodOfDeath = MOD_GRAVITY;
	bolt->splashMethodOfDeath = MOD_GRAVITY;
	bolt->clipmask = MASK_SHOT;

	//PKMOD Ergodic 03/28/01 - Modify trType so that gravity well can be blown (was TR_GRAVITY)
	//PKMOD Ergodic 03/28/01 - revert to TR_GRAVITY
	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	//PKMOD - Ergodic 07/02/00 - increase the throw speed from 200 to 300
	//PKMOD - Ergodic 08/04/00 - increase the throw speed from 300 to 600
	//PKMOD - Ergodic 08/07/00 - increase the throw speed from 600 to 400
	//PKMOD Ergodic 03/28/01 - Make it slow, now that it is pushable
	//PKMOD Ergodic 04/27/01 - change velocity (was 100)
	//PKMOD Ergodic 07/02/01 - change velocity (was 250)
	VectorScale( dir, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	//PKMOD Ergodic 03/28/01 - Make the model size very large
	VectorSet (bolt->r.mins, -32, -32, -32);
	VectorSet (bolt->r.maxs, 32, 32, 32);

	return bolt;
}

/*
=================
BearTrapDeath
=================
*/
void BearTrapDeath ( gentity_t *ent ) {

	ent->touch = 0; //PKMOD - Ergodic 08/06/00 change NULL to 0 because of compiler warnings
	G_AddEvent( ent, EV_BEARTRAP_DIE, 0 );

	//PKMOD - Ergodic 06/29/00 - remove the beartrap count if attached to player
	if ( ent->target_ent )
		if ( ent->target_ent->client->ps.stats[STAT_BEARTRAPS_ATTACHED] > 0 );
			ent->target_ent->client->ps.stats[STAT_BEARTRAPS_ATTACHED]--;

	ent->nextthink = level.time + 100; 
	ent->think = G_FreeEntity;
}

/*
=================
BearTrapKill
=================
*/
void BearTrapKill( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	BearTrapDeath( self );
}

/*
=================
BearTrapFollow
=================
*/
void BearTrapFollow ( gentity_t *ent ) {

	//remove beartrap if player has disconnected
	if (!strcmp( ent->target_ent->classname,"disconnected" ) || ( ent->target_ent->client->pers.connected == CON_DISCONNECTED )) {
		G_FreeEntity( ent );
		return;
	}

	//remove beartrap if player has died
	if (( ent->target_ent->client->ps.pm_type == PM_DEAD ) || ( ent->target_ent->s.eFlags == EF_DEAD )) {
		G_FreeEntity( ent );
		return;
	}

	//remove beartrap if player's health drops to, or below, zero 
	if ( ent->target_ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		G_FreeEntity( ent );
		return;
	}

	//has the target moved
	if (( ent->r.currentOrigin != ent->target_ent->r.currentOrigin ) ||	( ent->s.angles != ent->target_ent->client->ps.viewangles ) || ( ent->count != ent->target_ent->client->ps.viewheight )) {
		G_SetOrigin( ent, ent->target_ent->r.currentOrigin );
//		VectorCopy( ent->target_ent->r.currentOrigin, ent->r.currentOrigin );
//		VectorCopy( ent->target_ent->s.angles, ent->s.angles ); 
//PKMOD - Ergodic 06/28/00 - modify beartrap angles so that beartrap will face the correct angles
		VectorCopy( ent->target_ent->client->ps.viewangles, ent->r.currentAngles );
		VectorCopy( ent->target_ent->client->ps.viewangles, ent->s.angles );
		VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );
		ent->count = ent->target_ent->client->ps.viewheight;

//PKMOD Ergodic debug 06/28/00 
//Com_Printf("BearTrapFollow - r.currentAngles>%s<,  s.apos.trBase>%s<\n", vtos(ent->r.currentAngles), vtos(ent->s.apos.trBase));

		ent->damage = 1 + 4 * random();

		//PKMOD - Ergodic 07/11/00 add quad affects
		if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
			ent->damage *= g_quadfactor.value;
			//PKMOD Ergodic 04/16/03 - debug damage
			//Com_Printf( "BearTrapFollow - quad beartrap damage\n" );
		}

	}

	if ( ent->timestamp < level.time ) {
		if ( ent->damage > 0 ) {
			G_Damage( ent->target_ent, ent, ent->parent, NULL, NULL, ent->damage, 0, MOD_BEARTRAP);
		}
		ent->timestamp = level.time + 400 + 250 * random();
		ent->damage = 0;
	}


	if (ent->wait > level.time) {
//PKMOD - Ergodic 07/11/00 modify the re-think time was (200)
		ent->nextthink = level.time + 500;
		trap_LinkEntity( ent );
	}
	else {
		//Remove the beartrap entity
		ent->nextthink = level.time + 100; 
		ent->think = BearTrapDeath;
	}
}
/*
=================
BearTrapTouch
=================
*/
void BearTrapTouch ( gentity_t *ent, gentity_t *other, trace_t *trace ) {

	//PKMOD - Ergodic 02/25/04 - debug beartrap near pendulum on pka10_30 (inactive)
	//if ( other->client )
//		Com_Printf("BearTrapTouch - other->classname>%s<, netname>%s<, health>%d<\n", other->classname, other->client->pers.netname, other->client->ps.stats[STAT_HEALTH] );
//	else 
	//	Com_Printf("BearTrapTouch - other->classname>%s<, other->s.eType>%d<\n", other->classname, other->s.eType );

	//operate on clients only
	if ( !other->client )
		return;

	//PKMOD - Ergodic 02/18/01 - if team game, don't attack teammate
	//				note: OnSameTeam insures g_gametype.integer >= GT_TEAM 
	if ( OnSameTeam (other, ent->parent ) )
		return;


	//owner is immune to the beartrap for a moment
	if ( (other == ent->parent) && (ent->wait > level.time) ) {
		return;
	}

	//clients are immune to beartrap upon respawning or teleporting
	//PKMOD Ergodic - 04/07/01 - rename variable to add autosentry
	if ( other->BearTrap_Autosentry_ImmuneTime > level.time ) {
		//PKMOD Ergodic - 12/18/03 - immune time is negated when owning player has HASTE powerup
		if ( ent->parent->client->ps.powerups[PW_HASTE] == 0 )
			return;
	}

	//Snap!
	G_AddEvent( ent, EV_BEARTRAP_SNAP, 0 );

	//PKMOD - Ergodic 10/20/03 - Turn off CONTENTS_BODY so player will not be clipped
	//			or player's movement will not be stuttery
	ent->r.contents &= ~CONTENTS_BODY;

	//PKMOD - Ergodic 06/30/00 - add beartraps to the player that is currently "snapped"
	other->client->ps.stats[STAT_BEARTRAPS_ATTACHED]++;

	//set eType so missile logic will no longer apply, maybe should be placed in missile stop logic
	ent->s.eType = ET_BEARTRAP_FOLLOW;

	//PKMOD - Ergodic 08/30/01 - clear the team fields
	ent->s.generic1 = 0;

	//PKMOD - Ergodic 03/11/01 - set the health of the following beartrap
	ent->health = 75 + rand() % 50;	//03/11/01 - set health to 75 + random amount

	ent->target_ent = other;
	ent->wait = level.time + 15000 + 5000 * random(); //Follow for 15 seconds + 5 random seconds
	
	ent->timestamp = 0; //set the first follow to cause damage

	ent->think = BearTrapFollow;
	ent->nextthink = level.time + 200;

	trap_LinkEntity( ent );

	ent->touch = 0;	//PKMOD - Ergodic 08/06/00 change NULL to 0 because of compiler warnings
}

/*
=================
Beartrap_Think

this function will test the parent's status and remove entity if parent's status has changed 
=================
*/
void Beartrap_Think ( gentity_t *ent ) {

	//PKMOD - Ergodic 03/23/01 - debug (inactive)
//	Com_Printf( "Beartrap_Think - init\n" );

	//PKMOD - Ergodic 03/23/01 - remove beartrap if the owner player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		//PKMOD - Ergodic 03/23/01 - debug (inactive)
//		Com_Printf( "Beartrap_Think - disconnected\n" );
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 03/23/01 - remove beartrap if owner's status has changed
	if ( ent->s.modelindex != ent->parent->client->sess.sessionTeam ) {
		//PKMOD - Ergodic 03/23/01 - debug (inactive)
//		Com_Printf( "Beartrap_Think - session mismatch\n" );
		G_FreeEntity( ent );
		return;
	}
	
	//PKMOD - Ergodic 03/23/01 - check for end of duration
	if ( ent->timestamp > level.time ) {
		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	}
	else {
		//PKMOD - Ergodic 03/23/01 - debug (inactive)
//		Com_Printf( "Beartrap_Think - calling beartrap death\n" );
		//Remove the beartrap
		ent->nextthink = level.time + 200; 
		ent->think = BearTrapDeath;
	}
}


/*
=================
fire_beartrap
co-opted entity fields...
	wait :	don't strike until after "wait" time == beartrap_time
			when active, "wait" will hold the beartrap's duration 
	count :	hold player's current viewheight setting
	timestamp : beartrap duration (endtime) 03/23/01
	generic1: Beartrap invisibility factor 10/10/03
04/16/03 - ShooterSpeed : if call is from a shooter then reduce the lifespan of the trap
=================
*/
gentity_t *fire_beartrap (gentity_t *self, vec3_t start, vec3_t dir, int shooterSpeed ) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	//Ergodic - 10/31/00 set name to lowercase, to fix missing model beartrap bug  
	bolt->classname = "beartrap";

	//Ergodic - 04/16/03 - Shooter logic
	if ( shooterSpeed ) {
		bolt->timestamp = level.time + 2000 + rand() % 10;	// BearTrap lives ~ 2 seconds
		bolt->wait = level.time; // beartrap_time, 0 seconds to get away.
	} else {
		bolt->timestamp = level.time + 75000 + rand() % 10;	// BearTrap lives ~ 80 seconds
		bolt->wait = level.time + 3000; // beartrap_time, 3 seconds to get away.
	}
	bolt->nextthink = level.time + 300; // check for all parent status
	bolt->think = Beartrap_Think;

	//PKMOD TEMP
	bolt->s.eType = ET_BEARTRAP;
	bolt->clipmask = MASK_SHOT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
//PKMOD Ergodic 06/09/00 reset model back to original name, multi-model controlled elsewhere
	bolt->s.weapon = WP_BEARTRAP;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;

	//PKMOD - Ergodic 03/23/01 - add team parameters
	//Ergodic - 04/16/03 - Shooter logic
	if ( shooterSpeed ) {
		bolt->s.modelindex = TEAM_FREE;
		//PKMOD - Ergodic 08/30/01 - set the team fields so bots will not shoot their own team's beartraps
		//PKMOD - Ergodic 10/10/03 - BOT AI will use modelindex instead of generic1 in order to free generic1 for "cell damages"
		//	bolt->s.generic1 = TEAM_FREE;
	} else {
		bolt->s.modelindex = self->client->sess.sessionTeam;
		//PKMOD - Ergodic 08/30/01 - set the team fields so bots will not shoot their own team's beartraps
		//PKMOD - Ergodic 10/10/03 - BOT AI will use modelindex instead of generic1 in order to free generic1 for "cell damages"
		//	bolt->s.generic1 = self->client->sess.sessionTeam;
	}

	//PKMOD - Ergodic 10/10/03 - set invisibility factor
	bolt->s.generic1 = 0;

	//PKMOD - Ergodic 03/15/01 - use hard coded weapon number in order to make function
	//			compatible with dragon deployable weapons
	//bolt->item = BG_FindItemForWeapon( self->client->ps.weapon );
	bolt->item = BG_FindItemForWeapon( WP_BEARTRAP );


//	bolt->s.modelindex = bolt->item - bg_itemlist;	// store item number in modelindex
//	bolt->s.modelindex = 1 + (self->item - bg_itemlist);	// store item number in modelindex
//	bolt->s.modelindex2 = 1; // This is non-zero is it's a dropped item
//	bolt->item = &bg_itemlist[bolt->s.modelindex];

	//PKMOD - Ergodic 07/25/03 - add inivisibility logic to BT,
	//							don't make initial BT invisible until it hits floor
	bolt->s.modelindex2 = 0; // set modelindex2 to zero for initially fired BT

	bolt->parent = self;
	bolt->takedamage = qtrue;

	//PKMOD - Ergodic 10/08/03 - set contents of beartrap so it will interact with
	//			damaging devices
	//		Ergodic 10/10/03 - BODY or'ed with TRIGGER
	//		Ergodic 10/10/03 - DEBUG Stuttering effect... Remove contents body
	//			Note: CONTENTS_BODY will cause Player to either clip onto surface of stutter when moving
	//				Will turn off CONTENTS_BODY after Beartrap has snapped onto player
	bolt->r.contents = CONTENTS_BODY | CONTENTS_TRIGGER;


	//++++
	//PKMOD - Ergodic 10/22/04 - set size of beartrap after beartrap settles on the ground
	//----


	//PKMOD - Ergodic 03/11/01 - set the health of the following beartrap
	bolt->health = 185 + rand() % 50;	//03/11/01 - set health to 185 + random amount
									//q1 original value 50
	
	bolt->methodOfDeath = MOD_BEARTRAP;
	bolt->splashMethodOfDeath = MOD_BEARTRAP;
	bolt->clipmask = MASK_SHOT;
	bolt->touch = BearTrapTouch;
	bolt->die = BearTrapKill;
	bolt->target_ent = NULL;

	//PKMOD - Ergodic 08/05/01 - clear the PKA entity flags (now using a beartrap death flag)
	bolt->pka_flags = 0;

	//PKMOD - Ergodic 08/30/01 - set the team fields so bots will not shoot their own team's beartraps
	//PKMOD - Ergodic 10/10/03 - removed because s.generic1 is already set above
	//bolt->s.generic1 = self->client->sess.sessionTeam;

	//PKMOD - Ergodic 07/05/00 - work on Beartrap boundaries 
	//PKMOD - Ergodic 07/10/01 - remove this section for a test 
//			VectorSet (bolt->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -0.10 * ITEM_RADIUS);
//			VectorSet (bolt->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 1.5 * ITEM_RADIUS);

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
		//Ergodic - 04/16/03 - Shooter logic
	if ( shooterSpeed ) {
		//PKMOD - Ergodic 04/16/03 - set the speed according to the level setting
		VectorScale( dir, shooterSpeed, bolt->s.pos.trDelta );
	} else {
		//PKMOD - Ergodic 08/07/00 increase speed from 300 to 400
		VectorScale( dir, 400, bolt->s.pos.trDelta );
	}

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	trap_LinkEntity (bolt);

	return bolt;
}

//PKMOD
//=============================================================================
//





//PKMOD
//=============================================================================
//

/*
=================
fire_plasma

=================
*/
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "plasma";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->splashDamage = 15;
	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}	

//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_bfg
=================
*/
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;
	VectorCopy( start, bolt->s.pos.trBase );
	//PKMOD - Ergodic 01/10/2001 - increase rocket speed by 20% (was 900)
	VectorScale( dir, 1080, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}


//PKMOD - Ergodic 10/04/00 - abstract the dragon missle speed
//#define	DRAGON_TONGUE_SPEED	800		//was 600 - Ergodic 10/19/00
//PKMOD - Ergodic 07/04/01 - Create separate dragon tongue speeds
#define	DRAGON_TONGUE_LAUNCH_SPEED			850
#define	DRAGON_TONGUE_ITEM_PULL_SPEED		650
#define	DRAGON_TONGUE_DEPLOY_SPEED			700
//+++
//	defined in bg_pmove.c
//
//	#define	DRAGON_TONGUE_PLAYER_PULL_SPEED		800 
//+++



//PKMOD - Ergodic 10/03/00 - think function for returning dragon's tongue
//PKMOD - Ergodic 02/16/01 - change logic to track the item and then place the tongue
//				at the item's location with an offset towrads the parent player
/*
=================
DragonReturn
=================
*/
void DragonReturn ( gentity_t *ent ) {

//	vec3_t		hold_vec;
	vec3_t		hold_vec_item;
	vec3_t		return_point;
	float		dist;
	int			velocity;

	if ( ent->parent->client->hook == NULL ) {	//hook no longer exists, removed by GW
		ent->AirFist_ResetTime = 0;
		ent->nextthink = level.time + 100; 
		ent->think = G_FreeEntity;
	}

	//PKMOD - Ergodic 02/21/01 - check the status of the item
	//			This should handle case of dragon's item being intercepted
	//			by another player
	if ( ent->target_ent ) {		//not null
		if ( ( ent->target_ent->r.svFlags & SVF_NOCLIENT ) ||
					( ent->target_ent->s.eFlags & EF_NODRAW ) ) {
			ent->AirFist_ResetTime = 0;
			ent->nextthink = level.time + 100; 
			ent->think = G_FreeEntity;
		}
	}

	if ( ent->AirFist_ResetTime == 0 ) {	//check dragon's item hold state 
		//if dragon is  no longer carrying an item then end the function
		//PKMOD Ergodic debug 10/04/00
//		Com_Printf( "In DragonReturn - removing entity\n" );
		//PKMOD - Ergodic 10/06/00 - reset the release target_ent's settings
		if ( ent->target_ent != NULL ) { //if target_ent is defined then reset target_ent's gravity
			//PKMOD - Ergodic 10/06/00 - debug inactive
//			Com_Printf( "In DragonReturn - resetting gravity & bounce\n" );
			ent->target_ent->s.pos.trType = TR_GRAVITY;
			ent->target_ent->s.eFlags |= EF_BOUNCE_HALF;
		}
		ent->AirFist_ResetTime = 0;
		Weapon_HookFree(ent->parent->client->hook);
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 10/07/00 - set the return point to the muzzle height
	VectorCopy(ent->parent->r.currentOrigin, return_point);
	//PKMOD - Ergodic 02/16/01 - (whoops) fix the pointer assignment to the parent
//	return_point[2] += ent->client->ps.viewheight;
	//viewheight: DEFAULT_VIEWHEIGHT - 26, CROUCH_VIEWHEIGHT - 12
	// add 2/3's of the viewheight
	if ( ent->parent->client->ps.viewheight > CROUCH_VIEWHEIGHT )
		return_point[2] += 17;
	else
		return_point[2] += 8;

	//Set the return direction - toward the parent
	//PKMOD - Ergodic 02/16/01 - only track the item
//	VectorSubtract(return_point, ent->r.currentOrigin, hold_vec);

	//PKMOD - Ergodic 10/06/00 - Set the return direction - toward the parent
	VectorSubtract(return_point, ent->target_ent->r.currentOrigin, hold_vec_item);

	//PKMOD Ergodic debug 10/06/00 - comment out 
//	G_SetOrigin( ent->target_ent, ent->r.currentOrigin );
//	VectorCopy( ent->r.currentOrigin, ent->target_ent->r.currentOrigin );

	dist = VectorLength( hold_vec_item );

	//PKMOD - Ergodic 10/04/00 - debug inactive 
//	Com_Printf( "In DragonReturn - dist>%f<\n", dist );


	//PKMOD - Ergodic 02/16/01 - reduce the item/tongue speed if close to the player
	//PKMOD - Ergodic 07/04/01 - use item pull speed definition
	if ( dist <= 100 )
		velocity = ( int )DRAGON_TONGUE_ITEM_PULL_SPEED * dist / 100;
	else
		velocity = DRAGON_TONGUE_ITEM_PULL_SPEED;

	//PKMOD - Ergodic 02/16/01 - debug (inactive)
//	Com_Printf( "DragonReturn - %d, dist:%f, velocity:%d\n", level.time, dist, velocity );

	if ( dist >= 50 ) {
		//dragon movement
	//PKMOD - Ergodic 02/16/01 - only track the item
//		VectorNormalize(hold_vec);

		//calculate the return vector velocity

		//PKMOD - Ergodic 02/16/01 - synchronize the tongue's velocity to the item's velocity
		VectorScale(hold_vec_item, velocity, ent->s.pos.trDelta );

		//PKMOD - Ergodic 02/16/01 - synchronize the tongue's location to the item's location
		VectorCopy( ent->target_ent->r.currentOrigin,  ent->r.currentOrigin );

		//PKMOD - Ergodic 03/10/01 - set tongue's location in the old origin for cgame hack
		VectorCopy( ent->r.currentOrigin, ent->s.origin2 );

		//item movement
		VectorNormalize(hold_vec_item);
		VectorScale(hold_vec_item, velocity, ent->target_ent->s.pos.trDelta );
		//PKMOD - Ergodic 10/06/00 - bump up the item by 1 unit
		ent->target_ent->s.pos.trBase[2] += 1;

		//PKMOD - Ergodic 02/12/01 - force the trBase location
		VectorCopy( ent->target_ent->r.currentOrigin, ent->target_ent->s.pos.trBase );

		ent->eventTime = level.time;
		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	} else {
		//PKMOD - Ergodic 10/04/00 - debug inactive 
//		Com_Printf( "In DragonReturn - removing entity\n" );
		ent->AirFist_ResetTime = 0;
		Weapon_HookFree(ent->parent->client->hook);
		ent->nextthink = level.time + 100; 
		ent->think = G_FreeEntity;
	}

}


//PKMOD - Ergodic 10/03/00 - touch function for grapple touching items
/*
=================
DragonTouch
=================
*/
void DragonTouch ( gentity_t *ent, gentity_t *other, trace_t *trace ) {

	//if the dragon hits the parent(owner) then it is "returned" so terminate entity
	if ( other == ent->parent ) {
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 08/25/01 - debug other (inactive)
//	Com_Printf( "DragonTouch - OTHER: classname>%s<, eType>%d<, eTag>%d<\n", other->classname, other->s.eType, other->item->giTag );


	if ( other->s.eType != ET_ITEM ) 
		return;

	if ( ent->AirFist_ResetTime != 0 ) //check dragon's item hold state 
		return;

	ent->AirFist_ResetTime = 1; //set dragon's item hold state to ON

	//set the items return paramters if first touched
	if (( other->AirFist_Level == 0 ) && !( other->flags & FL_DROPPED_ITEM )) {
		VectorCopy(other->s.origin, other->pka_originalposition);
		other->AirFist_Level = 1;
		other->nextthink = level.time + (25 + ( rand() % 11 ) ) * 1000; //restore in ~30 seconds
		other->think = PKA_RestoreItem;
	}

	//PKMOD - Ergodic 10/03/00 set item to be affected by gravity
//	other->s.pos.trType = TR_GRAVITY;
	other->s.pos.trType = TR_LINEAR;	//missile type
	other->s.pos.trTime = level.time;
	//PKMOD - Ergodic 02/12/01 set item to bounce
	other->s.eFlags |= EF_BOUNCE_HALF;

	//PKMOD - Ergodic 10/05/00 set dragon's target item's
	ent->target_ent = other;

	//PKMOD - Ergodic 10/05/00 set dragon's current origin to item's current origin
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );

	//PKMOD - Ergodic 03/10/01 - set tongue's location in the old origin for cgame hack
	VectorCopy( ent->r.currentOrigin, ent->s.origin2 );

	//PKMOD - Ergodic 10/05/00 set dragon's offset time
	ent->s.pos.trTime = level.time;

	ent->nextthink = level.time + 300;
	ent->think = DragonReturn;

	trap_LinkEntity( ent );

	//PKMOD - Ergodic 10/04/00 - debug inactive
//	Com_Printf( "DragonTouch - setting think function\n" );

}

/*
=================
fire_grapple
=================
*/
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*hook;

	//PKMOD - Ergodic 10/05/00 - debug inactive 
//	Com_Printf( "fire_grapple - Initializing...\n" );

	VectorNormalize (dir);

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_MISSILE;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon = WP_GRAPPLING_HOOK;
	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	//PKMOD - Ergodic 10/04/00 - set the clipmask to "player" type
	//PKMOD - Ergodic 01/10/01 - make clipmask the same as rocket
//	hook->clipmask = MASK_SHOT;
	hook->clipmask = CONTENTS_BODY | MASK_SHOT | CONTENTS_TRIGGER; 

	//PKMOD - Ergodic 01/26/01 - debug interaction with autosentry (inactive)
//	hook->r.contents = CONTENTS_BODY;

	//PKMOD - Ergodic 10/04/00 - set missile size
	VectorSet (hook->r.mins, -8, -8, -8);
	VectorSet (hook->r.maxs, 8, 8, 8);

	//PKMOD - Ergodic 10/05/00 - set initial item hold flag to zero
	hook->AirFist_ResetTime = 0;

	//PKMOD - Ergodic 10/06/00 - set target to null
	hook->target_ent = NULL;
	
	hook->parent = self;
	//PKMOD - Ergodic 10/04/00 - set the trigger flag (10/05/00 NOTE: Will cause sticking)
//	hook->r.contents = CONTENTS_TRIGGER;
	//PKMOD - Ergodic 10/03/00 - create a touch function
//	hook->touch = DragonTouch;

	hook->s.pos.trType = TR_LINEAR;
	hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	hook->s.otherEntityNum = self->s.number; // use to match beam in client
	VectorCopy( start, hook->s.pos.trBase );
	//PKMOD - Ergodic 07/04/01 - use launch speed definition
	VectorScale( dir, DRAGON_TONGUE_LAUNCH_SPEED, hook->s.pos.trDelta );
	SnapVector( hook->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, hook->r.currentOrigin);

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player)
	//PKMOD - Ergodic 03/11/01 - clear the dragon return usage vector
	VectorSet (hook->s.origin2, 0, 0, 0);

	self->client->hook = hook;

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player) (inactive)
//	Com_Printf("fire_grapple - start>%s<\n", vtos(start) );

	//PKMOD - Ergodic 10/05/00 - debug size inactive
//	Com_Printf("fire_grapple - mins>%s<, maxs.%s<\n", vtos(hook->r.mins), vtos(hook->r.maxs));

	return hook;
}

/*
=================
fire_dragon_deploy

PKMOD - Ergodic 03/14/01 - add dragon deployable weapon fire
=================
*/
gentity_t *fire_dragon_deploy (gentity_t *self, vec3_t start, vec3_t dir ) {
	gentity_t	*hook;
	//PKMOD - Ergodic 08/24/01 - localize deployed weapon number
	int			deployed_weapon;

	//PKMOD - Ergodic 03/24/01 - debug (inactive)
//	Com_Printf( "fire_dragon_deploy - Initializing...\n" );

	VectorNormalize (dir);

	//PKMOD - Ergodic 08/24/01 - localize deployed weapon number
	deployed_weapon = self->client->ps.generic1;

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_DRAGON_DEPLOY;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;

	//PKMOD - Ergodic 07/11/01 - set the direction for autosentry launch direct
	VectorCopy ( dir, hook->s.angles );


	hook->s.generic1 = deployed_weapon;
	hook->s.weapon = WP_GRAPPLING_HOOK;

	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	//PKMOD - Ergodic 10/04/00 - set the clipmask to "player" type
	//PKMOD - Ergodic 01/10/01 - make clipmask the same as rocket
//	hook->clipmask = MASK_SHOT;
	hook->clipmask = CONTENTS_BODY | MASK_SHOT | CONTENTS_TRIGGER; 

	//PKMOD - Ergodic 01/26/01 - debug interaction with autosentry (inactive)
//	hook->r.contents = CONTENTS_BODY;

	//PKMOD - Ergodic 10/04/00 - set missile size
	VectorSet (hook->r.mins, -8, -8, -8);
	VectorSet (hook->r.maxs, 8, 8, 8);

	//PKMOD - Ergodic 10/05/00 - set initial item hold flag to zero
	hook->AirFist_ResetTime = 0;

	//PKMOD - Ergodic 06/23/01 - add team parameters
	hook->s.modelindex = self->client->sess.sessionTeam;


	//PKMOD - Ergodic 10/06/00 - set target to null
	hook->target_ent = NULL;
	
	hook->parent = self;
	//PKMOD - Ergodic 10/04/00 - set the trigger flag (10/05/00 NOTE: Will cause sticking)
//	hook->r.contents = CONTENTS_TRIGGER;
	//PKMOD - Ergodic 10/03/00 - create a touch function
//	hook->touch = DragonTouch;

	hook->s.pos.trType = TR_LINEAR;
	hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	hook->s.otherEntityNum = self->s.number; // use to match beam in client
	VectorCopy( start, hook->s.pos.trBase );
	//PKMOD - Ergodic 07/04/01 - use launch speed definition
	VectorScale( dir, DRAGON_TONGUE_DEPLOY_SPEED, hook->s.pos.trDelta );
	SnapVector( hook->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, hook->r.currentOrigin);

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player)
	//PKMOD - Ergodic 03/11/01 - clear the dragon return usage vector
	VectorSet (hook->s.origin2, 0, 0, 0);

	self->client->hook = hook;

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player) (inactive)
//	Com_Printf("fire_grapple - start>%s<\n", vtos(start) );

	//PKMOD - Ergodic 10/05/00 - debug size inactive
//	Com_Printf("fire_grapple - mins>%s<, maxs.%s<\n", vtos(hook->r.mins), vtos(hook->r.maxs));


	//PKMOD - Ergodic 08/25/01 - optimze the dragon deploy variable
	deployed_weapon = deployed_weapon & 15;

	//PKMOD - Ergodic 08/25/01 - validate the dragon deploy
	//		prevents bug of try to deploy a "0" weapon number
	//PKMOD - Ergodic 12/15/01 - add gauntlet to deployable weapon list
	if ( ( deployed_weapon == WP_GRAVITY ) || ( deployed_weapon == WP_SENTRY ) ||
		( deployed_weapon == WP_BEARTRAP ) || ( deployed_weapon == WP_BEANS ) || 
		( deployed_weapon == WP_GAUNTLET ) ) {
	
		//PKMOD - Ergodic 12/15/01 - only decrement the ammo if not the gauntlet
		if ( deployed_weapon != WP_GAUNTLET ) {
			//decrement the  players ammo
			self->client->ps.ammo[ ( deployed_weapon ) ] -= 1;

			//if out of ammo then remove weapon from inventory
			//PKMOD - Ergodic 08/14/01 - change comparison from "== 0" to "< 1" to fix
			//			infinite ammo bug
			//PKMOD - Ergodic 08/24/01 - localize deployed weapon number
			if ( self->client->ps.ammo[ ( deployed_weapon ) ] < 1 ) {
				//PKMOD - Ergodic 08/24/01 - debug removal of machinegun when testing infinite ammo bug (inactive)
//				Com_Printf("fire_dragon_deploy - deployed_weapon>%d<, generic1>%d<, removing weapon>%d<\n", deployed_weapon, self->client->ps.generic1, self->client->ps.generic1 & 15 );

				self->client->ps.ammo[ ( deployed_weapon ) ] = 0;
				self->client->ps.stats[STAT_WEAPONS] &= ~( 1 << ( deployed_weapon ) );
				//PKMOD - Ergodic 06/23/01 - if out of ammo, reset the deploy weapon
				self->client->ps.generic1 = 0; 
			}
		}
	}
	else {
		//PKMOD - Ergodic 08/29/01 - check if player is carrying the flag
		if ( self->client->ps.powerups[PW_REDFLAG] ) {
			self->client->ps.generic1 = PW_REDFLAG;
				//PKMOD - Ergodic 08/29/01 - clear the dragon deploy variable 
				self->client->ps.generic1 = 0;
				self->client->ps.powerups[PW_REDFLAG] = 0; //clear the redflag
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {
				self->client->ps.generic1 = PW_BLUEFLAG;
				//PKMOD - Ergodic 08/29/01 - clear the dragon deploy variable 
				self->client->ps.generic1 = 0;
				self->client->ps.powerups[PW_BLUEFLAG] = 0; //clear the blueflag
			}
			else
				//PKMOD - Ergodic 08/29/01 - clear the dragon deploy variable 
				self->client->ps.generic1 = 0;
	}

	return hook;
}


/*
=================
fire_dragon_deploy

PKMOD - Ergodic 03/14/01 - activate dragon deployable weapon upon contact with a surface
=================
*/
//PKMOD - Ergodic 12/20/01 - modify activate_dragon_deploy call to add struck entity
//PKMOD - Ergodic 01/26/02 - modify activate_dragon_deploy call to add bytedir for gauntlet
void activate_dragon_deploy ( gentity_t *ent, gentity_t *other, int bytedir ) {
	gentity_t	*m;
	vec3_t		hold_direction;
	gitem_t		*item;

	//PKMOD - Ergodic 03/24/01 - debug (inactive)
//	Com_Printf( "activate_dragon_deploy - Initializing...\n" );

	VectorNormalize2( ent->s.pos.trDelta, hold_direction );

	switch ( ent->s.generic1 & 15 ) {
		case	WP_GRAVITY:
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "fire_dragon_deploy - WP_GRAVITY\n" );
//			if ( VectorLength (hold_direction) > 0 )
//				return;

			m = fire_gravity( ent->parent, ent->r.currentOrigin, hold_direction );
			break;
		case 	WP_SENTRY:
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "fire_dragon_deploy - WP_SENTRY\n" );
//			if ( VectorLength (hold_direction) > 0 )
//				return;

			m = fire_autosentry( ent->parent, ent->r.currentOrigin, hold_direction );
			break;
		case 	WP_BEARTRAP:
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "fire_dragon_deploy - WP_BEARTRAP\n" );
//			if ( VectorLength (hold_direction) > 0 )
//				return;

			// Ergodic 04/16/03 - ShooterSpeed set to 0
			m = fire_beartrap( ent->parent, ent->r.currentOrigin, hold_direction, 0 );
			break;
		case 	WP_BEANS:
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "fire_dragon_deploy - WP_BEANS\n" );
//			if ( VectorLength (hold_direction) > 0 )
//				return;

			item = BG_FindItemForWeapon( WP_BEANS );

			//PKMOD - Ergodic 05/25/01 - set the location
//			VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase);

			//PKMOD - Ergodic 07/25/01 - modify call to new custom function
			m = Throw_Item( ent, item);
			break;
			
		//PKMOD - Ergodic 08/29/01 - add case for deploying flag
		case 	PW_REDFLAG:
			//PKMOD - Ergodic 03/04/01 - debug (inactive)
//			Com_Printf( "Cmd_DragonDeploy_f - PW_REDFLAG\n" );
			item = BG_FindItemForPowerup( PW_REDFLAG );
			m = Throw_Item( ent, item);
			ent->parent->client->ps.powerups[PW_REDFLAG] = 0;	//Clear the flag ownership
			break;

		//PKMOD - Ergodic 08/29/01 - add case for deploying flag
		case 	PW_BLUEFLAG:
			//PKMOD - Ergodic 03/04/01 - debug (inactive)
//			Com_Printf( "Cmd_DragonDeploy_f - PW_BLUEFLAG\n" );
			item = BG_FindItemForPowerup( PW_BLUEFLAG );
			m = Throw_Item( ent, item);
			ent->parent->client->ps.powerups[PW_REDFLAG] = 0;	//Clear the flag ownership
			break;

			//PKMOD - Ergodic 12/20/01 - add case for deploying gauntlet_blade
		case 	WP_GAUNTLET:
			//PKMOD - Ergodic 12/20/01 - debug (inactive)
//			Com_Printf( "activate_dragon_deploy - WP_GAUNTLET\n" );
			// send blood impact
			//PKMOD - Ergodic 01/26/02 - add damage to player
			if ( other->takedamage ) {
				gentity_t	*tent;
				int			damage;

				if ( other->client ) {
					tent = G_TempEntity( ent->r.currentOrigin, EV_MISSILE_HIT );
					tent->s.otherEntityNum = other->s.number;
					tent->s.eventParm = bytedir;
					tent->s.weapon = ent->s.weapon;
				}

				damage = 35;

				if (ent->parent->client->ps.powerups[PW_QUAD] ) {
					G_AddEvent( ent->parent, EV_POWERUP_QUAD, 0 );
					damage *= g_quadfactor.value;
				}
				//PKMOD - Ergodic 10/23/02 - Create new Means of Death for the Dragon Blade
				G_Damage( other, ent->parent, ent->parent, hold_direction, ent->r.currentOrigin, damage, 0, MOD_DRAGONBLADE );

			}
			break;

		default:	//exit since current weapon can not be deployed (default to tongue)
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
			//			Com_Printf( "G_RunMissile - invalid weapon for dragon deploy\n" );

			//PKMOD - Ergodic 03/25/01 - Clear the dragon hold state of the missile
			//PKMOD - Ergodic 05/08/01 - change logic to break out of switch statement,
			//			this may fix opstack overflow
			ent->s.generic1 = 0;
			break;
	}

	//PKMOD - Ergodic 08/29/01 - disable the following if statement...
	//PKMOD - Ergodic 05/08/01 - add exit for invalid deployable weapon
//	if ( ent->s.generic1 == 0 )	
//			return;

//	if ( ( ent->s.generic1 & 15 ) != WP_BEANS ) {
//		m->damage *= ent->damage;
//		m->splashDamage *= ent->damage;
//	}

	//PKMOD - Ergodic 03/25/01 - Clear the dragon hold state of the missile
	ent->s.generic1 = 0;

}

#ifdef MISSIONPACK
/*
=================
fire_nail
=================
*/
#define NAILGUN_SPREAD	500

gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up ) {
	gentity_t	*bolt;
	vec3_t		dir;
	vec3_t		end;
	float		r, u, scale;

	bolt = G_Spawn();
	bolt->classname = "nail";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAILGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->methodOfDeath = MOD_NAIL;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( start, bolt->s.pos.trBase );

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * NAILGUN_SPREAD * 16;
	r = cos(r) * crandom() * NAILGUN_SPREAD * 16;
	VectorMA( start, 8192 * 16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );

	scale = 555 + random() * 1800;
	VectorScale( dir, scale, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );

	VectorCopy( start, bolt->r.currentOrigin );

	return bolt;
}	


/*
=================
fire_prox
=================
*/
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t dir ) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "prox mine";
	bolt->nextthink = level.time + 3000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PROX_LAUNCHER;
	bolt->s.eFlags = 0;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_PROXIMITY_MINE;
	bolt->splashMethodOfDeath = MOD_PROXIMITY_MINE;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	// count is used to check if the prox mine left the player bbox
	// if count == 1 then the prox mine left the player bbox and can attack to it
	bolt->count = 0;

	//FIXME: we prolly wanna abuse another field
	bolt->s.generic1 = self->client->sess.sessionTeam;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
#endif

