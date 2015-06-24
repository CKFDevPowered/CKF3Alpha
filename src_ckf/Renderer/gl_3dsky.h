#pragma once

extern vec3_t _3dsky_view;
extern vec3_t _3dsky_mins;
extern vec3_t _3dsky_maxs;
extern float _3dsky_mvmatrix[16];
extern mplane_t _3dsky_frustum[4];
extern qboolean draw3dsky;
extern vec3_t _3dsky_camera;
extern vec3_t _3dsky_center;
extern int _3dsky_enable;
extern float _3dsky_scale;

extern cvar_t *r_3dsky;
extern cvar_t *r_3dsky_debug;

void R_Init3DSky(void);
void R_Clear3DSky(void);
void R_Render3DSky(void);
void R_ViewOriginFor3DSky(float *org);
void R_Draw3DSkyEntities(void);
void R_Add3DSkyEntity(cl_entity_t *ent);
void R_Setup3DSkyModel(void);
void R_Finish3DSkyModel(void);