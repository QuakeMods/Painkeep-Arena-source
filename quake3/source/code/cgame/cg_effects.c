// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"


/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = crandom()*5 + 6;

		VectorAdd (move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ) {
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random( &seed ) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	} else {
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

#ifndef MISSIONPACK
	re->customShader = cgs.media.teleportEffectShader;
#endif
	re->hModel = cgs.media.teleportEffectModel;
	AxisClear( re->axis );

	VectorCopy( org, re->origin );
#ifdef MISSIONPACK
	re->origin[2] += 16;
#else
	re->origin[2] -= 24;
#endif
}

/*
==================
CG_PersentrySpawnEffect

PKMOD - Ergodic 06/08/02 - add personal sentry teleport model
==================
*/
void CG_PersentrySpawnEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.persentry_teleportEffectModel;
	AxisClear( re->axis );

	VectorCopy( org, re->origin );
	re->origin[2] -= 24;
}


#ifdef MISSIONPACK
/*
===============
CG_LightningBoltBeam
===============
*/
void CG_LightningBoltBeam( vec3_t start, vec3_t end ) {
	localEntity_t	*le;
	refEntity_t		*beam;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SHOWREFENTITY;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;

	beam = &le->refEntity;

	VectorCopy( start, beam->origin );
	// this is the end point
	VectorCopy( end, beam->oldorigin );

	beam->reType = RT_LIGHTNING;
	beam->customShader = cgs.media.lightningShader;
}

/*
==================
CG_KamikazeEffect
==================
*/
void CG_KamikazeEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_KAMIKAZE;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;//2250;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy( org, re->origin );

}

/*
==================
CG_ObeliskExplode
==================
*/
void CG_ObeliskExplode( vec3_t org, int entityNum ) {
	localEntity_t	*le;
	vec3_t origin;

	// create an explosion
	VectorCopy( org, origin );
	origin[2] += 64;
	le = CG_MakeExplosion( origin, vec3_origin,
						   cgs.media.dishFlashModel,
						   cgs.media.rocketExplosionShader,
						   600, qtrue );
	le->light = 300;
	le->lightColor[0] = 1;
	le->lightColor[1] = 0.75;
	le->lightColor[2] = 0.0;
}

/*
==================
CG_ObeliskPain
==================
*/
void CG_ObeliskPain( vec3_t org ) {
	float r;
	sfxHandle_t sfx;

	// hit sound
	r = rand() & 3;
	if ( r < 2 ) {
		sfx = cgs.media.obeliskHitSound1;
	} else if ( r == 2 ) {
		sfx = cgs.media.obeliskHitSound2;
	} else {
		sfx = cgs.media.obeliskHitSound3;
	}
	trap_S_StartSound ( org, ENTITYNUM_NONE, CHAN_BODY, sfx );
}


/*
==================
CG_InvulnerabilityImpact
==================
*/
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles ) {
	localEntity_t	*le;
	refEntity_t		*re;
	int				r;
	sfxHandle_t		sfx;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULIMPACT;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityImpactModel;

	VectorCopy( org, re->origin );
	AnglesToAxis( angles, re->axis );

	r = rand() & 3;
	if ( r < 2 ) {
		sfx = cgs.media.invulnerabilityImpactSound1;
	} else if ( r == 2 ) {
		sfx = cgs.media.invulnerabilityImpactSound2;
	} else {
		sfx = cgs.media.invulnerabilityImpactSound3;
	}
	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, sfx );
}

/*
==================
CG_InvulnerabilityJuiced
==================
*/
void CG_InvulnerabilityJuiced( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULJUICED;
	le->startTime = cg.time;
	le->endTime = cg.time + 10000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityJuicedModel;

	VectorCopy( org, re->origin );
	VectorClear(angles);
	AnglesToAxis( angles, re->axis );

	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, cgs.media.invulnerabilityJuicedSound );
}

#endif

