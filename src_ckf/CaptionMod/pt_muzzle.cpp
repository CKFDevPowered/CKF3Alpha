#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"
#include "qgl.h"

extern cl_entity_t ent;

void PerpendicularVector(vec3_t dst, vec3_t src);

class CPSMuzzle : public CPartSystemAttachment
{
public:
	CPSMuzzle(){}
	void Init(int type, int parts, int childs, cl_entity_t *entity, int attachindex, int size)
	{
		CPartSystemAttachment::Init(type, parts, childs, entity);
		m_firstview = (entity == cl_viewent) ? true : false;
		m_attachindex = attachindex;
		m_size = size;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Update(void)
	{
		vec3_t ang;
		VectorCopy(m_entity->angles, ang);
		ang[0] *= -1;
		gEngfuncs.pfnAngleVectors(ang, m_vel, m_right, m_up);
	}
public:
	vec3_t m_vel, m_right, m_up;
	int m_attachindex;
	int m_firstview;
	int m_size;
};

class CPSMuzzleFlash : public CPartSystemEntity
{
public:
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MuzzleFlash, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		CPSMuzzle *pParent = (CPSMuzzle *)m_parent;

		//must calc org here or attachment will be wrong
		VectorMA(pParent->m_attachment[pParent->m_attachindex], frac2 * p->vel[0] - frac2 * frac2 * p->vel[1], pParent->m_vel, org);

		ent.curstate.rendermode = kRenderAddColor;
		COLOR_FADE(255, 131, 11);
		ALPHA_FADE_INOUT2(0.25, 0.50);

		ent.curstate.scale = (0.1*frac + 0.01) * p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot + 60 * frac2;

		if(pParent->m_firstview)
			qglDepthRange(0, 0.3);
		
		R_DrawTGASprite(&ent, &g_texStarFlash);

		if(pParent->m_firstview)
			qglDepthRange(0, 1);
	}
	virtual void Update(void)
	{
		if(!m_dead)
		{
			CALC_ADDCOUNT(3);
			for(int i = 0; i < count; ++i)
				AddParticle(i, count);
		}
	}
	void AddParticle(int i, int count)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		CPSMuzzle *pParent = (CPSMuzzle *)m_parent;

		if(pParent->m_size == 2)
		{
			p->vel[0] = 48;//v
			p->vel[1] = 16;//0.5a
		}
		else if(pParent->m_size == 1)
		{
			p->vel[0] = 36;//v
			p->vel[1] = 12;//0.5a
		}
		else
		{
			p->vel[0] = 24;//v
			p->vel[1] = 6;//0.5a
		}
		VectorCopy(pParent->m_attachment[pParent->m_attachindex], p->org);

		COLOR_RANDOM_LERP(255, 223, 94, 218, 158, 77);
		p->col[3] = 140;
		p->rot = RANDOM_FLOAT(-60, 60);
		p->life = RANDOM_FLOAT(0.1, 0.15);
		p->die = g_flClientTime + p->life + (double)i / (double)count * g_flFrameTime;
		if(pParent->m_size == 2)
			p->scale = 2.0;
		else if(pParent->m_size == 1)
			p->scale = 1.0;
		else
			p->scale = 0.5;
	}
};

class CPSMuzzleSpark : public CPartSystemEntity
{
public:
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MuzzleSpark, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		CPSMuzzle *pParent = (CPSMuzzle *)m_parent;

		vec3_t vel;
		//must calc org here or attachment will be wrong
		VectorClear(vel);
		VectorMA(vel, frac2 * p->vel[0], pParent->m_vel, vel);
		VectorMA(vel, frac2 * p->vel[1], pParent->m_right, vel);
		VectorMA(vel, frac2 * p->vel[2], pParent->m_up, vel);
		VectorAdd(vel, pParent->m_attachment[pParent->m_attachindex], org);

		ent.curstate.rendermode = kRenderTransAdd;	
		COLOR_FADE_IN(0.5, 255, 121, 1);
		ALPHA_FADE_OUT(0.1);

		ent.curstate.scale = 0.003;
		ent.curstate.fuser1 = p->scale;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(vel, ent.curstate.velocity);

		if(pParent->m_firstview)
			qglDepthRange(0, 0.3);

		R_DrawTGATracer(&ent, &g_texBrightGlowY);
		
		if(pParent->m_firstview)
			qglDepthRange(0, 1);
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->vel[0] = RANDOM_FLOAT(24, 48);
		p->vel[1] = RANDOM_FLOAT(-4, 4);
		p->vel[2] = RANDOM_FLOAT(0, 4);

		CPSMuzzle *pParent = (CPSMuzzle *)m_parent;
		VectorCopy(pParent->m_attachment[pParent->m_attachindex], p->org);

		p->scale = RANDOM_FLOAT(8, 16);

