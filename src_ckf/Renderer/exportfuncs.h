#pragma once

void BTE_Init(void);

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void HUD_Init(void);
int HUD_VidInit(void);
void HUD_Reset(void);
void V_CalcRefdef(struct ref_params_s *pparams);
void HUD_DrawNormalTriangles(void);
void HUD_DrawTransparentTriangles(void);
int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
int HUD_UpdateClientData(client_data_t *pcldata, float flTime);
void HUD_Shutdown(void);
int HUD_AddEntity(int type, cl_entity_t *ent, const char *model);
void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int (*pfnAddVisibleEntity)(cl_entity_t *),	void (*pfnTempEntPlaySound)( TEMPENTITY *, float damp));
int HUD_Redraw(float time, int intermission);
void HUD_CreateEntities(void);
void HUD_Frame(double time);

void Sys_ErrorEx(const char *fmt, ...);
char *UTIL_VarArgs(char *format, ...);

#define GetCallAddress(addr) (addr + (*(DWORD *)((addr)+1)) + 5)

#define SIG_NOT_FOUND(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum£º%d", name, g_dwEngineBuildnum);