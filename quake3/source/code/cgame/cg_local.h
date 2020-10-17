// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "../game/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#ifdef MISSIONPACK
#define CG_FONT_THRESHOLD 0.1
#endif

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
//PKMOD - Ergodic 05/11/01 - make a smaller icon size for display multiple holdables
#define	ICON_SIZE_SMALL		32

#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		8

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"sarge"
#ifdef MISSIONPACK
#define	DEFAULT_TEAM_MODEL		"james"
#define	DEFAULT_TEAM_HEAD		"*james"
#else
#define	DEFAULT_TEAM_MODEL		"sarge"
#define	DEFAULT_TEAM_HEAD		"sarge"
#endif

#define DEFAULT_REDTEAM_NAME		"Stroggs"
#define DEFAULT_BLUETEAM_NAME		"Pagans"

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
	lerpFrame_t		legs, torso, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	// railgun trail spawning
	vec3_t			railgunImpact;
	qboolean		railgunFlash;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	int				snapShotTime;	// last time this entity was found in a snapshot
	//PKMOD - Ergodic 10/20/01 - hold the radiate state timer
	int				PKA_RadiateTime;
	//PKMOD - Ergodic 11/30/01 - hold the radiate infected state timer
	int				PKA_RadiateInfectTime;
	//PKMOD - Ergodic 11/17/01 - add new custom shader for radiate effect on simple items
	qhandle_t		PKA_customShader;		// use one image for the entire thing

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
	LE_SCALED_SPRITE_EXPLOSION,		//PKMOD - Ergodic 01/12/04 - add LE for quad beans

#ifdef MISSIONPACK
	LE_KAMIKAZE,
	LE_INVULIMPACT,
	LE_INVULJUICED,
	LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2			 = 0x0008			// sound 2 for kamikaze
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;		
} localEntity_t;

//======================================================================


typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	qboolean	perfect;
	int				team;
	//PKMOD - Ergodic 02/28/04 - add PainKiller medal awards to ownerdraw 
	int				painkillerCount;
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot
	//PKMOD - Ergodic 01/09/02 - add info to structure so Private Bot will not appear in scoreboard 
	int				privateBot;		// 0 = Not Private Bot, 1 = Private Bot

	vec3_t			color1;
	vec3_t			color2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;

	int				breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	char			headModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
	char			redTeam[MAX_TEAMNAME];
	char			blueTeam[MAX_TEAMNAME];
	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;

	qhandle_t		modelIcon;

	animation_t		animations[MAX_TOTALANIMATIONS];

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
	qboolean		loopFireSound;
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;

//PKMOD - Ergodic 10/13/00 - add hubinfo fields
typedef struct {
	char			info[128];
} hubInfoDisplay_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16
 
typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;
	
	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];
	//PKMOD - Ergodic 01/27/02 - add slower rotating items
	vec3_t		autoAnglesSlow;
	vec3_t		autoAxisSlow[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start

	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera


	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;

	qhandle_t	redCubeModel;
	qhandle_t	blueCubeModel;
	qhandle_t	redCubeIcon;
	qhandle_t	blueCubeIcon;
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];
	qhandle_t	flagShader[4];

	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	qhandle_t	redFlagFlapSkin;
	qhandle_t	blueFlagFlapSkin;
	qhandle_t	neutralFlagFlapSkin;

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

#ifdef MISSIONPACK
	qhandle_t	overloadBaseModel;
	qhandle_t	overloadTargetModel;
	qhandle_t	overloadLightsModel;
	qhandle_t	overloadEnergyModel;

	qhandle_t	harvesterModel;
	qhandle_t	harvesterRedSkin;
	qhandle_t	harvesterBlueSkin;
	qhandle_t	harvesterNeutralModel;
#endif

	qhandle_t	armorModel;
	qhandle_t	armorIcon;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	smoke2;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	railRingsShader;
	qhandle_t	railCoreShader;

	qhandle_t	lightningShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;

	qhandle_t	selectShader;
	qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	smokePuffShader;
	qhandle_t	smokePuffRageProShader;
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	plasmaBallShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;
#ifdef MISSIONPACK
	qhandle_t	nailPuffShader;
	qhandle_t	blueProxMine;
