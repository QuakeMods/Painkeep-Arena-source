// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_combat.c

#include "g_local.h"


/*
============
PrivateBotOwnerTeam
PKMOD - Ergodic 03/07/02 - check for owner vs PB team or PB vs owner team
PKMOD - Ergodic 03/29/02 - rename function from PrivateBotOwnerFrag to PrivateBotOwnerTeam
============
*/
qboolean PrivateBotOwnerTeam( gentity_t *ent1, gentity_t *ent2 ) {

	//PKMOD - Ergodic 03/-07/02 - check for Private Bot fragging owner
	if ( ent1->r.svFlags & SVF_PRIVATEBOT ) {
		if ( ent1->parent->client->ps.clientNum == ent2->client->ps.clientNum )
			return qtrue;
	}

	//PKMOD - Ergodic 03/-07/02 - check for owner fragging Private Bot
	if ( ent2->r.svFlags & SVF_PRIVATEBOT ) {
		if ( ent2->parent->client->ps.clientNum == ent1->client->ps.clientNum )
			return qtrue;
	}

	return qfalse;
}


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}
	// show score plum
	ScorePlum(ent, origin, score);
	//

	//PKMOD - Ergodic 01/08/02 - give Private Bot's frags to owner
	if ( ent->r.svFlags & SVF_PRIVATEBOT ) {
		//PKMOD - Ergodic 02/10/02 - send FRAG message to Private Bot's owner
			G_AddEvent( ent->parent, EV_PRIVATEBOT_FRAG, 0 );

		//add to client scores	
		ent->parent->client->ps.persistant[PERS_SCORE] += score;
		if ( g_gametype.integer == GT_TEAM )
			level.teamScores[ ent->parent->client->ps.persistant[PERS_TEAM] ] += score;
	}
	else {
		ent->client->ps.persistant[PERS_SCORE] += score;
		if ( g_gametype.integer == GT_TEAM )
			level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	}
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player

PKMOD - Ergodic 03/25/01 - add definition to control PKA spew percentage
		if carrying more than 1 pka weapon of a particular type then only spew
		a second item a certain percentage of the time
=================
*/

#define PKA_WEAPON_SPEW_PERCENT 40 

void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;
	float		angle;
	//PKMOD - Ergodic - 08/05/00 pkitems weapon spew, add another variable
	int			i, itemcount;
	gentity_t	*drop;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( weapon == WP_MACHINEGUN || weapon == WP_GRAPPLING_HOOK ) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	//PKMOD - Ergodic 10/03/00 - Drop the grappling_hook
//	if ( weapon > WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK && 
//		self->client->ps.ammo[ weapon ] ) {
	//PKMOD - Ergodic 04/02/02 - Private Bot will not drop the weapon
	if ( !( self->r.svFlags & SVF_PRIVATEBOT ) ) {

		if ( (weapon > WP_MACHINEGUN) && ( weapon == WP_GRAPPLING_HOOK || self->client->ps.ammo[ weapon ] ) ) {
			// find the item type for this weapon
			item = BG_FindItemForWeapon( weapon );
	
			// spawn the item
			Drop_Item( self, item, 0 );

			//PKMOD - Ergodic - 08/05/00 decrement ammo count incase carried weapon is pkitem
			self->client->ps.ammo[ weapon ] -= 1;
		}
	}

