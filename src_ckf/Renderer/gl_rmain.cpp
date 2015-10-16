#include "gl_local.h"
#include "cJSON.h"
#include "screen.h"
#include <IBTEClient.h>

ref_funcs_t gRefFuncs;

refdef_t *r_refdef;

ref_params_t r_params;

float gldepthmin, gldepthmax;

cl_entity_t *r_worldentity;
model_t *r_worldmodel;
int *cl_numvisedicts;
int cl_maxvisedicts;

RECT *window_rect;

float *videowindowaspect;
float *windowvideoaspect;
float videowindowaspect_old;
float windowvideoaspect_old;

cl_entity_t **cl_visedicts_old;
cl_entity_t **cl_visedicts_new;
cl_entity_t **currententity;
int *maxTransObjs;
transObjRef **transObjects;
int numTransObjs;
GLuint screenframebuffer;
GLuint drawframebuffer;
GLuint readframebuffer;

mleaf_t **r_viewleaf, **r_oldviewleaf;
texture_t *r_notexture_mip;

int mirrortexturenum;
qboolean mirror;
mplane_t *mirror_plane;

float yfov;
float screenaspect;

vec_t *vup;
vec_t *vpn;
vec_t *vright;
vec_t *r_origin;
vec_t *modelorg;
vec_t *r_entorigin;

int *r_framecount;
int *r_visframecount;

frame_t *cl_frames;
int *cl_parsecount;

char com_token[1024];

float r_world_matrix[16];
float r_projection_matrix[16];

qboolean gl_framebuffer_object = false;
qboolean gl_shader_support = false;
qboolean gl_program_support = false;
qboolean gl_msaa_support = false;
qboolean gl_msaa_blit_support = false;
qboolean gl_csaa_support = false;
qboolean gl_float_buffer_support = false;
qboolean gl_s3tc_compression_support = false;

int gl_mtexable = 0;
int gl_max_texture_size = 0;
float gl_max_ansio = 0;
float gl_force_ansio = 0;
int gl_msaa_samples = 0;
int gl_csaa_samples = 0;

FBO_Container_t s_MSAAFBO;
FBO_Container_t s_BackBufferFBO;
FBO_Container_t s_3DHUDFBO;
FBO_Container_t s_WaterFBO;
FBO_Container_t s_ShadowFBO;
FBO_Container_t s_DownSampleFBO[DOWNSAMPLE_BUFFERS];
FBO_Container_t s_LuminFBO[LUMIN_BUFFERS];
FBO_Container_t s_Lumin1x1FBO[LUMIN1x1_BUFFERS];
FBO_Container_t s_BlurFBO[BLUR_BUFFERS];
FBO_Container_t s_ToneMapFBO;
FBO_Container_t s_HUDInWorldFBO;
FBO_Container_t s_CloakFBO;

qboolean bDoMSAAFBO = true;
qboolean bDoScaledFBO = true;
qboolean bDoDirectBlit = true;
qboolean bDoHDR = true;
qboolean bNoStretchAspect = false;

cvar_t *ati_subdiv = NULL;
cvar_t *ati_npatch = NULL;

cvar_t *r_bmodelinterp = NULL;
cvar_t *r_bmodelhighfrac = NULL;
cvar_t *r_norefresh = NULL;
cvar_t *r_drawentities = NULL;
cvar_t *r_drawviewmodel = NULL;
cvar_t *r_speeds = NULL;
cvar_t *r_fullbright = NULL;
cvar_t *r_decals = NULL;
cvar_t *r_lightmap = NULL;
cvar_t *r_shadows = NULL;
cvar_t *r_mirroralpha = NULL;
cvar_t *r_wateralpha = NULL;
cvar_t *r_dynamic = NULL;
cvar_t *r_novis = NULL;
cvar_t *r_mmx = NULL;
cvar_t *r_traceglow = NULL;
cvar_t *r_wadtextures = NULL;
cvar_t *r_glowshellfreq = NULL;
cvar_t *r_detailtextures = NULL;

cvar_t *gl_vsync = NULL;
cvar_t *gl_ztrick = NULL;
cvar_t *gl_finish = NULL;
cvar_t *gl_clear = NULL;
cvar_t *gl_cull = NULL;
cvar_t *gl_texsort = NULL;
cvar_t *gl_smoothmodels = NULL;
cvar_t *gl_affinemodels = NULL;
cvar_t *gl_flashblend = NULL;
cvar_t *gl_playermip = NULL;
cvar_t *gl_nocolors = NULL;
cvar_t *gl_keeptjunctions = NULL;
cvar_t *gl_reporttjunctions = NULL;
cvar_t *gl_wateramp = NULL;
cvar_t *gl_dither = NULL;
cvar_t *gl_spriteblend = NULL;
cvar_t *gl_polyoffset = NULL;
cvar_t *gl_lightholes = NULL;
cvar_t *gl_zmax = NULL;
cvar_t *gl_alphamin = NULL;
cvar_t *gl_overdraw = NULL;
cvar_t *gl_watersides = NULL;
cvar_t *gl_overbright = NULL;
cvar_t *gl_envmapsize = NULL;
cvar_t *gl_flipmatrix = NULL;
cvar_t *gl_monolights = NULL;
cvar_t *gl_fog = NULL;
cvar_t *gl_wireframe = NULL;
cvar_t *gl_ansio = NULL;
cvar_t *developer = NULL;
cvar_t *gl_round_down = NULL;
cvar_t *gl_picmip = NULL;
cvar_t *gl_max_size = NULL;

cvar_t *v_lightgamma = NULL;
cvar_t *v_brightness = NULL;
cvar_t *v_gamma = NULL;

cvar_t *cl_righthand = NULL;

refdef_t *R_GetRefDef(void)
{
	return r_refdef;
}

int R_GetDrawPass(void)
{
	if(drawreflect)
		return r_draw_reflect;
	if(drawrefract)
		return r_draw_refract;
	if(drawshadow)
		return r_draw_shadow;
	if(draw3dhud)
		return r_draw_3dhud;
	if(drawshadowscene)
		return r_draw_shadowscene;
	if(draw3dsky)
		return r_draw_3dsky;
	if(drawhudinworld)
		return r_draw_hudinworld;
	return r_draw_normal;
}

int R_GetSupportExtension(void)
{
	int ext = 0;

	if(s_BackBufferFBO.s_hBackBufferFBO)
		ext |= r_ext_fbo;
	if(s_MSAAFBO.s_hBackBufferFBO)
		ext |= r_ext_msaa;
	if(water_program)
		ext |= r_ext_water;
	if(gl_shader_support)
		ext |= r_ext_shader;
	if(shadow_program)
		ext |= r_ext_shadow;

	return ext;
}

qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
	int i;

	if(draw3dhud || drawshadow)
		return false;

	//no frustum cull in 3dsky box
	if(draw3dsky)
		return false;

	if(drawreflect || drawrefract)
	{
		for (i = 0; i < 4; i++)
		{
			if (BoxOnPlaneSide(mins, maxs, &custom_frustum[i]) == 2)
				return true;
		}
		return false;
	}

	return gRefFuncs.R_CullBox(mins, maxs);
}

void R_RotateForEntity(vec_t *origin, cl_entity_t *e)
{
}

void R_DrawSpriteModel(cl_entity_t *entity)
{
	R_Setup3DSkyModel();

	gRefFuncs.R_DrawSpriteModel(entity);

	R_Finish3DSkyModel();
}

void R_GetSpriteAxes(cl_entity_t *entity, int type, float *vforwrad, float *vright, float *vup)
{
	gRefFuncs.R_GetSpriteAxes(entity, type, vforwrad, vright, vup);
}

void R_SpriteColor(mcolor24_t *col, cl_entity_t *entity, int renderamt)
{
	gRefFuncs.R_SpriteColor(col, entity, renderamt);
}

float GlowBlend(cl_entity_t *entity)
{
	return gRefFuncs.GlowBlend(entity);
}

int CL_FxBlend(cl_entity_t *entity)
{
	return gRefFuncs.CL_FxBlend(entity);
}

void R_Clear(void)
{
	if (r_mirroralpha->value != 1.0)
	{
		if (gl_clear->value)
			qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			qglClear(GL_DEPTH_BUFFER_BIT);

		gldepthmin = 0;
		gldepthmax = 0.5;
		qglDepthFunc(GL_LEQUAL);
	}
	else if (gl_ztrick->value)
	{
		static int trickframe;

		if (gl_clear->value)
			qglClear(GL_COLOR_BUFFER_BIT);

		trickframe++;

		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			qglDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			qglDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		if (gl_clear->value)
			qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			qglClear(GL_DEPTH_BUFFER_BIT);

		gldepthmin = 0;
		gldepthmax = 1;
		qglDepthFunc(GL_LEQUAL);
	}

	qglDepthRange(gldepthmin, gldepthmax);
}

void R_AddTEntity(cl_entity_t *pEnt)
{
	float dist;
	vec3_t v;

	if (numTransObjs >= (*maxTransObjs))
		g_pMetaSave->pEngineFuncs->Con_Printf("R_AddTEntity: Too many objects");

	if (!pEnt->model || pEnt->model->type != mod_brush || pEnt->curstate.rendermode != kRenderTransAlpha)
	{
		VectorAdd(pEnt->model->mins, pEnt->model->maxs, v);
		VectorScale(v, 0.5, v);
		VectorAdd(v, pEnt->origin, v);
		VectorSubtract(r_origin, v, v);
		dist = DotProduct(v, v);
	}
	else
	{
		dist = 1000000000;
	}

	(*transObjects)[numTransObjs].pEnt = pEnt;
	(*transObjects)[numTransObjs].distance = dist;
	numTransObjs ++;
}

#define CURRENT_DRAW_PLAYER_STATE ((entity_state_t *)( (char *)cl_frames + sizeof(frame_t) * parsecount + sizeof(entity_state_t) * (*currententity)->index) )

entity_state_t *R_GetPlayerState(int index)
{
	return (entity_state_t *)( (char *)cl_frames + sizeof(frame_t) * ((*cl_parsecount) % 63) + sizeof(entity_state_t) * index );
}

void R_DrawEntitiesOnList(void)
{
	int i, j, numvisedicts, parsecount, candraw3dsky;

	if (!r_drawentities->value)
		return;

	numvisedicts = *cl_numvisedicts;
	parsecount = (*cl_parsecount) & 63;

	numTransObjs = 0;

	candraw3dsky = (_3dsky_enable && r_3dsky->value > 0) ? true : false;

	for (i = 0; i < numvisedicts; i++)
	{
		(*currententity) = cl_visedicts_new[i];

		if ((*currententity)->curstate.rendermode != kRenderNormal || (*currententity)->curstate.renderfx == kRenderFxCloak)
		{
			R_AddTEntity(*currententity);
			continue;
		}

		if( !candraw3dsky && (*currententity)->curstate.entityType == ET_3DSKYENTITY )//if( !candraw3dsky && ((*currententity)->curstate.effects & EF_3DSKY) )
			continue;

		switch ((*currententity)->model->type)
		{
			case mod_brush:
			{
				R_DrawBrushModel(*currententity);
				break;
			}

			case mod_studio:
			{
				R_Setup3DSkyModel();
				if ((*currententity)->player)
				{
					gpStudioInterface->StudioDrawPlayer(STUDIO_RENDER | STUDIO_EVENTS, CURRENT_DRAW_PLAYER_STATE );
				}
				else
				{
					if ((*currententity)->curstate.movetype == MOVETYPE_FOLLOW)
					{
						for (j = 0; j < numvisedicts; j++)
						{
							if (cl_visedicts_new[j]->index == (*currententity)->curstate.aiment)
							{
								*currententity = cl_visedicts_new[j];

								if ((*currententity)->player)
								{
									gpStudioInterface->StudioDrawPlayer(0, CURRENT_DRAW_PLAYER_STATE );
								}
								else
								{
									gpStudioInterface->StudioDrawModel(0);
								}

								*currententity = cl_visedicts_new[i];
								break;
							}
						}
					}

					gpStudioInterface->StudioDrawModel(STUDIO_RENDER | STUDIO_EVENTS);
					
				}
				R_Finish3DSkyModel();
				break;
			}

			default:
			{
				break;
			}
		}
	}

	*r_blend = 1.0;

	for (i = 0; i < numvisedicts; i++)
	{
		*currententity = cl_visedicts_new[i];

		if ((*currententity)->curstate.rendermode != kRenderNormal)
		{
			continue;
		}

		switch ((*currententity)->model->type)
		{
			case mod_sprite:
			{
				if ((*currententity)->curstate.body)
				{
					float *pAttachment = R_GetAttachmentPoint((*currententity)->curstate.skin, (*currententity)->curstate.body);
					VectorCopy(pAttachment, r_entorigin);
				}
				else
				{
					VectorCopy((*currententity)->origin, r_entorigin);
				}

				R_DrawSpriteModel(*currententity);
				break;
			}
		}
	}
}

int R_QuickSortTEntities(const void *a, const void *b)
{
	return ((transObjRef *)a)->distance > ((transObjRef *)b)->distance;
}

