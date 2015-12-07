#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"

extern cl_entity_t ent;

class CPSGrenadeTrail : public CPartSystemEntity
{
public:
	CPSGrenadeTrail(){};
	void Init(int type, int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(type, parts, childs, entity);
		VectorCopy(entity->origin, m_lastorg);
		m_lasttime = g_flClientTime;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);
		float fracCol2 = frac2 * frac2;
		float fracCol = 1-fracCol2;

		ent.curstate.rendermode = kRenderGlow;
		ent.curstate.rendercolor.r = (p->col[0] * fracCol + 200 * fracCol2);
		ent.curstate.rendercolor.g = (p->col[1] * fracCol + 30 * fracCol2);
		ent.curstate.rendercolor.b = (p->col[2] * fracCol + 30 * fracCol2);
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = 0.05;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_dead && m_entity->curstate.frame < 1)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		int i, div;
		float fdiv;
		vec3_t sub;
		part_t *p;

		VectorSubtract(m_lastorg, m_entity->origin, sub);
		div = (int)(VectorLength(sub) / 2.0f);
		for(i = 0; i < div; ++i)
		{
			p = AllocParticle();
			if(!p) break;

			fdiv = (float)i / div;
			VectorMA(m_entity->origin, fdiv, sub, p->org);
			if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
			{
				p->col[0] = 255;
				p->col[1] = 30;
				p->col[2] = 80;
			}
			else
			{
				p->col[0] = 30;
				p->col[1] = 80;
				p->col[2] = 255;
			}
			p->col[3] = 64;
			p->life = 0.24;
			p->die = g_flClientTime + (m_lasttime - g_flClientTime) * fdiv + p->life;
		}
		if(div > 0)
		{
			VectorCopy(m_entity->origin, m_lastorg);
			m_lasttime = g_flClientTime;
		}
	}
protected:
	vec3_t m_lastorg;
	float m_lasttime;
};

class CPSGrenadeTimer : public CPartSystemEntity
{
public:
	CPSGrenadeTimer(){};
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_GrenadeTimer, parts, childs, entity);
		Reset();
	}
	void Reset(void)
	{
		m_nextemit = g_flClientTime + 0.2;
		m_emitcount = 0;
		SetDie(2.2);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementEntity(p, org, m_entity);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderGlow;
		ent.curstate.renderamt = p->col[3];

		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];

		ent.curstate.scale = 0.03 + 0.05*frac2;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texCircle1);
	}
	virtual void Update(void)
	{
		if(!m_dead && g_flClientTime > m_nextemit)
		{
			AddParticle();
			m_emitcount ++;
			m_nextemit = g_flClientTime + max(0.6 / m_emitcount, 0.06);;		
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
		{
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(36, 72);
			p->col[2] = 36;
		}
		else
		{
			p->col[0] = 36;
			p->col[1] = RANDOM_LONG(36, 72);
			p->col[2] = 255;
		}
		p->col[3] = 200;
		p->life = 0.2;
		p->die = g_flClientTime + p->life;
	}
protected:
	int m_emitcount;
	float m_nextemit;
};

class CPSStickyTimer : public CPartSystemEntity
{
public:
	CPSStickyTimer(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_StickyTimer, parts, childs, entity);
		Reset();
	}
	void Reset(void)
	{
		m_nextemit = g_flClientTime + 0.9;
		m_emitcount = 0;
		SetDie(1.3);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementEntity(p, org, m_entity);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderGlow;
		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = 0.03 + 0.05*frac2;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texCircle2);
	}
	virtual void Update(void)
	{
		if(!m_dead && g_flClientTime > m_nextemit)
		{
			AddParticle();
			m_emitcount ++;
			m_nextemit = g_flClientTime + 0.09;	
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
		{
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(36, 72);
			p->col[2] = 36;
		}
		else
		{
			p->col[0] = 36;
			p->col[1] = RANDOM_LONG(36, 72);
			p->col[2] = 255;
		}
		p->col[3] = 255;
		p->life = 0.2;
		p->die = g_flClientTime + p->life;
	}
protected:
	int m_emitcount;
	float m_nextemit;
};

