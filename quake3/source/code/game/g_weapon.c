// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"

static	float	s_quadFactor;
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;

#define NUM_NAILSHOTS 10

//PKMOD - Ergodic 05/25/01 add definition for beartrap and gravity well launch sound (Player "umph")
void	PM_AddEvent( int newEvent );

/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
//PKMOD - Ergodic 02/11/02 - move this code section to the beginning of the file
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}


/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
//PKMOD - Ergodic 02/11/02 - move this code section to the beginning of the file
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}


/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET

======================================================================
*/

void Weapon_Gauntlet( gentity_t *ent ) {

}

/*
===============
CheckGauntletAttack
===============
*/
qboolean CheckGauntletAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, 32, forward, end);

	trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}

	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	if (ent->client->ps.powerups[PW_QUAD] ) {
		G_AddEvent( ent, EV_POWERUP_QUAD, 0 );
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if( ent->client->persistantPowerup && ent->client->persistantPowerup->item && ent->client->persistantPowerup->item->giTag == PW_DOUBLER ) {
		s_quadFactor *= 2;
	}
#endif

	damage = 50 * s_quadFactor;
	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_GAUNTLET );

	return qtrue;
}

/*PKMOD -Add Weapons. 
	WP_GRAVITY,
	WP_SENTRY,
	WP_BEARTRAP,
	WP_CHAINLG,
	WP_AIRFIST,
	WP_NAILGUN,
	PKMOD -Add Weapons. */
/*
=======================================================================
GRAVITY WELL
=======================================================================
*/
//PKMOD - Ergodic 05/17/00 reset quad functionality in gravity well 
void Weapon_fire_gravity (gentity_t *ent ) {
       gentity_t *m;

		PM_AddEvent( EV_JUMP );  //Ergodic 05/25/01 - add hurl sound (same as jump sound)

		// extra vertical velocity
		//PKMOD - Ergodic 12/17/00 - add "f" to force float for 1.27g compliance 
		forward[2] += 0.2f;
		VectorNormalize( forward );
		m = fire_gravity(ent, muzzle, forward);
		//PKMOD - Ergodic 05/08/01 - don't need to quad the gravity well
//		m->damage *= s_quadFactor;
//		m->splashDamage *= s_quadFactor;
}



/*
=======================================================================
AUTO SENTRY
=======================================================================
*/
void Weapon_fire_autosentry (gentity_t *ent ) {
		gentity_t *m;

		PM_AddEvent( EV_JUMP );  //Ergodic 05/25/01 - add hurl sound (same as jump sound)

		m = fire_autosentry(ent, muzzle, forward);
		//PKMOD - Ergodic 05/08/01 - don't need to quad the autosentry
		//			fixes bug with quad always on
//		m->damage *= s_quadFactor;
//		m->splashDamage *= s_quadFactor;
}
/*
=======================================================================
BEAR TRAP
=======================================================================
*/
void Weapon_fire_beartrap (gentity_t *ent ) {
		gentity_t *m;

		PM_AddEvent( EV_JUMP );  //Ergodic 05/25/01 - add hurl sound (same as jump sound)

		// Ergodic 04/16/03 - ShooterSpeed set to 0
		m = fire_beartrap(ent, muzzle, forward, 0);
		//PKMOD - Ergodic 05/08/01 - don't need to quad the beartrap
//		m->damage *= s_quadFactor;
//		m->splashDamage *= s_quadFactor;

}
/*
=======================================================================
CHAIN LIGHTNING GUN
=======================================================================
*/
/*PKMOD - Ergodic 06/05/03 - remove dead code for CLG...
void Weapon_fire_chainlg (gentity_t *ent ) {
       gentity_t *m;

       m = fire_chainlg(ent, muzzle, forward);
       m->damage *= s_quadFactor;
       m->splashDamage *= s_quadFactor;
}
end 06/05/03 remove section*/

/*
=======================================================================
AIR FIST
=======================================================================
*/

//PKMOD - Ergodic 02/19/01 - make defintions in order to optimize
#define		AF_KICKBACK_VELOCITY 250
#define		AF_KICKBACK_DISTANCE 175
//PKMOD - Ergodic 04/25/01 - change quadfactor from 1.3f to 1.7f
//PKMOD - Ergodic 12/16/02 - change quadfactor from 1.7f to 2.1f
//#define		AF_MAX_QUADFACTOR 1.7f	
#define		AF_MAX_QUADFACTOR 2.1f	

void Weapon_fire_airfist (gentity_t *ent) {
	float		hold_af_effect;
	vec3_t		forward, right, up;
	trace_t		tr;
	vec3_t		kickback_proximity;
	float		AF_WATER_EFFECT = 1.0;
	gentity_t	*m;
	int			hold_af_level;
	float		local_quadFactor;

/*Constants...
	AF_MAX_RADIUS
	AF_MAX_DAMAGE
	AF_MAX_VELOCITY
	MOD_AIRFIST
	AF_UP_VELOCITY
*/
	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	hold_af_level = ent->client->ps.stats[STAT_AIRFIST_LEVEL];

	//PKMOD - Ergodic 11/16/00, copy current airfist level to holding stat
	//			airfist levels will be (4,3,2,1,0)
	ent->client->ps.stats[STAT_PRIOR_AIRFIST_LEVEL] = hold_af_level;

	//Ergodic - AirFist kickback code 02/20/00
	//PKMOD - Ergodic 12/17/00 - add "f" to force float for 1.27g compliance 
	if (ent->waterlevel > 2)
		AF_WATER_EFFECT = 0.4f;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	//PKMOD - Ergodic 10/10/00 - move fire_airfist call after setting direction (duh)

	//PKMOD - Ergodic 02/19/01 - set the quadfactor max to AF_MAX_QUADFACTOR
	if ( s_quadFactor > AF_MAX_QUADFACTOR )
		local_quadFactor = AF_MAX_QUADFACTOR;
	else
		local_quadFactor = s_quadFactor;

	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	if ( hold_af_level != 0 )
		m =	fire_airfist (ent, muzzle, forward, local_quadFactor);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );
	VectorMA (muzzle, AF_KICKBACK_DISTANCE, forward, kickback_proximity);
//	VectorMA (ent->r.currentOrigin, KICKBACK_DISTANCE, ent->client->ps.viewangles, kickback_proximity);

//  determine if we are facing any solid surface (Ground/Wall)
	trap_Trace (&tr, muzzle, NULL, NULL, kickback_proximity, ent->s.number, CONTENTS_SOLID);
//Ergodic Debug
//Com_Printf("trace contents = %i\n", tr.contents);
//Com_Printf("trace surfaceFlags = %i\n", tr.surfaceFlags);

	//Set the raw kickback velocity
	//if the AirFist is exhausted then set value to very low, but NOT zero
	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	//PKMOD - Ergodic 12/17/00 - add "f" to force float for 1.27g compliance 
	//PKMOD - Ergodic 02/19/01 - fix conversion of STAT_AIRFIST_LEVEL
	switch ( hold_af_level ) {
	case 4:
		hold_af_effect = 1.00f;
		break;
	case 3:
		hold_af_effect = 0.90f;
		break;
	case 2:
		hold_af_effect = 0.75f;
		break;
	case 1:
		hold_af_effect = 0.50;
		break;
	default: 
		hold_af_effect = 0.10f;
		break;
	}


	//calculate any additional kicback if facing a close by surface
	if ( tr.contents & CONTENTS_SOLID ) {
		//did we hit a solid object (wall,floor)
//		VectorSubtract(tr.endpos, ent->r.currentOrigin, hold_vec);
//		dist = VectorLength( hold_vec );
//Ergodic Debug
//Com_Printf("kickback tr.fraction * 100 = %f\n", tr.fraction * 100.0);
		hold_af_effect *= - 3.0 * ( 1.0 - tr.fraction );
	}
	else {
		//PKMOD - Ergodic 02/19/01 - not next to any thing
		hold_af_effect *= - 2.0;
	}

	//Complete the kickback velocity calculation
	//Set the raw kickback velocity
	hold_af_effect *= (local_quadFactor * AF_KICKBACK_VELOCITY * AF_WATER_EFFECT);

//PKMOD Ergodic debug (inactive)
//	Com_Printf("Weapon_fire_airfist: AirFist_Level:%d, effect*100:%f\n", hold_af_level, 100*hold_af_effect);  //Ergodic debug

	//set the kickback pulse velocity
	VectorMA(ent->client->ps.velocity,hold_af_effect,forward,ent->client->ps.velocity);
//set an upward displacement
//		ent->client->ps.velocity[2] += AF_UP_VELOCITY * hold_af_effect / 2;

	//reduce the AirFist_Level
	//PKMOD - Ergodic 11/15/00, move airfist level to playerstate
	//			airfist levels will be (4,3,2,1,0)
	if ( hold_af_level > 0 )
		ent->client->ps.stats[STAT_AIRFIST_LEVEL]--;
}

/*
=======================================================================
NAIL GUN
=======================================================================
*/
void Weapon_fire_nailgun (gentity_t *ent ) {
       gentity_t *m;

       m = fire_nailgun(ent, muzzle, forward);
       m->damage *= s_quadFactor;
       m->splashDamage *= s_quadFactor;
}
//PKMOD


/*
======================================================================

MACHINEGUN

======================================================================
*/

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = (int)v[i];
		} else {
			v[i] = (int)v[i] + 1;
		}
	}
}

#ifdef MISSIONPACK
#define CHAINGUN_SPREAD		600
#endif
#define MACHINEGUN_SPREAD	200
#define	MACHINEGUN_DAMAGE	7
#define	MACHINEGUN_TEAM_DAMAGE	5		// wimpier MG in teamplay