#endif

	qhandle_t	numberShaders[11];

	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;

	// powerup shaders
	qhandle_t	quadShader;
	qhandle_t	redQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	battleWeaponShader;
	qhandle_t	hastePuffShader;
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
	qhandle_t	railExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;

	// special effects models
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;
#ifdef MISSIONPACK
	qhandle_t	kamikazeEffectModel;
	qhandle_t	kamikazeShockWave;
	qhandle_t	kamikazeHeadModel;
	qhandle_t	kamikazeHeadTrail;
	qhandle_t	guardPowerupModel;
	qhandle_t	scoutPowerupModel;
	qhandle_t	doublerPowerupModel;
	qhandle_t	ammoRegenPowerupModel;
	qhandle_t	invulnerabilityImpactModel;
	qhandle_t	invulnerabilityJuicedModel;
	qhandle_t	medkitUsageModel;
	qhandle_t	dustPuffShader;
	qhandle_t	heartShader;
#endif
	qhandle_t	invulnerabilityPowerupModel;

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;

	// sounds
	sfxHandle_t	quadSound;
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	sfxHandle_t	sfx_railg;
	sfxHandle_t	sfx_rockexp;
	sfxHandle_t	sfx_plasmaexp;
#ifdef MISSIONPACK
	sfxHandle_t	sfx_proxexp;
	sfxHandle_t	sfx_nghit;
	sfxHandle_t	sfx_nghitflesh;
	sfxHandle_t	sfx_nghitmetal;
	sfxHandle_t	sfx_chghit;
	sfxHandle_t	sfx_chghitflesh;
	sfxHandle_t	sfx_chghitmetal;
	sfxHandle_t kamikazeExplodeSound;
	sfxHandle_t kamikazeImplodeSound;
	sfxHandle_t kamikazeFarSound;
	sfxHandle_t useInvulnerabilitySound;
	sfxHandle_t invulnerabilityImpactSound1;
	sfxHandle_t invulnerabilityImpactSound2;
	sfxHandle_t invulnerabilityImpactSound3;
	sfxHandle_t invulnerabilityJuicedSound;
	sfxHandle_t obeliskHitSound1;
	sfxHandle_t obeliskHitSound2;
	sfxHandle_t obeliskHitSound3;
	sfxHandle_t	obeliskRespawnSound;
	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;
	sfxHandle_t	youSuckSound;
#endif
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t hitSound;
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

	sfxHandle_t weaponHoverSound;

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t neutralFlagReturnedSound;
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t	youHaveFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
	sfxHandle_t holyShitSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

#ifdef MISSIONPACK
	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t teamLeaderShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;
	qhandle_t flagShaders[3];
	sfxHandle_t	countPrepareTeamSound;

	sfxHandle_t ammoregenSound;
	sfxHandle_t doublerSound;
	sfxHandle_t guardSound;
	sfxHandle_t scoutSound;
#endif

	//PKMOD - Ergodic 01/28/04 - Dynamic HUD activation: move flagShaders & heartShader to active code
	qhandle_t flagShaders[3];
	qhandle_t	heartShader;

	//PKMOD - Ergodic 01/30/04 - Dynamic HUD activation: move the following Shaders to active code
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;

	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
	sfxHandle_t	n_healthSound;
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;
	//PKMOD - Ergodic 12/19/00 - remove Team Arena proxmine sounds
