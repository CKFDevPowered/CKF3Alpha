#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "qgl.h"
#include "particle.h"

extern cl_entity_t ent;
int CL_TraceEntity_Ignore(physent_t *pe);
void PerpendicularVector(vec3_t dst, vec3_t src);
int VectorCompare (const float *v1, const float *v2);
void EV_GetGunPosition(int idx, int attachment, float *origin);

//tracer

class CBaseTracer : public CParticleSystem
{
public:
	CBaseTracer(){}
	void Init(int type, int parts, int team, float delay, float speed)
	{
		CParticleSystem::Init(type, parts, 0);
		SetDie(1.5);
		m_team = team;
		m_cull = false;
		m_sort = false;

		m_curdelay = 0;
		m_delay = delay;
		m_speed = speed;
	}
	virtual void Render(part_t *p, float *org)
	{
		vec3_t	viewdir, cross;
		vec3_t	start, end, dir, tmp;
		float sd, ed;
		float sv, ev;
		float frac;

		float width = 1.25f;

		if(m_team != 0)
			width = 1.75f;

		// calculate distance
		VectorSubtract(p->vel, p->org, dir);
		float dist = VectorLength(dir);
		float distadd = min(5000, dist);
		float fulldist = dist + distadd;

		float life = fulldist / m_speed;

		//start time
		frac = (g_flClientTime - p->scale) / life;

		if(frac < 0)
			return;

		//go die
		if(frac > 1)
		{
			p->die = 0;
			return;
		}

		// calculate our distance along our path
		ed = fulldist * frac;
		sd = ed - 5000;

		ev = min( 1.0f, 1.0f - (ed - dist) / distadd );
		sv = 0.0f;

		// clip start V to end V
		sv = min(ev, sv);

		// clip to start
		ed = max( 0.0f, ed );
		sd = max( 0.0f, sd );

		// clip to end
		ed = min( dist, ed );
		sd = min( dist, sd );

		if( sd == ed )
			return;

		//VectorCopy(p->org, start);

		VectorNormalize(dir);

		VectorMA(p->org, ed, dir, end );
		VectorMA(p->org, sd, dir, start );

		//VectorSubtract( end, start, linedir );
		VectorSubtract( end, refdef->vieworg, viewdir );

		CrossProduct( dir, viewdir, cross );
		VectorNormalize( cross );

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		qglEnable(GL_BLEND);
		if(m_team == 0)
			qglBlendFunc(GL_SRC_COLOR, GL_ONE);
		else
			qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		qglColor4ub(p->col[0], p->col[1], p->col[2], 255);
		qglDepthMask(0);

		gRefExports.RefAPI.GL_DisableMultitexture();
		if(m_team >= 0 && m_team <= 2)
		{
			gRefExports.RefAPI.GL_Bind(g_texTracer[m_team].tex);
		}

		qglDisable(GL_CULL_FACE);

		qglBegin( GL_QUADS );
		VectorMA( start, -width, cross, tmp );
		qglTexCoord2f( 0.0f, sv );
		qglVertex3fv( tmp );
		VectorMA( start, width, cross, tmp );
		qglTexCoord2f( 1.0f, sv );
		qglVertex3fv( tmp );
		VectorMA( end, width, cross, tmp );
		qglTexCoord2f( 1.0f, ev );
		qglVertex3fv( tmp );
		VectorMA( end, -width, cross, tmp );
		qglTexCoord2f( 0.0f, ev );
		qglVertex3fv( tmp );
		qglEnd();

		qglEnable(GL_CULL_FACE);

		if(gl_wireframe->value != 0)
		{
			qglColor4f(0,1,0,1);
			qglDisable(GL_TEXTURE_2D);
			qglBegin(GL_LINE_LOOP);
			VectorMA( start, -width, cross, tmp );
			qglVertex3fv( tmp );
			VectorMA( start, width, cross, tmp );
			qglVertex3fv( tmp );
			VectorMA( end, width, cross, tmp );
			qglVertex3fv( tmp );
			VectorMA( end, -width, cross, tmp );
			qglVertex3fv( tmp );
			qglEnd();
			qglEnable(GL_TEXTURE_2D);
		}

		qglDepthMask(1);
	}
	void AddParticle(vec3_t src, vec3_t dst)
	{
		part_t *p = AllocParticle();

		VectorCopy(src, p->org);
		VectorCopy(dst, p->vel);

		switch(m_team)
		{
		case 0:
			COLOR_RANDOM_LERP(255, 234, 151, 255, 201, 138);
			p->col[3] = 255;
			break;
		case 1:
			//COLOR_RANDOM_LERP(255, 2, 2, 255, 0, 24);
			p->col[0] = 255;
			p->col[1] = 255;
			p->col[2] = 255;
			p->col[3] = 255;
			break;
		case 2:
			//COLOR_RANDOM_LERP(0, 30, 255, 2, 38, 255);
			p->col[0] = 255;
			p->col[1] = 255;
			p->col[2] = 255;
			p->col[3] = 255;
			break;
		}
		p->scale = g_flClientTime + m_curdelay;
		p->life = 99999;
		p->die = g_flClientTime + p->life;
		m_curdelay += m_delay;
	}
	virtual void EmitParticle(vec3_t dst){}
protected:
	int m_team;
	float m_delay;
	float m_curdelay;
	float m_speed;
};

