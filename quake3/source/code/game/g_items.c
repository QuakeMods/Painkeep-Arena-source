// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_ARMOR		25
#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define	RESPAWN_HOLDABLE	60
#define	RESPAWN_MEGAHEALTH	35//120
#define	RESPAWN_POWERUP		120
//PKMOD - Ergodic 07/05/00 set respawn times for PainkeepArena Items
#define	RESPAWN_PKA_LONG	95		//06/29/01 change from 105 to 95
#define	RESPAWN_PKA_EXTRA	75		//11/20/03 made for radiate
#define	RESPAWN_PKA_MEDIUM	45
#define	RESPAWN_PKA_SHORT	30


//======================================================================

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int			i;
	gclient_t	*client;

	if ( !other->client->ps.powerups[ent->item->giTag] ) {
		// round timing to seconds to make multiple powerup timers
		// count in sync
		other->client->ps.powerups[ent->item->giTag] = 
			level.time - ( level.time % 1000 );
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;

	// give any nearby players a "denied" anti-reward
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		vec3_t		delta;
		float		len;
		vec3_t		forward;
		trace_t		tr;

		client = &level.clients[i];
		if ( client == other->client ) {
			continue;
		}
		if ( client->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			continue;
		}

    // if same team in team game, no sound
    // cannot use OnSameTeam as it expects to g_entities, not clients
  	if ( g_gametype.integer >= GT_TEAM && other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
      continue;
    }

		// if too far away, no sound
		VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
		len = VectorNormalize( delta );
		if ( len > 192 ) {
			continue;
		}

		// if not facing, no sound
		//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//		AngleVectors( client->ps.viewangles, forward, NULL, NULL );
		AngleVectorsForward( client->ps.viewangles, forward );
		if ( DotProduct( delta, forward ) < 0.4 ) {
			continue;
		}

		// if not line of sight, no sound
		trap_Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID );
		if ( tr.fraction != 1.0 ) {
			continue;
		}

		// anti-reward
		client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
	}
	return RESPAWN_POWERUP;
}

//======================================================================

#ifdef MISSIONPACK
int Pickup_PersistantPowerup( gentity_t *ent, gentity_t *other ) {
	int		clientNum;
	char	userinfo[MAX_INFO_STRING];
	float	handicap;
	int		max;

	other->client->ps.stats[STAT_PERSISTANT_POWERUP] = ent->item - bg_itemlist;
	other->client->persistantPowerup = ent;

	switch( ent->item->giTag ) {
	case PW_GUARD:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		max = (int)(2 *  handicap);

		other->health = max;
		other->client->ps.stats[STAT_HEALTH] = max;
		other->client->ps.stats[STAT_MAX_HEALTH] = max;
		other->client->ps.stats[STAT_ARMOR] = max;
		other->client->pers.maxHealth = max;

		break;

	case PW_SCOUT:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		other->client->ps.stats[STAT_ARMOR] = 0;
		break;

	case PW_DOUBLER:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	case PW_AMMOREGEN:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		memset(other->client->ammoTimes, 0, sizeof(other->client->ammoTimes));
		break;
	default:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	}

	return -1;
}

//======================================================================
#endif

int Pickup_Holdable( gentity_t *ent, gentity_t *other ) {

//	other->client->ps.stats[STAT_HOLDABLE_ITEM] |= ent->item - bg_itemlist;

	//if the first holdable then set the active holdable
	if ( !other->client->ps.stats[STAT_HOLDABLE_ITEM] ) {
		other->client->ps.stats[STAT_ACTIVE_HOLDABLE] = ent->item->giTag;
	}

	//PKMOD - Ergodic 05/11/01 - allow holding of more than 1 type of
	//			holdable but only 1 of each kind
	other->client->ps.stats[STAT_HOLDABLE_ITEM] |= (1 << ent->item->giTag);

	//PKMOD - Ergodic 10/18/01 - TEST: set the modelindex2 if picking up the radiate holdable
//	if ( ent->item->giTag == HI_RADIATE )
//		other->client->ps.stats[STAT_PKA_ITEMS] |=  ( 1 << PKA_IRRADIATED ) ;

	//PKMOD Ergodic - 07/08/01, remove kamizaki entity flag
//	if( ent->item->giTag == HI_KAMIKAZE ) {
//		other->client->ps.eFlags |= EF_KAMIKAZE;
//	}

	//PKMOD Ergodic - 12/03/01, add differing respawn times for new holdables
	//PKMOD Ergodic - 12/16/02, tweak the respawn times
	if ( ent->item->giTag == HI_RADIATE )
		return RESPAWN_PKA_EXTRA + crandom() * 11;

	if ( ent->item->giTag == HI_PERSENTRY )
		return RESPAWN_HOLDABLE + 8 + crandom() * 15;

	if ( ent->item->giTag == HI_BOTLEGS )
		return RESPAWN_HOLDABLE + crandom() * 12;

	if ( ent->item->giTag == HI_BOTTORSO )
		return RESPAWN_HOLDABLE + crandom() * 11;

	if ( ent->item->giTag == HI_BOTHEAD )
		return RESPAWN_HOLDABLE + 10 + crandom() * 15;

	return RESPAWN_HOLDABLE;
}

//======================================================================

//PKMOD - Ergodic 02/08/04 - NOTES - these value should be in sync with void Add_Ammo in g_items.c

void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	ent->client->ps.ammo[weapon] += count;

//PKMOD - Ergodic 06/01/00 Add Max Inventory Capacities for PK Item ammo
	switch (weapon) {
	case WP_BEARTRAP:
		if ( ent->client->ps.ammo[weapon] > 3 ) {
			ent->client->ps.ammo[weapon] = 3;
		}
		break;
	case WP_SENTRY:
		if ( ent->client->ps.ammo[weapon] > 3 ) {
			ent->client->ps.ammo[weapon] = 3;
		}
		break;
	case WP_GRAVITY:
		if ( ent->client->ps.ammo[weapon] > 1 ) {
			ent->client->ps.ammo[weapon] = 1;
		}
		break;
	case WP_BEANS:
		if ( ent->client->ps.ammo[weapon] > 1 ) {
			ent->client->ps.ammo[weapon] = 1;
		}
		break;
//PKMOD - Ergodic 06/14/00 - add pickup of the exploding shotgun shells
//          Cap the Pickup at 10 shells
	case WP_EXPLODING_SHELLS:
		if ( ent->client->ps.ammo[weapon] > 10 ) {
			ent->client->ps.ammo[weapon] = 10;
		}
		break;
//PKMOD - Ergodic 05/18/01 - Airfist has displayable ammo corresponding to airfist_level
	case WP_AIRFIST:
		if ( ent->client->ps.ammo[weapon] > 4 ) {
			ent->client->ps.ammo[weapon] = 4;
		}
		break;

	default:
		if ( ent->client->ps.ammo[weapon] > 200 ) {
			ent->client->ps.ammo[weapon] = 200;
		}
	}
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

