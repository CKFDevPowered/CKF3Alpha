#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"

#include "qgl.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"

vec3_t g_TracerEndPos;

void R_DrawSpriteModelEx(cl_entity_t *e)
{
	vec3_t		v_forward, v_right, v_up, point;
	msprite_t		*psprite;
	mspriteframe_t	*frame;
	float vscale, hscale;
	cl_entity_t *se; 

	psprite = (msprite_t *)e->model->cache.data;

	frame = gpRefFuncs->R_GetSpriteFrame(psprite, (int)floor(e->curstate.frame));

	if(!frame)
		return;

	se = *CurrentEntity;
	*CurrentEntity = e;
	
	mcolor24_t col;

	gpRefFuncs->R_SpriteColor(&col, e, e->curstate.renderamt);

	if(e->curstate.rendermode == kRenderTransColor)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ALPHA);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
	}
	if(e->curstate.rendermode == kRenderTransAdd)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderGlow)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDisable(GL_DEPTH_TEST);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderTransAlpha)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
		qglDepthMask(0);
	}

	qglEnable(GL_BLEND);

	gpRefFuncs->R_GetSpriteAxes(e, psprite->type, v_forward, v_right, v_up);

	gpRefFuncs->GL_DisableMultitexture();

	gpRefFuncs->GL_Bind(frame->gl_texturenum);

	qglEnable(GL_ALPHA_TEST);
	qglBegin(GL_QUADS);

	hscale = e->curstate.scale;
	if(e->curstate.fuser1 != 0 && e->curstate.fuser1 != 1)
		vscale = hscale*e->curstate.fuser1;
	else
		vscale = hscale;

	qglTexCoord2f(0, 1);
	VectorMA(e->origin, frame->down*vscale, v_up, point);
	VectorMA(point, frame->left*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(0, 0);
	VectorMA(e->origin, frame->up*vscale, v_up, point);
	VectorMA(point, frame->left*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1, 0);
	VectorMA(e->origin, frame->up*vscale, v_up, point);
	VectorMA(point, frame->right*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1, 1);
	VectorMA(e->origin, frame->down*vscale, v_up, point);
	VectorMA(point, frame->right*hscale, v_right, point);
	qglVertex3fv(point);

	qglEnd();

	if(gl_wireframe->value != 0)
	{
		qglColor4f(0,1,0,1);
		qglDisable(GL_TEXTURE_2D);
		qglBegin(GL_LINE_LOOP);
		VectorMA(e->origin, frame->down*vscale, v_up, point);
		VectorMA(point, frame->left*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, frame->up*vscale, v_up, point);
		VectorMA(point, frame->left*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, frame->up*vscale, v_up, point);
		VectorMA(point, frame->right*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, frame->down*vscale, v_up, point);
		VectorMA(point, frame->right*hscale, v_right, point);
		qglVertex3fv(point);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}

	qglDisable(GL_ALPHA_TEST);
	qglDepthMask(1);
	if(e->curstate.rendermode)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		qglDisable(GL_BLEND);
		qglEnable(GL_DEPTH_TEST);
	}

	*CurrentEntity = se;
}

