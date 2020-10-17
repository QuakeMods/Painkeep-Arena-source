// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef MISSIONPACK // bk001205
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	//PKMOD - Ergodic 12/18/02 - expand target's name from 32 TO 64, To include "Private Bot"
	//char		targetName[32];
	char		targetName[64];
	char		attackerName[32];
	gender_t	gender;
	clientInfo_t	*ci;
	//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
	int			rndmsg;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	//pkmod - Ergodic 12/18/02 - add Private Bot Info if needed
	if ( ent->time2 == 1 )
		strcat( targetName, "'s Private Bot" );
		
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "suicides";
		break;
	case MOD_FALLING:
		message = "cratered";
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
	case MOD_SLIME:
		message = "melted";
		break;
	case MOD_LAVA:
		message = "does a back flip into the lava";
		break;
	case MOD_TARGET_LASER:
		message = "saw the light";
		break;
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
	//PKMOD - Ergodic 12/05/00 - add new types of PKA shooters
	case MOD_SHOOTER_LIGHTNING:
		rndmsg = rand() % 4;  //Generate random numbers: {0,1,2,3}
		switch (rndmsg) {
			case 0:
				message = "was zapped extra crispy";
				break;
			case 1: 
				message = "was short-circuited";
				break;
			case 2: 
				message = "was not properly grounded";
				break;
			default:
				message = "had a shocking experience";
				break;
		}
	default:
		message = NULL;
		break;
	}

	//pkmod - Ergodic 12/18/02 - if suicide and not a Private Bot
	if ( ( attacker == target ) && ( ent->time2 != 1 ) ) {
		gender = ci->gender;
		//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
		rndmsg = rand() % 3;  //Generate random numbers: {0,1,2}

		switch (mod) {
#ifdef MISSIONPACK
		case MOD_KAMIKAZE:
			message = "goes out with a bang";
			break;
#endif
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
		case MOD_GRAVITY:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "was sucked into her own well";
						break;
					case 1: 
						message = "left this dimension through her own door";
						break;
					default:
						message = "now knows the fate of the universe";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "was sucked into its own well";
						break;
					case 1: 
						message = "left this dimension through its own door";
						break;
					default:
						message = "now knows the fate of the universe";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "was sucked into his own well";
						break;
					case 1: 
						message = "left this dimension through his own door";
						break;
					default:
						message = "now knows the fate of the universe";
						break;
				}
			}
			break;

		case MOD_BEARTRAP:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "lost some limbs in her own beartrap";
						break;
					case 1: 
						message = "was snared by her own beartrap";
						break;
					default:
						message = "should have jumped over her own beartrap";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "lost some limbs in its own beartrap";
						break;
					case 1: 
						message = "was snared by its own beartrap";
						break;
					default:
						message = "should have jumped over its her own beartrap";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "lost some limbs in his own beartrap";
						break;
					case 1: 
						message = "was snared by his own beartrap";
						break;
					default:
						message = "should have jumped over his own beartrap";
						break;
				}
			}
			break;

		case MOD_SENTRY:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "forgot about her own sentry";
						break;
					case 1: 
						message = "mistook her own sentry for an ATM";
						break;
					default:
						message = "was shot up by her own sentry";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "forgot about its own sentry";
						break;
					case 1: 
						message = "mistook its own sentry for an ATM";
						break;
					default:
						message = "was shot up by its own sentry";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "forgot about his own sentry";
						break;
					case 1: 
						message = "mistook his own sentry for an ATM";
						break;
					default:
						message = "was shot up by his own sentry";
						break;
				}
			}
			break;

		//PKMOD - Ergodic 01/13/01 - exploding autosentry will produce splash damage
		case MOD_SENTRY_SPLASH:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "was caught in her own sentry's detonation";
						break;
					case 1: 
						message = "was shredded by her own sentry's shrapnel";
						break;
					default:
						message = "got to close to her little R2D2 buddy";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "was caught its her own sentry's detonation";
						break;
					case 1: 
						message = "was shredded by its own sentry's shrapnel";
						break;
					default:
						message = "got to close to its little R2D2 buddy";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "was caught in his own sentry's detonation";
						break;
					case 1: 
						message = "was shredded by his own sentry's shrapnel";
						break;
					default:
						message = "got to close to his little R2D2 buddy";
						break;
				}
			}
			break;

		//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
		case MOD_LIGHTNING_WATER_DISCHARGE:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "dishonored herself by committing electrical suicide";
						break;
					case 1:
						message = "probably wet her bed many times in her youth";
						break;
					default:
						message = "was fried by her own juice";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "dishonored itself by committing electrical suicide";
						break;
					case 1: 
						message = "probably wet its bed many times in its youth";
						break;
					default:
						message = "was fried by its own juice";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "dishonored himself by committing electrical suicide";
						break;
					case 1: 
						message = "probably wet his bed many times in his youth";
						break;
					default:
						message = "was fried by his own juice";
						break;
				}
			}
			break;

		//PKMOD - Ergodic 07/02/01 - add reverse damage on lightning from autosentry
		case MOD_REVERSE_LIGHTNING:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "Shafted her own ignorant self";
						break;
					case 1:
						message = "had her own lightning reflected";
						break;
					default:
						message = "was juiced by her own hand";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "Shafted its own ignorant self";
						break;
					case 1: 
						message = "had its own lightning reflected";
						break;
					default:
						message = "was juiced by its own hand";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "Shafted his own ignorant self";
						break;
					case 1: 
						message = "had his own lightning reflected";
						break;
					default:
						message = "was juiced by his own hand";
						break;
				}
			}
			break;

		//PKMOD - Ergodic 10/29/01 - add holdable radiation death
		case MOD_RADIATION:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "was burned by her own radiation";
						break;
					case 1:
						message = "made her own internal organs glow";
						break;
					default:
						message = "stumbled into her own nuclear trap";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "was burned by its own radiation";
						break;
					case 1: 
						message = "made its own internal parts glow";
						break;
					default:
						message = "stumbled into its own nuclear trap";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "was burned by his own radiation";
						break;
					case 1: 
						message = "made his own internal organs glow";
						break;
					default:
						message = "stumbled into his own nuclear trap";
						break;
				}
			}
			break;

		case MOD_EXPLODING_SHELLS:
			//PKMOD Ergodic 06/16/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "was ignited by her own shells";
						break;
					case 1: 
						message = "was toasted by her own lava pellets";
						break;
					default:
						message = "was evaporated by her own exploding shells";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "was ignited by its own shells";
						break;
					case 1: 
						message = "was toasted by its own lava pellets";
						break;
					default:
						message = "was evaporated by its own exploding shells";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "was ignited by his own shells";
						break;
					case 1: 
						message = "was toasted by his own lava pellets";
						break;
					default:
						message = "was evaporated by his own exploding shells";
						break;
				}
			}
			break;

		case MOD_NAIL:
			//PKMOD Ergodic 08/01/00 Enable Multi-MOD messages
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "was burned by her own blue spike";
						break;
					case 1: 
						message = "should not have touched her own blue stump";
						break;
					default:
						message = "got impaled on her own hot nail";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "was burned by its own blue spike";
						break;
					case 1: 
						message = "should not have touched its own blue stump";
						break;
					default:
						message = "got impaled on its own hot nail";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "was burned by his own blue spike";
						break;
					case 1: 
						message = "should not have touched his own blue stump";
						break;
					default:
						message = "got impaled on his own hot nail";
						break;
				}
			}
			break;

