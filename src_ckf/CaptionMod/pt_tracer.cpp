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
//tracer

class CPSTracer : public CPartSystemAttachment
{
public:
	CPSTracer(){}
	void Init(int type, int parts, int team, cl_entity_t *entity, int attachindex, float delay)
	{
		CPartSystemAttachment::Init(type, parts, 0, entity);
		SetDie(0);//default life
		m_team = team;
		m_cull = false;
		m_sort = false;
		m_attachindex = attachindex;

		m_curdelay = 0;
		m_delay = delay;
	}
	virtual void Render(part_t *p, float *org)
	{
		vec3_t	linedir, viewdir, cross;
		vec3_t	start, end, dir;
		float sd, ed, td;
		float width = 1.25f;

		if(m_entity)
			VectorCopy(m_attachment[m_attachindex], p->org);

		if(p->modn != 0)
			width = 1.75f;

		float frac;
		vec3_t	tmp;

		// calculate distance
		VectorCopy(p->vel, dir);

		// calculate fraction
		frac = p->life - ( p->die - g_flClientTime ) + g_flFrameTime;

		if(frac <= g_flFrameTime)//delayed
			return;

		// calculate our distance along our path
		sd = p->scale * frac;
		ed = sd - p->rot;

		// clip to start
		sd = max( 0.0f, sd );
		ed = max( 0.0f, ed );

		if(( sd == 0.0f ) && ( ed == 0.0f ))
			return;

		// clip it
		td = p->rot;
		if( td != 0.0f )
		{
			sd = min( sd, td );
			ed = min( ed, td );
		}

		VectorCopy(p->org, start);

		VectorMA(start, sd, dir, end );
		VectorMA(start, ed, dir, start );

		VectorSubtract( end, start, linedir );
		VectorSubtract( end, refdef->vieworg, viewdir );

		CrossProduct( linedir, viewdir, cross );
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

		int cullOff = (gRefExports.R_GetDrawPass() == r_draw_reflect);

		if(cullOff)
			qglDisable(GL_CULL_FACE);

		qglBegin( GL_QUADS );
		VectorMA( start, -width, cross, tmp );
		qglTexCoord2f( 0.0f, 0.0f );
		qglVertex3fv( tmp );
		VectorMA( start, width, cross, tmp );
		qglTexCoord2f( 1.0f, 0.0f );
		qglVertex3fv( tmp );
		VectorMA( end, width, cross, tmp );
		qglTexCoord2f( 1.0f, 1.0f );
		qglVertex3fv( tmp );
		VectorMA( end, -width, cross, tmp );
		qglTexCoord2f( 0.0f, 1.0f );
		qglVertex3fv( tmp );
		qglEnd();

		if(cullOff)
			qglEnable(GL_CULL_FACE);

		if(gl_wireframe->value != 0)
		{
			qglColor4f(0,1,0,1);
			qglDisable(GL_TEXTURE_2D);
			qglBegin(GL_LINE_LOOP);
			VectorMA( start, -width, cross, tmp );
			qglTexCoord2f( 0.0f, 0.0f );
			qglVertex3fv( tmp );
			VectorMA( start, width, cross, tmp );
			qglTexCoord2f( 1.0f, 0.0f );
			qglVertex3fv( tmp );
			VectorMA( end, width, cross, tmp );
			qglTexCoord2f( 1.0f, 1.0f );
			qglVertex3fv( tmp );
			VectorMA( end, -width, cross, tmp );
			qglTexCoord2f( 0.0f, 1.0f );
			qglVertex3fv( tmp );
			qglEnd();
			qglEnable(GL_TEXTURE_2D);
		}

		qglDepthMask(1);
	}
	void AddParticle(float *src, float *dst)
	{
		vec3_t dir;

		part_t *p = AllocParticle();
		if(!p) return;

		VectorSubtract(dst, src, dir);

		p->rot = VectorLength(dir);//length
		p->scale = min(max(p->rot * 10.0f, 2000), 12000);
		p->life = p->rot / p->scale;

		if(m_die < g_flClientTime + p->life + 0.5)
			SetDie(p->life + 0.5);//make it longer

		VectorCopy(src, p->org);
		VectorNormalize(dir);
		VectorCopy(dir, p->vel);

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
		p->modn = m_team;
		p->die = p->life + g_flClientTime + m_curdelay;
		m_curdelay += m_delay;
	}
public:
	int m_team;
	int m_attachindex;
	float m_delay;
	float m_curdelay;
};

static CPSTracer *g_pCurTracer;

void R_BeginTracer(int iTracerColor, int iNumTracer)
{
	g_pCurTracer = new CPSTracer;
	g_pCurTracer->Init(PS_DefaultTracer, iNumTracer, iTracerColor, NULL, 0, 0);

	R_AddPartSystem(g_pCurTracer);
}

void R_BeginTracerDelayed(int iTracerColor, int iNumTracer, float flDelay)
{
	g_pCurTracer = new CPSTracer;
	g_pCurTracer->Init(PS_DefaultTracer, iNumTracer, iTracerColor, NULL, 0, flDelay);

	R_AddPartSystem(g_pCurTracer);
}

void R_BeginTracerAttachment(int iTracerColor, int iNumTracer, cl_entity_t *pEntity, int iAttachIndex)
{
	g_pCurTracer = new CPSTracer;
	g_pCurTracer->Init(PS_DefaultTracer, iNumTracer, iTracerColor, pEntity, iAttachIndex, 0);

	R_AddPartSystem(g_pCurTracer);
}

void R_EmitTracer(vec3_t vecSrc, vec3_t vecDst)
{
	if(g_pCurTracer)
	{
		g_pCurTracer->AddParticle(vecSrc, vecDst);
	}
}

//impact

class CPSBulletImpact : public CPartSystemCoord
{
public:
	CPSBulletImpact(){}
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

class CPSBulletImpactDebris : public CPSBulletImpact
{
public:
	CPSBulletImpactDebris(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSBulletImpact::Init(PS_BulletImpactDebris, parts, org, vel, right, up);
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

class CPSBulletImpactSpark : public CPSBulletImpact
{
public:
	CPSBulletImpactSpark(){}
	void Init(int parts, float *org, float *vel, float *right, float *up)
	{
		CPSBulletImpact::Init(PS_BulletImpactSprak, parts, org, vel, right, up);
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
		if(!p) return;

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

	CPSBulletImpactSpark *pSpark = new CPSBulletImpactSpark;
	pSpark->Init(8, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 8; ++i)
		pSpark->AddParticle();

	CPSBulletImpactDebris *pDebris = new CPSBulletImpactDebris;
	pDebris->Init(8, vecStart, vecNormal, vecRight, vecUp);
	for(i = 0; i < 8; ++i)
		pDebris->AddParticle();

	pCore->AddChild(pSpark);
	pCore->AddChild(pDebris);

	R_AddPartSystem(pCore);
}