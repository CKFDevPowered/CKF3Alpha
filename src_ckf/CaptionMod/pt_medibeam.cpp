#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"

extern cl_entity_t ent;

class CPSMedibeam : public CPartSystemAttachment
{
public:
	CPSMedibeam(){}
	void Init(int parts, int childs, cl_entity_t *entity, cl_entity_t *target, int team)
	{
		CPartSystemAttachment::Init(PS_MediBeam, parts, childs, entity);
		m_target = target;
		Reset(team);
	}
	void Reset(int team)
	{
		SetDie(1.25f);
		m_team = team;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementNone(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);
		ent.curstate.rendermode = kRenderTransAdd;
		ent.curstate.renderfx = kRenderFxNone;
		ent.curstate.rendercolor.r = p->col[0];
		ent.curstate.rendercolor.g = p->col[1];
		ent.curstate.rendercolor.b = p->col[2];
		ent.curstate.renderamt = p->col[3];
		ent.curstate.scale = .06f;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot;

		R_DrawTGASprite(&ent, &g_texMediBeam);
	}
	virtual void Update(void)
	{
		int i, size;
		vec3_t vecSrc, vecFwd, vecRight, vecUp, vecMid;
		part_t *p;

		if(m_entity->player)
		{
			if(m_entity == gEngfuncs.GetLocalPlayer() && CL_CanDrawViewModel())
			{
				VectorCopy(gEngfuncs.GetViewModel()->attachment[0], vecSrc);
				gEngfuncs.pfnAngleVectors(refdef->viewangles, vecFwd, vecRight, vecUp);
			}
			else
			{
				VectorCopy(m_attachment[0], vecSrc);
				gEngfuncs.pfnAngleVectors(m_entity->angles, vecFwd, vecRight, vecUp);
			}
		}
		else
		{
			VectorCopy(m_entity->origin, vecSrc);		
			vecSrc[2] += 24;
			gEngfuncs.pfnAngleVectors(m_entity->angles, vecFwd, vecRight, vecUp);
		}
		VectorMA(vecSrc, 48, vecFwd, vecMid);

		size = m_part.size();
		for(i = 0; i < size; ++i)
		{
			p = &m_part[i];
			CALC_FRACTION(p);
			p->org[0] = frac * frac * vecSrc[0] + 2 * frac * frac2 * vecMid[0] + frac2 * frac2 * m_target->origin[0];
			p->org[1] = frac * frac * vecSrc[1] + 2 * frac * frac2 * vecMid[1] + frac2 * frac2 * m_target->origin[1];
			p->org[2] = frac * frac * vecSrc[2] + 2 * frac * frac2 * vecMid[2] + frac2 * frac2 * m_target->origin[2];
		}

		if(!m_dead)
		{
			CALC_ADDCOUNT(3);

			for(i = 0; i < count; ++i)
			{
				p = AllocParticle();
				VectorMA(vecSrc, 200 * g_flFrameTime * (float)i / count, vecFwd, p->org);
				p->col[0] = (m_team == 0) ? 255 : 50;
				p->col[1] = 80;
				p->col[2] = (m_team == 0) ? 50 : 255;
				p->col[3] = 128;
				p->rot = g_flClientTime * 360;
				p->life = 0.8f;
				p->die = g_flClientTime + p->life;
			}
		}
	}
	void AddParticle(void)
	{

	}
public:
	int m_team;
	cl_entity_t *m_target;
	float m_savedattachment[4];
};

void R_KillMediBeam(cl_entity_t *pEntity, cl_entity_t *pTarget)
{
	CParticleSystem *t;
	int i, size;
	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->GetType() == PS_MediBeam)
		{
			CPSMedibeam *pMedibeam = (CPSMedibeam *)t;
			if(pMedibeam->m_entity == pEntity && pMedibeam->m_target == pTarget && !pMedibeam->GetDead())
			{
				pMedibeam->SetDead(1);
			}
		}
	}
}

void R_MediBeam(cl_entity_t *pEntity, cl_entity_t *pTarget, int iTeam)
{
	CParticleSystem *t;
	CPSMedibeam *pMedibeam;
	int i, size;

	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->GetType() == PS_MediBeam)
		{
			pMedibeam = (CPSMedibeam *)t;
			if(pMedibeam->m_entity == pEntity && pMedibeam->m_target == pTarget && pMedibeam->GetDead() < 2)
			{
				pMedibeam->SetDead(0);
				pMedibeam->Reset(iTeam);
				return;
			}
		}
	}

	pMedibeam = new CPSMedibeam;
	pMedibeam->Init(100, 0, pEntity, pTarget, iTeam);

	R_AddPartSystem(pMedibeam);
}