#pragma once

#include <vector>
#include <algorithm>
#include <sselib.h>

//add new resource here
extern tgasprite_t g_texTracer[3];
extern tgasprite_t g_texFlamethrowerFire[5];
extern tgasprite_t g_texRocketSmoke[5];
extern tgasprite_t g_texMediBeam;
extern tgasprite_t g_texBrightGlowY;
extern tgasprite_t g_texSoftGlow;
extern tgasprite_t g_texSoftGlowTrans;
extern tgasprite_t g_texMultiJumpSmoke[5];
extern tgasprite_t g_texCircle1;
extern tgasprite_t g_texCircle2;
extern tgasprite_t g_texSingleFlame;
extern tgasprite_t g_texDebris[6];
extern tgasprite_t g_texStarFlash;
extern tgasprite_t g_texCritHit;
extern tgasprite_t g_texMiniCritHit;

typedef vec_t matrix3x4[3][4];

//add new particle system here
enum PartSysEnum
{
	PS_RocketTrail,
	PS_RocketBurst,
	PS_RocketFire,
	PS_RocketCritGlow,
	PS_RocketCritSpark,
	PS_StickyTrail,
	PS_GrenadeTrail,
	PS_GrenadeTimer,
	PS_GrenadeCritTrail,
	PS_GrenadeCritGlow,
	PS_GrenadeCritSpark,
	PS_StickyTimer,
	PS_MultiJumpTrail,
	PS_MultiJumpSmoke,
	PS_MultiJumpSpark,
	PS_DisguiseSmoke,
	PS_DisguiseFlash,
	PS_MediBeam,
	PS_MediPluse,
	PS_MediMuzzle,
	PS_FlameThrow,
	PS_FlameThrowPilot,
	PS_FlameThrowFire,
	PS_FlameThrowCritGlow,
	PS_FlameThrowCritSpark,
	PS_ExplosionWall,
	PS_ExplosionMidAir,
	PS_ExplosionDebris,
	PS_ExplosionDustup,
	PS_ExplosionDustup2,
	PS_ExplosionCoreFlash,
	PS_ExplosionSmoke,
	PS_ExplosionFloatieEmbers,
	PS_ExplosionFlyingEmbers,
	PS_ExplosionFlash,
	PS_DefaultTracer,
	PS_MinigunMuzzle,
	PS_ShotgunMuzzle,
	PS_PistolMuzzle,
	PS_MuzzleFlash,
	PS_MuzzleSpark,
	PS_MuzzleSmoke,
	PS_CritText,
	PS_MiniCritText,
	PS_HitDamageText,
	PS_BurningPlayer,
	PS_BurningPlayerGlow,
	PS_BulletImpact,
	PS_BulletImpactDebris,
	PS_BulletImpactSprak
};

typedef struct
{
	vec3_t vel;
	vec3_t org;
	float die;
	float life;
	byte col[4];
	//for custom
	float rot;
	float scale;
	int modn;
	int bone;
	qboolean free;
}part_t;

class CParticleSystem
{
public:
	CParticleSystem(){}
	~CParticleSystem()
	{
		int i, size;
		size = m_child.size();
		for(i = 0; i < size; ++i)
		{
			delete m_child[i];
		}
	}
	part_t *AllocParticle(void);
	void UpdateParticles(void);
	void AddChild(CParticleSystem *t){ m_child.push_back(t); t->SetParent(this); }
public:
	void SetParent(CParticleSystem *t){ m_parent = t; }
	void SetDie(float die){ m_die = g_flClientTime + die; }
	void SetDead(int dead)
	{
		m_dead = dead;
		int size = m_child.size();
		for(int i = 0; i < size; ++i)
		{
			m_child[i]->SetDead(dead);
		}
	}
	int GetDead(void){return m_dead;}
	int GetCull(void ){ return m_cull;}
	int GetSort(void ){ return m_sort;}
	int GetType(void){return m_type;}
	CParticleSystem *GetChild(int i){return m_child[i];}
	void Init(int type, int numpart, int numchild);

public:
	virtual void Movement(part_t *p, float *org){};
	virtual void Render(part_t *p, float *org){};
	virtual void Update(void){};
	virtual qboolean IsBindBone(void){ return false; }
	virtual qboolean IsBindAttachment(void){ return false; }
	virtual qboolean IsFollowEntity(void){ return false; }

public:
	std::vector<part_t> m_part;
	float	m_die;
	int		m_dead;
	int		m_type;
	int		m_cull;
	int		m_sort;
	std::vector<CParticleSystem *>m_child;
	CParticleSystem *m_parent;
};