//PKMOD - Ergodic 08/05/00 debug hack - give everyone a full pack - inactive
//	self->client->ps.ammo[ WP_BEARTRAP ] = 3;
//	self->client->ps.ammo[ WP_BEANS ] = 1;
//	self->client->ps.ammo[ WP_SENTRY ] = 3;
//	self->client->ps.ammo[ WP_GRAVITY ] = 1;

	//PKMOD - Ergodic - 08/05/00 pkitems weapon spew
	// find the item type for the beartrap
	//PKMOD - Ergodic - 03/25/01 reduce spew to 1 + PKA_WEAPON_SPEW_PERCENT if > 1 
	itemcount = self->client->ps.ammo[ WP_BEARTRAP ];
	if ( itemcount ) {
		item = BG_FindItemForWeapon( WP_BEARTRAP );
		Drop_Item( self, item, rand() % 360 );

		if ( itemcount > 1) {
			if ( ( rand() % 100 ) < PKA_WEAPON_SPEW_PERCENT ) 
					Drop_Item( self, item, rand() % 360 );
		}
	}

	//PKMOD - Ergodic - 08/05/00 pkitems weapon spew
	// find the item type for the beans
	//PKMOD - Ergodic - 03/25/01 reduce spew to 1 + PKA_WEAPON_SPEW_PERCENT if > 1 
	itemcount = self->client->ps.ammo[ WP_BEANS ];
	if ( itemcount ) {
		item = BG_FindItemForWeapon( WP_BEANS );
		Drop_Item( self, item, rand() % 360 );

		if ( itemcount > 1) {
			if ( ( rand() % 100 ) < PKA_WEAPON_SPEW_PERCENT ) 
					Drop_Item( self, item, rand() % 360 );
		}
	}

	//PKMOD - Ergodic - 08/05/00 pkitems weapon spew
	// find the item type for the sentry
	//PKMOD - Ergodic - 03/25/01 reduce spew to 1 + PKA_WEAPON_SPEW_PERCENT if > 1 
	itemcount = self->client->ps.ammo[ WP_SENTRY ];
	if ( itemcount ) {
		item = BG_FindItemForWeapon( WP_SENTRY );
		Drop_Item( self, item, rand() % 360 );

		if ( itemcount > 1) {
			if ( ( rand() % 100 ) < PKA_WEAPON_SPEW_PERCENT ) 
					Drop_Item( self, item, rand() % 360 );
		}
	}

	//PKMOD - Ergodic - 08/05/00 pkitems weapon spew
	// find the item type for the sentry
	//PKMOD - Ergodic - 03/25/01 reduce spew to 1 + PKA_WEAPON_SPEW_PERCENT if > 1 
	itemcount = self->client->ps.ammo[ WP_GRAVITY ];
	if ( itemcount ) {
		item = BG_FindItemForWeapon( WP_GRAVITY );
		Drop_Item( self, item, rand() % 360 );

		if ( itemcount > 1) {
			if ( ( rand() % 100 ) < PKA_WEAPON_SPEW_PERCENT ) 
					Drop_Item( self, item, rand() % 360 );
		}
	}

	//PKMOD - Ergodic 11/01/00 - spew holdables
	//PKMOD - Ergodic 05/11/01 - allow holding of more than 1 type of
	//			holdable but only 1 of each kind
	if ( self->client->ps.stats[STAT_HOLDABLE_ITEM] > 0 ) {
		for ( itemcount = HI_TELEPORTER; itemcount < HI_NUM_HOLDABLE; itemcount++ ) {
			//PKMOD - Ergodic 01/08/02 - only drop 1 piece of the private bot
			if ( itemcount == HI_BOTLEGS ) {
				i = ( self->client->ps.stats[STAT_HOLDABLE_ITEM] >> HI_BOTLEGS ) & 7;
				switch (i) {
				case 1:
					itemcount = HI_BOTLEGS;
					break;
				case 2:
					itemcount = HI_BOTTORSO;
					break;
				case 3:
					if ( rand() % 2 )
						itemcount = HI_BOTLEGS;
					else
						itemcount = HI_BOTTORSO;
					break;
				case 4:
					itemcount = HI_BOTHEAD;
					break;
				case 5:
					if ( rand() % 2 )
						itemcount = HI_BOTLEGS;
					else
						itemcount = HI_BOTHEAD;
					break;
				case 6:
					if ( rand() % 2 )
						itemcount = HI_BOTTORSO;
					else
						itemcount = HI_BOTHEAD;
					break;
				case 7:
						itemcount = HI_BOTLEGS + ( rand() % 3 );
					break;
				default:
						itemcount = 0;		//no Private Bot pieces are held
					break;
				}
				//if holding at least 1 Private Bot piece then spew it
				if ( itemcount != 0 ) {
					item = BG_FindItemForHoldable( itemcount );
					Drop_Item( self, item, rand() % 360 );
				}
				//update itemcount index to last Private Bot Piece
				itemcount = HI_BOTHEAD;
			}
			else {  
				if ( self->client->ps.stats[STAT_HOLDABLE_ITEM] & ( 1 << itemcount ) ) {
					item = BG_FindItemForHoldable( itemcount );
					Drop_Item( self, item, rand() % 360 );
				}
			}
		}
	}

	// drop all the powerups if not in teamplay
	if ( g_gametype.integer != GT_TEAM ) {
		angle = 45;
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if ( self->client->ps.powerups[ i ] > level.time ) {
				item = BG_FindItemForPowerup( i );
				if ( !item ) {
					continue;
				}
				drop = Drop_Item( self, item, angle );
				// decide how many seconds it has left
				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}
				angle += 45;
			}
		}
	}
}

