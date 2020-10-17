// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * crandom();
		} else {
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, 0, 0, 64 );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail (clientInfo_t *ci, vec3_t start, vec3_t end) {
	vec3_t axis[36], move, move2, next_move, vec, temp;
	float  len;
	int    i, j, skip;
 
	localEntity_t *le;
	refEntity_t   *re;
 
#define RADIUS   4
#define ROTATION 1
#define SPACING  5
 
	start[2] -= 4;
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	PerpendicularVector(temp, vec);
	for (i = 0 ; i < 36; i++) {
		RotatePointAroundVector(axis[i], vec, temp, i * 10);//banshee 2.4 was 10
	}
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
 
	VectorMA(move, 20, vec, move);
	VectorCopy(move, next_move);
	VectorScale (vec, SPACING, vec);

	if (cg_oldRail.integer != 0) {
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;
		return;
	}
	skip = -1;
 
	j = 18;
    for (i = 0; i < len; i += SPACING) {
		if (i != skip) {
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
            re = &le->refEntity;
            le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
            le->startTime = cg.time;
            le->endTime = cg.time + (i>>1) + 600;
            le->lifeRate = 1.0 / (le->endTime - le->startTime);

            re->shaderTime = cg.time / 1000.0f;
            re->reType = RT_SPRITE;
            re->radius = 1.1f;
			re->customShader = cgs.media.railRingsShader;

            re->shaderRGBA[0] = ci->color2[0] * 255;
            re->shaderRGBA[1] = ci->color2[1] * 255;
            re->shaderRGBA[2] = ci->color2[2] * 255;
            re->shaderRGBA[3] = 255;

            le->color[0] = ci->color2[0] * 0.75;
            le->color[1] = ci->color2[1] * 0.75;
            le->color[2] = ci->color2[2] * 0.75;
            le->color[3] = 1.0f;

            le->pos.trType = TR_LINEAR;
            le->pos.trTime = cg.time;

			VectorCopy( move, move2);
            VectorMA(move2, RADIUS , axis[j], move2);
            VectorCopy(move2, le->pos.trBase);

            le->pos.trDelta[0] = axis[j][0]*6;
            le->pos.trDelta[1] = axis[j][1]*6;
            le->pos.trDelta[2] = axis[j][2]*6;
		}

        VectorAdd (move, vec, move);

        j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
	}
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

#ifdef MISSIONPACK
/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.nailPuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
#endif

/*
==========================
CG_NailTrail
==========================
*/
static void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi ) {
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	vec3_t			velocity, xvelocity, origin;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				t, startTime, step;

	float	waterScale = 1.0f;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	step = 50;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
    re->shaderTime = cg.time / 1000.0f;
    re->reType = RT_SPRITE;
    re->radius = 0.25f;
	re->customShader = cgs.media.railRingsShader;
	le->bounceFactor = 0.3f;

    re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
    re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
    re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
    re->shaderRGBA[3] = 63;

    le->color[0] = wi->flashDlightColor[0] * 0.2;
    le->color[1] = wi->flashDlightColor[1] * 0.2;
    le->color[2] = wi->flashDlightColor[2] * 0.2;
    le->color[3] = 0.25f;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;

}

//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player)
/*
=============
VectorToString PKMOD REMOVE THIS LATER

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
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
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin;
	entityState_t	*es;
	vec3_t			forward, up;
	refEntity_t		beam;

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player) (inactive)
//	int				dragon_distance;

	es = &ent->currentState;

	//PKMOD - Ergodic 03/10/01 - set tongue's location in the old origin for cgame hack
//	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	ent->trailTime = cg.time;

	memset( &beam, 0, sizeof( beam ) );
	//FIXME adjust for muzzle position
	VectorCopy ( cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin );
	beam.origin[2] += 26;
	AngleVectors( cg_entities[ ent->currentState.otherEntityNum ].lerpAngles, forward, NULL, up );
	VectorMA( beam.origin, -6, up, beam.origin );
	//PKMOD - Ergodic 03/10/01 - set tongue's location in the old origin for cgame hack
//	VectorCopy( origin, beam.oldorigin );

	//PKMOD - Ergodic 03/11/01 - if dragon return origin is set then use it, else use launch value
	//				This will imply that dragon will not work correctly at coordinates (0,0,0) 
	//				fix in the patch
	if ( VectorLength( es->origin2 ) > 1 ) {
		VectorCopy( es->origin2, beam.oldorigin );
	}
	else {
		BG_EvaluateTrajectory( &es->pos, cg.time, origin );
		VectorCopy( origin, beam.oldorigin );
	}

	//PKMOD - Ergodic 03/10/01 - debug dragon return (trail appears behind player) (inactive)
//	dragon_distance = Distance( beam.origin, beam.oldorigin );
//	Com_Printf( "CG_GrappleTrail - origin>%s<, old>%s<, dist>%d<\n", vtos(beam.origin), vtos(beam.oldorigin), dragon_distance );


	if (Distance( beam.origin, beam.oldorigin ) < 64 )
		return; // Don't draw if close

	//PKMOD - Ergodic 10/03/00 change flash to railgun style to make a tighter beam
	beam.reType = RT_RAIL_CORE;
//	beam.reType = RT_LIGHTNING;
	//PKMOD - Ergodic 10/03/00 dragon beam shader effect
	beam.customShader = cgs.media.dragonboltShader;
//	beam.customShader = cgs.media.lightningShader;

	AxisClear( beam.axis );
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &beam );
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	CG_RocketTrail( ent, wi );
}

/*
==========================
//PKMOD - Ergodic 01/11/01 - add a nail trail that is "slimmed" down  
CG_PKANailTrail
==========================
*/
static void PKANailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 200; //was 50

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 48 );	//was 8
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	//PKMOD - Ergodic 03/04/01 - add lightning gun to barrel list
	//PKMOD - Ergodic 03/27/01 - remove lightning gun to barrel list 
	//PKMOD - Ergodic 05/15/03 - re-add lightning gun to barrel list for Uber's clg model 
//	if ( weaponNum == WP_MACHINEGUN || weaponNum == WP_GAUNTLET || weaponNum == WP_BFG ) {
	if ( weaponNum == WP_MACHINEGUN || weaponNum == WP_GAUNTLET || weaponNum == WP_LIGHTNING ) {
		strcpy( path, item->world_model[0] );
		COM_StripExtension( path, path );
		strcat( path, "_barrel.md3" );
		weaponInfo->barrelModel = trap_R_RegisterModel( path );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path );
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	switch ( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
		break;

	case WP_LIGHTNING:
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		//PKMOD - Ergodic 09/06/00 add new staionary sound from Mongusta
		//12/16/00 - add non compressed flag
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight Stat#6a.wav", qfalse );

		//12/16/00 - add non compressed flag
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav", qfalse );

		//PKMOD - Ergodic 08/22/00 add multiple fire sounds from Mongusta
		//12/16/00 - add non compressed flag
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight Fire#2.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight Fire#3.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons2/chainlightning/Chainlight Fire#5.wav", qfalse );

		//PKMOD - Ergodic 08/21/00 set the shader to chainlightning
		cgs.media.chainlightningShader = trap_R_RegisterShader( "chainlightningBolt" );
		//PKMOD - Ergodic 08/21/00 set the map hit model to correct color
		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/clghit1.md3" );
		//12/16/00 - add non compressed flag
		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav", qfalse );

		break;

	//PKMOD - Ergodic 10/03/00 hand models
	//PKMOD - Ergodic 10/03/00 set the shader to the dragon
	case WP_GRAPPLING_HOOK:
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/dragon/tongue.md3" );
		weaponInfo->weaponModel = trap_R_RegisterModel( "models/weapons2/dragon/dragonhold.md3" );
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/dragon/dragonhold_hand.md3" );

		//PKMOD - Ergodic 02/14/02 - use the plasmagun flyby sound for the moving tongue
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );

		//PKMOD - Ergodic 02/14/02 - hack - store the gauntlet blade in the flashsound[3]
		//				Note: this sound will not be hard as a flashsound but will be called in
		//						cg_ents.c(CG_Dragon_Deploy) when gauntletblade is being deployed.
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons2/gauntlet/Bladewhirl.wav", qfalse );

		//PKMOD - Ergodic 10/03/00 set the beam shader to the dragon
		cgs.media.dragonboltShader = trap_R_RegisterShader( "DragonBolt" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->missileDlightColor, 1.0f, 1.0f, 0.5f );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 0.5f );
		//12/16/00 - add non compressed flag
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		//PKMOD - Ergodic 10/03/00 set the dragon fire sound
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons2/dragon/gpulling.wav", qfalse );
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/vulcan/wvulfire.wav", qfalse );
		weaponInfo->loopFireSound = qtrue;
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;
#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/wnalfire.wav", qfalse );
		break;