//PKMOD - Ergodic 12/03/00 - modify routine so damage is externally quaded
void Bullet_Fire (gentity_t *ent, float spread, int damage ) {
	trace_t		tr;
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t		impactpoint, bouncedir;
#endif
	float		r;
	float		u;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			i, passent;

//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
//	damage *= s_quadFactor;

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * spread * 16;
	r = cos(r) * crandom() * spread * 16;
	VectorMA (muzzle, 8192*16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	passent = ent->s.number;
	for (i = 0; i < 10; i++) {

		//PKMOD - Ergodic 01/30/01 - send a trace to detect an autosentry
		trap_Trace( &tr, muzzle,  NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			return;
		}
		traceEnt = &g_entities[ tr.entityNum ];

		//PKMOD - Ergodic 06/22/01 - Debug zombie hits (inactive)
//		Com_Printf( "Bullet_Fire: traceEnt->classname>%s<, traceEnt->s.eType>%d<\n", traceEnt->classname, traceEnt->s.eType );


		//PKMOD - Ergodic 01/30/01 - if NOT detect an autosentry with this specific comparison...
		if ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) {
			//We did not hit an autosentry so send trace to detect a passent entity
			trap_Trace (&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT );
			traceEnt = &g_entities[ tr.entityNum ];
		}

		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			return;
		}

		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards( tr.endpos, muzzle );

		// send bullet impact
		//PKMOD - Ergodic 01/13/01 - include damage the autosentry turret
		if ( traceEnt->takedamage && (traceEnt->client || ( traceEnt->s.eType == ET_AUTOSENTRY_TURRET ) )  ) {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
			tent->s.eventParm = traceEnt->s.number;
			if( LogAccuracyHit( traceEnt, ent ) ) {
				ent->client->accuracy_hits++;
			}
		} else {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
			tent->s.eventParm = DirToByte( tr.plane.normal );
		}
		tent->s.otherEntityNum = ent->s.number;

		if ( traceEnt->takedamage) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if (G_InvulnerabilityEffect( traceEnt, forward, tr.endpos, impactpoint, bouncedir )) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					VectorCopy( impactpoint, muzzle );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else {
					VectorCopy( tr.endpos, muzzle );
					passent = traceEnt->s.number;
				}
				continue;
			}
			else {
#endif
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_MACHINEGUN);
#ifdef MISSIONPACK
			}
#endif
		}
		break;
	}

}

//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
void shooter_bullet_fire (gentity_t *ent, vec3_t start, vec3_t dir) {
	VectorCopy(start, muzzle);
	VectorNormalize2( dir, forward );
	Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE );
}

/*
======================================================================

BFG

======================================================================
*/

void BFG_Fire ( gentity_t *ent ) {
	gentity_t	*m;

	m = fire_bfg (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

qboolean ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent ) {
	trace_t		tr;
	int			damage, i, passent;
	gentity_t	*traceEnt;
#ifdef MISSIONPACK
	vec3_t		impactpoint, bouncedir;
#endif
	vec3_t		tr_start, tr_end;

	passent = ent->s.number;
	VectorCopy( start, tr_start );
	VectorCopy( end, tr_end );
	for (i = 0; i < 10; i++) {

		//PKMOD - Ergodic 01/30/01 - send a trace to detect an autosentry
		trap_Trace( &tr, tr_start,  NULL, NULL, tr_end, ENTITYNUM_NONE, MASK_SHOT );
		traceEnt = &g_entities[ tr.entityNum ];

		//PKMOD - Ergodic 01/30/01 - if NOT detect an autosentry with this specific comparison...
		if ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) {
			//We did not hit an autosentry so send trace to detect a passent entity
			trap_Trace (&tr, tr_start, NULL, NULL, tr_end, passent, MASK_SHOT );
			traceEnt = &g_entities[ tr.entityNum ];
		}

			//PKMOD - Ergodic 01/13/01 - debug inactive
//	if ( traceEnt->takedamage)
//		Com_Printf("ShotgunPellet with damage - traceEnt->classname >%s<\n",traceEnt->classname );
//	else
//		Com_Printf("ShotgunPellet without damage - traceEnt->classname >%s<\n",traceEnt->classname );
//	if ( traceEnt->s.eType == ET_AUTOSENTRY_BASE ) {
//		Com_Printf("ShotgunPellet - autosentry base\n" );
//	}
//	if ( traceEnt->s.eType == ET_AUTOSENTRY_TURRET ) {
//		Com_Printf("ShotgunPellet - autosentry turret\n" );
//	}


		// send bullet impact
		if (  tr.surfaceFlags & SURF_NOIMPACT ) {
			return qfalse;
		}

		if ( traceEnt->takedamage) {
			damage = DEFAULT_SHOTGUN_DAMAGE * s_quadFactor;
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if (G_InvulnerabilityEffect( traceEnt, forward, tr.endpos, impactpoint, bouncedir )) {
					G_BounceProjectile( tr_start, impactpoint, bouncedir, tr_end );
					VectorCopy( impactpoint, tr_start );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else {
					VectorCopy( tr.endpos, tr_start );
					passent = traceEnt->s.number;
				}
				continue;
			}
			else {
				G_Damage( traceEnt, ent, ent, forward, tr.endpos,
					damage, 0, MOD_SHOTGUN);
				if( LogAccuracyHit( traceEnt, ent ) ) {
					return qtrue;
				}
			}
#else
			G_Damage( traceEnt, ent, ent, forward, tr.endpos,	damage, 0, MOD_SHOTGUN);
				if( LogAccuracyHit( traceEnt, ent ) ) {
					return qtrue;
				}
#endif
		}
		return qfalse;
	}
	return qfalse;
}

// this should match CG_ShotgunPattern
void ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;
	int			oldScore;
	qboolean	hitClient = qfalse;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	oldScore = ent->client->ps.persistant[PERS_SCORE];

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		if( ShotgunPellet( origin, end, ent ) && !hitClient ) {
			hitClient = qtrue;
			ent->client->accuracy_hits++;
		}
	}

}


void weapon_supershotgun_fire (gentity_t *ent) {
	gentity_t		*tent;

	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;
	//PKMOD - Ergodic 01/16/01 - pass into the entity the number of the weapon
	tent->s.weapon = WP_SHOTGUN;

	ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
}

//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
void shooter_supershotgun_fire (gentity_t *ent, vec3_t start, vec3_t dir) {
	gentity_t		*tent;

	// send shotgun blast
	tent = G_TempEntity( start, EV_SHOTGUN );
	VectorScale( dir, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;
	tent->s.weapon = WP_SHOTGUN;

	ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize( forward );

	m = fire_grenade (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_rocket (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void Weapon_Plasmagun_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_plasma (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
======================================================================

RAILGUN

======================================================================
*/


/*
=================
weapon_railgun_fire
=================
*/
#define	MAX_RAIL_HITS	4

//PKMOD - Ergodic 07/12/01 - set infront value - Attacker is infront of target if higher than this value
#define RAILGUN_SHIELD_INFRONT 0.70	

void weapon_railgun_fire (gentity_t *ent) {
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t impactpoint, bouncedir;
#endif
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	int			hold_damage;
	int			i;
	int			hits;
	int			unlinked;
	int			passent;
	gentity_t	*unlinkedEntities[MAX_RAIL_HITS];
	vec3_t		hold_vec;
	vec3_t		hold_forward, hold_right, hold_up, hold_muzzle;
	float		dot;

	damage = 100 * s_quadFactor;

	VectorMA (muzzle, 8192, forward, end);

	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;
	do {

		hold_damage = damage;

		//PKMOD - Ergodic 01/30/01 - send a trace to detect an autosentry
		trap_Trace( &trace, muzzle,  NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );
		traceEnt = &g_entities[ trace.entityNum ];

		//PKMOD - Ergodic 01/30/01 - if NOT detect an autosentry with this specific comparison...
		if ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) {
			//We did not hit an autosentry so send trace to detect a passent entity
			trap_Trace (&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );
			traceEnt = &g_entities[ trace.entityNum ];
		}

		if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL ) {
			break;
		}

		//PKMOD - Ergodic 01/30/01 - this is done before
//		traceEnt = &g_entities[ trace.entityNum ];

		if ( traceEnt->takedamage ) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if ( G_InvulnerabilityEffect( traceEnt, forward, trace.endpos, impactpoint, bouncedir ) ) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					// snap the endpos to integers to save net bandwidth, but nudged towards the line
					SnapVectorTowards( trace.endpos, muzzle );
					// send railgun beam effect
					tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );
					// set player number for custom colors on the railtrail
					tent->s.clientNum = ent->s.clientNum;
					VectorCopy( muzzle, tent->s.origin2 );
					// move origin a bit to come closer to the drawn gun muzzle
					VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
					VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );
					tent->s.eventParm = 255;	// don't make the explosion at the end
					//
					VectorCopy( impactpoint, muzzle );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
			}
			else {
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
#else
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				//PKMOD - Ergodic 07/12/01 - add shield for railgun if target is holding a sentry 
				//			Note: shield effect only works for the target holding an autosentry 
				//					that is aimed at the attacking player
				if ( ( traceEnt->client ) && ( traceEnt->s.weapon == WP_SENTRY ) ) { 

					//PKMOD - Ergodic 07/03/01 - calculate if target is facing attacker
					AngleVectorsForward ( traceEnt->client->ps.viewangles, hold_forward );
					VectorSubtract( ent->r.currentOrigin, traceEnt->r.currentOrigin, hold_vec) ;

					VectorNormalize ( hold_vec );

					dot = DotProduct (hold_vec, hold_forward);

					if ( dot > RAILGUN_SHIELD_INFRONT ) {
						//reduce damage to 10%
						//PKMOD - Ergodic 12/16/02 - change reflected damge to 5%
						hold_damage = damage / 20;
	
						//PKMOD - Ergodic 02/11/02 - autosentry shield will reflect rail beam 
						//PKMOD - reflect the rail beam back towards the attacker
//						G_HalfBounceProjectile( muzzle, trace.endpos, hold_forward, end );
						CalcMuzzlePointOrigin ( traceEnt, traceEnt->client->oldOrigin, hold_forward, hold_right, hold_up, hold_muzzle );
						VectorMA (hold_muzzle, 8192*16, hold_forward, end);

						// snap the endpos to integers to save net bandwidth, but nudged towards the line
						SnapVectorTowards( trace.endpos, muzzle );
						// send railgun beam effect
						tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );
						// set player number for custom colors on the railtrail
						tent->s.clientNum = ent->s.clientNum;
						VectorCopy( muzzle, tent->s.origin2 );
						// move origin a bit to come closer to the drawn gun muzzle
						VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
						VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );
						tent->s.eventParm = 255;	// don't make the explosion at the end
						//
						VectorCopy( trace.endpos, muzzle );
						// the player can hit him/herself with the bounced rail
						passent = ENTITYNUM_NONE;
					}

				}
				//PKMOD - Ergodic 10/28/02 - use target's angles and not the rail holder's
//				G_Damage (traceEnt, ent, ent, forward, trace.endpos, hold_damage, 0, MOD_RAILGUN);
				G_Damage (traceEnt, ent, ent, hold_forward, trace.endpos, hold_damage, 0, MOD_RAILGUN);
#endif
		}
		if ( trace.contents & CONTENTS_SOLID ) {
			break;		// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity( traceEnt );
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while ( unlinked < MAX_RAIL_HITS );

	// link back in any entities we unlinked
	for ( i = 0 ; i < unlinked ; i++ ) {
		trap_LinkEntity( unlinkedEntities[i] );
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;

	//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters (check for client)
	if ( ent->client ) {
		// give the shooter a reward sound if they have made two railgun hits in a row
		if ( hits == 0 ) {
			// complete miss
			ent->client->accurateCount = 0;
		} else {
			// check for "impressive" reward sound
			ent->client->accurateCount += hits;
			if ( ent->client->accurateCount >= 2 ) {
				ent->client->accurateCount -= 2;
				ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
				// add the sprite over the player's head
				//PKMOD - Ergodic 08/08/00 add reset for PAINKILLER awarded
				ent->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP  | EF_AWARD_PAINKILLER );
				ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
				ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
			}
		}
		ent->client->accuracy_hits++;
	}

}

