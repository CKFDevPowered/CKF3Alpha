#define FL_FLY (1<<0)
#define FL_SWIM (1<<1)
#define FL_CONVEYOR (1<<2)
#define FL_CLIENT (1<<3)
#define FL_INWATER (1<<4)
#define FL_MONSTER (1<<5)
#define FL_GODMODE (1<<6)
#define FL_NOTARGET (1<<7)
#define FL_SKIPLOCALHOST (1<<8)
#define FL_ONGROUND (1<<9)
#define FL_PARTIALGROUND (1<<10)
#define FL_WATERJUMP (1<<11)
#define FL_FROZEN (1<<12)
#define FL_FAKECLIENT (1<<13)
#define FL_DUCKING (1<<14)
#define FL_FLOAT (1<<15)
#define FL_GRAPHED (1<<16)
#define FL_IMMUNE_WATER (1<<17)
#define FL_IMMUNE_SLIME (1<<18)
#define FL_IMMUNE_LAVA (1<<19)

#define FL_PROXY (1<<20)
#define FL_ALWAYSTHINK (1<<21)
#define FL_BASEVELOCITY (1<<22)
#define FL_MONSTERCLIP (1<<23)
#define FL_ONTRAIN (1<<24)
#define FL_WORLDBRUSH (1<<25)
#define FL_SPECTATOR (1<<26)
#define FL_CUSTOMENTITY (1<<29)
#define FL_KILLME (1<<30)
#define FL_DORMANT (1<<31)

#define FTRACE_SIMPLEBOX (1<<0)

#define WALKMOVE_NORMAL 0
#define WALKMOVE_WORLDONLY 1
#define WALKMOVE_CHECKONLY 2

#define MOVETYPE_NONE 0
#define MOVETYPE_ANGLENOCLIP 1
#define MOVETYPE_ANGLECLIP 2
#define MOVETYPE_WALK 3
#define MOVETYPE_STEP 4
#define MOVETYPE_FLY 5
#define MOVETYPE_TOSS 6
#define MOVETYPE_PUSH 7
#define MOVETYPE_NOCLIP 8
#define MOVETYPE_FLYMISSILE 9
#define MOVETYPE_BOUNCE 10
#define MOVETYPE_BOUNCEMISSILE 11
#define MOVETYPE_FOLLOW 12
#define MOVETYPE_PUSHSTEP 13

#define SOLID_NOT 0
#define SOLID_TRIGGER 1
#define SOLID_BBOX 2
#define SOLID_SLIDEBOX 3
#define SOLID_BSP 4

#define DEAD_NO 0
#define DEAD_DYING 1
#define DEAD_DEAD 2
#define DEAD_RESPAWNABLE 3
#define DEAD_DISCARDBODY 4

#define DAMAGE_NO 0
#define DAMAGE_YES 1
#define DAMAGE_AIM 2

#define EF_BRIGHTFIELD 1
#define EF_MUZZLEFLASH 2
#define EF_BRIGHTLIGHT 4
#define EF_DIMLIGHT 8
#define EF_INVLIGHT 16
#define EF_NOINTERP 32
#define EF_LIGHT 64
#define EF_NODRAW 128

#define EFLAG_SLERP 1

#define CONTENTS_EMPTY -1
#define CONTENTS_SOLID -2
#define CONTENTS_WATER -3
#define CONTENTS_SLIME -4
#define CONTENTS_LAVA -5
#define CONTENTS_SKY -6
#define CONTENTS_ORIGIN -7
#define CONTENTS_CLIP -8
#define CONTENTS_CURRENT_0 -9
#define CONTENTS_CURRENT_90 -10
#define CONTENTS_CURRENT_180 -11
#define CONTENTS_CURRENT_270 -12
#define CONTENTS_CURRENT_UP -13
#define CONTENTS_CURRENT_DOWN -14
#define CONTENTS_TRANSLUCENT -15
#define CONTENTS_LADDER -16

#define CONTENT_FLYFIELD -17
#define CONTENT_GRAVITY_FLYFIELD -18
#define CONTENT_FOG -19