/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum( int client, vec3_t org, int score ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;

	// only visualize for the client that scored
	if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0) {
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;
	
	VectorCopy( org, le->pos.trBase );
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis( angles, re->axis );
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, qhandle_t shader,
								int msec, qboolean isSprite ) {
	float			ang;
	localEntity_t	*ex;
	int				offset;
	vec3_t			tmpVec, newOrigin;

	if ( msec <= 0 ) {
		CG_Error( "CG_MakeExplosion: msec = %i", msec );
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if ( isSprite ) {
		ex->leType = LE_SPRITE_EXPLOSION;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale( dir, 16, tmpVec );
		VectorAdd( tmpVec, origin, newOrigin );
	} else {
		ex->leType = LE_EXPLOSION;
		VectorCopy( origin, newOrigin );

		// set axis with random rotate
		if ( !dir ) {
			AxisClear( ex->refEntity.axis );
		} else {
			ang = rand() % 360;
			VectorCopy( dir, ex->refEntity.axis[0] );
			RotateAroundDirection( ex->refEntity.axis, ang );
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy( newOrigin, ex->refEntity.origin );
	VectorCopy( newOrigin, ex->refEntity.oldorigin );

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed( vec3_t origin, int entityNum ) {
	localEntity_t	*ex;

	if ( !cg_blood.integer ) {
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;
	
	VectorCopy ( origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->refEntity.radius = 24;

	ex->refEntity.customShader = cgs.media.bloodExplosionShader;

	// don't show player's own blood in view
	if ( entityNum == cg.snap->ps.clientNum ) {
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}



/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;

	le->leBounceSoundType = LEBS_BLOOD;
	le->leMarkType = LEMT_BLOOD;
}

/*
==================
//PKMOD - Ergodic 12/14/00 - make gib less intensive
CG_LaunchGibAlternate
==================
*/
void CG_LaunchGibAlternate( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 3000 + random() * 2500;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	//PKMOD - Ergodic 12/16/00 - use "f" to force float
	le->bounceFactor = 0.3f;

	if ( random() > 0.4 ) {
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
	else {
		le->leBounceSoundType = LEBS_BLOOD;
		le->leMarkType = LEMT_BLOOD;
	}
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	if ( rand() & 1 ) {
		CG_LaunchGib( origin, velocity, cgs.media.gibSkull );
	} else {
		CG_LaunchGib( origin, velocity, cgs.media.gibBrain );
	}

	// allow gibs to be turned off for speed
	if ( !cg_gibs.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibAbdomen );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibArm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibChest );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFist );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFoot );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibForearm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibIntestine );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_BigExplode( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*1.5;
	velocity[1] = crandom()*EXP_VELOCITY*1.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.0;
	velocity[1] = crandom()*EXP_VELOCITY*2.0;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.5;
	velocity[1] = crandom()*EXP_VELOCITY*2.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );
}

//PKMOD - Ergodic 12/14/00 - add autosentry gib models
#define AUTOSENTRY_SHOWGIB	0.7		//show gib 70% of the time
//PKMOD - Ergodic 11/22/00 - autosentry death routine
//PKMOD - Ergodic 12/14/00 - add autosentry gib models
void CG_AutoSentryDie( vec3_t AutoSentryOrigin ) {
	vec3_t	origin, velocity;

		//PKMOD Ergodic debug 12/14/00 inactive
//		Com_Printf("CG_AutoSentryDie\n" );


	VectorCopy( AutoSentryOrigin, origin );

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib1 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib2 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib3 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib4 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib5 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib6 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib7 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib8 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib9 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib10 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib11 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib12 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGib( origin, velocity, cgs.media.pkasentry_gib13 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib14 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib15 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib16 );
	}

	if ( random() < AUTOSENTRY_SHOWGIB ) {
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGibAlternate( origin, velocity, cgs.media.pkasentry_gib17 );
	}

}

//PMKOD - Ergodic - 05/26/00 - Beartrap death
/*
==================
CG_BearTrapDie
==================
*/
void CG_BearTrapDie( vec3_t BearTrapOrigin ) {
	vec3_t	origin, velocity;
	int		number_of_gibs;

	VectorCopy( BearTrapOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.pkabeartrapgib1 );

	VectorCopy( BearTrapOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.pkabeartrapgib2 );

	VectorCopy( BearTrapOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.pkabeartrapgib3 );

	number_of_gibs = random() * 5;

	while (number_of_gibs > 0) {
		VectorCopy( BearTrapOrigin, origin );
		velocity[0] = crandom()*GIB_VELOCITY;
		velocity[1] = crandom()*GIB_VELOCITY;
		velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
		CG_LaunchGib( origin, velocity, cgs.media.pkabeartrapgib4 );
		number_of_gibs--;
	}

}


//PKMOD - Ergodic debug position
char	*CG_vtos_2( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

/*
==================
CG_Lightning_FX

Create a light flash effect
Note: effect must be turned off
==================
*/
//PKMOD - Ergodic 09/30/01 - event parm will send intensity
//PKMOD - Ergodic 10/01/01 - add mulitple points
#define MAX_LIGHTNING_POINTS 25
#define MEAN_LIGHTNING_DISTANCE 20

void CG_Lightning_FX( vec3_t position, int intensity ) {
	int		i, r, g, b;
	int		indx;
	vec3_t	hold_position; 

	//PKMOD Ergodic debug 07/19/00 (inactive)
//Com_Printf("CG_Lightning_FX - position>%s<, intensity>%d<\n", CG_vtos_2(position), intensity );


	r = 255;
	g = 255;
	b = 255;
	i = intensity;
	for (indx = 1; indx < MAX_LIGHTNING_POINTS; indx++ ) {
		hold_position[0] = position[0] + crandom() * MEAN_LIGHTNING_DISTANCE; 
		hold_position[1] = position[1] + crandom() * MEAN_LIGHTNING_DISTANCE; 
		hold_position[2] = position[2] + crandom() * MEAN_LIGHTNING_DISTANCE; 
		trap_R_AddLightToScene( hold_position, i, r, g, b );
	}

}


/*
==================
CG_Lightning_Water_Discharge

//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
==================
*/
void CG_Lightning_Water_Discharge (vec3_t origin, int msec) {

	localEntity_t		*le;

	if (msec <= 0)
		msec = 300;

	le = CG_SmokePuff (	origin,				// where
				vec3_origin,				// where to
				((48 + (msec * 10)) / 16),	// radius
				1, 1, 1, 1,					// RGBA color shift
				300 + msec,					// duration
				cg.time,					// start when?
				0,							//fade in time
				0,							// flags (?)
				trap_R_RegisterShader ("models/weaphits/electric.tga"));

	le->leType = LE_SCALE_FADE;
}


/*
==================
CG_TootBubbles
//PKMOD - Ergodic 04/13/01 - Toots underwater
==================
*/
#define		TootBubbleCount	10
//PKMOD - Ergodic 04/14/01 - Soft code
#define		TootDisplacementMax 6			//	displacement from center
#define		TootDisplacementOffset 13		// 1 + 2 * TootDisplacementMax
#define		Toot_H_VelocityMax	7			// horizontal velocity of bubbles
#define		Toot_H_VelocityOffset 15		// 1 + 2 * Toot_H_VelocityMax
#define		Toot_V_VelocityMax	20			// Vertical velocity of bubbles


void CG_TootBubbles( vec3_t start ) {
	vec3_t		hold_vec;
	int			i;
	int			bubbles;
	
	bubbles = 10 + rand() % TootBubbleCount;	//{10, 19}
	VectorCopy ( start, hold_vec );

	for ( i = 1; i < bubbles; i++ ) {
		localEntity_t	*le;
		refEntity_t		*re;

		VectorCopy ( start, hold_vec ); //random location neary the hiney
		hold_vec[0] += TootDisplacementMax - rand() % TootDisplacementOffset;	//{-6, 6}
		hold_vec[1] += TootDisplacementMax - rand() % TootDisplacementOffset;	//{-6, 6}
		hold_vec[2] += TootDisplacementMax - rand() % TootDisplacementOffset;	//{-6, 6}

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( hold_vec, le->pos.trBase );

		le->pos.trDelta[0] = Toot_H_VelocityMax - rand() % Toot_H_VelocityOffset;	//{-7, 7}
		le->pos.trDelta[1] = Toot_H_VelocityMax - rand() % Toot_H_VelocityOffset;	//{-7, 7}
		le->pos.trDelta[2] = Toot_V_VelocityMax + rand() % Toot_V_VelocityMax;	//{20, 39}

	}
}

/*
======================
CG_BeansToot

//PKMOD - Ergodic 04/13/01 - Renders fecal plumes or bubbles from eating beans
======================
*/
void CG_BeansToot( vec3_t origin ) {
	int			sourceContentType;
	vec3_t		hiney_location;
	localEntity_t	*smoke;
	vec3_t			up = {0, 0, 5};

	VectorCopy( origin, hiney_location);
	hiney_location[2] +=16;		//waist displacement height


	sourceContentType = trap_CM_PointContents( hiney_location, 0 );

	if ( sourceContentType & CONTENTS_WATER ) {
		CG_TootBubbles( hiney_location );
	}
	else {
		//PKMOD - Ergodic 12/27/00 add beans shader
		smoke = CG_SmokePuff( hiney_location, up, 
			  32, 
			  1, 1, 1, 0.33f,
			  1500, 
			  cg.time, 0,
			  LEF_PUFF_DONT_SCALE, 
			  cgs.media.pkafartPuffShader );
	}

}

	//PKMOD - Ergodic 01/05/04 - add quad farting logic for differing CG graphic sequence
/*
======================
CG_BeansToot

PKMOD - Ergodic 01/05/04 - add quad farting logic for differing CG graphic sequence
		 - Renders fecal plumes or bubbles from eating beans
		 Ergodic 01/12/04 - add in scaling function local entity
======================
*/
void CG_QuadBeansToot( vec3_t origin ) {
	int			sourceContentType;
	vec3_t		hiney_location;
	localEntity_t	*smoke;
//	vec3_t			up = {0, 0, 5};


	//PKMOD - Ergodic 01/12/04 - debug quad fart activation (inactive)
	//Com_Printf( "inside cg_quadbeanstoot\n" );

	VectorCopy( origin, hiney_location);
	hiney_location[2] +=16;		//waist displacement height


	sourceContentType = trap_CM_PointContents( hiney_location, 0 );

	if ( sourceContentType & CONTENTS_WATER ) {
		CG_TootBubbles( hiney_location );
	}

	smoke = CG_AllocLocalEntity();

	smoke->leType = LE_SCALED_SPRITE_EXPLOSION;

	smoke->startTime = cg.time;
	smoke->endTime = smoke->startTime + 1100;  //Keep in sync with cg_localents

	// bias the time so all shader effects start correctly
	smoke->refEntity.shaderTime = smoke->startTime / 1000.0f;

	smoke->refEntity.hModel = cgs.media.pkaquadbeansModel;
	smoke->refEntity.customShader = cgs.media.pkaquadbeansShader;

	// set origin
	VectorCopy( hiney_location, smoke->refEntity.origin );
	VectorCopy( hiney_location, smoke->refEntity.oldorigin );

	smoke->light = 300;
	smoke->lightColor[0] = 1;
	smoke->lightColor[1] = 0.75;
	smoke->lightColor[2] = 0.0;

}

/*
==================
CG_Radiation

//PKMOD - Ergodic 10/14/01 - add radiation sparking effect
==================
*/

#define CG_RADIATION_COUNT 20
#define CG_RADIATION_MIN_RADIUS 10
#define CG_RADIATION_DELTA_RADIUS 25		//delta radius = max radius - min radius
#define CG_RADIATION_MIN_TIME 300
#define CG_RADIATION_DELTA_TIME 500			//delta time = max time - min time
void CG_Radiation( vec3_t start ) {
	vec3_t		move;
	int			i;

	for ( i = 1; i < CG_RADIATION_COUNT; i++ ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + CG_RADIATION_MIN_TIME + random() * CG_RADIATION_DELTA_TIME;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 2;
		switch ( rand() % 6 ) {	//random numbers: { 0, 1, 2, 3, 4, 5 }
			case 0:
				re->customShader = cgs.media.radiate1Shader;
				break;
			case 1:
				re->customShader = cgs.media.radiate2Shader;
				break;
			case 2:
				re->customShader = cgs.media.radiate3Shader;
				break;
			case 3:
				re->customShader = cgs.media.radiate4Shader;
				break;
			case 4:
				re->customShader = cgs.media.radiate5Shader;
				break;
			default:
				re->customShader = cgs.media.radiate6Shader;
				break;
		}
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		move[0] = start[0] + crandom() * ( CG_RADIATION_MIN_RADIUS + random() * CG_RADIATION_DELTA_RADIUS );
		move[1] = start[1] + crandom() * ( CG_RADIATION_MIN_RADIUS + random() * CG_RADIATION_DELTA_RADIUS );
		move[2] = start[2] + crandom() * ( CG_RADIATION_MIN_RADIUS + random() * CG_RADIATION_DELTA_RADIUS );

		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = crandom()*10;
		le->pos.trDelta[1] = crandom()*10;
		le->pos.trDelta[2] = crandom()*10;

	}
}

/*
=====================
CG_RadiationTrail

PKMOD - Ergodic 11/30/01 - add radiation trail to infected player
=====================
*/
void CG_RadiationTrail0( const vec3_t p ) {

	vec3_t vel;
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->radius = 3;

	re = &le->refEntity;
	re->rotation = Q_random( &seed ) * 360;
	re->radius = 3;
	re->shaderTime = cg.time / 1000.0f;

	le->leType = LE_FALL_SCALE_FADE;
	le->startTime = cg.time;
	le->fadeInTime = 0;
	le->endTime = cg.time + 1000 + (rand() % 1001);
	if ( le->fadeInTime > le->startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = 1;
	le->color[1] = 1; 
	le->color[2] = 1;
	le->color[3] = 1;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = le->startTime;
	vel[0] = 25 - 2 * ( rand() % 25 );
	vel[1] = 25 - 2 * ( rand() % 25 );
	vel[2] = 25 - 2 * ( rand() % 25 );
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = cgs.media.radiationTrailShader;

	// rage pro can't alpha fade, so use a different shader
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	} else {
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	// drop a total of 2 units over its lifetime
	le->pos.trDelta[2] = 2;

}


/*
=====================
CG_RadiationTrail

PKMOD - Ergodic 11/30/01 - add radiation trail to infected player
=====================
*/
void CG_RadiationTrail( const vec3_t p, vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ) {
//	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
//	re->rotation = Q_random( &seed ) * 360;
	re->rotation = rand() % 360;		//PKMOD - Ergodic 11/30/01
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	} else {
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	// drop a total of 5 units over its lifetime
	le->pos.trDelta[2] = 20;

}

