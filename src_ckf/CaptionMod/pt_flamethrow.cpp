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
void ClipVelocity(vec_t *in, vec_t *normal, vec_t *out, float overbounce);
qboolean EV_IsLocal( int idx );

class CFlameThrowCore : public CPartSystemAttachment
{
public:
	CFlameThrowCore(){}
	void Init(int parts, int childs, cl_entity_t *entity, int team)
	{
		CPartSystemAttachment::Init(PS_FlameThrow, parts, childs, entity);
		Reset(team);
	}
	void Reset(int team)
	{
		SetDie(4.0f);
		m_team = team;
	}
	virtual void Movement(part_t *p, float *org)
	{
		p->vel[2] += 32 * g_flFrameTime;
		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorAdd(p->org, m_pos, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		ent.curstate.renderfx = kRenderFxNone;
		if(frac < 0.4)
		{
			if(m_team == 1)
			{
				ent.curstate.rendercolor.r = 255;
				ent.curstate.rendercolor.g = 130;
				ent.curstate.rendercolor.b = 39;
			}
			else
			{
				ent.curstate.rendercolor.r = 252;
				ent.curstate.rendercolor.g = 197;
				ent.curstate.rendercolor.b = 92;
			}
		}
		else
		{
			if(m_team == 1)
			{
				ent.curstate.rendercolor.r = p->col[0]*(1-frac2/0.6)+255*frac2/0.6;
				ent.curstate.rendercolor.g = p->col[1]*(1-frac2/0.6)+130*frac2/0.6;
				ent.curstate.rendercolor.b = p->col[2]*(1-frac2/0.6)+39*frac2/0.6;
			}
			else
			{
				ent.curstate.rendercolor.r = p->col[0]*(1-frac2/0.6)+252*frac2/0.6;
				ent.curstate.rendercolor.g = p->col[1]*(1-frac2/0.6)+197*frac2/0.6;
				ent.curstate.rendercolor.b = p->col[2]*(1-frac2/0.6)+92*frac2/0.6;
			}
		}
		ALPHA_FADE_INOUT2(0.2, 0.8);

		ent.curstate.scale = .9f * frac2 * frac2 + .1f;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = frac2;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot+30*frac;

		if(m_firstview)
			qglDepthRange(0, 0.8);

		R_DrawTGASprite(&ent, &g_texFlamethrowerFire[p->modn]);

		if(m_firstview)
			qglDepthRange(0, 1);
	}
	virtual void Update(void)
	{
		part_t *p;
		int i, size;
		pmtrace_t pmtrace;	
		float rnd;
		vec3_t vecSrc, vecDst, vecVel;
		if(EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
		{
			VectorCopy(refparams.forward, m_fwd);
			VectorCopy(refparams.right, m_right);
			VectorCopy(refparams.up, m_up);
			VectorCopy(cl_viewent->attachment[0], vecSrc);
			m_firstview = true;
		}
		else
		{
			VectorSubtract(m_attachment[0], m_attachment[1], m_fwd);
			VectorNormalize(m_fwd);
			PerpendicularVector(m_right, m_fwd);
			CrossProduct(m_fwd, m_right, m_up);

			VectorCopy(m_attachment[0], vecSrc);
			m_firstview = false;
		}

		VectorMultiply(m_fwd, 220, vecVel);
		rnd = RANDOM_FLOAT(-10, 10);
		VectorMA(vecVel, rnd, m_right, vecVel);
		rnd = RANDOM_FLOAT(-10, 10);
		VectorMA(vecVel, rnd, m_up, vecVel);

		size = m_part.size();
		for(i = 0; i < size; ++i)
		{
			p = &m_part[i];
			VectorAdd(vecSrc, p->org, p->org);
			VectorMA(p->org, g_flFrameTime, p->vel, vecDst);
			gEngfuncs.pEventAPI->EV_PlayerTrace(p->org, vecDst, PM_STUDIO_BOX, -1, &pmtrace );
			if ( pmtrace.fraction != 1 )
			{
				// Place at contact point
				VectorMA( pmtrace.endpos, 4, pmtrace.plane.normal, vecDst);
				ClipVelocity( p->vel, pmtrace.plane.normal, p->vel, 1.05);
			}
			VectorCopy(vecDst, p->org);
			VectorSubtract(p->org, vecSrc, p->org);
		}

		VectorCopy(vecSrc, m_pos);
		VectorCopy(vecVel, m_vel);

		if(!m_dead)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_vel, p->vel);
		if(m_team == 1)
		{
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(0, 154);
			p->col[2] = RANDOM_LONG(0, 130);
		}
		else
		{
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(154, 244);
			p->col[2] = RANDOM_LONG(130, 229);
		}
		p->col[3] = 250;
		p->life = 0.6;
		p->die = g_flClientTime + p->life;
		p->modn = RANDOM_LONG(0,4);	
		p->rot = RANDOM_LONG(-180,180);
		p->scale = RANDOM_FLOAT(1, 2);
	}
public:
	int m_team;
	int m_firstview;
	vec3_t m_pos;
	vec3_t m_vel;
	vec3_t m_fwd;
	vec3_t m_right;
	vec3_t m_up;
};

class CFlameThrowFire : public CPartSystemEntity
{
public:
	CFlameThrowFire(){}
	void Init (int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_FlameThrowFire, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;
		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorAdd(p->org, parent->m_pos, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		COLOR_FADE(255, 42, 0);
		ALPHA_FADE_OUT(0.75);

		ent.curstate.scale = (1+1*frac2)*0.008;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		CFlameThrowCore *pParent = (CFlameThrowCore *)m_parent;
	
		if(pParent->m_firstview)
			qglDepthRange(0, 0.3);

		R_DrawTGASprite(&ent, &g_texBrightGlowY);

		if(pParent->m_firstview)
			qglDepthRange(0, 1);
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
		float rnd;
		int i;
		part_t *p;
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;

		CALC_ADDCOUNT(2);
		for(i = 0; i < count; ++i)
		{
			p = AllocParticle();
			p->col[0] = 255;
			p->col[1] = RANDOM_LONG(162, 216);
			p->col[2] = 0;
			p->col[3] = RANDOM_LONG(200, 255);

			rnd = RANDOM_FLOAT(150, 200);
			VectorMultiply(parent->m_fwd, rnd, p->vel);
			rnd = RANDOM_FLOAT(-12, 12);
			VectorMA(p->vel, rnd, parent->m_right, p->vel);
			rnd = RANDOM_FLOAT(-12, 12);
			VectorMA(p->vel, rnd, parent->m_up, p->vel);
			p->life = RANDOM_FLOAT(0.1, 0.6);
			p->die = g_flClientTime + p->life;
		}
	}
protected:
};

class CFlameThrowPilot : public CPartSystemEntity
{
public:
	CFlameThrowPilot(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_FlameThrowPilot, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;
		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorAdd(p->org, parent->m_pos, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		COLOR_FADE(243, 144, 27);
		ALPHA_FADE_OUT(0.5);

		if(frac2 < 0.5)
			ent.curstate.scale = (1+2*frac2 / 0.5) * p->scale;
		else
			ent.curstate.scale = 3 * p->scale;
		ent.curstate.fuser1 = 2;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);

		ent.angles[2] = 0;

		CFlameThrowCore *pParent = (CFlameThrowCore *)m_parent;
	
		if(pParent->m_firstview)
			qglDepthRange(0, 0.3);

		R_DrawTGATracer(&ent, &g_texSingleFlame);

		if(pParent->m_firstview)
			qglDepthRange(0, 1);
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
		float rnd;
		int i;
		part_t *p;
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;

		CALC_ADDCOUNT(3);
		for(i = 0; i < count; ++i)
		{
			p = AllocParticle();
			COLOR_RANDOM_LERP(0, 126, 255, 89, 85, 255);
			p->col[0] = RANDOM_LONG(0, 40);
			p->col[1] = RANDOM_LONG(40, 80);
			p->col[2] = 255;
			p->col[3] = 64;

			rnd = 100;
			VectorMultiply(parent->m_fwd, rnd, p->vel);
			rnd = RANDOM_FLOAT(-2, 2);
			VectorMA(p->vel, rnd, parent->m_right, p->vel);
			rnd = RANDOM_FLOAT(-2, 2);
			VectorMA(p->vel, rnd, parent->m_up, p->vel);

			p->life = RANDOM_FLOAT(0.2, 0.3);
			p->die = g_flClientTime + p->life;
			p->scale = 0.03;
		}
	}
protected:
};

class CFlameThrowCritGlow : public CPartSystemEntity
{
public:
	CFlameThrowCritGlow(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_FlameThrowCritGlow, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		p->vel[2] += 48 * g_flFrameTime;
		CALC_FRACTION(p);
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;
		VectorMA(p->org, g_flFrameTime, p->vel, p->org);
		VectorAdd(p->org, parent->m_pos, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;
		if(p->col[2] == 255)
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 73 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 60 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 97 * frac2;
		}
		else
		{
			ent.curstate.rendercolor.r = p->col[0] * frac + 255 * frac2;
			ent.curstate.rendercolor.g = p->col[1] * frac + 66 * frac2;
			ent.curstate.rendercolor.b = p->col[2] * frac + 0 * frac2;
		}
		ALPHA_FADE_INOUT2(0.3, 0.5);

		ent.curstate.scale = 0.1+0.3*frac2;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		CFlameThrowCore *pParent = (CFlameThrowCore *)m_parent;
	
		if(pParent->m_firstview)
			qglDepthRange(0, 0.3);

		R_DrawTGASprite(&ent, &g_texSoftGlow);

		if(pParent->m_firstview)
			qglDepthRange(0, 1);
	}
	virtual void Update(void)
	{
		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;
		if(!m_dead && ( parent->m_team == 1 || parent->m_team == 2 ))
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		float rnd;
		part_t *p;

		CFlameThrowCore *parent = (CFlameThrowCore *)m_parent;

		p = AllocParticle();

		if(parent->m_team == 1)
		{
			p->col[0] = RANDOM_LONG(246, 255);
			p->col[1] = RANDOM_LONG(161, 214);
			p->col[2] = RANDOM_LONG(161, 164);
		}
		else
		{
			p->col[0] = RANDOM_LONG(5, 56);
			p->col[1] = RANDOM_LONG(46, 168);
			p->col[2] = 255;
		}
		p->col[3] = RANDOM_LONG(180, 240);

		rnd = RANDOM_FLOAT(300, 400);
		VectorMultiply(parent->m_fwd, rnd, p->vel);
		rnd = RANDOM_FLOAT(-12, 12);
		VectorMA(p->vel, rnd, parent->m_right, p->vel);
		rnd = RANDOM_FLOAT(-12, 12);
		VectorMA(p->vel, rnd, parent->m_up, p->vel);

		p->life = 1.0;
		p->die = g_flClientTime + p->life;
	}
protected:
};

void R_FlameThrow(cl_entity_t *pEntity, int iTeam)
{
	CFlameThrowCore *pOldFlame;

	if(iTeam == 0)
	{
		pOldFlame = (CFlameThrowCore *)R_FindPartSystem(PS_FlameThrow, pEntity);
		if(pOldFlame && !pOldFlame->GetDead())
		{
			pOldFlame->SetDead(1);
			pOldFlame->Reset(iTeam);
		}
		return;
	}
	else
	{
		pOldFlame = (CFlameThrowCore *)R_FindPartSystem(PS_FlameThrow, pEntity);
		if(pOldFlame)
		{
			pOldFlame->SetDead(0);
			pOldFlame->Reset(iTeam);
			return;
		}
	}

	CFlameThrowCore *pFlame = new CFlameThrowCore;
	pFlame->Init(100, 3, pEntity, iTeam);

	CFlameThrowFire *pFire = new CFlameThrowFire;
	pFire->Init(150, 0, pEntity);

	CFlameThrowPilot *pPilot = new CFlameThrowPilot;
	pPilot->Init(150, 0, pEntity);

	CFlameThrowCritGlow *pCritGlow = new CFlameThrowCritGlow;
	pCritGlow->Init(50, 0, pEntity);

	pFlame->AddChild(pFire);
	pFlame->AddChild(pPilot);
	pFlame->AddChild(pCritGlow);
	R_AddPartSystem(pFlame);
}

class CAirBlast : public CPartSystemAttachment
{
public:
	CAirBlast(){}
	void Init (int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemAttachment::Init(PS_AirBlast, parts, childs, entity);

		m_emitted = false;
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		if(m_firstview)
		{
			VectorMA(cl_viewent->attachment[0], frac2 * 120, m_fwd, org);
		}
		else
		{
			vec3_t vecSub;
			VectorSubtract(m_attachment[0], m_attachment[1], vecSub);
			VectorNormalize(vecSub);
			VectorMA(m_attachment[0], frac2 * (2 - frac2) * 80, vecSub, org);
		}
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderShaderConc;
		ent.curstate.rendercolor.r = 255;
		ent.curstate.rendercolor.g = 255;
		ent.curstate.rendercolor.b = 255;
		ALPHA_FADE_OUT(0.35);

		ent.curstate.scale = (1.2+0.3*frac2)*0.36;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;
	
		if(m_firstview)
			qglDepthRange(0, 0.3);

		ent.curstate.fuser2 = 0.02 * frac2 * frac * 4;
		ent.curstate.fuser3 = 0.2 * frac2 * frac * 4;

		gRefExports.R_RenderCloakTexture();

		R_DrawTGASprite(&ent, &g_texConcNormal);

		if(m_firstview)
			qglDepthRange(0, 1);
	}
	virtual void Update(void)
	{
		if(EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
		{
			VectorCopy(refparams.forward, m_fwd);
			VectorCopy(refparams.right, m_right);
			VectorCopy(refparams.up, m_up);
			m_firstview = true;
		}
		else
		{
			VectorSubtract(m_attachment[0], m_attachment[1], m_fwd);
			VectorNormalize(m_fwd);
			PerpendicularVector(m_right, m_fwd);
			CrossProduct(m_fwd, m_right, m_up);
			m_firstview = false;
		}

		if(!m_dead && !m_emitted)
		{
			AddParticle();
			m_emitted = true;
		}
	}
	void AddParticle(void)
	{
		part_t *p;

		p = AllocParticle();
		p->col[0] = 255;
		p->col[1] = 255;
		p->col[2] = 255;
		p->col[3] = 166;

		p->life = 0.4;
		p->die = g_flClientTime + p->life;
	}
protected:
	qboolean m_firstview, m_emitted;
	vec3_t m_fwd, m_right, m_up;
};

void R_AirBlast(cl_entity_t *pEntity)
{
	R_FlameThrow(pEntity, 0);

	CAirBlast *pBlast = new CAirBlast;
	pBlast->Init(1, 3, pEntity);
	pBlast->SetCull(false);

	R_AddPartSystem(pBlast);
}