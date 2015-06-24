#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "studio_util.h"
#include "particle.h"

extern cl_entity_t ent;

class CPSBurningPlayer : public CPartSystemBones
{
public:
	CPSBurningPlayer(){}
	void Init(int parts, int childs, cl_entity_t *entity, int team, float time)
	{
		CPartSystemBones::Init(PS_BurningPlayer, parts, childs, entity);
		Reset(team, time);
	}
	void Reset(int team, float time)
	{
		SetDie(time);
		m_team = team;
	}
	virtual void Movement(part_t *p, float *org)
	{
		CALC_FRACTION(p);
		VectorTransform(p->org, m_lighttransform[p->bone], org);
		VectorMA(org, frac2, p->vel, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		if(!m_bonesaved)
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
		if(!m_dead)
		{
			if(!(m_entity->curstate.effects & EF_AFTERBURN))
			{
				m_dead = 1;
			}

			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		COLOR_RANDOM_LERP(231, 143, 81, 252, 158, 118);
		p->col[3] = RANDOM_LONG(128, 200);

		if(m_entity->model)
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
		CPSBurningPlayer *parent = (CPSBurningPlayer *)m_parent;
		VectorTransform(p->org, parent->m_lighttransform[p->bone], org);
		//VectorMA(org, frac2, p->vel, org);
	}
	virtual void Render(part_t *p, float *org)
	{
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
		if(!m_parent->m_dead && m_part.size() < 40)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

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

		if(m_entity->model)
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

void R_BurningPlayer(cl_entity_t *pEntity, int iTeam, float flTime)
{
	CPSBurningPlayer *pCore = (CPSBurningPlayer *)R_FindPartSystem(PS_BurningPlayer, pEntity);

	if(pCore)
	{
		if(flTime <= 0)
		{
			pCore->SetDead(1);
		}
		else
		{
			pCore->SetDead(0);
			pCore->Reset(iTeam - 1, flTime);
		}
		return;
	}

	pCore = new CPSBurningPlayer;
	pCore->Init(150, 1, pEntity, iTeam - 1, flTime);

	CPSBurningPlayerGlow *pGlow = new CPSBurningPlayerGlow;
	pGlow->Init(40, 0, pEntity);

	pCore->AddChild(pGlow);

	R_AddPartSystem(pCore);
}