#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.5f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = trap_R_RegisterShader( "bfgExplosion" );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/bfg.md3" );
		//12/16/00 - add non compressed flag
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse
			);
		break;

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
//PKMOD Ergodic 05/17/00 multi-model hack
	case WP_GRAVITY:
		//PKMOD - Ergodic 03/17/01 new PKA gravity well weapon to the "hold" model
		weaponInfo->weaponModel = trap_R_RegisterModel( "models/weapons2/gwell/gwphold.md3" );
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/gwell/gwphold_hand.md3" );
		//12/16/00 - add non compressed flag
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons2/gwell/gravity_carry.wav", qfalse );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/gwell/gwp.md3" );
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		//PKMOD - Ergodic 07/11/00 - launch sound done in EV_GRAVITY_RELEASED
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons2/gwell/gravity_released.wav" );
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 1.0f, 0.7f, 0.5f );
//PKMOD - Ergodic 05/17/00 - make launch sound same as player jump sound (effort to throw heavy item)
//PKMOD - Ergodic 05/17/00 - don't register sound since it will be programatically controlled
//		weaponInfo->flashSound[0] = CG_CustomSound( es->number, "*jump1.wav" );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
//	case WP_GRAVITY_LAUNCHED:
//		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/grenade1.md3" );
//		weaponInfo->wiTrailTime = 700;
//		weaponInfo->trailRadius = 32;
//		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7, 0.5 );
//PKMOD - Ergodic 05/17/00 - make launch sound same as player jump sound (effort to throw heavy item)
//PKMOD - Ergodic 05/17/00 - don't register sound since it will be programatically controlled
//		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons2/gwell/gravity_released.wav" );
//		break;
	case WP_SENTRY:
		weaponInfo->weaponModel = trap_R_RegisterModel( "models/weapons2/autosentry/autosentry_hold.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 1.0f, 0.7f, 0.5f );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
	case WP_BEARTRAP:
		weaponInfo->weaponModel = trap_R_RegisterModel( "models/weapons2/beartrap/bearhold.md3" );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/beartrap/beartrap.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 1.0f, 0.7f, 0.5f );
//PKMOD - Ergodic 05/27/00 - don't register sound since it will be programatically controlled
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav" );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
//	case WP_CHAINLG:
//		MAKERGB( weaponInfo->flashDlightColor, 0.6, 0.6, 1 );
//		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav" );
//		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav" );
//
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/lightning/lg_fire.wav" );
//		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBolt" );
//		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/crackle.md3" );
//		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav" );
//		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav" );
//		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav" );
//
//		break;
//PKMOD - Ergodic 05/17/00 modify airfist registration parameters
	case WP_AIRFIST:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/airfist/airfist.md3" );
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		//12/16/00 - add non compressed flag
		//PKMOD - Ergodic 07/01/01 - sound will be added programmatically later in this module
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons2/airfist/agfire22.wav", qfalse );

		break;
	case WP_NAILGUN:
//PKMOD - Ergodic 07/28/00 add nailgun model
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/nailgun/nail.md3" );
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 1.0f, 1.0f, 0.1f );
//PKMOD - Ergodic 08/05/00 changed the weapon sounds to mongusta's files
		//12/16/00 - add non compressed flag
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons2/nailgun/nailgun_carry.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons2/nailgun/nailgunf1.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons2/nailgun/nailgunf2.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons2/nailgun/nailgunf3.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons2/nailgun/nailgunf4.wav", qfalse );
//		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		//PKMOD - Ergodic 08/01/00 set nailgun impact
		cgs.media.nailImpactShader = trap_R_RegisterShader( "nailImpact" );
//PKMOD - Ergodic 01/11/01 add trail parameters
		weaponInfo->missileTrailFunc = PKANailTrail;
		weaponInfo->wiTrailTime = 400;
		weaponInfo->trailRadius = 8;

		break;
	case WP_EXPLODING_SHELLS:
		//PKMOD - Ergodic 06/18/00 install custom shaders
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		//12/16/00 - add non compressed flag
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;
	case WP_BEANS:
		weaponInfo->weaponModel = trap_R_RegisterModel( "models/weapons2/beans/beanshold.md3" );
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/beans/beanshold_hand.md3" );
		//PKMOD - Ergodic 12/16/00 - add "f" to force a float declaration
		MAKERGB( weaponInfo->flashDlightColor, 1.0f, 0.7f, 0.5f );
		break;

	//PKMOD

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	//PKMOD - Ergodic 03/05/01 - CLG has different pickup model than handhold model (inactive)
	//PKMOD - Ergodic 03/27/01 - code was inactivated due to CLG will not have a rotating barrel
//	if ( ( item->giTag == WP_LIGHTNING ) && ( item->giType == IT_WEAPON ) ) 
//		itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );


	itemInfo->icon = trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// change weapon
	if ( frame >= ci->animations[TORSO_DROP].firstFrame 
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= ci->animations[TORSO_ATTACK].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK2].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}
	
	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	vec3_t			forward;
	vec3_t			muzzlePoint, endPoint;
//	vec3_t			holdv;
	int				shaftee; //12/15/03 hold number of player being directly shafted 
	qboolean		straightbeam = qtrue;

	//PKMOD - Ergodic 07/06/01 - debug (inactive)
//	Com_Printf("CG_LightningBolt\n");

	if ( cent->currentState.weapon != WP_LIGHTNING ) {
		//PKMOD - Ergodic 12/06/00 - debug inactive
//		Com_Printf("CG_LightningBolt - nop exit\n");
		return;
	}

	//PKMOD - Ergodic 07/06/01 - debug lightning strike (inactive)