//PKMOD - Ergodic 06/14/00 - add pickup of the exploding shotgun shells
//          implies shotgun pickup of exploding shotgun only if player
//          cuurently has a regular shotgun
	if (ent->item->giTag == WP_EXPLODING_SHELLS) {
		if ( other->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_SHOTGUN) ) {
			other->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_EXPLODING_SHELLS );
		}
	}

	Add_Ammo (other, ent->item->giTag, quantity);

	return RESPAWN_AMMO;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other) {
	int		quantity;

	if ( ent->count < 0 ) {
		quantity = 0; // None for you, sir!
	} else {
		if ( ent->count ) {
			quantity = ent->count;
		} else {
			quantity = ent->item->quantity;
		}

		// dropped items and teamplay weapons always have full ammo
		if ( ! (ent->flags & FL_DROPPED_ITEM) && g_gametype.integer != GT_TEAM ) {
			// respawning rules
			// drop the quantity if the already have over the minimum
			if ( other->client->ps.ammo[ ent->item->giTag ] < quantity ) {
				quantity = quantity - other->client->ps.ammo[ ent->item->giTag ];
			} else {
				quantity = 1;		// only add a single shot
			}
		}
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );
//PKMOD - Ergodic 06/14/00 - add pickup of the exploding shotgun, implies shotgun pickup
//			This code should only be executed when player is carrying a dropped weapon
//          that is an exploding shells gun
	if (ent->item->giTag == WP_EXPLODING_SHELLS) {
		other->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SHOTGUN );
	}

//PKMOD - Ergodic 06/14/00 - add pickup of the shotgun implies
//          implies of exploding shotgun only if player
//          cuurently is carrying exploding shotgun shells
	if (ent->item->giTag == WP_SHOTGUN) {
		if ( other->client->ps.ammo[WP_EXPLODING_SHELLS] ) {
			other->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_EXPLODING_SHELLS );
		}
	}

	Add_Ammo( other, ent->item->giTag, quantity );

	if (ent->item->giTag == WP_GRAPPLING_HOOK)
		other->client->ps.ammo[ent->item->giTag] = -1; // unlimited ammo

//PKMOD - Ergodic New Weapons: Give Airfist unlimited ammo 02/19/00
//PKMOD - Ergodic 05/18/01 - Airfist has displayable ammo corresponding to airfist_level
//	if (ent->item->giTag == WP_AIRFIST)
//		other->client->ps.ammo[ent->item->giTag] = -1; // unlimited ammo

	//PKMOD - Ergodic 04/06/01 move TeamRespawn after PKA weapons

	//PKMOD - Ergodic 07/05/00 set respawn times for PainkeepArena Items
	if (ent->item->giTag == WP_GRAVITY)
		//PKMOD - Ergodic 05/16/01 - Gravity Wells don't spawn in for a while
		return RESPAWN_PKA_LONG + crandom() * RESPAWN_PKA_SHORT;

	if (ent->item->giTag == WP_SENTRY)
		return RESPAWN_PKA_SHORT;

	if (ent->item->giTag == WP_BEARTRAP)
		return RESPAWN_PKA_SHORT;

	if (ent->item->giTag == WP_BEANS)
		return RESPAWN_PKA_SHORT;

	// team deathmatch has slow weapon respawns
	if ( g_gametype.integer == GT_TEAM ) {
		return g_weaponTeamRespawn.integer;
	}

	return g_weaponRespawn.integer;
}


//======================================================================

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;

	// small and mega healths will go over the max
#ifdef MISSIONPACK
	if( other->client && bg_itemlist[other->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD ) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	}
	else
#endif
	if ( ent->item->quantity != 5 && ent->item->quantity != 100 ) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	} else {
		max = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}

	//PKMOD - Ergodic 06/18/00 change logic around to delay the commitment of added health
	if ( ent->count ) {
		quantity = other->health + ent->count;
	} else {
		quantity = other->health + ent->item->quantity;
	}

	if ( quantity > max ) {
		quantity = max;
	}

	//PKMOD - Ergodic 06/19/00 if health pickup could be used then save it and reset beans flag
	if ( other->health != quantity ) {
		other->health = quantity;
		other->client->ps.stats[STAT_HEALTH] = other->health;
		//PKMOD - Ergodic 06/18/00 turn off the beans status flag
		other->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_BEANS );
	}

	if ( ent->item->quantity == 100 ) {		// mega health respawns slow
		return RESPAWN_MEGAHEALTH;
	}

	return RESPAWN_HEALTH;
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) {
#ifdef MISSIONPACK
	int		upperBound;

	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;

	if( other->client && bg_itemlist[other->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD ) {
		upperBound = other->client->ps.stats[STAT_MAX_HEALTH];
	}
	else {
		upperBound = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}

	if ( other->client->ps.stats[STAT_ARMOR] > upperBound ) {
		other->client->ps.stats[STAT_ARMOR] = upperBound;
	}
#else
	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
	if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
		other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}
#endif

	return RESPAWN_ARMOR;
}

//======================================================================

