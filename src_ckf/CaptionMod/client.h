#pragma once

#include <vector>

typedef struct
{
	qboolean bIsAlive;
	int iTeam;
	int iClass;
	int iHealth;
	int iFrags;
	int iDeaths;
	int iDominates;
	int iPing;
}PlayerInfo;

typedef struct
{
	int iKill;
	int iDeath;
	int iAssist;
	int iDemolish;
	int iCapture;
	int iDefence;
	int iDominate;
	int iRevenge;
	int iUbercharge;
	int iHeadshot;
	int iTeleport;
	int iHealing;
	int iBackstab;
	int iBonus;
}PlayerStatsInfo;

typedef struct
{
	int iSlot;
	int iMaxClip;
	int iMaxAmmo;
	int iState;
	int iBody;
	int iSkin;
}WeaponInfo;

typedef struct
{
	int level;
	int flags;
	float health;
	float maxhealth;
	int upgrade;
	float progress;
	float updatetime;
	cl_entity_t *ent;
}buildable_t;

typedef struct
{
	int level;
	int flags;
	float health;
	float maxhealth;
	int upgrade;
	float progress;
	float updatetime;
	cl_entity_t *ent;

	int ammo;
	int maxammo;
	int rocket;
	int maxrocket;	
	int killcount;
}sentry_t;

typedef struct
{
	int level;
	int flags;
	float health;
	float maxhealth;
	int upgrade;
	float progress;
	float updatetime;
	cl_entity_t *ent;

	int metal;
	int maxmetal;
}dispenser_t;

typedef struct
{
	int level;
	int flags;
	float health;
	float maxhealth;
	int upgrade;
	float progress;
	float updatetime;
	cl_entity_t *ent;

	float charge;
	float chargerate;
	float chargetime;
	int ready;
	int frags;
}telein_t;

typedef struct
{
	int level;
	int flags;
	float health;
	float maxhealth;
	int upgrade;
	float progress;
	float updatetime;
	cl_entity_t *ent;
}teleout_t;

typedef struct
{
	sentry_t sentry;
	dispenser_t dispenser;
	telein_t telein;
	teleout_t teleout;
}BuildInfo;

typedef struct
{
	physent_t physents[32];
	int numphysent;
}customzone_t;

typedef struct
{
	int iHudPosition;
	char szName[32];
	int iState;
	int iCapTeam;
	int iTeam;
	int iCapPlayers;
	float flProgress;
	float flCapRate;
	float flMessageTime;
}controlpoint_t;

typedef std::vector<physent_t> zonevector;
typedef std::vector<controlpoint_t> cpvector;

int CL_GetViewBody(void);
int CL_GetViewSkin(void);

qboolean CL_IsAlive(void);
qboolean CL_CanDrawViewModel(void);
qboolean CL_IsThirdPerson(void);
qboolean CL_IsFirstPersonSpec(void);
qboolean CL_IsValidPlayer(int team, int playerclass);

void CL_SetupPMTrace(int idx);
void CL_FinishPMTrace(void);
int CL_TraceEntity_Ignore(physent_t *pe);

cl_entity_t *CL_FindEntityInSphere(float *org, float rad, int (*pfnFilter)(cl_entity_t *ent));
model_t *CL_LoadTentModel(const char *pName);
void CL_CreateTempEntity(cl_entity_t *pEntity, model_t *mod);
void ShowHudMenu(int type, int keys);

#define	DEFAULT_VIEWHEIGHT	28
#define VEC_DUCK_VIEW 12

#define OBS_NONE				0
#define OBS_CHASE_LOCKED		1
#define OBS_CHASE_FREE			2
#define OBS_ROAMING				3
#define OBS_IN_EYE				4
#define OBS_MAP_FREE			5
#define OBS_MAP_CHASE			6

#define INSET_OFF				0
#define	INSET_CHASE_FREE		1
#define	INSET_IN_EYE			2
#define	INSET_MAP_FREE			3
#define	INSET_MAP_CHASE			4

#define CLASS_SCOUT 1
#define CLASS_HEAVY 2
#define CLASS_SOLDIER 3
#define CLASS_PYRO 4
#define CLASS_SNIPER 5
#define CLASS_MEDIC 6
#define CLASS_ENGINEER 7
#define CLASS_DEMOMAN 8
#define CLASS_SPY 9