class CCoordTracer : public CBaseTracer
{
public:
	CCoordTracer(){}
	void Init(int type, int parts, int team, float delay, float speed, vec3_t src)
	{
		CBaseTracer::Init(type, parts, team, delay, speed);

		VectorCopy(src, m_src);
	}
	virtual void EmitParticle(vec3_t dst)
	{
		AddParticle(m_src, dst);
	}
private:
	vec3_t m_src;
};

class CEntityTracer : public CBaseTracer
{
public:
	CEntityTracer(){}
	void Init(int type, int parts, int team, float delay, float speed, int entindex, int attachment)
	{
		CBaseTracer::Init(type, parts, team, delay, speed);

		m_entindex = entindex;
		m_attachment = attachment;
	}
	virtual void Render(part_t *p, float *org)
	{
		EV_GetGunPosition(m_entindex, m_attachment, p->org);

		CBaseTracer::Render(p, org);
	}
	virtual void EmitParticle(vec3_t dst)
	{
		vec3_t src;
		EV_GetGunPosition(m_entindex, m_attachment, src);

		AddParticle(src, dst);

		//gEngfuncs.Con_Printf("EmitParticle: src(%.2f,%.2f,%.2f) dst(%.2f,%.2f,%.2f)\n", src[0], src[1], src[2], dst[0], dst[1], dst[2]);
	}
private:
	int m_entindex;
	int m_attachment;
};

static CBaseTracer *g_pCurTracer;

void R_BeginCoordTracer(int iTracerColor, int iNumTracer, float flDelay, float flSpeed, vec3_t vecSrc)
{
	CCoordTracer *pTracer = new CCoordTracer;
	pTracer->Init(PS_CoordTracer, iNumTracer, iTracerColor, flDelay, flSpeed, vecSrc);

	R_AddPartSystem(pTracer);

	g_pCurTracer = (CBaseTracer *)pTracer;
}

void R_BeginEntityTracer(int iTracerColor, int iNumTracer, float flDelay, float flSpeed, int iEntityIndex, int iAttachIndex)
{
	CEntityTracer *pTracer = new CEntityTracer;
	pTracer->Init(PS_EntityTracer, iNumTracer, iTracerColor, flDelay, flSpeed, iEntityIndex, iAttachIndex);

	R_AddPartSystem(pTracer);

	g_pCurTracer = (CBaseTracer *)pTracer;
}

void R_EmitTracer(vec3_t vecDst)
{
	if(g_pCurTracer)
		g_pCurTracer->EmitParticle(vecDst);
}

//impact

