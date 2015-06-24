#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"

extern cl_entity_t ent;

class CPSRocketFire : public CPartSystemEntity
{
public:
	CPSRocketFire(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_RocketFire, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;	
		COLOR_FADE(72, 37, 0);
		ALPHA_FADE_INOUT(0.1);

		ent.curstate.scale = 0.24*frac;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texBrightGlowY);
	}
	virtual void Update(void)
	{
		if(!m_dead)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		int i;
		part_t *p;
		vec3_t vecDiff;

		CALC_ADDCOUNT(2);

		VectorSubtract(m_entity->curstate.origin, m_entity->prevstate.origin, vecDiff);

		for(i = 0; i < count; ++i)
		{
			p = AllocParticle();
			if(!p) break;
			p->vel[0] = RANDOM_FLOAT(-3, 3);
			p->vel[1] = RANDOM_FLOAT(-3, 3);
			p->vel[2] = RANDOM_FLOAT(-3, 3);
			VectorMA(m_entity->origin, (float)i / count, vecDiff, p->org);
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(168, 234);
			p->col[2] = 0;
			p->col[3] = 200;
			p->life = 0.2;
			p->die = g_flClientTime + p->life - g_flFrameTime * (float)i / count;
		}
	}
protected:
};

class CPSRocketBurst : public CPartSystemEntity
{
public:
	CPSRocketBurst(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_RocketBurst, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementEntity(p, org, m_entity);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;	
		ent.curstate.rendercolor.r = p->col[0] * frac + 143 * frac2;
		ent.curstate.rendercolor.g = p->col[1] * frac + 57 * frac2;
		ent.curstate.rendercolor.b = p->col[2] * frac + 0 * frac2;
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = p->scale * (1*frac2+0.5) * 0.015;
		ent.curstate.fuser1 = 2+2*frac;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);

		R_DrawTGATracer(&ent, &g_texBrightGlowY);
	}
	virtual void Update(void)
	{
		if(!m_dead)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		vec3_t vpn, vright, vup;
		gEngfuncs.pfnAngleVectors(m_entity->angles, vpn, vright, vup);

		VectorMultiply(vpn, RANDOM_FLOAT(-5, 0), p->vel);
		VectorMA(p->vel, RANDOM_FLOAT(-15, 15), vright, p->vel);
		VectorMA(p->vel, RANDOM_FLOAT(-15, 15), vup, p->vel);
		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(168, 228);
		p->col[2] = 0;
		p->col[3] = RANDOM_LONG(200, 255);	
		p->scale = RANDOM_FLOAT(1.0, 2.0);
		p->life = 0.2;
		p->die = g_flClientTime + p->life;
	}
protected:

};

class CPSRocketCritGlow : public CPartSystemEntity
{
public:
	CPSRocketCritGlow(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_RocketCritGlow, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		if(p->col[2] == 0)
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 72 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 0 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 0 * frac2;
		}
		else
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 0 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 28 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 110 * frac2;
		}
		ALPHA_FADE_INOUT(0.1);

		ent.curstate.scale = 0.15+0.15*frac;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texBrightGlowY);
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

		p->vel[0] = RANDOM_FLOAT(-3, 3);
		p->vel[1] = RANDOM_FLOAT(-3, 3);
		p->vel[2] = RANDOM_FLOAT(-3, 3);
		p->org[0] = RANDOM_FLOAT(-1, 1);
		p->org[1] = RANDOM_FLOAT(-1, 1);
		p->org[2] = RANDOM_FLOAT(-1, 1);

		if(m_entity->curstate.skin == 0)
		{
			p->col[0] = RANDOM_LONG(106, 255);;
			p->col[1] = RANDOM_LONG(0, 72);
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

class CPSRocketCritSpark : public CPartSystemEntity
{
public:
	CPSRocketCritSpark(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_RocketCritSpark, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		if(p->col[0] == 255)
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

		ent.curstate.scale = 0.1*frac;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texBrightGlowY);
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

		p->vel[0] = RANDOM_FLOAT(-24, 24);
		p->vel[1] = RANDOM_FLOAT(-24, 24);
		p->vel[2] = RANDOM_FLOAT(-24, 24);
		p->org[0] = RANDOM_FLOAT(-2, 2);
		p->org[1] = RANDOM_FLOAT(-2, 2);
		p->org[2] = RANDOM_FLOAT(-2, 2);

		if(m_entity->curstate.skin == 0)
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

class CPSRocketTrail : public CPartSystemEntity
{
public:
	CPSRocketTrail(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_RocketTrail, parts, childs, entity);
		VectorCopy(entity->origin, m_lastorg);
		m_lasttime = g_flClientTime;

		CPSRocketFire *pFire = new CPSRocketFire;
		pFire->Init(15, 0, entity);

		CPSRocketBurst *pBurst = new CPSRocketBurst;
		pBurst->Init(30, 0, entity);

		CPSRocketCritGlow *pCritGlow = new CPSRocketCritGlow;
		pCritGlow->Init(30, 0, entity);

		CPSRocketCritSpark *pCritSpark = new CPSRocketCritSpark;
		pCritSpark->Init(15, 0, entity);

		AddChild(pFire);
		AddChild(pBurst);
		AddChild(pCritGlow);
		AddChild(pCritSpark);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		ent.curstate.renderfx = kRenderFxNone;		
		ent.curstate.rendercolor.r = 200;
		ent.curstate.rendercolor.g = 200;
		ent.curstate.rendercolor.b = 210;
		ent.curstate.renderamt = 200 * frac;

		if(frac > 0.9)
			ent.curstate.scale = .025f * frac / 0.9 + .025f;
		else
			ent.curstate.scale = .075f * frac2 / 0.9 + .05f;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = frac2*p->rot;

		R_DrawTGASprite(&ent, &g_texRocketSmoke[p->modn]);
	}
	virtual void Update(void)
	{
		if(!m_dead)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p;
		vec3_t sub;
		int i, div;
		float fdiv;

		VectorSubtract(m_lastorg, m_entity->origin, sub);
		div = (int)(VectorLength(sub) / 8.0f);
		for(i = 0; i < div; ++i)
		{
			p = AllocParticle();
			if(!p) break;
			fdiv = (float)i / div;
			VectorMA(m_entity->origin, fdiv, sub, p->org);
			p->vel[0] = RANDOM_FLOAT(-4,4);
			p->vel[1] = RANDOM_FLOAT(-4,4);
			p->vel[2] = RANDOM_FLOAT(-2,6);
			p->rot = RANDOM_FLOAT(-45, 45);
			p->modn = RANDOM_LONG(0,4);
			p->life = 1.5;
			p->die = g_flClientTime + (m_lasttime - g_flClientTime) * fdiv + p->life;
		}
		if(div > 0)
		{
			VectorCopy(m_entity->origin, m_lastorg);
			m_lasttime = g_flClientTime;
		}
	}
public:
	vec3_t m_lastorg;
	float m_lasttime;
};

void R_RocketTrail(cl_entity_t *pEntity)
{
	CPSRocketTrail *pTrail = (CPSRocketTrail *)R_FindPartSystem(PS_RocketTrail, pEntity);

	if(pTrail && !pTrail->GetDead())
	{
		return;
	}

	pTrail = new CPSRocketTrail;
	pTrail->Init(100, 4, pEntity);

	R_AddPartSystem(pTrail);
}