void R_DrawTGASprite(cl_entity_t *e, tgasprite_t *tgaspr)
{
#if 1
	vec3_t		v_forward, v_right, v_up, point;
	float vscale, hscale, dist;
	mcolor24_t col;
	pmtrace_t pmtrace;

	*CurrentEntity = e;
	gpRefFuncs->R_SpriteColor(&col, e, e->curstate.renderamt);

	if(e->curstate.rendermode == kRenderTransColor)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ALPHA);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
	}
	if(e->curstate.rendermode == kRenderTransAdd)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderGlow)
	{
		cl_pmove->usehull = 2;
		pmtrace = cl_pmove->PM_PlayerTraceEx(refdef->vieworg, e->origin, r_traceglow->value ? (PM_GLASS_IGNORE | PM_STUDIO_IGNORE) : PM_GLASS_IGNORE, CL_TraceEntity_Ignore);
		VectorSubtract(e->origin, refdef->vieworg, point);
		dist = VectorLength(point);
		if ((1.0 - pmtrace.fraction) * dist > 8)
			return;

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDisable(GL_DEPTH_TEST);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderTransAlpha)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderAddColor)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_COLOR, GL_ONE);
		col.r = col.r * e->curstate.renderamt / 255;
		col.g = col.g * e->curstate.renderamt / 255;
		col.b = col.b * e->curstate.renderamt / 255;
		qglColor4ub(col.r,col.g,col.b,255);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderTransAlphaNoDepth)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
		qglDisable(GL_DEPTH_TEST);
		qglDepthMask(0);
	}
	qglEnable(GL_BLEND);

	gpRefFuncs->R_GetSpriteAxes(e, SPR_VP_PARALLEL, v_forward, v_right, v_up);

	gpRefFuncs->GL_DisableMultitexture();

	gpRefFuncs->GL_Bind(tgaspr->tex);

	qglEnable(GL_ALPHA_TEST);
	qglBegin(GL_QUADS);

	hscale = e->curstate.scale;
	if(e->curstate.fuser1 != 0 && e->curstate.fuser1 != 1)
		vscale = hscale*e->curstate.fuser1;
	else
		vscale = hscale;

	int fnum = max(min( e->curstate.frame , 1), 0) * (tgaspr->numframes-1);
	tgasprite_frame_t *f = &tgaspr->frame[fnum];

	qglTexCoord2f(f->texcoord[0][0], f->texcoord[0][1]);
	VectorMA(e->origin, -(tgaspr->h/2)*vscale, v_up, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[1][0], f->texcoord[1][1]);
	VectorMA(e->origin, (tgaspr->h/2)*vscale, v_up, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[2][0], f->texcoord[2][1]);
	VectorMA(e->origin, (tgaspr->h/2)*vscale, v_up, point);
	VectorMA(point, (tgaspr->w/2)*hscale, v_right, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[3][0], f->texcoord[3][1]);
	VectorMA(e->origin, -(tgaspr->h/2)*vscale, v_up, point);
	VectorMA(point, (tgaspr->w/2)*hscale, v_right, point);
	qglVertex3fv(point);

	qglEnd();

	if(gl_wireframe->value != 0)
	{
		qglColor4f(0,1,0,1);
		qglDisable(GL_TEXTURE_2D);
		qglBegin(GL_LINE_LOOP);
		VectorMA(e->origin, -(tgaspr->h/2)*vscale, v_up, point);
		VectorMA(point, -(tgaspr->w/2)*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, (tgaspr->h/2)*vscale, v_up, point);
		VectorMA(point, -(tgaspr->w/2)*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, (tgaspr->h/2)*vscale, v_up, point);
		VectorMA(point, (tgaspr->w/2)*hscale, v_right, point);
		qglVertex3fv(point);
		VectorMA(e->origin, -(tgaspr->h/2)*vscale, v_up, point);
		VectorMA(point, (tgaspr->w/2)*hscale, v_right, point);
		qglVertex3fv(point);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}

	qglDisable(GL_ALPHA_TEST);
	qglDepthMask(1);
	if(e->curstate.rendermode)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		qglDisable(GL_BLEND);
		qglEnable(GL_DEPTH_TEST);
	}
#endif
}

void R_DrawTGATracer(cl_entity_t *e, tgasprite_t *tgaspr)
{
#if 1
	vec3_t linedir, viewdir, cross, cross2, point;
	float vscale, hscale;
	mcolor24_t col;

	VectorCopy(e->curstate.velocity, linedir);
	VectorSubtract(e->origin, refdef->vieworg, viewdir);

	CrossProduct(linedir, viewdir, cross);
	CrossProduct(cross, viewdir, cross2);
	VectorNormalize( cross );
	VectorNormalize( cross2 );

	*CurrentEntity = e;
	gpRefFuncs->R_SpriteColor(&col, e, e->curstate.renderamt);

	if(e->curstate.rendermode == kRenderTransColor)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ALPHA);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
	}
	if(e->curstate.rendermode == kRenderTransAdd)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderGlow)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_ONE, GL_ONE);
		qglColor4ub(col.r,col.g,col.b,255);
		qglDisable(GL_DEPTH_TEST);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderTransAlpha)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4ub(col.r,col.g,col.b,e->curstate.renderamt);
		qglDepthMask(0);
	}
	if(e->curstate.rendermode == kRenderAddColor)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_COLOR, GL_ONE);
		col.r = col.r * e->curstate.renderamt / 255;
		col.g = col.g * e->curstate.renderamt / 255;
		col.b = col.b * e->curstate.renderamt / 255;
		qglColor4ub(col.r,col.g,col.b,255);
		qglDepthMask(0);
	}

	qglEnable(GL_BLEND);

	gpRefFuncs->GL_DisableMultitexture();

	gpRefFuncs->GL_Bind(tgaspr->tex);

	qglEnable(GL_ALPHA_TEST);
	qglBegin(GL_QUADS);

	hscale = e->curstate.scale;
	if(e->curstate.fuser1 != 0 && e->curstate.fuser1 != 1)
		vscale = hscale*e->curstate.fuser1;
	else
		vscale = hscale;

	int fnum = max(min( e->curstate.frame , 1), 0) * (tgaspr->numframes-1);
	tgasprite_frame_t *f = &tgaspr->frame[fnum];
