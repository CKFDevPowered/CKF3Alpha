#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"

cvar_t *chase_active;
cvar_t *r_drawentities;
cvar_t *volume;
cvar_t *r_traceglow;

cvar_t *g_x;
cvar_t *g_y;
cvar_t *g_w;
cvar_t *g_h;

cvar_t *g_crosshair_style;//for choosing crosshair style
cvar_t *g_crosshair_color;
cvar_t *g_crosshair_a;
cvar_t *g_crosshair_width;//for custom crosshair
cvar_t *g_crosshair_length;
cvar_t *g_crosshair_gap;
cvar_t *g_sniperdot_min;
cvar_t *g_sniperdot_max;
cvar_t *g_sniperdot_mindist;
cvar_t *g_sniperdot_maxdist;
cvar_t *g_3dmenu;
cvar_t *g_3dhud;
cvar_t *g_lod1;
cvar_t *g_lod2;
cvar_t *g_traillevel;
cvar_t *g_particle_debug;
cvar_t *g_particle_sort;
cvar_t *g_deathmsg_clean;
cvar_t *g_deathmsg_max;

cvar_t *gl_wireframe;
cvar_t *cl_righthand;
cvar_t *r_decals;

void Cvar_HudInit(void)
{
	r_drawentities = gEngfuncs.pfnGetCvarPointer("r_drawentities");
	volume = gEngfuncs.pfnGetCvarPointer("volume");
	r_traceglow = gEngfuncs.pfnGetCvarPointer("r_traceglow");
	gl_wireframe = gEngfuncs.pfnGetCvarPointer("gl_wireframe");
	cl_righthand = gEngfuncs.pfnGetCvarPointer("cl_righthand");
	r_decals = gEngfuncs.pfnGetCvarPointer("r_decals");
}

void Cvar_Init(void)
{
	g_x = gEngfuncs.pfnRegisterVariable("xx", "0", FCVAR_CLIENTDLL);
	g_y = gEngfuncs.pfnRegisterVariable("yy", "0", FCVAR_CLIENTDLL);
	g_w = gEngfuncs.pfnRegisterVariable("ww", "0", FCVAR_CLIENTDLL);
	g_h = gEngfuncs.pfnRegisterVariable("hh", "0", FCVAR_CLIENTDLL);

	g_crosshair_a = gEngfuncs.pfnRegisterVariable("ckf_crosshair_alpha", "255", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);//RGB & Alpha
	g_crosshair_color = gEngfuncs.pfnRegisterVariable("ckf_crosshair_color", "255 255 255", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_crosshair_width = gEngfuncs.pfnRegisterVariable("ckf_crosshair_width", "2", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);//custom crosshair
	g_crosshair_length = gEngfuncs.pfnRegisterVariable("ckf_crosshair_length", "10", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_crosshair_gap = gEngfuncs.pfnRegisterVariable("ckf_crosshair_gap", "10", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);//gaps between dot
	g_crosshair_style = gEngfuncs.pfnRegisterVariable("ckf_crosshair_type", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);//crosshair style
	g_lod1 = gEngfuncs.pfnRegisterVariable("ckf_lod_1", "400", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_lod2 = gEngfuncs.pfnRegisterVariable("ckf_lod_2", "800", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_3dmenu = gEngfuncs.pfnRegisterVariable("ckf_3dmenu", "2", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_3dhud = gEngfuncs.pfnRegisterVariable("ckf_3dhud", "2", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_traillevel = gEngfuncs.pfnRegisterVariable("ckf_traillevel", "3", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_sniperdot_min = gEngfuncs.pfnRegisterVariable("ckf_sniperdot_min", "0.2", FCVAR_CLIENTDLL);
	g_sniperdot_max = gEngfuncs.pfnRegisterVariable("ckf_sniperdot_max", "0.6", FCVAR_CLIENTDLL);
	g_sniperdot_mindist = gEngfuncs.pfnRegisterVariable("ckf_sniperdot_mindist", "150", FCVAR_CLIENTDLL);
	g_sniperdot_maxdist = gEngfuncs.pfnRegisterVariable("ckf_sniperdot_maxdist", "800", FCVAR_CLIENTDLL);
	g_particle_debug = gEngfuncs.pfnRegisterVariable("ckf_particle_debug", "0", FCVAR_CLIENTDLL);
	g_particle_sort = gEngfuncs.pfnRegisterVariable("ckf_particle_sort", "1", FCVAR_CLIENTDLL);
	g_deathmsg_clean = gEngfuncs.pfnRegisterVariable("ckf_deathmsg_clean", "10.0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
	g_deathmsg_max = gEngfuncs.pfnRegisterVariable("ckf_deathmsg_max", "5", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

	chase_active = gEngfuncs.pfnGetCvarPointer("chase_active");
}