//	if ( cent->currentState.eFlags & EF_LIGHTNINGSTRIKE )
//		Com_Printf("CG_LightningBolt - EF_LIGHTNINGSTRIKE is set\n");
//	else
//		Com_Printf("CG_LightningBolt - EF_LIGHTNINGSTRIKE inactive\n");

	memset( &beam, 0, sizeof( beam ) );

	//PKMOD - Ergodic 07/08/01 - only display shaft if NOT striking player
	//PKMOD - Ergodic 12/15/03 - change logic to use location of CLG struck player
	//if ( cent->currentState.eFlags & EF_LIGHTNINGSTRIKE )
	//	return;
	if ( cent->currentState.eFlags & EF_LIGHTNINGSTRIKE ) {
		shaftee = cent->currentState.otherEntityNum2;
		//minimally validate client		
		if ( shaftee >= 0 && shaftee < MAX_CLIENTS ) {
			centity_t *target;

			target = &cg_entities[ shaftee ];

			//PKMOD - Ergodic 12/15/03 - debug lightning strike (inactive)
			//Com_Printf("CG_LightningBolt - inside valid client, shaftee>%d<, org>%s<, targ>%s<\n", shaftee, CG_vtos( origin), CG_vtos(target->lerpOrigin) );

			VectorCopy( target->lerpOrigin, beam.oldorigin );
			beam.oldorigin[2] += DEFAULT_VIEWHEIGHT;		//raise the impact point up

			VectorCopy( origin, beam.origin ); //CLG Muzzle Tag location

			straightbeam = qfalse;

		}	
	}

	if ( straightbeam ) {		//here if we are not using lockon shaft beam
		// CPMA  "true" lightning
		if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0)) {
			vec3_t angle;
			int i;

			for (i = 0; i < 3; i++) {
				float a = cent->lerpAngles[i] - cg.refdefViewAngles[i];
				if (a > 180) {
					a -= 360;
				}
				if (a < -180) {
					a += 360;
				}

				angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);
				if (angle[i] < 0) {
					angle[i] += 360;
				}
				if (angle[i] > 360) {
					angle[i] -= 360;
				}
			}

		//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors(angle, forward, NULL, NULL );
			AngleVectorsForward( angle, forward );
			VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
		} else {
			// !CPMA
			//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
			AngleVectorsForward( cent->lerpAngles, forward );
			VectorCopy(cent->lerpOrigin, muzzlePoint );
		}

	//PKMOD - Ergodic 09/10/00 - remove original aiming vector aiming
	//PKMOD - Ergodic 09/20/00 - reset original aiming vector aiming
	//PKMOD - Ergodic 07/07/01 - if player strike then set the beam aim, co-opt field: angles2
//	VectorCopy( cent->currentState.angles2, forward );
	//PKMOD - Ergodic 07/07/01 - debug angles2
//	Com_Printf("CG_LightningBolt: angles2>%s<\n", vtos( cent->currentState.angles2 ) );
	//PKMOD - Ergodic 07/08/01 - reset original aiming vector aiming
	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		AngleVectorsForward( cent->lerpAngles, forward );

	//PKMOD - Ergodic 09/10/00 - set the aim, co-opt field: apos.trDelta
	//	AngleVectors( cent->currentState.apos.trDelta, cent->currentState.apos.trDelta forward, NULL, NULL );
	//PKMOD - Ergodic 09/20/00 - undo apos aiming
//	VectorCopy( cent->currentState.apos.trDelta, holdv );


	//PKMOD - Ergodic 09/20/00 - undo apos aiming
//	VectorCopy( cent->currentState.apos.trDelta, forward );

		// FIXME: crouch
		muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	//PKMOD - Ergodic 01/16/01 - don't draw shaft if doing a Lightning Discharge in water
		if (trap_CM_PointContents (muzzlePoint, 0) & MASK_WATER)
			return;

		VectorMA( muzzlePoint, 14, forward, muzzlePoint );

		// project forward by the lightning range
		VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

		// see if it hit a wall
		CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
			cent->currentState.number, MASK_SHOT );

		// this is the endpoint
		VectorCopy( trace.endpos, beam.oldorigin );

		// use the provided origin, even though it may be slightly
		// different than the muzzle origin
		VectorCopy( origin, beam.origin );
	}

//PKMOD - Ergodic 08/21/00 change flash to railgun style to make a tighter beam
	beam.reType = RT_RAIL_CORE;
//	beam.reType = RT_LIGHTNING;
//PKMOD - Ergodic 08/21/00 set the shader to chainlightning
	beam.customShader = cgs.media.chainlightningShader;
	trap_R_AddRefEntityToScene( &beam );

	if ( straightbeam ) {		//here if we are not using lockon shaft beam
		// add the impact flare if it hit something
		if ( trace.fraction < 1.0 ) {
			vec3_t	angles;
			vec3_t	dir;

			VectorSubtract( beam.oldorigin, beam.origin, dir );
			VectorNormalize( dir );

			memset( &beam, 0, sizeof( beam ) );
			beam.hModel = cgs.media.lightningExplosionModel;

			VectorMA( trace.endpos, -16, dir, beam.origin );

			// make a random orientation
			angles[0] = rand() % 360;
			angles[1] = rand() % 360;
			angles[2] = rand() % 360;
			AnglesToAxis( angles, beam.axis );
			trap_R_AddRefEntityToScene( &beam );
		}
	}
}


/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
static void CG_SpawnRailTrail( centity_t *cent, vec3_t origin ) {
	clientInfo_t	*ci;

	if ( cent->currentState.weapon != WP_RAILGUN ) {
		return;
	}
	if ( !cent->pe.railgunFlash ) {
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RailTrail( ci, origin, cent->pe.railgunImpact );
}


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	//PKMOD - Ergodic 12/19/01 - aways spin the gauntlet on the dragon
	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
#ifdef MISSIONPACK
		if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, trap_S_RegisterSound( "sound/weapons/vulcan/wvulwind.wav", qfalse ) );
		}
#endif
	}

	return angle;
}