#ifdef MISSIONPACK

/*
=================
TossClientCubes
=================
*/
extern gentity_t	*neutralObelisk;

void TossClientCubes( gentity_t *self ) {
	gitem_t		*item;
	gentity_t	*drop;
	vec3_t		velocity;
	vec3_t		angles;
	vec3_t		origin;

	self->client->ps.generic1 = 0;

	// this should never happen but we should never
	// get the server to crash due to skull being spawned in
	if (!G_EntitiesFree()) {
		return;
	}

	if( self->client->sess.sessionTeam == TEAM_RED ) {
		item = BG_FindItem( "Red Cube" );
	}
	else {
		item = BG_FindItem( "Blue Cube" );
	}

	angles[YAW] = (float)(level.time % 360);
	angles[PITCH] = 0;	// always forward
	angles[ROLL] = 0;

	//PKMOD - Ergodic 02/14/02 - optimize AngeVectors call when only "FORWARD" is needed
//	AngleVectors( angles, velocity, NULL, NULL );
	AngleVectorsForward( angles, velocity );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	if( neutralObelisk ) {
		VectorCopy( neutralObelisk->s.pos.trBase, origin );
		origin[2] += 44;
	} else {
		VectorClear( origin ) ;
	}

	drop = LaunchItem( item, origin, velocity );

	drop->nextthink = level.time + g_cubeTimeout.integer * 1000;
	drop->think = G_FreeEntity;
	drop->spawnflags = self->client->sess.sessionTeam;
}


/*
=================
TossClientPersistantPowerups
=================
*/
void TossClientPersistantPowerups( gentity_t *ent ) {
	gentity_t	*powerup;

	if( !ent->client ) {
		return;
	}

	if( !ent->client->persistantPowerup ) {
		return;
	}

	powerup = ent->client->persistantPowerup;

	powerup->r.svFlags &= ~SVF_NOCLIENT;
	powerup->s.eFlags &= ~EF_NODRAW;
	powerup->r.contents = CONTENTS_TRIGGER;
	trap_LinkEntity( powerup );

	ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
	ent->client->persistantPowerup = NULL;
}
#endif


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
//	gentity_t *ent;
//	int i;

	//PKMOD Ergodic - 07/08/01, remove kamizaki entity flag
	//if this entity still has kamikaze
//	if (self->s.eFlags & EF_KAMIKAZE) {
//		// check if there is a kamikaze timer around for this owner
//		for (i = 0; i < MAX_GENTITIES; i++) {
//			ent = &g_entities[i];
//			if (!ent->inuse)
//				continue;
//			if (ent->activator != self)
//				continue;
//			if (strcmp(ent->classname, "kamikaze timer"))
//				continue;
//			G_FreeEntity(ent);
//			break;
//		}
//	}
	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
		return;
	}

	GibEntity( self, 0 );
}


