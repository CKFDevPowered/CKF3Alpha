#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "studio_util.h"
#include "particle.h"
#include "qgl.h"

void R_UpdateParticles(void);

cl_entity_t ent;

tgasprite_t g_texTracer[3];
tgasprite_t g_texFlamethrowerFire[5];
tgasprite_t g_texRocketSmoke[5];
tgasprite_t g_texMediBeam;
tgasprite_t g_texBrightGlowY;
tgasprite_t g_texSoftGlow;
tgasprite_t g_texSoftGlowTrans;
tgasprite_t g_texMultiJumpSmoke[5];
tgasprite_t g_texCircle1;
tgasprite_t g_texCircle2;
tgasprite_t g_texCircle4;
tgasprite_t g_texSingleFlame;
tgasprite_t g_texDebris[6];
tgasprite_t g_texStarFlash;
tgasprite_t g_texCritHit;
tgasprite_t g_texMiniCritHit;
tgasprite_t g_texElectric1;
tgasprite_t g_texConcNormal;

std::vector<vispart_t> g_visparts;
std::vector<CParticleSystem *> g_partsystems;

void R_LoadTGASprite(tgasprite_t *tga, const char *path, int xsplit, int ysplit, int startframe, int numframes)
{
	int i, x, y, width, height;
	int tex, w, h;
	tgasprite_frame_t *frame;

	if (xsplit < 1 || ysplit < 1 || startframe < 0 || numframes < 1)
	{
		gEngfuncs.Con_Printf("R_LoadTGASprite failed to load %s. (Invalid parameters)\n", path);
		tga->isValid = false;
		return;
	}
	if (xsplit * ysplit - startframe < numframes)
	{
		gEngfuncs.Con_Printf("R_LoadTGASprite failed to load %s. (Invalid frame)\n", path);
		tga->isValid = false;
		return;
	}

	width = 0;
	height = 0;

	tex = gRefExports.R_LoadTextureEx(path, path, &width, &height, GLT_SYSTEM, false, false);

	if (!tex || !width || !height)
	{
		gEngfuncs.Con_Printf("R_LoadTGASprite failed to load %s. (Invalid TGA file)\n", path);
		tga->isValid = false;
		return;
	}

	w = width / xsplit;
	h = height / ysplit;
	if (w <= 0 || h <= 0)
	{
		gEngfuncs.Con_Printf("R_LoadTGASprite failed to load %s. (Invalid frame size)\n", path);
		tga->isValid = false;
		return;
	}

	frame = new tgasprite_frame_t[numframes];
	for (i = 0; i < numframes; i++)
	{
		x = (i + startframe) % xsplit;
		y = (i + startframe) / ysplit;
		frame[i].texcoord[0][0] = (float)x / xsplit;
		frame[i].texcoord[0][1] = (float)(y + 1) / ysplit;
		frame[i].texcoord[1][0] = (float)x / xsplit;
		frame[i].texcoord[1][1] = (float)y / ysplit;
		frame[i].texcoord[2][0] = (float)(x + 1) / xsplit;
		frame[i].texcoord[2][1] = (float)y / ysplit;
		frame[i].texcoord[3][0] = (float)(x + 1) / xsplit;
		frame[i].texcoord[3][1] = (float)(y + 1) / ysplit;
	}
	tga->isValid = true;
	tga->tex = tex;
	tga->w = w;
	tga->h = h;
	tga->numframes = numframes;
	tga->frame = frame;
}