/*
======================
CG_MachinegunSpinAngle

PKMOD - Ergodic 12/19/01 - aways spin the gauntlet blade on the dragon
  ======================
*/
static float	CG_GauntletSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	cent->pe.barrelTime = cg.time;
	cent->pe.barrelAngle = AngleMod( angle );
	cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	if ( powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( gun );
	} else {
		trap_R_AddRefEntityToScene( gun );

		if ( powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
		if ( powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
	}
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team ) {
	refEntity_t	gun;
	refEntity_t	barrel;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
//	int	col;

	weaponNum = cent->currentState.weapon;

	//PKMOD - Ergodic 07/05/01 - debug (inactive)
//	if ( rand() % 100 > 97 )
//		Com_Printf( "CG_AddPlayerWeapon - generic1:%d\n", cent->currentState.generic1 );

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if ( ps ) {
		if ( cg.predictedPlayerState.weapon == WP_RAILGUN 
			&& cg.predictedPlayerState.weaponstate == WEAPON_FIRING ) {
			float	f;

			f = (float)cg.predictedPlayerState.weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = 
			gun.shaderRGBA[2] = 255 * ( 1.0 - f );
		} else {
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound );
			cent->pe.lightningFiring = qtrue;
		} else if ( weapon->readySound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound );
		}
	}

	CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon");

	CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );

	//PKMOD - Ergodic 03/01/01 - Dragon alternate fire
	//PKMOD - Ergodic 03/02/01 - modify logic to be based on weaponNum and not ammo
	//PKMOD - Ergodic 03/13/01 - if current weapon is not the dragon then don't use the alternate weapon
	if ( weaponNum == WP_GRAPPLING_HOOK ) {
		//PKMOD - Ergodic 07/05/01 - use cent and not cg.snap so deploy weapon will appear
		//			on multiplayer entities 
		if ( cent->currentState.generic1 ) {
			refEntity_t		gun2;
			weaponInfo_t	*weapon2;
			weapon_t		deployNum;

			//PKMOD - Ergodic 03/14/01 - mask off the weapon bits (0 - 15)
			//deployNum = cg.snap->ps.generic1 & 15;	
			//PKMOD - Ergodic 07/05/01 - use "ps" and not "cg" so deploy weapon 
			//			will appear on multiplayer models and not just local client
			deployNum = cent->currentState.generic1 & 15;	

			// add the weapon
			memset( &gun2, 0, sizeof( gun2 ) );
			VectorCopy( parent->lightingOrigin, gun2.lightingOrigin );
			gun2.shadowPlane = parent->shadowPlane;
			gun2.renderfx = parent->renderfx;

			//PKMOD - Ergodic 12/15/01 - debug deploy gauntlet (inactive)
//			Com_Printf( "CG_AddPlayerWeapon - deploying>%d<\n", deployNum );


			//PKMOD - Ergodic 08/29/01 - move code to include Flag as deploy weapon
			//set the deploy model
			if ( ( deployNum == PW_REDFLAG ) || ( deployNum == PW_BLUEFLAG ) ) {
				if ( deployNum == PW_REDFLAG )
					gun2.hModel = cgs.media.redFlagModel;
				else
					gun2.hModel = cgs.media.blueFlagModel;
			}
			else {		//NOT A FLAG: deploy a PA WEAPON
				CG_RegisterWeapon( WP_GRAPPLING_HOOK );
				weapon2 = &cg_weapons[ deployNum ];
				//PKMOD - Ergodic 12/16/01 - show the barrel if depoying the gauntlet
				if ( deployNum == WP_GAUNTLET ) {
					//PKMOD - Ergodic 12/16/01 - select the barrel model
					gun2.hModel = weapon2->barrelModel;
					//PKMOD - Ergodic 12/16/01 - make gauntlet blade rotate
					angles[YAW] = 0;		// was CG_MachinegunSpinAngle( cent );
					angles[PITCH] = 0;  //was -90
					angles[ROLL] = CG_GauntletSpinAngle( cent );
					AnglesToAxis( angles, gun2.axis );
				}
				else
					gun2.hModel = weapon2->weaponModel;
			}

			if (!gun2.hModel) {
				return;
			}
			//PKMOD - Ergodic 03/01/01 - debug inactive
			//		Com_Printf( "CG_AddPlayerWeapon - attaching beartrap to dragon\n" );

			//PKMOD - Ergodic 12/16/01 - rotate the gauntlet_blade
			if ( deployNum == WP_GAUNTLET ) {
				//PKMOD - Ergodic 01/26/02 - only display gauntlet if NOT firing
				if ( !( cent->currentState.eFlags & EF_FIRING ) ) {
					CG_PositionRotatedEntityOnTag( &gun2, &gun, gun.hModel, "tag_gauntlet" );
					CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );
				}
			}
			else {
				CG_PositionEntityOnTag( &gun2, &gun, gun.hModel, "tag_tongue" );
				CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );
			}
		}
	}


	// add the spinning barrel
	if ( weapon->barrelModel ) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add the flash
	if ( ( weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK )
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash ) {
			return;
		}
	}

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel) {
		return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis( angles, flash.axis );

	// colorize the railgun blast
	if ( weaponNum == WP_RAILGUN ) {
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		flash.shaderRGBA[0] = 255 * ci->color1[0];
		flash.shaderRGBA[1] = 255 * ci->color1[1];
		flash.shaderRGBA[2] = 255 * ci->color1[2];
	}

	CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash");
	trap_R_AddRefEntityToScene( &flash );

	if ( ps || cg.renderingThirdPerson ||
		cent->currentState.number != cg.predictedPlayerState.clientNum ) {

		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, flash.origin );

		// add rail trail
		CG_SpawnRailTrail( cent, flash.origin );

		if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flash.origin, 300 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;

	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin );
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 ) {
		fovOffset = -0.2 * ( cg_fov.integer - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) {
	int		i;
	int		bits;
	int		count;
	int		x, y, w;
	char	*name;
	float	*color;

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	count = 0;
	for ( i = 1 ; i < 16 ; i++ ) {
		if ( bits & ( 1 << i ) ) {
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for ( i = 1 ; i < 16 ; i++ ) {
		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		CG_RegisterWeapon( i );

		// draw weapon icon
		CG_DrawPic( x, y, 32, 32, cg_weapons[i].weaponIcon );

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( x-4, y-4, 40, 40, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
	}

	// draw the selected name
	if ( cg_weapons[ cg.weaponSelect ].item ) {
		name = cg_weapons[ cg.weaponSelect ].item->pickup_name;
		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			x = ( SCREEN_WIDTH - w ) / 2;
			CG_DrawBigStringColor(x, y - 22, name, color);
		}
	}

	trap_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	//PKMOD - Ergodic 06/14/00 handle weapon WP_SHOTGUN, shotgun/exploding shells weapon selection
	if (i == WP_SHOTGUN) {
		if ( (!cg.snap->ps.ammo[WP_SHOTGUN]) && (!cg.snap->ps.ammo[WP_EXPLODING_SHELLS]) ) {
			return qfalse;
		}
	}
//PKMOD - Ergodic 05/18/01 - Airfist has displayable ammo corresponding to airfist_level
//				airfist may have "useable" zero ammo
	else if ( ( !cg.snap->ps.ammo[i] ) && ( i != WP_AIRFIST ) ) {
		return qfalse;
	}

	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;
	//PKMOD - Ergodic 08/16/03 - PKA full weapon cycling
	int max_weapon;

	//PKMOD - Ergodic 08/16/03 - PKA full weapon cycling
	if ( cg_pkafullweaponcycling.integer )
		max_weapon = WP_EXPLODING_SHELLS - 1;
	else
		max_weapon = WP_GRAPPLING_HOOK;
 
	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

//PMMOD - Ergodic 06/10/00 For this type of cycling to work
//			must follow weapon_t in bg_public.h for special
//			PK Weapons. Assume grapple is the highest regular weapon. 
	for ( i = 0 ; i < WP_NUM_WEAPONS; i++ ) {
		if (cg.weaponSelect == WP_EXPLODING_SHELLS) {
			cg.weaponSelect = WP_SHOTGUN + 1;
		}
		else {
			cg.weaponSelect++;
			//PKMOD - Ergodic 08/16/03 - PKA full weapon cycling
			//			if ( cg.weaponSelect > WP_GRAPPLING_HOOK ) {
			if ( cg.weaponSelect > max_weapon ) {
				cg.weaponSelect = 0;
			}
		}
		//PKMOD - Ergodic 08/18/03 - don't cycle to Gauntlet using "normal" weapon cycling
		if ( cg_pkafullweaponcycling.integer != 1 )
			if ( cg.weaponSelect == WP_GAUNTLET ) {
				continue;		// never cycle to gauntlet
			}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			//PKMOD - Ergodic 06/14/00 handle weapon WP_SHOTGUN, shotgun/exploding shells weapon selection
			if (cg.weaponSelect == WP_SHOTGUN) {
				if ( cg.snap->ps.ammo[WP_EXPLODING_SHELLS] ) {
					cg.weaponSelect = WP_EXPLODING_SHELLS;
				}

			} //end if (cg.weaponSelect==WP_SHOTGUN), weapon WP_SHOTGUN selection

			break;
		}
	}

	//PKMOD - Ergodic 04/03/01 - code to fix selecting empty ammo weapon
	if ( i == WP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}

}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;
	//PKMOD - Ergodic 08/18/03 - PKA full weapon cycling
	int max_weapon;

	//PKMOD - Ergodic 08/18/03 - PKA full weapon cycling
	if ( cg_pkafullweaponcycling.integer )
		max_weapon = WP_EXPLODING_SHELLS - 1;
	else
		max_weapon = WP_GRAPPLING_HOOK;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

//PMMOD - Ergodic 06/10/00 For this type of cycling to work
//			must follow weapon_t in bg_public.h for special
//			PK Weapons. Assume grapple is the highest regular weapon. 
	for ( i = 0 ; i < WP_NUM_WEAPONS; i++ ) {
		if (cg.weaponSelect == WP_EXPLODING_SHELLS) {
			cg.weaponSelect = WP_SHOTGUN - 1;
		}
		else {
			cg.weaponSelect--;
			//PMMOD - Ergodic 07/26/01 - fix pka hold cycling
			//PKMOD - Ergodic 08/18/03 - PKA full weapon cycling
			//if ( cg.weaponSelect > WP_GRAPPLING_HOOK ) {
			//	cg.weaponSelect = WP_GRAPPLING_HOOK;
			//}
			//else if ( cg.weaponSelect < 0 ) {
			//	cg.weaponSelect = WP_GRAPPLING_HOOK;
			//}
			if ( cg.weaponSelect > max_weapon ) {
				cg.weaponSelect = max_weapon;
			}
			else if ( cg.weaponSelect < 0 ) {
				cg.weaponSelect = max_weapon;
			}
		}

		//PKMOD - Ergodic 08/18/03 - don't cycle to Gauntlet using "normal" weapon cycling
		if ( cg_pkafullweaponcycling.integer != 1 )
			if ( cg.weaponSelect == WP_GAUNTLET ) {
				continue;		// never cycle to gauntlet
			}

		//PKMOD - Ergodic 04/03/01 - debug "BT & MG bug" (inactive) 
//		Com_Printf("CG_PrevWeapon_f - cg.weaponSelect>%d<, i>%d<\n", cg.weaponSelect, i );

		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			//PKMOD - Ergodic 06/14/00 handle weapon WP_SHOTGUN, shotgun/exploding shells weapon selection
			if (cg.weaponSelect == WP_SHOTGUN) {
				if ( cg.snap->ps.ammo[WP_EXPLODING_SHELLS] ) {
					cg.weaponSelect = WP_EXPLODING_SHELLS;
				}

			} //end if (cg.weaponSelect==WP_SHOTGUN), weapon WP_SHOTGUN selection

			break;
		}
	}

	//PKMOD - Ergodic 04/03/01 - code to fix (BT & MG bug)
//	if ( i == WP_GRAPPLING_HOOK ) {
	if ( i == WP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > 15 ) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	//PKMOD - Ergodic 03/12/02 - check for auto-deploy
	if ( cg.weaponSelect == num ) {
		//PKMOD - Ergodic 10/23/02 - add reset dragon after holding dragon deploy and press "weapon 10"
		if ( (( num >= WP_GRAVITY ) && ( num <= WP_BEANS )) || ( num == WP_GAUNTLET ) || ( num == WP_GRAPPLING_HOOK ) )
			CG_Weapon_DragonDeploy();
		return;
	}

	//PKMOD - Ergodic 06/09/00 handle weapon 1 pka weapons cycling
	//PKMOD - Ergodic 03/12/02 - turn wepon 1 cycling off
	/*---------------begin: weapon 1 cycling ----------------
	if (num == 1) {
		original = cg.weaponSelect;
		cg.weaponSelect = 1;
//PMMOD - Ergodic 06/09/00 For this type of cycling to work
//			must follow weapon_t in bg_public.h for special
//			PK Weapons. This code will cycle from Gauntlet, 
//			Gravity Well, AutoSentry, BearTrap, and Beans.
		if ( (original == 1) || ((original >= WP_GRAVITY) && (original <= WP_BEANS)) ) {
			if ( original == 1 ) {
				original = WP_GRAVITY - 1;
			}

			for ( i = (original + 1) ; i <= WP_BEANS ; i++ ) {
				if (  cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << i) ) {
					cg.weaponSelect = i;  // found the next weapon 1 selection
					break;
				}
			}
		}

		return; // always return from here on weapon 1 selection
	} //end if (num==1), weapon 1 cycling
	---------------end: weapon 1 cycling ----------------*/

	//PKMOD - Ergodic 06/14/00 handle weapon WP_SHOTGUN, shotgun/exploding shells weapon selection
	if (num == WP_SHOTGUN) {
		if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
			return;		// don't have the shotgun weapon
		}
		//we have the shotgun 
		if ( !cg.snap->ps.ammo[WP_EXPLODING_SHELLS] ) {
			cg.weaponSelect = num;
		}
		else {
			cg.weaponSelect = WP_EXPLODING_SHELLS;
		}

		return; // always return from here on weapon WP_SHOTGUN selection
	} //end if (num==WP_SHOTGUN), weapon WP_SHOTGUN selection

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	//PKMOD - Ergodic 06/28/01 - turn off dragon deploy if enabled and selecting dragon
	if ( ( num == WP_GRAPPLING_HOOK ) && ( ( cg.snap->ps.generic1 & 15 ) > 0 ) ) {
		char	command[128];

		Com_sprintf( command, 128, "gdeploy 0" );
		trap_SendClientCommand( command );
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	//PKMOD - Ergodic 07/06/00 handle case of out exploding shells ammo
	//			test wp_shotgun for selectablility first
	if ( cg.weaponSelect == WP_EXPLODING_SHELLS ) {
		if ( CG_WeaponSelectable( WP_SHOTGUN ) ) {
			cg.weaponSelect = WP_SHOTGUN;
			return;
		}
	}

//PKMOD - Ergodic 06/10/00 For this type of cycling to work
//			must follow weapon_t in bg_public.h for special
//			PK Weapons. Assume grapple is the highest regular weapon.
	for ( i = WP_GRAPPLING_HOOK - 1; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}

//PKMOD - Ergodic 07/02/00 weapon commands
/*
===============
CG_CMD_Weapon_Select - select the specified weapon
===============
*/
void CG_CMD_Weapon_Select( int weapon_num ) {
	//PKMOD - Ergodic 04/04/01 - save last weapon (04/05/01 - code done in bg_pmove)
//	int		original_weapon;

	//PKMOD - Ergodic 07/25/01 - debug airfist select (inactive)
//	Com_Printf( "CG_CMD_Weapon_Select - weapon_num:%d, stat:%d  ammo:%d\n", weapon_num, cg.snap->ps.stats[STAT_WEAPONS], cg.snap->ps.stats[STAT_WEAPONS], cg.snap->ps.ammo[weapon_num] );

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

//PKMOD - Ergodic 12/25/00 move to later down in the function
//	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon_num ) ) ) {
		return;		// don't have the weapon
	}

		//PKMOD - Ergodic 06/28/01 - debug (inactive)
//	Com_Printf( "CG_CMD_Weapon_Select - weapon_num:%d, ps.generic1:%d, predictedPlayerState.generic1:%d\n", weapon_num, cg.snap->ps.generic1, cg.predictedPlayerState.generic1 );

	//PKMOD - Ergodic 06/28/01 - turn off dragon deploy if enabled and selecting dragon
	if ( ( weapon_num == WP_GRAPPLING_HOOK ) && ( ( cg.snap->ps.generic1 & 15 ) > 0 ) ) {
		char	command[128];

		Com_sprintf( command, 128, "gdeploy 0" );
		trap_SendClientCommand( command );
		cg.weaponSelectTime = cg.time;
		cg.weaponSelect = weapon_num;
		return;
	}

	//PKMOD - Ergodic 04/04/01 - save last weapon (04/05/01 - code done in bg_pmove)
//	original_weapon = cg.weaponSelect;

	//PKMOD - Ergodic 12/25/00 add logic to not switch if out of ammo
	//PKMOD - Ergodic 06/14/00 handle weapon WP_SHOTGUN, shotgun/exploding shells weapon selection
	if (weapon_num == WP_SHOTGUN) {
		//do we have exploding shells?
		if ( cg.snap->ps.ammo[WP_EXPLODING_SHELLS] > 0 ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = WP_EXPLODING_SHELLS;

			//PKMOD - Ergodic 04/04/01 - save last weapon if weapon number has changed
			//PKMOD - Ergodic 04/05/01 - code done in bg_pmove
//			if ( original_weapon != cg.weaponSelect )
//				cg.weaponLast = original_weapon;

			return;
		}
		//do we have regular shells?
		if ( cg.snap->ps.ammo[WP_SHOTGUN] > 0 ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = WP_SHOTGUN;

			//PKMOD - Ergodic 04/04/01 - save last weapon if weapon number has changed
			//PKMOD - Ergodic 04/05/01 - code done in bg_pmove
//			if ( original_weapon != cg.weaponSelect )
//				cg.weaponLast = original_weapon;
		}

	} //end if (num==WP_SHOTGUN), weapon WP_SHOTGUN selection
	else {
		//all other weapons
//PKMOD - Ergodic 05/18/01 - Airfist has displayable ammo corresponding to airfist_level
//				airfist may have "useable" zero ammo
//PKMOD - Ergodic 07/25/01 - fix airfist selection
//		if ( ( cg.snap->ps.ammo[weapon_num] != 0 ) && ( weapon_num != WP_AIRFIST ) ) {
		if ( ( cg.snap->ps.ammo[weapon_num] != 0 ) || ( weapon_num == WP_AIRFIST ) ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = weapon_num;

			//PKMOD - Ergodic 04/04/01 - save last weapon if weapon number has changed
			//PKMOD - Ergodic 04/05/01 - code done in bg_pmove
//			if ( original_weapon != cg.weaponSelect )
//				cg.weaponLast = original_weapon;
		}
	}
}