#ifdef MISSIONPACK
		case MOD_PROXIMITY_MINE:
			if( gender == GENDER_FEMALE ) {
				message = "found her prox mine";
			} else if ( gender == GENDER_NEUTER ) {
				message = "found it's prox mine";
			} else {
				message = "found his prox mine";
			}
			break;
#endif

		case MOD_CRUSH_CREDIT:
			//PKMOD Ergodic 01/11/01 - Enable Multi-MOD messages for Activator of trap
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "squished her own dumb self";
						break;
					case 1: 
						message = "is now 2D thanks to herself";
						break;
					default:
						message = "was flattened by her own trap";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "squished its own dumb self";
						break;
					case 1: 
						message = "is now 2D thanks to itself";
						break;
					default:
						message = "was flattened by its own trap";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "squished his own dumb self";
						break;
					case 1: 
						message = "is now 2D thanks to himself";
						break;
					default:
						message = "was flattened by his own trap";
						break;
				}
			}
			break;

		//PKMOD - Ergodic 01/05/04 - add QUAD FART beans toot damage (self damage)
		case MOD_QUADBEANS_BLAST:
			if ( gender == GENDER_FEMALE ) {
				switch (rndmsg) {
					case 0:
						message = "melted her lungs by using Quad Beans";
						break;
					case 1: 
						message = "suffocated in her own enhanced fecal frenzy";
						break;
					default:
						message = "was kramered in her own caustic bio-gas";
						break;
				}
			}
			else if ( gender == GENDER_NEUTER ) {
				switch (rndmsg) {
					case 0:
						message = "melted its lungs by using Quad Beans";
						break;
					case 1: 
						message = "suffocated in its own enhanced fecal frenzy";
						break;
					default:
						message = "was kramered in its own caustic bio-gas";
						break;
				}
			}
			else {
				switch (rndmsg) {
					case 0:
						message = "melted his lungs by using Quad Beans";
						break;
					case 1: 
						message = "suffocated in his own enhanced fecal frenzy";
						break;
					default:
						message = "was kramered in his own caustic bio-gas";
						break;
				}
			}
			break;

		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

	// check for kill messages from the current clientNum
	//pkmod - Ergodic 12/18/02 - if suicide and not a Private Bot
	//if ( attacker == cg.snap->ps.clientNum ) {
	if ( ( attacker == cg.snap->ps.clientNum ) && ( ent->time2 != 1 ) ) {
		char	*s;

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else {
			s = va("You fragged %s", targetName );
		}
#ifdef MISSIONPACK
		if (!(cg_singlePlayerActive.integer && cg_cameraOrbit.integer)) {
			CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} 
#else
		CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
#endif

		// print the text message as well
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
	rndmsg = rand() % 3;  //Generate random numbers: {0,1,2}

	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was caught by";
					break;
				case 1: 
					message = "got";
					message2 = "'s point";
					break;
				default:
					message = "merged with";
					message2 = "'s spear";
					break;
			}
			break;
		case MOD_GAUNTLET:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was touched by";
					break;
				case 1: 
					message = "got too close to";
					message2 = "'s reach";
					break;
				default:
					message = "was ripped to pieces by";
					break;
			}
			break;
		case MOD_MACHINEGUN:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was actually killed by";
					message2 = "'s wimpy default weapon";
					break;
				case 1: 
					message = "received excessive rat-a-tap-tap from";
					break;
				default:
					message = "was slower on the draw than";
					break;
			}
			break;
		case MOD_SHOTGUN:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "ate a whole lot of";
					message2 = "'s pellets";
					break;
				case 1: 
					message = "was double-barreled by";
					break;
				default:
					message = "was blasted by";
					message2 = "'s shotgun";
					break;
			}
			break;
		case MOD_GRENADE:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "caught";
					message2 = "'s pineapple toss";
					break;
				case 1: 
					message = "triggered";
					message2 = "'s hot potato";
					break;
				default:
					message = "'s parts were scattered by";
					message2 = "'s lucky throw";
					break;
			}
			break;
		case MOD_GRENADE_SPLASH:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "didn't run away quick enough from";
					message2 = "'s grenade";
					break;
				case 1: 
					message = "was spammed by";
					break;
				default:
					message = "was perforated by";
					message2 = "'s shrapnel";
					break;
			}
			break;
		case MOD_ROCKET:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was personally detonated";
					message2 = "'s rocket";
					break;
				case 1: 
					message = "caught a strong-one from";
					break;
				default:
					message = "reached Valhalla on";
					message2 = "'s missile";
					break;
			}
			break;
		case MOD_ROCKET_SPLASH:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "should have zagged from";
					message2 = "'s rocket";
					break;
				case 1: 
					message = "almost got away from";
					message2 = "'s wrath missile";
					break;
				default:
					message = "was splashed down by";
					break;
			}
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_RAILGUN:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was 'Dirty Harry`d' by";
					break;
				case 1: 
					message = "was bored through by";
					message2 = "'s magnum slug";
					break;
				default:
					message = "got another hole from";
					message2 = "'s magnum";
					break;
			}
			break;
		case MOD_LIGHTNING:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was shafted by";
					break;
				case 1: 
					message = "was plugged into the wall socket by";
					break;
				default:
					message = "was turned extra-crispy by";
					message2 = "'s juice";
					break;
			}
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;

