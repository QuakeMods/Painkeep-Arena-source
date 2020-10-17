// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"


//PKMOD - Ergodic debug position
char	*CG_vtos( const vec3_t v ) {
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
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

//AxisClear( entity->axis );
	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		trap_S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	//PKMOD - Ergodic 11/19/02 - add code to work-around loopSound Bug (does not carry over to cgame)
	if ( cent->currentState.time2 && ( cent->currentState.eType == ET_MOVER ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
			cgs.gameSounds[ cent->currentState.time2 ] );
	}
	// add loop sound
	else {
		if ( cent->currentState.loopSound ) {
			if (cent->currentState.eType != ET_SPEAKER) {
				trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
					cgs.gameSounds[ cent->currentState.loopSound ] );
			} else {
				trap_S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
					cgs.gameSounds[ cent->currentState.loopSound ] );
			}
		}
	}


	// constant light glow
	if ( cent->currentState.constantLight ) {
		int		cl;
		int		i, r, g, b;

		cl = cent->currentState.constantLight;
		r = cl & 255;
		g = ( cl >> 8 ) & 255;
		b = ( cl >> 16 ) & 255;
		i = ( ( cl >> 24 ) & 255 ) * 4;
		trap_R_AddLightToScene( cent->lerpOrigin, i, r, g, b );
	}

}


/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.gameModels[s1->modelindex];

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
	if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < cent->miscTime ) {
		return;
	}

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
	float			scale;
	weaponInfo_t	*wi;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) {
		CG_Error( "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
		return;
	}

	item = &bg_itemlist[ es->modelindex ];
	if ( cg_simpleItems.integer && item->giType != IT_TEAM ) {
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.radius = 14;

		//PKMOD - Ergodic 11/16/01 - add logic to display radiation effect for simple item
		if ( es->modelindex2 & PKAEF_IRRADIATED ) {
			if ( cent->PKA_RadiateTime < cg.time ) {
				switch (rand() % 3) {	  //Generate random numbers: {0,1,2}
					case 0:
						cent->PKA_customShader = cgs.media.radiate1SimpleIcon;
						break;
					case 1: 
						cent->PKA_customShader = cgs.media.radiate2SimpleIcon;
						break;
					default:
						cent->PKA_customShader = cgs.media.radiate3SimpleIcon;
						break;
				}
				cent->PKA_RadiateTime = cg.time + 2000;		//every 1 seconds
				ent.customShader = cent->PKA_customShader;
			}
			else	if ( ( cent->PKA_RadiateTime - cg.time ) > 1000 )
						ent.customShader = cent->PKA_customShader;
					else
						ent.customShader = cg_items[es->modelindex].icon;
		}
		else		// item is not radiated
			ent.customShader = cg_items[es->modelindex].icon;

		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent);
		return;
	}

	//PKMOD - Ergodic 10/29/01 - add logic to display radiation effect
	if ( es->modelindex2 & PKAEF_IRRADIATED ) {
		if ( cent->PKA_RadiateTime < cg.time ) {
			CG_Radiation( cent->lerpOrigin );
			cent->PKA_RadiateTime = cg.time + 1000;		//every 1 seconds
		}
	}

	// items bob up and down continuously
	scale = 0.005 + cent->currentState.number * 0.00001;
	cent->lerpOrigin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;

	memset (&ent, 0, sizeof(ent));

	//PKMOD - Ergodic 12/03/01 - add custom skins for Private Bot
	if ( item->giType == IT_HOLDABLE ) {
		switch ( item->giTag ) {
		case HI_BOTLEGS:		
			ent.customSkin = cgs.media.privatebot_legsSkin;
			//PKMOD - Ergodic 12/01/01 - raise the bot legs by several units
			cent->lerpOrigin[2] += 22;
			break;
		case HI_BOTTORSO:
			ent.customSkin = cgs.media.privatebot_torsoSkin;
			break;
		case HI_BOTHEAD:
			ent.customSkin = cgs.media.privatebot_headSkin;
			break;
		default:
			break;
		}
	}

	//PKMOD - Ergodic 06/01/02 - set the frame for the autosentry
	//			autosentry frames are 0..29
	if (( item->giType == IT_WEAPON ) && ( item->giTag == WP_SENTRY )) {
		ent.frame = 29;
	}

	//PKMOD - Ergodic 01/27/02 - we will do the rotation function later
	// autorotate at one of two speeds