//	sfxHandle_t	wstbimplSound;
//	sfxHandle_t	wstbimpmSound;
//	sfxHandle_t	wstbimpdSound;
//	sfxHandle_t	wstbactvSound;

	//PKMOD - Ergodic 05/21/00 add models and sounds and shaders
	sfxHandle_t	sfx_pkagravitylaunched;
	qhandle_t	pkabeartrapgib1;
	qhandle_t	pkabeartrapgib2;
	qhandle_t	pkabeartrapgib3;
	qhandle_t	pkabeartrapgib4;
	qhandle_t	pkabeartrap; //Ergodic 05/31/00
	qhandle_t	pkabeartrapfollow; //Ergodic 06/11/00
	sfxHandle_t	sfx_pkabeartrapdrop;
	sfxHandle_t	sfx_pkabeartrapbreakup;
	sfxHandle_t	sfx_pkabeartrapsnap;

	//PKMOD - Ergodic 03/23/01 - add team parameters
	qhandle_t	pkabeartrap_red;
	qhandle_t	pkabeartrap_blue;


	//PKMOD - Ergodic 01/16/01 - add model for exploding shells weaponhit
	qhandle_t	explshellsFlashModel;
	
	qhandle_t	shellsExplosionShader1; //Ergodic 06/18/00 (updated 01/16/01)
	qhandle_t	shellsExplosionShader2; //Ergodic 01/16/01
	qhandle_t	shellsExplosionShader3; //Ergodic 01/16/01
	qhandle_t	shellsExplosionShader4; //Ergodic 01/16/01
	qhandle_t	shellsExplosionShader5; //Ergodic 01/16/01
	qhandle_t	shellsExplosionShader6; //Ergodic 01/16/01

	qhandle_t	pkagravitywelluniverse;  //Ergodic 03/18/01

	//PKMOD - Ergodic 08/01/00 - nailgun models/shaders
	qhandle_t	nailImpactShader;
	qhandle_t	nailFlashModel;
	qhandle_t	nailMarkShader;
	qhandle_t	nail1;  //Ergodic 08/03/00 static nail entity

	//PKMOD - Ergodic 08/08/00 PAINKILLER awarded after every 10 PKitem kills
	qhandle_t	medalPainKiller;
	sfxHandle_t	painkillerSound;

	//PKMOD - Ergodic 08/21/00 ChainLightning special shaders
	qhandle_t	chainlightningShader;

	//PKMOD - Ergodic 08/22/00 ChainLightning Gun player hit shader effect
	qhandle_t	clgplayerhitShader;

	//PKMOD - Ergodic 08/22/00 ChainLightning strike sounds
	sfxHandle_t	sfx_chainlightningstrike1;
	sfxHandle_t	sfx_chainlightningstrike2;

	//PKMOD - Ergodic 08/25/00 nailgun ricochet sounds
	sfxHandle_t	sfx_nailrico1;
	sfxHandle_t	sfx_nailrico2;
	sfxHandle_t	sfx_nailrico3;
	sfxHandle_t	sfx_nailrico4;

	//PKMOD - Ergodic 09/06/00 gravity well item suck sounds from Mongusta
	sfxHandle_t	sfx_pkagravitywell_suck1;
	sfxHandle_t	sfx_pkagravitywell_suck2;
	sfxHandle_t	sfx_pkagravitywell_suck3;

	//PKMOD - Ergodic 09/29/00 shaders for hub voting images
	qhandle_t	voting_levelshot_0;
	qhandle_t	voting_levelshot_1;
	qhandle_t	voting_levelshot_2;
	qhandle_t	voting_levelshot_3;
	qhandle_t	voting_levelshot_4;
	qhandle_t	voting_levelshot_5;
	qhandle_t	voting_levelshot_6;
	qhandle_t	voting_levelshot_7;
	qhandle_t	voting_levelshot_8;
	qhandle_t	voting_levelshot_9;
	qhandle_t	voting_levelshot_10;
	qhandle_t	voting_levelshot_11;
	qhandle_t	voting_levelshot_12;
	qhandle_t	voting_levelshot_13;
	qhandle_t	voting_levelshot_14;
	qhandle_t	voting_levelshot_15;
	qhandle_t	voting_levelshot_16;
	qhandle_t	voting_levelshot_17;
	qhandle_t	voting_levelshot_18;
	qhandle_t	voting_levelshot_19;
	qhandle_t	voting_levelshot_20;
	qhandle_t	voting_levelshot_21;
	qhandle_t	voting_levelshot_22;
	qhandle_t	voting_levelshot_23;
	qhandle_t	voting_levelshot_24;

	//PKMOD - Ergodic 10/03/00 dragon beam shader effect
	qhandle_t	dragonboltShader;

	//PKMOD - Ergodic 11/12/00 airfist flash model shader
	qhandle_t	airfistFlashShader;

	//PKMOD - Ergodic 11/16/00 update airfist flash model to correspond to airfist_level
	qhandle_t	airfist4FlashModel;
	qhandle_t	airfist3FlashModel;
	qhandle_t	airfist2FlashModel;
	qhandle_t	airfist1FlashModel;
	qhandle_t	airfist0FlashModel;

	//PKMOD - Ergodic 11/22/00 add autosentry handles
	sfxHandle_t	sfx_pkasentrydrop;
	//PKMOD - Ergodic 11/27/00 autosentry pickup model will be pkasentry_deploy0