class CBulletImpact : public CPartSystemCoord
{
public:
	CBulletImpact(){}
	void Init(int type, int parts, float *org, float *vel, float *right, float *up)
	{
		CPartSystemCoord::Init(type, parts, 0, org, vel);
		if(right) VectorCopy(right, m_right);
		if(up) VectorCopy(up, m_up);
	}
public:
	vec3_t m_right;
	vec3_t m_up;
};

class CBulletImpactDebris : public CBulletImpact
{
public:
	CBulletImpactDebris(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CBulletImpact::Init(PS_BulletImpactDebris, parts, org, vel, right, up);
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

		ent.curstate.scale = p->scale * (0.3 + 0.7 * frac);
		ent.curstate.fuser1 = 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		ent.angles[2] = p->rot * frac2;

		R_DrawTGASprite(&ent, &g_texDebris[p->modn]);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-1, 1);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-1, 1);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(0, 1);
		VectorMA(p->vel, rnd, m_vel, p->vel);
		VectorNormalize(p->vel);
		float speed = RANDOM_FLOAT(96, 256);
		VectorScale(p->vel, speed);

		COLOR_RANDOM_LERP(56, 54, 44, 41, 38, 33)
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.04, 0.06);
		p->rot = RANDOM_FLOAT(-200, 200);
		p->life = RANDOM_FLOAT(0.1, 1.0);
		p->modn = RANDOM_LONG(0, 5);
		p->die = g_flClientTime + p->life;
	}
};

class CBulletImpactSpark : public CBulletImpact
{
public:
	CBulletImpactSpark(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CBulletImpact::Init(PS_BulletImpactSprak, parts, org, vel, right, up);
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
		float spd = VectorLength(p->vel);
		if(spd < 80)
			ent.curstate.fuser1 = 1;
		else
			ent.curstate.fuser1 = 12 * (spd / 80 - 1) + 1;
		ent.curstate.frame = 0;

		VectorCopy(org, ent.origin);
		VectorCopy(p->vel, ent.curstate.velocity);
		ent.angles[2] = 0;

		R_DrawTGATracer(&ent, &g_texBrightGlowY);

		//test
		ent.curstate.fuser1 = 1;
		ent.curstate.scale *= 1.5;
		VectorCopy(g_TracerEndPos, ent.origin);
		R_DrawTGASprite(&ent, &g_texBrightGlowY);
	}
	void AddParticle(void)
	{
		float rnd;

		part_t *p = AllocParticle();

		VectorCopy(m_org, p->org);

		rnd = RANDOM_FLOAT(-1, 1);
		VectorMA(p->vel, rnd, m_right, p->vel);
		rnd = RANDOM_FLOAT(-1, 1);
		VectorMA(p->vel, rnd, m_up, p->vel);
		rnd = RANDOM_FLOAT(0, 1);
		VectorMA(p->vel, rnd, m_vel, p->vel);
		VectorNormalize(p->vel);
		float speed = RANDOM_FLOAT(64, 144);
		VectorScale(p->vel, speed);

		COLOR_RANDOM_LERP(176, 106, 5, 253, 209, 1);
		p->col[3] = 255;
		p->scale = RANDOM_FLOAT(0.5, 1.5);
		p->life = RANDOM_FLOAT(0.1, 1.0);
		p->die = g_flClientTime + p->life;
	}
};

void R_BulletImpact(vec3_t vecStart, vec3_t vecNormal)
{
	int i;
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
	CParticleSystem *pCore = new CParticleSystem;
	pCore->Init(PS_BulletImpact, 0, 2);
	pCore->SetDie(1.5);

	CBulletImpactSpark *pSpark = new CBulletImpactSpark;
	pSpark->Init(8, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 8; ++i)
		pSpark->AddParticle();

	CBulletImpactDebris *pDebris = new CBulletImpactDebris;
	pDebris->Init(8, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 8; ++i)
		pDebris->AddParticle();

	pCore->AddChild(pSpark);
	pCore->AddChild(pDebris);

	R_AddPartSystem(pCore);
}