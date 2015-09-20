#include <metahook.h>
#include <IBTEClient.h>
#include "exportfuncs.h"
#include "gl_local.h"
#include "screen.h"
#include "command.h"
#include "parsemsg.h"
#include "qgl.h"

//Error when can't find sig
void Sys_ErrorEx(const char *fmt, ...)
{
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if(g_pMetaSave->pEngineFuncs)
		g_pMetaSave->pEngineFuncs->pfnClientCmd("escape\n");
	if(g_pBTEClient)
		MessageBox((g_dwEngineBuildnum >= 5953) ? NULL : g_pBTEClient->GetMainHWND(), msg, "Error", MB_ICONERROR);
	else
		MessageBox(NULL, msg, "Error", MB_ICONERROR);
	exit(0);
}

int Q_stricmp_slash(const char *s1, const char *s2)
{
	int c1, c2;

	while (1)
	{
		c1 = *s1++;
		c2 = *s2++;

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');

			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');

			if (c1 == '\\')
				c1 = '/';

			if (c2 == '\\')
				c2 = '/';

			if (c1 != c2)
				return -1;
		}

		if (!c1)
			return 0;
	}
	return -1;
}

char *UTIL_VarArgs(char *format, ...)
{
	va_list argptr;
	static int index = 0;
	static char string[16][1024];

	va_start(argptr, format);
	vsprintf(string[index], format, argptr);
	va_end(argptr);

	char *result = string[index];
	index = (index + 1) % 16;
	return result;
}

#define LIB_NOT_FOUND(name) Sys_ErrorEx("Couldn't load: %s", name);

IBTEClient *g_pBTEClient = NULL;

void BTE_Init(void)
{
	HINTERFACEMODULE hBTEClient = (HINTERFACEMODULE)GetModuleHandle("CSBTE.dll");
	//if(!hBTEClient)
	//	LIB_NOT_FOUND("CSBTE.dll");
	if(hBTEClient)
		g_pBTEClient = (IBTEClient *)((CreateInterfaceFn)Sys_GetFactory(hBTEClient))(BTECLIENT_API_VERSION, NULL);
}

cl_exportfuncs_t gClientfuncs =
{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	V_CalcRefdef,
	HUD_AddEntity,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	HUD_Shutdown,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	HUD_Frame,
	NULL,
	HUD_TempEntUpdate,
	NULL,
	NULL,
	NULL,
	HUD_GetStudioModelInterface,
	NULL,
	NULL,
	NULL
};

cl_enginefunc_t gEngfuncs;
engine_studio_api_t IEngineStudio;
r_studio_interface_t StudioInterface;
r_studio_interface_t *gpStudioInterface;

double g_flFrameTime = 0;

shaderapi_t ShaderAPI = 
{
	R_CompileShader,
	GL_UseProgram,
	GL_EndProgram,
	GL_GetUniformLoc,
	GL_GetAttribLoc,
	GL_Uniform1i,
	GL_Uniform2i,
	GL_Uniform3i,
	GL_Uniform4i,
	GL_Uniform1f,
	GL_Uniform2f,
	GL_Uniform3f,
	GL_Uniform4f,
	GL_VertexAttrib3f,
	GL_VertexAttrib3fv,
	GL_MultiTexCoord2f,
	GL_MultiTexCoord3f
};

engrefapi_t RefAPI = 
{
	GL_Bind,
	GL_SelectTexture,
	GL_DisableMultitexture,
	GL_EnableMultitexture,
	R_DrawBrushModel,
	R_DrawSpriteModel,
	R_GetSpriteAxes,
	R_SpriteColor,
	GlowBlend,
	CL_FxBlend,
	R_CullBox,
	GL_SwapBuffer
};

ref_export_t gRefExports =
{
	//common
	R_GetDrawPass,
	R_GetSupportExtension,
	//water
	R_SetWaterParm,
	//studio
	R_GLStudioDrawPointsEx,
	R_GetPlayerState,
	//refdef
	R_PushRefDef,
	R_UpdateRefDef,
	R_PopRefDef,
	R_GetSavedViewOrg,
	R_GetRefDef,
	//shadow
	R_CreateShadowLight,
	//texture
	R_GLGenTexture,
	R_GetTexLoaderBuffer,
	R_LoadTextureEx,
	GL_LoadTextureEx,
	R_GetCurrentGLTexture,
	GL_UploadDXT,
	LoadBMP,
	LoadTGA,
	LoadPNG,
	LoadDDS,
	LoadJPEG,
	SaveBMP,
	SaveTGA,
	SavePNG,
	SaveJPEG,
	//capture screen
	R_GetSCRCaptureBuffer,
	//3dsky
	R_Add3DSkyEntity,
	R_Setup3DSkyModel,
	R_Finish3DSkyModel,
	//2d postprocess
	R_BeginFXAA,
	R_BeginDrawRoundRect,
	R_BeginDrawHudMask,
	//cloak
	R_RenderCloakTexture,
	//3dhud
	R_Get3DHUDTexture,
	R_Draw3DHUDQuad,
	R_BeginDrawTrianglesInHUD_Direct,
	R_BeginDrawTrianglesInHUD_FBO,
	R_FinishDrawTrianglesInHUD,
	R_BeginDrawHUDInWorld,
	R_FinishDrawHUDInWorld,
	//shader
	ShaderAPI,
	RefAPI
};

