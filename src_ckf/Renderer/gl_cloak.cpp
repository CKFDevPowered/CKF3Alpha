#include "gl_local.h"
#include "screen.h"

int cloak_texture = 0;
int cloak_program = 0;

cloak_uniform_t cloak_uniform;

cvar_t *r_cloak_debug;

void R_InitCloak(void)
{
	if(gl_shader_support)
	{
		const char *cloak_vscode = (const char *)g_pMetaSave->pEngineFuncs->COM_LoadFile("resource\\shader\\cloak_shader.vsh", 5, 0);
		const char *cloak_fscode = (const char *)g_pMetaSave->pEngineFuncs->COM_LoadFile("resource\\shader\\cloak_shader.fsh", 5, 0);
		if(cloak_vscode && cloak_fscode)
		{
			cloak_program = R_CompileShader(cloak_vscode, cloak_fscode, "cloak_shader.vsh", "cloak_shader.fsh");
			if(cloak_program)
			{
				SHADER_UNIFORM_INIT(cloak, refract, "refract");
				SHADER_UNIFORM_INIT(cloak, eyepos, "eyepos");
				SHADER_UNIFORM_INIT(cloak, cloakfactor, "cloakfactor");
				SHADER_UNIFORM_INIT(cloak, refractamount, "refractamount");
			}
		}
		g_pMetaSave->pEngineFuncs->COM_FreeFile((void *)cloak_vscode);
		g_pMetaSave->pEngineFuncs->COM_FreeFile((void *)cloak_fscode);
	}

	if(!s_CloakFBO.s_hBackBufferFBO)
	{
		cloak_texture = GL_GenTexture();
		GL_Bind(cloak_texture);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	else
	{
		cloak_texture = s_CloakFBO.s_hBackBufferTex;
	}

	r_cloak_debug = g_pMetaSave->pEngineFuncs->pfnRegisterVariable("r_cloak_debug", "0", FCVAR_CLIENTDLL);
}

void R_RenderCloakTexture(void)
{
	if(s_CloakFBO.s_hBackBufferFBO && s_BackBufferFBO.s_hBackBufferFBO)
	{
		if(s_MSAAFBO.s_hBackBufferFBO)
			R_GLBindFrameBuffer(GL_READ_FRAMEBUFFER, s_MSAAFBO.s_hBackBufferFBO);
		else
			R_GLBindFrameBuffer(GL_READ_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);

		R_GLBindFrameBuffer(GL_DRAW_FRAMEBUFFER, s_CloakFBO.s_hBackBufferFBO);

		qglBlitFramebufferEXT(0, 0, glwidth, glheight, 0, 0, s_CloakFBO.iWidth, s_CloakFBO.iHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		R_GLBindFrameBuffer(GL_FRAMEBUFFER, lastframebuffer);
	}
	else
	{
		GL_Bind(cloak_texture);
		qglEnable(GL_TEXTURE_2D);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, glwidth, glheight);
	}
}