//	qhandle_t	pkasentry_pickup;
	//PKMOD - Ergodic 11/25/00 add autosentry deploy models
	//PKMOD - Ergodic 05/31/02 - use autosentry animation model
	/*
	qhandle_t	pkasentry_deploy0;
	qhandle_t	pkasentry_deploy1;
	qhandle_t	pkasentry_deploy2;
	qhandle_t	pkasentry_deploy3;
	qhandle_t	pkasentry_deploy4;
	qhandle_t	pkasentry_deploy5;
	qhandle_t	pkasentry_deploy6;
	qhandle_t	pkasentry_deploy7;
	qhandle_t	pkasentry_deploy8;
	qhandle_t	pkasentry_deploy9;
	qhandle_t	pkasentry_deploy10;
	*/
	//PKMOD - Ergodic 03/20/01 - add team parameters
	//PKMOD - Ergodic 05/31/02 - use autosentry animation model
	qhandle_t	pkasentry_red;
	qhandle_t	pkasentry_blue;
	//PKMOD - Ergodic 06/01/02 - add default autosentry model for dragon deploy
	qhandle_t	pkasentry;
	/*
	qhandle_t	pkasentry_deploy0_red;
	qhandle_t	pkasentry_deploy1_red;
	qhandle_t	pkasentry_deploy2_red;
	qhandle_t	pkasentry_deploy3_red;
	qhandle_t	pkasentry_deploy4_red;
	qhandle_t	pkasentry_deploy5_red;
	qhandle_t	pkasentry_deploy6_red;
	qhandle_t	pkasentry_deploy7_red;
	qhandle_t	pkasentry_deploy8_red;
	qhandle_t	pkasentry_deploy9_red;
	qhandle_t	pkasentry_deploy10_red;
	qhandle_t	pkasentry_deploy0_blue;
	qhandle_t	pkasentry_deploy1_blue;
	qhandle_t	pkasentry_deploy2_blue;
	qhandle_t	pkasentry_deploy3_blue;
	qhandle_t	pkasentry_deploy4_blue;
	qhandle_t	pkasentry_deploy5_blue;
	qhandle_t	pkasentry_deploy6_blue;
	qhandle_t	pkasentry_deploy7_blue;
	qhandle_t	pkasentry_deploy8_blue;
	qhandle_t	pkasentry_deploy9_blue;
	qhandle_t	pkasentry_deploy10_blue;
	*/

	//PKMOD - Ergodic 12/02/00 add split autosentry models
	qhandle_t	pkasentry_base;
	//PKMOD - Ergodic 03/20/01 - add team parameters
	qhandle_t	pkasentry_base_red;
	qhandle_t	pkasentry_base_blue;

	//PKMOD - Ergodic 03/20/01 - add team parameters
	qhandle_t	pkasentry_turret;
	qhandle_t	pkasentry_turret_red;
	qhandle_t	pkasentry_turret_blue;

	//PKMOD - Ergodic 12/14/00 - add autosentry gib models
	qhandle_t	pkasentry_gib1;
	qhandle_t	pkasentry_gib2;
	qhandle_t	pkasentry_gib3;
	qhandle_t	pkasentry_gib4;
	qhandle_t	pkasentry_gib5;
	qhandle_t	pkasentry_gib6;
	qhandle_t	pkasentry_gib7;
	qhandle_t	pkasentry_gib8;
	qhandle_t	pkasentry_gib9;
	qhandle_t	pkasentry_gib10;
	qhandle_t	pkasentry_gib11;
	qhandle_t	pkasentry_gib12;
	qhandle_t	pkasentry_gib13;
	qhandle_t	pkasentry_gib14;
	qhandle_t	pkasentry_gib15;
	qhandle_t	pkasentry_gib16;
	qhandle_t	pkasentry_gib17;

	//PKMOD - Ergodic 12/26/00 add beans fart sounds
	sfxHandle_t	sfx_pkafart1;
	sfxHandle_t	sfx_pkafart2;
	sfxHandle_t	sfx_pkafart3;
	sfxHandle_t	sfx_pkafart4;
	sfxHandle_t	sfx_pkafart5;
	//PKMOD - Ergodic 06/30/01 add two more fart sounds from original Q1 PK
	sfxHandle_t	sfx_pkafart6;
	sfxHandle_t	sfx_pkafart7;

	//PKMOD - Ergodic 06/30/01 add airfist sounds for all types of situations
	sfxHandle_t	sfx_pkaairfistfire;
	sfxHandle_t	sfx_pkaairfistwaterfire;
	sfxHandle_t	sfx_pkaairfistempty;
	sfxHandle_t	sfx_pkaairfistwaterempty;

	//PKMOD - Ergodic 12/27/00 add beans shader
	qhandle_t	pkafartPuffShader;

	//PKMOD - Ergodic 12/28/00 add flash model for autosentry
	qhandle_t	autosentryFlashModel;

	//PKMOD - Ergodic 01/13/01 - add autosentry fire sounds from mongusta
	qhandle_t	sfx_pkasentry1;
	qhandle_t	sfx_pkasentry2;
	qhandle_t	sfx_pkasentry3;

	//PKMOD - Ergodic 01/21/01 - add coordinate model for exploding shells debug
	qhandle_t	coordFlashModel;

	//PKMOD - Ergodic 03/26/01 - autosentry sonar ping sounds
	sfxHandle_t	sfx_pkasentry_ping1;
	sfxHandle_t	sfx_pkasentry_ping2;
	sfxHandle_t	sfx_pkasentry_ping3;

	//PKMOD - Ergodic 04/06/01 - add autosentry missile sprite
	qhandle_t	autosentryBallShader;

	//PKMOD - Ergodic 06/20/01 - add legs model for swinging zombie
	qhandle_t	pkazombie_legsModel;
	qhandle_t	pkazombie_legsSkin;

	//PKMOD - Ergodic 06/21/01 - add torso model for swinging zombie
	qhandle_t	pkazombie_torsoModel;
	qhandle_t	pkazombie_torsoSkin;

	//PKMOD - Ergodic 06/21/01 - add head model for swinging zombie
	qhandle_t	pkazombie_headModel;
	qhandle_t	pkazombie_headSkin;

	//PKMOD - Ergodic 07/03/01 ChainLightning reflect sounds
	sfxHandle_t	sfx_chainlightningreflect1;
	sfxHandle_t	sfx_chainlightningreflect2;

	//PKMOD - Ergodic 10/14/01 - add shaders for radiation holdable spark effect
	qhandle_t	radiate1Shader;
	qhandle_t	radiate2Shader;
	qhandle_t	radiate3Shader;
	qhandle_t	radiate4Shader;
	qhandle_t	radiate5Shader;
	qhandle_t	radiate6Shader;

	//PKMOD - Ergodic 11/17/01 - add icons for simple items that are radiated
	qhandle_t	radiate1SimpleIcon;
	qhandle_t	radiate2SimpleIcon;
	qhandle_t	radiate3SimpleIcon;

	//PKMOD - Ergodic 11/27/01 - add radition trail for infected players
	qhandle_t	radiationTrailShader;

	//PKMOD - Ergodic 12/03/01 - New Holdables - Private Bot pickup skins
	qhandle_t	privatebot_legsSkin;
	qhandle_t	privatebot_torsoSkin;
	qhandle_t	privatebot_headSkin;

	//PKMOD - Ergodic 12/05/01 - Holdable: radiate sounds
	qhandle_t	pkaradiatewarningSound;
	qhandle_t	pkaradiateitemSound;
	qhandle_t	pkaradiateplayerSound;
	//PKMOD - Ergodic 08/02/02 - Holdable: radiate activation sound
	sfxHandle_t pkaradiateactivationSound;

	//PKMOD - Ergodic 12/07/01 - Holdable: Private Bot HUD Icons
	qhandle_t	pkapribot_001Icon;
	qhandle_t	pkapribot_010Icon;
	qhandle_t	pkapribot_011Icon;
	qhandle_t	pkapribot_100Icon;
	qhandle_t	pkapribot_101Icon;
	qhandle_t	pkapribot_110Icon;
	qhandle_t	pkapribot_111Icon;

	//PKMOD - Ergodic 12/16/01 - add new model for repositioned deployed gauntlet blade