void R_SortTEntities(void)
{
	qsort((*transObjects), numTransObjs, sizeof(transObjRef), R_QuickSortTEntities);
}

void R_DrawTEntitiesOnList(int onlyClientDraw)
{
	int i, j, numvisedicts, parsecount, candraw3dsky;

	if (!r_drawentities->value)
		return;

	numvisedicts = *cl_numvisedicts;
	parsecount = (*cl_parsecount) & 63;

	candraw3dsky = (_3dsky_enable && r_3dsky->value > 0) ? true : false;

	R_SortTEntities();

	if (!onlyClientDraw)
	{
		for (i = 0; i < numTransObjs; i++)
		{
			(*currententity) = (*transObjects)[i].pEnt;

			//if( !candraw3dsky && ((*currententity)->curstate.effects & EF_3DSKY) )
			if( !candraw3dsky && (*currententity)->curstate.entityType == ET_3DSKYENTITY )
				continue;

			qglDisable(GL_FOG);

			*r_blend = gRefFuncs.CL_FxBlend(*currententity);

			if (*r_blend <= 0)
				continue;

			*r_blend = (*r_blend) / 255.0;

			if ((*currententity)->curstate.rendermode == kRenderGlow && (*currententity)->model->type != mod_sprite)
				g_pMetaSave->pEngineFuncs->Con_DPrintf("Non-sprite set to glow!\n");

			switch ((*currententity)->model->type)
			{
				case mod_brush:
				{
					if (*g_bUserFogOn)
					{
						if ((*currententity)->curstate.rendermode != kRenderGlow && (*currententity)->curstate.rendermode != kRenderTransAdd)
							qglEnable(GL_FOG);
					}

					R_DrawBrushModel(*currententity);
					break;
				}

				case mod_sprite:
				{
					if ((*currententity)->curstate.body)
					{
						float *pAttachment = R_GetAttachmentPoint((*currententity)->curstate.skin, (*currententity)->curstate.body);
						VectorCopy(pAttachment, r_entorigin);
					}
					else
					{
						VectorCopy((*currententity)->origin, r_entorigin);
					}

					if ((*currententity)->curstate.rendermode == kRenderGlow)
						(*r_blend) *= gRefFuncs.GlowBlend(*currententity);

					if ((*r_blend) != 0)
						R_DrawSpriteModel(*currententity);

					break;
				}

				case mod_studio:
				{
					R_Setup3DSkyModel();
					if ((*currententity)->player)
					{
						gpStudioInterface->StudioDrawPlayer(STUDIO_RENDER | STUDIO_EVENTS, CURRENT_DRAW_PLAYER_STATE );
					}
					else
					{
						if ((*currententity)->curstate.movetype == MOVETYPE_FOLLOW)
						{
							for (j = 0; j < numvisedicts; j++)
							{
								if ((*transObjects)[j].pEnt->index == (*currententity)->curstate.aiment)
								{
									*currententity = (*transObjects)[j].pEnt;

									if ((*currententity)->player)
									{
										gpStudioInterface->StudioDrawPlayer(0, CURRENT_DRAW_PLAYER_STATE );
									}
									else
									{
										gpStudioInterface->StudioDrawModel(0);
									}

									*currententity = (*transObjects)[i].pEnt;
									break;
								}
							}
						}

						gpStudioInterface->StudioDrawModel(STUDIO_RENDER | STUDIO_EVENTS);
					}
					R_Finish3DSkyModel();

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

    GL_DisableMultitexture();

    qglEnable(GL_ALPHA_TEST);
    qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (*g_bUserFogOn)
		qglDisable(GL_FOG);

	HUD_DrawTransparentTriangles();

	if (*g_bUserFogOn)
		qglEnable(GL_FOG);

	numTransObjs = 0;
	*r_blend = 1.0;
}

void R_DrawBrushModel(cl_entity_t *entity)
{
	R_Setup3DSkyModel();

	gRefFuncs.R_DrawBrushModel(entity);

	R_Finish3DSkyModel();
}

void R_DrawViewModel(void)
{
}

void R_PreDrawViewModel(void)
{
}

void R_PolyBlend(void)
{
}

void R_SetFrustum(void)
{
	gRefFuncs.R_SetFrustum();
}

void R_ForceCVars(qboolean mp)
{
	if (gRefFuncs.R_ForceCVars)
	{
		gRefFuncs.R_ForceCVars(mp);
		return;
	}

	if (gl_polyoffset->value < 0 && gl_polyoffset->value < -0.001)
		Cvar_DirectSet(gl_polyoffset, "-0.001");
	if (gl_polyoffset->value > 0 && gl_polyoffset->value < 0.001)
		Cvar_DirectSet(gl_polyoffset, "0.001");
	else if (gl_polyoffset->value > 0 && gl_polyoffset->value > 25)
		Cvar_DirectSet(gl_polyoffset, "25");

	if (v_lightgamma->value < 1.8)
	{
		Cvar_DirectSet(v_lightgamma, "1.8");
		GL_BuildLightmaps();
	}
}

int SignbitsForPlane(mplane_t *out)
{
	int bits, j;

	bits = 0;

	for (j = 0; j < 3; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}

	return bits;
}

void R_SetupFrame(void)
{
	float gl_wireframe_value = gl_wireframe->value;
	float r_dynamic_value = r_dynamic->value;

	if (R_GetDrawPass() == r_draw_normal)
		R_ForceCVars(g_pMetaSave->pEngineFuncs->GetMaxClients() > 1);

	gRefFuncs.R_SetupFrame();

	if(developer->value != 0)
	{
		gl_wireframe->value = gl_wireframe_value;
	}
	if(developer->value != 0)
	{
		r_dynamic->value = r_dynamic_value;
	}
}

void MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	qglFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

float CalcFov(float fov_x, float width, float height)
{
	float a;
	float x;

	if (fov_x < 1 || fov_x > 179)
		x = 1;
	else
		x = tan(fov_x / 360 * M_PI);

	x = width / x;
	a = atanf(height / x);
	a = a * 360 / M_PI;
	return a;
}

void R_SetupGL(void)
{
	gRefFuncs.R_SetupGL();
}

void R_CalcRefdef(struct ref_params_s *pparams)
{
	memcpy(&r_params, pparams, sizeof(struct ref_params_s));

	yfov = CalcFov(scr_fov_value, r_refdef->vrect.width, r_refdef->vrect.height);
	screenaspect = (float)r_refdef->vrect.width / r_refdef->vrect.height;
}

void R_RenderScene(void)
{
	R_UploadLightmaps();
	Draw_UpdateAnsios();

	if(_3dsky_enable && r_3dsky->value)
	{
		R_ViewOriginFor3DSky(_3dsky_view);
	}

	if(!r_refdef->onlyClientDraws)
	{
		if(shadow_program && r_shadow->value)
		{
			if (!(shadow_update_counter % 2))
			{
				R_UpdateShadow();
			}
			shadow_update_counter ++;
		}
		if(water_program && r_water->value)
		{
			if (!(water_update_counter % 2))
			{
				R_UpdateWater();			
			}
			water_update_counter ++;
		}
	}

	R_Clear();

	if(_3dsky_enable && r_3dsky->value)
	{
		R_Render3DSky();
	}

	qglBindFramebufferEXT(GL_FRAMEBUFFER, screenframebuffer);

	gRefFuncs.R_RenderScene();
}

void CheckMultiTextureExtensions(void)
{
	if (gl_mtexable)
	{
		TEXTURE0_SGIS = GL_TEXTURE0;
		TEXTURE1_SGIS = GL_TEXTURE1;
		TEXTURE2_SGIS = GL_TEXTURE2;
		TEXTURE3_SGIS = GL_TEXTURE3;
	}
	else
	{
		Sys_ErrorEx("don't support multitexture extension!");
	}
}

void GL_ClearFBO(FBO_Container_t *s)
{
	s->s_hBackBufferFBO = 0;
	s->s_hBackBufferCB = 0;
	s->s_hBackBufferDB = 0;
	s->s_hBackBufferTex = 0;
	s->iWidth = s->iHeight = 0;
}

void GL_FreeFBO(FBO_Container_t *s)
{
	if (s->s_hBackBufferFBO)
		qglDeleteFramebuffersEXT(1, &s->s_hBackBufferFBO);

	if (s->s_hBackBufferCB)
		qglDeleteRenderbuffersEXT(1, &s->s_hBackBufferCB);

	if (s->s_hBackBufferDB)
		qglDeleteRenderbuffersEXT(1, &s->s_hBackBufferDB);

	if (s->s_hBackBufferTex)
		qglDeleteTextures(1, &s->s_hBackBufferTex);

	GL_ClearFBO(s);
}

void R_GLGenFrameBuffer(FBO_Container_t *s)
{
	qglGenFramebuffersEXT(1, &s->s_hBackBufferFBO);
	qglBindFramebufferEXT(GL_FRAMEBUFFER, s->s_hBackBufferFBO);
}

void R_GLGenRenderBuffer(FBO_Container_t *s, qboolean depth)
{
	if(!depth)
	{
		qglGenRenderbuffersEXT(1, &s->s_hBackBufferCB);
		qglBindRenderbufferEXT(GL_RENDERBUFFER, s->s_hBackBufferCB);
	}
	else
	{
		qglGenRenderbuffersEXT(1, &s->s_hBackBufferDB);
		qglBindRenderbufferEXT(GL_RENDERBUFFER, s->s_hBackBufferDB);
	}
}

void R_GLRenderBufferStorage(FBO_Container_t *s, qboolean depth, GLuint iInternalFormat, qboolean multisample)
{
	if(multisample)
	{
		if(gl_csaa_support)
			qglRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, gl_csaa_samples, gl_msaa_samples, iInternalFormat, s->iWidth, s->iHeight);
		else
			qglRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, gl_msaa_samples, iInternalFormat, s->iWidth, s->iHeight);
	}
	else
	{
		qglRenderbufferStorageEXT(GL_RENDERBUFFER, iInternalFormat, s->iWidth, s->iHeight);
	}
	qglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, (depth) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (depth) ? s->s_hBackBufferDB : s->s_hBackBufferCB);
}

void R_GLFrameBufferColorTexture(FBO_Container_t *s, GLuint iInternalFormat)
{
	s->s_hBackBufferTex = GL_GenTexture();
	GL_Bind(s->s_hBackBufferTex);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D, 0, iInternalFormat, s->iWidth, s->iHeight, 0, GL_RGBA, (iInternalFormat != GL_RGBA && iInternalFormat != GL_RGBA8) ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);

	qglFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->s_hBackBufferTex, 0);
}

