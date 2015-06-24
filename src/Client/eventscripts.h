#if !defined (EVENTSCRIPTSH)
#define EVENTSCRIPTSH

#define DEFAULT_VIEWHEIGHT 28
#define VEC_DUCK_VIEW 12

#define FTENT_FADEOUT 0x00000080

void EV_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype);
void EV_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity);
void EV_GetGunPosition(struct event_args_s *args, float *pos, float *origin);
void EV_GetDefaultShellInfo(struct event_args_s *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale);
void EV_GetDefaultShellInfo(struct event_args_s *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection);
qboolean EV_IsLocal(int idx);
qboolean EV_IsPlayer(int idx);
void EV_CreateTracer(float *start, float *end);
struct cl_entity_s *GetEntity(int idx);
struct cl_entity_s *GetViewEntity(void);
void EV_MuzzleFlash(void);

extern void (*g_pfnEV_FireElite)(struct event_args_s *args);

extern "C"
{
void EV_FireGlock1(struct event_args_s *args);
void EV_FireGlock2(struct event_args_s *args);
void EV_FireShotGunSingle(struct event_args_s *args);
void EV_FireShotGunDouble(struct event_args_s *args);
void EV_FireMP5(struct event_args_s *args);
void EV_FireMP52(struct event_args_s *args);
void EV_FirePython(struct event_args_s *args);
void EV_FireGauss(struct event_args_s *args);
void EV_SpinGauss(struct event_args_s *args);
void EV_Crowbar(struct event_args_s *args);
void EV_FireCrossbow(struct event_args_s *args);
void EV_FireCrossbow2(struct event_args_s *args);
void EV_FireRpg(struct event_args_s *args);
void EV_EgonFire(struct event_args_s *args);
void EV_EgonStop(struct event_args_s *args);
void EV_HornetGunFire(struct event_args_s *args);
void EV_TripmineFire(struct event_args_s *args);
void EV_SnarkFire(struct event_args_s *args);

void EV_FireUSP(struct event_args_s *args);
void EV_FireMP5N(struct event_args_s *args);
void EV_FireAK47(struct event_args_s *args);
void EV_FireAug(struct event_args_s *args);
void EV_FireDeagle(struct event_args_s *args);
void EV_FireG3SG1(struct event_args_s *args);
void EV_FireSG550(struct event_args_s *args);
void EV_FireGlock18(struct event_args_s *args);
void EV_FireM249(struct event_args_s *args);
void EV_FireM3(struct event_args_s *args);
void EV_FireM4A1(struct event_args_s *args);
void EV_FireMac10(struct event_args_s *args);
void EV_FireP90(struct event_args_s *args);
void EV_FireP228(struct event_args_s *args);
void EV_FireAWP(struct event_args_s *args);
void EV_FireScout(struct event_args_s *args);
void EV_FireSG552(struct event_args_s *args);
void EV_FireTMP(struct event_args_s *args);
void EV_FireFiveSeven(struct event_args_s *args);
void EV_FireUMP45(struct event_args_s *args);
void EV_FireXM1014(struct event_args_s *args);
void EV_FireEliteLeft(struct event_args_s *args);
void EV_FireEliteRight(struct event_args_s *args);
void EV_Knife(struct event_args_s *args);
void EV_FireGalil(struct event_args_s *args);
void EV_FireFamas(struct event_args_s *args);

void EV_RemoveAllDecals(struct event_args_s *args);
void EV_CreateSmoke(struct event_args_s *args);

void EV_TrainPitchAdjust(struct event_args_s *args);
void EV_VehiclePitchAdjust(struct event_args_s *args);
}

class Script *EVS_GetScript(const char *name);
class Script *EVS_LoadScript(const char *name);
void EVS_ReleaseAllScript(void);

#endif