//PKMOD - Ergodic 06/05/03 - list order must match meansOfDeath_t in bg_public.h
// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_PLASMA",
	"MOD_PLASMA_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_GRAPPLE",
	//PKMOD -Add Weapons. 
	"MOD_GRAVITY",
	"MOD_SENTRY",
	"MOD_BEARTRAP",
	"MOD_AIRFIST",
	"MOD_NAILGUN",
	"MOD_NAIL",  //PKMOD - Ergodic 08/01/00
	"MOD_EXPLODING_SHELLS",
	"MOD_EXPLODING_SHELLS_SPLASH",
	//PKMOD - Ergodic 12/05/00 - add new types of PKA shooters
	"MOD_SHOOTER_LIGHTNING",
	//PKMOD - Ergodic 01/11/01 - give frag credit to activator of CRUSH Death
	"MOD_CRUSH_CREDIT",
	//PKMOD - Ergodic 01/13/01 - exploding autosentry will produce splash damage
	"MOD_SENTRY_SPLASH",
	//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
	"MOD_LIGHTNING_WATER_DISCHARGE",
	//PKMOD - Ergodic 02/01/01 - add can of beans toot damage
	"MOD_BEANS_BLAST",
	//PKMOD - Ergodic 07/02/01 - add reverse damage on lightning from autosentry
	"MOD_REVERSE_LIGHTNING",
	//PKMOD - Ergodic 10/29/01 - add holdable radiation death
	"MOD_RADIATION",
	//PKMOD - Ergodic 06/08/02 - add personal sentry death
	"MOD_PERSONALSENTRY",
	//PKMOD - Ergodic 10/23/02 - Create new Means of Death for the Dragon Blade
	"MOD_DRAGONBLADE",
	//PKMOD - Ergodic 06/06/03 - Create new Means of Death for the Private Bot for logging use in games.log
	"MOD_PRIVATEBOT",
	//PKMOD - Ergodic 01/05/04 - add QUAD FART beans toot damage
	"MOD_QUADBEANS_BLAST"
};

#ifdef MISSIONPACK
/*
==================
Kamikaze_DeathActivate
==================
*/
void Kamikaze_DeathActivate( gentity_t *ent ) {
	G_StartKamikaze(ent);
	G_FreeEntity(ent);
}

/*
==================
Kamikaze_DeathTimer
==================
*/
void Kamikaze_DeathTimer( gentity_t *self ) {
	gentity_t *ent;

	ent = G_Spawn();
	ent->classname = "kamikaze timer";
	VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->think = Kamikaze_DeathActivate;
	ent->nextthink = level.time + 5 * 1000;

	ent->activator = self;
}

#endif

/*
==================
CheckAlmostCapture
==================
*/
void CheckAlmostCapture( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if this player was carrying a flag
	if ( self->client->ps.powerups[PW_REDFLAG] ||
		self->client->ps.powerups[PW_BLUEFLAG] ||
		self->client->ps.powerups[PW_NEUTRALFLAG] ) {
		// get the goal flag this player should have been going for
		if ( g_gametype.integer == GT_CTF ) {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_blueflag";
			}
			else {
				classname = "team_CTF_redflag";
			}
		}
		else {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_redflag";
			}
			else {
				classname = "team_CTF_blueflag";
			}
		}
		ent = NULL;
		do
		{
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT) ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
CheckAlmostScored
==================
*/
void CheckAlmostScored( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if the player was carrying cubes
	if ( self->client->ps.generic1 ) {
		if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
			classname = "team_redobelisk";
		}
		else {
			classname = "team_blueobelisk";
		}
		ent = G_Find(NULL, FOFS(classname), classname);
		// if we found the destination obelisk
		if ( ent ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}


/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	// check for an almost capture
	CheckAlmostCapture( self, attacker );
	// check for a player that almost brought in cubes
	//PKMOD - Ergodic 03/03/01 - remove this code since PKA will use the generic1 variable
//	CheckAlmostScored( self, attacker );

	if (self->client && self->client->hook) {
		//PKMOD - Ergodic 06/23/01 - if player is "actively" deploying a weapon
		//			when killed - then weapon will be released
		if ( self->client->hook->s.generic1 != 0 ) {
				//PKMOD - Ergodic 12/20/01 - modify activate_dragon_deploy call to add struck entity
				//PKMOD - Ergodic 01/26/02 - modify activate_dragon_deploy call to add bytedir for gauntlet
				activate_dragon_deploy ( self->client->hook, NULL, 0 );
		}
		Weapon_HookFree(self->client->hook);
	}
#ifdef MISSIONPACK
	if ((self->client->ps.eFlags & EF_TICKING) && self->activator) {
		self->client->ps.eFlags &= ~EF_TICKING;
		self->activator->think = G_FreeEntity;
		self->activator->nextthink = level.time;
	}
#endif
	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	//PKMOD - Ergodic 06/06/03 - create a custom log record if killer is a Private Bot
	if ( attacker->r.svFlags & SVF_PRIVATEBOT ) {
		int		killer2;
		char	*killerName2;
		killer2 = attacker->parent->s.number;
		killerName2 = attacker->parent->client->pers.netname;

		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
		killer2, self->s.number, MOD_PRIVATEBOT, killerName2, 
		self->client->pers.netname, "MOD_PRIVATEBOT" );
	}
	else {
		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
		killer, self->s.number, meansOfDeath, killerName, 
		self->client->pers.netname, obit );
	}

	//PKMOD - Ergodic 06/06/03 - example log message...
