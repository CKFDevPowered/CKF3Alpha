#pragma once

SHADER_UNIFORM_BEGINDEF(cloak)
{
	int refract;
	int eyepos;
	int cloakfactor;
	int refractamount;	
}
SHADER_UNIFORM_ENDDEF(cloak)

extern int cloak_program;
extern int cloak_texture;
extern cloak_uniform_t cloak_uniform;
extern cvar_t *r_cloak_debug;

SHADER_UNIFORM_BEGINDEF(conc)
{
	int refractmap;
	int normalmap;
	int packedfactor;
}
SHADER_UNIFORM_ENDDEF(conc)

void R_InitCloak(void);
void R_RenderCloakTexture(void);
void R_BeginRenderConc(float flBlurFactor, float flRefractFactor);
int R_GetCloakTexture(void);