RECT *VID_GetWindowRect(void)
{
	return window_rect;
}

void hudGetMousePos(struct tagPOINT *ppt)
{
	g_pMetaSave->pEngineFuncs->pfnGetMousePos(ppt);

	if ( !g_bWindowed && g_pBTEClient )
	{
		RECT rectWin;
		GetWindowRect(g_pBTEClient->GetMainHWND(), &rectWin);
		int videoW = g_iVideoWidth;
		int videoH = g_iVideoHeight;
		int winW = rectWin.right - rectWin.left;
		int winH = rectWin.bottom - rectWin.top;
		ppt->x *= (float)videoW / winW;
		ppt->y *= (float)videoH / winH;
		ppt->x *= (windowvideoaspect_old - 1) * (ppt->x - videoW / 2);
		ppt->y *= (videowindowaspect_old - 1) * (ppt->y - videoH / 2);
	}
}

void hudGetMousePosition(int *x, int *y)
{
	g_pMetaSave->pEngineFuncs->GetMousePosition(x, y);

	if ( !g_bWindowed && g_pBTEClient )
	{
		RECT rectWin;
		GetWindowRect(g_pBTEClient->GetMainHWND(), &rectWin);
		int videoW = g_iVideoWidth;
		int videoH = g_iVideoHeight;
		int winW = rectWin.right - rectWin.left;
		int winH = rectWin.bottom - rectWin.top;
		*x *= (float)videoW / winW;
		*y *= (float)videoH / winH;
		*x *= (windowvideoaspect_old - 1) * (*x - videoW / 2);
		*y *= (videowindowaspect_old - 1) * (*y - videoH / 2);
	}
}

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	if(g_dwEngineBuildnum < 5953)
	{
		pEnginefuncs->pfnGetMousePos = hudGetMousePos;
		pEnginefuncs->GetMousePosition = hudGetMousePosition;
	}

	Cmd_GetCmdBase = *(cmd_function_t *(**)(void))((DWORD)pEnginefuncs + 0x198);

	return 1;
}

#define METARENDER_SKYCAMERA 1
#define METARENDER_SHADOWMGR 2

int MsgFunc_MetaRender(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int type = READ_BYTE();
	if(type == METARENDER_SKYCAMERA)
	{
		_3dsky_camera[0] = READ_COORD();
		_3dsky_camera[1] = READ_COORD();
		_3dsky_camera[2] = READ_COORD();
		_3dsky_center[0] = READ_COORD();
		_3dsky_center[1] = READ_COORD();
		_3dsky_center[2] = READ_COORD();
		char *model = READ_STRING();
		if(model && model[0])
		{
			model_t *mod = IEngineStudio.Mod_ForName(model, 0);
			if(mod)
			{
				VectorCopy(mod->mins, _3dsky_mins);
				VectorCopy(mod->maxs, _3dsky_maxs);
			}
		}
		_3dsky_enable = true;
	}
	else if(type == METARENDER_SHADOWMGR)
	{
		vec3_t angles;
		angles[0] = READ_COORD();
		angles[1] = READ_COORD();
		angles[2] = READ_COORD();
		float radius = READ_COORD();
		float fard = READ_COORD();
		float scale = READ_COORD();
		int texsize = READ_SHORT();
		char *affectmodel = READ_STRING();
		R_CreateShadowManager(affectmodel, angles, radius, fard, scale, texsize);
	}
	
	return 1;
}

void HUD_Init(void)
{
	SCR_Init();
	GL_Init();
	R_Init();

	g_pMetaSave->pEngineFuncs->pfnHookUserMsg("MetaRender", MsgFunc_MetaRender);

	//cvar registered in client.dll HUD_Init();
	cl_righthand = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("cl_righthand");
}

