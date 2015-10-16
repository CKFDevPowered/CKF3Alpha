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
qboolean EV_IsPlayer( int idx );

class CPSCritPlayerWeapon : public CPartSystemEntity
{
public:
	CPSCritPlayerWeapon(){}
	void Init(int parts, int childs, cl_entity_t *entity, int team)
	{
		CPartSystemEntity::Init(PS_CritPlayerWeapon, parts, childs, entity);
		Reset(team);
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
			VectorTransform(p->org, bones->m_lighttransform[p->bone], org);
		}
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAdd;
		ent.curstate.renderfx = kRenderFxNone;
		if(m_team == 0)
		{
			COLOR_FADE_IN2(0.2, 1, 176, 0, 0);
		}
		else
		{
			COLOR_FADE_IN2(0.2, 1, 0, 100, 249);
		}
		if(frac > 0.66)
			ent.curstate.renderamt = p->col[3] * frac2 / 0.33;
		else
			ent.curstate.renderamt = p->col[3] * frac / 0.66;

		ent.curstate.scale = (fabs(sin( frac2 / (2*M_PI) )) * 0.01f + 0.015f) * p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot;

		R_DrawTGASprite(&ent, &g_texElectric1);
	}
	virtual void Update(void)
	{
		if(!m_dead && GetActivePartCount() < 80 && m_entity->curstate.weaponmodel)
		{
			//if(!(m_entity->curstate.effects & EF_CRITBOOST))
			//{
			//	m_dead = 1;
			//}
			for(int i = 0; i < 3; ++i)
				AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(m_team == 0)
		{
			COLOR_RANDOM_LERP(255, 148, 70, 255, 108, 0);
		}
		else
		{
			COLOR_RANDOM_LERP(125, 125, 255, 116, 162, 255);
		}
		p->col[3] = 255;

		model_t *mod = IEngineStudio.GetModelByIndex(m_entity->curstate.weaponmodel);
		studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(mod);

		if(pstudiohdr && pstudiohdr->numbodyparts > 0)
		{
			int bodypart = RANDOM_LONG(0, pstudiohdr->numbodyparts-1);
			mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + bodypart;

			int index = ((int)m_entity->curstate.scale / pbodypart->base) % pbodypart->nummodels;
			mstudiomodel_t *psubmodel = (mstudiomodel_t *)((byte *)pstudiohdr + pbodypart->modelindex) + index;

			vec3_t				*pstudioverts;
			byte				*pvertbone;

			pstudioverts = (vec3_t *)((byte *)pstudiohdr + psubmodel->vertindex);				
			pvertbone = ((byte *)pstudiohdr + psubmodel->vertinfoindex);

			int i = RANDOM_LONG(0, psubmodel->numverts-1);
			p->bone = pvertbone[i];
			VectorCopy(pstudioverts[i], p->org);
		}

		p->rot = RANDOM_FLOAT(0, 360);
		p->scale = RANDOM_FLOAT(2, 3);
		p->life = RANDOM_FLOAT(0.1f, 0.2f);
		p->die = g_flClientTime + p->life;
	}
public:
	int m_team;
};

