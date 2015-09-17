#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"
#include "studio_util.h"
#include "weapon.h"
#include "tent.h"

static int g_texMask[2];
int g_texPlayerMan[2][9];

static cl_entity_t g_PlayerEntity;

static void UpdateModel(void)
{
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	g_PlayerEntity.model = pPlayer->model;
	g_PlayerEntity.index = pPlayer->index;
	g_PlayerEntity.curstate.skin = pPlayer->curstate.skin;
	g_PlayerEntity.curstate.weaponmodel = pPlayer->curstate.weaponmodel;
	g_PlayerEntity.curstate.scale = pPlayer->curstate.scale;
	if(g_Player.m_pActiveItem)
	{
		g_PlayerEntity.curstate.sequence = g_Player.pev.sequence;
	}
	else
	{
		g_PlayerEntity.curstate.sequence = 45;
	}
	g_PlayerEntity.curstate.gaitsequence = 1;

	g_PlayerEntity.curstate.rendermode = kRenderNormal;
	g_PlayerEntity.curstate.renderfx = kRenderFxNone;
	g_PlayerEntity.curstate.renderamt = 255;
}

static void RenderModel(int x, int y, int w, int h)
{
	if(g_PlayerEntity.model == NULL)
		return;

	cl_entity_t *pEnt = &g_PlayerEntity;

	gRefExports.R_PushRefDef();
	VectorClear(refdef->viewangles);
	VectorClear(refdef->vieworg);
	gRefExports.R_UpdateRefDef();
	VectorClear(pEnt->origin);
	VectorClear(pEnt->angles);

	pEnt->origin[0] = 200;
	pEnt->angles[1] = 180-30;

	pEnt->curstate.colormap = (g_iTeam == 1) ? (1 | (1<<8)) : (140 | (140<<8));

	cl_entity_t *pSaveEnt = *CurrentEntity;
	*CurrentEntity = pEnt;

	int centerX = x+w/2;
	int centerY = y+h/2;
	if(g_3dhud->value >= 2 && (g_RefSupportExt & r_ext_fbo))
	{
		int left = w/2 + ScreenHeight * 0.05;
		int top = h/2 + ScreenHeight * 0.05;

		gRefExports.R_BeginDrawTrianglesInHUD_FBO(centerX, centerY, left, top);
		g_StudioRenderer.StudioDrawPlayer_3DHUD();
		gRefExports.R_FinishDrawTrianglesInHUD();

		qglEnable(GL_SCISSOR_TEST);
		qglScissor(0, ScreenHeight -(y + h), ScreenWidth, ScreenHeight);

		if(g_RefSupportExt & r_ext_shader)
			gRefExports.R_BeginFXAA(ScreenWidth, ScreenHeight);

		gRefExports.R_Draw3DHUDQuad(centerX, centerY, left, top);

		if(g_RefSupportExt & r_ext_shader)
			gRefExports.ShaderAPI.GL_EndProgram();

		qglDisable(GL_SCISSOR_TEST);
	}
	else
	{
		int x2 = centerX-ScreenWidth/2;
		int y2 = centerY-ScreenHeight/2;

		qglEnable(GL_SCISSOR_TEST);
		qglScissor(0, ScreenHeight -(y + h), ScreenWidth, ScreenHeight);

		gRefExports.R_BeginDrawTrianglesInHUD_Direct(centerX, centerY);
		g_StudioRenderer.StudioDrawPlayer_3DHUD();
		gRefExports.R_FinishDrawTrianglesInHUD();

		qglDisable(GL_SCISSOR_TEST);
	}

	*CurrentEntity = pSaveEnt;
	gRefExports.R_PopRefDef();
}

void DrawCross(int x, int y, float health, float scale);