#ifdef MISSIONPACK
		case MOD_NAIL:
			message = "was nailed by";
			break;
		case MOD_CHAINGUN:
			message = "got lead poisoning from";
			message2 = "'s Chaingun";
			break;
		case MOD_PROXIMITY_MINE:
			message = "was too close to";
			message2 = "'s Prox Mine";
			break;
		case MOD_KAMIKAZE:
			message = "falls to";
			message2 = "'s Kamikaze blast";
			break;
		case MOD_JUICED:
			message = "was juiced by";
			break;
#endif

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

/*PKMOD - Ergodic 06/05/03 - remove dead code for CLG...
		case MOD_HARPOON:
               message = "penetrated by";
		break;
end 06/05/03 remove section*/

		case MOD_GRAVITY:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was sucked into";
					message2 = "'s well";
					break;
				case 1: 
					message = "left this dimension through";
					message2 = "'s door";
					break;
				default:
					message = "knows the fate of the universe thanks to";
					break;
			}
		break;
		case MOD_SENTRY:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "mistook";
					message2 = "'s sentry for an ATM";
					break;
				case 1: 
					message = "was shot up by";
					message2 = "'s sentry";
					break;
				default:
					message = "was mowed down by";
					message2 = "'s sentry";
					break;
			}
		break;
		//PKMOD - Ergodic 01/13/01 - exploding autosentry will produce splash damage
		case MOD_SENTRY_SPLASH:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was caught in";
					message2 = "'s detonating sentry";
					break;
				case 1: 
					message = "was shredded by the shrapnel from";
					message2 = "'s sentry";
					break;
				default:
					message = "tried to play with";
					message2 = "'s little R2D2 buddy";
					break;
			}
		break;
		case MOD_BEARTRAP:
			//PKMOD Ergodic 06/12/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "lost some limbs in";
					message2 = "'s beartrap";
					break;
				case 1: 
					message = "was snared by";
					message2 = "'s beartrap";
					break;
				default:
					message = "should have jumped over";
					message2 = "'s beartrap";
					break;
			}
		break;