class CPSCritFirstWeapon : public CPartSystemEntity
{
public:
	CPSCritFirstWeapon(){}
	void Init(int parts, int childs, cl_entity_t *entity, int team)
	{
		CPartSystemEntity::Init(PS_CritFirstWeapon, parts, childs, entity);
		Reset(team);
	}
	void Reset(int team)
	{
		m_team = team;
	}
	virtual void Movement(part_t *p, float *org)
	{

	}
	virtual void Render(part_t *p, float *org)
	{
		if(CL_IsThirdPerson())
			return;

		CALC_FRACTION(p);

		g_StudioRenderer.m_pCurrentEntity = m_entity;
		entity_bones_t *bones = g_StudioRenderer.GetEntityBones();
		if(bones)
		{
			VectorTransform(p->org, bones->m_lighttransform[p->bone], org);
		}

		ent.curstate.rendermode = kRenderTransAdd;
		ent.curstate.renderfx = kRenderFxNone;
		if(m_team == 0)
		{
			COLOR_FADE_IN2(0.2, 1, 176, 0, 0);
		}
		else
		{
			COLOR_FADE_IN2(0.2, 1, 0, 100, 249);
		}
		if(frac > 0.66)
			ent.curstate.renderamt = p->col[3] * frac2 / 0.33;
		else
			ent.curstate.renderamt = p->col[3] * frac / 0.66;

		ent.curstate.scale = (fabs(sin( frac2 / (2*M_PI) )) * 0.01f + 0.015f) * p->scale;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot;

		R_DrawTGASprite(&ent, &g_texElectric1);
	}
	virtual void Update(void)
	{
		if(!m_dead && GetActivePartCount() < 80 && m_entity->model)
		{
			for(int i = 0; i < 2; ++i)
				AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		if(m_team == 0)
		{
			COLOR_RANDOM_LERP(255, 148, 70, 255, 108, 0);
		}
		else
		{
			COLOR_RANDOM_LERP(125, 125, 255, 116, 162, 255);
		}
		p->col[3] = 255;

		studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_entity->model);

		if(pstudiohdr && pstudiohdr->numbodyparts > 0)
		{
			int bodypart = RANDOM_LONG(0, pstudiohdr->numbodyparts-1);
			mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + bodypart;

			int currentbodypart = bodypart;
			while(!stricmp(pbodypart->name, "arms"))
			{
				bodypart = (bodypart + 1) % pstudiohdr->numbodyparts;
				if(bodypart == currentbodypart)
					break;
				pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + bodypart;				
			}

			int index = ((int)m_entity->curstate.body / pbodypart->base) % pbodypart->nummodels;
			mstudiomodel_t *psubmodel = (mstudiomodel_t *)((byte *)pstudiohdr + pbodypart->modelindex) + index;

			vec3_t				*pstudioverts;
			byte				*pvertbone;

			pstudioverts = (vec3_t *)((byte *)pstudiohdr + psubmodel->vertindex);				
			pvertbone = ((byte *)pstudiohdr + psubmodel->vertinfoindex);

			int i = RANDOM_LONG(0, psubmodel->numverts-1);
			p->bone = pvertbone[i];
			VectorCopy(pstudioverts[i], p->org);
		}

		p->rot = RANDOM_FLOAT(0, 360);
		p->scale = RANDOM_FLOAT(2, 3);
		p->life = RANDOM_FLOAT(0.1f, 0.2f);
		p->die = g_flClientTime + p->life;
	}
public:
	int m_team;
};