//PKMOD - Ergodic 07/30/00 Move item back to original map location
//			after it was moved by airfist,gravity well, or grapple.
//			AirFist_Level will hold the item's move state flag (1==moved , 0==not moved)
void PKA_RestoreItem ( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	//PKMOD - Ergodic 07/30/00 debug - inactive
//	Com_Printf("PKA_RestoreItem - %s\n", ent->classname);
//	if (ent->item->giTag == WP_LIGHTNING )
//		Com_Printf("PKA_RestoreItem - eflags>%d<,  svFlags>%d<,  clipmask>%d<,  groundEntityNum>%d<,  spawnflags>%d<\n", ent->s.eFlags, ent->r.svFlags, ent->clipmask, ent->s.groundEntityNum, ent->spawnflags);
	
	
	//turn the item "off"
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->s.eFlags &= ~EF_BOUNCE_HALF;
	ent->r.contents = 0;

	//reset the items, position and move state
	VectorCopy(ent->pka_originalposition, ent->s.origin);

	G_SetOrigin( ent, ent->pka_originalposition);

	//PKMOD - Ergodic 07/31/00 make the object fall to the ground, must be done after G_SetOrigin
//	ent->s.pos.trType = TR_GRAVITY;

	ent->AirFist_Level = 0;

	//PKMOD - Ergodic 01/12/01 - drop item to floor if needed
	//			this code section was snitched from FinishSpawningItem
	if ( ent->spawnflags & 1 ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			//PKMOD - Ergodic 01/12/01 - this code should never be executed
			G_Printf ("PKA_RestoreItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}


	ent->nextthink = level.time + 1000; //respawn in 1 second
	ent->think = RespawnItem;

	trap_LinkEntity( ent );
}

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {

	//PKMOD - Ergodic 07/30/00 debug inactive
//	Com_Printf("RespawnItem - %s\n", ent->classname);


	// randomly select from teamed entities
	if (ent->team) {
		gentity_t	*master;
		int	count;
		int choice;

		if ( !ent->teammaster ) {
			G_Error( "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
			;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap_LinkEntity (ent);

	if ( ent->item->giType == IT_POWERUP ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/poweruprespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	if ( ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_KAMIKAZE ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/kamikazerespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	//PKMOD - Ergodic 07/30/00 debug inactive
//	if (ent->item->giTag == WP_LIGHTNING )
//		Com_Printf("7-RespawnItem(2) - %s : %s\n", ent->classname, vtos(ent->s.origin));


	//PKMOD - Ergodic 11/23/01 - if respawning a dropped item then it must be radiated
	if ( ent->flags & FL_DROPPED_ITEM ) {
		if (g_gametype.integer == GT_CTF && ent->item->giType == IT_TEAM) { // Special case for CTF flags
			ent->think = Team_DroppedFlagThink;
			ent->nextthink = level.time + 30000;
		} else { // auto-remove after 30 seconds
			ent->think = G_FreeEntity;
		//PKMOD - Ergodic 06/29/01 - weapons spew : change drop time from 30 seconds to 13 + random(0..3)
			ent->nextthink = level.time + 13000 + ( rand() % 4 );
		}
	}
	else {
		ent->nextthink = 0;
	}
}

/*
===============
RadiateThink
PKMOD - Ergodic 10/25/01 - add think function for radiation effect
===============
*/
#define	ITEM_RADIATION_DURATION 25000		//11/20/2003 decrease item radiation duration from 35 to 25 seconds
#define	PLAYER_RADIATION_DURATION 10000		//11/20/2003 Decrease from 12 to 10 seconds

void PlayerRadiateThink (gentity_t *ent) {

	int		radiation_damage;

	//remove radiation if owning player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		//PKMOD - Ergodic 10/29/01 - remove the radiation entity
		G_FreeEntity( ent );
		return;
	}

	//remove radiation if player has died
	if (( ent->target_ent->client->ps.pm_type == PM_DEAD ) || ( ent->target_ent->s.eFlags == EF_DEAD )) {
		//PKMOD - Ergodic 11/30/01 - remove the player's radiation
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_IRRADIATED );
		G_FreeEntity( ent );
		return;
	}

	//remove radiation if player's health drops to, or below, zero 
	if ( ent->target_ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		//PKMOD - Ergodic 11/30/01 - remove the player's radiation
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_IRRADIATED );
		G_FreeEntity( ent );
		return;
	}

	//PKMOD - Ergodic 11/20/03 - new function of the Battle Suit
	//remove radiation if player has the BATTLESUIT 
	if ( ent->target_ent->client->ps.powerups[PW_BATTLESUIT] ) {
		//PKMOD - Ergodic 11/30/01 - remove the player's radiation
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_IRRADIATED );
		G_FreeEntity( ent );
		return;
	}

/*
	//PKMOD - Ergodic 10/29/01 - add a radius damage to the radiation
	if ( ent->parent->client->ps.powerups[PW_QUAD] ) {
		splashdamage = 45 + rand() % 15;
		splashradius = 150 + rand() % 30;
	}
	else {
		splashdamage = 20 + rand() % 10;
		splashradius = 120 + rand() % 30;
	}

	dummy = G_RadiusDamage( ent->parent->r.currentOrigin, ent->parent, splashdamage, splashradius, ent->parent, MOD_BEANS_BLAST ); 
*/

	//PKMOD - Ergodic 10/29/01 - apply the radiation damage
	//PKMOD - Ergodic 11/20/03 - increase the radiation damage
//	radiation_damage = 1 + random() * 2;		// either 1 or 2 damage
	radiation_damage = 2 + random() * 2;		// either 2 or 3 damage
	G_Damage( ent->target_ent, ent, ent->parent, NULL, NULL, radiation_damage, 0, MOD_RADIATION);

	if ( ent->wait > level.time ) {
		ent->eventTime = level.time;
		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	}
	else {
		//PKMOD - Ergodic 11/30/01 - remove the player's radiation
		ent->target_ent->client->ps.stats[STAT_PKA_ITEMS] &= ~( 1 << PKA_IRRADIATED );
		G_FreeEntity( ent );
	}
}

void ItemRadiateThink (gentity_t *ent) {

	//remove radiation if owning player has disconnected
	if (!strcmp( ent->parent->classname,"disconnected" ) || ( ent->parent->client->pers.connected == CON_DISCONNECTED )) {
		//PKMOD - Ergodic 10/29/01 - remove the item's radiation
		ent->prevTrain->pka_flags &= ~PKAEF_IRRADIATED;
		//PKMOD - Ergodic 10/29/01 - set the entitystate for cgame communication
		ent->s.modelindex2 = ent->pka_flags;
		G_FreeEntity( ent );
		return;
	}

	if ( ent->wait > level.time ) {
		ent->eventTime = level.time;
		ent->nextthink = level.time + 300;
		trap_LinkEntity( ent );
	}
	else {
		//PKMOD - Ergodic 10/29/01 - remove the item's radiation
		ent->prevTrain->pka_flags &= ~PKAEF_IRRADIATED;
		//PKMOD - Ergodic 10/29/01 - set the entitystate for cgame communication
		ent->prevTrain->s.modelindex2 = ent->pka_flags;
		G_FreeEntity( ent );

	}
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;
	qboolean	predict;

	//PKMOD - Ergodic 10/05/00 - debug inactive 
//	Com_Printf( "Touch_Item other:>%s<\n", other->classname );

	if (!other->client)
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

	//PKMOD - Ergodic 02/19/01 - if item was on dragon, then end the dragon
	//			do this code before BG_CanItemBeGrabbed becuase item on dragon
	//			may not be able to be picked up due to limits
	if ( other->client->hook ) {		//if other has hook
		//if item on hook is the same as item just touched
		if ( other->client->hook->target_ent->s.number == ent->s.number ) { 
			Weapon_HookFree( other->client->hook );
		}
	}

	//PKMOD - Ergodic 10/25/01 - check if item is irradiated
	//PKMOD - Ergodic 10/26/01 - and player is not radiated
	//PKMOD - Ergodic 10/29/01 - radiation logic
	//			case 0: Item NOT radiated and Player NOT radiated
	//				action 0: Pick up item
	//			case 1: Item NOT radiated and Player radiated
	//				action 1: Radiate Item and NO item pickup 
	//			case 2: Item radiated and Player NOT radiated
	//				action 2: Radiate Player and pickup item 
	//			case 3: Item radiated and Player radiated
	//				action 3: Player NOT Radiated and pickup item 
	//
	//	Note: case 2 and case 3 result in item pickup
	//---------------

	//			case 1: Item NOT radiated and Player radiated
	//				action 1: Radiate Item and NO item pickup 
	//PKMOD - Ergodic 10/26/01 - check if player is irradiated
	if ( ( other->client->ps.powerups[PW_RADIATE] > 0 ) && !( ent->pka_flags & PKAEF_IRRADIATED ) ) {
		gentity_t	*radiate;

		//turn item radiation on
		ent->pka_flags |= PKAEF_IRRADIATED;
		//PKMOD - Ergodic 10/29/01 - set the entitystate for cgame communication
		ent->s.modelindex2 = ent->pka_flags;

		//spawn a radiation entity
		radiate = G_Spawn();
		radiate->classname = "radiate";
		radiate->s.eType = ET_GENERAL;
		radiate->r.ownerNum = other->s.number;
		radiate->parent = other;
		radiate->think = ItemRadiateThink;
		radiate->wait = level.time + ITEM_RADIATION_DURATION + 5000 * random(); //Radiate for 20 seconds + 5 random seconds
		radiate->nextthink = level.time + 300;
		radiate->eventTime = level.time;
		//PKMOD - Ergodic 10/28/01 - store the item entity int the radiation entity
		radiate->prevTrain = ent;

		trap_LinkEntity( radiate );

		//PKMOD - Ergodic 10/28/01 - store the radiation entity in the item
		ent->prevTrain = radiate;

		//PKMOD - Ergodic 10/29/01 - make item invisible for a few moments
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;

		ent->nextthink = level.time + 4000 + 2000 * random();	//average 5 second

		//PKMOD - Ergodic, 07/30/00 item move logic
		if ( ent->AirFist_Level == 1 )
			ent->think = PKA_RestoreItem;
		else
			ent->think = RespawnItem;

		trap_LinkEntity( ent );

		//PKMOD - Ergodic 12/05/01 - play the Radiate Item sound
		if (predict) {
			G_AddPredictableEvent( other, EV_ITEM_RADIATE, ent->s.modelindex );
		} else {
			G_AddEvent( other, EV_ITEM_RADIATE, ent->s.modelindex );
		}

		return;
	}

	//			case 2: Item radiated and Player NOT radiated
	//				action 2: Radiate Player and pickup item
	//PKMOD - Ergodic 11/02/03 - don't infect player if he has the BATTLESUIT 
	if ( ( ent->pka_flags & PKAEF_IRRADIATED ) && ( other->client->ps.powerups[PW_RADIATE] == 0 ) 
		&& ( other->client->ps.powerups[PW_BATTLESUIT] == 0 ) ) {
		gentity_t	*radiate;

		//turn radiation off
		ent->pka_flags &= ~PKAEF_IRRADIATED;
		//PKMOD - Ergodic 11/17/01 - set the entitystate for cgame communication
		ent->s.modelindex2 = ent->pka_flags;

		//spawn a radiation entity
		radiate = G_Spawn();
		radiate->classname = "radiate";
		radiate->s.eType = ET_GENERAL;
		radiate->r.ownerNum = ent->prevTrain->r.ownerNum;
		radiate->parent = ent->prevTrain->parent;
		radiate->think = PlayerRadiateThink;
		radiate->target_ent = other;
		radiate->wait = level.time + PLAYER_RADIATION_DURATION + 5000 * random(); //Follow for 10 seconds + 5 random seconds
		radiate->nextthink = level.time + 300;
		radiate->eventTime = level.time;

		trap_LinkEntity( radiate );

		//PKMOD - Ergodic 11/30/01 - remove the player's radiation
		other->client->ps.stats[STAT_PKA_ITEMS] |= ( 1 << PKA_IRRADIATED );

	}


	// the same pickup rules are used for client side and server side
	if ( !BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps ) ) {
		return;
	}

	G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );

	predict = other->client->pers.predictItemPickup;
	//PKMOD - Ergodic 07/30/00 debug inactive
//	if (ent->item->giTag == WP_LIGHTNING ) {
//		Com_Printf("Touch_Item %s : %s TrType:%d\n", ent->classname, vtos(ent->s.origin), ent->s.pos.trType);
//		Com_Printf("Touch_Item - eflags>%d<,  svFlags>%d<,  clipmask>%d<,  groundEntityNum>%d<,  spawnflags>%d<\n", ent->s.eFlags, ent->r.svFlags, ent->clipmask, ent->s.groundEntityNum, ent->spawnflags);
//	}


	// call the item-specific pickup function
	switch( ent->item->giType ) {
	case IT_WEAPON:
		respawn = Pickup_Weapon(ent, other);
//		predict = qfalse;
		break;
	case IT_AMMO:
		respawn = Pickup_Ammo(ent, other);
//		predict = qfalse;
		break;
	case IT_ARMOR:
		respawn = Pickup_Armor(ent, other);
		break;
	case IT_HEALTH:
		respawn = Pickup_Health(ent, other);
		break;
	case IT_POWERUP:
		respawn = Pickup_Powerup(ent, other);
		predict = qfalse;
		break;
#ifdef MISSIONPACK
	case IT_PERSISTANT_POWERUP:
		respawn = Pickup_PersistantPowerup(ent, other);
		break;
#endif
	case IT_TEAM:
		respawn = Pickup_Team(ent, other);
		break;
	case IT_HOLDABLE:
		respawn = Pickup_Holdable(ent, other);
		break;
	//PKMOD - Ergodic 09/21/00 - voting entity touch function
	//PKMOD - Ergodic 09/24/00 - add voting mode stat
	case IT_VOTING:
	//PKMOD - Ergodic 01/19/02 - reuse STAT_VOTING_MODE field for other purposes
//		if ( !other->client->ps.stats[STAT_VOTING_MODE] ) {  // don't re-vote 
//			other->client->ps.stats[STAT_VOTING_MODE] = 1;
			//PKMOD Ergodic 09/27/00 - debug
//			Com_Printf("Touch_Item - map index (ent->count) map name >%d<\n", ent->count );
//			Com_Printf("Touch_Item - map name >%s<\n", hubInfo[ent->count].map_name );
//			Com_Printf("Touch_Item - map title >%s<\n", hubInfo[ent->count].map_title );
//			Com_Printf("Touch_Item - map shader >%s<\n", hubInfo[ent->count].map_shader_index );
//		}
		return;
		break;

	default:
		return;
	}

	if ( !respawn ) {
		return;
	}

	//			case 2: Item radiated and Player NOT radiated
	//				action 2: play Radiate Player sound 
	if ( ( ent->pka_flags & PKAEF_IRRADIATED ) && ( other->client->ps.powerups[PW_RADIATE] == 0 ) ) {
		//PKMOD - Ergodic 12/05/01 - play the Radiate Item sound
		if (predict) {
			G_AddPredictableEvent( other, EV_PLAYER_RADIATE, ent->s.modelindex );
		} else {
			G_AddEvent( other, EV_PLAYER_RADIATE, ent->s.modelindex );
		}
	}
	else {
		int		hold_event = EV_ITEM_PICKUP;

		//PKMOD - Ergodic 02/07/02 - check for newly completed Private Bot
		if ( ( ent->item->giType == IT_HOLDABLE ) && ( ( ent->item->giTag >= HI_BOTLEGS ) && ( ent->item->giTag <= HI_BOTHEAD ) ) ) {
			if ( ( other->client->ps.stats[STAT_HOLDABLE_ITEM] & ( 7 << HI_BOTLEGS ) ) == ( 7 << HI_BOTLEGS ) )
				hold_event = EV_COMPLETED_PRIVATEBOT;
		}
			
		// play the normal pickup sound
		if (predict) {
			G_AddPredictableEvent( other, hold_event, ent->s.modelindex );
		} else {
			G_AddEvent( other, hold_event, ent->s.modelindex );
		}
	}

	// powerup pickups are global broadcasts
	if ( ent->item->giType == IT_POWERUP || ent->item->giType == IT_TEAM) {
		// if we want the global sound to play
		if (!ent->speed) {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			te->r.svFlags |= SVF_BROADCAST;
		} else {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			// only send this temp entity to a single client
			te->r.svFlags |= SVF_SINGLECLIENT;
			te->r.singleClient = other->s.number;
		}
	}

	// fire item targets
	G_UseTargets (ent, other);

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	//PKAMOD - Ergodic 04/06/01 - ignore "wait" on PKA items 
	if ( ( ent->item->giType != IT_WEAPON ) || ( ( ent->item->giTag < WP_GRAVITY ) || ( ent->item->giTag > WP_BEANS ) ) ) {
		// non zero wait overrides respawn time
		if ( ent->wait ) {
			respawn = ent->wait;
		}

		// random can be used to vary the respawn time
		if ( ent->random ) {
			respawn += crandom() * ent->random;
			if ( respawn < 1 ) {
				respawn = 1;
			}
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	// ZOID
	// A negative respawn times means to never respawn this item (but don't 
	// delete it).  This is used by items that are respawned by third party 
	// events such as ctf flags
	if ( respawn <= 0 ) {
		//PKMOD - Ergodic, 07/30/00 item move logic
		ent->AirFist_Level = 0;

		ent->nextthink = 0;
		ent->think = 0;
	} else {
		ent->nextthink = level.time + respawn * 1000;

		//PKMOD - Ergodic, 07/30/00 item move logic
		if ( ent->AirFist_Level == 1 )
			ent->think = PKA_RestoreItem;
		else
			ent->think = RespawnItem;
	}
	trap_LinkEntity( ent );
}


//======================================================================
//PKMOD - Ergodic 11/16/00 - add target_remove code
//		only remove the holdable if you have the specific holdable
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Holdable( gentity_t *ent, gentity_t *other ) {
	
	if ( other->client->ps.stats[STAT_HOLDABLE_ITEM] == ( ent->item - bg_itemlist ) ) {
		other->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
		return qtrue;
	}

	return qfalse;
}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;
	int		hold_return;

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	//removing exploding shotgun shells implies possibly removing
	//   the exploding shells gun
	if (ent->item->giTag == WP_EXPLODING_SHELLS) {
		//if you are removing all exploding shells then remove the weapon 
		if ( quantity >= other->client->ps.ammo[ent->item->giTag] ) 
			other->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_EXPLODING_SHELLS );
	}
	
	quantity = other->client->ps.ammo[ent->item->giTag] - quantity;
	
	//PKMOD - Ergodic 11/20/00 - if removing ammo then return qtrue
	if ( other->client->ps.ammo[ent->item->giTag] > 0)
		hold_return = qtrue;
	else
		hold_return = qfalse;

	if ( quantity <= 0 )
		other->client->ps.ammo[ent->item->giTag] = 0;
	else
		other->client->ps.ammo[ent->item->giTag] = quantity;

	return hold_return;
}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Weapon (gentity_t *ent, gentity_t *other) {
	int		hold_return;
	//PKMOD - Ergodic 11/19/00 - debug inactive
//	Com_Printf( "Remove_Weapon: classname:>%s<\n", ent->classname );

	//PKMOD - Ergodic 11/20/00 - if removing weapon then set return to qtrue
	if ( other->client->ps.stats[STAT_WEAPONS] & ( 1 << ent->item->giTag ) ) 
		hold_return = qtrue;
	else
		hold_return = qfalse;

	// remove the weapon
	other->client->ps.stats[STAT_WEAPONS] &= ~( 1 << ent->item->giTag );

	//if removing the exploding shotgun then also remove the non-exploding shotgun
	if ( ent->item->giTag == WP_EXPLODING_SHELLS ) {
		if ( other->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_SHOTGUN ) ) {
			hold_return = qtrue;
			other->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_SHOTGUN );
		}
	}

	//if removing the shotgun then also remove the exploding shells shotgun
	if ( ent->item->giTag == WP_SHOTGUN ) {
		if ( other->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_EXPLODING_SHELLS ) ) {
			hold_return = qtrue;
			other->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_EXPLODING_SHELLS );
		}
	}

	//if removing a pkaitem then remove all of the ammo
	if (ent->item->giTag == WP_GRAVITY)
		other->client->ps.ammo[ent->item->giTag] = 0;

	if (ent->item->giTag == WP_SENTRY)
		other->client->ps.ammo[ent->item->giTag] = 0;

	if (ent->item->giTag == WP_BEARTRAP)
		other->client->ps.ammo[ent->item->giTag] = 0;

	if (ent->item->giTag == WP_BEANS)
		other->client->ps.ammo[ent->item->giTag] = 0;

	return hold_return;
}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Health (gentity_t *ent, gentity_t *other) {
	int			quantity;

	//PKMOD - Ergodic 06/18/00 change logic around to delay the commitment of added health
	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	G_Damage (other, NULL, NULL, NULL, NULL, quantity, DAMAGE_NO_ARMOR, MOD_TRIGGER_HURT);

	return qtrue;

}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Armor( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int		hold_return;

	//PKMOD - Ergodic 11/20/00 - if removing armor then set return to qtrue
	if ( other->client->ps.stats[STAT_ARMOR] ) 
		hold_return = qtrue;
	else
		hold_return = qfalse;

	quantity = other->client->ps.stats[STAT_ARMOR] - ent->item->quantity;

	if ( quantity <= 0 )
		other->client->ps.stats[STAT_ARMOR] = 0;
	else
		other->client->ps.stats[STAT_ARMOR] = quantity;

	return hold_return;
}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
qboolean Remove_Powerup( gentity_t *ent, gentity_t *other ) {

	//PKMOD - Ergodic 11/20/00 - debug inactive
//	Com_Printf( "Remove_Powerup other->client->ps.powerups[ent->item->giTag]>%d<\n", other->client->ps.powerups[ent->item->giTag] );

	//do nothing and return if client does not have particular powerup 
	if ( other->client->ps.powerups[ent->item->giTag] == 0 )
		return qfalse;
		
	other->client->ps.powerups[ent->item->giTag] = 0;

	return qtrue;
}