/*
===============
CG_LastWeapon_f - select the last wepon held
PKMOD - Ergodic 04/04/01 
===============
*/
void CG_LastWeapon_f( void ) {

	//PKMOD - Ergodic 04/04/01 - debug (inactive)
//	Com_Printf( "CG_LastWeapon_f - current>%d<, last>%d<\n", cg.weaponSelect, cg.snap->ps.stats[ STAT_LAST_WEAPON ] );

	CG_CMD_Weapon_Select( cg.snap->ps.stats[ STAT_LAST_WEAPON ] );

}


/*
===============
CG_Weapon_Gauntlet - select the Gauntlet
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_Gauntlet( void ) {
	CG_CMD_Weapon_Select( WP_GAUNTLET );
}

/*
===============
CG_Weapon_MachineGun - select the MachineGun
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_MachineGun( void ) {
	CG_CMD_Weapon_Select( WP_MACHINEGUN );
}

/*
===============
CG_Weapon_ShotGun - select the ShotGun
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_ShotGun( void ) {
	CG_CMD_Weapon_Select( WP_SHOTGUN );
}

/*
===============
CG_Weapon_AirFist - select the AirFist
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_AirFist( void ) {
	CG_CMD_Weapon_Select( WP_AIRFIST );
}

/*
===============
CG_Weapon_NailGun - select the NailGun
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_NailGun( void ) {
	CG_CMD_Weapon_Select( WP_NAILGUN );
}

/*
===============
CG_Weapon_GrenadeLauncher - select the Grenade Launcher
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_GrenadeLauncher( void ) {
	CG_CMD_Weapon_Select( WP_GRENADE_LAUNCHER );
}

/*
===============
CG_Weapon_RocketLauncher - select the Rocket Launcher
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_RocketLauncher( void ) {
	CG_CMD_Weapon_Select( WP_ROCKET_LAUNCHER );
}

/*
===============
CG_Weapon_LightningGun - select the LightningGun
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_LightningGun( void ) {
	CG_CMD_Weapon_Select( WP_LIGHTNING );
}

/*
===============
CG_Weapon_RailGun - select the RailGun
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_RailGun( void ) {
	CG_CMD_Weapon_Select( WP_RAILGUN );
}

/*
===============
CG_Weapon_Harpoon - select the Harpoon
PKMOD - Ergodic 07/12/00 
===============
*/
void CG_Weapon_Harpoon( void ) {
	CG_CMD_Weapon_Select( WP_GRAPPLING_HOOK );
}