#if 0
	qglTexCoord2f(f->texcoord[0][0], f->texcoord[0][1]);
	VectorMA(e->origin, -(tgaspr->h/2)*vscale, cross2, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[1][0], f->texcoord[1][1]);
	VectorMA(e->origin, (tgaspr->h/2)*vscale, cross2, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[2][0], f->texcoord[2][1]);
	VectorMA(e->origin, (tgaspr->h/2)*vscale, cross2, point);
	VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[3][0], f->texcoord[3][1]);
	VectorMA(e->origin, -(tgaspr->h/2)*vscale, cross2, point);
	VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

#else
	qglTexCoord2f(f->texcoord[0][0], f->texcoord[0][1]);
	VectorMA(e->origin, -(tgaspr->h)*vscale, cross2, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[1][0], f->texcoord[1][1]);
	VectorMA(e->origin, 0, cross2, point);
	VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[2][0], f->texcoord[2][1]);
	VectorMA(e->origin, 0, cross2, point);
	VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);

	qglTexCoord2f(f->texcoord[3][0], f->texcoord[3][1]);
	VectorMA(e->origin, -(tgaspr->h)*vscale, cross2, point);
	VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
	qglVertex3fv(point);
#endif
	qglEnd();

	VectorMA(e->origin, -(tgaspr->h)*vscale, cross2, g_TracerEndPos);

	if(gl_wireframe->value != 0)
	{
		qglColor4f(0,1,0,1);
		qglDisable(GL_TEXTURE_2D);
		qglBegin(GL_LINE_LOOP);
		VectorMA(e->origin, -(tgaspr->h/2)*vscale, cross2, point);
		VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
		qglVertex3fv(point);
		VectorMA(e->origin, (tgaspr->h/2)*vscale, cross2, point);
		VectorMA(point, -(tgaspr->w/2)*hscale, cross, point);
		qglVertex3fv(point);
		VectorMA(e->origin, (tgaspr->h/2)*vscale, cross2, point);
		VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
		qglVertex3fv(point);
		VectorMA(e->origin, -(tgaspr->h/2)*vscale, cross2, point);
		VectorMA(point, (tgaspr->w/2)*hscale, cross, point);
		qglVertex3fv(point);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}

	qglDisable(GL_ALPHA_TEST);
	qglDepthMask(1);
	if(e->curstate.rendermode)
	{
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		qglDisable(GL_BLEND);
		qglEnable(GL_DEPTH_TEST);
	}
#endif
}

void R_Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h, qboolean bLocalXY)
{
	gpRefExports->R_PushRefDef();
	VectorClear(refdef->viewangles);
	VectorClear(refdef->vieworg);
	gpRefExports->R_UpdateRefDef();

	*CurrentEntity = pEntity;

	int centerX = x + w/2;
	int centerY = y + h/2;
	if(g_RefSupportExt & r_ext_fbo)
	{
		int left = w/2;
		int top = h/2;

		gpRefExports->R_BeginDrawTrianglesInHUD_FBO(centerX, centerY, left, top);
		if(pEntity->player)
			g_StudioRenderer.StudioDrawPlayer_3DHUD();
		else
			g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
		gpRefExports->R_FinishDrawTrianglesInHUD();

		if(g_RefSupportExt & r_ext_shader)
			gpRefExports->R_BeginFXAA(ScreenWidth, ScreenHeight);

		if(bLocalXY)
			gpRefExports->R_Draw3DHUDQuad(left, top, left, top);
		else
			gpRefExports->R_Draw3DHUDQuad(centerX, centerY, left, top);

		if(g_RefSupportExt & r_ext_shader)
			gpRefExports->ShaderAPI.GL_EndProgram();
	}
	else
	{
		gpRefExports->R_BeginDrawTrianglesInHUD_Direct(centerX, centerY);
		if(pEntity->player)
			g_StudioRenderer.StudioDrawPlayer_3DHUD();
		else
			g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
		gpRefExports->R_FinishDrawTrianglesInHUD();
	}
	gpRefExports->R_PopRefDef();
}