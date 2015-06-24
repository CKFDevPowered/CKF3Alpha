#include "hud.h"
#include "cl_util.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#include "tri.h"
#include "configs.h"

FogParameters g_FogParameters;

void RenderFog(void)
{
	FogParameters fog;
	int bOn;

	fog.color[0] = g_FogParameters.color[0];
	fog.color[1] = g_FogParameters.color[1];
	fog.color[2] = g_FogParameters.color[2];
	fog.density = g_FogParameters.density;
	fog.affectsSkyBox = g_FogParameters.affectsSkyBox;

	if (cl_fog_skybox)
		fog.affectsSkyBox = cl_fog_skybox->value;

	if (cl_fog_density)
		fog.density = cl_fog_density->value;

	if (cl_fog_r)
		fog.color[0] = cl_fog_r->value;

	if (cl_fog_g)
		fog.color[1] = cl_fog_g->value;

	if (cl_fog_b)
		fog.color[2] = cl_fog_b->value;

	if (g_iWaterLevel <= 1)
		bOn = fog.density > 0.0; 
	else
		bOn = false;

	gEngfuncs.pTriAPI->FogParams(fog.density, fog.affectsSkyBox);
	gEngfuncs.pTriAPI->Fog(fog.color, 100, 2000, bOn);
}

void HUD_DrawNormalTriangles(void)
{
	if(gCkfClientFuncs.HUD_DrawNormalTriangles)
		gCkfClientFuncs.HUD_DrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();
}

void HUD_DrawTransparentTriangles(void)
{
	gExportfuncs.HUD_DrawTransparentTriangles();

	if(gCkfClientFuncs.HUD_DrawTransparentTriangles)
		gCkfClientFuncs.HUD_DrawTransparentTriangles();

	if (gConfigs.bEnableClientUI)
		g_pViewPort->RenderMapSprite();

	RenderFog();
}