void GL_GenerateFBO(void)
{
	if (!gl_framebuffer_object)
		return;

	bNoStretchAspect = (g_pMetaSave->pEngineFuncs->CheckParm("-stretchaspect", NULL) == 0);

	if (g_pMetaSave->pEngineFuncs->CheckParm("-nomsaa", NULL))
		bDoMSAAFBO = false;

	//if (!gl_msaa_blit_support)
	//	bDoMSAAFBO = false;

	if (g_pMetaSave->pEngineFuncs->CheckParm("-nofbo", NULL))
		bDoScaledFBO = false;

	if (g_pMetaSave->pEngineFuncs->CheckParm("-directblit", NULL))
		bDoDirectBlit = true;

	if (g_pMetaSave->pEngineFuncs->CheckParm("-nodirectblit", NULL))
		bDoDirectBlit = false;

	if (g_pMetaSave->pEngineFuncs->CheckParm("-nohdr", NULL))
		bDoHDR = false;

	if(!gl_float_buffer_support)
		bDoHDR = false;

	if (!qglGenFramebuffersEXT || !qglBindFramebufferEXT || !qglBlitFramebufferEXT)
		bDoScaledFBO = false;

	GL_ClearFBO(&s_MSAAFBO);
	GL_ClearFBO(&s_BackBufferFBO);
	GL_ClearFBO(&s_3DHUDFBO);
	GL_ClearFBO(&s_ShadowFBO);
	GL_ClearFBO(&s_WaterFBO);
	for(int i = 0; i < DOWNSAMPLE_BUFFERS; ++i)
		GL_ClearFBO(&s_DownSampleFBO[i]);
	for(int i = 0; i < LUMIN_BUFFERS; ++i)
		GL_ClearFBO(&s_LuminFBO[i]);
	for(int i = 0; i < LUMIN1x1_BUFFERS; ++i)
		GL_ClearFBO(&s_Lumin1x1FBO[i]);
	for(int i = 0; i < BLUR_BUFFERS; ++i)
		GL_ClearFBO(&s_BlurFBO[i]);
	GL_ClearFBO(&s_ToneMapFBO);
	GL_ClearFBO(&s_HUDInWorldFBO);
	GL_ClearFBO(&s_CloakFBO);

	if(!bDoScaledFBO)
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("FBO backbuffer rendering disabled.\n");
		bDoHDR = false;
	}

	qglEnable(GL_TEXTURE_2D);

	GLuint iColorInternalFormat = GL_RGBA8;

	if(bDoHDR)
	{
		iColorInternalFormat = GL_RGBA16F;

		const char *s_HDRColor;
		if(g_pInterface->CommandLine->CheckParm("-hdrcolor", &s_HDRColor))
		{
			if(s_HDRColor && s_HDRColor[0] >= '0' && s_HDRColor[0] <= '9')
			{
				int i_HDRColor = atoi(s_HDRColor);
				if(i_HDRColor == 8)
					iColorInternalFormat = GL_RGBA8;
				else if(i_HDRColor == 16)
					iColorInternalFormat = GL_RGBA16F;
				else if(i_HDRColor == 32)
					iColorInternalFormat = GL_RGBA32F;
			}
		}
	}

	if (bDoScaledFBO && gl_msaa_support && bDoMSAAFBO)
	{
		const char *s_Samples;
		gl_msaa_samples = 4;
		if(gl_csaa_support)
			gl_csaa_samples = 8;
		if(g_pInterface->CommandLine->CheckParm("-msaa", &s_Samples))
		{
			if(s_Samples && s_Samples[0] >= '0' && s_Samples[0] <= '9')
			{
				int i_Samples = atoi(s_Samples);
				if(i_Samples == 4)
					gl_msaa_samples = 4;
				else if(i_Samples == 8)
					gl_msaa_samples = 8;
				else if(i_Samples == 16)
					gl_msaa_samples = 16;
			}
		}

		if(g_pInterface->CommandLine->CheckParm("-csaa", &s_Samples))
		{
			if(s_Samples && s_Samples[0] >= '0' && s_Samples[0] <= '9')
			{
				int i_Samples = atoi(s_Samples);
				if(i_Samples == 4)
					gl_csaa_samples = 4;
				else if(i_Samples == 8)
					gl_csaa_samples = 8;
				else if(i_Samples == 16)
					gl_csaa_samples = 16;
			}
		}

		s_MSAAFBO.iWidth = g_iVideoWidth;
		s_MSAAFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_MSAAFBO);
		//color
		R_GLGenRenderBuffer(&s_MSAAFBO, false);
		R_GLRenderBufferStorage(&s_MSAAFBO, false, iColorInternalFormat, true);
		//depth
		R_GLGenRenderBuffer(&s_MSAAFBO, true);
		R_GLRenderBufferStorage(&s_MSAAFBO, true, GL_DEPTH_COMPONENT24, true);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_MSAAFBO);
			gl_msaa_support = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("Error initializing MSAA frame buffer\n");
		}
	}
	else
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("MSAA backbuffer rendering disabled.\n");
	}

	if (bDoScaledFBO)
	{
		if (s_MSAAFBO.s_hBackBufferFBO)
			qglEnable(GL_MULTISAMPLE);

		s_BackBufferFBO.iWidth = g_iVideoWidth;
		s_BackBufferFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_BackBufferFBO);
		//color
		R_GLFrameBufferColorTexture(&s_BackBufferFBO, iColorInternalFormat);
		//depth
		R_GLGenRenderBuffer(&s_BackBufferFBO, true);
		R_GLRenderBufferStorage(&s_BackBufferFBO, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, false);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_MSAAFBO);
			GL_FreeFBO(&s_BackBufferFBO);		
			g_pMetaSave->pEngineFuncs->Con_Printf("FBO backbuffer rendering disabled due to create error.\n");
		}
	}

	//3D HUD FBO
	s_3DHUDFBO.iWidth = g_iVideoWidth;
	s_3DHUDFBO.iHeight = g_iVideoHeight;

	//fbo
	R_GLGenFrameBuffer(&s_3DHUDFBO);
	//color
	R_GLFrameBufferColorTexture(&s_3DHUDFBO, GL_RGBA8);
	//depth
	R_GLGenRenderBuffer(&s_3DHUDFBO, true);
	R_GLRenderBufferStorage(&s_3DHUDFBO, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, false);

	if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		GL_FreeFBO(&s_3DHUDFBO);
		g_pMetaSave->pEngineFuncs->Con_Printf("3DHUD FBO rendering disabled due to create error.\n");
	}

	//HUDInWorld FBO
	R_GLGenFrameBuffer(&s_HUDInWorldFBO);

	//Shadow FBO
	R_GLGenFrameBuffer(&s_ShadowFBO);
	qglDrawBuffer(GL_NONE);
	qglReadBuffer(GL_NONE);

	//Water FBO
	water_texture_size = gl_max_texture_size / 4;
	s_WaterFBO.iWidth = water_texture_size;
	s_WaterFBO.iHeight = water_texture_size;
	//fbo
	R_GLGenFrameBuffer(&s_WaterFBO);
	//depth
	R_GLGenRenderBuffer(&s_WaterFBO, true);
	R_GLRenderBufferStorage(&s_WaterFBO, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, false);

	if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		GL_FreeFBO(&s_WaterFBO);
		g_pMetaSave->pEngineFuncs->Con_Printf("Water FBO rendering disabled due to create error.\n");
	}

	//DownSample FBO
	int downW = g_iVideoWidth;
	int downH = g_iVideoHeight;
	for(int i = 0; i < DOWNSAMPLE_BUFFERS && bDoHDR; ++i)
	{
		downW >>= 2;
		downH >>= 2;
		s_DownSampleFBO[i].iWidth = downW;
		s_DownSampleFBO[i].iHeight = downH;
		//fbo
		R_GLGenFrameBuffer(&s_DownSampleFBO[i]);
		//color
		R_GLFrameBufferColorTexture(&s_DownSampleFBO[i], iColorInternalFormat);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_DownSampleFBO[i]);
			bDoHDR = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("DownSample FBO #%d rendering disabled due to create error.\n", i);
		}
	}

	//Luminance FBO
	downW = 64;
	downH = 64;
	//64x64 16x16 4x4 1x1
	for(int i = 0; i < LUMIN_BUFFERS && bDoHDR; ++i)
	{
		s_LuminFBO[i].iWidth = downW;
		s_LuminFBO[i].iHeight = downH;
		//fbo
		R_GLGenFrameBuffer(&s_LuminFBO[i]);
		//color
		R_GLFrameBufferColorTexture(&s_LuminFBO[i], GL_R32F);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_LuminFBO[i]);
			bDoHDR = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("Luminance FBO #%d rendering disabled due to create error.\n", i);
		}
		downW >>= 2;
		downH >>= 2;
	}

	//Luminance
	for(int i = 0; i < LUMIN1x1_BUFFERS && bDoHDR; ++i)
	{
		s_Lumin1x1FBO[i].iWidth = 1;
		s_Lumin1x1FBO[i].iHeight = 1;
		//fbo
		R_GLGenFrameBuffer(&s_Lumin1x1FBO[i]);
		//color
		R_GLFrameBufferColorTexture(&s_Lumin1x1FBO[i], GL_R32F);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_Lumin1x1FBO[i]);
			bDoHDR = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("Luminance FBO #%d rendering disabled due to create error.\n", i);
		}
	}

	//Blur FBO
	for(int i = 0; i < BLUR_BUFFERS && bDoHDR; ++i)
	{
		s_BlurFBO[i].iWidth = (g_iVideoWidth >> DOWNSAMPLE_BUFFERS);
		s_BlurFBO[i].iHeight = (g_iVideoHeight >> DOWNSAMPLE_BUFFERS);

		//fbo
		R_GLGenFrameBuffer(&s_BlurFBO[i]);
		//color
		R_GLFrameBufferColorTexture(&s_BlurFBO[i], iColorInternalFormat);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_BlurFBO[i]);
			bDoHDR = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("Blur FBO #%d rendering disabled due to create error.\n", i);
		}
	}

	if(bDoHDR)
	{
		s_ToneMapFBO.iWidth = g_iVideoWidth;
		s_ToneMapFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_ToneMapFBO);
		//color
		R_GLFrameBufferColorTexture(&s_ToneMapFBO, GL_RGBA8);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_ToneMapFBO);
			bDoHDR = false;
			g_pMetaSave->pEngineFuncs->Con_Printf("Tone mapping FBO #%d rendering disabled due to create error.\n");
		}
	}

	s_CloakFBO.iWidth = g_iVideoWidth;
	s_CloakFBO.iHeight = g_iVideoHeight;

	//fbo
	R_GLGenFrameBuffer(&s_CloakFBO);
	//color
	R_GLFrameBufferColorTexture(&s_CloakFBO, GL_RGBA8);

	if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		GL_FreeFBO(&s_CloakFBO);
		g_pMetaSave->pEngineFuncs->Con_Printf("Cloak FBO rendering disabled due to create error.\n");
	}

	qglBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	readframebuffer = drawframebuffer = 0;
}

