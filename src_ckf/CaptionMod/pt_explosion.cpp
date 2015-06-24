#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"

extern cl_entity_t ent;

void PerpendicularVector(vec3_t dst, vec3_t src);
int VectorCompare (const float *v1, const float *v2);

class CPSExplosion : public CPartSystemCoord
{
public:
	CPSExplosion(){}
	void Init(int type, int parts, float *org, float *vel, float *right, float *up)
	{
		CPartSystemCoord::Init(type, parts, 0, org, vel);
		if(right) VectorCopy(right, m_right);
		if(up) VectorCopy(up, m_up);
	}
protected:
	vec3_t m_right;
	vec3_t m_up;
};

class CPSExplosionDebris : public CPSExplosion
{
public:
	CPSExplosionDebris(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionDebris, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		pmtrace_t *tr;

		p->vel[2] -= 800 * g_flFrameTime;
		VectorMA(p->org, g_flFrameTime, p->vel, org);

		tr = cl_pmove->PM_TraceLineEx(p->org, org, PM_STUDIO_IGNORE, 2, CL_TraceEntity_Ignore );

		if ( tr->fraction != 1 )
		{
			VectorCopy(tr->endpos, org);
		}
		VectorCopy(org, p->org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		COLOR_FADE(82, 60, 31);
		ALPHA_FADE_OUT(0.5);

		ent.curstate.scale = p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot;

		R_DrawTGASprite(&ent, &g_texDebris[p->modn]);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-250, 250);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-250, 250);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(100, 400);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.02, 0.08);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.7, 1.4);
		p->modn = RANDOM_LONG(0, 5);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-250, 250);
		p->vel[1] = RANDOM_FLOAT(-250, 250);
		p->vel[2] = RANDOM_FLOAT(-250, 250);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.02, 0.08);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.7, 1.4);
		p->modn = RANDOM_LONG(0, 5);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionDustup : public CPSExplosion
{
public:
	CPSExplosionDustup(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionDustup, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		COLOR_FADE(42, 32, 19);
		ALPHA_FADE_OUT(0.7);

		ent.curstate.scale = p->scale * 0.05 + 0.05 * frac2;
		ent.curstate.fuser1 = 4 + 8 * frac2;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;
		VectorCopy(p->vel, ent.curstate.velocity);

		R_DrawTGATracer(&ent, &g_texSoftGlowTrans);
	}
	virtual void Update(void)
	{
		if(!m_dead)
		{
			int count = 100.0 * g_flFrameTime;
			if(VectorCompare(m_vel, g_vecZero))
			{
				for(int i = 0; i < count; ++i)
					AddParticle();
			}
			else
			{
				for(int i = 0; i < count; ++i)
					AddParticleMA();
			}
		}
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-320, 320);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-320, 320);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(0, 320);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = RANDOM_LONG(160, 240);
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(0.1, 0.3);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-320, 320);
		p->vel[1] = RANDOM_FLOAT(-320, 320);
		p->vel[2] = RANDOM_FLOAT(-320, 320);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = RANDOM_LONG(160, 240);
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(0.1, 0.3);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionDustup2 : public CPSExplosion
{
public:
	CPSExplosionDustup2(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionDustup2, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		COLOR_FADE(42, 32, 19);
		ALPHA_FADE_OUT(0.7);

		ent.curstate.scale = p->scale * 0.2 + 0.4 * frac2;
		ent.curstate.fuser1 = frac + 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot;

		R_DrawTGASprite(&ent, &g_texSoftGlowTrans);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-48, 48);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-48, 48);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(80, 140);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = RANDOM_LONG(128, 212);
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.4, 0.6);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-48, 48);
		p->vel[1] = RANDOM_FLOAT(-48, 48);
		p->vel[2] = RANDOM_FLOAT(-48, 48);

		p->col[0] = 255;
		p->col[1] = RANDOM_LONG(42, 216);
		p->col[2] = 36;
		p->col[3] = RANDOM_LONG(128, 212);
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.4, 0.6);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionCoreFlash : public CPSExplosion
{
public:
	CPSExplosionCoreFlash(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionCoreFlash, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		COLOR_FADE_IN(0.7, 182, 57, 14);
		ALPHA_FADE_OUT(0.7);

		ent.curstate.scale = p->scale * 0.15 + 0.3 * frac2;
		ent.curstate.fuser1 = frac2 * 2 + 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot + 24 * frac2;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-400, 400);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-400, 400);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(200, 400);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		COLOR_RANDOM_LERP(209, 69, 0, 255, 102, 0);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.05, 0.17);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-400, 400);
		p->vel[1] = RANDOM_FLOAT(-400, 400);
		p->vel[2] = RANDOM_FLOAT(-400, 400);

		COLOR_RANDOM_LERP(209, 69, 0, 255, 102, 0);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->rot = RANDOM_FLOAT(0, 360);
		p->life = RANDOM_FLOAT(0.05, 0.17);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionSmoke : public CPSExplosion
{
public:
	CPSExplosionSmoke(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionSmoke, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		if(VectorLength(p->vel) > 60)
		{
			VectorMA(p->vel, -0.5, p->vel, p->vel);
		}
		p->vel[2] += 120 * g_flFrameTime;
	
		VectorMA(p->org, g_flFrameTime, p->vel, org);
		VectorCopy(org, p->org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlpha;
		COLOR_FADE_IN(0.3, 82, 80, 80);
		ALPHA_FADE_OUT(0.5);

		ent.curstate.scale = (0.4 + 0.6 * frac2) * p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot + 24 * frac2;

		R_DrawTGASprite(&ent, &g_texMultiJumpSmoke[p->modn]);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-940, 940);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-940, 940);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(525, 2200);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		COLOR_RANDOM_LERP(104, 57, 42, 103, 86, 58);
		p->col[3] = RANDOM_LONG(180, 255);
		p->scale = RANDOM_FLOAT(0.3, 0.5);
		p->rot = RANDOM_FLOAT(-35, 35);
		p->life = RANDOM_FLOAT(0.6, 2.0);
		p->die = g_flClientTime + p->life;
		p->modn = RANDOM_LONG(0, 4);
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-940, 940);
		p->vel[1] = RANDOM_FLOAT(-940, 940);
		p->vel[2] = RANDOM_FLOAT(450, 2000);

		COLOR_RANDOM_LERP(104, 57, 42, 103, 86, 58);
		p->col[3] = RANDOM_LONG(180, 255);
		p->scale = RANDOM_FLOAT(0.3, 0.5);
		p->rot = RANDOM_FLOAT(-35, 35);
		p->life = RANDOM_FLOAT(0.6, 2.0);
		p->die = g_flClientTime + p->life;
		p->modn = RANDOM_LONG(0, 4);
	}
};