class CPartSystemEntity : public CParticleSystem
{
public:
	CPartSystemEntity(){}
	void Init(int type, int parts, int childs, cl_entity_t *entity)
	{
		CParticleSystem::Init(type, parts, childs);
		m_entity = entity;
	}
	virtual qboolean IsFollowEntity(void){ return true; }
	cl_entity_t *GetFollowEntity(void){ return m_entity; }
public:
	cl_entity_t *m_entity;
};

class CPartSystemBones : public CPartSystemEntity
{
public:
	CPartSystemBones(){}
	void Init(int type, int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(type, parts, childs, entity);
		memset(m_lighttransform, 0, sizeof(m_lighttransform));
		m_bonesaved = false;
	}
	virtual qboolean IsBindBone(void){ return true; }

public:
	matrix3x4 m_lighttransform[MAXSTUDIOBONES];
	qboolean m_bonesaved;
};

class CPartSystemAttachment : public CPartSystemEntity
{
public:
	CPartSystemAttachment(){}
	void Init(int type, int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(type, parts, childs, entity);
		memset(m_attachment, 0, sizeof(m_attachment));
		m_attachmentsaved = false;
	}
	virtual qboolean IsBindAttachment(void){ return true; }

public:
	vec3_t m_attachment[4];
	qboolean m_attachmentsaved;
};

class CPartSystemCoord : public CParticleSystem
{
public:
	CPartSystemCoord(){}
	void Init(int type, int parts, int childs, float *org, float *vel)
	{
		CParticleSystem::Init(type, parts, childs);
		if(org) VectorCopy(org, m_org);
		if(vel) VectorCopy(vel, m_vel);
	}
protected:
	vec3_t m_org;
	vec3_t m_vel;
};

typedef struct
{
	part_t *part;
	CParticleSystem *partsys;
	vec3_t org;
	float dist;
	int discard;
}vispart_t;

//export

void R_DrawTGASprite(cl_entity_t *e, tgasprite_t *tgaspr);
void R_DrawTGATracer(cl_entity_t *e, tgasprite_t *tgaspr);

void R_KillPartSystem(cl_entity_t *pEntity, int killme);
CParticleSystem *R_FindPartSystem(PartSysEnum type, cl_entity_t *entity);
void R_AddPartSystem(CParticleSystem *t);

extern std::vector<CParticleSystem *> g_partsystems;

//marco

//renderer

#define CALC_FRACTION(p) float frac = (p->die - g_flClientTime)/p->life;\
	frac = min(max(frac, 0.0), 1.0);\
	float frac2 = 1-frac;

#define COLOR_FADE(nr, ng, nb) 	ent.curstate.rendercolor.r = p->col[0] * frac + nr * frac2;\
	ent.curstate.rendercolor.g = p->col[1] * frac + ng * frac2;\
	ent.curstate.rendercolor.b = p->col[2] * frac + nb * frac2;