/*
  0:33 Kill: 2 1 7: Ergodic[evolve] killed Daemia by MOD_ROCKET_SPLASH
  0:33 Kill: 2 2 7: Ergodic[evolve] killed Ergodic[evolve] by MOD_ROCKET_SPLASH
  9:06 Kill: 2 1 1: Ergodic[evolve] killed Daemia by MOD_SHOTGUN
*/

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;

	//PKMOD - Ergodic 12/18/02 - send a different information if PB was killed
	if ( self->r.svFlags & SVF_PRIVATEBOT ) { 
		//if PB then send in the Owners name
		//PKMOD - Ergodic 12/16/03 - address PB death message bug where
		//			PB's owner's name was missing
		//ent->s.otherEntityNum = ent->parent->s.number;
		ent->s.otherEntityNum = self->parent->s.number;
		ent->s.time2 = 1;		//time2 = 1 flag will signify a PB
	} 
	else {
		//Not a PB death
		ent->s.otherEntityNum = self->s.number;
		ent->s.time2 = 0;		//time2 = 0 flag will signify NOT a PB
	}
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		//PKMOD - Ergodic 03/07/02 - check for owner vs PB frag and PB vs owner frag
		if ( attacker == self || OnSameTeam (self, attacker ) || PrivateBotOwnerTeam( self, attacker ) ) {
			AddScore( attacker, self->r.currentOrigin, -1 );
		} else {
			AddScore( attacker, self->r.currentOrigin, 1 );

			//PKMOD - Ergodic 08/08/00 for new PKA awards add switch logic
			switch ( meansOfDeath ) {

				//PKMOD - Ergodic 10/23/02 - Create new Means of Death for the Dragon Blade
				case MOD_DRAGONBLADE:
				case MOD_GAUNTLET:

				
					// play humiliation on player
					attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;

					// add the sprite over the player's head
					attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP  | EF_AWARD_PAINKILLER );
					attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET;
					attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

					// also play humiliation on target
					self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
					break;

							//PKMOD - PK Weapon Deaths... 
				case MOD_GRAPPLE:
				case MOD_GRAVITY:
				case MOD_SENTRY:
				case MOD_BEARTRAP:
				case MOD_NAIL:
				case MOD_NAILGUN:
				case MOD_EXPLODING_SHELLS:
				case MOD_EXPLODING_SHELLS_SPLASH:
				case MOD_AIRFIST:
				case MOD_LIGHTNING:
				//PKMOD - Ergodic 08/02/02 - add holdable radiation deaths and persentry kills to list 
				case MOD_RADIATION:
				case MOD_PERSONALSENTRY:
				//PKMOD - Ergodic 01/13/01 - exploding autosentry will produce splash damage
				case MOD_SENTRY_SPLASH:
					//PKMOD - Ergodic 08/08/00 PAINKILLER awarded after every 10 PKitem kills
					//PKMOD - Ergodic 12/17/00 - place PERS_HUB_FLAG as first bit of PERS_PAINKILLER_COUNT
					//				thus add 2 and not 1 to increment PERS_PAINKILLER_COUNT
					attacker->client->ps.persistant[PERS_PAINKILLER_COUNT] += 2;
					//PKMOD - Ergodic 12/17/00 - Right shift PERS_PAINKILLER_COUNT to remove first HUB_FLAG bit
					if ( ( (attacker->client->ps.persistant[PERS_PAINKILLER_COUNT] >> 1) % 10 ) == 0 ) {

						// add the sprite over the player's head
						attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP  | EF_AWARD_PAINKILLER );
						attacker->client->ps.eFlags |= EF_AWARD_PAINKILLER;
						attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
					}
					break;

				default: 
					break;
			}



			//PKMOD - Ergodic 08/08/00 do not award excellents for shaft and gravity wells
			//			since they are already multi-kill weapons
			if ( ( meansOfDeath != MOD_GRAVITY ) && ( meansOfDeath != MOD_LIGHTNING ) ) {
				// check for two kills in a short amount of time
				// if this is close enough to the last kill, give a reward sound
				if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
					// play excellent on player
					attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

					// add the sprite over the player's head
					attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP  | EF_AWARD_PAINKILLER );
					attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
					attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
				}
			}

			attacker->client->lastKillTime = level.time;
		}
	} else {
		AddScore( self, self->r.currentOrigin, -1 );
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE) {
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
			self->client->ps.powerups[PW_NEUTRALFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	//PKMOD - Ergodic 12/29/00 - debug surface flags inactive 
//	Com_Printf("player_die - contents>%d<\n", contents );

	//PKMOD - Ergodic 11/01/02 - Added: do not drop weapons (spew) if death by Gravity Well 
	if ( !( contents & CONTENTS_NODROP ) && ( meansOfDeath != MOD_GRAVITY ) ) {
		TossClientItems( self );
	}
	else {
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
		}
		else if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
		}
	}