void R_Particles_VidInit(void)
{
	int i, j;
	if(!g_iHudVidInitalized)
	{
		R_LoadTGASprite(&g_texTracer[0], "resource/tga/tracer1new.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texTracer[1], "resource/tga/tracer2.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texTracer[2], "resource/tga/tracer3.tga", 1, 1, 0, 1);

		for(i = 0; i < 5; ++i)
			R_LoadTGASprite(&g_texRocketSmoke[i], "resource/tga/smokelit.tga", 4, 2, i,	1);

		for(i = 0; i < 5; ++i)
			R_LoadTGASprite(&g_texMultiJumpSmoke[i], "resource/tga/smoke2lit.tga", 8, 1, i, 1);

		R_LoadTGASprite(&g_texBrightGlowY, "resource/tga/brightglow_y.tga", 1, 1, 0, 1);

		for(i = 0, j = 0; i < 5; ++i)
		{
			R_LoadTGASprite(&g_texFlamethrowerFire[i], "resource/tga/flamethrowerfire102.dds", 10, 10, j, (i == 1) ? 21 : 19);
			j += (i == 1) ? 21 : 19;
		}

		R_LoadTGASprite(&g_texMediBeam, "resource/tga/medicbeam_curl.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texSoftGlow, "resource/tga/softglow.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texSoftGlowTrans, "resource/tga/softglow_translucent.tga", 1, 1, 0, 1);		
		R_LoadTGASprite(&g_texCircle1, "resource/tga/circle1.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texCircle2, "resource/tga/circle2.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texCircle4, "resource/tga/circle4.dds", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texSingleFlame, "resource/tga/singleflame.tga", 1, 1, 0, 1);

		for(i = 0; i < 6; ++i)
			R_LoadTGASprite(&g_texDebris[i], "resource/tga/debris_chunk.tga", 8, 1, i, 1);

		R_LoadTGASprite(&g_texStarFlash, "resource/tga/starflash01.tga", 1, 1, 0, 1);

		R_LoadTGASprite(&g_texCritHit, "resource/tga/crit.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texMiniCritHit, "resource/tga/minicrit.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texElectric1, "resource/tga/electric1.tga", 1, 1, 0, 1);
		R_LoadTGASprite(&g_texConcNormal, "resource/tga/conc_normal.tga", 1, 1, 0, 1);
	}

	g_partsystems.clear();
	g_partsystems.reserve(64);

	g_visparts.clear();
	g_visparts.reserve(1024);
}

CParticleSystem *R_FindPartSystem(PartSysEnum type, cl_entity_t *pEntity)
{
	int i, size;
	CParticleSystem *t;

	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->IsFollowEntity() && type == t->GetType())
		{
			if(((CPartSystemEntity *)t)->GetFollowEntity() == pEntity)
			{
				return t;
			}
		}
	}
	return NULL;
}

void R_AddPartSystem(CParticleSystem *t)
{
	g_partsystems.push_back(t);
}

void R_KillPartSystem(cl_entity_t *pEntity, int instant)
{
	int i, size;
	CParticleSystem *t;

	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->IsFollowEntity())
		{
			if(((CPartSystemEntity *)t)->GetFollowEntity() == pEntity)
			{
				t->SetDead(instant ? 2 : 1);
			}
		}
	}
}

void R_KillAllEntityPartSystem(int instant)
{
	int i, size;
	CParticleSystem *t;

	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->IsFollowEntity())
		{
			t->SetDead(instant ? 2 : 1);
		}
	}
}

void R_AddVisParticles(CParticleSystem *t, part_t *p)
{
	vispart_t vp;

	vp.part = p;
	vp.partsys = t;
	vp.discard = 0;
	t->Movement(p, vp.org);

	g_visparts.push_back(vp);
}

bool R_SortVisParticle(const vispart_t &v1, const vispart_t &v2)
{
	return v1.dist > v2.dist;
}