//PKMOD - Ergodic 11/21/00 - add new types of PKA shooters
void shooter_railgun_fire (gentity_t *ent, vec3_t start, vec3_t dir) {
	s_quadFactor = 1;
	VectorCopy(start, muzzle);
	VectorNormalize2( dir, forward );
	weapon_railgun_fire( ent );
}

/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

void Weapon_GrapplingHook_Fire (gentity_t *ent)
{
		//PKMOD - Ergodic 10/05/00 - debug inactive 
//	Com_Printf( "Weapon_GrapplingHook_Fire - Initializing..." );

		//PKMOD - Ergodic 10/05/00 - debug inactive 
//	if ( !ent->client->fireHeld )
//		Com_Printf( " (!fireHeld) &&" );
//	else
//		Com_Printf( " (fireHeld) &&" );

		//PKMOD - Ergodic 10/05/00 - debug inactive 
//	if ( !ent->client->hook )
//		Com_Printf( " (!hook)\n" );
//	else
//		Com_Printf( " (hook)\n" );

	//PKMOD - Ergodic 03/14/01 - add alternate fire on Dragon "dragon deploy"
	if (!ent->client->fireHeld && !ent->client->hook) {
		if ( ent->client->ps.generic1 == 0 ) {
			//PKMOD - Ergodic 03/14/01 - debug (inactive)
//			Com_Printf( "Weapon_GrapplingHook_Fire - fire_grapple\n" );
			fire_grapple (ent, muzzle, forward);
		}
		else {
			//PKMOD - Ergodic 03/14/01 - debug  (inactive)
//			Com_Printf( "Weapon_GrapplingHook_Fire - fire_dragon_deploy\n" );
			fire_dragon_deploy ( ent, muzzle, forward );
			//PKMOD - Ergodic 06/23/01 - don't reset the deploy weapon unless
			//			out of ammo - This logic will be handled in "fire_dragon_deploy"
//			ent->client->ps.generic1 = 0; 
		}
	}

	ent->client->fireHeld = qtrue;
}

void Weapon_HookFree (gentity_t *ent)
{
	//PKMOD - Ergodic 10/06/00 - reset the release target_ent's settings
	if ( ent->target_ent != NULL ) { //if target_ent is defined then reset target_ent's gravity
		//PKMOD - Ergodic 10/06/00 - debug inactive
//		Com_Printf( "In DragonReturn - resetting gravity & bounce\n" );
		ent->target_ent->s.pos.trType = TR_GRAVITY;
		ent->target_ent->s.eFlags |= EF_BOUNCE_HALF;
	}

	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity( ent );
}

void Weapon_HookThink (gentity_t *ent)
{
	if (ent->enemy) {
		vec3_t v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards( v, oldorigin );	// save net bandwidth

		G_SetOrigin( ent, v );
	}

	VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}

/*
======================================================================

LIGHTNING GUN

======================================================================
*/

//PKMOD - Ergodic 12/29/00 - set cap of entity search to 128 (instead of 1024)
#define	MAX_CHAINLIGHTNING_SEARCH 128
//PKMOD - Ergodic 12/29/00 - set MAX chainlightning distance to 512 (instead of 768)
#define	MAX_CHAINLIGHTNING_RANGE 512
//PKMOD - Ergodic 04/21/01 - set lightning damage (was 8)
#define	CHAINLIGHTNING_DAMAGE 16
//PKMOD - Ergodic 05/28/01 - set msec delay to look for next target
#define	CHAINLIGHTNING_DELAY 600
//PKMOD - Ergodic 07/03/01 - set infront value - Attacker is infront of target if higher than this value
#define CHAINLIGHTNING_REFLECT_INFRONT 0.50	

/*
=================
G_LightningDischargeDamage
PKMOD - Ergodic 01/15/01 - radius damage from lightning discarge into water
=================
*/
qboolean G_LightningDischargeDamage(vec3_t origin, gentity_t *ent, int damage) {
	gentity_t	*ext_ent;
	float		dist;
	vec3_t		dist_v;
	int			indx;

	int			localdamage;

	int			dischargeradius;
	qboolean	struckanotherplayer;

	dischargeradius = damage + 16;
	if ( dischargeradius > ( MAX_CHAINLIGHTNING_RANGE * 1.25 ) )
		dischargeradius = MAX_CHAINLIGHTNING_RANGE * 1.25; //cap the damage radius at a sensible range

	struckanotherplayer = qfalse;

	ext_ent = &g_entities[0]; //set the first player entity

	for ( indx = 0 ; indx < MAX_CLIENTS  ; indx++, ext_ent++) {
		
		if ( !ext_ent->inuse ) {
			continue;
		}

		VectorSubtract( ext_ent->r.currentOrigin, origin, dist_v );
		dist = VectorLength( dist_v );

		if ( dist > dischargeradius ) { //is the target within range
			continue;
		}

		if( !CanDamage (ext_ent, origin) ) { //If not visible then forget it
			continue;
		}

		//check target's game status
		if (( ext_ent->client->ps.pm_type == PM_DEAD ) || ( ext_ent->s.eFlags == EF_DEAD )) {
			continue;
		}
		//check target's health status
		if (( ext_ent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ext_ent->health <= 0 )) {
			continue;
		}


		//calculate the damage
		localdamage = damage * (1.0 - dist / dischargeradius);

 
		//PKMOD - Ergodic 03/22/02 - only increase accuracy if struck another player
		if ( ext_ent->client->ps.clientNum != ent->client->ps.clientNum )
			struckanotherplayer = qtrue;

		// push the center of mass higher than the origin so players
		// get knocked into the air more
		dist_v[2] += 32;

		G_Damage (ext_ent, NULL, ent, dist_v, origin, localdamage, DAMAGE_RADIUS, MOD_LIGHTNING_WATER_DISCHARGE);

	}

	return struckanotherplayer;
}


void ChainLightningStrike (gentity_t *prev, gentity_t *target);


//PKMOD - Ergodic 06/23/00 - determine whether you can shaft another entity
void  Find_ChainLightningTarget( gentity_t *ent ) {
	gentity_t	*ext_ent;
	float		dist;
	//PKMOD - Ergodic 12/29/00 - use new capacity definitions
	int			entityList[MAX_CHAINLIGHTNING_SEARCH];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e;
	//PKMOD - Ergodic 12/29/00 - use new capacity definitions
	float 		hold_distance = MAX_CHAINLIGHTNING_RANGE;
	gentity_t	*hold_target = NULL; 

	//PKMOD - Ergodic 12/29/00 - use new capacity definitions
	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = ent->target_ent->r.currentOrigin[i] - MAX_CHAINLIGHTNING_RANGE;
		maxs[i] = ent->target_ent->r.currentOrigin[i] + MAX_CHAINLIGHTNING_RANGE;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_CHAINLIGHTNING_SEARCH );
	//PKMOD - Ergodic 12/28/00 - debug inactive 
//	Com_Printf("Find_ChainLightningTarget - numListedEntities>%d<\n", numListedEntities );

	//PKMOD - Ergodic 06/23/00 - use a gross approximation of distance
	for ( e = 0 ; e < numListedEntities; e++ ) {

		ext_ent = &g_entities[entityList[ e ]];
	
		if( !CanDamage (ext_ent, ent->target_ent->r.currentOrigin) )  //is candidate target visible?
			continue;

		if ( !ext_ent->client ) //is candidate a client
			continue;

		if ( ext_ent == ent->parent ) //is candidate the shaft originator
			continue;

		if ( ( ext_ent->client->ps.stats[STAT_PKA_ITEMS] & ( 1 << PKA_SHAFTED ) )  ) //is candidate currently being shafted
			continue;

		//Found a candidate!
		//Locate the closest candidate
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( ent->target_ent->r.currentOrigin[i] < ext_ent->r.absmin[i] ) {
				v[i] = ext_ent->r.absmin[i] - ent->target_ent->r.currentOrigin[i];
			} else if ( ent->target_ent->r.currentOrigin[i] > ext_ent->r.absmax[i] ) {
				v[i] = ent->target_ent->r.currentOrigin[i] - ext_ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );

		if ( dist < hold_distance ) {
			hold_distance = dist;
			hold_target = ext_ent;
		}

	}
	if ( hold_target != NULL ) {
		//PKMOD - Ergodic debug 06/27/00 - inactive
//		Com_Printf("Find_ChainLightningTarget - target found >%s<\n", hold_target->client->pers.netname);
		ChainLightningStrike (ent, hold_target);
	}
	else {
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("Find_ChainLightningTarget - can not find a target\n");
		i = 1; //nop
	}
}