void GL_Init(void)
{
	QGL_Init();

	CheckMultiTextureExtensions();

	GL_GenerateFBO();
}

void GL_Shutdown(void)
{
	GL_FreeFBO(&s_MSAAFBO);
	GL_FreeFBO(&s_BackBufferFBO);
	GL_FreeFBO(&s_3DHUDFBO);
	GL_FreeFBO(&s_ShadowFBO);
	GL_FreeFBO(&s_WaterFBO);
	for(int i = 0; i < DOWNSAMPLE_BUFFERS; ++i)
		GL_FreeFBO(&s_DownSampleFBO[i]);
	for(int i = 0; i < BLUR_BUFFERS; ++i)
		GL_FreeFBO(&s_BlurFBO[i]);
	for(int i = 0; i < LUMIN1x1_BUFFERS; ++i)
		GL_FreeFBO(&s_Lumin1x1FBO[i]);
	for(int i = 0; i < BLUR_BUFFERS; ++i)
		GL_FreeFBO(&s_BlurFBO[i]);
	GL_FreeFBO(&s_ToneMapFBO);
}

void GL_BeginRendering(int *x, int *y, int *width, int *height)
{
	static int vsync = -1;
	if (gl_vsync->value != vsync)
	{
		if (qwglSwapIntervalEXT)
		{
			if (gl_vsync->value > 0)
				qwglSwapIntervalEXT(1);
			else
				qwglSwapIntervalEXT(0);
		}

		vsync = gl_vsync->value;
	}

	gRefFuncs.GL_BeginRendering(x, y, width, height);

	screenframebuffer = 0;

	//no MSAA here
	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;
		qglBindFramebufferEXT(GL_FRAMEBUFFER, screenframebuffer);
	}
	
	qglClearColor(0.0, 0.0, 0.0, 1.0);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void R_RenderView(void)
{
	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		if (s_MSAAFBO.s_hBackBufferFBO)
			screenframebuffer = s_MSAAFBO.s_hBackBufferFBO;
		else
			screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;

		qglBindFramebufferEXT(GL_FRAMEBUFFER, screenframebuffer);
	}

	gRefFuncs.R_RenderView();

	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		//Do MSAA here so HUD won't be AA
		if (s_MSAAFBO.s_hBackBufferFBO)
		{
			qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
			qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, s_MSAAFBO.s_hBackBufferFBO);
			qglBlitFramebufferEXT(0, 0, s_MSAAFBO.iWidth, s_MSAAFBO.iHeight, 0, 0, s_BackBufferFBO.iWidth, s_BackBufferFBO.iHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		R_BeginHUDQuad();
		if(bDoHDR && r_hdr->value > 0)
		{
			//normal downsample
			R_DownSample(&s_BackBufferFBO, &s_DownSampleFBO[0], false);//(1->1/4)
			R_DownSample(&s_DownSampleFBO[0], &s_DownSampleFBO[1], false);//(1/4)->(1/16)
			
			//log Luminance DownSample
			R_LuminPass(&s_DownSampleFBO[1], &s_LuminFBO[0], 1);//(1/16)->64x64
			//Luminance DownSample
			R_LuminPass(&s_LuminFBO[0], &s_LuminFBO[1], 0);//64x64->16x16
			R_LuminPass(&s_LuminFBO[1], &s_LuminFBO[2], 0);//16x16->4x4
			R_LuminPass(&s_LuminFBO[2], &s_LuminFBO[3], 0);//4x4->1x1
			//exp Luminance DownSample 4x4->1x1
			R_LuminPass(&s_LuminFBO[2], &s_Lumin1x1FBO[2], 2);
			//Luminance Adaptation
			R_LuminAdaptation(&s_Lumin1x1FBO[2], &s_Lumin1x1FBO[!last_luminance], &s_Lumin1x1FBO[last_luminance], g_flFrameTime);
			last_luminance = !last_luminance;
			//Gaussian blur (1/16) image
			R_PartialBlur(&s_DownSampleFBO[1], &s_BlurFBO[0], false);
			R_PartialBlur(&s_BlurFBO[0], &s_BlurFBO[1], true);
			//Tone mapping
			R_ToneMapping(&s_BackBufferFBO, &s_BlurFBO[1], &s_Lumin1x1FBO[last_luminance], &s_ToneMapFBO);
			
			R_BlitToFBO(&s_ToneMapFBO, &s_BackBufferFBO);
		}
	}

	screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;
	qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, screenframebuffer);
	qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, screenframebuffer);
}

