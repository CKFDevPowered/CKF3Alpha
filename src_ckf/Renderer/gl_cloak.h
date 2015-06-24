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

void R_InitCloak(void);
void R_RenderCloakTexture(void);