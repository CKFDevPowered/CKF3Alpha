#include "gl_local.h"
#include "screen.h"

qboolean refract;
float refract_texture_width;
float refract_texture_height;
float refract_texture_minsize;
byte *refract_pixels;
byte *depth_pixels;

void R_SetupRefract(void)
{
	R_SaveRefDef();
	R_UpdateRefDef();

	R_SetupGL();

	qglViewport(0, 0, refract_texture_width, refract_texture_height);

	r_refdef->onlyClientDraws = true;
	refract = true;

	R_SetupClip(false);

	R_RenderWaterFog();

	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curwater->refract, 0);
	}

	qglClearColor(waterfog_col[0], waterfog_col[1], waterfog_col[2], 1);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void R_FinishRefract(void)
{
	R_FinalWaterFog();

	qglDisable(GL_CLIP_PLANE0);

	if(!s_BackBufferFBO.s_hBackBufferFBO)
	{
		GL_Bind(curwater->refract);
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, refract_texture_width, refract_texture_height, 0);
	}

	R_RestoreRefDef();

	r_refdef->onlyClientDraws = false;
	refract = false;
}