void GL_EndRendering(void)
{
	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);
		qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
		qglClearColor(0, 0, 0, 0);
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int windowW = g_iVideoWidth;
		int windowH = g_iVideoHeight;

		windowW = window_rect->right - window_rect->left;
		windowH = window_rect->bottom - window_rect->top;

		int dstX = 0;
		int dstY = 0;
		int dstX2 = windowW;
		int dstY2 = windowH;

		*videowindowaspect = *windowvideoaspect = 1;

		float videoAspect = (float)g_iVideoWidth / g_iVideoHeight;
		float windowAspect = (float)windowW / windowH;
		if ( bNoStretchAspect )
		{
			if ( windowAspect < videoAspect )
			{
				dstY = (windowH - 1.0 / videoAspect * windowW) / 2.0;
				dstY2 = windowH - dstY;
				if(videowindowaspect)
					*videowindowaspect = videoAspect / windowAspect;
				else
					videowindowaspect_old = videoAspect / windowAspect;
			}
			else
			{
				dstX = (windowW - windowH * videoAspect) / 2.0;
				dstX2 = windowW - dstX;
				if(videowindowaspect)
					*windowvideoaspect = windowAspect / videoAspect;
				else
					windowvideoaspect_old = windowAspect / videoAspect;
			}
		}

		if ( bDoDirectBlit )
		{
			qglBlitFramebufferEXT(0, 0, g_iVideoWidth, g_iVideoHeight, dstX, dstY, dstX2, dstY2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
		else
		{
			qglDisable(GL_BLEND);
			qglDisable(GL_LIGHTING);
			qglDisable(GL_DEPTH_TEST);
			qglDisable(GL_ALPHA_TEST);
			qglDisable(GL_CULL_FACE);
			qglMatrixMode(GL_PROJECTION);
			qglPushMatrix();
			qglLoadIdentity();
			qglOrtho(0.0, windowW, windowH, 0.0, -1.0, 1.0);
			qglMatrixMode(GL_MODELVIEW);
			qglPushMatrix();
			qglLoadIdentity();
			qglViewport(0, 0, windowW, windowH);
			qglEnable(GL_TEXTURE_2D);

			qglColor4f(1, 1, 1, 1);			
			qglBindTexture(GL_TEXTURE_2D, s_BackBufferFBO.s_hBackBufferTex);
			qglBegin(GL_QUADS);
			qglTexCoord2f(0, 1);
			qglVertex3f(dstX, dstY, 0);
			qglTexCoord2f(1, 1);
			qglVertex3f(dstX2, dstY, 0);
			qglTexCoord2f(1, 0);
			qglVertex3f(dstX2, dstY2, 0);
			qglTexCoord2f(0, 0);
			qglVertex3f(dstX, dstY2, 0);
			qglEnd();

			qglBindTexture(GL_TEXTURE_2D, 0);
			qglMatrixMode(GL_PROJECTION);
			qglPopMatrix();
			qglMatrixMode(GL_MODELVIEW);
			qglPopMatrix();
			qglDisable(GL_TEXTURE_2D);
		}
		qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, 0);
	}

	GL_SwapBuffer();
}