		p->col[0] = 255;
		p->col[1] = 255;
		p->col[2] = 255;
		p->col[3] = 255;
		p->life = RANDOM_FLOAT(0.08, 0.15);
		p->die = g_flClientTime + p->life;
	}
};

class CPSMuzzleSmoke : public CPartSystemEntity
{
public:
	CPSMuzzleSmoke(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MuzzleSmoke, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		p->vel[2] += 30 * g_flFrameTime;

		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorCopy(p->org, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		COLOR_FADE(80, 80, 80);
		ALPHA_FADE_OUT(0.3);

		ent.curstate.scale = (0.3 + 0.2 * frac2) * p->scale * 0.1;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot + 24 * frac2;

		R_DrawTGASprite(&ent, &g_texSoftGlowTrans);
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		CPSMuzzle *pParent = (CPSMuzzle *)m_parent;
		VectorCopy(pParent->m_attachment[pParent->m_attachindex], p->org);

		p->vel[0] = RANDOM_FLOAT(-10, 10);
		p->vel[1] = RANDOM_FLOAT(-10, 10);
		p->vel[2] = RANDOM_FLOAT(10, 20);

		COLOR_RANDOM_LERP(138, 130, 119, 255, 255, 255);
		p->col[3] = RANDOM_LONG(32, 64);
		p->scale = RANDOM_FLOAT(0.7, 1.6);
		p->rot = RANDOM_FLOAT(-25, 25);
		p->life = RANDOM_FLOAT(0.6, 1.3);
		p->die = g_flClientTime + p->life;
	}
};

void R_MinigunMuzzle(cl_entity_t *pEntity, int attachment)
{
	int i, iSize;

	iSize = 2;//size 2 = minigun

	CPSMuzzle *pFind = (CPSMuzzle *)R_FindPartSystem(PS_MinigunMuzzle, pEntity);
	if(pFind)
	{
		pFind->SetDie(1.0);
		pFind->GetChild(0)->SetDie(0.1);
		return;
	}

	CPSMuzzle *pCore = new CPSMuzzle;
	pCore->Init(PS_MinigunMuzzle, 0, 4, pEntity, attachment, iSize);
	pCore->SetDie(1.0);

	CPSMuzzleFlash *pFlash = new CPSMuzzleFlash;
	pFlash->Init(10, 0, pEntity);
	pFlash->SetDie(0.1);

	CPSMuzzleSpark *pSpark = new CPSMuzzleSpark;
	pSpark->Init(6, 0, pEntity);

	CPSMuzzleSmoke *pSmoke = new CPSMuzzleSmoke;
	pSmoke->Init(5, 0, pEntity);

	pCore->AddChild(pFlash);
	pCore->AddChild(pSpark);
	pCore->AddChild(pSmoke);

	//add particles at the end, or will crash
	for(i = 0; i < 6; ++i)
		pSpark->AddParticle();
	for(i = 0; i < 5; ++i)
		pSmoke->AddParticle();

	R_AddPartSystem(pCore);
}

void R_ShotgunMuzzle(cl_entity_t *pEntity, int attachment)
{
	int i, iSize;

	iSize = 1;//size 1 = shotgun

	CPSMuzzle *pCore = new CPSMuzzle;
	pCore->Init(PS_ShotgunMuzzle, 0, 4, pEntity, attachment, iSize);
	pCore->SetDie(1.5);

	CPSMuzzleFlash *pFlash = new CPSMuzzleFlash;
	pFlash->Init(10, 0, pEntity);
	pFlash->SetDie(0.1);

	CPSMuzzleSpark *pSpark = new CPSMuzzleSpark;
	pSpark->Init(6, 0, pEntity);

	CPSMuzzleSmoke *pSmoke = new CPSMuzzleSmoke;
	pSmoke->Init(5, 0, pEntity);

	pCore->AddChild(pFlash);
	pCore->AddChild(pSpark);
	pCore->AddChild(pSmoke);

	//add particles at the end, or will crash
	for(i = 0; i < 6; ++i)
		pSpark->AddParticle();
	for(i = 0; i < 5; ++i)
		pSmoke->AddParticle();

	R_AddPartSystem(pCore);
}

void R_PistolMuzzle(cl_entity_t *pEntity, int attachment)
{
	int iSize;

	iSize = 0;//size 0 = pistol

	CPSMuzzle *pCore = new CPSMuzzle;
	pCore->Init(PS_PistolMuzzle, 0, 4, pEntity, attachment, iSize);
	pCore->SetDie(1.5);

	CPSMuzzleFlash *pFlash = new CPSMuzzleFlash;
	pFlash->Init(10, 0, pEntity);
	pFlash->SetDie(0.1);

	pCore->AddChild(pFlash);

	R_AddPartSystem(pCore);
}