//PKMOD - Ergodic 11/16/00 - add target_remove code
//PKMOD - Ergodic 11/20/00 - return qboolean for whether item was removed or not
void Touch_Item_Remove (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int		hold_return;

	//PKMOD - Ergodic 11/20/00 - debug inactive
//	Com_Printf( "Touch_Item_Remove ent:>%s<\n", ent->classname );

	if (!other->client)
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

	hold_return = qfalse;

	//PKMOD - Ergodic 11/20/00 - debug inactive
//	Com_Printf( "Touch_Item_Remove ent->item->giType:>%d<\n", ent->item->giType );

	// call the item-specific remove function
	switch( ent->item->giType ) {
	case IT_WEAPON:
		//PKMOD - Ergodic 11/20/00 - never remove the machinegun (default weapon)
		if ( ent->item->giTag == WP_MACHINEGUN )
			return;

		//PKMOD - Ergodic 11/20/00 - never remove the gauntlet (default melee weapon)
		if ( ent->item->giTag == WP_GAUNTLET )
			return;

		hold_return = Remove_Weapon(ent, other);
		break;
	case IT_AMMO:
		hold_return = Remove_Ammo(ent, other);
		break;
	case IT_ARMOR:
		hold_return = Remove_Armor(ent, other);
		break;
	case IT_HEALTH:
		hold_return = Remove_Health(ent, other);
		break;
	case IT_POWERUP:
		hold_return = Remove_Powerup(ent, other);
		break;
	case IT_HOLDABLE:
		hold_return = Remove_Holdable(ent, other);
		break;

	default:
		break;
	}

	//if we did not remove anything then just return
	if ( !hold_return )
		return;

	G_LogPrintf( "Item Removed: %i %s\n", other->s.number, ent->item->classname );

	// play the normal pickup sound
	if ( other->client->pers.predictItemPickup ) {
		G_AddPredictableEvent( other, EV_ITEM_REMOVE, ent->s.modelindex );
	} else {
		G_AddEvent( other, EV_ITEM_REMOVE, ent->s.modelindex );
	}

	// powerup pickups are global broadcasts
	if ( ent->item->giType == IT_POWERUP || ent->item->giType == IT_TEAM) {
		gentity_t	*te;

		//PKMOD - Ergodic 11/20/00 - debug inactive
//		Com_Printf( "Touch_Item_Remove calling EV_GLOBAL_ITEM_REMOVE\n" );

		te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_REMOVE );
		te->s.eventParm = ent->s.modelindex;
		te->r.svFlags |= SVF_BROADCAST;
	}

	// fire item targets
	G_UseTargets (ent, other);

}