/*
===============
CG_Weapon_GravityWell - select the gravity well
PKMOD - Ergodic 07/02/00 
===============
*/
void CG_Weapon_GravityWell( void ) {
	CG_CMD_Weapon_Select( WP_GRAVITY );
}

/*
===============
CG_Weapon_Sentry - select the autosentry
PKMOD - Ergodic 07/02/00
===============
*/
void CG_Weapon_Sentry( void ) {
	CG_CMD_Weapon_Select( WP_SENTRY );
}
/*
===============
CG_Weapon_Beans - select the beartrap
PKMOD - Ergodic 07/02/00
===============
*/
void CG_Weapon_BearTrap( void ) {
	CG_CMD_Weapon_Select( WP_BEARTRAP );
}

/*
===============
CG_Weapon_Beans - select the can of pork-n-beans
PKMOD - Ergodic 07/02/00
===============
*/
void CG_Weapon_Beans( void ) {
	CG_CMD_Weapon_Select( WP_BEANS );
}


//PKMOD - Ergodic 05/20/00 switch to gauntlet
/*
===================
CG_PKA_OutOfAmmoChange

The current pka weapon has just run out of ammo
===================
*/
void CG_PKA_OutOfAmmoChange( void ) {

	//PKMOD - Ergodic 08/24/01 - debug removal of machinegun when testing infinite ammo bug (inactive)
//	Com_Printf("CG_PKA_OutOfAmmoChange - removing weapon>%d<\n", cg.weaponSelect );

	//PKMOD - Ergodic 06/27/00 physically remove the weapon from inventory
	//PKMOD - Ergodic 03/02/01 - Does this work in the game code (I think not)?
	cg.snap->ps.stats[STAT_WEAPONS] &= ~( 1 << cg.weaponSelect );

	cg.weaponSelectTime = cg.time;
	cg.weaponSelect = 1;
}


//PKMOD - Ergodic 03/01/01 dragon deploy pka weapon 
/*
===================
CG_Weapon_DragonDeploy

Mount the current PKA weapon on the dragon

  if dragon's ammo = -1 then no weapon is being deployed
  else weapon number (- ammo) == the weapon_t enum
===================
*/
void CG_Weapon_DragonDeploy( void ){
	char	command[128];
	int		deployNumber;

	//PKMOD - Ergodic 08/29/01 - debug (inactive)
//	Com_Printf( "CG_Weapon_DragonDeploy - ps.generic1:%d, predictedPlayerState.generic1:%d\n", cg.snap->ps.generic1, cg.predictedPlayerState.generic1 );

	//PKMOD - Ergodic 06/28/01 - turn off dragon deploy if enabled
	if ( ( cg.weaponSelect == WP_GRAPPLING_HOOK ) && ( ( cg.snap->ps.generic1 & 15 ) > 0 ) ) {
		Com_sprintf( command, 128, "gdeploy 0" );
		trap_SendClientCommand( command );
		//set the weapon to the dragon
		cg.weaponSelect = WP_GRAPPLING_HOOK;
		cg.weaponSelectTime = cg.time;
		return;
	}

	//check if player has a dragon in the inventory	
	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << WP_GRAPPLING_HOOK ) ) ) {
		return;		// don't have the weapon
	}

	//PKMOD - Ergodic 05/08/01 - change logic structure to "if" from "switch"
	//PKMOD - Ergodic 08/29/01 - cchange logic to include flag carrying
	//PKMOD - Ergodic 12/15/01 - add gauntlet to deployable weapon list
	//PKMOD - Ergodic 03/12/02 - optimize selction logic
	if ( (( cg.weaponSelect >= WP_GRAVITY ) && ( cg.weaponSelect <= WP_BEANS ) ) || 
		( cg.weaponSelect == WP_GAUNTLET ) ) {
		deployNumber = cg.weaponSelect;

		//PKMOD - Ergodic 12/15/01 - only check the ammo if not the gauntlet
		if ( deployNumber != WP_GAUNTLET ) {

			//PKMOD - Ergodic 07/22/01 - check if player has ammo - to fix infinite ammo bug	
			if ( cg.snap->ps.ammo[ cg.weaponSelect ] < 1 ) {
				return;		// don't have the required ammo
			}
		}
	}
	else {
		//PKMOD - Ergodic 08/29/01 - check if player is carrying the flag
		if ( cg.snap->ps.powerups[PW_BLUEFLAG] )
			deployNumber = PW_BLUEFLAG;
		else if ( cg.snap->ps.powerups[PW_REDFLAG] ) 
				deployNumber = PW_REDFLAG;
			else
				return;		//exit since current weapon or flag can not be deployed

	}

	//++++
	//here if weapon can be deployed
	//++++

	//PKMOD - Ergodic 03/01/01 - debug (inactive)
//	Com_Printf( "CG_Weapon_DragonDeploy - valid deployable weapon\n" );


	//here if weapon can be deployed

	//set the dragon ammo to be the negative of the weapon number
//	cg.predictedPlayerState.ammo[WP_GRAPPLING_HOOK] =  0 - cg.weaponSelect;

	//PKMOD - Ergodic 07/22/01 - debug infinite ammo (inactive)