/*PKMOD - Ergodic 06/05/03 - remove dead code for CLG...
		case MOD_CHAINLG:
               message = "was fried by";
		break;
		case MOD_A2K:
               message = "was obliterated by";
		break;
		case MOD_EMPNUKE:
               message = "was nuked by";
		break;
end 06/05/03 remove section*/

		case MOD_AIRFIST:
			switch (rndmsg) {
				case 0:
					message = "was blown by";
					message2 = "'s airfist";
					break;
				case 1: 
					message = "was flattened by";
					message2 = "'s draft";
					break;
				default:
					message = "was punched out by";
					message2 = "'s wind";
					break;
			}
		break;
		case MOD_EXPLODING_SHELLS:
			//PKMOD Ergodic 06/16/00 Enable Multi-MOD messages
			//PKMOD Ergodic 07/14/00 fixed grammar
			switch (rndmsg) {
				case 0:
					message = "was ignited by";
					message2 = "'s shells";
					break;
				case 1: 
					message = "was toasted by";
					message2 = "'s lava pellets";
					break;
				default:
					message = "was incinerated by";
					message2 = "'s exploding shells";
					break;
			}
			break;
		case MOD_EXPLODING_SHELLS_SPLASH:
			//PKMOD Ergodic 06/16/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "could not out run";
					message2 = "'s exploding shells";
					break;
				case 1: 
					message = "was hot-waxed by";
					break;
				default:
					message = "felt";
					message2 = "'s napalm";
					break;
			}
			break;
		case MOD_NAILGUN:
			//PKMOD Ergodic 08/01/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was nailed by";
					break;
				case 1: 
					message = "was pinned in place by";
					message2 = "'s spike";
					break;
				default:
					message = "got a new hole added by";
					message2 = "'s nailgun";
					break;
			}
		break;
		case MOD_NAIL:
			//PKMOD Ergodic 08/01/00 Enable Multi-MOD messages
			switch (rndmsg) {
				case 0:
					message = "was burned by";
					message2 = "'s blue spike";
					break;
				case 1: 
					message = "should not have touched";
					message2 = "'s blue stump";
					break;
				default:
					message = "got impaled on";
					message2 = "'s hot nail";
					break;
			}
		break;
		case MOD_CRUSH_CREDIT:
			//PKMOD Ergodic 01/11/01 - Enable Multi-MOD messages for Activator of trap
			switch (rndmsg) {
				case 0:
					message = "was squished by";
					break;
				case 1: 
					message = "is now '2D' thanks to";
					break;
				default:
					message = "was flattened by";
					message2 = "'s trap";
					break;
			}
		break;

		//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
		case MOD_LIGHTNING_WATER_DISCHARGE:
			switch (rndmsg) {
				case 0:
					message = "was caught by";
					message2 = "'s electrical discharge";
					break;
				case 1: 
					message = "needs to stay away from";
					message2 = "'s lame water trap";
					break;
				default:
					message = "was fried by";
					message2 = "'s suicide shaft";
					break;
			}
		break;

		//PKMOD - Ergodic 02/01/01 - add can of beans toot damage
		case MOD_BEANS_BLAST:
			switch (rndmsg) {
				case 0:
					message = "was engulfed by";
					message2 = "'s fecal plume";
					break;
				case 1: 
					message = "was smothered thanks to";
					message2 = "'s Bean blast";
					break;
				default:
					message = "was poisoned by";
					message2 = "'s gas shroud";
					break;
			}
		break;

		//PKMOD - Ergodic 10/29/01 - add holdable radiation death
		case MOD_RADIATION:
			switch (rndmsg) {
				case 0:
					message = "was burned by";
					message2 = "'s radiation";
					break;
				case 1:
					message = "'s flesh was set ablaze by";
					message2 = "'s nuclear trap";
					break;
				default:
					message = "succumbed to radiation exposure from";
					break;
			}
		break;

		//PKMOD - Ergodic 08/02/02 - add holdable Personal Sentry death
		case MOD_PERSONALSENTRY:
			switch (rndmsg) {
				case 0:
					message = "ate some shoulder mounted love from";
					message2 = "'s auto cannon";
					break;
				case 1: 
					message = "was chewed up by";
					message2 = "'s A.I. pet";
					break;
				default:
					message = "was hosed down by";
					message2 = "'s Personal Sentry";
					break;
			}
		break;

		//PKMOD - Ergodic 10/23/02 - Create new Means of Death for the Dragon Blade
		case MOD_DRAGONBLADE:
			switch (rndmsg) {
				case 0:
					message = "was bitten by the teeth of";
					message2 = "'s Dragon";
					break;
				case 1: 
					message = "lost some chunks of flesh from";
					message2 = "'s Dragon Bite";
					break;
				default:
					message = "was carved into bits by";
					message2 = "'s Dragon Blade";
					break;
			}
		break;

		//PKMOD - Ergodic 01/05/04 - add QUAD FART beans toot damage
		case MOD_QUADBEANS_BLAST:
			switch (rndmsg) {
				case 0:
					message = "'s lungs were melted by";
					message2 = "'s Quaded Beans";
					break;
				case 1: 
					message = "suffocated in";
					message2 = "'s enhanced fecal frenzy";
					break;
				default:
					message = "was kramered in";
					message2 = "'s caustic bio-gas";
					break;
			}
		break;



		//PKMOD
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		default:
			message = "was killed by";
			break;
		}

		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;
	
	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} else {
			//PKMOD - Ergodic 12/08/01 - give unique message for the PRIAVTE BOT
			if ( itemNum == HI_BOTHEAD )
				CG_CenterPrint( "Activating Private Bot", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
			else {
				item = BG_FindItemForHoldable( itemNum );
				CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
			}
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

	//PKMOD - Ergodic 10/13/01 - add new holdable code for RADIATE
	case HI_RADIATE:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		//PKMOD - Ergodic 08/02/02 - Holdable: radiate activation sound
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.pkaradiateactivationSound );
		break;

	//PKMOD - Ergodic 12/08/01 - add new holdable code for PRIVATE BOT
	case HI_BOTHEAD:
		break;

	//PKMOD - Ergodic 08/02/02 - add new holdable code for PERSONAL SENTRY (no sound)
	case HI_PERSENTRY:
		break;

#ifdef MISSIONPACK
	case HI_KAMIKAZE:
		break;

	case HI_PORTAL:
		break;
	case HI_INVULNERABILITY:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound );
		break;
#endif
	}

}


//PKMOD - Ergodic 11/17/00 - code for removing item from client
static void CG_ItemRemove( int itemNum ) {
	int		hold_giTag;
	int		i;
	qboolean	holding_shotgun;

//	cg.itemPickup = 0;  //was itemNum
//	cg.itemPickupTime = cg.time;
//	cg.itemPickupBlendTime = cg.time;

	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		hold_giTag = bg_itemlist[itemNum].giTag;  //get the weapon tag

		cg.snap->ps.stats[ STAT_WEAPONS ] &=  ~( 1 << hold_giTag ); //remove the weapon

		holding_shotgun = qfalse;
		//if removing the exploding shotgun then also remove the non-exploding shotgun
		if ( hold_giTag == WP_EXPLODING_SHELLS ) {
			holding_shotgun = qtrue;
			cg.snap->ps.stats[ STAT_WEAPONS ] &= ~( 1 << WP_SHOTGUN );
		}

		//if removing the shotgun then also remove the exploding shells shotgun
		if ( hold_giTag == WP_SHOTGUN ) {
			holding_shotgun = qtrue;
			cg.snap->ps.stats[ STAT_WEAPONS ] &= ~( 1 << WP_EXPLODING_SHELLS );
		}

		// do we hold the weapon that is to be removed?
		if ( ( cg.weaponSelect == hold_giTag ) || holding_shotgun ) {
			switch ( hold_giTag ) {
			case WP_BEARTRAP:
			case WP_GRAVITY:
			case WP_SENTRY:
			case WP_BEANS:
				cg.weaponSelect = 1;
				break;
			default:
				for ( i = WP_GRAPPLING_HOOK - 1; i > 0 ; i-- ) {
					//modelled after CG_WeaponSelectable( )
					//check ammo
					if (i == WP_SHOTGUN) {
						if ( (!cg.snap->ps.ammo[WP_SHOTGUN]) && (!cg.snap->ps.ammo[WP_EXPLODING_SHELLS]) ) {
							continue;
						}
					}
					else if ( !cg.snap->ps.ammo[i] ) {
						continue;
					}
					//check weapon
					if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
						continue;
					}

					cg.weaponSelect = i;
					break;
				}
			}
			cg.weaponSelectTime = cg.time;
		}

	}
}

