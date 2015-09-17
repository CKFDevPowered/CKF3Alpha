#pragma once

#include "enginedef.h"

#define MAX_GAUSSIAN_SAMPLES 16
#define LUMIN1x1_BUFFERS 3
#define DOWNSAMPLE_BUFFERS 2
#define LUMIN_BUFFERS 4
#define BLUR_BUFFERS 2

SHADER_UNIFORM_BEGINDEF(hud_drawhudmask)
{
	int base;
	int src_col;
}
SHADER_UNIFORM_ENDDEF(hud_drawhudmask)

SHADER_UNIFORM_BEGINDEF(hud_drawcolormask)
{
	int base;
	int alpha_range;
	int offset;
}
SHADER_UNIFORM_ENDDEF(hud_drawcolormask)

SHADER_UNIFORM_BEGINDEF(hud_drawroundrect)
{
	int base;
	int center;
	int radius;
	int blurdist;
}
SHADER_UNIFORM_ENDDEF(hud_drawroundrect)

SHADER_UNIFORM_BEGINDEF(pp_fxaa)
{
	int tex0;
	int rt_w;
	int rt_h;
}
SHADER_UNIFORM_ENDDEF(pp_fxaa)

SHADER_UNIFORM_BEGINDEF(pp_downsample)
{
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_downsample)

SHADER_UNIFORM_BEGINDEF(pp_downsample2x2)
{
	int texelsize;
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_downsample2x2)

SHADER_UNIFORM_BEGINDEF(pp_lumin)
{
	int texelsize;
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_lumin)

SHADER_UNIFORM_BEGINDEF(pp_luminlog)
{
	int texelsize;
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_luminlog)

SHADER_UNIFORM_BEGINDEF(pp_luminexp)
{
	int texelsize;
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_luminexp)

SHADER_UNIFORM_BEGINDEF(pp_luminadapt)
{
	int curtex;
	int adatex;
	int frametime;
}
SHADER_UNIFORM_ENDDEF(pp_luminadapt)

SHADER_UNIFORM_BEGINDEF(pp_gaussianblur_h)
{
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_gaussianblur_h)

SHADER_UNIFORM_BEGINDEF(pp_gaussianblur_v)
{
	int tex;
}
SHADER_UNIFORM_ENDDEF(pp_gaussianblur_v)

SHADER_UNIFORM_BEGINDEF(pp_tonemap)
{
	int basetex;
	int blurtex;
	int lumtex;
	int blurfactor;
	int exposure;
	int darkness;
	int contrast;
}
SHADER_UNIFORM_ENDDEF(pp_tonemap)

extern qboolean drawhudinworld;
extern qboolean draw3dhud;
extern int pp_fxaa_program;
extern cvar_t *r_hdr;
extern cvar_t *r_hudinworld_debug;
extern int r_hudinworld_texture;

void R_InitBlur(const char *vs_code, int samples);
void R_BeginHUDQuad(void);

void R_BeginFXAA(int w, int h);
void R_BeginDrawRoundRect(int centerX, int centerY, float radius, float blurdist);
void R_BeginDrawHudMask(int r, int g, int b);
void R_EndProgram(void);

int R_Get3DHUDTexture(void);
void R_Draw3DHUDQuad(int x, int y, int left, int top);
void R_BeginDrawTrianglesInHUD_Direct(int x, int y);
void R_BeginDrawTrianglesInHUD_FBO(int x, int y, int left, int top);
void R_FinishDrawTrianglesInHUD(void);
void R_BeginDrawHUDInWorld(int texid, int w, int h);
void R_FinishDrawHUDInWorld(void);

void R_DownSample(FBO_Container_t *src, FBO_Container_t *dst, qboolean filter2x2);
void R_LuminPass(FBO_Container_t *src, FBO_Container_t *dst, int logexp);
void R_LuminAdaptation(FBO_Container_t *src, FBO_Container_t *dst, FBO_Container_t *ada, double frametime);
void R_PartialBlur(FBO_Container_t *src, FBO_Container_t *dst, qboolean vertical);
void R_ToneMapping(FBO_Container_t *src, FBO_Container_t *blur, FBO_Container_t *lum, FBO_Container_t *dst);
void R_BlitToScreen(FBO_Container_t *src);
void R_BlitToFBO(FBO_Container_t *src, FBO_Container_t *dst);

void GLBeginHud(void);