int HUD_VidInit(void)
{
	SCR_VidInit();
	R_VidInit();

	return 1;
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	R_CalcRefdef(pparams);
}

void HUD_DrawNormalTriangles(void)
{
	gExportfuncs.HUD_DrawNormalTriangles();
}

void HUD_DrawTransparentTriangles(void)
{
	gExportfuncs.HUD_DrawTransparentTriangles();	
}

int HUD_Redraw(float time, int intermission)
{
	if(waters_active && r_water_debug->value > 0 && r_water_debug->value <= 2)
	{
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
		qglColor4f(1,1,1,1);

		qglEnable(GL_TEXTURE_2D);
		switch((int)r_water_debug->value)
		{
		case 1:
			qglBindTexture(GL_TEXTURE_2D, waters_active->reflectmap);
			qglBegin(GL_QUADS);
			qglTexCoord2f(0,1);
			qglVertex3f(0,0,0);
			qglTexCoord2f(1,1);
			qglVertex3f(glwidth/2,0,0);
			qglTexCoord2f(1,0);
			qglVertex3f(glwidth/2,glheight/2,0);
			qglTexCoord2f(0,0);
			qglVertex3f(0,glheight/2,0);
			qglEnd();
			break;
		case 2:
			qglBindTexture(GL_TEXTURE_2D, waters_active->refractmap);
			qglBegin(GL_QUADS);
			qglTexCoord2f(0,1);
			qglVertex3f(0,0,0);
			qglTexCoord2f(1,1);
			qglVertex3f(glwidth/2,0,0);
			qglTexCoord2f(1,0);
			qglVertex3f(glwidth/2,glheight/2,0);
			qglTexCoord2f(0,0);
			qglVertex3f(0,glheight/2,0);
			qglEnd();
			break;
		default:
			break;
		}
	}
	else if(sdlights_active && r_shadow_debug->value)
	{
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
		qglColor4f(1,1,1,1);

		qglEnable(GL_TEXTURE_2D);
		qglBindTexture(GL_TEXTURE_2D, sdlights_active->depthmap);

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,1);
		qglVertex3f(0,0,0);
		qglTexCoord2f(1,1);
		qglVertex3f(glwidth/2,0,0);
		qglTexCoord2f(1,0);
		qglVertex3f(glwidth/2,glheight/2,0);
		qglTexCoord2f(0,0);
		qglVertex3f(0,glheight/2,0);
		qglEnd();
		qglEnable(GL_ALPHA_TEST);
	}
	else if(r_cloak_debug->value)
	{
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
		qglColor4f(1,1,1,1);

		qglEnable(GL_TEXTURE_2D);
		qglBindTexture(GL_TEXTURE_2D, cloak_texture);

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,1);
		qglVertex3f(0,0,0);
		qglTexCoord2f(1,1);
		qglVertex3f(glwidth/2,0,0);
		qglTexCoord2f(1,0);
		qglVertex3f(glwidth/2,glheight/2,0);
		qglTexCoord2f(0,0);
		qglVertex3f(0,glheight/2,0);
		qglEnd();
		qglEnable(GL_ALPHA_TEST);
	}
	else if(r_hudinworld_debug->value && r_hudinworld_texture)
	{
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
		qglColor4f(1,1,1,1);

		qglEnable(GL_TEXTURE_2D);
		qglBindTexture(GL_TEXTURE_2D, r_hudinworld_texture);

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,1);
		qglVertex3f(0,0,0);
		qglTexCoord2f(1,1);
		qglVertex3f(glwidth/2,0,0);
		qglTexCoord2f(1,0);
		qglVertex3f(glwidth/2,glheight/2,0);
		qglTexCoord2f(0,0);
		qglVertex3f(0,glheight/2,0);
		qglEnd();
		qglEnable(GL_ALPHA_TEST);
	}
	return 1;
}

extern float (*pbonetransform)[MAXSTUDIOBONES][3][4];