#define WEAPON_ROCKETLAUNCHER 1
#define WEAPON_SNIPERIFLE 2
#define WEAPON_SMG 3
#define WEAPON_KUKRI 4
#define WEAPON_SHOTGUN 5
#define WEAPON_SHOVEL 6
#define WEAPON_STICKYLAUNCHER 7
#define WEAPON_GRENADELAUNCHER 8
#define WEAPON_BOTTLE 9
#define WEAPON_SCATTERGUN 10
#define WEAPON_PISTOL 11
#define WEAPON_BAT 12
#define WEAPON_MINIGUN 13
#define WEAPON_FIST 14
#define WEAPON_FIREAXE 15
#define WEAPON_FLAMETHROWER 16
#define WEAPON_REVOLVER 17
#define WEAPON_SAPPER 18
#define WEAPON_BUTTERFLY 19
#define WEAPON_SYRINGEGUN 20
#define WEAPON_MEDIGUN 21
#define WEAPON_BONESAW 22
#define WEAPON_DISGUISEKIT 23
#define WEAPON_WRENCH 24
#define WEAPON_BUILDPDA 25
#define WEAPON_DESTROYPDA 26

#define MAX_WEAPONS 128

#define MENU_CLOSE 0
#define MENU_CHOOSETEAM 1
#define MENU_CHOOSECLASS 2
#define MENU_INTRO 3

#define HUDMENU_DISGUISE 1
#define HUDMENU_BUILD 2
#define HUDMENU_DEMOLISH 3

#define MENUBUF_MOTD 1
#define MENUBUF_MAPINFO 2
#define MENUBUF_CLASSINTRO 3

#define MENUKEY_1 (1<<0)
#define MENUKEY_2 (1<<1)
#define MENUKEY_3 (1<<2)
#define MENUKEY_4 (1<<3)
#define MENUKEY_5 (1<<4)
#define MENUKEY_6 (1<<5)
#define MENUKEY_7 (1<<6)
#define MENUKEY_8 (1<<7)
#define MENUKEY_9 (1<<8)
#define MENUKEY_0 (1<<9)
#define MENUKEY_10 (1<<10)

#define HUDMENU_DISGUISE 1
#define HUDMENU_BUILD 2
#define HUDMENU_DEMOLISH 3

#define BUILDABLE_SENTRY 1
#define BUILDABLE_DISPENSER 2
#define BUILDABLE_ENTRANCE 3
#define BUILDABLE_EXIT 4

#define HUDBUILD_RESET 255

#define DISGUISE_YES 1
#define DISGUISE_NO 0

#define CLOAK_NO 0
#define CLOAK_BEGIN 1
#define CLOAK_YES 2
#define CLOAK_STOP 3

#define CloakBegin_Duration (1.2f)

#define CloakStop_Duration (2.2f)

enum playerstats_e
{
	STATS_CLEAR = 0,
	STATS_KILL,
	STATS_DEATH,
	STATS_ASSIST,
	STATS_DEMOLISH,
	STATS_CAPTURE,
	STATS_DEFENCE,
	STATS_DOMINATE,
	STATS_REVENGE,
	STATS_UBERCHARGE,
	STATS_HEADSHOT,
	STATS_TELEPORT,
	STATS_HEALING,
	STATS_BACKSTAB,
	STATS_BONUS,
	STATS_MAX_NUM
};

enum fx_e
{
	FX_STICKYTRAIL = 0,
	FX_ROCKETTRAIL,
	FX_GRENADETRAIL,
	FX_KILLTRAIL,
	FX_KILLTRAIL_INSTANT,
	FX_BOTTLEBREAK,
	FX_MULTIJUMPTRAIL,
	FX_EXPLOSION_MIDAIR,
	FX_EXPLOSION_WALL,
	FX_STICKYKILL,
	FX_COORDTRACER,
	FX_ATTACHTRACER,
	FX_MEDIBEAM,
	FX_KILLMEDIBEAM,
	FX_BLUEPRINT,
	FX_DISGUISESMOKE,
	FX_SPYWATCH,
	FX_FLAMETHROW,
	FX_CRITHIT,
	FX_MINICRITHIT,
	FX_HITDAMAGE,
	FX_ADDHEALTH,
	FX_ADDTIME,
	FX_ADDMETAL,
	FX_BURNINGPLAYER,
	FX_CLOAKBEGIN,
	FX_CLOAKSTOP,
	FX_DISGUISEHINT
};