#ifdef MISSIONPACK
	TossClientPersistantPowerups( self );
	if( g_gametype.integer == GT_HARVESTER ) {
		TossClientCubes( self );
	}
#endif

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	self->r.maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	if ( (self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else {
		// normal death
		static int i;

		switch ( i ) {
		case 0:
			anim = BOTH_DEATH1;
			break;
		case 1:
			anim = BOTH_DEATH2;
			break;
		case 2:
		default:
			anim = BOTH_DEATH3;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim = 
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim = 
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		G_AddEvent( self, EV_DEATH1 + i, killer );

		// the body can still be gibbed
		self->die = body_die;

		// globally cycle through the different death animations
		i = ( i + 1 ) % 3;

#ifdef MISSIONPACK
		if (self->s.eFlags & EF_KAMIKAZE) {
			Kamikaze_DeathTimer( self );
		}
#endif
	}

	//PKMOD - Ergodic 01/07/02 - Remove Private Bot upon death
	if ( self->r.svFlags & SVF_PRIVATEBOT ) {
		//PKMOD - Ergodic 01/07/02 - debug removing Private Bot (inactive)
//		Com_Printf("player_die - removing PRIVATE BOT client>%d<\n", self->client->ps.clientNum );

		//PKMOD - Ergodic 02/27/04 - Reset the Private Bot flag
		//		This flag may cause next clients that start in the same area to be unable to join
		//		in the server
		self->r.svFlags &=  ~SVF_PRIVATEBOT;

//		trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", self->client->ps.clientNum ) );
		//PKMOD - Ergodic 01/19/02 - use trap_DropClient instead of clientkick 
		trap_DropClient( self->client->ps.clientNum, "is destroyed" );
		//PKMOD - Ergodic 03/18/02 - decrement the Private Bot Counter 
		active_private_bots--;	
	}
	else {
		//PKMOD - Ergodic 01/07/02 - debug removing Private Bot (inactive)
//		Com_Printf("player_die - not a private bot>%d<, svFlags>%d<\n", self->client->ps.clientNum, self->r.svFlags );

		//Not a Private Bot
		trap_LinkEntity (self);
	}

}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags) {
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

#ifdef MISSIONPACK
/*
================
G_InvulnerabilityEffect
================
*/
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir ) {
	gentity_t	*impact;
	vec3_t		intersections[2], vec;
	int			n;

	if ( !targ->client ) {
		return qfalse;
	}
	VectorCopy(dir, vec);
	VectorInverse(vec);
	// sphere model radius = 42 units
	n = RaySphereIntersections( targ->client->ps.origin, 42, point, vec, intersections);
	if (n > 0) {
		impact = G_TempEntity( targ->client->ps.origin, EV_INVUL_IMPACT );
		VectorSubtract(intersections[0], targ->client->ps.origin, vec);
		vectoangles(vec, impact->s.angles);
		impact->s.angles[0] += 90;
		if (impact->s.angles[0] > 360)
			impact->s.angles[0] -= 360;
		if ( impactpoint ) {
			VectorCopy( intersections[0], impactpoint );
		}
		if ( bouncedir ) {
			VectorCopy( vec, bouncedir );
			VectorNormalize( bouncedir );
		}
		return qtrue;
	}
	else {
		return qfalse;
	}
}
#endif
/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			knockback;
	int			max;
#ifdef MISSIONPACK
	vec3_t		bouncedir, impactpoint;
#endif

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}
#ifdef MISSIONPACK
	if ( targ->client && mod != MOD_JUICED) {
		if ( targ->client->invulnerabilityTime > level.time) {
			if ( dir && point ) {
				G_InvulnerabilityEffect( targ, dir, point, impactpoint, bouncedir );
			}
			return;
		}
	}