//	Com_Printf( "CG_Weapon_DragonDeploy - cg.weaponSelect:%d, ammo:%d\n", cg.weaponSelect, cg.snap->ps.ammo[ cg.weaponSelect ] );

	//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//	Com_Printf("CG_Weapon_DragonDeploy - weaponSelect>%d<, ammo>%d<\n", cg.weaponSelect, cg.snap->ps.ammo[cg.weaponSelect] );

	//PKMOD - Ergodic 03/03/01 - set generic1 to the alternate weapon
//	cg.snap->ps.generic1 = cg.weaponSelect;
//	cg.predictedPlayerState.generic1 = cg.weaponSelect;

	//PKMOD - Ergodic 02/20/04 - debug infinite flag bug (inactive)
	//Com_Printf( "CG_Weapon_DragonDeploy - deployNumber>%d<\n", deployNumber );



	Com_sprintf( command, 128, "gdeploy %i", deployNumber );
	trap_SendClientCommand( command );

	//PKMOD - Ergodic 03/02/01 - debug (inactive)
//	Com_Printf( "CG_Weapon_DragonDeploy - setting weapon to %d\n", cg.weaponSelect );

	//set the weapon to the dragon
	cg.weaponSelect = WP_GRAPPLING_HOOK;
	cg.weaponSelectTime = cg.time;
}


/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;


	//PKMOD - Ergodic 10/14/01 - debug radiation effect (inactive)
	//PKMOD - Ergodic 10/20/01 - update to punctuate the radiation effect (inactive)
//	if ( cent->PKA_RadiateTime < cg.time ) {
//		CG_Radiation( cg.snap->ps.origin );
//		cent->PKA_RadiateTime = cg.time + 1000;		//every 1 seconds
//	}

	//PKMOD - Ergodic 08/21/01 - debug infinite ammo bug (inactive)
//	Com_Printf("CG_FireWeapon - firing weapon\n" );

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ ent->weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	//PKMOD - Ergodic 11/12/00 add 200 msecs to the airfist flashtime
	//PKMOD - Ergodic 11/16/00 add multiple flash models depending on airfist_level
	if ( ent->weapon == WP_AIRFIST ) {
		trace_t		tr;
		int sourceContentType;
		vec3_t		af_forward;
		vec3_t		af_end;
		vec3_t		af_muzzle;

		cent->muzzleFlashTime = cg.time + 200;
		//PKMOD - Ergodic 07/07/01 - use new packing scheme to encode airfist level
		switch ( ( cent->currentState.time2 >> 4 ) & 7) {
		case 4:
			weap->flashModel = cgs.media.airfist4FlashModel;
			break;
		case 3:
			weap->flashModel = cgs.media.airfist3FlashModel;
			break;
		case 2:
			weap->flashModel = cgs.media.airfist2FlashModel;
			break;
		case 1:
			weap->flashModel = cgs.media.airfist1FlashModel;
			break;
		default:
			weap->flashModel = cgs.media.airfist0FlashModel;
			break;
		}

		//PKMOD - Ergodic 07/01/01 - determine which airfist sound to play
		VectorCopy( cg.snap->ps.origin, af_muzzle );
		af_muzzle[2] += cg.snap->ps.viewheight;
		//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors( cg.snap->ps.viewangles, af_forward, NULL, NULL );
		AngleVectorsForward( cg.snap->ps.viewangles, af_forward );
		VectorMA( af_muzzle, 14, af_forward, af_muzzle );
		VectorMA( af_muzzle, 10, af_forward, af_end );

		CG_Trace( &tr, af_muzzle, NULL, NULL, af_end, cg.predictedPlayerState.clientNum, MASK_SHOT );
		sourceContentType = trap_CM_PointContents( af_muzzle, 0 );

		// FIXME: should probably move this cruft into CG_BubbleTrail
		if ( sourceContentType & CONTENTS_WATER ) {
			if ( cg.snap->ps.ammo[ WP_AIRFIST ] )
				weap->flashSound[0] = cgs.media.sfx_pkaairfistwaterfire;
			else
				weap->flashSound[0] = cgs.media.sfx_pkaairfistwaterempty;
		}
		else {
			if ( cg.snap->ps.ammo[ WP_AIRFIST ] )
				weap->flashSound[0] = cgs.media.sfx_pkaairfistfire;
			else
				weap->flashSound[0] = cgs.media.sfx_pkaairfistempty;
		}
	

	}
	else
		cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_LIGHTNING ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}
}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	vec3_t			sprOrg;
	vec3_t			sprVel;

	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	//PKMOD - Ergodic 02/14/02 - debug weapon number (inactive)
//	Com_Printf( "CG_MissileHitWall - weapon>%d<\n", weapon );

	switch ( weapon ) {
	default:
#ifdef MISSIONPACK
	case WP_NAILGUN:
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_nghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_nghitmetal;
		} else {
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#endif
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
#endif
	case WP_GRENADE_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
	case WP_ROCKET_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		if (cg_oldRocket.integer == 0) {
			// explosion sprite animation
			VectorMA( origin, 24, dir, sprOrg );
			VectorScale( dir, 64, sprVel );

			CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
		}
		break;
	case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 24;
		break;
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 16;
		break;
	case WP_BFG:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 32;
		isSprite = qtrue;
		break;
	case WP_SHOTGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = 4;
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_chghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_chghitmetal;
		} else {
			sfx = cgs.media.sfx_chghit;
		}
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
#endif

	case WP_MACHINEGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r == 0 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 1 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
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
	case WP_GRAVITY:
		//PKMOD Need shaders and SFX for this soon
//		mark = cgs.media.burnMarkShader;
//		mod = cgs.media.pkagravitylaunched;
//		shader = cgs.media.rocketExplosionShader;
//		sfx = cgs.media.sfx_pkagravitylaunched;


//		mod = cgs.media.dishFlashModel;
//		shader = cgs.media.grenadeExplosionShader;
//		sfx = cgs.media.sfx_rockexp;
//		mark = cgs.media.burnMarkShader;
//		radius = 64;
//		light = 300;
//		isSprite = qtrue;
		break;
	case WP_SENTRY:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = 4;
		break;
	case WP_BEARTRAP:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = cgs.media.sfx_pkabeartrapdrop;
		radius = 4;
		break;
	case WP_CHAINLG:
		//PKMOD rearrange for chain effect
		// no explosion at lg impact, 
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
	case WP_AIRFIST:
        //PKMOD probably remove mark altogether. Maybe add air mark
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
	case WP_NAILGUN:
		//PKMOD - Ergodic 08/01/00 set nailgun up correctly
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.nailImpactShader;
		mark = cgs.media.nailMarkShader;

		//PKMOD - Ergodic 08/25/00 add new nailgun ricochet sounds
		r = rand() & 63; //bias against sound #2
		if ( r == 0 ) {
			sfx = cgs.media.sfx_nailrico2;
		} else if ( r < 21 ) {
			sfx = cgs.media.sfx_nailrico1;
		} else if ( r < 41 ) {
			sfx = cgs.media.sfx_nailrico3;
		} else {
			sfx = cgs.media.sfx_nailrico4;
		}

		radius = 8;
		break;
