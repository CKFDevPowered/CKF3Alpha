#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "studio_util.h"
#include "particle.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

extern cl_entity_t ent;

qboolean EV_IsLocal( int idx );

class CPSBurningPlayer : public CPartSystemEntity
{
public:
	CPSBurningPlayer(){}
	void Init(int parts, int childs, cl_entity_t *entity, int team)
	{
		CPartSystemEntity::Init(PS_BurningPlayer, parts, childs, entity);
		Reset(team);

		m_mod = entity->model;
	}
	void Reset(int team)
	{
		m_team = team;
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		g_StudioRenderer.m_pCurrentEntity = m_entity;
		entity_bones_t *bones = g_StudioRenderer.GetEntityBones();
		if(bones)
		{
			VectorTransform(p->org, bones->m_bonetransform[p->bone], org);
			VectorMA(org, frac2, p->vel, org);
		}
	}
	virtual void Render(part_t *p, float *org)
	{
		if(m_entity->player && EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
			return;

		CALC_FRACTION(p);
		ent.curstate.rendermode = kRenderAddColor;
		ent.curstate.renderfx = kRenderFxNone;

		COLOR_FADE(255, 130, 39);
		ALPHA_FADE_INOUT2(0.2, 0.8);

		ent.curstate.scale = (.05f * frac2 + .1f) * p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot + frac2*30;

		R_DrawTGASprite(&ent, &g_texFlamethrowerFire[p->modn]);
	}
	virtual void Update(void)
	{
		if(m_entity->model != m_mod)
		{
			RemoveInvalid();
			m_mod = m_entity->model;
		}

		if(!m_dead && m_entity->model)
		{
			if(!(m_entity->curstate.effects & EF_AFTERBURN))
			{
				m_dead = 1;
			}

			AddParticle();
		}
	}
	void RemoveInvalid(void)
	{
		if(!m_entity->model)
			return;

		studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_entity->model);

		for(int i = 0;i < m_part.size(); ++i)
		{
			if(!m_part[i].free)
			{
				if(m_part[i].bone > pstudiohdr->numbones)
					m_part[i].die = 0;
			}
		}
	}
	void AddParticle(void)
	{
		if(m_entity->player && EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
			return;

		part_t *p = AllocParticle();

		COLOR_RANDOM_LERP(231, 143, 81, 252, 158, 118);
		p->col[3] = RANDOM_LONG(128, 200);

		{
			studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_entity->model);
			if(pstudiohdr && pstudiohdr->numhitboxes > 0)
			{
				float rfrac;
				mstudiobbox_t *pbboxes = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
				int i = RANDOM_LONG(0, pstudiohdr->numhitboxes-1);
				p->bone = pbboxes[i].bone;
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[0] = pbboxes[i].bbmax[0] * rfrac + pbboxes[i].bbmin[0] * (1-rfrac);
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[1] = pbboxes[i].bbmax[1] * rfrac + pbboxes[i].bbmin[1] * (1-rfrac);
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[2] = pbboxes[i].bbmax[2] * rfrac + pbboxes[i].bbmin[2] * (1-rfrac);
			}
		}

		p->vel[0] = RANDOM_FLOAT(-6, 6);
		p->vel[1] = RANDOM_FLOAT(-6, 6);
		p->vel[2] = RANDOM_FLOAT(8, 16);
		p->life = RANDOM_FLOAT(0.3, 1.5);
		p->die = g_flClientTime + p->life;
		p->modn = RANDOM_LONG(0, 4);
		p->rot = RANDOM_LONG(-180,180);
		p->scale = RANDOM_FLOAT(1, 1.5);
	}
public:
	int m_team;
	model_t *m_mod;
};

class CPSBurningPlayerGlow : public CPartSystemEntity
{
public:
	CPSBurningPlayerGlow(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_BurningPlayerGlow, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		g_StudioRenderer.m_pCurrentEntity = m_entity;
		entity_bones_t *bones = g_StudioRenderer.GetEntityBones();
		if(bones)
		{
			VectorTransform(p->org, bones->m_bonetransform[p->bone], org);
		}
	}
	virtual void Render(part_t *p, float *org)
	{
		if(m_entity->player && EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
			return;

		CALC_FRACTION(p);
		CPSBurningPlayer *parent = (CPSBurningPlayer *)m_parent;

		ent.curstate.rendermode = kRenderTransAdd;
		if(parent->m_team == 0)
		{
			COLOR_FADE(242, 0, 34);
		}
		else
		{
			COLOR_FADE(34, 0, 242);
		}
		ALPHA_FADE_INOUT(0.3);

		ent.curstate.scale = 0.2;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_parent->m_dead && GetActivePartCount() < 40 && m_entity->model)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		if(m_entity->player && EV_IsLocal(m_entity->index) && !CL_IsThirdPerson())
			return;

		part_t *p = AllocParticle();

		CPSBurningPlayer *parent = (CPSBurningPlayer *)m_parent;
		if(parent->m_team == 0)
		{
			COLOR_RANDOM_LERP(192, 145, 35, 251, 90, 0);
		}
		else
		{
			COLOR_RANDOM_LERP(35, 145, 192, 0, 90, 251);
		}
		p->col[3] = RANDOM_LONG(100, 122);

		{
			studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_entity->model);
			if(pstudiohdr && pstudiohdr->numhitboxes > 0)
			{
				float rfrac;
				mstudiobbox_t *pbboxes = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
				int i = RANDOM_LONG(0, pstudiohdr->numhitboxes-1);
				p->bone = pbboxes[i].bone;
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[0] = pbboxes[i].bbmax[0] * rfrac + pbboxes[i].bbmin[0] * (1-rfrac);
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[1] = pbboxes[i].bbmax[1] * rfrac + pbboxes[i].bbmin[1] * (1-rfrac);
				rfrac = RANDOM_FLOAT(0, 1);
				p->org[2] = pbboxes[i].bbmax[2] * rfrac + pbboxes[i].bbmin[2] * (1-rfrac);
			}
		}

		p->life = 0.4;
		p->die = g_flClientTime + p->life;
	}
private:
};

void R_BurningPlayer(cl_entity_t *pEntity, int iTeam, float flDuration)
{
	CPSBurningPlayer *pCore = (CPSBurningPlayer *)R_FindPartSystem(PS_BurningPlayer, pEntity);

	if(pCore)
	{
		if(flDuration <= 0)
		{
			pCore->SetDead(1);
		}
		else
		{
			pCore->SetDead(0);
			pCore->Reset(iTeam - 1);
			pCore->SetDie(flDuration);
		}
		return;
	}

	pCore = new CPSBurningPlayer;
	pCore->Init(150, 1, pEntity, iTeam - 1);
	pCore->SetDie(flDuration);

	CPSBurningPlayerGlow *pGlow = new CPSBurningPlayerGlow;
	pGlow->Init(40, 0, pEntity);

	pCore->AddChild(pGlow);

	R_AddPartSystem(pCore);
}