class CPSExplosionFloatieEmbers : public CPSExplosion
{
public:
	CPSExplosionFloatieEmbers(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionFloatieEmbers, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);
		if(VectorLength(p->vel) > 60)
		{
			VectorMA(p->vel, -0.225, p->vel, p->vel);
		}
		else
		{
			p->vel[2] += 16 * g_flFrameTime;
			p->vel[0] = RANDOM_FLOAT(-32, 32);
			p->vel[1] = RANDOM_FLOAT(-32, 32);
		}

		VectorMA(p->org, g_flFrameTime, p->vel, org);
		VectorCopy(org, p->org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		COLOR_FADE_IN2(0.3, 0.7, 255, 84, 0);
		ALPHA_FADE_OUT(0.2);

		ent.curstate.scale = p->scale * 0.01;
		ent.curstate.fuser1 = max(VectorLength(p->vel) / 60.0, 1);
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);
		ent.angles[2] = 0;

		R_DrawTGATracer(&ent, &g_texBrightGlowY);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-1200, 1200);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-1200, 1200);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(400, 1600);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		COLOR_RANDOM_LERP(233, 88, 0, 255, 185, 16);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(1.5, 2.5);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-1200, 1200);
		p->vel[1] = RANDOM_FLOAT(-1200, 1200);
		p->vel[2] = RANDOM_FLOAT(400, 1600);

		COLOR_RANDOM_LERP(233, 88, 0, 255, 185, 16);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(1.5, 2.5);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionFlyingEmbers : public CPSExplosion
{
public:
	CPSExplosionFlyingEmbers(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionFlyingEmbers, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		p->vel[2] -= 400 * g_flFrameTime;

		VectorMA(p->org, g_flFrameTime, p->vel, org);
		VectorCopy(org, p->org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		COLOR_FADE_IN(0.7, 255, 84, 0);
		ALPHA_FADE_OUT(0.5);

		ent.curstate.scale = p->scale * frac * 0.006;
		if(frac2 < 0.4)
			ent.curstate.fuser1 = 1 + 35 * frac2 / 0.4;
		else
			ent.curstate.fuser1 = max(VectorLength(p->vel) / 40.0, 36);
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);
		ent.angles[2] = 0;

		R_DrawTGATracer(&ent, &g_texCircle2);

		//test
		ent.curstate.fuser1 = 1;
		ent.curstate.scale *= 1.5;
		VectorCopy(g_TracerEndPos, ent.origin);
		R_DrawTGASprite(&ent, &g_texCircle2);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-800, 800);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-800, 800);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(400, 1200);
		VectorMA(p->vel, rnd, m_vel, p->vel);

		COLOR_RANDOM_LERP(255, 224, 16, 233, 100, 0);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(0.1, 0.34);
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		p->vel[0] = RANDOM_FLOAT(-800, 800);
		p->vel[1] = RANDOM_FLOAT(-800, 800);
		p->vel[2] = RANDOM_FLOAT(400, 1200);

		COLOR_RANDOM_LERP(255, 224, 16, 233, 100, 0);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1);
		p->life = RANDOM_FLOAT(0.1, 0.34);
		p->die = g_flClientTime + p->life;
	}
};

