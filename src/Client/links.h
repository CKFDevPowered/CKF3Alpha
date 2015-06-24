extern void *gpViewPortInterface;

extern bool *g_bGunSmoke;
extern vec3_t *g_pv_angles;

extern void (*g_pfnEV_HLDM_CreateSmoke)(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate);
extern void (*g_pfnEV_HLDM_DecalGunshot)(struct pmtrace_s *pTrace, int iBulletType, float scale, int r, int g, int b, bool bCreateSparks, char cTextureType);
extern void (*g_pfnEV_HLDM_FireBullets)(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, int iPenetration);
extern void (*g_pfnEV_VehiclePitchAdjust)(event_args_t *args);
extern void (*g_pfnEV_RemoveAllDecals)(struct event_args_s *args);
extern void (*g_pfnEV_CreateSmoke)(struct event_args_s *args);
extern void (*g_pfnEV_EjectBrass)(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity);

extern float *(*g_pfnGetClientColor)(int clientIndex);

extern void (__fastcall *g_pfnCCounterStrikeViewport_StartProgressBar)(void *pthis, int, const char *title, int numTicks, int startTicks, bool isTimeBased);
extern void (__fastcall *g_pfnCCounterStrikeViewport_UpdateProgressBar)(void *pthis, int, const char *statusText, int tick);
extern void (__fastcall *g_pfnCCounterStrikeViewport_StopProgressBar)(void *pthis, int);

extern class CStudioModelRenderer *g_pStudioRenderer;

extern int CL_IsNewClient(void);
extern void CL_LinkNewClient(unsigned long dwBaseAddress);