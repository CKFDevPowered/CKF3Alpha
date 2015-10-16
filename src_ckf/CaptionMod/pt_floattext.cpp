#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "particle.h"
#include "hud_base.h"

extern cl_entity_t ent;

void R_DrawHitDamageText(const wchar_t *text, int width, int height);
extern int g_texFloatText;

class CPSCritText : public CParticleSystem
{
public:
	CPSCritText(){}
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_CritText, parts, childs);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;

		if(frac2 < 0.2)
			ent.curstate.scale = (1 + (frac - 0.8) / 0.2) * 0.125;
		else
			ent.curstate.scale = 0.125;

		COLOR_FADE(255, 0, 0);
		ALPHA_FADE_OUT(0.75);

		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texCritHit);
	}
	void AddParticle(float *src)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->life = 2;

		VectorCopy(src, p->org);
		p->vel[2] = 20;

		p->col[0] = 0;
		p->col[1] = 255;
		p->col[2] = 30;
		p->col[3] = 255;

		p->die = p->life + g_flClientTime;
	}
public:

};

class CPSMiniCritText : public CParticleSystem
{
public:
	CPSMiniCritText(){}
	void Init(int parts, int childs)
	{
		CParticleSystem::Init(PS_MiniCritText, parts, childs);
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderAddColor;

		if(frac2 < 0.2)
			ent.curstate.scale = (1 + (frac - 0.8) / 0.2) * 0.125;
		else
			ent.curstate.scale = 0.125;

		COLOR_FADE(255, 0, 0);
		ALPHA_FADE_OUT(0.75);

		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		R_DrawTGASprite(&ent, &g_texCritHit);
	}
	void AddParticle(float *src)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->life = 2;

		VectorCopy(src, p->org);
		p->vel[2] = 20;

		COLOR_RANDOM_LERP(234, 255, 0, 255, 234, 0);
		p->col[3] = 255;

		p->die = p->life + g_flClientTime;
	}
public:

};

void R_CritText(vec3_t vecSrc)
{
	CPSCritText *pText = new CPSCritText;
	pText->Init(0, 1);
	pText->SetDie(2.0);
	pText->AddParticle(vecSrc);
	R_AddPartSystem(pText);
}

void R_MiniCritText(vec3_t vecSrc)
{
	CPSMiniCritText *pText = new CPSMiniCritText;
	pText->Init(0, 1);
	pText->SetDie(2.0);
	pText->AddParticle(vecSrc);
	R_AddPartSystem(pText);
}

class CPSHitDamageText : public CParticleSystem
{
public:
	CPSHitDamageText(){}
	void Init(void)
	{
		CParticleSystem::Init(PS_HitDamageText, 1, 0);

		m_frame.texcoord[0][0] = 0;
		m_frame.texcoord[0][1] = 0;
		m_frame.texcoord[1][0] = 0;
		m_frame.texcoord[1][1] = 1;
		m_frame.texcoord[2][0] = 1;
		m_frame.texcoord[2][1] = 1;
		m_frame.texcoord[3][0] = 1;
		m_frame.texcoord[3][1] = 0;
		m_sprite.frame = &m_frame;
		m_sprite.w = ScreenWidth;
		m_sprite.h = ScreenHeight;
		m_sprite.numframes = 1;
		m_sprite.tex = g_texFloatText;
	}
	virtual void Movement(part_t *p, float *org)
	{
		R_ParticleMovementSimple(p, org);
	}
	virtual void Render(part_t *p, float *org)
	{
		if(gRefExports.R_GetDrawPass() != r_draw_normal)
			return;

		CALC_FRACTION(p);

		ent.curstate.rendermode = kRenderTransAlphaNoDepth;

		if(frac2 < 0.2)
			ent.curstate.scale = (1 + (frac - 0.8) / 0.2) * 0.05;
		else
			ent.curstate.scale = 0.05;

		COLOR_FADE(255, 255, 0);
		ALPHA_FADE_OUT(0.75);

 		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = 0;

		gRefExports.R_PushFrameBuffer();

		gRefExports.R_BeginDrawHUDInWorld(m_sprite.tex, m_sprite.w, m_sprite.h);

		R_DrawHitDamageText(m_text, m_sprite.w, m_sprite.h);

		gRefExports.R_FinishDrawHUDInWorld();

		gRefExports.R_PopFrameBuffer();

		R_DrawTGASprite(&ent, &m_sprite);
	}
	void AddParticle(int damage, float *src)
	{
		part_t *p = AllocParticle();
		if(!p) return;

		p->life = 2;

		VectorCopy(src, p->org);
		p->vel[2] = 20;

		p->col[0] = 255;
		p->col[1] = 0;
		p->col[2] = 0;
		p->col[3] = 255;

		p->die = p->life + g_flClientTime;

		wsprintfW(m_text, L"%d", damage);
	}
public:
	tgasprite_frame_t m_frame;
	tgasprite_t m_sprite;

	wchar_t m_text[64];	
};

void R_HitDamageText(int damage, vec3_t vecSrc)
{
	if(!(g_RefSupportExt & r_ext_fbo))
		return;

	CPSHitDamageText *pText = new CPSHitDamageText;
	pText->Init();
	pText->SetDie(2.0);
	pText->AddParticle(damage, vecSrc);
	R_AddPartSystem(pText);
}