/*
================
CG_ItemPickup

A new item was picked up this frame
//PKMOD - Ergodic 07/06/00 - modified "autoswitch" logic for exploding shells and PKA Items
//PKMOD - Ergodic 07/06/00 (later in the day) - if pickup item is ammo_expshell and current 
//			weapon is the WP_SHOTGUN then switch to WP_EXPLODING_SHELLS
//PKMOD - Ergodic 07/06/00 handle case of picking up a "dropped" WP_EXPLODING_SHELLS when
//			the current weapon is the WP_SHOTGUN then switch to WP_EXPLODING_SHELLS
================
*/
static void CG_ItemPickup( int itemNum ) {
	int		hold_giTag;

	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		hold_giTag = bg_itemlist[itemNum].giTag;
		// select it immediately
		if ( cg_autoswitch.integer && hold_giTag != WP_MACHINEGUN ) {
			//PKMOD - Ergodic 07/06/00 - do not autoswitch to PKA Items
			if ( hold_giTag == WP_GRAVITY ) 
				return;
			if ( hold_giTag == WP_SENTRY ) 
				return;
			if ( hold_giTag == WP_BEARTRAP ) 
				return;
			if ( hold_giTag == WP_BEANS ) 
				return;
			//PKMOD - Ergodic 07/06/00 - Logic for excploding shells
			//        if exploding shells in inventory then switch to exploding shells gun
			if ( hold_giTag == WP_SHOTGUN ) {
				if ( cg.snap->ps.ammo[WP_EXPLODING_SHELLS] )
					cg.weaponSelect = WP_EXPLODING_SHELLS;
				else
					cg.weaponSelect = WP_SHOTGUN;

				cg.weaponSelectTime = cg.time;
				return;
			}

			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
			return;
		}

		//PKMOD - Ergodic 07/06/00 handle case of picking up a "dropped" WP_EXPLODING_SHELLS when
		//			the current weapon is the WP_SHOTGUN then switch to WP_EXPLODING_SHELLS
		if ( hold_giTag == WP_EXPLODING_SHELLS ) {
			if ( cg.weaponSelect == WP_SHOTGUN ) {
				cg.weaponSelect = WP_EXPLODING_SHELLS;
				cg.weaponSelectTime = cg.time;
				return;
			}
		}

	}
	//case of picking up ammo_expshells 07/06/00 (later in the day)
	else if ( ( bg_itemlist[itemNum].giType == IT_AMMO ) && bg_itemlist[itemNum].giTag == WP_EXPLODING_SHELLS) {
		if ( cg.weaponSelect == WP_SHOTGUN ) { //is the shotgun the current model
			//change weapon to exploding shells
			cg.weaponSelect = WP_EXPLODING_SHELLS;
			cg.weaponSelectTime = cg.time;
		}
	}
}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
		{
			localEntity_t	*smoke;
			vec3_t			up = {0, 0, 1};


			smoke = CG_SmokePuff( cent->lerpOrigin, up, 
						  32, 
						  1, 1, 1, 0.33f,
						  1000, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );
		}

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;
#ifdef MISSIONPACK
	case EV_TAUNT_YES:
		DEBUGNAME("EV_TAUNT_YES");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
		break;
	case EV_TAUNT_NO:
		DEBUGNAME("EV_TAUNT_NO");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
		break;
	case EV_TAUNT_FOLLOWME:
		DEBUGNAME("EV_TAUNT_FOLLOWME");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
		break;
	case EV_TAUNT_GETFLAG:
		DEBUGNAME("EV_TAUNT_GETFLAG");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
		break;
	case EV_TAUNT_GUARDBASE:
		DEBUGNAME("EV_TAUNT_GUARDBASE");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
		break;
	case EV_TAUNT_PATROL:
		DEBUGNAME("EV_TAUNT_PATROL");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
		break;
#endif
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//			if(cg_debugEvents.integer)
//				Com_Printf("CG_EntityEvent - Pickup, Item>%d<, Weapon>%d<, BT_AMMO>%d<\n", item->giTag, cent->currentState.weapon, cg.snap->ps.ammo[WP_BEARTRAP] );


			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
#ifdef MISSIONPACK
				switch (item->giTag ) {
					case PW_SCOUT:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.scoutSound );
					break;
					case PW_GUARD:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.guardSound );
					break;
					case PW_DOUBLER:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.doublerSound );
					break;
					case PW_AMMOREGEN:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.ammoregenSound );
					break;
				}