class CPSExplosionFlash : public CPSExplosion
{
public:
	CPSExplosionFlash(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSExplosion::Init(PS_ExplosionFlash, parts, org, vel, right, up);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		COLOR_FADE(252, 119, 0);
		ALPHA_FADE_OUT(0.6);

		ent.curstate.scale = (frac2 * 31 + 1) * 0.09;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texBrightGlowY);
	}
	void AddParticle(void)
	{
	
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		COLOR_RANDOM_LERP(253, 228, 169, 224, 185, 153);
		p->col[3] = 212;
		p->life = 0.15;
		p->die = g_flClientTime + p->life;
	}
	void AddParticleMA(void)
	{
		AddParticle();
	}
};

void R_ExplosionWall(vec3_t vecStart, vec3_t vecNormal)
{
	int i, iNumEmit;
	vec3_t vecRight, vecUp;

	if(VectorCompare(vecNormal, g_vecZero))
	{
		VectorClear(vecRight);
		VectorClear(vecUp);
		vecNormal[0] = 0;
		vecNormal[1] = 0;
		vecNormal[2] = 1;
	}
	else
	{
		PerpendicularVector(vecRight, vecNormal);
		CrossProduct(vecRight, vecNormal, vecUp);
	}

	CParticleSystem *pExplosionCore = new CParticleSystem;
	pExplosionCore->Init(PS_ExplosionWall, 0, 5);
	pExplosionCore->SetDie(3.0);

	CPSExplosionDebris *pDebris = new CPSExplosionDebris;
	pDebris->Init(45, vecStart, vecNormal, vecRight, vecUp);
	iNumEmit = RANDOM_LONG(35, 45);
	for(i = 0; i < iNumEmit; ++i)
		pDebris->AddParticle();

	CPSExplosionDustup *pDustup = new CPSExplosionDustup;
	pDustup->Init(24, vecStart, vecNormal, vecRight, vecUp);
	pDustup->SetDie(0.12);

	CPSExplosionDustup2 *pDustup2 = new CPSExplosionDustup2;
	pDustup2->Init(6, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 6; ++i)
		pDustup2->AddParticle();

	CPSExplosionCoreFlash *pCoreFlash = new CPSExplosionCoreFlash;
	pCoreFlash->Init(12, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 12; ++i)
		pCoreFlash->AddParticle();

	CPSExplosionSmoke *pSmoke = new CPSExplosionSmoke;
	pSmoke->Init(8, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 8; ++i)
		pSmoke->AddParticle();

	CPSExplosionFloatieEmbers *pFloatieEmbers = new CPSExplosionFloatieEmbers;
	pFloatieEmbers->Init(20, vecStart, vecNormal, vecRight, vecUp);
	iNumEmit = RANDOM_LONG(16, 20);
	for(i = 0; i < iNumEmit; ++i)
		pFloatieEmbers->AddParticle();

	CPSExplosionFlyingEmbers *pFlyingEmbers = new CPSExplosionFlyingEmbers;
	pFlyingEmbers->Init(16, vecStart, vecNormal, vecRight, vecUp);
	iNumEmit = RANDOM_LONG(8, 16);
	for(i = 0; i < iNumEmit; ++i)
		pFlyingEmbers->AddParticle();

	CPSExplosionFlash *pFlash = new CPSExplosionFlash;
	pFlash->Init(1, vecStart, vecNormal, vecRight, vecUp);
	pFlash->AddParticle();

	pExplosionCore->AddChild(pDebris);
	pExplosionCore->AddChild(pDustup);
	pExplosionCore->AddChild(pDustup2);
	pExplosionCore->AddChild(pCoreFlash);
	pExplosionCore->AddChild(pSmoke);
	pExplosionCore->AddChild(pFloatieEmbers);
	pExplosionCore->AddChild(pFlyingEmbers);
	pExplosionCore->AddChild(pFlash);

	R_AddPartSystem(pExplosionCore);
}