class CPSGrenadeCritTrail : public CPartSystemEntity
{
public:
	CPSGrenadeCritTrail(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_GrenadeCritTrail, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		COLOR_FADE(205, 87, 0);
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = 0.15*frac*p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_dead && m_entity->curstate.frame > 0 && m_entity->curstate.movetype != MOVETYPE_NONE)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->org[0] = RANDOM_FLOAT(-1, 1);
		p->org[1] = RANDOM_FLOAT(-1, 1);
		p->org[2] = RANDOM_FLOAT(-1, 1);
		p->vel[0] = p->org[0] * 2;
		p->vel[1] = p->org[1] * 2;
		p->vel[2] = p->org[2] * 2;

		if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
		{
			p->col[0] = RANDOM_LONG(185, 255);;
			p->col[1] = RANDOM_LONG(30, 77);
			p->col[2] = RANDOM_LONG(0, 77);
		}
		else
		{
			p->col[0] = RANDOM_LONG(0, 18);
			p->col[1] = RANDOM_LONG(0, 96);
			p->col[2] = 255;
		}
		p->col[3] = RANDOM_LONG(128, 255);
		p->scale = RANDOM_FLOAT(1, 2);

		VectorAdd(m_entity->origin, p->org, p->org);
		p->life = 0.4;
		p->die = g_flClientTime + p->life;
	}
protected:

};

class CPSGrenadeCritGlow : public CPartSystemEntity
{
public:
	CPSGrenadeCritGlow(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_GrenadeCritGlow, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementEntity(p, org, m_entity);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		if(p->col[2] == 0)
		{
			COLOR_FADE(199, 0, 0);
		}
		else
		{
			COLOR_FADE(0, 48, 255);
		}
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = 0.2*frac;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_dead && m_entity->curstate.frame > 0 && m_entity->curstate.movetype != MOVETYPE_NONE)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
		{
			p->col[0] = RANDOM_LONG(180, 255);;
			p->col[1] = RANDOM_LONG(25, 96);
			p->col[2] = 0;
		}
		else
		{
			p->col[0] = 115;
			p->col[1] = 158;
			p->col[2] = 255;
		}
		p->col[3] = 255;

		p->life = RANDOM_FLOAT(0.3, 0.4);
		p->die = g_flClientTime + p->life;
	}
protected:

};

class CPSGrenadeCritSpark : public CPartSystemEntity
{
public:
	CPSGrenadeCritSpark(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_GrenadeCritSpark, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		if(p->col[2] != 255)
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 255 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 0 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 0 * frac2;
		}
		else
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 0 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 66 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 255 * frac2;
		}
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = 0.03+0.03*frac;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_dead && m_entity->curstate.frame > 0)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->vel[0] = RANDOM_FLOAT(-12, 12);
		p->vel[1] = RANDOM_FLOAT(-12, 12);
		p->vel[2] = RANDOM_FLOAT(-12, 12);
		p->org[0] = RANDOM_FLOAT(-2, 2);
		p->org[1] = RANDOM_FLOAT(-2, 2);
		p->org[2] = RANDOM_FLOAT(-2, 2);

		if(m_entity->curstate.skin == 0 || m_entity->curstate.skin == 2)
		{
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(0, 180);
			p->col[2] = 0;
		}
		else
		{
			p->col[0] = 0;
			p->col[1] = RANDOM_LONG(72, 126);
			p->col[2] = 255;
		}
		p->col[3] = 255;

		VectorAdd(m_entity->origin, p->org, p->org);
		p->life = RANDOM_FLOAT(0.2, 0.6);
		p->die = g_flClientTime + p->life;
	}
protected:

};