#endif
	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_OBELISK && CheckObeliskAttack( targ, attacker ) ) {
		return;
	}
#endif
	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if ( attacker->client && attacker != targ ) {
		max = attacker->client->ps.stats[STAT_MAX_HEALTH];
#ifdef MISSIONPACK
		if( bg_itemlist[attacker->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD ) {
			max /= 2;
		}
#endif
		damage = damage * max / 100;
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}
	
	//PKMOD - Ergodic 03/19/01 - give the autosentry more kickback
	//PKMOD - Ergodic 03/25/01 - reduce autosentry knockback from 3 to 2
	//PKMOD - Ergodic 04/06/01 - remove additional autosentry knockback
//	if ( mod == MOD_SENTRY )
//		knockback = 2 * damage;
//	else
//		knockback = damage;

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	//PKMOD - Ergodic 01/07/04 - debug knockback from quadbeans (inactive)
	//Com_Printf( "G_Damage - damage>%d<, knockback>%d<\n", damage, knockback );

	//PKMOD - Ergodic 01/07/04 - don't push the player so high on self quadbeans damage
	if (( mod == MOD_QUADBEANS_BLAST ) && ( targ == attacker )) {
		knockback = knockback / 1.6;
		//PKMOD - Ergodic 01/07/04 - debug knockback from quadbeans (inactive)
		//Com_Printf( "G_Damage - MOD_QUADBEANS_BLAST reducing knockback to >%d<\n", knockback );
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
#ifdef MISSIONPACK
		if ( mod != MOD_JUICED && targ != attacker && !(dflags & DAMAGE_NO_TEAM_PROTECTION) && OnSameTeam (targ, attacker)  ) {
#else	
		//PKMOD - Ergodic 05/10/01 - Gravity Well always kills (regardless of team affiliation)
		if ( targ != attacker && OnSameTeam (targ, attacker) && ( mod != MOD_GRAVITY )  ) {
#endif
			if ( !g_friendlyFire.integer ) {
				return;
			}
		}
#ifdef MISSIONPACK
		if (mod == MOD_PROXIMITY_MINE) {
			if (inflictor && inflictor->parent && OnSameTeam(targ, inflictor->parent)) {
				return;
			}
			if (targ == attacker) {
				return;
			}
		}
#endif

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	//PKMOD - Ergodic 12/16/02 - Battlesuit will protect against stepped on nails
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) || ( mod == MOD_NAIL ) ) {
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS]--;
		} else {
			attacker->client->ps.persistant[PERS_HITS]++;
		}

		//PKMOD - Ergodic 02/05/02 - Clear the (prior) ATTACK bit settings
		attacker->client->ps.stats[STAT_PKA_BITS] &= ~PKA_BITS_SENTRYATTACK;

		//PKMOD - Ergodic 02/05/02 - set the attack bits for the proper sounds
		switch ( mod ) {
			case MOD_BEARTRAP:
				attacker->client->ps.stats[STAT_PKA_BITS] |= PKA_BITS_BEARTRAPATTACK;
				break;
			case MOD_SENTRY:
				attacker->client->ps.stats[STAT_PKA_BITS] |= PKA_BITS_SENTRYATTACK;
				break;
			case MOD_SENTRY_SPLASH:
				attacker->client->ps.stats[STAT_PKA_BITS] |= PKA_BITS_SENTRYATTACK;
				break;
			case MOD_RADIATION:
				attacker->client->ps.stats[STAT_PKA_BITS] |= PKA_BITS_RADIATEATTACK;
				break;
			default:
				break;
		}

		attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health<<8)|(client->ps.stats[STAT_ARMOR]);
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker) {
		damage *= 0.5;
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;
	save = 0;

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	if ( g_debugDamage.integer ) {
		G_Printf( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
			targ->health, take, asave );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF ) {
#else	
	if( g_gametype.integer == GT_CTF) {
#endif
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take) {
		targ->health = targ->health - take;
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}
			
		if ( targ->health <= 0 ) {
			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			return;
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take);
		}
	}

}