//	qhandle_t	pkagauntlet_bladeModel;

	//PKMOD - Ergodic 02/05/02 - add attack sounds for beartrap and autosentry
	sfxHandle_t beartrap_attackSound;
	sfxHandle_t autosentry_attackSound;
	sfxHandle_t radiate_attackSound;

	//PKMOD - Ergodic 02/07/02 - add Private Bot completed sound
	sfxHandle_t pkapribot_complete;

	//PKMOD - Ergodic 02/10/02 - send FRAG message to Private Bot's owner
	sfxHandle_t pkapribot_frag1;
	sfxHandle_t pkapribot_frag2;

	//PKMOD - Ergodic 02/14/02 - explosive shells hit sounds
	sfxHandle_t	sfx_expgunhit1;
	sfxHandle_t	sfx_expgunhit2;
	sfxHandle_t	sfx_expgunhit3;
	//PKMOD - Ergodic 07/10/02 - add 2 more explosive shells hit sounds
	sfxHandle_t	sfx_expgunhit4;
	sfxHandle_t	sfx_expgunhit5;

	//PKMOD - Ergodic 05/07/02 - add active personal sentry model
	qhandle_t	persentry_active;

	//PKMOD - Ergodic 06/08/02 - add personal sentry teleport model
	qhandle_t	persentry_teleportEffectModel;

	//PKMOD - Ergodic 06/12/02 - add personal sentry missile shader
	qhandle_t	personalsentryBallShader;

	//PKMOD - Ergodic 08/02/02 - Holdable: Personal Sentry hover sound
	qhandle_t	pkapersentryhoverSound;
	//PKMOD - Ergodic 08/26/02 - add Personal Sentry fire sounds from StarDagger
	qhandle_t	pkapersentry_fire1;
	qhandle_t	pkapersentry_fire2;
	qhandle_t	pkapersentry_fire3;

	//PKMOD - Ergodic 09/11/02 - add private bot field effect
	qhandle_t	privatebot_CueModel;

	//PKMOD - Ergodic 10/14/02 - Add the gravity well expanding wave
	qhandle_t	pkagravitywellwave;

	//PKMOD - Ergodic 10/18/02 - Add the gravity well spark
	qhandle_t	pkagravitywellspark;

	//PKMOD - Ergodic 07/18/03 - add invisible Beartrap Shaders
	qhandle_t	pkainvisbeartrap1;
	qhandle_t	pkainvisbeartrap2;
	qhandle_t	pkainvisbeartrap3;
	qhandle_t	pkainvisbeartrap4;
	qhandle_t	pkainvisbeartrap5;
	qhandle_t	pkainvisbeartrap6;
	qhandle_t	pkainvisbeartrap7;
	qhandle_t	pkainvisbeartrap8;
	qhandle_t	pkainvisbeartrap9;
	qhandle_t	pkainvisbeartrap10;
	qhandle_t	pkainvisbeartrap11;
	qhandle_t	pkainvisbeartrap12;
	qhandle_t	pkainvisbeartrap13;
	qhandle_t	pkainvisbeartrap14;
	qhandle_t	pkainvisbeartrap15;
	qhandle_t	pkainvisbeartrap16;
	qhandle_t	pkainvisbeartrap17;
	qhandle_t	pkainvisbeartrap18;
	qhandle_t	pkainvisbeartrap19;
	qhandle_t	pkainvisbeartrap20;

	//PKMOD - Ergodic 08/20/03 - add special shader for shooter lightning
	qhandle_t	shooterlightningShader;

	//PKMOD - Ergodic 09/18/03 - Beartrap Invisibility Spark sprite shaders
	qhandle_t	pkabeartrapspark1Shader;
	qhandle_t	pkabeartrapspark2Shader;
	qhandle_t	pkabeartrapspark3Shader;
	qhandle_t	pkabeartrapspark4Shader;
	qhandle_t	pkabeartrapspark5Shader;

	//PKMOD - Ergodic 11/21/03 - Earthquake sound for Gravity Well effect on out of reach players
	//PKMOD - Ergodic 12/07/03 - removed, code moved to global sound
	//qhandle_t	pkaearthquake;

	//PKMOD - Ergodic 12/08/03 - Add chargeup sound for BearTrap, Autosentry invisibility
	qhandle_t	pkachargeup;

	//PKMOD - Ergodic 12/13/03 - add invisible Autosentry Shaders
	qhandle_t	pkainvisautosentry1;
	qhandle_t	pkainvisautosentry2;
	qhandle_t	pkainvisautosentry3;
	qhandle_t	pkainvisautosentry4;
	qhandle_t	pkainvisautosentry5;
	qhandle_t	pkainvisautosentry6;
	qhandle_t	pkainvisautosentry7;
	qhandle_t	pkainvisautosentry8;
	qhandle_t	pkainvisautosentry9;
	qhandle_t	pkainvisautosentry10;
	qhandle_t	pkainvisautosentry11;
	qhandle_t	pkainvisautosentry12;
	qhandle_t	pkainvisautosentry13;
	qhandle_t	pkainvisautosentry14;
	qhandle_t	pkainvisautosentry15;
	qhandle_t	pkainvisautosentry16;
	qhandle_t	pkainvisautosentry17;
	qhandle_t	pkainvisautosentry18;
	qhandle_t	pkainvisautosentry19;
	qhandle_t	pkainvisautosentry20;

	//PKMOD - Ergodic 01/07/04 - add quad farting logic for differing CG graphic sequence
	qhandle_t	pkaquadbeansShader;
	qhandle_t	pkaquadbeansModel;

	//PKMOD - Ergodic 03/17/04 - add quad beans fart sounds
	sfxHandle_t	sfx_pkaquadfart1;
	sfxHandle_t	sfx_pkaquadfart2;
	sfxHandle_t	sfx_pkaquadfart3;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];
	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// orders
	int currentOrder;
	qboolean orderPending;
	int orderTime;
	int currentVoiceClient;
	int acceptOrderTime;
	int acceptTask;
	int acceptLeader;
	char acceptVoice[MAX_NAME_LENGTH];

	// media
	cgMedia_t		media;

	//PKMOD - Ergodic 10/13/00 - add hubinfo fields
	hubInfoDisplay_t	hubInfoDisplay[3];

} cgs_t;


