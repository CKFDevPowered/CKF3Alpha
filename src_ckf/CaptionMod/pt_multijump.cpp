#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"

extern cl_entity_t ent;

class CPSMultiJumpTrail : public CPartSystemEntity
{
public:
	CPSMultiJumpTrail(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MultiJumpTrail, parts, childs, entity);
		VectorCopy(entity->origin, m_lastorg);
		m_lasttime = g_flClientTime;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];
		ent.curstate.renderamt = p->col[3] * frac;
		ent.curstate.scale = .08f * frac2 + .06f;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = frac2*120 + p->rot;

		R_DrawTGASprite(&ent, &g_texSoftGlowTrans);
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
		div = (int)(VectorLength(sub) / 2.0f);
		for(i = 0; i < div; ++i)
		{
			p = AllocParticle();
			if(!p) break;
			fdiv = (float)i / div;
			VectorMA(m_entity->origin, fdiv, sub, p->org);
			p->org[2] -= 32;
			p->col[0] = 255;
			p->col[1] = 255;
			p->col[2] = 255;
			p->col[3] = 10;
			p->life = 1.0;
			p->rot = RANDOM_FLOAT(0, 360);
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

class CPSMultiJumpSmoke : public CPartSystemEntity
{
public:
	CPSMultiJumpSmoke(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MultiJumpSmoke, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];
		ent.curstate.renderamt = p->col[3] * frac;

		ent.curstate.scale = p->scale * (1 * frac2 + 2) * 0.075;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		if(p->modn % 2 == 0)
			ent.angles[2] = frac2 * 90 + p->rot;
		else
			ent.angles[2] = frac2 * -90 + p->rot;

		R_DrawTGASprite(&ent, &g_texMultiJumpSmoke[p->modn]);
	}
	virtual void Update(void)
	{
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->org[0] = RANDOM_FLOAT(-4, 4);
		p->org[1] = RANDOM_FLOAT(-4, 4);
		p->org[2] = RANDOM_FLOAT(-4, 0);
		p->vel[0] = p->org[0] * 4;
		p->vel[1] = p->org[1] * 4;
		p->vel[2] = p->org[2] * 4;
		VectorAdd(p->org, m_entity->origin, p->org);
		p->org[2] -= 32;

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(224, 236);;
		p->col[2] = RANDOM_LONG(184, 224);
		p->col[3] = RANDOM_LONG(81, 163);
		p->scale = RANDOM_FLOAT(1, 1.5);
		p->life = RANDOM_FLOAT(0.3, 0.8);
		p->modn = RANDOM_LONG(0, 4);
		p->rot = RANDOM_FLOAT(-30, 30);
		p->die = g_flClientTime + p->life;
	}
public:

};

class CPSMultiJumpSpark : public CPartSystemEntity
{
public:
	CPSMultiJumpSpark(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_MultiJumpSpark, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;	
		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];
		ALPHA_FADE_OUT(0.5);

		ent.curstate.scale = (1*frac2+0.5) * 0.01;
		ent.curstate.fuser1 = 8;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);

		R_DrawTGATracer(&ent, &g_texSoftGlowTrans);
	}
	virtual void Update(void)
	{
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_entity->origin, p->org);
		p->org[2] -= 32;

		p->vel[0] = RANDOM_FLOAT(-64, 64);
		p->vel[1] = RANDOM_FLOAT(-64, 64);
		p->vel[2] = RANDOM_FLOAT(-16, 16);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(224, 236);
		p->col[2] = RANDOM_LONG(184, 224);
		p->col[3] = RANDOM_LONG(150, 200);
		p->life = 0.3;
		p->die = g_flClientTime + p->life;
	}
public:
};

void R_MultiJumpTrail(cl_entity_t *pEntity)
{
	int i, iEmitNum;

	CPSMultiJumpTrail *pTrail;

	pTrail = (CPSMultiJumpTrail *)R_FindPartSystem(PS_MultiJumpTrail, pEntity);

	if(pTrail && !pTrail->GetDead())
	{
		return;
	}

	pTrail = new CPSMultiJumpTrail;
	pTrail->Init(150, 2, pEntity);

	CPSMultiJumpSmoke *pSmoke = new CPSMultiJumpSmoke;
	pSmoke->Init(6, 0, pEntity);
	iEmitNum = RANDOM_LONG(4, 6);
	for(i = 0; i < iEmitNum; ++i)
		pSmoke->AddParticle();

	CPSMultiJumpSpark *pSpark = new CPSMultiJumpSpark;
	pSpark->Init(6, 0, pEntity);
	for(i = 0; i < 6; ++i)
		pSpark->AddParticle();

	pTrail->AddChild(pSmoke);
	pTrail->AddChild(pSpark);
	R_AddPartSystem(pTrail);
}