//PKMOD - Ergodic 02/17/01 - Add Vectors first and then scalar multiply  
#define	VectorAM(v, s, b, o)	((o)[0]=((v)[0]+(b)[0])*(s),(o)[1]=((v)[1]+(b)[1])*(s),(o)[2]=((v)[2]+(b)[2])*(s))

/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.

//PKMOD - Ergodic 02/17/01 - Modify to make it run faster

Phase	location	relative additions from prior phase
start	(0,0)		
1		(15,15)		dest[0] += 15.0, dest[1] += 15.0;
2		(15,-15)	dest[1] -= 30.0
3		(-15,-15)	dest[0] -= 30.0
4		(-15,15)	dest[1] += 30.0
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
//	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
/*
PKMOD - Ergodic 02/17/01 - Modify to make it run faster  
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
*/
//Phase	location	relative additions from prior phase
//start	(0,0)		
	VectorAM ( targ->r.absmin, 0.5, targ->r.absmax, dest ); //PKMOD optimized
	
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
/*
PKMOD - Ergodic 02/17/01 - Modify to make it run faster  
	VectorCopy (midpoint, dest);
*/
//Phase	location	relative additions from prior phase
//1		(15,15)		dest[0] += 15.0, dest[1] += 15.0;
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

/*
PKMOD - Ergodic 02/17/01 - Modify to make it run faster  
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
*/
//Phase	location	relative additions from prior phase
//2		(15,-15)	dest[1] -= 30.0
	dest[1] -= 30.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

/*
PKMOD - Ergodic 02/17/01 - Modify to make it run faster  
	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
*/
//Phase	location	relative additions from prior phase
//3		(-15,-15)	dest[0] -= 30.0
	dest[0] -= 30.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

/*
PKMOD - Ergodic 02/17/01 - Modify to make it run faster  
	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
*/
//Phase	location	relative additions from prior phase
//4		(-15,15)	dest[1] += 30.0
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;

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

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

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

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}


/*
============
PKA_RadiusDamage
//PKMOD - Ergodic 01/21/01 - Add damage for exploding shells
//			Debug code for exploding shells
============
qboolean PKA_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod, float u, float r) {

	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
	//PKMOD - Ergodic 01/20/01 - debug entity and temp vector
	gentity_t		*debug_tent;

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

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

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

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			//PKMOD - Ergodic 01/18/01 - Debug (exploding shells hitting self) 
//			Com_Printf("G_RadiusDamage: origin>%s<, attacker>%s<, damage>%f<, radius>%f<\n", vtos( origin ), vtos( attacker->r.currentOrigin ), damage * 100, radius * 100 );
//			Com_Printf("G_RadiusDamage: who>%s<, points>%f<, dist>%f<\n", ent->client->pers.netname, points * 100, dist * 100 );
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
			//PKMOD - Ergodic 01/19/01 - Debug (exploding shells hitting self) 
			//PKMOD - Ergodic 01/19/01 - Set the Hit to true (should not be done here) 
			hitClient = qtrue; //PKMOD - Ergodic 01/19/01 - debug
	//PKMOD - Ergodic 01/22/01- debug exploding shells
	//send coord 
	debug_tent = G_TempEntity( origin, EV_COORD );
	debug_tent->s.weapon = WP_EXPLODING_SHELLS;

		}
	}

	return hitClient;
}
 end PKA_RadiusDamage debug code */