int HudClass_VidInit(void)
{
	g_texMask[0] = Surface_LoadTGA("resource\\tga\\mask_class_red");
	g_texMask[1] = Surface_LoadTGA("resource\\tga\\mask_class_blu");

	g_texPlayerMan[0][0] = Surface_LoadTGA("resource\\tga\\class_scout_red");
	g_texPlayerMan[0][1] = Surface_LoadTGA("resource\\tga\\class_heavy_red");
	g_texPlayerMan[0][2] = Surface_LoadTGA("resource\\tga\\class_soldier_red");
	g_texPlayerMan[0][3] = Surface_LoadTGA("resource\\tga\\class_pyro_red");
	g_texPlayerMan[0][4] = Surface_LoadTGA("resource\\tga\\class_sniper_red");
	g_texPlayerMan[0][5] = Surface_LoadTGA("resource\\tga\\class_medic_red");
	g_texPlayerMan[0][6] = Surface_LoadTGA("resource\\tga\\class_engineer_red");
	g_texPlayerMan[0][7] = Surface_LoadTGA("resource\\tga\\class_demoman_red");
	g_texPlayerMan[0][8] = Surface_LoadTGA("resource\\tga\\class_spy_red");

	g_texPlayerMan[1][0] = Surface_LoadTGA("resource\\tga\\class_scout_blu");
	g_texPlayerMan[1][1] = Surface_LoadTGA("resource\\tga\\class_heavy_blu");
	g_texPlayerMan[1][2] = Surface_LoadTGA("resource\\tga\\class_soldier_blu");
	g_texPlayerMan[1][3] = Surface_LoadTGA("resource\\tga\\class_pyro_blu");
	g_texPlayerMan[1][4] = Surface_LoadTGA("resource\\tga\\class_sniper_blu");
	g_texPlayerMan[1][5] = Surface_LoadTGA("resource\\tga\\class_medic_blu");
	g_texPlayerMan[1][6] = Surface_LoadTGA("resource\\tga\\class_engineer_blu");
	g_texPlayerMan[1][7] = Surface_LoadTGA("resource\\tga\\class_demoman_blu");
	g_texPlayerMan[1][8] = Surface_LoadTGA("resource\\tga\\class_spy_blu");

	CL_CreateTempEntity(&g_PlayerEntity, NULL);
	g_PlayerEntity.index = 0;
	g_PlayerEntity.player = 1;
	g_PlayerEntity.curstate.effects |= EF_3DMENU;
	g_PlayerEntity.curstate.iuser1 = 128;
	g_PlayerEntity.curstate.iuser2 = 192;
	g_PlayerEntity.curstate.vuser1[0] = 0;
	g_PlayerEntity.curstate.vuser1[1] = 0;

	return 1;
}

int HudClass_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if((g_iHideHUD & HIDEHUD_HEALTH))
		return 0;

	if(gEngfuncs.IsSpectateOnly())
		return 0;

	if(g_iClass <= 0 || g_iHealth <= 0)
		return 0;

	int maskx,masky, manx,many;
	int maskw, maskh, manw, manh;
	float maskscale, manscale;
	//cal xy
	maskx = 14*(ScreenHeight*4/3)/640;		//see hud_health.cpp
	masky = (480 - 59)*ScreenHeight/480;		
	manx = 25*(ScreenHeight*4/3)/640;
	many = (480 - 88)*ScreenHeight/480;

	int maskTexID = g_texMask[g_iTeam-1];
	int manTexID = g_texPlayerMan[g_iTeam-1][g_iClass-1];
	//cal scale
	g_pSurface->DrawGetTextureSize(maskTexID, maskw, maskh);
	g_pSurface->DrawGetTextureSize(manTexID, manw, manh);
	maskscale = (ScreenHeight*90.0f/maskw)/480;	
	manscale = (ScreenHeight*75.0f/manw)/480;

	g_pSurface->DrawSetColor(255, 255, 255, 255);
	g_pSurface->DrawSetTexture(maskTexID);
	g_pSurface->DrawTexturedRect(maskx, masky, maskx+maskw*maskscale, masky+maskh*maskscale);

	if(g_3dhud->value > 0)
	{
		UpdateModel();
		RenderModel(manx, many, manw*manscale, manh*manscale);
	}
	else
	{
		g_pSurface->DrawSetTexture(manTexID);
		g_pSurface->DrawTexturedRect(manx, many, manx+manw*manscale, many+manh*manscale);
	}

	return 1;
}