enum
{
	ROUND_NORMAL = 0,
	ROUND_SETUP,
	ROUND_END,
	ROUND_OVERTIME,
	ROUND_WAIT
};

#define EF_INVULNERABLE				(1<<8)//256
#define EF_CRITBOOST				(1<<9)
#define EF_AFTERBURN				(1<<10)
#define EF_SHADOW					(1<<11)
#define EF_3DSKY					(1<<12)
#define EF_3DMENU					(1<<13)

#define FL_LOCK_DUCK			(1<<17)
#define	FL_LOCK_JUMP			(1<<18)
#define FL_MULTIJUMP			(1<<19)

#define CDFLAG_LIMIT (1<<0)
#define CDFLAG_FREEZE (1<<1)
#define CDFLAG_PLANTING (1<<2)
#define CDFLAG_NOBUILD (1<<3)

#define CLASS_PLAYER 2
#define CLASS_WEAPONBOX 20
#define CLASS_PROJECTILE 21
#define CLASS_BUILDABLE 22
#define CLASS_CONTROLPOINT 23

#define MAP_NOBUILDABLE 1
#define MAP_CONTROLPOINT 2
#define MAP_LIGHTSHADOW 3

#define MESSAGEPART_START 0
#define MESSAGEPART_MID 1
#define MESSAGEPART_END 2

#define BUILD_BUILDING (1<<0)
#define BUILD_UPGRADING (1<<1)
#define BUILD_SAPPERED (1<<2)
#define BUILD_EMPED (1<<3)
#define BUILD_ACTIVE (1<<4)

#define HIDEHUD_WEAPONS (1<<0)
#define HIDEHUD_CHARGE (1<<1)
#define HIDEHUD_ALL (1<<2)
#define HIDEHUD_HEALTH (1<<3)
#define HIDEHUD_TIMER (1<<4)
#define HIDEHUD_MONEY (1<<5)
#define HIDEHUD_CROSSHAIR (1<<6)
#define	HIDEHUD_FLASHLIGHT (1<<7)//HL special

#define SCOREATTRIB_DEAD (1<<0)
#define SCOREATTRIB_BOMB (1<<1)
#define SCOREATTRIB_VIP (1<<2)

enum
{
	CP_IDLE = 0,
	CP_CAPTURING,
	CP_BLOCKED,
	CP_BACK,
	CP_LOCKED
};

extern hud_player_info_t g_HudPlayerInfo[33];
extern PlayerInfo g_PlayerInfo[33];
extern WeaponInfo g_WeaponInfo[MAX_WEAPONS];
extern BuildInfo g_Build;
extern PlayerStatsInfo g_PlayerStatsInfo;
extern zonevector g_NoBuildZones;
extern cpvector g_ControlPoints;

extern int g_iClass;
extern int g_iTeam;
extern int g_iHealth;
extern int g_iMaxHealth;
extern int g_iWeaponID;
extern int g_iHideHUD;
extern int g_iForceFOV;
extern int g_iDefaultFOV;

extern int g_iMenu;
extern int g_iMenuKeys;
extern int g_iHudMenu;
extern int g_iHudMenuKeys;

extern int g_iViewModelBody;
extern int g_iViewModelSkin;
extern int g_iDisgMenuTeam;

extern int g_WeaponSelect;

extern int g_iRedTeamScore;
extern int g_iBlueTeamScore;
extern int g_iRedPlayerNum;
extern int g_iBluePlayerNum;
extern int g_iSpectatorNum;
extern int g_iLocalPlayerNum;

extern char g_szServerName[64];

extern int g_iRoundStatus;
extern int g_iLastRoundStatus;
extern int g_iMaxRoundTime;
extern float g_flRoundEndTime;
extern float g_flRoundStatusChangeTime;
extern int g_iCapPointIndex;

extern int g_ScoreBoardEnabled;

#define SPY_MIN_ALPHA 50