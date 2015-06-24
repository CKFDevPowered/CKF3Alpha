#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include <demo_api.h>
#include "demo.h"
#include "configs.h"
#include "msghook.h"
#include "modules.h"
#include <cl_entity.h>
#include "game_shared/voice_status.h"
#include <event_args.h>
#include <pmtrace.h>
#include <usercmd.h>
#include "kbutton.h"

#include "CounterStrikeViewport.h"

BOOL g_bIsNewClient = FALSE;

void *gpViewPortInterface = (void *)0x1A3A7B8;

extra_player_info_t *g_PlayerExtraInfo = (extra_player_info_t *)0x1A25618;
hostage_info_t *g_HostageInfo = (hostage_info_t *)0x19F73E8;

bool *g_bGunSmoke = (bool *)0x1A1C1A8;
vec3_t *g_pv_angles = (vec3_t *)0x1A34008;

void (*g_pfnEV_HLDM_CreateSmoke)(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate) = (void (*)(float *, float *, int, float, int, int, int, int, float *, bool, int))0x19017D0;
void (*g_pfnEV_HLDM_DecalGunshot)(pmtrace_t *pTrace, int iBulletType, float scale, int r, int g, int b, bool bCreateSparks, char cTextureType) = (void (*)(pmtrace_t *, int, float, int, int, int, bool, char))0x19020B0;
void (*g_pfnEV_HLDM_FireBullets)(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, int iPenetration) = (void (*)(int, float *, float *, float *, int, float *, float *, float *, float, int, int, int *, int))0x1902460;
void (*g_pfnEV_VehiclePitchAdjust)(event_args_t *args) = (void (*)(event_args_t *))0x1904380;
void (*g_pfnEV_RemoveAllDecals)(struct event_args_s *args) = (void (*)(struct event_args_s *))0x190A290;
void (*g_pfnEV_CreateSmoke)(struct event_args_s *args) = (void (*)(struct event_args_s *))0x190A080;
void (*g_pfnEV_EjectBrass)(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity) = (void (*)(float *, float *, float, int, int, int, int))0x1945180;

float *(*g_pfnGetClientColor)(int clientIndex) = (float *(*)(int))0x1943360;

void (__fastcall *g_pfnCCounterStrikeViewport_StartProgressBar)(void *pthis, int, const char *title, int numTicks, int startTicks, bool isTimeBased) = (void (__fastcall *)(void *, int, const char *, int, int, bool))0x1927370;
void (__fastcall *g_pfnCCounterStrikeViewport_UpdateProgressBar)(void *pthis, int, const char *statusText, int tick) = (void (__fastcall *)(void *, int, const char *, int))0x1927420;
void (__fastcall *g_pfnCCounterStrikeViewport_StopProgressBar)(void *pthis, int) = (void (__fastcall *)(void *, int))0x1927430;

class CStudioModelRenderer *g_pStudioRenderer = (CStudioModelRenderer *)0x1A20F90;

int CL_IsNewClient(void)
{
	return g_bIsNewClient;
}

void CL_LinkNewClient(unsigned long dwBaseAddress)
{
	g_bIsNewClient = TRUE;

	gpViewPortInterface = (void *)(dwBaseAddress + 0x1439E8);
	g_PlayerExtraInfo = (extra_player_info_t *)(dwBaseAddress + 0x12E3C0);
	g_HostageInfo = (hostage_info_t *)(dwBaseAddress + 0xFFD98);

	g_bGunSmoke = (bool *)(dwBaseAddress + 0x124C30);
	g_pv_angles = (vec3_t *)(dwBaseAddress + 0x13CE20);

	g_pfnEV_HLDM_CreateSmoke = (void (*)(float *, float *, int, float, int, int, int, int, float *, bool, int))(dwBaseAddress + 0x17D0);
	g_pfnEV_HLDM_DecalGunshot = (void (*)(pmtrace_t *, int, float, int, int, int, bool, char))(dwBaseAddress + 0x20B0);
	g_pfnEV_HLDM_FireBullets = (void (*)(int, float *, float *, float *, int, float *, float *, float *, float, int, int, int *, int))(dwBaseAddress + 0x2460);
	g_pfnEV_VehiclePitchAdjust = (void (*)(event_args_t *))(dwBaseAddress + 0x4380);
	g_pfnEV_RemoveAllDecals = (void (*)(struct event_args_s *))(dwBaseAddress + 0xA290);
	g_pfnEV_CreateSmoke = (void (*)(struct event_args_s *))(dwBaseAddress + 0xA080);
	g_pfnEV_EjectBrass = (void (*)(float *, float *, float, int, int, int, int))(dwBaseAddress + 0x45F70);

	g_pfnGetClientColor = (float *(*)(int))(dwBaseAddress + 0x444B0);

	g_pfnCCounterStrikeViewport_StartProgressBar = (void (__fastcall *)(void *, int, const char *, int, int, bool))(dwBaseAddress + 0x27080);
	g_pfnCCounterStrikeViewport_UpdateProgressBar = (void (__fastcall *)(void *, int, const char *, int))(dwBaseAddress + 0x27130);
	g_pfnCCounterStrikeViewport_StopProgressBar = (void (__fastcall *)(void *, int))(dwBaseAddress + 0x27140);

	g_pStudioRenderer = (CStudioModelRenderer *)(dwBaseAddress + 0x29A28);
}