void GL_SwapBuffer(void)
{
	if(gRefFuncs.GL_SwapBuffer)
		gRefFuncs.GL_SwapBuffer();
}

void R_InitCvars(void)
{
	static cvar_t s_gl_texsort = { "gl_texsort", "0", 0, 0, 0 };

	r_bmodelinterp = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_bmodelinterp");
	r_bmodelhighfrac = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_bmodelhighfrac");
	r_norefresh = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_norefresh");
	r_drawentities = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_drawentities");
	r_drawviewmodel = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_drawviewmodel");
	r_speeds = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_speeds");
	r_fullbright = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_fullbright");
	r_decals = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_decals");
	r_lightmap = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_lightmap");
	r_shadows = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_shadows");
	r_mirroralpha = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_mirroralpha");
	r_wateralpha = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_wateralpha");
	r_dynamic = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_dynamic");
	r_mmx = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_mmx");
	r_traceglow = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_traceglow");
	r_wadtextures = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_wadtextures");
	r_glowshellfreq = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_glowshellfreq");
	r_novis = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_novis");
	r_novis->flags &= ~FCVAR_SPONLY;

	r_detailtextures = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("r_detailtextures");

	gl_vsync = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_vsync");

	if (!gl_vsync)
		gl_vsync = g_pMetaSave->pEngineFuncs->pfnRegisterVariable("gl_vsync", "1", FCVAR_ARCHIVE);

	gl_ztrick = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_ztrick");

	if (!gl_ztrick)
		gl_ztrick = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_ztrick_old");

	gl_finish = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_finish");
	gl_clear = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_clear");
	gl_cull = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_cull");
	gl_texsort = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_texsort");

	if (!gl_texsort)
		gl_texsort = &s_gl_texsort;

	gl_smoothmodels = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_smoothmodels");
	gl_affinemodels = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_affinemodels");
	gl_flashblend = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_flashblend");
	gl_playermip = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_playermip");
	gl_nocolors = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_nocolors");
	gl_keeptjunctions = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_keeptjunctions");
	gl_reporttjunctions = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_reporttjunctions");
	gl_wateramp = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_wateramp");
	gl_dither = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_dither");
	gl_spriteblend = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_spriteblend");
	gl_polyoffset = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_polyoffset");
	gl_lightholes = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_lightholes");
	gl_zmax = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_zmax");
	gl_alphamin = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_alphamin");
	gl_overdraw = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_overdraw");
	gl_watersides = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_watersides");
	gl_overbright = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_overbright");
	gl_envmapsize = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_envmapsize");
	gl_flipmatrix = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_flipmatrix");
	gl_monolights = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_monolights");
	gl_fog = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_fog");

	gl_wireframe = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_wireframe");
	gl_wireframe->flags &= ~FCVAR_SPONLY;

	gl_round_down = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_round_down");
	gl_picmip = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_picmip");
	gl_max_size = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_max_size");
	gl_max_size->value = gl_max_texture_size;

	developer = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("developer");

	g_pMetaSave->pEngineFuncs->Cvar_SetValue("r_detailtextures", 1);

	gl_ansio = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gl_ansio");
	if (!gl_ansio)
		gl_ansio = g_pMetaSave->pEngineFuncs->pfnRegisterVariable("gl_ansio", "1", FCVAR_ARCHIVE);

	const char *s_ansio;
	gl_force_ansio = 0;
	if(g_pInterface->CommandLine->CheckParm("-ansio", &s_ansio))
	{
		gl_force_ansio = gl_max_ansio;
		if(s_ansio && s_ansio[0] && s_ansio[0] >= '0' && s_ansio[0] <= '9')
		{
			float f_ansio = atof(s_ansio);
			gl_force_ansio = max(min(f_ansio, gl_max_ansio), 1);
		}
	}

	v_lightgamma = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("lightgamma");
	v_brightness = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("brightness");
	v_gamma = g_pMetaSave->pEngineFuncs->pfnGetCvarPointer("gamma");
}