void ChainLightningThink( gentity_t *ent ) {
	gentity_t	*hold_link;
	gentity_t	*hold_source;
	gentity_t	*hold_prev;
	vec3_t		end;
	int			hold_damage;
	vec3_t		hold_vec;
	vec3_t		hold_forward;
	float		dot;


	//PKMOD - Ergodic debug 05/12/02 - debug 000 location for shaft (inactive)
//	Com_Printf("ChainLightningThink\n");

	//PKMOD - Ergodic 09/09/00 - add logic for missing the first target
	//has shaft player missed target
	//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
	if ( ! ( ent->parent->client->ps.eFlags & EF_LIGHTNINGSTRIKE ) )  {
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_SHAFTED );
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 03/10/01 - add logic for weapon switch
	if ( ent->parent->s.weapon != WP_LIGHTNING )  {
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_SHAFTED );
		G_FreeEntity( ent );
		return;
	}

	//currently tracking a target need to validate
	//has shaft player owner disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningThink - disconnected parent remove\n");
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_SHAFTED );
		G_FreeEntity( ent );
		return;
	}

	//has shaft player owner died
	if (( ent->parent->client->ps.pm_type == PM_DEAD ) || ( ent->parent->s.eFlags == EF_DEAD )) {
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningThink - dead parent remove\n");
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_SHAFTED );
		G_FreeEntity( ent );
		return;
	}

	if ( ent->prevTrain == NULL ) { //first shaft entity
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningThink - first shaft\n");
		hold_source = ent->parent;
	}
	else {
		hold_source = ent->prevTrain->target_ent;
	}
	
	if( (!CanDamage (hold_source, ent->target_ent->r.currentOrigin) || (!( ent->parent->s.eFlags & EF_FIRING )) ) )  { 
		//is candidate target visible or owner stopped firing?
		//Can't damage the target so remove the entity chain
		//travel down the lightning entity chain so to remove linked entities
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningThink - !CanDamage or stopped firing\n");

		while ( ent != NULL ) {
			if ( ent->prevTrain != NULL ) {
				ent->prevTrain->nextTrain = NULL;
			}
			//remove the shaft flag
			ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_SHAFTED );
			hold_link = ent->nextTrain;
			G_FreeEntity( ent );
			//PKMOD - Ergodic debug 06/24/00 - inactive
//			Com_Printf("ChainLightningThink - removing trains\n");
			ent = hold_link;
		}
		return;
	}

	// set aiming directions from prev to target
	if ( ent->prevTrain == NULL ) { 
		//at the owner
		hold_prev = ent->parent;
	}
	else {
		//not the the owner
		hold_prev = ent->prevTrain->target_ent;
	}

	//PKMOD - Ergodic 07/12/00 New clg coding 
	G_SetOrigin( ent, hold_source->r.currentOrigin );
	//co-opt the angles variable to hold target location
	VectorCopy( ent->target_ent->r.currentOrigin, ent->s.angles );

	//PKMOD - Ergodic debug 06/24/00 - inactive
//	Com_Printf("ChainLightningThink - applying damage\n");

//	VectorSubtract(ent->target_ent->r.currentOrigin, hold_prev->r.currentOrigin, hold_vec);
	
//	AngleVectors (hold_prev->client->ps.viewangles, forward, right, up);

//	CalcMuzzlePoint ( hold_prev, forward, right, up, muzzle );
//	VectorMA (muzzle, 32, forward, end);

	//create zap beam
//	zap_dist = VectorLength( v );
//	VectorMA( muzzle, zap_dist, hold_vec, end );


//	trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

//	if ( tr.entityNum == ENTITYNUM_NONE ) {
//		return;
//	}

//	traceEnt = &g_entities[ tr.entityNum ];

	//PKMOD Ergodic 07/01/00 send message to cg to creat the shaft beam
	if ( ent->prevTrain != NULL ) //if not the first shaft from the parent
		//PKMOD - Ergodic debug 07/12/00 inactive
//		Com_Printf("ChainLightningThink - adding a new EV_CHAIN_SHAFT event\n");

//		G_AddEvent( ent, EV_CHAIN_SHAFT, 0 );

	//do we need this following "tent" stuff?
//	tent = G_TempEntity( end, EV_MISSILE_HIT );
//	tent->s.otherEntityNum = ent->target_ent->s.number;
//	tent->s.eventParm = DirToByte( tr.plane.normal );
//	tent->s.weapon = ent->parent->s.weapon;
	if( LogAccuracyHit( ent->target_ent, ent->parent ) ) {
		ent->client->accuracy_hits++;
	}



	//PKMOD - Ergodic 07/02/01 - reverse damage if target is holding a sentry 
	//			Note: shield effect only works for the prime target, chain shafted
	//				players will not have any protection
	if ( ( ent->prevTrain == NULL ) && ( ent->target_ent->s.weapon == WP_SENTRY ) ) { //if first target 

		//PKMOD - Ergodic 07/03/01 - calculate if target is facing attacker
		//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors (ent->target_ent->client->ps.viewangles, hold_forward, NULL, NULL);
		AngleVectorsForward( ent->target_ent->client->ps.viewangles, hold_forward );
		VectorSubtract( ent->parent->r.currentOrigin, ent->target_ent->r.currentOrigin, hold_vec) ;

		VectorNormalize ( hold_vec );

		dot = DotProduct (hold_vec, hold_forward);
		//PKMOD - Ergodic debug 07/03/01 - debug infrontedness (inactive)
//		Com_Printf("ChainLightningThink - dot * 100>%f<\n", dot * 100);

	
		if ( dot > CHAINLIGHTNING_REFLECT_INFRONT ) {

			//reverse the damage vector
			VectorScale( forward, -1, hold_vec ); 
			//give owner 80% damage
			//PKMOD - Ergodic 12/16/02 - increase reflectivity to 90%
			//hold_damage = 8 * ent->damage  / 10 - 1 + ( rand() % 3 ); // damage: (11, 12, 13)
			hold_damage = 9 * ent->damage  / 10 - 1 + ( rand() % 3 ); // damage: (13, 14, 15)
			G_Damage( ent->parent, ent->parent, ent->parent, hold_vec, end, hold_damage, 0, MOD_REVERSE_LIGHTNING);
			//give target 20 % damage
			//PKMOD - Ergodic 12/16/02 - decrease target reflectivity to 10%
			//hold_damage = 2 * ent->damage  / 10 - 1 + ( rand() % 3 ); // damage: (2, 3, 4)
			hold_damage = ent->damage  / 10 - 1 + ( rand() % 3 ); // damage: (0, 1, 2)
			G_Damage( ent->target_ent, ent->parent, ent->parent, hold_vec, end, hold_damage, 0, MOD_LIGHTNING);

			//PKMOD - Ergodic 07/03/01 ChainLightning reflect sounds
			G_AddEvent( ent, EV_CHAINLIGHTNING_REFLECT, 0 );
		}
		else {
			//NOT infront - give target normal damage 
			G_Damage( ent->target_ent, ent->parent, ent->parent, forward, end, ent->damage, 0, MOD_LIGHTNING);
		}
	}
	else {
		//give target damage
		G_Damage( ent->target_ent, ent->parent, ent->parent, forward, end, ent->damage, 0, MOD_LIGHTNING);
	}

	//PKMOD - Ergodic debug 06/27/00 - inactive
//	Com_Printf("ChainLightningThink - after applying damage\n");

	//look for new target if at end of chain
	if ( ent->nextTrain == NULL ) {
		//currently not tracking a target
	//PKMOD - Ergodic debug 06/27/00 - inactive
//	Com_Printf("ChainLightningThink - attempting to find next target\n");
		//PKMOD - Ergodic 05/28/01 - only find next target if time is greater than delay period
		if ( ent->wait < level.time ) 
			Find_ChainLightningTarget( ent );
	}

	ent->nextthink = level.time + 150;	//12/29/00 was 100
	ent->eventTime = level.time;
	trap_LinkEntity( ent );
}