void R_GrenadeTrail(cl_entity_t *pEntity)
{
	CParticleSystem *pOldTrail = R_FindPartSystem(PS_GrenadeTrail, pEntity);
	if(pOldTrail && !pOldTrail->GetDead())
	{
		CPSGrenadeTimer *pOldTimer = (CPSGrenadeTimer *)pOldTrail->GetChild(0);
		if(pOldTimer)
		{
			pOldTimer->SetDead(0);
			pOldTimer->Reset();
		}
		return;
	}
	CPSGrenadeTrail *pTrail = new CPSGrenadeTrail;
	pTrail->Init(PS_GrenadeTrail, 100, 4, pEntity);

	CPSGrenadeTimer *pTimer = new CPSGrenadeTimer;
	pTimer->Init(20, 0, pEntity);

	CPSGrenadeCritTrail *pCritTrail = new CPSGrenadeCritTrail;
	pCritTrail->Init(100, 0, pEntity);

	CPSGrenadeCritGlow *pCritGlow = new CPSGrenadeCritGlow;
	pCritGlow->Init(20, 0, pEntity);

	CPSGrenadeCritSpark *pCritSpark = new CPSGrenadeCritSpark;
	pCritSpark->Init(20, 0, pEntity);

	pTrail->AddChild(pTimer);
	pTrail->AddChild(pCritTrail);
	pTrail->AddChild(pCritGlow);
	pTrail->AddChild(pCritSpark);
	R_AddPartSystem(pTrail);
}

void R_StickyTrail(cl_entity_t *pEntity)
{
	CParticleSystem *pOldTrail = R_FindPartSystem(PS_StickyTrail, pEntity);
	if(pOldTrail && !pOldTrail->GetDead())
	{
		CPSStickyTimer *pOldTimer = (CPSStickyTimer *)pOldTrail->GetChild(0);
		if(pOldTimer)
		{
			pOldTimer->SetDead(0);
			pOldTimer->Reset();
		}
		return;
	}
	CPSGrenadeTrail *pTrail = new CPSGrenadeTrail;
	pTrail->Init(PS_StickyTrail, 100, 4, pEntity);

	CPSStickyTimer *pTimer = new CPSStickyTimer;
	pTimer->Init(20, 0, pEntity);

	CPSGrenadeCritTrail *pCritTrail = new CPSGrenadeCritTrail;
	pCritTrail->Init(100, 0, pEntity);

	CPSGrenadeCritGlow *pCritGlow = new CPSGrenadeCritGlow;
	pCritGlow->Init(20, 0, pEntity);

	CPSGrenadeCritSpark *pCritSpark = new CPSGrenadeCritSpark;
	pCritSpark->Init(20, 0, pEntity);

	pTrail->AddChild(pTimer);
	pTrail->AddChild(pCritTrail);
	pTrail->AddChild(pCritGlow);
	pTrail->AddChild(pCritSpark);
	R_AddPartSystem(pTrail);
}

class CPSStickyKill : public CParticleSystem
{
public:
	CPSStickyKill(){}
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_StickyKill, parts, childs);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		if(p->col[2] != 255)
		{
			COLOR_FADE(71, 61, 46);
		}
		else
		{
			COLOR_FADE(65, 75, 93);
		}
		ALPHA_FADE_OUT(0.1);

		ent.curstate.scale = 0.1;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
	}
	void AddParticle(vec3_t origin, int skin)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(skin == 0 || skin == 2)
		{
			p->col[0] = 255;
			p->col[1] = 84;
			p->col[2] = 0;
		}
		else
		{
			p->col[0] = 0;
			p->col[1] = 150;
			p->col[2] = 255;
		}
		p->col[3] = 255;

		VectorCopy(origin, p->org);
		p->life = 3.0f;
		p->die = g_flClientTime + p->life;
	}
protected:

};

void R_StickyKill(vec3_t origin, int skin)
{
	CPSStickyKill *pKill = new CPSStickyKill;
	pKill->Init(1, 0);
	pKill->AddParticle(origin, skin);

	R_AddPartSystem(pKill);
}