//	case WP_NAILGUN + 128:
//		//PKMOD - Ergodic 08/11/00 set nailgun hitting player
//		mod = cgs.media.bulletFlashModel;
//		shader = cgs.media.nailImpactShader;
//		mark = cgs.media.nailMarkShader;
//
//		r = rand() & 3;
//		if ( r < 2 ) {
//			sfx = cgs.media.sfx_ric1;
//		} else if ( r == 2 ) {
//			sfx = cgs.media.sfx_ric2;
//		} else {
//			sfx = cgs.media.sfx_ric3;
//		}
//		duration = 250; 
//		radius = 8;
//		break;
	case WP_EXPLODING_SHELLS:
		mod = cgs.media.explshellsFlashModel;

		//PKMOD - Ergodic 01/16/01 - add multi-shaders for exploding shells
		switch ( rand() % 6 ) {	//random numbers: { 0, 1, 2, 3, 4, 5 }
			case 0:
				shader = cgs.media.shellsExplosionShader1;
				break;
			case 1:
				shader = cgs.media.shellsExplosionShader2;
				break;
			case 2:
				shader = cgs.media.shellsExplosionShader3;
				break;
			case 3:
				shader = cgs.media.shellsExplosionShader4;
				break;
			case 4:
				shader = cgs.media.shellsExplosionShader5;
				break;
			default:
				shader = cgs.media.shellsExplosionShader6;
				break;
		}

		//PKMOD - Ergodic 02/14/02 - explosive shells hit sounds
		//PKMOD - Ergodic 07/10/02 - add 2 more explosive shells hit sounds
		switch ( rand() % 4 ) {		//cases: 0, 1, 2, 3, 4
			case	0:
				sfx = cgs.media.sfx_expgunhit1;
				break;
			case	1:
				sfx = cgs.media.sfx_expgunhit2;
				break;
			case	2:
				sfx = cgs.media.sfx_expgunhit3;
				break;
			case	3:
				sfx = cgs.media.sfx_expgunhit4;
				break;
			default:
				sfx = cgs.media.sfx_expgunhit5;
				break;
		}

		mark = cgs.media.burnMarkShader;
		radius = 16;
		light = 25;
		//PKMOD - Ergodic 01/16/01 not a sprite anymore because sprite size seems too big
//		isSprite = qtrue;
		duration = 700;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;

	}
	//PKMOD

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir, 
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
		if ( weapon == WP_RAILGUN ) {
			// colorize with client color
			VectorCopy( cgs.clientinfo[clientNum].color1, le->color );
		}
	}

	//
	// impact mark
	//
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	if ( weapon == WP_RAILGUN ) {
		float	*color;

		// colorize with client color
		color = cgs.clientinfo[clientNum].color2;
		CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse );
	} else {
		CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse );
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum ) {
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
//	//PKMOD - Ergodic 08/11/00 - add impact on player from nail
//            commentary - this code does not work visually, due to player movements
//				- and will be removed
//	case WP_NAILGUN:
//		CG_MissileHitWall( weapon + 128, 0, origin, dir ); //Weapon hack
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
		break;
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
//PKMOD - Ergodic 06/18/00 change the call parameters to add in weapon argument 
static void CG_ShotgunPellet( vec3_t start, vec3_t end, int skipNum, int weapon ) {
	trace_t		tr;
	int sourceContentType, destContentType;

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	sourceContentType = trap_CM_PointContents( start, 0 );
	destContentType = trap_CM_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( start, tr.endpos, 32 );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( start, trace.endpos, 32 );
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, 32 );
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	//PKMOD - Ergodic 06/18/00 change the call argument from WP_SHOTGUN to weapon 
	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( weapon, tr.endpos, tr.plane.normal, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if ( tr.surfaceFlags & SURF_METALSTEPS ) {
			//PKMOD - Ergodic 06/18/00 change the call argument from WP_SHOTGUN to weapon
			//PKMOD - Ergodic 01/16/01 - fixed missing exploding shells graphic
			CG_MissileHitWall( weapon, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL );
		} else {
			//PKMOD - Ergodic 06/18/00 change the call argument from WP_SHOTGUN to weapon 
			//PKMOD - Ergodic 01/16/01 - fixed missing exploding shells graphic
			CG_MissileHitWall( weapon, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
//PKMOD - Ergodic 06/18/00 change the call parameters to add in weapon argument 
//PKMOD - Ergodic 10/31/02 - Q3A Ver 1.32 change the call parameters to add in seed 
static void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum, int weapon ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;
	//PKMOD - Ergodic 01/16/01 make the exploding shells tighter in spread 
	int			spread;
	int			count;
	//PKMOD - Ergodic 01/20/01 - debug shotgun origin
//	vec3_t		backward;
	//PKMOD - Ergodic 01/22/01 - optimize the end location
	vec3_t		hold_end;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );
	//PKMOD - Ergodic 01/16/01 make the exploding shells tighter in spread 
	//PKMOD - Ergodic 01/18/01 make the pellet counts equal
	if ( weapon == WP_EXPLODING_SHELLS ) {
		spread = DEFAULT_SHOTGUN_SPREAD * 12;
		count = DEFAULT_SHOTGUN_COUNT;
		VectorMA( origin, 8192 * 12, forward, hold_end);
	}
	else {
		spread = DEFAULT_SHOTGUN_SPREAD * 16;	//default spread
		count = DEFAULT_SHOTGUN_COUNT;
		VectorMA( origin, 8192 * 16, forward, hold_end);
	}

	//PKMOD - Ergodic 01/20/01 - debug shotgun origin
//	VectorSet(backward, 0,0,0);
//	VectorMA(backward, -1, forward, backward);
//	CG_MissileHitWall( weapon, 0, origin, backward, IMPACTSOUND_METAL );

	// generate the "random" spread pattern
	for ( i = 0 ; i < count ; i++ ) {
	//PKMOD - Ergodic Debug - only 1 pellet
//	for ( i = 0 ; i < 1 ; i++ ) {
		//PKMOD - Ergodic 01/16/01 make the exploding shells tighter in spread 
		r = Q_crandom( &seed ) * spread;
		u = Q_crandom( &seed ) * spread;

		VectorCopy( hold_end, end );
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum, weapon );
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
	vec3_t	v;
	int		contents;

	VectorSubtract( es->origin2, es->pos.trBase, v );
	VectorNormalize( v );
	VectorScale( v, 32, v );
	VectorAdd( es->pos.trBase, v, v );
	if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t			up;

		contents = trap_CM_PointContents( es->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) ) {
			VectorSet( up, 0, 0, 8 );
			CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}

//PKMOD - Ergodic 06/18/00 change the call parameters to add in weapon argument 
//PKMOD - Ergodic 10/31/02 - Q3A Ver 1.32 change the call parameters to add in es->eventParm as the seed 
	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum, es->weapon );
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
//PKMOD - Ergodic 03/26/01 - add autosentry into CG_Tracer, autosentrys produce
//			larger traces   
void CG_Tracer( vec3_t source, vec3_t dest, qboolean autosentry ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;
	//PKMOD - Ergodic 03/26/01 - add autosentry tracer hold values
	float		pkatracerLength, pkatracerWidth;

	if ( autosentry ) {
		pkatracerLength = 150.0f;		//default = 100.0
		pkatracerWidth = 2.5f;		//default = 1.0
	}
	else {
		pkatracerLength = cg_tracerLength.value;
		pkatracerWidth = cg_tracerWidth.value;
	}

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + pkatracerLength;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, pkatracerWidth, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -pkatracerWidth, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -pkatracerWidth, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, pkatracerWidth, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		AngleVectorsForward( cg.snap->ps.viewangles, forward );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	AngleVectorsForward( cent->currentState.apos.trBase, forward );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, qboolean autosentry ) {
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			sourceContentType = trap_CM_PointContents( start, 0 );
			destContentType = trap_CM_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, 32 );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( start, trace.endpos, 32 );
			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( end, trace.endpos, 32 );
			}

			// draw a tracer
			//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
			if ( autosentry ) {
				if ( random() < 0.90 ) {
					//PKMOD - Ergodic 03/26/01 - add autosentry into CG_Tracer  
					CG_Tracer( start, end, autosentry );
				}
			}
			else if ( random() < cg_tracerChance.value ) {
				//PKMOD - Ergodic 03/26/01 - add autosentry into CG_Tracer  
				CG_Tracer( start, end, autosentry );
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
		CG_Bleed( end, fleshEntityNum );
	} else {
		CG_MissileHitWall( WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
	}

}