extern vec3_t g_vecZero;

void R_ExplosionMidAir(vec3_t vecStart)
{
	int i, iNumEmit;
	vec3_t vecRight, vecUp;

	VectorClear(vecRight);
	VectorClear(vecUp);

	CParticleSystem *pExplosionCore = new CParticleSystem;
	pExplosionCore->Init(PS_ExplosionMidAir, 0, 5);
	pExplosionCore->SetDie(3.0);

	CPSExplosionDebris *pDebris = new CPSExplosionDebris;
	pDebris->Init(45, vecStart, g_vecZero, g_vecZero, g_vecZero);
	iNumEmit = RANDOM_LONG(35, 45);
	for(i = 0; i < iNumEmit; ++i)
		pDebris->AddParticleMA();

	CPSExplosionDustup *pDustup = new CPSExplosionDustup;
	pDustup->Init(24, vecStart, g_vecZero, g_vecZero, g_vecZero);
	pDustup->SetDie(0.12);

	CPSExplosionDustup2 *pDustup2 = new CPSExplosionDustup2;
	pDustup2->Init(6, vecStart, g_vecZero, g_vecZero, g_vecZero);
	for(i = 0; i < 6; ++i)
		pDustup2->AddParticleMA();

	CPSExplosionCoreFlash *pCoreFlash = new CPSExplosionCoreFlash;
	pCoreFlash->Init(12, vecStart, g_vecZero, g_vecZero, g_vecZero);
	for(i = 0; i < 12; ++i)
		pCoreFlash->AddParticleMA();

	CPSExplosionSmoke *pSmoke = new CPSExplosionSmoke;
	pSmoke->Init(8, vecStart, g_vecZero, g_vecZero, g_vecZero);
	for(i = 0; i < 8; ++i)
		pSmoke->AddParticleMA();

	CPSExplosionFloatieEmbers *pFloatieEmbers = new CPSExplosionFloatieEmbers;
	pFloatieEmbers->Init(20, vecStart, g_vecZero, g_vecZero, g_vecZero);
	iNumEmit = RANDOM_LONG(16, 20);
	for(i = 0; i < iNumEmit; ++i)
		pFloatieEmbers->AddParticleMA();

	CPSExplosionFlyingEmbers *pFlyingEmbers = new CPSExplosionFlyingEmbers;
	pFlyingEmbers->Init(16, vecStart, g_vecZero, g_vecZero, g_vecZero);
	iNumEmit = RANDOM_LONG(8, 16);
	for(i = 0; i < iNumEmit; ++i)
		pFlyingEmbers->AddParticleMA();

	CPSExplosionFlash *pFlash = new CPSExplosionFlash;
	pFlash->Init(1, vecStart, g_vecZero, g_vecZero, g_vecZero);
	pFlash->AddParticleMA();

	pExplosionCore->AddChild(pDebris);
	pExplosionCore->AddChild(pDustup);
	pExplosionCore->AddChild(pDustup2);
	pExplosionCore->AddChild(pCoreFlash);
	pExplosionCore->AddChild(pSmoke);
	pExplosionCore->AddChild(pFloatieEmbers);
	pExplosionCore->AddChild(pFlyingEmbers);
	pExplosionCore->AddChild(pFlash);

	R_AddPartSystem(pExplosionCore);
}