void R_DrawParticles(void)
{
	vispart_t *v;
	vec3_t vdist;
	int i, size;

	size = g_visparts.size();

	for(i = 0; i < size; ++i)
	{
		v = &g_visparts[i];
		if(v->partsys->GetCull() && gRefExports.RefAPI.R_CullBox(v->org, v->org))
		{
			v->discard = 1;
		}
		else
		{
			VectorSubtract(v->org, refdef->vieworg, vdist);
			v->dist = VectorLength(vdist);
			v->discard = 0;
		}
	}

	if(size && g_particle_sort->value)
	{
		if(g_particle_debug->value)
		{
			double start = cl_pmove->Sys_FloatTime();
			std::sort(g_visparts.begin(), g_visparts.end(), R_SortVisParticle);
			double end = cl_pmove->Sys_FloatTime();
			gEngfuncs.Con_Printf("%d particles sorted, %.2f msec used\n", size, (end-start)*1000 );
		}
		else
		{
			std::sort(g_visparts.begin(), g_visparts.end(), R_SortVisParticle);
		}
	}

	for(i = 0; i < size; ++i)
	{
		v = &g_visparts[i];

		if(!v->discard)
		{
			v->partsys->Render(v->part, v->org);
		}
	}
}

int CParticleSystem::GetActivePartCount(void)
{
	int size = m_part.size();
	int count = 0;
	for(int i = 0; i < size; ++i)
	{
		if(!m_part[i].free)
			++ count;
	}
	return count;
}

void CParticleSystem::KillAllParticles(void)
{
	int size = m_part.size();
	for(int i = 0; i < size; ++i)
	{
		m_part[i].die = 0;
	}
}

void CParticleSystem::Init(int type, int numpart, int numchild)
{
	m_type = type;
	m_part.reserve(numpart);
	m_child.reserve(numchild);

	m_dead = 0;
	m_die = g_flClientTime + 99999;
	m_cull = true;
	m_parent = NULL;
}

part_t *CParticleSystem::AllocParticle(void)
{
	int i, size;
	part_t *pp;
	part_t p;

	size = m_part.size();
	for(i = 0; i < size; ++i)
	{
		pp = &m_part[i];
		if(pp->free)
		{
			memset(pp, 0, sizeof(part_t));
			pp->free = 0;
			return pp;
		}
	}

	memset(&p, 0, sizeof(part_t));
	p.free = 0;
	m_part.push_back(p);
	return &m_part.back();
}

void CParticleSystem::UpdateParticles(void)
{
	int i, size;
	part_t *p;

	Update();
	size = m_part.size();
	if(size)
	{
		for(i = 0; i < size; ++i)
		{
			p = &m_part[i];

			if(!p->free && g_flClientTime > p->die)
				p->free = true;

			if(!p->free)
				R_AddVisParticles(this, p);
		}
	}

	if(!m_dead && g_flClientTime > m_die)//mark for dead
	{
		SetDead(1);
	}
	else if(m_dead == 1 && !GetActivePartCount())//ready for delete
	{
		SetDead(2);
	}
}

void R_RecursiveUpdateParticles(CParticleSystem *partsystem)
{
	CParticleSystem *t;

	std::vector<CParticleSystem *> &v = (partsystem) ? partsystem->m_child : g_partsystems;

	if(v.empty())
		return;

	std::vector<CParticleSystem *>::iterator &it = v.begin();
	std::vector<CParticleSystem *>::iterator &end = v.end();
	while(it != end)
	{
		t = (*it);
		if(t->GetDead() >= 2 && !partsystem)
		{
			delete t;
			it = v.erase(it);
			end = v.end();
			continue;
		}
		if(t->GetDead() < 2)
		{
			t->UpdateParticles();
			R_RecursiveUpdateParticles(t);
		}
		++it;
	}
}

void R_UpdateParticles(void)
{
	g_visparts.clear();

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

	if(g_particle_debug->value)
	{
		double start = cl_pmove->Sys_FloatTime();
		R_RecursiveUpdateParticles(NULL);
		double end = cl_pmove->Sys_FloatTime();
		gEngfuncs.Con_Printf("%d vis particles added, %f msec used.\n", g_visparts.size(), (end-start)*1000);
	}
	else
	{
		R_RecursiveUpdateParticles(NULL);
	}
}