//PKMOD - Ergodic 05/11/01 - register holdables into their own array
//		for optimization purposes
extern	int				cg_holdable[ HI_NUM_HOLDABLE ];

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_noVoiceChats;
extern	vmCvar_t		cg_noVoiceText;
extern  vmCvar_t		cg_scorePlum;
extern	vmCvar_t		cg_smoothClients;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
//extern	vmCvar_t		cg_pmove_fixed;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_noTaunt;
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_oldRail;
extern	vmCvar_t		cg_oldRocket;
extern	vmCvar_t		cg_oldPlasma;
extern	vmCvar_t		cg_trueLightning;
//PKMOD - Ergodic 08/16/03 - add cvar for PKA full weapon cycling
extern	vmCvar_t		cg_pkafullweaponcycling;

#ifdef MISSIONPACK
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_currentSelectedPlayerName;
extern	vmCvar_t		cg_singlePlayer;
extern	vmCvar_t		cg_enableDust;
extern	vmCvar_t		cg_enableBreath;
extern	vmCvar_t		cg_singlePlayerActive;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;
extern	vmCvar_t		cg_obeliskRespawnDelay;
#endif

//PKMOD - Ergodic 02/02/04 - Enable this code so that POSTGAME will show proper completion time 
extern	vmCvar_t		cg_singlePlayerActive;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;