//color £¯----
#define COLOR_FADE_IN(t, nr, ng, nb) if(frac2 > t){\
	ent.curstate.rendercolor.r = nr;\
	ent.curstate.rendercolor.g = ng;\
	ent.curstate.rendercolor.b = nb;\
	}else{\
	float flColorLerp = frac2 / t;\
	ent.curstate.rendercolor.r = p->col[0] * (1-flColorLerp) + nr * flColorLerp;\
	ent.curstate.rendercolor.g = p->col[1] * (1-flColorLerp) + ng * flColorLerp;\
	ent.curstate.rendercolor.b = p->col[2] * (1-flColorLerp) + nb * flColorLerp;\
}

//color ___£¯----
#define COLOR_FADE_IN2(t, t2, nr, ng, nb) if(frac2 < t){\
	ent.curstate.rendercolor.r = p->col[0];\
	ent.curstate.rendercolor.g = p->col[1];\
	ent.curstate.rendercolor.b = p->col[1];\
	}else if(frac2 < t2){\
	float flColorLerp = (frac2-t) / (t2-t);\
	ent.curstate.rendercolor.r = p->col[0] * (1-flColorLerp) + nr * flColorLerp;\
	ent.curstate.rendercolor.g = p->col[1] * (1-flColorLerp) + ng * flColorLerp;\
	ent.curstate.rendercolor.b = p->col[2] * (1-flColorLerp) + nb * flColorLerp;\
	}else{\
	ent.curstate.rendercolor.r = nr;\
	ent.curstate.rendercolor.g = ng;\
	ent.curstate.rendercolor.b = nb;\
}

//alpha £¯£Ü
#define ALPHA_FADE_INOUT(time) if(frac > (1-time))\
		ent.curstate.renderamt = p->col[3] * frac2 / time;\
	else\
		ent.curstate.renderamt = p->col[3] * frac / (1-time);

//alpha £¯£þ£Ü
#define ALPHA_FADE_INOUT2(time, time2) if(frac > (1-time))\
		ent.curstate.renderamt = p->col[3] * frac2 / time;\
	else if(frac > (1-time2))\
		ent.curstate.renderamt = p->col[3];\
	else\
		ent.curstate.renderamt = p->col[3] * frac / (1-time2);

//alpha £þ£Ü
#define ALPHA_FADE_OUT(time2) if(frac > (1-time2))\
		ent.curstate.renderamt = p->col[3];\
	else\
		ent.curstate.renderamt = p->col[3] * frac / (1-time2);

//initalizer
#define CALC_ADDCOUNT(n) int count = (int)((double)n * g_flFrameTime / (1 / 60.0));if(count < 1) count = 1;

#define COLOR_RANDOM_LERP(r1, g1, b1, r2, g2, b2) 	{float flColorLerp = RANDOM_FLOAT(0, 1);p->col[0] = r1 * flColorLerp + r2 * (1-flColorLerp);\
	p->col[1] = g1 * flColorLerp + g2 * (1-flColorLerp);\
	p->col[2] = b1 * flColorLerp + b2 * (1-flColorLerp);}

#define SPEED_RANDOM_LERP(r1, g1, b1, r2, g2, b2) 	{float flSpeedLerp = RANDOM_FLOAT(0, 1);p->vel[0] = r1 * flSpeedLerp + r2 * (1-flSpeedLerp);\
	p->vel[1] = g1 * flSpeedLerp + g2 * (1-flSpeedLerp);\
	p->vel[2] = b1 * flSpeedLerp + b2 * (1-flSpeedLerp);}

//movement
inline void R_ParticleMovementSimple(part_t *p, float *org)
{
	VectorMA(p->org, g_flFrameTime, p->vel, org);
	VectorCopy(org, p->org);
}

inline void R_ParticleMovementNone(part_t *p, float *org)
{
	VectorCopy(p->org, org);
}

inline void R_ParticleMovementEntity(part_t *p, float *org, cl_entity_t *entity)
{
	VectorMA(p->org, g_flFrameTime, p->vel, p->org);
	VectorAdd(p->org, entity->origin, org);
}

extern vec3_t g_vecZero;
extern vec3_t g_TracerEndPos;