int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio)
{
	DWORD addr;
	//Save FuncsTable
	gStudioFuncs.studioapi_StudioDrawPoints = pstudio->StudioDrawPoints;
	gStudioFuncs.studioapi_StudioSetupLighting = pstudio->StudioSetupLighting;
	gStudioFuncs.studioapi_SetupRenderer = pstudio->SetupRenderer;
	gStudioFuncs.studioapi_RestoreRenderer = pstudio->RestoreRenderer;

	//Vars in Engine Studio API
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->GetCurrentEntity, 0x10, "\xA1", 1);
	if(!addr)
		SIG_NOT_FOUND("currententity");
	currententity = *(cl_entity_t ***)(addr + 0x1);

	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->SetRenderModel, 0x10, "\xA3", 1);
	if(!addr)
		SIG_NOT_FOUND("r_model");
	r_model = *(model_t ***)(addr + 1);

	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->StudioSetHeader, 0x10, "\xA3", 1);
	if(!addr)
		SIG_NOT_FOUND("pstudiohdr");
	pstudiohdr = *(studiohdr_t ***)(addr + 1);

	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->SetForceFaceFlags, 0x10, "\xA3", 1);
	if(!addr)
		SIG_NOT_FOUND("g_ForcedFaceFlags");
	g_ForcedFaceFlags = *(int **)(addr + 1);
	
	//call	CL_FxBlend
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->StudioSetRenderamt, 0x50, "\xE8", 1);
	if(!addr)
		SIG_NOT_FOUND("CL_FxBlend");
	gRefFuncs.CL_FxBlend = (int (*)(cl_entity_t *))GetCallAddress(addr);

	//fstp    r_blend
	addr =(DWORD)g_pMetaHookAPI->SearchPattern((void *)pstudio->StudioSetRenderamt, 0x50, "\xD9\x1D", sizeof("\xD9\x1D")-1);
	if(!addr)
		SIG_NOT_FOUND("r_blend");
	r_blend = *(float **)(addr + 2);

	pbonetransform = (float (*)[MAXSTUDIOBONES][3][4])pstudio->StudioGetBoneTransform();
	plighttransform = (float (*)[MAXSTUDIOBONES][3][4])pstudio->StudioGetLightTransform();

	cl_viewent = g_pMetaSave->pEngineFuncs->GetViewModel();

	//Save Studio API
	memcpy(&IEngineStudio, pstudio, sizeof(IEngineStudio));
	memcpy(&StudioInterface, *ppinterface, sizeof(r_studio_interface_t));
	gpStudioInterface = *ppinterface;

	//InlineHook StudioAPI
	g_pMetaHookAPI->InlineHook(gStudioFuncs.studioapi_StudioDrawPoints, studioapi_StudioDrawPoints, (void *&)gStudioFuncs.studioapi_StudioDrawPoints);
	g_pMetaHookAPI->InlineHook(gStudioFuncs.studioapi_StudioSetupLighting, studioapi_StudioSetupLighting, (void *&)gStudioFuncs.studioapi_StudioSetupLighting);	
	g_pMetaHookAPI->InlineHook(gStudioFuncs.studioapi_SetupRenderer, studioapi_SetupRenderer, (void *&)gStudioFuncs.studioapi_SetupRenderer);
	g_pMetaHookAPI->InlineHook(gStudioFuncs.studioapi_RestoreRenderer, studioapi_RestoreRenderer, (void *&)gStudioFuncs.studioapi_RestoreRenderer);

	R_InitDetailTextures();
	R_LoadExtraTextureFile(false);
	R_LoadStudioTextures(false);

	return 1;
}

int HUD_UpdateClientData(client_data_t *pcldata, float flTime)
{
	scr_fov_value = pcldata->fov;
	return 1;
}

void HUD_Shutdown(void)
{
	R_Shutdown();
}

int HUD_AddEntity(int type, cl_entity_t *ent, const char *model)
{
	if(r_shadow->value && shadow_program && (ent->curstate.effects & EF_SHADOW))
	{
		R_AddEntityShadow(ent, model);
	}
	if(_3dsky_enable)
	{
		if(ent->curstate.origin[0] + ent->curstate.maxs[0] > _3dsky_mins[0] && 
			ent->curstate.origin[1] + ent->curstate.maxs[1] > _3dsky_mins[1] &&
			ent->curstate.origin[2] + ent->curstate.maxs[2] > _3dsky_mins[2] && 
			ent->curstate.origin[0] + ent->curstate.mins[0] < _3dsky_maxs[0] && 
			ent->curstate.origin[1] + ent->curstate.mins[1] < _3dsky_maxs[1] && 
			ent->curstate.origin[2] + ent->curstate.mins[2] < _3dsky_maxs[2])
		{
			if(!r_3dsky->value)
				return 0;

			R_Add3DSkyEntity(ent);
		}
	}
	return 1;
}

void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int (*pfnAddVisibleEntity)(cl_entity_t *),	void (*pfnTempEntPlaySound)( TEMPENTITY *, float damp))
{
	g_flFrameTime = frametime;
}

void HUD_Frame(double time)
{
	for(shadowlight_t *sl = sdlights_active; sl; sl = sl->next)
	{
		sl->free = true;
	}
}