//PKMOD - Ergodic 01/17/04 - Enable HUD in PKA3.0
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_currentSelectedPlayerName;


//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

//PKMOD - Ergodic 10/14/00 - add post vote music
void CG_StartPostVoteMusic( const char *postvotemusic );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore();
void CG_BuildSpectatorString();


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string, 
				   float charWidth, float charHeight, const float *modulate );


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

//PKMOD - Ergodic 10/13/00 - add hubinfo fields
extern	int numHubInfoLines;

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer();
void CG_SelectNextPlayer();
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead();
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat();
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText();
const char *CG_GetKillerText();
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending();
const char *CG_GameTypeString();
qboolean CG_YourTeamHasFlag();
qboolean CG_OtherTeamHasFlag();
qhandle_t CG_StatusHandle(int task);



//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
//PKMOD - Ergodic 12/06/00 - added for special lightning shooter event
void CG_ChainLightning( centity_t *cent );
//PKMOD - Ergodic 01/21/01 - add event to display coordinate model for exploding shells debug
void CG_Coord( entityState_t *es );
//PKMOD - Ergodic - debug code
char	*CG_vtos( const vec3_t v );

//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
void CG_ShotgunFire( entityState_t *es );
//void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );
//PKMOD - Ergodic 12/14/00 - add autosentry code for greater tracability 
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, qboolean autosentry );

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?