void ChainLightningStrike (gentity_t *prev, gentity_t *target) {
	gentity_t	*bolt;
	gentity_t	*hold_source;

//	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "chainlightning";

/*
	if ( ent->prevTrain == NULL ) { //first shaft entity
		hold_source = ent->parent;
	}
	else {
		hold_source = ent->prevTrain->target_ent;
	}
*/

//PKMOD - Ergodic debug 09/09/00 inactive
//	Com_Printf("ChainLightningStrike - client strike: count >%d<, target >%s<\n", bolt->count, target->client->pers.netname );

	if (prev->client) {
		bolt->s.eType = ET_GENERAL; //07/12/00
		bolt->r.ownerNum = prev->s.number;
		bolt->parent = prev;
		bolt->prevTrain = NULL;
		//PKMOD - Ergodic 04/21/01 - set lightning damage (was 8)
		bolt->damage = CHAINLIGHTNING_DAMAGE * s_quadFactor;
		bolt->count = 1;
		//PKMOD - Ergodic 05/13/02 - Bug Fix - set the source entity
		hold_source = prev;
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningStrike - client strike: count >%d<, target >%s<\n", bolt->count, target->client->pers.netname );
	}
	else {
		bolt->s.eType = ET_CHAIN_LIGHTNING; //07/12/00
		bolt->r.ownerNum = prev->r.ownerNum;
		bolt->parent = prev->parent;
		bolt->prevTrain = prev;
		bolt->damage = bolt->prevTrain->damage;
		prev->nextTrain = bolt;
		bolt->count = prev->count + 1;
		//PKMOD - Ergodic 05/13/02 - Bug Fix - set the source entity
		hold_source = prev->target_ent;
		//PKMOD - Ergodic debug 06/24/00 - inactive
//		Com_Printf("ChainLightningStrike - new entity strike: count >%d<, target >%s<\n", bolt->count, target->client->pers.netname );
	}

	bolt->nextTrain = NULL;

	//PKMOD Ergodic 06/23/00 - set the target associated with the chainlightning entity
	bolt->target_ent = target;

	//PKMOD Ergodic 06/23/00 - set the target's shafted flag
	bolt->target_ent->client->ps.stats[STAT_PKA_ITEMS] |= ( 1 << PKA_SHAFTED );

	//PKMOD - Ergodic 05/13/02 - Bug Fix - set the entity origin to the preceding shaft player
	G_SetOrigin( bolt, hold_source->r.currentOrigin );

	//PKMOD Ergodic 08/22/00 - set the bolt's location and sound event
	VectorCopy( bolt->target_ent->r.currentOrigin, bolt->s.angles );
	//PKMOD - Ergodic 05/12/02 - remove the following event call 
//	G_AddEvent( bolt, EV_CHAINLIGHTNING_STRIKE, 0 );

	//PKMOD Ergodic 05/28/01 - set a delay time to look for next target
	//			this functionality was suggest by player "ShitHead"
	bolt->wait = level.time + CHAINLIGHTNING_DELAY;
	
	bolt->think = ChainLightningThink;
	bolt->nextthink = level.time + 150;	//12/29/00 was 100
	bolt->eventTime = level.time;

	trap_LinkEntity( bolt );

}

void Weapon_LightningFire( gentity_t *ent ) {
	trace_t		tr;
	//PKMOD - Ergodic 07/27/01 - ass short muzzle definition
	//			01/05/04 0 - removed short_muzzle (not needed)
	//vec3_t		short_muzzle, end;
	vec3_t		end;
	gentity_t	*traceEnt, *tent;
//	int			damage;
	vec3_t		enemydirection_uv;
//	vec3_t		enemydirection_angles;
//	vec3_t		enemydirection_forward;
	float		lightning_aim;
//	int			hold_aim;
	//PKMOD - Ergodic 12/29/00 - debug clg_entity counts
//	gentity_t	*clg_ent;
//	int			clg_count = 0;

	//PKMOD - Ergodic 12/29/00 - debug clg_entity counts
//	while ((clg_ent = G_Find (clg_ent, FOFS(classname), "chainlightning")) != NULL) {
//		clg_count++;
//	}
	//PKMOD - Ergodic 12/29/00 - debug clg_entity counts
//	Com_Printf("Weapon_LightningFire - clg entities>%d<\n", clg_count );


//	damage = 8 * s_quadFactor;

	//PKMOD - Ergodic 01/15/01 - add Lightning Discharge (little help from code3arena:SARACEN)
	if (trap_PointContents (muzzle, -1) & MASK_WATER)
	{
		int zaps;
		gentity_t *tent;

		zaps = ent->client->ps.ammo[WP_LIGHTNING];	// determines size/power of discharge
		if ( zaps <= 0)
			return;	// prevents any subsequent frames causing second discharge + error

		SnapVectorTowards (muzzle, ent->s.origin);	// save bandwidth
		tent = G_TempEntity (muzzle, EV_LIGHTNING_WATER_DISCHARGE);
		tent->s.eventParm = zaps;				// duration / size of explosion graphic

		ent->client->ps.ammo[WP_LIGHTNING] = 0;		// drain ent's lightning count
		//PKMOD - Ergodic 04/21/01 - set lightning damage (was 8)
		if ( G_LightningDischargeDamage(muzzle, ent, CHAINLIGHTNING_DAMAGE * s_quadFactor * zaps ) )
			//PKMOD - Ergodic 03/22/02 - this called suddenly caused cvar errors in pka2.9x 
			//		changed base address from g_entities[ent->r.ownerNum] to ent
//			g_entities[ent->r.ownerNum].client->accuracy_hits++;
			ent->client->accuracy_hits++;
		
		return;
	}
// The SARACEN's Lightning Discharge - END

	//PKMOD - Ergodic 09/09/00 - add target lock-on functionality
	//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
	//PKMOD - Ergodic - 12/29/2000, modify lock-on code
	if ( ent->client->ps.eFlags & EF_LIGHTNINGSTRIKE ) {
		VectorSubtract(ent->Lightning_Strike_Enemy->r.currentOrigin, ent->r.currentOrigin, enemydirection_uv);
		VectorNormalizeFast(enemydirection_uv);
//		vectoangles ( enemydirection_uv, enemydirection_angles );
//		AngleVectors ( enemydirection_angles, enemydirection_forward, NULL, NULL );

		//assume that forward has not changed
		lightning_aim = DotProduct( enemydirection_uv, forward );
/* 12/15/03 - remove this section of code
//		hold_aim = lightning_aim * 100;
		//PKMOD - Ergodic debug 09/09/00
//		Com_Printf("Weapon_LightningFire - forward >%s<, ed_uv>%s<, aim>%d<\n",  vtos(forward), vtos(enemydirection_uv), hold_aim );
		if ( lightning_aim > 0.90 ) {
			//PKMOD - Ergodic 09/10/00 - set the aim, co-opt field: apos.trDelta
			//PKMOD - Ergodic 07/08/01 - remove the following line
//			VectorCopy( enemydirection_uv, ent->s.apos.trDelta );
//			trap_Trace( &tr, muzzle, NULL, NULL, ent->Lightning_Strike_Enemy->r.currentOrigin, ent->s.number, MASK_SHOT );
			//PKMOD - Ergodic 07/08/01 - create a temp entity for the shaft
			//			only display shaft via TempEntity if striking first player
 			tent = G_TempEntity( muzzle, EV_CHAINLIGHTNING_DISPLAY_STRIKE );
			VectorCopy( ent->Lightning_Strike_Enemy->r.currentOrigin, end );
			//PKMOD - Ergodic - 07/27/01 - add height to ending loctaion vector
			end[2] +=16;
			SnapVector( end );
			VectorCopy( end, tent->s.origin2 );
			//PKMOD - Ergodic - 12/15/03 - add the owner's entity number to produce a 
			//			TRUE shaft beam that eminates from the CLG Tag
			tent->s.otherEntityNum = ent->s.number;
		} 
		else {
*/
		if ( lightning_aim <= 0.90 ) {

			//turn off the juice
			//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
			ent->client->ps.eFlags &= ~EF_LIGHTNINGSTRIKE;
			//PKMOD - Ergodic 09/10/00 - set the aim, co-opt field: apos.trDelta
//			VectorCopy( ent->client->ps.viewangles, ent->s.apos.trDelta );
			VectorCopy( forward, ent->s.apos.trDelta );
		}
		return;
	}

	//PKMOD - Ergodic 12/29/00 - set MAX chainlightning distance to 512 (instead of 768)
	VectorMA( muzzle, MAX_CHAINLIGHTNING_RANGE, forward, end );


	//PKMOD - Ergodic 01/30/01 - send a trace to detect an autosentry
//	trap_Trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	trap_Trace( &tr, muzzle,  NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );


	if ( tr.entityNum == ENTITYNUM_NONE ) {
			return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	//PKMOD - Ergodic 09/10/00 - set the aim, co-opt field: apos.trDelta
//	VectorCopy( ent->client->ps.viewangles, ent->s.apos.trDelta );
	VectorCopy( forward, ent->s.apos.trDelta );

	//PKMOD - Ergodic 09/30/03 - debug event (inactive)
	//	Com_Printf("Weapon_LightningFire - Striking >%d<\n", traceEnt->s.eType );

	//PKMOD - Ergodic 09/30/03 - add case of Lightning striking BT to make it invisible
	if ( traceEnt->s.eType == ET_BEARTRAP ) {
		//PKMOD - Ergodic - 12/28/2000, not striking a player
		ent->target_ent->client->ps.eFlags &= ~EF_LIGHTNINGSTRIKE;

		//PKMOD - Ergodic 10/16/03 - Only owner can charge the Beartrap
		if ( traceEnt->r.ownerNum == ent->s.number ) {
			//PKMOD - Ergodic 10/10/03 - add in the strike info
			traceEnt->s.generic1++;

			if ( traceEnt->s.generic1 > MAX_INVISIBILITY_CHARGE ) {
				traceEnt->s.generic1 = MAX_INVISIBILITY_CHARGE;
			}
			else {
					//PKMOD - Ergodic 09/30/03 - debug event (inactive) 
					//			Com_Printf("Weapon_LightningFire - Detected Strike on Beartrap, damage units>%d<\n", traceEnt->s.generic1 );
					//PKMOD - Ergodic 12/06/03 - play the charge up sound	
					//PKMOD - Ergodic 12/08/03 - change to a temp entity to prevent sound collisions
					//G_AddEvent( ent, EV_CHAINLIGHTNING_CHARGE_UP, 0 );
					tent = G_TempEntity( tr.endpos, EV_CHAINLIGHTNING_CHARGE_UP );
			}
		}

	}

	//PKMOD - Ergodic 12/13/03 - add case of Lightning striking Autosentry to make it invisible
	else if ( traceEnt->s.eType == ET_AUTOSENTRY_TURRET ) {
		//PKMOD - Ergodic - 12/28/2000, not striking a player
		ent->target_ent->client->ps.eFlags &= ~EF_LIGHTNINGSTRIKE;

		//PKMOD - Ergodic 10/16/03 - Only owner can charge the Beartrap
		if ( traceEnt->r.ownerNum == ent->s.number ) {
			//PKMOD - Ergodic 10/10/03 - add in the strike info
			traceEnt->s.generic1++;

			if ( traceEnt->s.generic1 > MAX_INVISIBILITY_CHARGE ) {
				traceEnt->s.generic1 = MAX_INVISIBILITY_CHARGE;
			}
			else {
					//PKMOD - Ergodic 12/13/03 - debug event (inactive) 
					//Com_Printf("Weapon_LightningFire - Detected Strike on Autosentry, damage units>%d<\n", traceEnt->s.generic1 );
					//PKMOD - Ergodic 12/06/03 - play the charge up sound	
					//PKMOD - Ergodic 12/08/03 - change to a temp entity to prevent sound collisions
					//G_AddEvent( ent, EV_CHAINLIGHTNING_CHARGE_UP, 0 );
					tent = G_TempEntity( tr.endpos, EV_CHAINLIGHTNING_CHARGE_UP );
			}
			//PKMOD - Ergodic 12/13/03 - set the autosentry base to be the same invisibility
			//				value as the turret
			traceEnt->prevTrain->s.generic1 = traceEnt->s.generic1;
		}
	}



	else if ( traceEnt->takedamage && traceEnt->client ) {
		//PKMOD - Ergodic 09/20/00 - debug: set the logic to false
//		if ( ! ent->Lightning_Strike ) {
			ChainLightningStrike (ent, traceEnt);
			//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
			ent->client->ps.eFlags |= EF_LIGHTNINGSTRIKE;
			ent->Lightning_Strike_Enemy = traceEnt;

			//PKMOD - Ergodic - 12/15/03, Save the targeted entity number for use in
			//			CG_Weapons.
			//PKMOD - Ergodic - 12/16/03 Use stat location to store shaftee entity number
			ent->client->ps.stats[STAT_CLG_SHAFTEE_NUM] = traceEnt->s.number;
			//PKMOD - Ergodic 12/15/03 - debug otherEntityNum2 (inactive)
			//Com_Printf("Weapon_LightningFire - striking clientNum >%d<\n", traceEnt->s.number );

/* 12/15/03 - remove this section of code
			//PKMOD - Ergodic 07/27/01 - calculate short muzzle point location
			VectorCopy( ent->s.pos.trBase, short_muzzle );
			short_muzzle[2] += ent->client->ps.viewheight;
			VectorMA( short_muzzle, 3, forward, short_muzzle );
			// snap to integer coordinates for more efficient network bandwidth usage
			SnapVector( short_muzzle );

			//PKMOD - Ergodic 07/08/01 - create a temp entity for the shaft
			//			only display shaft via TempEntity if striking first player
 			tent = G_TempEntity( short_muzzle, EV_CHAINLIGHTNING_DISPLAY_STRIKE );

			VectorCopy( traceEnt->r.currentOrigin, end );
			SnapVector( end );
			VectorCopy( end, tent->s.origin2 );
			//PKMOD - Ergodic - 12/15/03 - add the owner's entity number to produce a 
			//			TRUE shaft beam that eminates from the CLG Tag
			tent->s.otherEntityNum = ent->s.number;
*/

//		}
//		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
//		tent->s.otherEntityNum = traceEnt->s.number;
//		tent->s.eventParm = DirToByte( tr.plane.normal );
//		tent->s.weapon = ent->s.weapon;
//		if( LogAccuracyHit( traceEnt, ent ) ) {
//			ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
//		}
	} else if ( !( tr.surfaceFlags & SURF_NOIMPACT ) ) {
		//PKMOD - Ergodic - 12/28/2000, created new structure for PKA flags 
		ent->target_ent->client->ps.eFlags &= ~EF_LIGHTNINGSTRIKE;
		tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}

	//PKMOD - Ergodic 09/09/00 - remove the following code which may do double hit damage 
//	if ( traceEnt->takedamage) {
//		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
//			damage, 0, MOD_LIGHTNING);
//	}

}


//PKMOD - Ergodic 12/05/00 - add new types of PKA shooters
void shooter_Lightning_fire( gentity_t *ent, vec3_t start, vec3_t end ) {
	gentity_t	*spark;
	trace_t		tr;
	gentity_t	*traceEnt;
	int			damage;

	spark = G_Spawn();

	//PKMOD Ergodic 08/20/03 - Force shooter lightning's beams to be green
	//		The new CLG will have a red beam
	//spark->s.eType = ET_CHAIN_LIGHTNING; //07/12/00
	spark->s.eType = ET_SHOOTER_LIGHTNING;
	G_SetOrigin( spark, start );


	//PKMOD Ergodic 08/22/00 - set the spark's location and sound event
	trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

	VectorCopy( end, spark->s.angles );
//	VectorCopy( tr.endpos, spark->s.angles );
	G_AddEvent( spark, EV_CHAINLIGHTNING_STRIKE, 0 );


	spark->think = G_FreeEntity;
	spark->nextthink = level.time + 1000 + ( rand() % 500 );

	trap_LinkEntity( spark );




	//PKMOD Ergodic 12/06/00 - debug inactive
//	Com_Printf("shooter_Lightning_fire - start>%s<, end>%s<\n", vtos( start ), vtos( end ) );



//	VectorMA( muzzle, LIGHTNING_RANGE, forward, end );


//	if ( tr.entityNum == ENTITYNUM_NONE ) {
//		return;
//	}

	traceEnt = &g_entities[ tr.entityNum ];

//	tent = G_TempEntity( tr.endpos, EV_SHOOTER_LIGHTNING );
//	tent->s.otherEntityNum = traceEnt->s.number;
//	tent->s.eventParm = DirToByte( tr.plane.normal );

//	if ( traceEnt->takedamage && traceEnt->client ) {
//		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
//		tent->s.otherEntityNum = traceEnt->s.number;
//		tent->s.eventParm = DirToByte( tr.plane.normal );
//		tent->s.weapon = ent->s.weapon;
//	}
//	else if ( !( tr.surfaceFlags & SURF_NOIMPACT ) ) {
//		tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
//		tent->s.eventParm = DirToByte( tr.plane.normal );
//	}

	damage = 10 + (rand() % 6);
	if ( traceEnt->takedamage) {
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, 0, MOD_SHOOTER_LIGHTNING);
	}
}

