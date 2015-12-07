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
qboolean EV_IsLocal( int idx );
void EV_GetGunPosition(int idx, int attachment, float *origin);;

class CMuzzleCore : public CParticleSystem
{
public:
	CMuzzleCore(){}
	void Init(int type, int parts, int childs, int entindex, int attachment, vec3_t angles, int size)
	{
		CParticleSystem::Init(type, parts, childs);
		m_entindex = entindex ;
		m_attachment = attachment;
		VectorCopy(angles, m_angles);
		m_size = size;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Update(void)
	{
		if(EV_IsLocal(m_entindex) && !CL_IsThirdPerson())
		{
			VectorCopy(refparams.forward, m_fwd);
			VectorCopy(refparams.right, m_right);
			VectorCopy(refparams.up, m_up);
			VectorCopy(cl_viewent->attachment[0], m_pos);
			m_firstview = true;
		}
		else
		{
			gEngfuncs.pfnAngleVectors(m_angles, m_fwd, m_right, m_up);

			EV_GetGunPosition(m_entindex, m_attachment, m_pos);
			m_firstview = false;
		}
	}
public:
	vec3_t m_angles;
	vec3_t m_fwd, m_right, m_up, m_pos;
	int m_entindex;
	int m_attachment;
	qboolean m_firstview;
	int m_size;
};

class CMuzzleFlash : public CParticleSystem
{
public:
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_MuzzleFlash, parts, childs);
		SetCull(false);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		CMuzzleCore *pParent = (CMuzzleCore *)m_parent;

		//must calc org here or attachment will be wrong
		VectorMA(pParent->m_pos, frac2 * p->vel[0] - frac2 * frac2 * p->vel[1], pParent->m_fwd, org);

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

		CMuzzleCore *pParent = (CMuzzleCore *)m_parent;

		if(pParent->m_size == 2)
		{
			p->vel[0] = 24;//v
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
		VectorCopy(pParent->m_pos, p->org);

		COLOR_RANDOM_LERP(255, 223, 94, 218, 158, 77);
		p->col[3] = 140;
		p->rot = RANDOM_FLOAT(-60, 60);
		p->life = RANDOM_FLOAT(0.1, 0.15);
		p->die = g_flClientTime + p->life + (double)i / (double)count * g_flFrameTime;
		if(pParent->m_size == 2)
			p->scale = 3.0;
		else if(pParent->m_size == 1)
			p->scale = 1.0;
		else
			p->scale = 0.5;
	}
};

class CMuzzleSpark : public CParticleSystem
{
public:
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_MuzzleSpark, parts, childs);
		SetCull(false);

		m_emitted = false;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		CMuzzleCore *parent = (CMuzzleCore *)m_parent;

		vec3_t vel;
		//must calc org here or attachment will be wrong
		VectorClear(vel);
		VectorMA(vel, frac2 * p->vel[0], parent->m_fwd, vel);
		VectorMA(vel, frac2 * p->vel[1], parent->m_right, vel);
		VectorMA(vel, frac2 * p->vel[2], parent->m_up, vel);
		VectorAdd(vel, parent->m_pos, org);

		ent.curstate.rendermode = kRenderTransAdd;	
		COLOR_FADE_IN(0.5, 255, 121, 1);
		ALPHA_FADE_OUT(0.1);

		ent.curstate.scale = 0.003;
		ent.curstate.fuser1 = p->scale;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(vel, ent.curstate.velocity);

		if(parent->m_firstview)
			qglDepthRange(0, 0.3);

		R_DrawTGATracer(&ent, &g_texBrightGlowY);
		
		if(parent->m_firstview)
			qglDepthRange(0, 1);
	}
	virtual void Update(void)
	{
		if(!m_emitted)
		{
			CMuzzleCore *parent = (CMuzzleCore *)m_parent;
			if(parent->m_size > 0)
			{
				for(int i = 0; i < 6; ++i)
					AddParticle();
			}
			m_emitted = true;
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();

		p->vel[0] = RANDOM_FLOAT(24, 48);
		p->vel[1] = RANDOM_FLOAT(-4, 4);
		p->vel[2] = RANDOM_FLOAT(0, 4);

		CMuzzleCore *parent = (CMuzzleCore *)m_parent;
		VectorCopy(parent->m_pos, p->org);

		p->scale = RANDOM_FLOAT(8, 16);

		p->col[0] = 255;
		p->col[1] = 255;
		p->col[2] = 255;
		p->col[3] = 255;
		p->life = RANDOM_FLOAT(0.08, 0.15);
		p->die = g_flClientTime + p->life;
	}
private:
	int m_emitted;
};