//PKMOD - Ergodic 07/02/00 PKA weapon commands
void CG_Weapon_GravityWell( void );
void CG_Weapon_Sentry( void );
void CG_Weapon_BearTrap( void );
void CG_Weapon_Beans( void );

//PKMOD - Ergodic 07/12/00 PKA weapon commands
void CG_Weapon_Gauntlet( void );
void CG_Weapon_MachineGun( void );
void CG_Weapon_ShotGun( void );
void CG_Weapon_AirFist( void );
void CG_Weapon_NailGun( void );
void CG_Weapon_GrenadeLauncher( void );
void CG_Weapon_RocketLauncher( void );
void CG_Weapon_LightningGun( void );
void CG_Weapon_RailGun( void );
void CG_Weapon_Harpoon( void );

//PKMOD - Ergodic 03/01/01 dragon deploy pka weapon 
void CG_Weapon_DragonDeploy( void );

//PKMOD - Ergodic 04/04/01 - add last weapon command 
void CG_LastWeapon_f( void );

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader, 
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p, 
				   const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
#ifdef MISSIONPACK
void CG_KamikazeEffect( vec3_t org );
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite );

//PKMOD - Ergodic 05/23/00 - 05/26/00 event/effect routines
void CG_BearTrapDie( vec3_t BearTrapOrigin );
//PKMOD - Ergodic 06/30/00 - modify beartrap code to be called by cg_player
void CG_BearTraps_Follow( centity_t *cent );
//PKMOD - Ergodic 07/01/00 - chain lightning gun stubs
void CG_ChainShaft( entityState_t *es );
//PKMOD - Ergodic 07/19/00 - display lightning flash
void CG_Lightning_FX( vec3_t position, int constantlight );
//PKMOD - Ergodic 11/22/00 - autosentry death routine
void CG_AutoSentryDie( vec3_t AutoSentryOrigin );

//PKMOD - Ergodic 01/15/01 - add Lightning Discharge in water
void CG_Lightning_Water_Discharge (vec3_t origin, int msec);

//PKMOD - Ergodic 02/20/01 - Add expanding gravity well
//void CG_GravityWellActivate( vec3_t org );
//PKMOD - Ergodic 11/02/02 - Pass in the whole centity
//PKMOD - Ergodic 11/02/02 - remove call
//CG_GravityWellActivate( centity_t *cent );

//PKMOD - Ergodic 04/13/01 - Renders fecal plumes or bubbles from eating beans
void CG_BeansToot( vec3_t origin );

//PKMOD - Ergodic 10/14/01 - add radiation sparking effect
void CG_Radiation( vec3_t start );

//PKMOD - Ergodic 11/30/01 - add radiation trail to infected player
//void CG_RadiationTrail0( const vec3_t p );

//PKMOD - Ergodic 12/01/01 - add new radiation trail function to infected player
void CG_RadiationTrail( const vec3_t p, 
				   vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );

//PKMOD - Ergodic 06/08/02 - add personal sentry teleport model
void CG_PersentrySpawnEffect( vec3_t org );

//PKMOD - Ergodic 01/05/04 - add quad farting logic for differing CG graphic sequence
void CG_QuadBeansToot( vec3_t origin );


//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_LoadVoiceChats( void );
void CG_ShaderStateChanged(void);
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );

//
//PKMOD - Ergodic 05/22/00 Add CG functions
//
void CG_PKA_OutOfAmmoChange( void );



//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );
int			trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
void	trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h, 
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );


typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t; // bk001201 - warning: useless keyword or type name in empty declaration

//PKMOD - Ergodic 10/10/2000 - voting images' shader assignments
extern		char	cg_voting_shader_flag[];

int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

void trap_SnapVector( float *v );

qboolean	trap_loadCamera(const char *name);
void		trap_startCamera(int time);
qboolean	trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );
//PKMOD - Ergodic 10/18/02 - draw super-sized particles...
void	CG_ParticleSparks2 (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
//PKMOD - Ergodic 07/20/03 - particles for beartrap
void CG_ParticleSparks3 (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);