#endif
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//PKMOD - Ergodic 12/05/01 - Radiate Item event
	case EV_ITEM_RADIATE:
		DEBUGNAME("EV_ITEM_RADIATE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.pkaradiateitemSound );
		break;

	//PKMOD - Ergodic 12/05/01 - Radiate Player event
	case EV_PLAYER_RADIATE:
		DEBUGNAME("EV_PLAYER_RADIATE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.pkaradiateplayerSound );
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//		if(cg_debugEvents.integer)
//			Com_Printf("CG_EntityEvent - Change Weapon>%d<, Select>%d<, BT_AMMO>%d<, predicted BT_AMMO>%d<\n",cent->currentState.weapon, cg.weaponSelect, cg.snap->ps.ammo[WP_BEARTRAP], cg.predictedPlayerState.ammo[WP_BEARTRAP] );
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		//PKMOD - Ergodic 08/13/01 - Debug dragon infinite (inactive)
//		if(cg_debugEvents.integer)
//			Com_Printf("CG_EntityEvent - Fire Weapon>%d<,  Select>%d<, generic1>%d<, BT_AMMO>%d<, predicted BT_AMMO>%d<\n",cent->currentState.weapon, cg.weaponSelect, cent->currentState.generic1 & 15, cg.snap->ps.ammo[WP_BEARTRAP], cg.predictedPlayerState.ammo[WP_BEARTRAP] );
		CG_FireWeapon( cent );
		break;

	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
		break;

#ifdef MISSIONPACK
	case EV_PROXIMITY_MINE_STICK:
		DEBUGNAME("EV_PROXIMITY_MINE_STICK");
		if( es->eventParm & SURF_FLESH ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound );
		} else 	if( es->eventParm & SURF_METALSTEPS ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound );
		}
		break;

	case EV_PROXIMITY_MINE_TRIGGER:
		DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound );
		break;
	case EV_KAMIKAZE:
		DEBUGNAME("EV_KAMIKAZE");
		CG_KamikazeEffect( cent->lerpOrigin );
		break;
	case EV_OBELISKEXPLODE:
		DEBUGNAME("EV_OBELISKEXPLODE");
		CG_ObeliskExplode( cent->lerpOrigin, es->eventParm );
		break;
	case EV_OBELISKPAIN:
		DEBUGNAME("EV_OBELISKPAIN");
		CG_ObeliskPain( cent->lerpOrigin );
		break;
	case EV_INVUL_IMPACT:
		DEBUGNAME("EV_INVUL_IMPACT");
		CG_InvulnerabilityImpact( cent->lerpOrigin, cent->currentState.angles );
		break;
	case EV_JUICED:
		DEBUGNAME("EV_JUICED");
		CG_InvulnerabilityJuiced( cent->lerpOrigin );
		break;
	case EV_LIGHTNINGBOLT:
		DEBUGNAME("EV_LIGHTNINGBOLT");
		CG_LightningBoltBeam(es->origin2, es->pos.trBase);
		break;
#endif
	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT );
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_METAL );
		break;

	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
		cent->currentState.weapon = WP_RAILGUN;
		// if the end was on a nomark surface, don't make an explosion
		CG_RailTrail( ci, es->origin2, es->pos.trBase );
		if ( es->eventParm != 255 ) {
			ByteToDir( es->eventParm, dir );
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT );
		}
		break;

	//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, qfalse );
		break;

	//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm, qfalse );
		break;

	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
		CG_ShotgunFire( es );
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
			DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;

				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_BLUEFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF) 
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
						 	CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_REDFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
						if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;

				case GTS_REDTEAM_SCORED:
					CG_AddBufferedSound(cgs.media.redScoredSound);
					break;
				case GTS_BLUETEAM_SCORED:
					CG_AddBufferedSound(cgs.media.blueScoredSound);
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.redLeadsSound);
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.blueLeadsSound);
					break;
				case GTS_TEAMS_ARE_TIED:
					CG_AddBufferedSound( cgs.media.teamsTiedSound );
					break;
#ifdef MISSIONPACK
				case GTS_KAMIKAZE:
					trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
					break;