class CMuzzleSmoke : public CParticleSystem
{
public:
	CMuzzleSmoke(){}
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_MuzzleSmoke, parts, childs);

		m_emitted = false;
	}
	virtual void Movement(part_t *p, float *org)
	{
		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorCopy(p->org, org);
	}
	virtual void Update(void)
	{
		if(!m_emitted)
		{
			CMuzzleCore *parent = (CMuzzleCore *)m_parent;
			if(parent->m_size > 0)
			{
				for(int i = 0; i < 5; ++i)
					AddParticle();
			}
			m_emitted = true;
		}
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlphaNoDepth;
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

		CMuzzleCore *parent = (CMuzzleCore *)m_parent;
		VectorCopy(parent->m_pos, p->org);

		p->vel[0] = RANDOM_FLOAT(-10, 10);
		p->vel[1] = RANDOM_FLOAT(-10, 10);
		p->vel[2] = RANDOM_FLOAT(30, 60);

		COLOR_RANDOM_LERP(138, 130, 119, 255, 255, 255);
		p->col[3] = RANDOM_LONG(32, 64);
		p->scale = RANDOM_FLOAT(0.9, 2.1);
		p->rot = RANDOM_FLOAT(-25, 25);
		p->life = RANDOM_FLOAT(0.6, 1.3);
		p->die = g_flClientTime + p->life;
	}
private:
	int m_emitted;
};

void R_MinigunMuzzle(int entindex, int attachment, vec3_t angles)
{
	int i, iSize;

	iSize = 2;

	CMuzzleCore *pCore = new CMuzzleCore;
	pCore->Init(PS_MinigunMuzzle, 0, 3, entindex, attachment, angles, iSize);
	pCore->SetDie(1.0);

	CMuzzleFlash *pFlash = new CMuzzleFlash;
	pFlash->Init(10, 0);
	pFlash->SetDie(0.1);

	//HZ: don't emit particles here since we don't get m_pos now
	CMuzzleSpark *pSpark = new CMuzzleSpark;
	pSpark->Init(6, 0);

	CMuzzleSmoke *pSmoke = new CMuzzleSmoke;
	pSmoke->Init(5, 0);

	//add particles later since we didn't have a parent at this time

	pCore->AddChild(pFlash);
	pCore->AddChild(pSpark);
	pCore->AddChild(pSmoke);

	R_AddPartSystem(pCore);
}

void R_ShotgunMuzzle(int entindex, int attachment, vec3_t angles)
{
	int i, iSize;

	iSize = 1;//size 1 = shotgun

	CMuzzleCore *pCore = new CMuzzleCore;
	pCore->Init(PS_ShotgunMuzzle, 0, 3, entindex, attachment, angles, iSize);
	pCore->SetDie(1.5);

	CMuzzleFlash *pFlash = new CMuzzleFlash;
	pFlash->Init(10, 0);
	pFlash->SetDie(0.1);

	CMuzzleSpark *pSpark = new CMuzzleSpark;
	pSpark->Init(6, 0);

	CMuzzleSmoke *pSmoke = new CMuzzleSmoke;
	pSmoke->Init(5, 0);

	pCore->AddChild(pFlash);
	pCore->AddChild(pSpark);
	pCore->AddChild(pSmoke);

	R_AddPartSystem(pCore);
}

void R_PistolMuzzle(int entindex, int attachment, vec3_t angles)
{
	int iSize;

	iSize = 0;//size 0 = pistol

	CMuzzleCore *pCore = new CMuzzleCore;
	pCore->Init(PS_PistolMuzzle, 0, 1, entindex, attachment, angles, iSize);
	pCore->SetDie(1.5);

	CMuzzleFlash *pFlash = new CMuzzleFlash;
	pFlash->Init(10, 0);
	pFlash->SetDie(0.1);

	pCore->AddChild(pFlash);

	R_AddPartSystem(pCore);
}