//======================================================================

/*
======================================================================

EXPLODING SHELLS
PKMOD - Ergodic 06/14/00

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
//#define	DEFAULT_SHOTGUN_DAMAGE	10

//qboolean Exploding_ShellsPellet( vec3_t start, vec3_t end, gentity_t *ent, float r, float u ) {
qboolean Exploding_ShellsPellet( vec3_t start, vec3_t end, gentity_t *ent ) {
	trace_t		tr;
	int			damage, passent;
	gentity_t		*traceEnt;
	qboolean		hitClient = qfalse;
	//PKMOD - Ergodic 01/19/01 - Debug variables (exploding shells) 
//	float		debug_fraction;	// time completed, 1.0 = didn't hit anything
//	int			debug_surfaceFlags;	// surface hit
//	int			debug_contents;	// contents on other side of surface hit
//	int			debug_entityNum;	// entity the contacted sirface is a part of
	
	//PKMOD - Ergodic 01/30/01 - send a trace to detect an autosentry
	trap_Trace( &tr, muzzle,  NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	//PKMOD - Ergodic 01/30/01 - if NOT detect an autosentry with this specific comparison...
	if ( ( traceEnt->s.eType > ET_AUTOSENTRY_TURRET ) || ( traceEnt->s.eType < ET_AUTOSENTRY_LAUNCH ) ) {
		//We did not hit an autosentry so send trace to detect a passent entity
		passent = ent->s.number;
		trap_Trace (&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT );
		traceEnt = &g_entities[ tr.entityNum ];
	}

	//PKMOD - Ergodic 01/19/01 - Debug variables (exploding shells) 
//	debug_fraction = tr.fraction;
//	debug_surfaceFlags = tr.surfaceFlags;
//	debug_contents = tr.contents;
//	debug_entityNum = tr.entityNum;
	
	// send bullet impact
	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	if ( traceEnt->takedamage) {
		//PKMOD - Ergodic 01/18/01 - update damage 12 - 18 per pellet
		//PKMOD - Ergodic 04/21/01 - weaken damage 8 - 14 per pellet
		damage = ( DEFAULT_SHOTGUN_DAMAGE - 2 + ( rand() % 7) ) * s_quadFactor;
	
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, 0, MOD_EXPLODING_SHELLS);
		if( LogAccuracyHit( traceEnt, ent ) ) {
			hitClient = qtrue;
		}
	}
	else { 	//PKMOD - Ergodic 01/18/01 - if traceEnt is not damageable then NULL it out
		traceEnt = NULL;
	}

	//PKMOD - Ergodic 01/18/01 - update splashdamage
	//			make spash damage 3/4 strength of damage from direct hit
//	damage = 3 * damage / 4 = "11";
	//PKMOD - Ergodic 04/21/01 - weaken radius damage ~1/2 direct damage
	damage = 6;

	//PKMOD - Ergodic 08/16/00 - Fix radiusdamage to not (double) strike the target 
	//			08/26/00 - Note: double strike fix was in g_combat.c
	//PKMOD - Ergodic 01/18/01 - update splashradius from 20 to 80
	//				comparison: rl splashradius = 120
	//				comparison: gl splashradius = 150
	//				comparison: pg splashradius = 20

	if( G_RadiusDamage( tr.endpos, ent, damage, 80, traceEnt, MOD_EXPLODING_SHELLS_SPLASH ) ) {
		hitClient = qtrue;
		//PKMOD - Ergodic 01/19/01 - Debug (exploding shells) inactive 
//		Com_Printf( "1 fr:%f, fl:%d, c:%d, en:%d}, \n", debug_fraction * 100, debug_surfaceFlags, debug_contents, debug_entityNum );
	}
//	else {
//		//PKMOD - Ergodic 01/19/01 - Debug (exploding shells) inactive
//		Com_Printf( "0 fr:%f, fl:%d, c:%d, en:%d}, \n", debug_fraction * 100, debug_surfaceFlags, debug_contents, debug_entityNum );
//	}

	return hitClient;
}

// this should match CG_ShotgunPattern
void Exploding_ShellsPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;
	int			oldScore;
	qboolean	hitClient = qfalse;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	oldScore = ent->client->ps.persistant[PERS_SCORE];

	// generate the "random" spread pattern
	//PKMOD - Ergodic 08/26/00 reduce shell pellets by 2 increase spread by 20%
	//PKMOD - Ergodic 01/16/01 change spread multiplier to 8 to match what is in cg_weapons
	//PKMOD - Ergodic 01/18/01 change spread multiplier to 12 to match what is in cg_weapons
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 12; //was 120%
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 12; //was 120%
		//PKMOD - Ergodic 01/19/01 - Debug (exploding shells) inactive
//		Com_Printf("{ %f, %f,", r * 100, u * 100 );
		//PKMOD - Ergodic 01/22/01 - add in distance multiplier of 12
		VectorMA( origin, 8192 * 12, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
//		if( Exploding_ShellsPellet( origin, end, ent, r, u ) && !hitClient ) {
		if( Exploding_ShellsPellet( origin, end, ent ) && !hitClient ) {
			hitClient = qtrue;
			ent->client->accuracy_hits++;
		}
	}
}

void weapon_exploding_shells_fire (gentity_t *ent) {
	gentity_t		*tent;
	//PKMOD - Ergodic 01/20/01 - debug entity and temp vector
//	gentity_t		*debug_tent;
//	vec3_t		hold_muzzle;

	//PKMOD - Ergodic 01/21/01 - nudge the origin with the cvar value's displacement
//	VectorCopy( muzzle, hold_muzzle );
//	VectorMA( hold_muzzle, x_explmuzzle.integer, right, hold_muzzle );
//	VectorMA( hold_muzzle, y_explmuzzle.integer, up, hold_muzzle );
//	VectorMA( hold_muzzle, z_explmuzzle.integer, forward, hold_muzzle );

	//PKMOD - Ergodic 01/20/01 - exploding shells muzzle displacement
//			debug usage only
//PKMOD Ergodic 12/06/00 - debug
//	Com_Printf( "weapon_exploding_shells_fire - displacements>%d, %d, %d<, hold_muzzle>%s<\n", x_explmuzzle.integer, y_explmuzzle.integer, z_explmuzzle.integer, vtos( hold_muzzle )  );

	// send coord 
//	debug_tent = G_TempEntity( hold_muzzle, EV_COORD );
//	debug_tent->s.weapon = ent->s.weapon;
//	debug_tent->s.otherEntityNum = ent->s.number;

	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	tent->s.weapon = ent->s.weapon;
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;

	Exploding_ShellsPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
}

/*
======================================================================

BEANS
PKMOD - Ergodic 06/14/00
		Ergodic 10/21/00 - add toots
		Ergodic 12/25/00 reactivate toots, and add some meat this time

======================================================================
*/
#define	TOOT_VELOCITY 220		//JUMP_VELOCITY	270
#define	TOOT_DURATION 18000		//18 seconds
//#define TOOT_QUADFART_CHANCE 13 //13 percent chance of quad farting
#define TOOT_QUADFART_CHANCE 23 //update 01/07/04 - 23 percent chance of quad farting