//	if ( item->giType == IT_HEALTH ) {
//		VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
//		AxisCopy( cg.autoAxisFast, ent.axis );
//	} else {
//		VectorCopy( cg.autoAngles, cent->lerpAngles );
//		AxisCopy( cg.autoAxis, ent.axis );
//	}

	wi = NULL;

	//PKMOD - Ergodic 09/21/00 - add logic so that voting entity will not rotate
	if ( item->giType == IT_VOTING ) {
		//PKMOD - Ergodic 09/28/00 - debug set angle to 45
//		VectorSet( cent->lerpAngles, 0, 45, 0 );
//		Com_Printf("CG_Item: IT_VOTING - cent->lerpAngles>%s<\n", CG_vtos(cent->lerpAngles));

//		AxisClear( ent.axis );

		// convert lerpangles into axis
		AnglesToAxis( cent->lerpAngles, ent.axis );
		
		//PKMOD - Ergodic 09/29/00 - debug 
//		Com_Printf("CG_Item - cent->currentState.otherEntityNum>%d<, hubInfo[cent->currentState.otherEntityNum].map_shader_index>%d<\n", cent->currentState.otherEntityNum, hubInfo[cent->currentState.otherEntityNum].map_shader_index );

		//PKMOD - Ergodic 09/21/00 - Set the voting image shader
		//PKMOD - Ergodic 10/10/00 - use otherEntityNum2 as flag for registering shader
		switch ( cent->currentState.otherEntityNum ) {
		case 0:
			if ( cg_voting_shader_flag[0] == '0' ) {
				cgs.media.voting_levelshot_0 = trap_R_RegisterShaderNoMip( "voting_levelshot_0" );
				cg_voting_shader_flag[0] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_0;
			break;
		case 1:
			if ( cg_voting_shader_flag[1] == '0' ) {
				cgs.media.voting_levelshot_1 = trap_R_RegisterShaderNoMip( "voting_levelshot_1" );
				cg_voting_shader_flag[1] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_1;
			break;
		case 2:
			if ( cg_voting_shader_flag[2] == '0' ) {
				cgs.media.voting_levelshot_2 = trap_R_RegisterShaderNoMip( "voting_levelshot_2" );
				cg_voting_shader_flag[2] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_2;
			break;
		case 3:
			if ( cg_voting_shader_flag[3] == '0' ) {
				cgs.media.voting_levelshot_3 = trap_R_RegisterShaderNoMip( "voting_levelshot_3" );
				cg_voting_shader_flag[4] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_3;
			break;
		case 4:
			if ( cg_voting_shader_flag[4] == '0' ) {
				cgs.media.voting_levelshot_4 = trap_R_RegisterShaderNoMip( "voting_levelshot_4" );
				cg_voting_shader_flag[4] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_4;
			break;
		case 5:
			if ( cg_voting_shader_flag[5] == '0' ) {
				cgs.media.voting_levelshot_5 = trap_R_RegisterShaderNoMip( "voting_levelshot_5" );
				cg_voting_shader_flag[5] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_5;
			break;
		case 6:
			if ( cg_voting_shader_flag[6] == '0' ) {
				cgs.media.voting_levelshot_6 = trap_R_RegisterShaderNoMip( "voting_levelshot_6" );
				cg_voting_shader_flag[6] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_6;
			break;
		case 7:
			//PKMOD - Ergodic 10/10/00 - debug  
//			Com_Printf("CG_Item - case 7 cg_voting_shader_flag[7]>%c<\n", cg_voting_shader_flag[7] );

			if ( cg_voting_shader_flag[7] == '0' ) {
				//PKMOD - Ergodic 10/10/00 - debug  
//				Com_Printf("CG_Item - setting cg_voting_shader_flag[7]\n" );
				cgs.media.voting_levelshot_7 = trap_R_RegisterShaderNoMip( "voting_levelshot_7" );
				cg_voting_shader_flag[7] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_7;
			break;
		case 8:
			if ( cg_voting_shader_flag[8] == '0' ) {
				cgs.media.voting_levelshot_8 = trap_R_RegisterShaderNoMip( "voting_levelshot_8" );
				cg_voting_shader_flag[8] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_8;
			break;
		case 9:
			if ( cg_voting_shader_flag[9] == '0' ) {
				cgs.media.voting_levelshot_9 = trap_R_RegisterShaderNoMip( "voting_levelshot_9" );
				cg_voting_shader_flag[9] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_9;
			break;
		case 10:
			if ( cg_voting_shader_flag[10] == '0' ) {
				cgs.media.voting_levelshot_10 = trap_R_RegisterShaderNoMip( "voting_levelshot_10" );
				cg_voting_shader_flag[10] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_10;
			break;
		case 11:
			if ( cg_voting_shader_flag[11] == '0' ) {
				cgs.media.voting_levelshot_11 = trap_R_RegisterShaderNoMip( "voting_levelshot_11" );
				cg_voting_shader_flag[11] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_11;
			break;
		case 12:
			if ( cg_voting_shader_flag[12] == '0' ) {
				cgs.media.voting_levelshot_12 = trap_R_RegisterShaderNoMip( "voting_levelshot_12" );
				cg_voting_shader_flag[12] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_12;
			break;
		case 13:
			if ( cg_voting_shader_flag[13] == '0' ) {
				cgs.media.voting_levelshot_13 = trap_R_RegisterShaderNoMip( "voting_levelshot_13" );
				cg_voting_shader_flag[13] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_13;
			break;
		case 14:
			if ( cg_voting_shader_flag[14] == '0' ) {
				cgs.media.voting_levelshot_14 = trap_R_RegisterShaderNoMip( "voting_levelshot_14" );
				cg_voting_shader_flag[14] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_14;
			break;
		case 15:
			if ( cg_voting_shader_flag[15] == '0' ) {
				cgs.media.voting_levelshot_15 = trap_R_RegisterShaderNoMip( "voting_levelshot_15" );
				cg_voting_shader_flag[15] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_15;
			break;
		case 16:
			if ( cg_voting_shader_flag[16] == '0' ) {
				cgs.media.voting_levelshot_16 = trap_R_RegisterShaderNoMip( "voting_levelshot_16" );
				cg_voting_shader_flag[16] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_16;
			break;
		case 17:
			if ( cg_voting_shader_flag[17] == '0' ) {
				cgs.media.voting_levelshot_17 = trap_R_RegisterShaderNoMip( "voting_levelshot_17" );
				cg_voting_shader_flag[17] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_17;
			break;
		case 18:
			if ( cg_voting_shader_flag[18] == '0' ) {
				cgs.media.voting_levelshot_18 = trap_R_RegisterShaderNoMip( "voting_levelshot_18" );
				cg_voting_shader_flag[18] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_18;
			break;
		case 19:
			if ( cg_voting_shader_flag[19] == '0' ) {
				cgs.media.voting_levelshot_19 = trap_R_RegisterShaderNoMip( "voting_levelshot_19" );
				cg_voting_shader_flag[19] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_19;
			break;
		case 20:
			if ( cg_voting_shader_flag[20] == '0' ) {
				cgs.media.voting_levelshot_20 = trap_R_RegisterShaderNoMip( "voting_levelshot_20" );
				cg_voting_shader_flag[20] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_20;
			break;
		case 21:
			if ( cg_voting_shader_flag[21] == '0' ) {
				cgs.media.voting_levelshot_21 = trap_R_RegisterShaderNoMip( "voting_levelshot_21" );
				cg_voting_shader_flag[21] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_21;
			break;
		case 22:
			if ( cg_voting_shader_flag[22] == '0' ) {
				cgs.media.voting_levelshot_22 = trap_R_RegisterShaderNoMip( "voting_levelshot_22" );
				cg_voting_shader_flag[22] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_22;
			break;
		case 23:
			if ( cg_voting_shader_flag[23] == '0' ) {
				cgs.media.voting_levelshot_23 = trap_R_RegisterShaderNoMip( "voting_levelshot_23" );
				cg_voting_shader_flag[23] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_23;
			break;
		case 24:
			if ( cg_voting_shader_flag[24] == '0' ) {
				cgs.media.voting_levelshot_24 = trap_R_RegisterShaderNoMip( "voting_levelshot_24" );
				cg_voting_shader_flag[24] = '1';
			}
			ent.customShader = cgs.media.voting_levelshot_24;
			break;
		default:
			ent.customShader = trap_R_RegisterShaderNoMip( "menu/art/unknownmap" );
			break;
		}

//		ent.hModel = cg_items[es->modelindex].models[0];

//		VectorCopy( cent->lerpOrigin, ent.origin);
//		VectorCopy( cent->lerpOrigin, ent.oldorigin);

		// add to refresh list
//		trap_R_AddRefEntityToScene(&ent);
//		return;
	}
	else {
		// autorotate at one of two speeds
		if ( item->giType == IT_HEALTH ) {
			VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
			AxisCopy( cg.autoAxisFast, ent.axis );
		//PKMOD - Ergodic 01/15/04 - change logic to "else if" for addition of code to rotate ammo slowly
		} else if ( ( item->giType == IT_HOLDABLE ) && ( item->giTag == HI_RADIATE ) ) {
			//PKMOD - Ergodic 01/27/02 - rotate the radiate pickup model very slowly
				VectorCopy( cg.autoAnglesSlow, cent->lerpAngles );
				AxisCopy( cg.autoAxisSlow, ent.axis );
		} else if ( item->giType == IT_AMMO ) {		//default rotation
				//PKMOD - Ergodic 01/15/04 - add code to rotate ammo slowly
				VectorCopy( cg.autoAnglesSlow, cent->lerpAngles );
				AxisCopy( cg.autoAxisSlow, ent.axis );
		} else {
				VectorCopy( cg.autoAngles, cent->lerpAngles );
				AxisCopy( cg.autoAxis, ent.axis );
		}

	}

	wi = NULL;
	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
	if ( item->giType == IT_WEAPON ) {
		wi = &cg_weapons[item->giTag];
		cent->lerpOrigin[0] -= 
			wi->weaponMidpoint[0] * ent.axis[0][0] +
			wi->weaponMidpoint[1] * ent.axis[1][0] +
			wi->weaponMidpoint[2] * ent.axis[2][0];
		cent->lerpOrigin[1] -= 
			wi->weaponMidpoint[0] * ent.axis[0][1] +
			wi->weaponMidpoint[1] * ent.axis[1][1] +
			wi->weaponMidpoint[2] * ent.axis[2][1];
		cent->lerpOrigin[2] -= 
			wi->weaponMidpoint[0] * ent.axis[0][2] +
			wi->weaponMidpoint[1] * ent.axis[1][2] +
			wi->weaponMidpoint[2] * ent.axis[2][2];

		cent->lerpOrigin[2] += 8;	// an extra height boost
	}

	//PKMOD - Ergodic 03/05/01 - CLG has different pickup model than handhold model (inactive)
	//PKMOD - Ergodic 03/27/01 - code was inactivated due to CLG will not have a rotating barrel
//	if ( ( item->giTag == WP_LIGHTNING ) && ( item->giType == IT_WEAPON ) ) 
//		ent.hModel = cg_items[es->modelindex].models[1];
//	else
//		ent.hModel = cg_items[es->modelindex].models[0];

	ent.hModel = cg_items[es->modelindex].models[0];

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	if ( msec >= 0 && msec < ITEM_SCALEUP_TIME ) {
		frac = (float)msec / ITEM_SCALEUP_TIME;
		VectorScale( ent.axis[0], frac, ent.axis[0] );
		VectorScale( ent.axis[1], frac, ent.axis[1] );
		VectorScale( ent.axis[2], frac, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	} else {
		frac = 1.0;
	}

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if ( ( item->giType == IT_WEAPON ) ||
		 ( item->giType == IT_ARMOR ) ) {
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
	if ( item->giType == IT_WEAPON ) {
		VectorScale( ent.axis[0], 1.5, ent.axis[0] );
		VectorScale( ent.axis[1], 1.5, ent.axis[1] );
		VectorScale( ent.axis[2], 1.5, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
#ifdef MISSIONPACK
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.weaponHoverSound );
#endif
	}

#ifdef MISSIONPACK
	if ( item->giType == IT_HOLDABLE && item->giTag == HI_KAMIKAZE ) {
		VectorScale( ent.axis[0], 2, ent.axis[0] );
		VectorScale( ent.axis[1], 2, ent.axis[1] );
		VectorScale( ent.axis[2], 2, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	}
#endif

	//PKMOD - Ergodic 05/19/02 - resize the Private Bot (head) to make it larger
	if ( item->giType == IT_HOLDABLE && item->giTag == HI_BOTHEAD ) {
		VectorScale( ent.axis[0], 2, ent.axis[0] );
		VectorScale( ent.axis[1], 2, ent.axis[1] );
		VectorScale( ent.axis[2], 2, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	//PKMOD - Ergodic 05/16/03 - add blade to "pickup" lightning gun model
	if ( ( item->giType == IT_WEAPON ) && ( item->giTag == WP_LIGHTNING ) ) {
		refEntity_t	barrel;

		memset( &barrel, 0, sizeof( barrel ) );

		barrel.hModel = wi->barrelModel;

		VectorCopy( ent.lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = ent.shadowPlane;
		barrel.renderfx = ent.renderfx;

		CG_PositionRotatedEntityOnTag( &barrel, &ent, wi->weaponModel, "tag_barrel" );

		AxisCopy( ent.axis, barrel.axis );
		barrel.nonNormalizedAxes = ent.nonNormalizedAxes;

		trap_R_AddRefEntityToScene( &barrel );
	}


#ifdef MISSIONPACK
	if ( item->giType == IT_WEAPON && wi->barrelModel ) {
		refEntity_t	barrel;

		memset( &barrel, 0, sizeof( barrel ) );

		barrel.hModel = wi->barrelModel;

		VectorCopy( ent.lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = ent.shadowPlane;
		barrel.renderfx = ent.renderfx;

		CG_PositionRotatedEntityOnTag( &barrel, &ent, wi->weaponModel, "tag_barrel" );

		AxisCopy( ent.axis, barrel.axis );
		barrel.nonNormalizedAxes = ent.nonNormalizedAxes;

		trap_R_AddRefEntityToScene( &barrel );
	}
#endif

	// accompanying rings / spheres for powerups
	if ( !cg_simpleItems.integer ) 
	{
		vec3_t spinAngles;

		VectorClear( spinAngles );

		if ( item->giType == IT_HEALTH || item->giType == IT_POWERUP )
		{
			if ( ( ent.hModel = cg_items[es->modelindex].models[1] ) != 0 )
			{
				if ( item->giType == IT_POWERUP )
				{
					ent.origin[2] += 12;
					spinAngles[1] = ( cg.time & 1023 ) * 360 / -1024.0f;
				}
				AnglesToAxis( spinAngles, ent.axis );
				
				// scale up if respawning
				if ( frac != 1.0 ) {
					VectorScale( ent.axis[0], frac, ent.axis[0] );
					VectorScale( ent.axis[1], frac, ent.axis[1] );
					VectorScale( ent.axis[2], frac, ent.axis[2] );
					ent.nonNormalizedAxes = qtrue;
				}
				trap_R_AddRefEntityToScene( &ent );
			}
		}
	}
}

//============================================================================

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;
//	int	col;

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}
/*
	if ( cent->currentState.modelindex == TEAM_RED ) {
		col = 1;
	}
	else if ( cent->currentState.modelindex == TEAM_BLUE ) {
		col = 2;
	}
	else {
		col = 0;
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[col][0], weapon->missileDlightColor[col][1], weapon->missileDlightColor[col][2] );
	}
*/
	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	if ( cent->currentState.weapon == WP_PLASMAGUN ) {
		ent.reType = RT_SPRITE;
		ent.radius = 16;
		ent.rotation = 0;
		ent.customShader = cgs.media.plasmaBallShader;
		trap_R_AddRefEntityToScene( &ent );
		return;
	}

	//PKMOD - Ergodic 06/12/02 - add logic personalsentry missile sprite [time2 == 251]
	//PKMOD - Ergodic 04/06/01 - add autosentry missile sprite
	if ( cent->currentState.weapon == WP_SENTRY ) {
		ent.reType = RT_SPRITE;
		ent.rotation = 0;
		if ( cent->currentState.time2 == 251 ) {
			//personalsentry missile details
			if ( ( rand() % 101 ) > 50 )
				ent.radius = 8;
			else
				ent.radius = 2;
			ent.customShader = cgs.media.personalsentryBallShader;
		} 
		else {
			//autosentry missile details
			if ( ( rand() % 101 ) > 96 )
				ent.radius = 16;
			else
				ent.radius = 2;
			ent.customShader = cgs.media.autosentryBallShader;
		}
		trap_R_AddRefEntityToScene( &ent );
		return;
	}


	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

#ifdef MISSIONPACK
	if ( cent->currentState.weapon == WP_PROX_LAUNCHER ) {
		if (s1->generic1 == TEAM_BLUE) {
			ent.hModel = cgs.media.blueProxMine;
		}
	}
#endif

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	//PKMOD - Ergodic 04/27/01 - make launched gravity well spin
	if ( ( s1->pos.trType != TR_STATIONARY ) || ( cent->currentState.weapon == WP_GRAVITY ) ) {
		RotateAroundDirection( ent.axis, cg.time / 4 );
	} else {
#ifdef MISSIONPACK
		if ( s1->weapon == WP_PROX_LAUNCHER ) {
			AnglesToAxis( cent->lerpAngles, ent.axis );
		}
		else
#endif
		{
			RotateAroundDirection( ent.axis, s1->time );
		}
	}

	// add to refresh list, possibly with quad glow
	CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
}

/*
===============
CG_Grapple

This is called when the grapple is sitting up against the wall
===============
*/
static void CG_Grapple( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

#if 0 // FIXME add grapple pull sound here..?
	// add missile sound
	if ( weapon->missileSound ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->missileSound );
	}
#endif

	// Will draw cable if needed
	CG_GrappleTrail ( cent, weapon );

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	//PKMOD Ergodic debug 06/29/01 (inactive)
//	if ( (rand() % 101) > 97 )
//		Com_Printf("CG_mover - eType>%d, cent->lerpAngles>%s<, cent->lerpOrigin>%s<, generic1>%d<\n", cent->currentState.eType, CG_vtos(cent->lerpAngles), CG_vtos(cent->lerpOrigin), cent->currentState.generic1);

		//PKMOD - Ergodic 11/19/02 - debug mover sound (inactive)
//	if ( cent->currentState.time2 ) {
//			Com_Printf( "CG_Mover - mover soundindex: %d (time2: %d) at lerp>%s<, vec3>%s<\n", cent->currentState.loopSound, cent->currentState.time2, CG_vtos(cent->lerpOrigin), CG_vtos(vec3_origin) ); 
//	}

	//PKMOD - Ergodic 06/11/01 - if zombie mover...
	if ( cent->currentState.generic1 ) {
		//PKMOD - Ergodic 06/11/01 - for zombie
		refEntity_t		legs;
		refEntity_t		torso;
		refEntity_t		head;
		int				frame; 

		memset( &legs, 0, sizeof(legs) );
		memset( &torso, 0, sizeof(torso) );
		memset( &head, 0, sizeof(head) );

		frame = ( (cg.time - s1->apos.trTime) / 1000 ) % 31 + 30;

		//PKMOD Ergodic debug 06/20/01 (inactive)
//		Com_Printf("CG_mover - zombie frame>%d<\n", frame);


		VectorCopy( cent->lerpOrigin, legs.origin);
		VectorCopy( cent->lerpOrigin, legs.oldorigin);
		AnglesToAxis( cent->lerpAngles, legs.axis );

		// legs
		legs.customSkin = cgs.media.pkazombie_legsSkin;
		legs.hModel = cgs.media.pkazombie_legsModel;
		legs.frame = frame;
		trap_R_AddRefEntityToScene(&legs);

		// torso
		AnglesToAxis( cent->lerpAngles, torso.axis );
		torso.customSkin = cgs.media.pkazombie_torsoSkin;
		torso.hModel = cgs.media.pkazombie_torsoModel;
		torso.frame = frame;

		CG_PositionRotatedEntityOnTag( &torso, &legs, legs.hModel, "tag_torso");
		trap_R_AddRefEntityToScene(&torso);

		// head
		AnglesToAxis( cent->lerpAngles, head.axis );
		head.customSkin = cgs.media.pkazombie_headSkin;
		head.hModel = cgs.media.pkazombie_headModel;
		head.frame = frame;

		CG_PositionRotatedEntityOnTag( &head, &torso, torso.hModel, "tag_head");
		trap_R_AddRefEntityToScene(&head);

		return;
	}

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the secondary model
	if ( s1->modelindex2 ) {
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent);
	}

}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->powerups;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
==================
PKMOD - CG_BearTrap_Follow, Ergodic 05/30/00 modeled from CG_BearTrap
PKMOD - Ergodic 06/29/00 - modify code to be called by cg_player
PKMOD - Ergodic 07/01/00 - Add blood trails and random displacement
==================
*/
#define	MAX_BEARTRAPS_VIEWABLE		3
#define	MAX_BEARTRAP_DISPLACEMENT	3
#define	BEARTRAP_BLOOD_FREQUENCY	90


static void CG_BeartTrap_BloodTrail( centity_t *cent ) {
	localEntity_t	*blood;
	vec3_t			origin;

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 8;

	//PKMOD - Ergodic 12/01/01 - debug vec3_origin (inactive)
//	Com_Printf("CG_BearTraps_BloodTrail - vec3_origin>%s<\n", CG_vtos(vec3_origin));

	blood = CG_SmokePuff( origin, vec3_origin, 
					  20,		// radius
					  1, 1, 1, 1,	// color
					  800,		// trailTime
					  cg.time,		// startTime
					  0,			//12/16/00 - add fadeInTime
					  0,		// flags
					  cgs.media.bloodTrailShader );

	// use the optimized version
	blood->leType = LE_FALL_SCALE_FADE;
	// drop a total of 40 units over its lifetime
	blood->pos.trDelta[2] = 10;
}


//PKMOD - Ergodic 07/05/00 add viewheight logic 
void CG_BearTrap_Display ( centity_t *cent, int player_viewheight ) {
	refEntity_t			ent;

//PKMOD Ergodic debug 06/28/00 
//Com_Printf("CG_BearTraps_Follow - cent->lerpAngles>%s<\n", CG_vtos(cent->lerpAngles));

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	//Add random displacement
	ent.origin[0] += (rand() % (MAX_BEARTRAP_DISPLACEMENT * 2 + 1) ) - MAX_BEARTRAP_DISPLACEMENT;
	ent.origin[1] += (rand() % (MAX_BEARTRAP_DISPLACEMENT * 2 + 1) ) - MAX_BEARTRAP_DISPLACEMENT;
	//PKMOD - Ergodic 07/05/00 add viewheight logic 
	//PKMOD - Ergodic 07/29/00 modify view height logic for death height 
	if (player_viewheight == DEFAULT_VIEWHEIGHT)
		ent.origin[2] += (rand() % (MAX_BEARTRAP_DISPLACEMENT * 2 + 1) ) - MAX_BEARTRAP_DISPLACEMENT - 6; //above viewheight?
	else if (player_viewheight == CROUCH_VIEWHEIGHT)
			ent.origin[2] += (rand() % (MAX_BEARTRAP_DISPLACEMENT + 1) ) - (MAX_BEARTRAP_DISPLACEMENT  + 24); //below viewheight?
		else // else DEAD_VIEWHEIGHT
			ent.origin[2] += (rand() % (MAX_BEARTRAP_DISPLACEMENT + 1) ) - (MAX_BEARTRAP_DISPLACEMENT  + 29); //death height

	AnglesToAxis( cent->lerpAngles, ent.axis );
//	ent.reType = RT_SPRITE;
	ent.hModel = cgs.media.pkabeartrapfollow;
//	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	trap_R_AddRefEntityToScene( &ent );

}

//PKMOD Ergodic 07/01/00 added more logic - multi-beartraps and blood  
//PKMOD - Ergodic 07/05/00 add viewheight logic 
//PKMOD - Ergodic 07/10/00 modified to place beartrap on bots 
void CG_BearTraps_Follow( centity_t *cent ) {
	int			i;
	int			beartrap_count;
	int			player_viewheight;

//PKMOD - Ergodic 07/10/00 unpack the beartraps_attached variable 
//PKMOD - Ergodic 12/19/00 use time2 instead of angles2 
//	player_viewheight = cent->currentState.angles2[BEARTRAPS_ATTACHED] / 100;
//	beartrap_count = cent->currentState.angles2[BEARTRAPS_ATTACHED] - (player_viewheight * 100);
	beartrap_count = cent->currentState.time2 & 3;		//mask of max three beartraps
	//PKMOD - Ergodic 07/07/01 - use new packing scheme to encode beartrap viewheight
	player_viewheight = ( cent->currentState.time2 >> 2 ) & 3;	//mask off the encoded viewheight

	if ( player_viewheight == 0 )
		player_viewheight = DEFAULT_VIEWHEIGHT;
	else	if ( player_viewheight == CROUCH_VIEWHEIGHT )
				player_viewheight = CROUCH_VIEWHEIGHT;
			else
				player_viewheight = DEAD_VIEWHEIGHT;

//	player_viewheight -= 50;  //un-normalize the viewpoint that was done in bg_misc.c;
//PKMOD Ergodic debug 12/19/00 inactive
//Com_Printf("CG_BearTraps_Follow - cent->lerpAngles>%s<\n", CG_vtos(cent->lerpAngles));

	for (i = 1; i <= beartrap_count; i++) {
		if (i > MAX_BEARTRAPS_VIEWABLE)  //don't display more than MAX_BEARTRAPS_VIEWABLE beartraps 
			break;
		CG_BearTrap_Display ( cent, player_viewheight );
	}

	if (BEARTRAP_BLOOD_FREQUENCY > ( rand() % 100 ) )
		CG_BeartTrap_BloodTrail( cent );

}


/*
==================
PKMOD - CG_BearTrap, Ergodic 05/30/00 modeled from CG_ITEM
==================
*/

static void CG_BearTrap( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;

	//PKMOD - Ergodic 12/13/03 - add timing for animation
	int					msec;


	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;

	//PKMOD - Ergodic 08/07/00 - set axis so that beartrap will fly flat 
	AxisClear( ent.axis );


	// spin as it moves
//	if ( s1->pos.trType != TR_STATIONARY ) {
//		RotateAroundDirection( ent.axis, cg.time / 4 );
//	} else {
	RotateAroundDirection( ent.axis, s1->time );
//	}


	//PKMOD - Ergodic 03/23/01 - add team parameters
	switch ( cent->currentState.modelindex ) { 
		case TEAM_RED:
			ent.hModel = cgs.media.pkabeartrap_red;
			break;
		case TEAM_BLUE:
			ent.hModel = cgs.media.pkabeartrap_blue;
			break;
		default:
			ent.hModel = cgs.media.pkabeartrap;
			ent.frame = 0; 
			//PKMOD - Ergodic 07/17/03 - set animation frame based on entity duration.
			//		The beartrap model has 55 frames (0..54) of animation.
			//
			//		Want to show 55 frames in 2500 milliseconds (2.5 second).
			//		Thus, each frame is shown for 2500/55 = ~45 milliseconds per frame

			//PKMOD - Ergodic 10/10/03 - [UPDATE] animation frame based on generic1
			
			//PKMOD - Ergodic 10/10/03 - Co-opt the centity_t variables:
			//			PKA_RadiateTime :: last invisibility charge state;
			//			PKA_RadiateInfectTime :: time for particle effect;

			if ( s1->generic1 ) {
				//here if Beartrap has a charge...

				//PKMOD - Ergodic 10/10/03 - Check if particles should be displayed
				//			Display particles if duration for particle effect is current

				//has the invisibility charge changed?
				if ( s1->generic1 != cent->PKA_RadiateTime ) {
					cent->PKA_RadiateTime = s1->generic1;		//Set client invisibility state
					cent->PKA_RadiateInfectTime = cg.time + 750;		//every 3/4 seconds
				}

				//is the invisibility duration still in effect?
				if ( cent->PKA_RadiateInfectTime > cg.time ) {
					//PKMOD - Ergodic 07/18/03 - calculate the position of the beartrap's "particle_tag"
					int				i;
					orientation_t	lerped;
					vec3_t			particles;

					//particle parameters
					vec3_t		vel;
					int			duration;
					float		x_offset;
					float		y_offset;
					float		speed;
					int			indx;
					vec3_t		spark_origin;	

					/*+++++
					//	Determine the location of the particles
					//----*/
					//void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
					//								qhandle_t parentModel, char *tagName ) {
					// lerp the tag (HERE: msec is the frame)
				
					//calculate particle frame based on generic1 with values of [0 ... MAX_INVISIBILITY_CHARGE]
					//		within a total of 55 frames of animation
					msec = 55 * s1->generic1 / MAX_INVISIBILITY_CHARGE;
	
					ent.frame = msec;

					trap_R_LerpTag( &lerped, ent.hModel, msec, msec, 1.0 - ent.backlerp, "tag_particles" );
					VectorCopy( ent.origin, particles );
					for ( i = 0 ; i < 3 ; i++ ) {
						VectorMA( particles, lerped.origin[i], ent.axis[i], particles );
					}

					/*+++++
					//	Show the particles
					//----*/
					VectorCopy(particles, spark_origin);
					spark_origin[2] += 5;	

					for ( indx = 1; indx < 2; indx++ ) {
						vel[0] = 2 * crandom();
						vel[1] = 2 * crandom();
						//PKMOD - Ergodic 10/15/03 - force all particles up
						vel[2] = 10 * random();	

						//PKMOD - Ergodic 10/15/03 - vary the duration
						duration = 750 + 50 * crandom();	

						x_offset = 2 * crandom();
						y_offset = 2 * crandom();

						speed = 5 * crandom();

						//PKMOD - Ergodic 07/20/03 - draw midsized particles...
						CG_ParticleSparks3 (spark_origin, vel, duration, x_offset, y_offset, speed);
					}
				}

			}
			else	//here if s1->generic1
				//PKMOD - Ergodic 07/17/03 - default msec to 0
				msec = 0;



			//PKMOD - Ergodic 07/18/03 - set custom shader for the invisibility process
			//		The Beartrap has 20 shaders to apply 
			//		
			//		Want to show 20 shaders in 2500 milliseconds (2.5 seconds)
			//		Thus each shader is show for 2500/20 = ~125 milliseconds per shader
			//PKMOD - Ergodic 08/18/03 - only show invisible shaders if beartrap hit the floor

			//PKMOD - Ergodic 10/10/04 - [UPDATE] shader frame based on generic1

			//calculate particle frame based on generic1 with values of [0 ... MAX_INVISIBILITY_CHARGE]
			//		within a total of 20 invisibility shaders
			msec = 20 * s1->generic1 / MAX_INVISIBILITY_CHARGE;

			switch ( msec ) {
				case 0:
					ent.customShader = cgs.media.pkainvisbeartrap1;
					break;
				case 1: 
					ent.customShader = cgs.media.pkainvisbeartrap2;
					break;
				case 2: 
					ent.customShader = cgs.media.pkainvisbeartrap3;
					break;
				case 3: 
					ent.customShader = cgs.media.pkainvisbeartrap4;
					break;
				case 4: 
					ent.customShader = cgs.media.pkainvisbeartrap5;
					break;
				case 5: 
					ent.customShader = cgs.media.pkainvisbeartrap6;
					break;
				case 6: 
					ent.customShader = cgs.media.pkainvisbeartrap7;
					break;
				case 7: 
					ent.customShader = cgs.media.pkainvisbeartrap8;
					break;
				case 8: 
					ent.customShader = cgs.media.pkainvisbeartrap9;
					break;
				case 9: 
					ent.customShader = cgs.media.pkainvisbeartrap10;
					break;
				case 10: 
					ent.customShader = cgs.media.pkainvisbeartrap11;
					break;
				case 11: 
					ent.customShader = cgs.media.pkainvisbeartrap12;
					break;
				case 12: 
					ent.customShader = cgs.media.pkainvisbeartrap13;
					break;
				case 13: 
					ent.customShader = cgs.media.pkainvisbeartrap14;
					break;
				case 14: 
					ent.customShader = cgs.media.pkainvisbeartrap15;
					break;
				case 15: 
					ent.customShader = cgs.media.pkainvisbeartrap16;
					break;
				case 16: 
					ent.customShader = cgs.media.pkainvisbeartrap17;
					break;
				case 17: 
					ent.customShader = cgs.media.pkainvisbeartrap18;
					break;
				case 18: 
					ent.customShader = cgs.media.pkainvisbeartrap19;
					break;
				default:
					ent.customShader = cgs.media.pkainvisbeartrap20;
					break;
			}
			break;
		}

		
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// add to refresh list, possibly with quad glow
	CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
}


/*
==================
PKMOD - CG_AutoSentry, Ergodic 11/22/00 modeled from CG_BearTrap

changes: Ergodic 05/31/02 - add new animated models
==================
*/

static void CG_AutoSentry( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t	*weapon;
	vec3_t				hold_dir;

	//PKMOD - Ergodic 11/25/00 add timing for animation
	int					msec;

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	//PKMOD Ergodic 11/26/00
	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	//PKMOD Ergodic debug 11/26/00 - inactive
//	if ( (rand() % 1000) > 975 )
//		Com_Printf("CG_AutoSentry - lerpAngles>%s<\n", CG_vtos( cent->lerpAngles ) );


	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
//	ent.hModel = cgs.media.pkasentry_pickup;

	//PKMOD - Ergodic 11/25/00 add timing for animation
	//			every 0.15 seconds (150 milliseconds)
	//		if miscTime has not been set, by drop event, then model
	//			should default to folded type

	//PKMOD - Ergodic 06/01/02 - remove the following old animation code
//	if ( cent->miscTime )
//		msec = ( cg.time - cent->miscTime ) / 150;
//	else
//		msec = 0;

	//PKMOD - Ergodic 06/01/02 - set animation frame based on entity duration.
	//		The autosentry model has 30 frames (1..30) of animation that is reversed
	//		in time. Frame 1 is the final animation frame and frame 30 is the first
	//		animation frame. 
	//
	//		each animation frame is 1/30 seconds in duration. 
	//		1/30 seconds ~ 35 MilliSeconds
	//		30 frames * 35 MS/frame = 1050 milliseconds 
	if ( cent->miscTime )
		msec = ( cg.time - cent->miscTime ) / 35;
	else
		//PKMOD - Ergodic 10/22/02 - default msec to 0
		//msec = 29; //was 29 -> produced a deployed sentry when launched
		msec = 0;

	//PKMOD - Ergodic 06/01/02 - calculate the frame
	if ( msec > 29 )
		ent.frame = 0;
	else
		ent.frame = 29 - msec;

	//PKMOD Ergodic debug 06/02/02 - debug autosentry animations (inactive)
//	if ( (rand() % 1000) > 900 )
//		Com_Printf("CG_AutoSentry - msec: %d, miscTime: %d\n", msec, cent->miscTime );

	//PKMOD Ergodic debug 11/26/00 - inactive
//	if ( (rand() % 1000) > 200 )
//		Com_Printf("CG_AutoSentry 0 - msec: %d, miscTime: %d\n", msec, cent->miscTime );

	//PKMOD - Ergodic 03/20/01 - add team parameters
	switch ( cent->currentState.modelindex ) { 
		case TEAM_RED:
			ent.hModel = cgs.media.pkasentry_red;
			break;
		case TEAM_BLUE:
			ent.hModel = cgs.media.pkasentry_blue;
			break;
		default:
			ent.hModel = cgs.media.pkasentry;
			break;
	}

	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
//	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
//		ent.axis[0][1] = 1;
//		ent.axis[0][1] = 1;
//		ent.axis[0][0] = 1;
//	}

	//PKMOD - Ergodic 08/07/00 - set axis so that autosentry will fly flat 
//	AxisClear( ent.axis );

	//PKMOD Ergodic 11/26/00 - set the initial direction
//	VectorCopy( cent->lerpAngles, ent.axis[0] );
//	RotateAroundDirection( ent.axis, s1->time );


//	hold_dir[0] = cent->lerpAngles[0];
//	hold_dir[1] = cent->lerpAngles[1];
//	hold_dir[2] = 0;

//	if ( VectorNormalize2( hold_dir, ent.axis[0] ) == 0 ) {
//		ent.axis[0][2] = 1;
//	}


	//PKMOD Ergodic 11/27/00 - Finally - This works!
	vectoangles( cent->lerpAngles, hold_dir);
	hold_dir[0] = 0;
	hold_dir[1] -= 90;  //offset
	hold_dir[2] = 0;
	AnglesToAxis( hold_dir, ent.axis );

	//PKMOD Ergodic 11/26/00 - remove this
//	AxisCopy( cg.autoAxis, ent.axis );

	//PKMOD Ergodic 11/26/00 - remove this
//	RotateAroundDirection( ent.axis, hold_dir[YAW] );

	//PKMOD Ergodic 11/26/00 - add entity to scene
	trap_R_AddRefEntityToScene (&ent);


	// add to refresh list, possibly with quad glow
//	CG_AddRefEntityWithPowerups( &ent, s1->powerups, TEAM_FREE );
}

/*
==================
PKMOD - CG_AutoSentry_Base, Ergodic 12/02/00 modeled from CG_AutoSentry
changes: Ergodic 12/13/03 - install invisibility code
==================
*/

static void CG_AutoSentry_Base( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;
	vec3_t				hold_dir;

	//PKMOD - Ergodic 12/13/03 - add timing for animation
	int					msec;

	//PKMOD Ergodic debug 12/02/00 - inactive
//	if ( (rand() % 1000) > 900 )
//		Com_Printf( "CG_AutoSentry_Base\n" );

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	//PKMOD Ergodic 11/26/00
	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	//PKMOD Ergodic debug 11/26/00 - inactive
//	if ( (rand() % 1000) > 975 )
//		Com_Printf("CG_AutoSentry - lerpAngles>%s<\n", CG_vtos( cent->lerpAngles ) );


	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;

	//PKMOD - Ergodic 03/20/01 - add team parameters
	switch ( cent->currentState.modelindex ) { 
		case TEAM_RED:
			ent.hModel = cgs.media.pkasentry_base_red;
			break;
		case TEAM_BLUE:
			ent.hModel = cgs.media.pkasentry_base_blue;
			break;
		default:
			ent.hModel = cgs.media.pkasentry_base;
			//PKMOD - Ergodic 10/10/03 - Install invisibility function
			//			Autosentry_Base will not show particles

			//PKMOD - Ergodic 07/18/03 - set custom shader for the invisibility process
			//		The Autosentry has 20 shaders to apply 
			//		
			//		Want to show 20 shaders in 2500 milliseconds (2.5 seconds)
			//		Thus each shader is show for 2500/20 = ~125 milliseconds per shader
			//PKMOD - Ergodic 08/18/03 - only show invisible shaders if beartrap hit the floor

			//PKMOD - Ergodic 10/10/04 - [UPDATE] shader frame based on generic1

			//calculate particle frame based on generic1 with values of [0 ... MAX_INVISIBILITY_CHARGE]
			//		within a total of 20 invisibility shaders
			msec = 20 * s1->generic1 / MAX_INVISIBILITY_CHARGE;

			switch ( msec ) {
				case 0:
					ent.customShader = cgs.media.pkainvisautosentry1;
					break;
				case 1: 
					ent.customShader = cgs.media.pkainvisautosentry2;
					break;
				case 2: 
					ent.customShader = cgs.media.pkainvisautosentry3;
					break;
				case 3: 
					ent.customShader = cgs.media.pkainvisautosentry4;
					break;
				case 4: 
					ent.customShader = cgs.media.pkainvisautosentry5;
					break;
				case 5: 
					ent.customShader = cgs.media.pkainvisautosentry6;
					break;
				case 6: 
					ent.customShader = cgs.media.pkainvisautosentry7;
					break;
				case 7: 
					ent.customShader = cgs.media.pkainvisautosentry8;
					break;
				case 8: 
					ent.customShader = cgs.media.pkainvisautosentry9;
					break;
				case 9: 
					ent.customShader = cgs.media.pkainvisautosentry10;
					break;
				case 10: 
					ent.customShader = cgs.media.pkainvisautosentry11;
					break;
				case 11: 
					ent.customShader = cgs.media.pkainvisautosentry12;
					break;
				case 12: 
					ent.customShader = cgs.media.pkainvisautosentry13;
					break;
				case 13: 
					ent.customShader = cgs.media.pkainvisautosentry14;
					break;
				case 14: 
					ent.customShader = cgs.media.pkainvisautosentry15;
					break;
				case 15: 
					ent.customShader = cgs.media.pkainvisautosentry16;
					break;
				case 16: 
					ent.customShader = cgs.media.pkainvisautosentry17;
					break;
				case 17: 
					ent.customShader = cgs.media.pkainvisautosentry18;
					break;
				case 18: 
					ent.customShader = cgs.media.pkainvisautosentry19;
					break;
				default:
					ent.customShader = cgs.media.pkainvisautosentry20;
					break;
			}
			break;
	}

	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	//PKMOD Ergodic 11/27/00 - Finally - This works!
	vectoangles( cent->lerpAngles, hold_dir);
	hold_dir[0] = 0;
	hold_dir[1] -= 90;  //offset
	hold_dir[2] = 0;
	AnglesToAxis( hold_dir, ent.axis );

	//PKMOD Ergodic 11/26/00 - add entity to scene
	trap_R_AddRefEntityToScene (&ent);

	// add to refresh list, possibly with quad glow
//	CG_AddRefEntityWithPowerups( &ent, s1->powerups, TEAM_FREE );
}


/*
==================
PKMOD - CG_AutoSentry_Turret, Ergodic 12/02/00 modeled from CG_AutoSentry
			modified 12/28/00 to add flash when firing
changes: Ergodic 12/13/03 - install invisibility code
==================
*/

static void CG_AutoSentry_Turret( centity_t *cent ) {
	refEntity_t			turret;
	refEntity_t			flash;
	entityState_t		*s1;
	const weaponInfo_t	*weapon;
//	vec3_t				hold_dir; //020522 - do not need
//	vec3_t				hold_autosentry_forward; //020522 - do not need
//	int					debug_yaw;
	vec3_t				angles;
	//PKMOD - Ergodic 04/12/01 - add quad effects 
	qhandle_t			quadShader;
//	float				hold_angle;
//	vec3_t				hold_angles; //020522 - debug variable (inactive)
	//PKMOD - Ergodic 12/13/01 add timing for invisible shaders
	int		msec;
	
	//PKMOD Ergodic debug 12/02/00 - inactive
//	if ( (rand() % 1000) > 900 )
//		Com_Printf( "CG_AutoSentry_Turret\n" );


	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	//PKMOD Ergodic 11/26/00
	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);
	//PKMOD - Ergodic 05/22/02 - do not need
//	AngleVectorsForward( cent->lerpAngles, hold_autosentry_forward );

	//PKMOD Ergodic debug 05/18/02 (inactive)
//	if ( (rand() % 1000) > 975 ) {
//		VectorScale( s1->angles, 100, hold_angles);
//		Com_Printf("CG_AutoSentry - lerpAngles>%s<, hold_angles>%s<\n", CG_vtos( cent->lerpAngles ), CG_vtos( hold_angles ) );
//	}

	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&turret, 0, sizeof(turret));
	VectorCopy( cent->lerpOrigin, turret.origin);
	VectorCopy( cent->lerpOrigin, turret.oldorigin);

	// setup skins
	turret.skinNum = cg.clientFrame & 1;
	turret.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	//PKMOD - Ergodic 03/20/01 - add team parameters
	switch ( cent->currentState.modelindex ) { 
		case TEAM_RED:
			quadShader = cgs.media.redQuadShader;
			turret.hModel = cgs.media.pkasentry_turret_red;
			break;
		case TEAM_BLUE:
			quadShader = cgs.media.quadShader;
			turret.hModel = cgs.media.pkasentry_turret_blue;
			break;
		default:
			quadShader = cgs.media.quadShader;
			turret.hModel = cgs.media.pkasentry_turret;
			//PKMOD - Ergodic 10/10/03 - Install invisibility function
			//		Co-opt the centity_t variables:
			//			PKA_RadiateTime :: last invisibility charge state;
			//			PKA_RadiateInfectTime :: time for particle effect;

			//PKMOD Ergodic 12/13/02 - debug invisibility (inactive)
			//Com_Printf("CG_AutoSentry_Turret - invisibility charge>%d<\n", s1->generic1 );

			if ( s1->generic1 ) {
				//here if autosentry has a charge...

				//PKMOD - Ergodic 10/10/03 - Check if particles should be displayed
				//			Display particles if duration for particle effect is current

				//has the invisibility charge changed?
				if ( s1->generic1 != cent->PKA_RadiateTime ) {
					cent->PKA_RadiateTime = s1->generic1;		//Set client invisibility state
					cent->PKA_RadiateInfectTime = cg.time + 750;		//every 3/4 seconds
				}

				//is the invisibility duration still in effect?
				if ( cent->PKA_RadiateInfectTime > cg.time ) {
					//PKMOD - Ergodic 07/18/03 - calculate the position of the beartrap's "particle_tag"
					int				i;
					orientation_t	lerped;
					vec3_t			particles;

					//particle parameters
					vec3_t		vel;
					int			duration;
					float		x_offset;
					float		y_offset;
					float		speed;
					int			indx;
					vec3_t		spark_origin;	

					/*+++++
					//	Determine the location of the particles
					//----*/
					if ( rand() % 2 )
						trap_R_LerpTag( &lerped, turret.hModel, msec, msec, 1.0 - turret.backlerp, "tag_flash_1" );
					else
						trap_R_LerpTag( &lerped, turret.hModel, msec, msec, 1.0 - turret.backlerp, "tag_flash_2" );
					VectorCopy( turret.origin, particles );
					for ( i = 0 ; i < 3 ; i++ ) {
						VectorMA( particles, lerped.origin[i], turret.axis[i], particles );
					}

					/*+++++
					//	Show the particles
					//----*/
					VectorCopy(particles, spark_origin);
					spark_origin[2] += 5;	

					//PKMOD - Ergodic 12/13/03 - add more particles from 2 to 4
					for ( indx = 1; indx < 4; indx++ ) {
						vel[0] = 2 * crandom();
						vel[1] = 2 * crandom();
						//PKMOD - Ergodic 10/15/03 - force all particles up
						vel[2] = 10 * random();	

						//PKMOD - Ergodic 10/15/03 - vary the duration
						duration = 750 + 50 * crandom();	

						x_offset = 2 * crandom();
						y_offset = 2 * crandom();

						speed = 5 * crandom();

						//PKMOD - Ergodic 07/20/03 - draw midsized particles...
						CG_ParticleSparks3 (spark_origin, vel, duration, x_offset, y_offset, speed);
					}
				}

			}
			else	//here if s1->generic1
				//PKMOD - Ergodic 07/17/03 - default msec to 0
				msec = 0;



			//PKMOD - Ergodic 07/18/03 - set custom shader for the invisibility process
			//		The Autosentry has 20 shaders to apply 
			//		
			//		Want to show 20 shaders in 2500 milliseconds (2.5 seconds)
			//		Thus each shader is show for 2500/20 = ~125 milliseconds per shader
			//PKMOD - Ergodic 08/18/03 - only show invisible shaders if beartrap hit the floor

			//PKMOD - Ergodic 10/10/04 - [UPDATE] shader frame based on generic1

			//calculate particle frame based on generic1 with values of [0 ... MAX_INVISIBILITY_CHARGE]
			//		within a total of 20 invisibility shaders
			msec = 20 * s1->generic1 / MAX_INVISIBILITY_CHARGE;

			switch ( msec ) {
				case 0:
					turret.customShader = cgs.media.pkainvisautosentry1;
					break;
				case 1: 
					turret.customShader = cgs.media.pkainvisautosentry2;
					break;
				case 2: 
					turret.customShader = cgs.media.pkainvisautosentry3;
					break;
				case 3: 
					turret.customShader = cgs.media.pkainvisautosentry4;
					break;
				case 4: 
					turret.customShader = cgs.media.pkainvisautosentry5;
					break;
				case 5: 
					turret.customShader = cgs.media.pkainvisautosentry6;
					break;
				case 6: 
					turret.customShader = cgs.media.pkainvisautosentry7;
					break;
				case 7: 
					turret.customShader = cgs.media.pkainvisautosentry8;
					break;
				case 8: 
					turret.customShader = cgs.media.pkainvisautosentry9;
					break;
				case 9: 
					turret.customShader = cgs.media.pkainvisautosentry10;
					break;
				case 10: 
					turret.customShader = cgs.media.pkainvisautosentry11;
					break;
				case 11: 
					turret.customShader = cgs.media.pkainvisautosentry12;
					break;
				case 12: 
					turret.customShader = cgs.media.pkainvisautosentry13;
					break;
				case 13: 
					turret.customShader = cgs.media.pkainvisautosentry14;
					break;
				case 14: 
					turret.customShader = cgs.media.pkainvisautosentry15;
					break;
				case 15: 
					turret.customShader = cgs.media.pkainvisautosentry16;
					break;
				case 16: 
					turret.customShader = cgs.media.pkainvisautosentry17;
					break;
				case 17: 
					turret.customShader = cgs.media.pkainvisautosentry18;
					break;
				case 18: 
					turret.customShader = cgs.media.pkainvisautosentry19;
					break;
				default:
					turret.customShader = cgs.media.pkainvisautosentry20;
					break;
			}
			break;
	}

	//PKMOD Ergodic 11/27/00 - Finally - This works!
	//PKMOD - Ergodic 02/09/02 - modify this
//	vectoangles( cent->lerpAngles, hold_dir);

		//PKMOD Ergodic 12/02/00 - debug inactive
//	if ( (rand() % 1000) > 975 ) {
//		debug_yaw = hold_dir[1];
//		Com_Printf("CG_AutoSentry_Turret - debug_yaw>%d<\n", debug_yaw );
//	}

	//PKMOD - Ergodic 05/17/02 - no need to touch the angles..
//	hold_dir[0] += 30;
//	hold_dir[1] -= 90;  //offset
//	hold_dir[2] -= 30;
//	AnglesToAxis( hold_dir, turret.axis );
	//PKMOD - Ergodic 02/18/02 - add this...
//	AngleVectors( hold_dir, turret.axis[0], turret.axis[1], turret.axis[2] );

	//PKMOD - 02/08/02 - new axis code
//	if ( VectorNormalize2( cent->lerpAngles, turret.axis[0] ) == 0 ) {
//		turret.axis[0][2] = 1;
//	}

//>>>>>
	//PKMOD - Ergodic 05/19/02 - new axis code (revisited)
	// convert direction of travel into axis
//	if ( VectorNormalize2( hold_dir, turret.axis[0] ) == 0 ) {
//		turret.axis[0][2] = 1;
//	}

//	AngleVectors (hold_angles, hold_forward, hold_right, hold_up);
//	vectorCopy( cent->lerpAngles, hold_dir);
	AnglesToAxis( cent->lerpAngles, turret.axis );

//	AngleVectorsForward( cent->lerpAngles, hold_autosentry_forward );
//	AnglesToAxis( hold_autosentry_forward, turret.axis );
	trap_R_AddRefEntityToScene ( &turret );

//	RotateAroundDirection( turret.axis, s1->time );

	// add to refresh list, possibly with quad glow
//	CG_AddRefEntityWithPowerups( &turret, s1, TEAM_FREE );
//>>>>>

	//PKMOD Ergodic 06/29/01 - use pos.trDelta to compute angle
	// convert direction of travel into axis
//	if ( VectorNormalize2( s1->pos.trDelta, turret.axis[0] ) == 0 ) {
//		turret.axis[0][2] = 1;
//	}

//	vectoangles( s1->pos.trDelta, hold_dir );
	//PKMOD - Ergodic 02/18/02 - remove this...
//	AnglesToAxis( hold_dir, turret.axis );



	//PKMOD Ergodic 11/26/00 - add entity to scene
//	trap_R_AddRefEntityToScene ( &turret );

	// add to refresh list, possibly with quad glow
//	CG_AddRefEntityWithPowerups( &ent, s1->powerups, TEAM_FREE );

	//PKMOD - Ergodic 04/12/01 - add quad effects if enabled in co-opted variable:time2
	if ( s1->time2 & 2 ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1 );
		turret.customShader = quadShader;
		trap_R_AddRefEntityToScene( &turret );
	}

	//PKMOD - Ergodic 12/28/00 - add firing logic (co-opt the time2 variable)
	//PKMOD - Ergodic 04/12/01 - modify so that time2 will be a packed variable
	if ( !( s1->time2 & 1) )		//if not firing then return
		return;

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( cent->lerpOrigin, flash.lightingOrigin );
//	flash.shadowPlane = RF_SHADOW_PLANE;
	flash.renderfx = RF_MINLIGHT;

	flash.hModel = cgs.media.autosentryFlashModel;
	cent->muzzleFlashTime = cg.time + 100;

	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis( angles, flash.axis );

	if ( random() > 0.5 ) //50 Percent
		CG_PositionRotatedEntityOnTag( &flash, &turret, cgs.media.pkasentry_turret, "tag_flash_1");
	else
		CG_PositionRotatedEntityOnTag( &flash, &turret, cgs.media.pkasentry_turret, "tag_flash_2");

	trap_R_AddRefEntityToScene( &flash );


}


/*
===============
PKMOD - CG_ChainShaft, Ergodic 07/12/00 display the inter-player lightning shafts 

  loosely based on CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/

//Ergodic - 07/12/00 this code is superceded by CG_ChainLightning and should be removed
void CG_ChainShaft( entityState_t *es ) {
	refEntity_t		beam;
	vec3_t			angles;
	vec3_t			dir;

	//PKMOD Ergodic debug 07/12/00 
//Com_Printf("CG_ChainShaft - origin>%s<, angles>%s<\n", CG_vtos(es->origin), CG_vtos(es->angles));


	memset( &beam, 0, sizeof( beam ) );

	VectorCopy( es->origin, beam.origin );
	VectorCopy( es->angles, beam.oldorigin );  //PKMOD - Ergodic 07/12/00 - co-opt hack: angles = target->origin

//PKMOD - Ergodic 08/21/00 change flash to railgun style to make a tighter beam
	beam.reType = RT_RAIL_CORE;
//	beam.reType = RT_LIGHTNING;
	//PKMOD - Ergodic 08/21/00 set the shader to chainlightning
	beam.customShader = cgs.media.chainlightningShader;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something

	VectorSubtract( beam.oldorigin, beam.origin, dir );
	VectorNormalize( dir );

	memset( &beam, 0, sizeof( beam ) );
	beam.hModel = cgs.media.lightningExplosionModel;

	VectorMA( beam.oldorigin, -16, dir, beam.origin );

	// make a random orientation
	angles[0] = rand() % 360;
	angles[1] = rand() % 360;
	angles[2] = rand() % 360;
	AnglesToAxis( angles, beam.axis );
	trap_R_AddRefEntityToScene( &beam );
	
}

void CG_ChainLightning( centity_t *cent ) {
	refEntity_t		beam;
	entityState_t	*es;
	vec3_t			angles;
//	vec3_t			dir;  //05/12/02 - don't need this variable

	es = &cent->currentState;
//PKMOD - Ergodic 07/12/00 - debug inactive
//Com_Printf("CG_ChainShaft - start\n");
//		if ( (rand() % 1000) > 950 )
//			Com_Printf("CG_ChainLightning - origin>%s<, angles>%s<\n", CG_vtos(cent->lerpOrigin), CG_vtos(es->angles));

	//PKMOD - Ergodic 05/12/02 - debug 000 location of clg beam (inactive)
//	Com_Printf("CG_ChainLightning - time>%d<, origin>%s<, angles>%s<\n", cg.time, CG_vtos(cent->lerpOrigin), CG_vtos(es->angles));


	memset( &beam, 0, sizeof( beam ) );

	VectorCopy( cent->lerpOrigin, beam.origin );
	VectorCopy( es->angles, beam.oldorigin );  //PKMOD - Ergodic 07/12/00 - co-opt hack: angles = target->origin

//PKMOD - Ergodic 08/21/00 change flash to railgun style to make a tighter beam
	beam.reType = RT_RAIL_CORE;
//	beam.reType = RT_LIGHTNING;
	//PKMOD - Ergodic 08/21/00 set the shader to chainlightning
	beam.customShader = cgs.media.chainlightningShader;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something

	//PKMOD - Ergodic 05/12/02 - don't need to do these next two steps
//	VectorSubtract( beam.oldorigin, beam.origin, dir );
//	VectorNormalize( dir );

	memset( &beam, 0, sizeof( beam ) );
	//PKMOD - Ergodic 05/12/02 - reinitialize the location of lightning explosion
	VectorCopy( es->angles, beam.origin );  //PKMOD - Ergodic 07/12/00 - co-opt hack: angles = target->origin

	beam.hModel = cgs.media.lightningExplosionModel;

	//test comment 12/05/00
//	VectorMA( beam.oldorigin, -16, dir, beam.origin );

	// make a random orientation
	angles[0] = rand() % 360;
	angles[1] = rand() % 360;
	angles[2] = rand() % 360;
	AnglesToAxis( angles, beam.axis );
	trap_R_AddRefEntityToScene( &beam );
	
}

//PKMOD - Ergodic 08/20/032 - copy of CG_ChainLightning for use in shooter lightning effect
void CG_ShooterLightning( centity_t *cent ) {
	refEntity_t		beam;
	entityState_t	*es;
	vec3_t			angles;
//	vec3_t			dir;  //05/12/02 - don't need this variable

	es = &cent->currentState;
//PKMOD - Ergodic 07/12/00 - debug inactive
//Com_Printf("CG_ChainShaft - start\n");
//		if ( (rand() % 1000) > 950 )
//			Com_Printf("CG_ChainLightning - origin>%s<, angles>%s<\n", CG_vtos(cent->lerpOrigin), CG_vtos(es->angles));

	//PKMOD - Ergodic 05/12/02 - debug 000 location of clg beam (inactive)
//	Com_Printf("CG_ChainLightning - time>%d<, origin>%s<, angles>%s<\n", cg.time, CG_vtos(cent->lerpOrigin), CG_vtos(es->angles));


	memset( &beam, 0, sizeof( beam ) );

	VectorCopy( cent->lerpOrigin, beam.origin );
	VectorCopy( es->angles, beam.oldorigin );  //PKMOD - Ergodic 07/12/00 - co-opt hack: angles = target->origin

//PKMOD - Ergodic 08/21/00 change flash to railgun style to make a tighter beam
	beam.reType = RT_RAIL_CORE;
//	beam.reType = RT_LIGHTNING;
	//PKMOD - Ergodic 08/21/00 set the shader to chainlightning
	beam.customShader = cgs.media.shooterlightningShader;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something

	//PKMOD - Ergodic 05/12/02 - don't need to do these next two steps
//	VectorSubtract( beam.oldorigin, beam.origin, dir );
//	VectorNormalize( dir );

	memset( &beam, 0, sizeof( beam ) );
	//PKMOD - Ergodic 05/12/02 - reinitialize the location of lightning explosion
	VectorCopy( es->angles, beam.origin );  //PKMOD - Ergodic 07/12/00 - co-opt hack: angles = target->origin

	beam.hModel = cgs.media.lightningExplosionModel;

	//test comment 12/05/00
//	VectorMA( beam.oldorigin, -16, dir, beam.origin );

	// make a random orientation
	angles[0] = rand() % 360;
	angles[1] = rand() % 360;
	angles[2] = rand() % 360;
	AnglesToAxis( angles, beam.axis );
	trap_R_AddRefEntityToScene( &beam );
	
}


/*
==================
PKMOD - CG_Nail, Ergodic 08/03/00 display the Nail 
==================
*/
static void CG_Nail( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.media.nail1;


//	VectorSet(ent.axis[1], 0,0,0);
//	VectorSet(ent.axis[2], 0,0,0);

	VectorCopy( s1->apos.trBase, ent.axis[0] );
	RotateAroundDirection( ent.axis, s1->time );

	//Ergodic debug - 08/04/00 - inactive
//	Com_Printf("CG_Nail - ent.axis[0]>%s<, [1]>%s<, [2]>%s<\n", CG_vtos(ent.axis[0]), CG_vtos(ent.axis[1]), CG_vtos(ent.axis[2]) );


	// add to refresh list
	trap_R_AddRefEntityToScene (&ent);
}

/*
==================
PKMOD - CG_Dragon_Deploy, Ergodic 03/14/01 - add dragon deployable model

		Will display the deployed model after it is launched form the dragon
==================
*/

static void CG_Dragon_Deploy( centity_t *cent ) {
	refEntity_t				ent;
	entityState_t			*s1;
	const weaponInfo_t		*weapon;	//weapon == dragon
	const weaponInfo_t		*weapon_deployed;	//weapon2 == deployable weapon 
	vec3_t					hold_dir;


	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		//PKMOD - Ergodic 02/14/02 - add bladewhirl sound if gauntlet, else play standard sound
		if ( ( s1->generic1 & 15 ) == WP_GAUNTLET ) 
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->flashSound[3] );
		else
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;

	//get the deployed weapon's information
	weapon_deployed = &cg_weapons[ s1->generic1 & 15 ];

	//PKMOD - Ergodic 08/07/00 - set axis so that beartrap will fly flat 
	AxisClear( ent.axis );

	//add the appropriate model
	switch ( s1->generic1 & 15 ) {
		case	WP_GRAVITY:
			ent.hModel = weapon_deployed->weaponModel;
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - WP_GRAVITY\n" );
			break;
		case 	WP_SENTRY:
//			ent.hModel = weapon_deployed->weaponModel;
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - WP_SENTRY\n" );
			//PKMOD - Ergodic 07/11/01 - rotate autosentry by 90 degrees
			// calculate the axis
			vectoangles( cent->lerpAngles, hold_dir);
			hold_dir[0] = 0;
			hold_dir[1] -= 90;  //offset
			hold_dir[2] = 0;
			AnglesToAxis( hold_dir, ent.axis );

			//PKMOD - Ergodic 06/02/02 - set the autosentry frame
			ent.frame = 29;

			//PKMOD - Ergodic 06/23/01 - add team parameters
			switch ( cent->currentState.modelindex ) { 
				case TEAM_RED:
					ent.hModel = cgs.media.pkasentry_red;
					break;
				case TEAM_BLUE:
					ent.hModel = cgs.media.pkasentry_blue;
					break;
				default:
					ent.hModel = cgs.media.pkasentry;
					break;
			}
			break;
		case 	WP_BEARTRAP:
			ent.hModel = cgs.media.pkabeartrap;
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - WP_BEARTRAP\n" );
				//PKMOD - Ergodic 06/23/01 - add team parameters
			switch ( cent->currentState.modelindex ) { 
				case TEAM_RED:
					ent.hModel = cgs.media.pkabeartrap_red;
					break;
				case TEAM_BLUE:
					ent.hModel = cgs.media.pkabeartrap_blue;
					break;
				default:
					ent.hModel = cgs.media.pkabeartrap;
					break;
			}
			break;
		case 	WP_BEANS:
			ent.hModel = weapon_deployed->weaponModel;
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - WP_BEANS\n" );
			break;
						
		//PKMOD - Ergodic 08/29/01 - add case for deploying flag
		case 	PW_REDFLAG:
			//PKMOD - Ergodic 03/04/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - PW_REDFLAG\n" );
			ent.hModel = cgs.media.redFlagModel;
			break;

		//PKMOD - Ergodic 08/29/01 - add case for deploying flag
		case 	PW_BLUEFLAG:
			//PKMOD - Ergodic 03/04/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - PW_BLUEFLAG\n" );
			ent.hModel = cgs.media.blueFlagModel;
			break;

		//PKMOD - Ergodic 12/15/01 - add case for deploying gauntlet
		case 	WP_GAUNTLET:
			//PKMOD - Ergodic 12/15/01 - debug (inactive)
//			Com_Printf( "CG_Dragon_Deploy - WP_GAUNTLET\n" );
			//PKMOD - Ergodic 12/16/01 - add new model for repositioned deployed gauntlet blade
			//PKMOD - Ergodic 12/17/01 - use the barrel model
			weapon = &cg_weapons[ WP_GAUNTLET ];
			vectoangles( cent->lerpAngles, hold_dir);
			//PKMOD - Ergodic 12/20/01 - rotate the blade to the proper angle
			hold_dir[PITCH] += 90;
			AnglesToAxis( hold_dir, ent.axis );

			//PKMOD - Ergodic 12/17/01 - use the barrel model
			ent.hModel = weapon->barrelModel;
			break;

		default:	//exit since current weapon can not be deployed (default to tongue)
			ent.hModel = weapon->weaponModel;
			//PKMOD - Ergodic 03/01/01 - debug (inactive)
//			Com_Printf( "CG_Weapon_DragonDeploy - invalid weapon for dragon deploy\n" );
			break;
	}


	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
//	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
//		ent.axis[0][1] = 1;
//		ent.axis[0][1] = 1;
//		ent.axis[0][0] = 1;
//	}


	// spin as it moves (removed 07/11/01)
//	RotateAroundDirection( ent.axis, s1->time );

	// add to refresh list, possibly with quad glow
	CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
}

/*
====================
PKMOD - Ergodic 11/02/02 - Code was moved from cg_localents to fix Invisible Gravity Well Bug
			Timing logic was copied from cg_autosentry
====================
*/
void CG_GravityWell_Activate( centity_t *cent ) {
	refEntity_t			orb;
	entityState_t		*s1;

	float		c;
	vec3_t		axis[3];
	vec3_t		hold_dir;
	int			t;
	float		dynamic_lumens;
	vec3_t		dynamic_color;
	int			frame;
	float		size;
	float		wsize;
	//particle parameters
	vec3_t		vel;
	int			duration;
	float		x_offset;
	float		y_offset;
	float		speed;
	int			indx;
	vec3_t		spark_origin;

	//get entity state information
	s1 = &cent->currentState;

	// create the render entity
	memset (&orb, 0, sizeof(orb));
	VectorCopy( cent->lerpOrigin, orb.origin);
	VectorCopy( cent->lerpOrigin, orb.oldorigin);
//	orb.reType = RT_MODEL;
	orb.hModel = cgs.media.pkagravitywelluniverse;
	//03/18/01 - add a shader
//	orb.customShader = cgs.media.quadShader;
	orb.skinNum = cg.clientFrame & 1;
	orb.renderfx = RF_MINLIGHT | RF_NOSHADOW;

	// calculate the axis
	VectorClear( hold_dir );
	AnglesToAxis( hold_dir, axis );

	//PKMOD - Ergodic 11/02/02 - debug invisible GW (inactive)
//	Com_Printf( "CG_GravityWell_Activate - cg.time>%d<, time2>%d<, cent->miscTime>%d<\n", cg.time, s1->time2, cent->miscTime );

	//PKMOD - Ergodic 11/03/02 - Use the server's level.time to drive the Gravity Well animations
	//			this will address the invisible Gravity Well bug
	t = cg.time - s1->time2;
	//calculate the time
	//if ( cent->miscTime )
	//	t = cg.time - cent->miscTime;
	//else
		//PKMOD - Ergodic 10/22/02 - default msec to 0
		//msec = 29; //was 29 -> produced a deployed sentry when launched
		//PKMOD - Ergodic 11/02/02 - debug invisible GW
		//		t = 0;
	//	t = 1000;

	//PKMOD - Ergodic 03/26/01 - add dynamic lighting
	MAKERGB( dynamic_color, 0.5f, 0.5f, 1.f );

	//++++++++++++++++++++++
	//	First Expansion
	//++++++++++++++++++++++
	if (t >= GWELL_EXPAND_STARTTIME_1 && t < GWELL_EXPAND_ENDTIME_1) {

//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
		orb.hModel = cgs.media.pkagravitywelluniverse;

		size = (float)(t - GWELL_EXPAND_STARTTIME_1) / (float)(GWELL_EXPAND_ENDTIME_1 - GWELL_EXPAND_STARTTIME_1);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;


		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 100 * size;
		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );

		VectorScale( axis[0], size * GWELL_ORB_RADIUS_1, orb.axis[0] );
		VectorScale( axis[1], size * GWELL_ORB_RADIUS_1, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_1, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_EXPAND_FADETIME_1) {
			c = (float)(t - GWELL_EXPAND_FADETIME_1) / (float)(GWELL_EXPAND_ENDTIME_1 - GWELL_EXPAND_FADETIME_1);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );
		return;// 03/25/01 - exit
	}

	//++++++++++++++++++++++
	//	First Contraction
	//++++++++++++++++++++++
	if (t >= GWELL_CONTRACTION_STARTTIME_1 && t < GWELL_CONTRACTION_ENDTIME_1) {

//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
//		orb.hModel = cgs.media.pkagravitywell_contraction;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model
		orb.hModel = cgs.media.pkagravitywelluniverse;


		size = 0.75 + 0.25 * (float)(GWELL_CONTRACTION_ENDTIME_1 - t) / (float)(GWELL_CONTRACTION_ENDTIME_1 - GWELL_CONTRACTION_STARTTIME_1);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;

		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 50 * size;
		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );

		VectorScale( axis[0], size * GWELL_ORB_RADIUS_1, orb.axis[0] );
		VectorScale( axis[1], size * GWELL_ORB_RADIUS_1, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_1, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_CONTRACTION_FADETIME_1) {
			c = (float)(t - GWELL_CONTRACTION_FADETIME_1) / (float)(GWELL_CONTRACTION_ENDTIME_1 - GWELL_CONTRACTION_FADETIME_1);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		return;// 03/25/01 - exit
	}

	//++++++++++++++++++++++
	//	Second Expansion
	//++++++++++++++++++++++
	if (t >= GWELL_EXPAND_STARTTIME_2 && t < GWELL_EXPAND_ENDTIME_2) {

		//PKMOD - Ergodic 10/13/02 - frame animations...
		//PKMOD - Ergodic 10/17/02 - frame animations...
		//  Start frame animation for sphere crimping
		//  20 frames / second = 1 new frame every 50 ms
		frame = (t - GWELL_EXPAND_STARTTIME_2) / 50;
		if ( frame > 29 )
			orb.frame = 29;
		else
			orb.frame = frame;

//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
		orb.hModel = cgs.media.pkagravitywelluniverse;

		size = 0.75 + (float)(t - GWELL_EXPAND_STARTTIME_2) / (float)(GWELL_EXPAND_ENDTIME_2 - GWELL_EXPAND_STARTTIME_2);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;

		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 150 * size;
		if ( dynamic_lumens > 255 )
			dynamic_lumens = 255;

		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );


		VectorScale( axis[0], size * GWELL_ORB_RADIUS_2, orb.axis[0] );
		VectorScale( axis[1], size * GWELL_ORB_RADIUS_2, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_2, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_EXPAND_FADETIME_2) {
			c = (float)(t - GWELL_EXPAND_FADETIME_2) / (float)(GWELL_EXPAND_ENDTIME_2 - GWELL_EXPAND_FADETIME_2);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );
		return;// 03/25/01 - exit
	}

	//++++++++++++++++++++++
	//	Second Contraction
	//++++++++++++++++++++++
	if (t >= GWELL_CONTRACTION_STARTTIME_2 && t < GWELL_CONTRACTION_ENDTIME_2) {

		//PKMOD - Ergodic 10/17/02 - frame animations...
		//  Start frame animation for sphere crimping
		//  20 frames / second = 1 new frame every 50 ms
		frame = (t - GWELL_EXPAND_STARTTIME_2) / 50;
		if ( frame > 79 )
			orb.frame = 40 + frame % 40;
		else
			orb.frame = frame;

//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
//		orb.hModel = cgs.media.pkagravitywell_contraction;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model
		orb.hModel = cgs.media.pkagravitywelluniverse;

		size = 1.5 + 0.25 * (float)(GWELL_CONTRACTION_ENDTIME_2 - t) / (float)(GWELL_CONTRACTION_ENDTIME_2 - GWELL_CONTRACTION_STARTTIME_2);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;

		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 100 * size;
		if ( dynamic_lumens > 255 )
			dynamic_lumens = 255;

		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );


		VectorScale( axis[0], size * GWELL_ORB_RADIUS_2, orb.axis[0] );
		VectorScale( axis[1], size * GWELL_ORB_RADIUS_2, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_2, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_CONTRACTION_FADETIME_2) {
			c = (float)(t - GWELL_CONTRACTION_FADETIME_2) / (float)(GWELL_CONTRACTION_ENDTIME_2 - GWELL_CONTRACTION_FADETIME_2);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		//+++
		//PKMOD - Ergodic 10/15/02 - Add the gravity well expanding wave
		//+++
		wsize = 0.5 * (float)(t - GWELL_CONTRACTION_STARTTIME_2) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_CONTRACTION_STARTTIME_2);
		orb.hModel = cgs.media.pkagravitywellwave;

		VectorScale( axis[0], wsize * GWELL_ORB_RADIUS_2, orb.axis[0] );
		VectorScale( axis[1], wsize * GWELL_ORB_RADIUS_2, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_2, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_EXPAND_FADETIME_3) {
			c = (float)(t - GWELL_EXPAND_FADETIME_3) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_EXPAND_FADETIME_3);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		return;// 03/25/01 - exit
	}

	//++++++++++++++++++++++
	//	Third Expansion
	//++++++++++++++++++++++
	if (t >= GWELL_EXPAND_STARTTIME_3 && t < GWELL_EXPAND_ENDTIME_3) {
		//PKMOD - Ergodic 10/17/02 - frame animations...
		//  Start frame animation for sphere crimping
		//  20 frames / second = 1 new frame every 50 ms
		frame = (t - GWELL_EXPAND_STARTTIME_2) / 50;
		if ( frame > 79 )
			orb.frame = 40 + frame % 40;
		else
			orb.frame = frame;

//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
		orb.hModel = cgs.media.pkagravitywelluniverse;

		size = 1.5 + (float)(t - GWELL_EXPAND_STARTTIME_3) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_EXPAND_STARTTIME_3);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;

		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 175 * size;
		if ( dynamic_lumens > 255 )
			dynamic_lumens = 255;

		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );


		VectorScale( axis[0], size * GWELL_ORB_RADIUS_3, orb.axis[0] );
		VectorScale( axis[1], size * GWELL_ORB_RADIUS_3, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_3, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_EXPAND_FADETIME_3) {
			c = (float)(t - GWELL_EXPAND_FADETIME_3) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_EXPAND_FADETIME_3);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		//+++
		//PKMOD - Ergodic 10/15/02 - Add the gravity well expanding wave
		//+++
		wsize = 0.75 * (float)(t - GWELL_CONTRACTION_STARTTIME_2) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_CONTRACTION_STARTTIME_2);
		orb.hModel = cgs.media.pkagravitywellwave;

		VectorScale( axis[0], wsize * GWELL_ORB_RADIUS_2, orb.axis[0] );
		VectorScale( axis[1], wsize * GWELL_ORB_RADIUS_2, orb.axis[1] );
		VectorScale( axis[2], size * GWELL_ORB_RADIUS_2, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_EXPAND_FADETIME_3) {
			c = (float)(t - GWELL_EXPAND_FADETIME_3) / (float)(GWELL_EXPAND_ENDTIME_3 - GWELL_EXPAND_FADETIME_3);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		return;// 03/25/01 - exit
	}

	//++++++++++++++++++++++
	//	Third Contraction
	//++++++++++++++++++++++
	if (t >= GWELL_CONTRACTION_STARTTIME_3 && t < GWELL_CONTRACTION_ENDTIME_3) {
		//PKMOD - Ergodic 10/17/02 - frame animations...
		//  Start frame animation for sphere crimping
		//  20 frames / second = 1 new frame every 50 ms
		frame = (t - GWELL_EXPAND_STARTTIME_2) / 50;
		if ( frame > 79 )
			orb.frame = 40 + frame % 40;
		else
			orb.frame = frame;

		//PKMOD - Ergodic 10/16/01 - debug last area (inactive)
//		if ( (t > ( GWELL_CONTRACTION_STARTTIME_3 + 300 )) && (t < ( GWELL_CONTRACTION_STARTTIME_3 + 700 ) ) ) {
//			Com_Printf("CG_GravityWell_Expand - t>%d<, frame>%d<, orb.frame>%d<\n", t, frame, orb.frame );
//		}


//		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.kamikazeExplodeSound );
//pkmod			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
//			le->leFlags |= LEF_SOUND1;
//		}

		//PKMOD - Ergodic 03/26/01 set model
//		orb.hModel = cgs.media.pkagravitywell_contraction;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model
		orb.hModel = cgs.media.pkagravitywelluniverse;

		size = (float)(GWELL_CONTRACTION_ENDTIME_3 - t) / (float)(GWELL_CONTRACTION_ENDTIME_3 - GWELL_CONTRACTION_STARTTIME_3);
		//PKMOD - Ergodic 02/27/01 - Debug force to One (inactive)
//		c = 0.25f;
		//PKMOD - Ergodic 10/13/02 - Add new animated gravity well model (resize)
		size = size / 3.0;

		//PKMOD - Ergodic 03/26/01 - add dynamic lighting
		dynamic_lumens = 50 + 125 * size;
		if ( dynamic_lumens > 255 )
			dynamic_lumens = 255;

		trap_R_AddLightToScene(orb.origin, dynamic_lumens, dynamic_color[0], dynamic_color[1], dynamic_color[2] );


		VectorScale( axis[0], 2.5 * size * GWELL_ORB_RADIUS_3, orb.axis[0] );
		VectorScale( axis[1], 2.5 * size * GWELL_ORB_RADIUS_3, orb.axis[1] );
		VectorScale( axis[2], 2.5 * size * GWELL_ORB_RADIUS_3, orb.axis[2] );
		orb.nonNormalizedAxes = qtrue;

		if (t > GWELL_CONTRACTION_FADETIME_3) {
			c = (float)(t - GWELL_CONTRACTION_FADETIME_3) / (float)(GWELL_CONTRACTION_ENDTIME_3 - GWELL_CONTRACTION_FADETIME_3);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		orb.shaderRGBA[0] = 0xff - c;
		orb.shaderRGBA[1] = 0xff - c;
		orb.shaderRGBA[2] = 0xff - c;
		orb.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &orb );

		//PKMOD - Ergodic 10/17/02 - draw some particles...
		VectorCopy(orb.origin, spark_origin);
		spark_origin[2] += 200;

		//PKMOD - Ergodic 03/18/04 - reduce index max from 10 to 4
		for ( indx = 1; indx < 4; indx++ ) {
			vel[0] = 10 * crandom();
			vel[1] = 10 * crandom();
			vel[2] = 10 * crandom();

			duration = 3000;

			x_offset = (t - GWELL_CONTRACTION_STARTTIME_3) * crandom() / 5;
			y_offset = (t - GWELL_CONTRACTION_STARTTIME_3) * crandom() / 5;

			speed = 10 * crandom();

			//PKMOD - Ergodic 10/18/02 - draw super-sized particles...
			CG_ParticleSparks2 (spark_origin, vel, duration, x_offset, y_offset, speed);
		}

		//PKMOD - Ergodic 10/18/02 - debug, count the number particle calls (inactive)
        //Com_Printf("CG_GravityWell_Expand\n");

		return;// 03/25/01 - exit
	}

	//PKMOD - Ergodic 01/04/02 - (inactive) debug, here is not part of gwell effect
//	Com_Printf("CG_GravityWell_Expand - delta time>%d<\n", t);

}


/*
==================
PKMOD - CG_Coord, Ergodic 01/21/01 display the Coord model 
==================
*/

//PKMOD - Ergodic 01/21/01 - add event to display coordinate model for exploding shells debug
void CG_Coord( entityState_t *es ) {
	localEntity_t	*le;
	float			light;
	vec3_t			lightColor;


	light = 100;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;
	//PKMOD - Ergodic 01/22/01 - debug location
	Com_Printf("CG_Coord: es->pos.trBase>%s<\n", CG_vtos(es->pos.trBase));


	le = CG_MakeExplosion( es->pos.trBase, 0, cgs.media.coordFlashModel, cgs.media.plasmaExplosionShader, 3000, qfalse );
//	le = CG_MakeExplosion( es->pos.trBase, es->angles, cgs.media.bulletFlashModel, cgs.media.plasmaExplosionShader, 3000, qfalse );
	le->light = light;
	VectorCopy( lightColor, le->lightColor );

}


//============================================================================

/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];
	//PKMOD - Ergodic 06/21/01 - mover is similar to zombie
	if ( ( cent->currentState.eType != ET_MOVER ) &&  ( cent->currentState.eType != ET_ZOMBIE ) ) {
		VectorCopy( in, out );
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );

	// FIXME: origin change when on a rotating object
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	vec3_t		current, next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) {
		CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {

	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) {
		// make sure the clients use TR_INTERPOLATE
		if ( cent->currentState.number < MAX_CLIENTS ) {
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
											cent->currentState.number < MAX_CLIENTS) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	//PKMOD - Ergodic 06/11/01 - if zombie mover...
	if ( cent->currentState.generic1 && ( cent->currentState.eType == ET_MOVER ) ) {
		//PKMOD - Ergodic 06/11/01 - for zombie
		vec3_t			hold_vec;
		float			hold_angle;

		BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

		//PKMOD - Ergodic 06/21/01 - rotate model 90 degrees "forward"
//		cent->lerpAngles[0] += 90;

		hold_angle = (cent->lerpAngles[2] + 90.0) * M_PI / 180.0;
		hold_vec[1] = (float) (cent->currentState.generic1 - 256) * cos( hold_angle );
		hold_vec[2] = (float) (cent->currentState.generic1 - 256) * sin( hold_angle );
		hold_vec[0] = 0;

		//PKMOD - Ergodic 06/11/01 - debug call
//		Com_Printf("CG_CalcEntityLerpPositions - deltaTime>%f<, trDelta>%s<\n", 100 * deltaTime, CG_vtos(cent->currentState.pos.trDelta) );

		VectorAdd( hold_vec, cent->currentState.pos.trBase, cent->lerpOrigin );

		//PKMOD - Ergodic 06/11/01 - debug call
//		Com_Printf("CG_CalcEntityLerpPositions - generic1>%d<, angle>%s<, location>%s<, hold>%s<\n", cent->currentState.generic1, CG_vtos(cent->lerpAngles), CG_vtos(cent->lerpOrigin), CG_vtos(hold_vec) );
	}
	else {
		// just use the current frame and evaluate as best we can
		BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
		BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
	}

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent != &cg.predictedPlayerEntity ) {
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
		cg.snap->serverTime, cg.time, cent->lerpOrigin );
	}
}

/*
===============
CG_TeamBase
===============
*/
static void CG_TeamBase( centity_t *cent ) {
	refEntity_t model;
#ifdef MISSIONPACK
	vec3_t angles;
	int t, h;
	float c;

	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF ) {
#else
	if ( cgs.gametype == GT_CTF) {
#endif
		// show the flag base
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );
		if ( cent->currentState.modelindex == TEAM_RED ) {
			model.hModel = cgs.media.redFlagBaseModel;
		}
		else if ( cent->currentState.modelindex == TEAM_BLUE ) {
			model.hModel = cgs.media.blueFlagBaseModel;
		}
		else {
			model.hModel = cgs.media.neutralFlagBaseModel;
		}
		trap_R_AddRefEntityToScene( &model );
	}
#ifdef MISSIONPACK
	else if ( cgs.gametype == GT_OBELISK ) {
		// show the obelisk
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );

		model.hModel = cgs.media.overloadBaseModel;
		trap_R_AddRefEntityToScene( &model );
		// if hit
		if ( cent->currentState.frame == 1) {
			// show hit model
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			//
			model.hModel = cgs.media.overloadEnergyModel;
			trap_R_AddRefEntityToScene( &model );
		}
		// if respawning
		if ( cent->currentState.frame == 2) {
			if ( !cent->miscTime ) {
				cent->miscTime = cg.time;
			}
			t = cg.time - cent->miscTime;
			h = (cg_obeliskRespawnDelay.integer - 5) * 1000;
			//
			if (t > h) {
				c = (float) (t - h) / h;
				if (c > 1)
					c = 1;
			}
			else {
				c = 0;
			}
			// show the lights
			AnglesToAxis( cent->currentState.angles, model.axis );
			//
			model.shaderRGBA[0] = c * 0xff;
			model.shaderRGBA[1] = c * 0xff;
			model.shaderRGBA[2] = c * 0xff;
			model.shaderRGBA[3] = c * 0xff;

			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene( &model );
			// show the target
			if (t > h) {
				if ( !cent->muzzleFlashTime ) {
					trap_S_StartSound (cent->lerpOrigin, ENTITYNUM_NONE, CHAN_BODY,  cgs.media.obeliskRespawnSound);
					cent->muzzleFlashTime = 1;
				}
				VectorCopy(cent->currentState.angles, angles);
				angles[YAW] += (float) 16 * acos(1-c) * 180 / M_PI;
				AnglesToAxis( angles, model.axis );

				VectorScale( model.axis[0], c, model.axis[0]);
				VectorScale( model.axis[1], c, model.axis[1]);
				VectorScale( model.axis[2], c, model.axis[2]);

				model.shaderRGBA[0] = 0xff;
				model.shaderRGBA[1] = 0xff;
				model.shaderRGBA[2] = 0xff;
				model.shaderRGBA[3] = 0xff;
				//
				model.origin[2] += 56;
				model.hModel = cgs.media.overloadTargetModel;
				trap_R_AddRefEntityToScene( &model );
			}
			else {
				//FIXME: show animated smoke
			}
		}
		else {
			cent->miscTime = 0;
			cent->muzzleFlashTime = 0;
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			// show the lights
			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene( &model );
			// show the target
			model.origin[2] += 56;
			model.hModel = cgs.media.overloadTargetModel;
			trap_R_AddRefEntityToScene( &model );
		}
	}
	else if ( cgs.gametype == GT_HARVESTER ) {
		// show harvester model
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy( cent->lerpOrigin, model.lightingOrigin );
		VectorCopy( cent->lerpOrigin, model.origin );
		AnglesToAxis( cent->currentState.angles, model.axis );

		if ( cent->currentState.modelindex == TEAM_RED ) {
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterRedSkin;
		}
		else if ( cent->currentState.modelindex == TEAM_BLUE ) {
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterBlueSkin;
		}
		else {
			model.hModel = cgs.media.harvesterNeutralModel;
			model.customSkin = 0;
		}
		trap_R_AddRefEntityToScene( &model );
	}
#endif
}

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

//PKMOD Ergodic debug 07/18/00 inactive
//	if ( cent->currentState.constantLight ) {
//		Com_Printf( "CG_AddCEntity constant light found\n" );
//	}

	// add automatic effects
	CG_EntityEffects( cent );

	switch ( cent->currentState.eType ) {
	default:
		CG_Error( "Bad entity type: %i\n", cent->currentState.eType );
		break;
//PKMOD - Ergodic 07/17/00 special spawn functions - do nothing 
	case ET_LIGHTNING_FX:
		//PKMOD Ergodic debug 07/18/00 (inactive)
//		Com_Printf( "ET_LIGHTNING_FX found\n" );
		break;
	case ET_INVISIBLE:
	case ET_PUSH_TRIGGER:
	//PKMOD - Ergodic 11/15/00 add functionality to make trigger_push silent
	case ET_QUIET_TRIGGER:
	case ET_TELEPORT_TRIGGER:
		break;
	case ET_GENERAL:
		CG_General( cent );
		break;
	//PKMOD - Ergodic 08/02/01 - create a new entity type for door_trigger
	//								fixes the bug of dragon deploy hitting doors
	case ET_DOOR_TRIGGER:
		CG_General( cent );
		break;
	//PKMOD - Ergodic 08/02/01 - create a new entity type for trigger_multiple
	//								fixes the bug of dragon deploy hitting doors
	case ET_TRIGGER_MULTIPLE:
		CG_General( cent );
		break;
	case ET_PLAYER:
		CG_Player( cent );
		break;
	case ET_ITEM:
		CG_Item( cent );
		break;
	case ET_MISSILE:
		CG_Missile( cent );
		break;
	case ET_MOVER:
		CG_Mover( cent );
		break;
	case ET_BEAM:
		CG_Beam( cent );
		break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
	case ET_SPEAKER:
		CG_Speaker( cent );
		break;
	case ET_GRAPPLE:
		CG_Grapple( cent );
		break;
	case ET_BEARTRAP:
		CG_BearTrap( cent );
		break;
	case ET_GRAVITY_WELL:
		//PKMOD - Ergodic 11/01/02 - This function call was moved from cg_event
		//			to fix the Invisible Gravity Well bug
		//CG_GravityWellActivate( cent->lerpOrigin );
		//PKMOD - Ergodic 11/02/02 - Pass in the whole centity
		//PKMOD - Ergodic 11/02/02 - Move function to cg_ents
		CG_GravityWell_Activate( cent );
		break;
	//PKMOD Ergodic - 07/12/2000, add chain lightning event type
	case ET_CHAIN_LIGHTNING:
		CG_ChainLightning( cent );
		break;
	case ET_BEARTRAP_FOLLOW:
		//PKMOD - Ergodic 06/30/00 do nothing since tracking is dealt with in cg_player
//		CG_BearTrap_Follow( cent );
		break;
	//PKMOD Ergodic - 08/03/2000, add nail event type
	case ET_NAIL:
		CG_Nail( cent );
		break;
	//PKMOD Ergodic - 11/26/2000, add deploy autosentry entity type
	case ET_AUTOSENTRY_DEPLOY:
		CG_AutoSentry( cent );
		break;
	//PKMOD Ergodic - 11/22/2000, add launch autosentry entity type
	//PKMOD Ergodic - 11/26/2000, separate the autosentry launch/deploy entity types
	case ET_AUTOSENTRY_LAUNCH:
		cent->miscTime = 0; //force the time to zero -> deploy0 model
		CG_AutoSentry( cent );
		break;
	//PKMOD Ergodic - 12/02/2000, add base autosentry entity type
	case ET_AUTOSENTRY_BASE:
		CG_AutoSentry_Base( cent );
		break;
	//PKMOD Ergodic - 12/02/2000, add turret autosentry entity type
	case ET_AUTOSENTRY_TURRET:
		CG_AutoSentry_Turret( cent );
		break;
	//PKMOD - Ergodic 03/14/01 - add dragon deployable weapon fire
	case ET_DRAGON_DEPLOY:
		CG_Dragon_Deploy( cent );
		break;
	//PKMOD - Ergodic 06/21/01 - zombie is similar to mover
	case ET_ZOMBIE:
		CG_Mover( cent );
		break;
	case ET_TEAM:
		CG_TeamBase( cent );
		break;
	//PKMOD - Ergodic 06/13/02 - the Personal Sentry driver entity should be ignored
	case ET_PERSONALSENTRY:
		break;

	//PKMOD Ergodic - 08/20/03, add Shooter_lightning entity type to differentiate between
	//			shooter_Lightning(green) and CLG lightning(red)
	case ET_SHOOTER_LIGHTNING:
		CG_ShooterLightning( cent );
		break;

	}
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	//PKMOD - Ergodic 01/27/02 - add slower rotating items
	cg.autoAnglesSlow[0] = 0;
	cg.autoAnglesSlow[1] = ( cg.time & 4095 ) * 360 / 4096.0f;
	cg.autoAnglesSlow[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );
	//PKMOD - Ergodic 01/27/02 - add slower rotating items
	AnglesToAxis( cg.autoAnglesSlow, cg.autoAxisSlow );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	CG_AddCEntity( &cg.predictedPlayerEntity );

	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}
}