#define CONTENT_EMPTY -1
#define CONTENT_SOLID -2
#define CONTENT_WATER -3
#define CONTENT_SLIME -4
#define CONTENT_LAVA -5
#define CONTENT_SKY -6

#define CHAN_AUTO 0
#define CHAN_WEAPON 1
#define CHAN_VOICE 2
#define CHAN_ITEM 3
#define CHAN_BODY 4
#define CHAN_STREAM 5
#define CHAN_STATIC 6
#define CHAN_NETWORKVOICE_BASE 7
#define CHAN_NETWORKVOICE_END 500

#define ATTN_NONE 0
#define ATTN_NORM 0.8
#define ATTN_IDLE 2.0
#define ATTN_STATIC 1.25 

#define PITCH_NORM 100
#define PITCH_LOW 95
#define PITCH_HIGH 120

#define VOL_NORM 1.0

#define PLAT_LOW_TRIGGER 1

#define SF_TRAIN_WAIT_RETRIGGER 1
#define SF_TRAIN_START_ON 4
#define SF_TRAIN_PASSABLE 8

#define IN_ATTACK (1<< 0)
#define IN_JUMP (1<< 1)
#define IN_DUCK (1<< 2)
#define IN_FORWARD (1<< 3)
#define IN_BACK (1<< 4)
#define IN_USE (1<< 5)
#define IN_CANCEL (1<< 6)
#define IN_LEFT (1<< 7)
#define IN_RIGHT (1<< 8)
#define IN_MOVELEFT (1<< 9)
#define IN_MOVERIGHT (1<<10)
#define IN_ATTACK2 (1<<11)
#define IN_RUN (1<<12)
#define IN_RELOAD (1<<13)
#define IN_ALT1 (1<<14)
#define IN_SCORE (1<<15)

#define BREAK_TYPEMASK 0x4F
#define BREAK_GLASS 0x01
#define BREAK_METAL 0x02
#define BREAK_FLESH 0x04
#define BREAK_WOOD 0x08

#define BREAK_SMOKE 0x10
#define BREAK_TRANS 0x20
#define BREAK_CONCRETE 0x40
#define BREAK_2 0x80

#define BOUNCE_GLASS BREAK_GLASS
#define BOUNCE_METAL BREAK_METAL
#define BOUNCE_FLESH BREAK_FLESH
#define BOUNCE_WOOD BREAK_WOOD
#define BOUNCE_SHRAP 0x10
#define BOUNCE_SHELL 0x20
#define BOUNCE_CONCRETE BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80

#define TE_BOUNCE_NULL 0
#define TE_BOUNCE_SHELL 1
#define TE_BOUNCE_SHOTSHELL 2

enum 
{
	kRenderNormal,
	kRenderTransColor,
	kRenderTransTexture,
	kRenderGlow,
	kRenderTransAlpha,
	kRenderTransAdd
};

enum 
{
	kRenderFxNone = 0,
	kRenderFxPulseSlow,
	kRenderFxPulseFast,
	kRenderFxPulseSlowWide,
	kRenderFxPulseFastWide,
	kRenderFxFadeSlow,
	kRenderFxFadeFast,
	kRenderFxSolidSlow,
	kRenderFxSolidFast,
	kRenderFxStrobeSlow,
	kRenderFxStrobeFast,
	kRenderFxStrobeFaster,
	kRenderFxFlickerSlow,
	kRenderFxFlickerFast,
	kRenderFxNoDissipation,
	kRenderFxDistort,
	kRenderFxHologram,
	kRenderFxDeadPlayer,
	kRenderFxExplode,
	kRenderFxGlowShell,
	kRenderFxClampMinScale
};

typedef int string_t;
typedef unsigned char byte;
typedef unsigned short word;

#define NULL 0
#define true 1
#define false 0

typedef float vec3_t[3];
typedef struct edict_s edict_t;

typedef int qboolean;

typedef struct
{
	byte r, g, b;
}
color24;

typedef struct
{
	unsigned r, g, b, a;
}
colorVec;

typedef struct event_args_s
{
	int flags;
	int entindex;

	float origin[3];
	float angles[3];
	float velocity[3];

	int ducking;

	float fparam1;
	float fparam2;

	int iparam1;
	int iparam2;

	int bparam1;
	int bparam2;
}
event_args_t;