void TootThink (gentity_t *ent) {
	vec3_t		hold_vec;

	int			splashdamage;
	int			splashradius;
	qboolean	dummy;
	//PKMOD - Ergodic 01/05/04 - add quad farting logic
	int			damagequadfart = 0;

	//remove toot if player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		G_FreeEntity( ent );
		return;
	}

	//remove toot if player has died
	if (( ent->parent->client->ps.pm_type == PM_DEAD ) || ( ent->parent->s.eFlags == EF_DEAD )) {
		G_FreeEntity( ent );
		return;
	}

	//remove toot if player's health drops to, or below, zero 
	if (( ent->parent->client->ps.stats[STAT_HEALTH] <= 0 ) || ( ent->parent->health <= 0 ) ) {
		G_FreeEntity( ent );
		return;
	}

	//remove too if beans flag not active
	if ( ! ( ent->parent->client->ps.stats[STAT_PKA_ITEMS] & ( 1 << PKA_BEANS ) )) {
		G_FreeEntity( ent );
		return;
	}

	//else give a hoot and do a toot!
	//Ummm, how do i do a toot?

	//PKMOD - Ergodic 01/05/04 - add quad farting logic...
	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		if ( ent->pka_flags & PKAEF_QUADFART ) {
			if ( (rand() % 100) < TOOT_QUADFART_CHANCE ) {
				damagequadfart = 1;
			}

		}
	}

	//calculate the vertical pulse velocity
	//PKMOD - Ergodic 01/05/04 - add quad farting logic...
	if ( damagequadfart ) {
		//PKMOD - Ergodic 01/07/04 - don't jump so high with quad farting because
		//		damage will make player jump too
		VectorSet (hold_vec, 0,0, (int)( rand() % 50 ) ); 	//jump velocity = 220 + (0-200)
	} else {
		VectorSet (hold_vec, 0,0, (int)(TOOT_VELOCITY + rand() % 200) ); 	//jump velocity = 220 + (0-200)
	}

	VectorAdd(hold_vec,ent->parent->client->ps.velocity, ent->parent->client->ps.velocity);
	//PKMOD - Ergodic 12/26/00 - debug inactive
//	Com_Printf( "TootThink - toot velocity >%s<\n", vtos(hold_vec) );

	//PKMOD - Ergodic 02/01/01 - add a radius damage to the bean toot
	//PKMOD - Ergodic 02/01/01 - cause fecal explosion damage
	//PKMOD - Ergodic 02/01/01 - make beans too quad-able
	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		if ( damagequadfart ) {
			splashdamage = 250 + rand() % 15;
			splashradius = 350 + rand() % 30;
		} else {
			//set default quad damage
			splashdamage = 45 + rand() % 15;
			splashradius = 150 + rand() % 30;
		}
	}
	else {
		splashdamage = 20 + rand() % 10;
		splashradius = 120 + rand() % 30;
	}

	//PKMOD - Ergodic 02/01/01 -  don't damage parent
	//PKMOD - Ergodic 01/05/04 - add quad farting logic...
	//			Quad farting will damage parent
	if ( damagequadfart ) {
		gentity_t	*tent;

		//PKMOD - Ergodic 01/07/04 - debug quadfarts (inactive)
		//Com_Printf("TootThink - attempting quadfarts\n" );
		//PKMOD - Ergodic 01/07/04 - use a temp entity for event instead of player event
		//				so event will not get overwritten bye hurt event from G_RadiusDamage
		tent = G_TempEntity( ent->parent->r.currentOrigin, EV_QUADBEANS_TOOT );
		//G_AddEvent( ent->parent, EV_QUADBEANS_TOOT, 0 );
		dummy = G_RadiusDamage( ent->parent->r.currentOrigin, ent->parent, splashdamage, splashradius, NULL, MOD_QUADBEANS_BLAST ); 
	}
	else { 
		G_AddEvent( ent->parent, EV_BEANS_TOOT, 0 );
		dummy = G_RadiusDamage( ent->parent->r.currentOrigin, ent->parent, splashdamage, splashradius, ent->parent, MOD_BEANS_BLAST ); 
	}

	if ( ent->wait > level.time ) {
		ent->eventTime = level.time;
		ent->nextthink = level.time + 1500 + rand() % 3000;
		trap_LinkEntity( ent );
	}
	else {
		//PKMOD - Ergodic 12/26/00 - debug inactive
//		Com_Printf( "TootThink - stopping toots\n" );
		G_FreeEntity( ent );
	}
}


void weapon_beans_eat( gentity_t *ent ) {
	int			max;
	gentity_t	*toot;
	int			toot_time;

	max = ent->client->ps.stats[STAT_MAX_HEALTH] * 2;

	ent->health += 100;

	if (ent->health > max ) {
		ent->health = max;
	}
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//set the time
	toot_time = TOOT_DURATION + 8000 * random(); //Follow for 18 seconds + 8 random seconds

	//PKMOD - Ergodic 06/18/00 set the beans status flag
	ent->client->ps.stats[STAT_PKA_ITEMS] |= ( 1 << PKA_BEANS );

	//PKMOD - Ergodic 06/03/01 - add bean powerup timer for armor countdown immunity
	// round timing to seconds to make multiple powerup timers
	// count in sync
	ent->client->ps.powerups[PW_BEANS] = toot_time + level.time - ( level.time % 1000 );

	//PKMOD - Ergodic 10/21/00 add toots
	//PKMOD - Ergodic 12/25/00 reactivate toots

	toot = G_Spawn();
	toot->classname = "beantoots";
	toot->s.eType = ET_GENERAL; //07/12/00
	toot->r.ownerNum = ent->s.number;
	toot->parent = ent;
	toot->think = TootThink;
	toot->wait = level.time + toot_time; //Follow for 12 seconds + 8 random seconds
	toot->nextthink = level.time + 1000 + rand() % 1000;
	toot->eventTime = level.time;

	//PKMOD - Ergodic 01/05/04 - add quad farting...
	//			quad farting will occur when player is already quaded and then Eats
	//			a can of beans.
	if ( ent->client->ps.powerups[PW_QUAD] ) {
		toot->pka_flags = PKAEF_QUADFART;
	}
	else {
		toot->pka_flags = 0;
	}

	trap_LinkEntity( toot );

}

//PKMOD - End

#ifdef MISSIONPACK
/*
======================================================================

NAILGUN

======================================================================
*/