/*
================
PKA_CloneItem

//PKMOD - Ergodic 01/11/01 Create a copy of that can be manipulated by the dragon and airfist
================
*/
/*PKA_CloneItem( gentity_t *ent ) {
	gentity_t	*clone;

	clone = G_Spawn();

}
*/


/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) {
	gentity_t	*dropped;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;
#ifdef MISSIONPACK
	if ((g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)			&& item->giType == IT_TEAM) { // Special case for CTF flags
#else
	if (g_gametype.integer == GT_CTF && item->giType == IT_TEAM) { // Special case for CTF flags
#endif
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + 30000;
		Team_CheckDroppedItem( dropped );
	} else { // auto-remove after 30 seconds
		dropped->think = G_FreeEntity;
		//PKMOD - Ergodic 06/29/01 - weapons spew : change drop time from 30 seconds to 13 + random(0..3)
		dropped->nextthink = level.time + 13000 + ( rand() % 4 );
	}

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity (dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
//PKMOD - Ergodic 11/23/01 - dropped items from radiated players will be radiated
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle ) {
	vec3_t	velocity;
	vec3_t	angles;
	gentity_t	*new_ent;		//PKMOD - Ergodic 11/23/01 - hold newly dropped item

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors( angles, velocity, NULL, NULL );
	AngleVectorsForward( angles, velocity );

	//PKMOD - Ergodic 08/06/00 - code variations for PKMOD weapon spew effect
	//				If angle is zero [less than 1] then code will default to original logic

	if ( angle < 1 ) {
		VectorScale( velocity, 150, velocity );
		velocity[2] += 200 + crandom() * 50;
	}
	else {
		//Weapon Spew code
		VectorScale( velocity, 150 + crandom() * 50, velocity ); //PKMOD - Ergodic 08/06/00 - increase randomness from 0 to 50
		velocity[2] += 200 + crandom() * 100;  //PKMOD - Ergodic 08/06/00 - increase randomness from 50 to 100
	}

	//PKMOD - Ergodic 11/23/01 - hold the new item
	new_ent = LaunchItem( item, ent->s.pos.trBase, velocity );

	//PKMOD - Ergodic 11/23/01 - pass radiation flag for dropped weapons
	if ( ent->client->ps.powerups[PW_RADIATE] > 0 ) {
		gentity_t	*radiate;		//PKMOD - Ergodic 11/23/01 - hold a new radiation entity

		//turn item radiation on
		new_ent->pka_flags |= PKAEF_IRRADIATED;
		//PKMOD - Ergodic 10/29/01 - set the entitystate for cgame communication
		new_ent->s.modelindex2 = new_ent->pka_flags;

		//spawn a radiation entity
		radiate = G_Spawn();
		radiate->classname = "radiate";
		radiate->s.eType = ET_GENERAL;
		radiate->r.ownerNum = ent->s.number;
		radiate->parent = ent;
		radiate->think = ItemRadiateThink;
		radiate->wait = level.time + ITEM_RADIATION_DURATION + 5000 * random(); //Radiate for 20 seconds + 5 random seconds
		radiate->nextthink = level.time + 300;
		radiate->eventTime = level.time;
		//PKMOD - Ergodic 10/28/01 - store the item entity int the radiation entity
		radiate->prevTrain = new_ent;

		trap_LinkEntity( radiate );

		//PKMOD - Ergodic 10/28/01 - store the radiation entity in the item
		new_ent->prevTrain = radiate;

	}

	return ( new_ent ); 
}

/*
================
Throw_Item

//PKMOD - Ergodic 07/25/01 - Spawns an item and throw it forwards
								used for dragon deploy of the beans
//PKMOD - Ergodic 08/29/01 -    and for deploying the flag (no code change)
================
*/
gentity_t *Throw_Item( gentity_t *ent, gitem_t *item ) {
	vec3_t	velocity;
	vec3_t	hold_direction;

	VectorNormalize2( ent->s.pos.trDelta, hold_direction );
	VectorScale( hold_direction, 600, velocity );

	//PKMOD - Ergodic 07/25/01 - debug item's velocity (inactive)
//	Com_Printf("Throw_Item - velocity>%s<\n", vtos( velocity ) );


	return LaunchItem( item, ent->r.currentOrigin, velocity );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;
	// useing an item causes it to respawn
	ent->use = Use_Item;

	//PKMOD - Ergodic 01/11/01 - debug inactive
//	Com_Printf( "FinishSpawningItem: classname>%s<, spawnflags>%d<\n", ent->classname, ent->spawnflags );

	if ( ent->spawnflags & 1 ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			G_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	// powerups don't spawn in for a while
	if ( ent->item->giType == IT_POWERUP ) {
		float	respawn;

		respawn = 45 + crandom() * 15;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	//PKMOD - Ergodic 05/16/01 - Gravity Wells don't spawn in for a while
	if ( ( ent->item->giType == IT_WEAPON ) && ( ent->item->giTag == WP_GRAVITY ) ) {
		float	respawn;

		respawn = RESPAWN_PKA_LONG + crandom() * RESPAWN_PKA_SHORT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	//PKMOD - Ergodic 12/16/02 - Personal Sentrys do not spawn in for a while
	if ( ( ent->item->giType == IT_HOLDABLE ) && ( ent->item->giTag == HI_PERSENTRY ) ) {
		float	respawn;

		respawn = RESPAWN_PKA_MEDIUM + crandom() * RESPAWN_PKA_SHORT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	//PKMOD - Ergodic 12/16/02 - Radiate does not spawn in for a while
	if ( ( ent->item->giType == IT_HOLDABLE ) && ( ent->item->giTag == HI_RADIATE ) ) {
		float	respawn;

		respawn = RESPAWN_PKA_MEDIUM + crandom() * RESPAWN_PKA_SHORT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}


	//PKMOD - Ergodic 12/16/02 - Private Bot (head) does not spawn in for a while
	if ( ( ent->item->giType == IT_HOLDABLE ) && ( ent->item->giTag == HI_BOTHEAD ) ) {
		float	respawn;

		respawn = RESPAWN_PKA_MEDIUM + crandom() * RESPAWN_PKA_SHORT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	trap_LinkEntity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

	// Set up team stuff
	Team_InitGame();

	if( g_gametype.integer == GT_CTF ) {
		gitem_t	*item;

		// check for the two flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
		}
	}
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_1FCTF ) {
		gitem_t	*item;

		// check for all three flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
		}
		item = BG_FindItem( "Neutral Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_neutralflag in map" );
		}
	}

	if( g_gametype.integer == GT_OBELISK ) {
		gentity_t	*ent;

		// check for the two obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map" );
		}
	}

	if( g_gametype.integer == GT_HARVESTER ) {
		gentity_t	*ent;

		// check for all three obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_neutralobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_neutralobelisk in map" );
		}
	}
#endif
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	RegisterItem( BG_FindItemForWeapon( WP_MACHINEGUN ) );
	RegisterItem( BG_FindItemForWeapon( WP_GAUNTLET ) );

	//PKMOD - Ergodic 06/25/00 register the exploding shells gun since players never
	//        pickup this item - but they will have it when exploding shells ammo is present 
	RegisterItem( BG_FindItemForWeapon( WP_EXPLODING_SHELLS ) );
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		G_Error( "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	G_Printf( "%i items registered\n", count );
	trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) {

	char name[128];

	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return trap_Cvar_VariableIntegerValue( name );
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {

	//PKMOD - Ergodic 06/25/00 remove the non-pka items from the map (plasmagun and BFG)
	if (!strcmp(ent->classname, "weapon_plasmagun")) {
		return;
	}
	if (!strcmp(ent->classname, "ammo_cells")) {
		return;
	}
	if (!strcmp(ent->classname, "weapon_bfg")) {
		return;
	}
	if (!strcmp(ent->classname, "ammo_bfg")) {
		return;
	}

	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );
	if ( G_ItemDisabled(item) )
		return;

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	if ( item->giType == IT_POWERUP ) {
		G_SoundIndex( "sound/items/poweruprespawn.wav" );
		G_SpawnFloat( "noglobalsound", "0", &ent->speed);
	}

#ifdef MISSIONPACK
	if ( item->giType == IT_PERSISTANT_POWERUP ) {
		ent->s.generic1 = ent->spawnflags;
	}
#endif

	//PKMOD - Ergodic 09/24/00 - encode the voting information into item->pickup_name
	if ( item->giType == IT_VOTING ) {

		//PKMOD - Ergodic 09/24/00 - create a holding variable
		char		hold_string[MAX_FILEPATH];
		char		*holdchar;
		int			vote_index;
//		int			indx;

		if ( Hub_Index >= MAX_HUB_INDEX ) {
			G_Error ( "G_SpawnItem: voting_mapname %s at location:%s exceeds the maximum voting_image entity count of MAX_HUB_INDEX(25)\n", ent->classname, vtos( ent->s.origin ) );
			return;
		}

		hubInfo[Hub_Index].map_votes = 0;		//set the vote counter to zero

		holdchar = hold_string;
		//verify map name...
		if ( !G_SpawnString( "voting_mapname", "No Map Name", &holdchar ) ) {
			G_Error ( "G_SpawnItem: %s doesn't have a voting_mapname at location:%s\n", ent->classname, vtos( ent->s.origin ) );
		}

		if ( strlen (holdchar) >= MAX_HUB_NAME ) {
			G_Error ( "G_SpawnItem: voting_mapname %s at location:%s is bigger than MAX_HUB_NAME (50) characters, please reduce filename size\n", ent->classname, vtos( ent->s.origin ) );
			return;
		}

		//Set the file name of the map
		strcpy( hubInfo[Hub_Index].map_name, holdchar );
//		strcpy( hold_mapname, holdchar );
		//PKMOD - Ergodic 09/24/00 - debug 
//		Com_Printf("G_SpawnItem 1.0 - hold_mapname >%s<\n", hold_mapname );

		//verify map title...
		if ( !G_SpawnString( "voting_maptitle", "No Map Title", &holdchar ) ) {
			G_Error ( "G_SpawnItem: %s doesn't have a voting_maptitle at location:%s\n", ent->classname, vtos( ent->s.origin ) );
		}
		if ( strlen (holdchar) > 124 ) {
			G_Error ( "G_SpawnItem: voting_maptitle %s at location:%s is bigger than MAX_HUB_NAME (50) characters, please reduce file Title size\n", ent->classname, vtos( ent->s.origin ) );
		}

		//Set the title of the map
		strcpy( hubInfo[Hub_Index].map_title, holdchar );

		//verify map shader...
		if ( !G_SpawnString( "voting_shader", "999", &holdchar ) ) {
			G_Error ( "G_SpawnItem: %s doesn't have a voting_shader at location:%s\n", ent->classname, vtos( ent->s.origin ) );
		}
		if ( !strcmp( holdchar, "999" ) ) {
			G_Error ( "G_SpawnItem: voting_shader %s at location:%s is undefined, please enter a shader index\n", ent->classname, vtos( ent->s.origin ) );
		}

		//PKMOD - Ergodic 09/29/00 - debug inactivated
//		Com_Printf("G_SpawnItem - Hub_Index>%d<, holdchar>%s<, atoi>%d<\n", Hub_Index, holdchar, atoi( holdchar ) );


		//Set the holder of the shader index
		vote_index = atoi( holdchar );

		if ( vote_index > MAX_HUB_INDEX ) {
			G_Error ( "G_SpawnItem: voting_shader %s at location:%s exceeds the MAX_HUB_INDEX of %d,  please enter a valid shader index\n", ent->classname, vtos( ent->s.origin ), MAX_HUB_INDEX );
		}
		//Set the shader of the map
		hubInfo[Hub_Index].map_shader_index = vote_index;

		//PKMOD - Ergodic 09/29/00 - debug inactivated 
//		Com_Printf("G_SpawnItem - hubInfo[Hub_Index].map_shader_index>%d<\n", hubInfo[Hub_Index].map_shader_index );


//		if( hubInfo[Hub_Index].map_shader_index ) {
//			G_Error ( "G_SpawnItem: voting_shader %s at location:%s is non numeric, please enter a shader index\n", ent->classname, vtos( ent->s.origin ) );
//		}

		//PKMOD - Ergodic 09/24/00 - debug 
//		Com_Printf("G_SpawnItem 1.1 - hold_maptitle >%s<\n", hold_maptitle );

		//set item's pickup name to the voting information
//		holdchar = item->pickup_name;
//		strcpy( holdchar, hold_mapname );
		//PKMOD - Ergodic 09/26/00 - debug
//		Com_Printf("G_SpawnItem 2.0 - hold_mapname via holdchar >%s<\n", holdchar );

//		holdchar = item->pickup_name  + (20 * sizeof (char));
//		strcpy( holdchar, hold_maptitle );
		//PKMOD - Ergodic 09/26/00 - debug
//		Com_Printf("G_SpawnItem 2.0 - hold_maptitle via holdchar >%s<\n", holdchar );

//		Com_Printf("G_SpawnItem 1 - classname >%s< has size >%d<\n", item->classname, sizeof( item->classname ) );
//		value = ent->classname;
//		Com_Printf("G_SpawnItem 2 - pickup_name >%s< has size >%d< has length >%d<\n", item->pickup_name, sizeof( item->pickup_name ), strlen( item->pickup_name ) );

		//CO-OPT variable - set the entitystate "otherEntityNum" to be the voting_image's index
		ent->s.otherEntityNum = vote_index;

		//PKMOD - Ergodic 10/09/00 - debug inactive
//		Com_Printf("G_SpawnItem:Installing classname >%s<, index >%d<, length of world_model[3] >%d<\n", item->classname, vote_index, strlen( item->world_model[3] ) );

		//PKMOD - Ergodic 10/09/00 - set the item's world_model[3] flag

		//store address of item's world_model[3] into holdchar
//		holdchar = item->world_model[3];

		//move offset to the vote_index
//		for (indx = 0; indx < vote_index; indx++) {
//			holdchar++;
//		}

		//set the world_model[3]'s vote_index to ON == 1
//		*holdchar = '1';

		//PKMOD - Ergodic 10/09/00 - debug inactive
//		Com_Printf("G_SpawnItem: voting_shader >%d<, world_model[3] >%s<\n", vote_index, item->world_model[3]  );

		//PKMOD - Ergodic 10/10/00 CO-OPT variable - set the entitystate "otherEntityNum2"
		//		to flag the initial registering of the shader on the cgame side
		ent->s.otherEntityNum2 = 1;

		Hub_Index++;		//increment "hub index"

	}
}

/*
================
G_SlideItem
PKMOD - Ergodic 02/14/01 - items sliding on surfaces when being pulled by the dragon 

================
*/
void G_SlideItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;
	float	change;
	int		i;


//	VectorCopy( ent->s.pos.trDelta, velocity );
	// hinder the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );

	//
	// modify velocity so it parallels all of the clip planes
	//

	// find a plane that it enters
	dot = DotProduct( velocity, trace->plane.normal ); 

	if (  dot < 0.1 ) {
		// move will interact with the plane

		// slide along the plane
		if ( dot < 0 ) {
			dot *= 1.001f;
		} else {
			dot /= 1.001f;
		}

		for ( i=0 ; i<3 ; i++ ) {
			change = trace->plane.normal[i]*dot;
			velocity[i] = velocity[i] - change;
		}
	}

	VectorCopy( velocity, ent->s.pos.trDelta ) ;

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
		ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}
	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	//PKMOD - Ergodic 12/26/00 - fix nodrop on "Pushed" PKA items
	//			do not permanently remove pushed 
	if ( ( contents & CONTENTS_NODROP ) &&  ( ent->AirFist_Level != 1 ) ){
		if (ent->item && ent->item->giType == IT_TEAM) {
			Team_FreeEntity(ent);
		} else {
			G_FreeEntity( ent );
		}
		
		return;
	}

	//PKMOD - Ergodic 02/12/01 if item is on grapple then don't bounce
	//PKMOD - Ergodic 02/14/01 add new function for item's on the dragon
	if ( ent->s.pos.trType == TR_LINEAR )	//missile type
		G_SlideItem( ent, &tr );
	else
		G_BounceItem( ent, &tr );
}