#define ENTITY_NORMAL (1<<0)
#define ENTITY_BEAM (1<<1)

typedef struct entity_state_s entity_state_t;

struct entity_state_s
{
	int entityType;
	int number;
	float msg_time;

	int messagenum;

	vec3_t origin;
	vec3_t angles;

	int modelindex;
	int sequence;
	float frame;
	int colormap;
	short skin;
	short solid;
	int effects;
	float scale;

	byte eflags;

	int rendermode;
	int renderamt;
	color24	rendercolor;
	int renderfx;

	int movetype;
	float animtime;
	float framerate;
	int body;
	byte controller[4];
	byte blending[4];
	vec3_t velocity;

	vec3_t mins;
	vec3_t maxs;

	int aiment;
	int owner; 

	float friction;
	float gravity;

	int team;
	int playerclass;
	int health;
	qboolean spectator;
	int weaponmodel;
	int gaitsequence;
	vec3_t basevelocity;
	int usehull;
	int oldbuttons;
	int onground;
	int iStepLeft;
	float flFallVelocity;

	float fov;
	int weaponanim;

	vec3_t startpos;
	vec3_t endpos;
	float impacttime;
	float starttime;

	int iuser1;
	int iuser2;
	int iuser3;
	int iuser4;
	float fuser1;
	float fuser2;
	float fuser3;
	float fuser4;
	vec3_t vuser1;
	vec3_t vuser2;
	vec3_t vuser3;
	vec3_t vuser4;
};

typedef struct
{
	byte mouthopen;
	byte sndcount;
	int sndavg;
}
mouth_t;

typedef struct
{
	float prevanimtime;
	float sequencetime;
	byte prevseqblending[2];
	vec3_t prevorigin;
	vec3_t prevangles;

	int prevsequence;
	float prevframe;

	byte prevcontroller[4];
	byte prevblending[2];
}
latchedvars_t;

typedef struct
{
	float animtime;

	vec3_t origin;
	vec3_t angles;
}
position_history_t;

typedef struct cl_entity_s cl_entity_t;

#define HISTORY_MAX 64

struct cl_entity_s
{
	int index;

	qboolean player;

	entity_state_t baseline;
	entity_state_t prevstate;
	entity_state_t curstate;

	int current_position;
	position_history_t ph[HISTORY_MAX];

	mouth_t mouth;

	latchedvars_t latched;

	float lastmove;

	vec3_t origin;
	vec3_t angles;

	vec3_t attachment[4];

	int trivial_accept;

	struct model_s *model;
	struct efrag_s *efrag;
	struct mnode_s *topnode;

	float syncbase;
	int visframe;
	colorVec cvFloorColor;
};

typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_357,
	BULLET_PLAYER_BUCKSHOT,
	BULLET_PLAYER_CROWBAR,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	BULLET_PLAYER_45ACP,
	BULLET_PLAYER_338MAG,
	BULLET_PLAYER_762MM,
	BULLET_PLAYER_556MM,
	BULLET_PLAYER_50AE,
	BULLET_PLAYER_57MM,
	BULLET_PLAYER_357SIG
}
Bullet;

cl_entity_t *GetViewEntity(void);
void VectorCopy(float *in, float *out);
void VectorScale(float *in, float scale, float *out);
void AngleVectors(const float *vecAngles, float *forward, float *right, float *up);
int FindModelIndex(const char *pmodel);
bool IsLocal(int idx);
bool UseLeftHand(void);
void ShotsFired(void);
void MuzzleFlash(void);
void WeaponAnimation(int sequence);
void CreateSmoke(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate);
void GetDefaultShellInfo(event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection);
void EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity);
void PlaySound(int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch);
void GetGunPosition(event_args_t *args, float *pos, float *origin);
void FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iPenetration);
long RandomLong(long lLow, long lHigh);
float RandomFloat(float flLow, float flHigh);
void ConsolePrintf(char *fmt, ...);
void ConsoleDPrintf(char *fmt, ...);