void Weapon_Nailgun_Fire (gentity_t *ent) {
	gentity_t	*m;
	int			count;

	for( count = 0; count < NUM_NAILSHOTS; count++ ) {
		m = fire_nail (ent, muzzle, forward, right, up );
		m->damage *= s_quadFactor;
		m->splashDamage *= s_quadFactor;
	}

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

PROXIMITY MINE LAUNCHER

======================================================================
*/

void weapon_proxlauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize( forward );

	m = fire_prox (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

#endif


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
//void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
//	VectorCopy( ent->s.pos.trBase, muzzlePoint );
//	muzzlePoint[2] += ent->client->ps.viewheight;
//	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
//	// snap to integer coordinates for more efficient network bandwidth usage
//	SnapVector( muzzlePoint );
//}


/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {

	//PKMOD - Ergodic 11/02/00
//	int		temp;

	//PKMOD - Ergodic 02/05/02 - debug view angles (inactive)
//	vec3_t	hold_vector1;
//	vec3_t	hold_vector2;
//	vec3_t	hold_vector3;
//	vec3_t	hold_vector4;
//	vec3_t	hold_vector5;


	if ( ent->client->ps.powerups[PW_QUAD] ) {
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if( ent->client->persistantPowerup && ent->client->persistantPowerup->item && ent->client->persistantPowerup->item->giTag == PW_DOUBLER ) {
		s_quadFactor *= 2;
	}
#endif

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if( ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_GAUNTLET ) {
#ifdef MISSIONPACK
		if( ent->s.weapon == WP_NAILGUN ) {
			ent->client->accuracy_shots += NUM_NAILSHOTS;
		} else {
			ent->client->accuracy_shots++;
		}
#else
		ent->client->accuracy_shots++;
#endif
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	//PKMOD - Ergodic 02/05/02 - debug view angles (inactive)
//	VectorScale( ent->client->ps.viewangles, 1000, hold_vector1 );
//	VectorScale( forward, 1000, hold_vector2 );
//	VectorScale( right, 1000, hold_vector3 );
//	VectorScale( up, 1000, hold_vector4 );
//	Com_Printf("FireWeapon: 1000*viewangles>%s<, 1000*forward>%s<, 1000*right>%s<, 1000*up>%s<\n", vtos( hold_vector1 ), vtos( hold_vector2 ), vtos( hold_vector3 ), vtos( hold_vector4 ) );


	//PKMOD - Ergodic 11/02/00
	//PKMOD - Ergodic 12/19/00 use time2 instead of angles2 
//	temp = ent->s.angles2[BEARTRAPS_ATTACHED] + 0.45; //rounding fun
//	temp = ent->s.time2;
	//PKMOD - Ergodic 11/02/00 - Debug inactive
//	Com_Printf("Fire_Weapon - STAT_BEARTRAPS_ATTACHED>%d<, angles2>%d<\n", ent->client->ps.stats[STAT_BEARTRAPS_ATTACHED], temp );

	// fire the specific weapon
	switch( ent->s.weapon ) {
	case WP_GAUNTLET:
		Weapon_Gauntlet( ent );
		break;
	case WP_LIGHTNING:
		Weapon_LightningFire( ent );
		break;
	case WP_SHOTGUN:
		weapon_supershotgun_fire( ent );
		break;
	case WP_MACHINEGUN:
		if ( g_gametype.integer != GT_TEAM ) {
			//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE * s_quadFactor );
		} else {
			//PKMOD - Ergodic 12/03/00 - calculate damage before Bullet_Fire routine is called
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_TEAM_DAMAGE * s_quadFactor );
		}
		break;
	case WP_GRENADE_LAUNCHER:
		weapon_grenadelauncher_fire( ent );
		break;
	case WP_ROCKET_LAUNCHER:
		Weapon_RocketLauncher_Fire( ent );
		break;
	case WP_PLASMAGUN:
		Weapon_Plasmagun_Fire( ent );
		break;
	case WP_RAILGUN:
		weapon_railgun_fire( ent );
		break;
	case WP_BFG:
		BFG_Fire( ent );
		break;
	case WP_GRAPPLING_HOOK:
		//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//		Com_Printf("Fire_Weapon - DRAGON: BT_AMMO>%d<\n", ent->client->ps.ammo[WP_BEARTRAP] );

		Weapon_GrapplingHook_Fire( ent );
		break;

#ifdef MISSIONPACK
	case WP_NAILGUN:
		Weapon_Nailgun_Fire( ent );
		break;
	case WP_PROX_LAUNCHER:
		weapon_proxlauncher_fire( ent );
		break;
	case WP_CHAINGUN:
		Bullet_Fire( ent, CHAINGUN_SPREAD, MACHINEGUN_DAMAGE );
		break;
#endif


	/*PKMOD -Add Weapons. 
	WP_GRAVITY,
	WP_SENTRY,
	WP_BEARTRAP,
	WP_CHAINLG,
	WP_AIRFIST,
	WP_NAILGUN,
	PKMOD -Add Weapons. */
	case WP_GRAVITY :
		Weapon_fire_gravity( ent );
		break;
	case WP_SENTRY :
		Weapon_fire_autosentry( ent );
		break;
	case WP_BEARTRAP :
		//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//		Com_Printf("Fire_Weapon - BEARTRAP: BT_AMMO>%d<\n", ent->client->ps.ammo[WP_BEARTRAP] );
		Weapon_fire_beartrap( ent );
		break;

/*PKMOD - Ergodic 06/05/03 - remove dead code for CLG...
	case WP_CHAINLG :
		Weapon_fire_chainlg( ent );
		break;
end 06/05/03 remove section*/

	case WP_AIRFIST :
		Weapon_fire_airfist( ent );
		break;
	case WP_NAILGUN :
		Weapon_fire_nailgun( ent );
		break;
	case WP_EXPLODING_SHELLS:
		weapon_exploding_shells_fire( ent );
		break;
	case WP_BEANS:
		weapon_beans_eat( ent );
		break;
	//PKMOD

	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}


#ifdef MISSIONPACK

/*
===============
KamikazeRadiusDamage
===============
*/
static void KamikazeRadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (!ent->takedamage) {
			continue;
		}

		// dont hit things we have already hit
		if( ent->kamikazeTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE );
			ent->kamikazeTime = level.time + 3000;
//		}
	}
}

/*
===============
KamikazeShockWave
===============
*/
static void KamikazeShockWave( vec3_t origin, gentity_t *attacker, float damage, float push, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 )
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		// dont hit things we have already hit
		if( ent->kamikazeShockTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			dir[2] += 24;
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE );
			//
			dir[2] = 0;
			VectorNormalize(dir);
			if ( ent->client ) {
				ent->client->ps.velocity[0] = dir[0] * push;
				ent->client->ps.velocity[1] = dir[1] * push;
				ent->client->ps.velocity[2] = 100;
			}
			ent->kamikazeShockTime = level.time + 3000;
//		}
	}
}

/*
===============
KamikazeDamage
===============
*/
static void KamikazeDamage( gentity_t *self ) {
	int i;
	float t;
	gentity_t *ent;
	vec3_t newangles;

	self->count += 100;

	if (self->count >= KAMI_SHOCKWAVE_STARTTIME) {
		// shockwave push back
		t = self->count - KAMI_SHOCKWAVE_STARTTIME;
		KamikazeShockWave(self->s.pos.trBase, self->activator, 25, 400,	(int) (float) t * KAMI_SHOCKWAVE_MAXRADIUS / (KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME) );
	}
	//
	if (self->count >= KAMI_EXPLODE_STARTTIME) {
		// do our damage
		t = self->count - KAMI_EXPLODE_STARTTIME;
		KamikazeRadiusDamage( self->s.pos.trBase, self->activator, 400,	(int) (float) t * KAMI_BOOMSPHERE_MAXRADIUS / (KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME) );
	}

	// either cycle or kill self
	if( self->count >= KAMI_SHOCKWAVE_ENDTIME ) {
		G_FreeEntity( self );
		return;
	}
	self->nextthink = level.time + 100;

	// add earth quake effect
	newangles[0] = crandom() * 2;
	newangles[1] = crandom() * 2;
	newangles[2] = 0;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE) {
			ent->client->ps.velocity[0] += crandom() * 120;
			ent->client->ps.velocity[1] += crandom() * 120;
			ent->client->ps.velocity[2] = 30 + random() * 25;
		}

		ent->client->ps.delta_angles[0] += ANGLE2SHORT(newangles[0] - self->movedir[0]);
		ent->client->ps.delta_angles[1] += ANGLE2SHORT(newangles[1] - self->movedir[1]);
		ent->client->ps.delta_angles[2] += ANGLE2SHORT(newangles[2] - self->movedir[2]);
	}
	VectorCopy(newangles, self->movedir);
}

/*
===============
G_StartKamikaze
===============
*/
void G_StartKamikaze( gentity_t *ent ) {
	gentity_t	*explosion;
	gentity_t	*te;
	vec3_t		snapped;

	// start up the explosion logic
	explosion = G_Spawn();

	explosion->s.eType = ET_EVENTS + EV_KAMIKAZE;
	explosion->eventTime = level.time;

	if ( ent->client ) {
		VectorCopy( ent->s.pos.trBase, snapped );
	}
	else {
		VectorCopy( ent->activator->s.pos.trBase, snapped );
	}
	SnapVector( snapped );		// save network bandwidth
	G_SetOrigin( explosion, snapped );

	explosion->classname = "kamikaze";
	explosion->s.pos.trType = TR_STATIONARY;

	explosion->kamikazeTime = level.time;

	explosion->think = KamikazeDamage;
	explosion->nextthink = level.time + 100;
	explosion->count = 0;
	VectorClear(explosion->movedir);

	trap_LinkEntity( explosion );

	if (ent->client) {
		//
		explosion->activator = ent;
		//
		ent->s.eFlags &= ~EF_KAMIKAZE;
		// nuke the guy that used it
		G_Damage( ent, ent, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_KAMIKAZE );
	}
	else {
		if ( !strcmp(ent->activator->classname, "bodyque") ) {
			explosion->activator = &g_entities[ent->activator->r.ownerNum];
		}
		else {
			explosion->activator = ent->activator;
		}
	}

	// play global sound at all clients
	te = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = GTS_KAMIKAZE;
}
#endif