#endif
				default:
					break;
			}
			break;
		}

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
		break;

	case EV_GIB_PLAYER:
		DEBUGNAME("EV_GIB_PLAYER");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		//PKMOD Ergodic - 07/08/01, remove kamizaki entity flag
	//	if ( !(es->eFlags & EF_KAMIKAZE) ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
	//	}
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	// PKMOD - Ergodic 07/11/00 Events
	case EV_GRAVITY_RELEASED:
		DEBUGNAME("EV_GRAVITY_RELEASED");
		cent->miscTime = cg.time;	// scale up from this
		//PKMOD - Ergodic 07/12/00 change from CHAN_voice to CHAN_AUTO
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_AUTO, cgs.media.sfx_pkagravitylaunched );
		//PKMOD - Ergodic 11/01/02 - Move the call of this function to cg_ents
		//			to fix the Invisible Gravity Well bug
		//CG_GravityWellActivate( cent->lerpOrigin );
		break;

	//
	// PKMOD - Ergodic 05/26/00 Events
	//
	case EV_BEARTRAP_DIE:				// PKMOD - Ergodic 05/26/00 Events
		DEBUGNAME("EV_BEARTRAP_DIE");
		ByteToDir( es->eventParm, dir );
		CG_BearTrapDie( position );
		break;

	case EV_BEARTRAP_SNAP:				// PKMOD - Ergodic 07/01/00 Events
		DEBUGNAME("EV_BEARTRAP_SNAP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkabeartrapsnap );
		break;

	case EV_BEARTRAP_DROP:				// PKMOD - Ergodic 08/07/00 Events
		DEBUGNAME("EV_BEARTRAP_DROP");

		//PKMOD - Ergodic 07/17/03 - use timer for driver of model animation 
		cent->miscTime = cg.time;	// scale up from this

		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkabeartrapdrop );
		break;

	case EV_CHAINLIGHTNING_STRIKE:				// PKMOD - Ergodic 08/22/00 Events
		DEBUGNAME("EV_CHAINLIGHTNING_STRIKE");
		//uses hack for target location stored in es.angles
				trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );

		if ( rand() % 2 ) //random number: { 0, 1 }
			trap_S_StartSound (es->angles, es->number, CHAN_AUTO, cgs.media.sfx_chainlightningstrike1 );
		else
			trap_S_StartSound (es->angles, es->number, CHAN_AUTO, cgs.media.sfx_chainlightningstrike2 );
		break;

	//PKMOD Ergodic 05/30/00 switch to gauntlet for PK Items noammo state
	case EV_PKA_NOAMMO:  
		DEBUGNAME("EV_PKA_NOAMMO");
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_PKA_OutOfAmmoChange();
		}
		break;

	//PKMOD Ergodic 07/19/00 display lightning flash
	case EV_LIGHTNING_FX:  
		DEBUGNAME("EV_LIGHTNING_FX");
		CG_Lightning_FX( position, es->eventParm ); //flash location and flash constantLight
		break;

	//PKMOD - Ergodic 09/06/00 gravity well item suck sounds from Mongusta
	case EV_GRAVITYWELL_SUCK:				// PKMOD - Ergodic 09/06/00 Events
		DEBUGNAME("EV_GRAVITYWELL_SUCK");
		switch ( rand() % 3 ) {	//random numbers: { 0, 1, 2 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkagravitywell_suck1 );
				break;
			case 1:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkagravitywell_suck2 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkagravitywell_suck3 );
				break;
		}
		break;

	//PKMOD - Ergodic 11/16/00 - add target_remove code
	case EV_ITEM_REMOVE:
		DEBUGNAME("EV_ITEM_REMOVE");
		{
			gitem_t	*item;
			int		index;
			//12/16/00 - add non compressed flag
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( "sound/items/lostitem.wav", qfalse ) );

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemRemove( index );
			}

		}
		break;

	//PKMOD - Ergodic 11/20/00 - add target_remove code for powerups
	case EV_GLOBAL_ITEM_REMOVE:
		DEBUGNAME("EV_GLOBAL_ITEM_REMOVE");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			//12/16/00 - add non compressed flag
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( "sound/items/lostpowerup.wav", qfalse ) );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemRemove( index );
			}
		}
		break;

	case EV_AUTOSENTRY_DROP:				// PKMOD - Ergodic 11/22/00 drop the sentry sound
		DEBUGNAME("EV_AUTOSENTRY_DROP");
		//PKMOD - Ergodic 11/25 - use timer for driver of model animation 
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentrydrop );
		break;

	case EV_AUTOSENTRY_DIE:				// PKMOD - Ergodic 11/22/00 blow-up the sentry sound
		DEBUGNAME("EV_AUTOSENTRY_DIE");
		ByteToDir( es->eventParm, dir );
		CG_AutoSentryDie( position );
		break;

	//PKMOD - Ergodic 12/06/00 - special lightning shooter event
	case EV_SHOOTER_LIGHTNING:
		DEBUGNAME("EV_SHOOTER_LIGHTNING");