void R_InitScreen(void)
{
	r_refdef->vrect.width = g_iVideoWidth;
	r_refdef->vrect.height = g_iVideoHeight;
}

void R_Init(void)
{
	if(g_dwEngineBuildnum >= 5953)
		gRefFuncs.FreeFBObjects();

	save_refdefstack = 0;

	R_InitCvars();
	R_InitScreen();
	R_InitTextures();
	R_InitShaders();
	R_InitWater();
	R_InitStudio();
	R_InitShadow();
	R_InitWSurf();
	R_InitRefHUD();
	R_Init3DSky();
	R_InitCloak();

	CL_VisEdicts_Patch();
	Lightmaps_Patch();

	Draw_Init();
}

void R_VidInit(void)
{
	memset(&r_params, 0, sizeof(r_params));
	R_ClearWater();
	R_ClearShadow();
	R_Clear3DSky();
}

void R_Shutdown(void)
{
	R_FreeTextures();
	R_FreeShaders();
}

void R_LoadRendererEntities(void)
{
	int i, j;
	char szFileName[256];
	char *pFile;

	numsdmanagers = 0;

	for(i = 0; i < 2; ++i)
	{
		if(i == 0)
		{
			sprintf(szFileName, "resource/renderer_entities.txt");
		}
		else
		{
			strcpy( szFileName, g_pMetaSave->pEngineFuncs->pfnGetLevelName() );
			if ( !strlen(szFileName) )
			{
				g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities failed to GetLevelName.\n");
				continue;
			}
			szFileName[strlen(szFileName)-4] = 0;
			strcat(szFileName, "_rent.txt");
		}

		pFile = (char *)g_pMetaSave->pEngineFuncs->COM_LoadFile(szFileName, 5, NULL);
		if (!pFile)
		{
			g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities failed to load %s.\n", szFileName);
			continue;
		}

		cJSON *root = cJSON_Parse(pFile);
		if (!root)
		{
			g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities failed to parse %s.\n", szFileName);
			continue;
		}
		//ent load
		int numentities = cJSON_GetArraySize(root);
		for(j = 0; j < numentities; ++j)
		{
			cJSON *ent = cJSON_GetArrayItem(root, j);
			if(!ent) continue;

			cJSON *type = cJSON_GetObjectItem(ent, "type");
			if(!type || !type->valuestring)
			{
				g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities parse %s with error, entity #%d has no \"type\" component.\n", szFileName, j);
				continue;
			}

			if(!strcmp(type->valuestring, "3dskybox"))
			{
				cJSON *camera = cJSON_GetObjectItem(ent, "camera");
				
				if(!camera || !camera->valuestring)
				{
					g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities parse %s with error, entity with type \"skycamera\" #%d has no \"camera\" component.\n", szFileName, j);
					continue;
				}
				
				cJSON *center = cJSON_GetObjectItem(ent, "center");
				if(!center || !center->valuestring)
				{
					g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities parse %s with error, entity with type \"skycamera\" #%d has no \"center\" component.\n", szFileName, j);
					continue;
				}
				sscanf(camera->valuestring, "%f %f %f", &_3dsky_camera[0], &_3dsky_camera[1], &_3dsky_camera[2]);

				sscanf(center->valuestring, "%f %f %f", &_3dsky_center[0], &_3dsky_center[1], &_3dsky_center[2]);
				
				cJSON *scale = cJSON_GetObjectItem(ent, "scale");
				if(scale && scale->valuestring)
				{
					_3dsky_scale = atof(scale->valuestring);
				}
				_3dsky_enable = true;
			}

			if(!strcmp(type->valuestring, "shadowmanager"))
			{
				cJSON *affectmodel = cJSON_GetObjectItem(ent, "affectmodel");
				cJSON *radius = cJSON_GetObjectItem(ent, "radius");
				cJSON *fard = cJSON_GetObjectItem(ent, "fard");
				cJSON *scale = cJSON_GetObjectItem(ent, "scale");
				cJSON *texsize = cJSON_GetObjectItem(ent, "texsize");
				cJSON *angles = cJSON_GetObjectItem(ent, "angles");
				if(!affectmodel || !affectmodel->valuestring)
				{
					g_pMetaSave->pEngineFuncs->Con_Printf("R_LoadRendererEntities parse %s with error, entity with type \"shadowmanager\" #%d has no \"affectmodel\" component.\n", szFileName, j);
					continue;
				}
				vec3_t angles2;
				float radius2;
				float fard2;
				float scale2;
				int texsize2;
				if(!angles || !angles->valuestring)
				{
					angles2[0] = 90;
					angles2[1] = 0;
					angles2[2] = 0;
				}
				else
					sscanf(angles->valuestring, "%f %f %f", &angles2[0], &angles2[1], &angles2[2]);

				if(!radius || !radius->valuestring)
					radius2 = 256;
				else
					radius2 = (float)atof(radius->valuestring);

				if(!fard || !fard->valuestring)
					fard2 = 64;
				else
					fard2 = (float)atof(fard->valuestring);
				if(!scale || !scale->valuestring)
					scale2 = 8;
				else
					scale2 = (float)atof(scale->valuestring);
				if(!texsize || !texsize->valuestring)
					texsize2 = 512;
				else
					texsize2 = atoi(texsize->valuestring);
				
				R_CreateShadowManager(affectmodel->valuestring, angles2, radius2, fard2, scale2, texsize2);
			}
		}
		//load end
		cJSON_Delete(root);
		g_pMetaSave->pEngineFuncs->COM_FreeFile((void *) pFile );
	}
}

void R_AllocObjects(int nMax)
{
	if(cl_maxvisedicts == 512)
	{
		gRefFuncs.R_AllocObjects(nMax);
		return;
	}
	if (*transObjects)
		g_pMetaSave->pEngineFuncs->Con_Printf("Transparent objects reallocate\n");

	*transObjects = (transObjRef *)gRefFuncs.Mem_Malloc(cl_maxvisedicts * sizeof(transObjRef));
	memset(*transObjects, 0, cl_maxvisedicts * sizeof(transObjRef));
	*maxTransObjs = cl_maxvisedicts;
	numTransObjs = 0;
}