class CPSCritFirstWeaponGlow : public CPartSystemEntity
{
public:
	CPSCritFirstWeaponGlow(){}
	void Init(int parts, int childs, cl_entity_t *entity)
	{
		CPartSystemEntity::Init(PS_CritFirstWeaponGlow, parts, childs, entity);
	}
	virtual void Movement(part_t *p, float *org)
	{
	}
	virtual void Render(part_t *p, float *org)
	{
		if(CL_IsThirdPerson())
			return;

		CALC_FRACTION(p);

		g_StudioRenderer.m_pCurrentEntity = m_entity;
		entity_bones_t *bones = g_StudioRenderer.GetEntityBones();
		if(bones)
		{
			VectorTransform(p->org, bones->m_lighttransform[p->bone], org);
		}

		CPSCritFirstWeapon *parent = (CPSCritFirstWeapon *)m_parent;
		ent.curstate.rendermode = kRenderTransAdd;
		if(parent->m_team == 0)
		{
			COLOR_FADE(249, 53, 0);
		}
		else
		{
			COLOR_FADE(0, 100, 249);
		}
		ALPHA_FADE_INOUT(0.5);

		ent.curstate.scale = 0.12;
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texSoftGlow);
	}
	virtual void Update(void)
	{
		if(!m_parent->m_dead && GetActivePartCount() < 25&& m_entity->model)
		{
			AddParticle();
		}
	}
	void AddParticle(void)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		CPSCritFirstWeapon *parent = (CPSCritFirstWeapon *)m_parent;
		if(parent->m_team == 0)
		{
			COLOR_RANDOM_LERP(255, 108, 0, 255, 31, 31);
		}
		else
		{
			COLOR_RANDOM_LERP(125, 125, 255, 116, 162, 255);
		}
		p->col[3] = 50;

		studiohdr_t *pstudiohdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_entity->model);

		if(pstudiohdr && pstudiohdr->numbodyparts > 0)
		{
			int bodypart = RANDOM_LONG(0, pstudiohdr->numbodyparts-1);
			mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + bodypart;

			int currentbodypart = bodypart;
			while(!stricmp(pbodypart->name, "arms"))
			{
				bodypart = (bodypart + 1) % pstudiohdr->numbodyparts;
				if(bodypart == currentbodypart)
					break;
				pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + bodypart;				
			}

			int index = ((int)m_entity->curstate.body / pbodypart->base) % pbodypart->nummodels;
			mstudiomodel_t *psubmodel = (mstudiomodel_t *)((byte *)pstudiohdr + pbodypart->modelindex) + index;

			vec3_t				*pstudioverts;
			byte				*pvertbone;

			pstudioverts = (vec3_t *)((byte *)pstudiohdr + psubmodel->vertindex);				
			pvertbone = ((byte *)pstudiohdr + psubmodel->vertinfoindex);

			int i = RANDOM_LONG(0, psubmodel->numverts-1);
			p->bone = pvertbone[i];
			VectorCopy(pstudioverts[i], p->org);
		}

		p->life = RANDOM_FLOAT(0.6f, 1);
		p->die = g_flClientTime + p->life;
	}
private:
};

void R_CritFirstWeapon(cl_entity_t *pEntity, int iTeam, float flDuration)
{
	CPSCritFirstWeapon *pOldCrit = (CPSCritFirstWeapon *)R_FindPartSystem(PS_CritFirstWeapon, pEntity);;

	if(pOldCrit)
	{
		if(flDuration <= 0)
		{
			pOldCrit->SetDead(1);
		}
		else
		{
			pOldCrit->SetDead(0);
			pOldCrit->Reset(iTeam-1);
			pOldCrit->SetDie(flDuration);
		}
		return;
	}
	CPSCritFirstWeapon *pCrit = new CPSCritFirstWeapon;
	pCrit->Init(80, 1, pEntity, iTeam-1);
	pCrit->SetDie(flDuration);
	pCrit->SetCull(false);

	CPSCritFirstWeaponGlow *pGlow = new CPSCritFirstWeaponGlow;
	pGlow->Init(40, 0, pEntity);
	pGlow->SetCull(false);

	pCrit->AddChild(pGlow);

	R_AddPartSystem(pCrit);
}

void R_CritPlayerWeapon(cl_entity_t *pEntity, int iTeam, float flDuration)
{
	if(EV_IsPlayer(pEntity->index) && EV_IsLocal(pEntity->index))
	{
		R_CritFirstWeapon(cl_viewent, iTeam, flDuration);
	}

	CPSCritPlayerWeapon *pOldCrit = (CPSCritPlayerWeapon *)R_FindPartSystem(PS_CritPlayerWeapon, pEntity);;

	if(pOldCrit)
	{
		if(flDuration <= 0)
		{
			pOldCrit->SetDead(1);
		}
		else
		{
			pOldCrit->SetDead(0);
			pOldCrit->Reset(iTeam-1);
			pOldCrit->SetDie(flDuration);
		}
		return;
	}
	CPSCritPlayerWeapon *pCrit = new CPSCritPlayerWeapon;
	pCrit->Init(64, 1, pEntity, iTeam-1);
	pCrit->SetDie(flDuration);

	R_AddPartSystem(pCrit);
}