//		ByteToDir( es->eventParm, dir );
//		CG_MissileHitWall( es->weapon, 0, position, dir );
		CG_ChainLightning( cent );
		break;

	//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
	case EV_AUTOSENTRY_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD, qtrue );
		break;

	//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
	case EV_AUTOSENTRY_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm, qtrue );
		break;

	//PKMOD - Ergodic 12/26/00 add Beans fart noises from Mongusta
	//PKMOD - Ergodic 04/13/01 - move gas effects routine to cg_effects and add bubbles
	//PKMOD - Ergodic 06/30/01 add two more fart sounds from original Q1 PK
	case EV_BEANS_TOOT:				// PKMOD - Ergodic 12/26/00 Events
		DEBUGNAME("EV_BEANS_TOOT");

		//add gas effects
		CG_BeansToot( cent->lerpOrigin );
		
		//add gas sounds
		//PKMOD - Ergodic 06/30/01 add two more fart sounds from original Q1 PK (was rand() % 5)
		switch ( rand() % 7 ) {	//random numbers: { 0, 1, 2, 3, 4, 5, 6 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart1 );
				break;
			case 1:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart2 );
				break;
			case 2:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart3 );
				break;
			case 3:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart4 );
				break;
			case 4:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart5 );
				break;
			case 5:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart6 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart7 );
				break;
		}

		break;

	//PKMOD - Ergodic 01/13/01 - add autosentry fire sounds from mongusta
	case EV_AUTOSENTRY_FIRE:
		DEBUGNAME("EV_AUTOSENTRY_FIRE");
		//add sentry firing noise
		switch ( rand() % 3 ) {	//random numbers: { 0, 1, 2 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry1 );
				break;
			case 1:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry2 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry3 );
				break;
		}

		break;

	//PKMOD - Ergodic 03/26/01 - add autosentry ping sound
	case EV_AUTOSENTRY_PING:
		DEBUGNAME("EV_AUTOSENTRY_PING");
		//add sentry firing noise
		switch ( rand() % 3 ) {	//random numbers: { 0, 1, 2 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry_ping1);
				break;
			case 1:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry_ping2 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkasentry_ping3 );
				break;
		}

		break;


	//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
	case EV_LIGHTNING_WATER_DISCHARGE:
		DEBUGNAME("EV_LIGHTNING_WATER_DISCHARGE");
		CG_Lightning_Water_Discharge (position, es->eventParm);	// eventParm is duration/size
		break;

	//PKMOD - Ergodic 01/21/01 - add event to display coordinate model for exploding shells debug
	case EV_COORD:
		DEBUGNAME("EV_COORD");
		CG_Coord( es );
		break;

	//PKMOD - Ergodic 07/03/01 ChainLightning reflect sounds
	case EV_CHAINLIGHTNING_REFLECT:				// PKMOD - Ergodic 07/03/01 Events
		DEBUGNAME("EV_CHAINLIGHTNING_REFLECT");
		//uses hack for target location stored in es.angles
		if ( rand() % 2 ) //random number: { 0, 1 }
			trap_S_StartSound (es->angles, es->number, CHAN_AUTO, cgs.media.sfx_chainlightningreflect1 );
		else
			trap_S_StartSound (es->angles, es->number, CHAN_AUTO, cgs.media.sfx_chainlightningreflect2 );
		break;

	//PKMOD - Ergodic 01/07/02 - send message to client if not all Private Bot parts are held
	case EV_INCOMPLETE_PRIVATEBOT:				// PKMOD - Ergodic 01/07/02 Events
		DEBUGNAME("EV_INCOMPLETE_PRIVATEBOT");

		//PKMOD - Ergodic 03/17/04 - only send this message to the owner
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_CenterPrint( "Incomplete Private Bot", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
			trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		}
		break;

	//PKMOD - Ergodic 03/18/02 - send message to client that no more Private Bots are available
	case EV_NOAVAILABLE_PRIVATEBOTS:				// PKMOD - Ergodic 03/18/02 Events
		DEBUGNAME("EV_NOAVAILABLE_PRIVATEBOTS");
		switch ( rand() % 4 ) {	//random numbers: { 0, 1, 2, 3 }
			case 0:
				CG_CenterPrint( "All Private Bots are active", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
				break;
			case 1:
				CG_CenterPrint( "All Private Bots are deployed", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
				break;
			case 2:
				CG_CenterPrint( "No Private Bots are left to assign", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
				break;
			default:
				CG_CenterPrint( "All Private Bots are allocated", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
				break;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	//PKMOD - Ergodic 02/07/02 - message client if Private Bot is completed
	
	case EV_COMPLETED_PRIVATEBOT:
		DEBUGNAME("EV_COMPLETED_PRIVATEBOT");
		{
			int		index;

			index = es->eventParm;		// player predicted

			trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.pkapribot_complete );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//PKMOD - Ergodic 02/10/02 - send FRAG message to Private Bot's owner
	case EV_PRIVATEBOT_FRAG:
		DEBUGNAME("EV_PRIVATEBOT_FRAG");
		switch ( rand() % 2 ) {	//random numbers: { 0, 1 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.pkapribot_frag1 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.pkapribot_frag2 );
				break;
		}
		break;

	//PKMOD - Ergodic 06/09/02 - create the teleport flash for the personal sentry (teleport in sound)
	case EV_TELE_IN_PERSONALSENTRY:
		DEBUGNAME( "EV_TELE_IN_PERSONALSENTRY" );
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_PersentrySpawnEffect( position );
		break;

	//PKMOD - Ergodic 08/03/02 - create the teleport flash for the personal sentry (teleport out sound)
	case EV_TELE_OUT_PERSONALSENTRY:
		DEBUGNAME( "EV_TELE_OUT_PERSONALSENTRY" );
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_PersentrySpawnEffect( position );
		break;

	//PKMOD - Ergodic 06/14/02 - create the firing sound for the personal sentry
	//PKMOD - Ergodic 08/26/02 - add Personal Sentry fire sounds from StarDagger
	case EV_FIRE_PERSONALSENTRY:
		DEBUGNAME( "EV_FIRE_PERSONALSENTRY" );
		//add sentry firing noise
		switch ( rand() % 3 ) {	//random numbers: { 0, 1, 2 }
			case 0:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.pkapersentry_fire1 );
				break;
			case 1:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.pkapersentry_fire2 );
				break;
			default:
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.pkapersentry_fire3 );
				break;
		}

		break;

	//PKMOD - Ergodic 11/21/03 - Play an earthquake sound
	//PKMOD - Ergodic 12/07/03 - removed, code moved to global sound
	//case EV_EARTHQUAKE:
	//	DEBUGNAME( "EV_EARTHQUAKE" );
	//	trap_S_StartSound (NULL, ENTITYNUM_NONE, CHAN_AUTO, cgs.media.pkaearthquake );
	//	break;

		//PKMOD - Ergodic 12/06/03 - play the charge up sound
	case EV_CHAINLIGHTNING_CHARGE_UP:
		DEBUGNAME("EV_CHAINLIGHTNING_CHARGE_UP");
		trap_S_StartSound(position, es->number, CHAN_AUTO, cgs.media.pkachargeup );
		break;

	//PKMOD - Ergodic 01/05/04 - add quad farting logic for differing CG graphic sequence
	case EV_QUADBEANS_TOOT:
		DEBUGNAME("EV_QUADBEANS_TOOT");

		//add gas effects
		CG_QuadBeansToot( cent->lerpOrigin );

		//PKMOD - Ergodic 03/17/04 - add new quad farting sounds that occur very rarely
		if ( ( rand() % 100 ) > 91 ) {
			switch ( rand() % 3 ) {	//random numbers: { 0, 1, 2 }
				case 0:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkaquadfart1 );
					break;
				case 1:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkaquadfart2 );
					break;
				default:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkaquadfart3 );
					break;
			}
		} else {
			switch ( rand() % 7 ) {	//random numbers: { 0, 1, 2, 3, 4, 5, 6 }
				case 0:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart1 );
					break;
				case 1:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart2 );
					break;
				case 2:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart3 );
					break;
				case 3:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart4 );
					break;
				case 4:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart5 );
					break;
				case 5:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart6 );
					break;
				default:
					trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.sfx_pkafart7 );
				break;
			}
		}

		//PKMOD - Ergodic 01/14/04 - add gib effect at beans blast area
		trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
		CG_GibPlayer( cent->lerpOrigin );

		break;



	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {

//PKMOD - Ergodic 09/30/01 - debug missing EV_LIGHTNING_FX event (inactive)
/*
	entityState_t	*es;
	int				event;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;
	if ( event == EV_LIGHTNING_FX )
		Com_Printf( "CG_CheckEvents detected!\n" );

*/

	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

