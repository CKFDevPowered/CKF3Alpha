#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"

#include "r_efx.h"

#include "eventscripts.h"
#include "event_api.h"
#include "pm_shared.h"

#include "pmtrace.h"
#include "ev_hldm.h"
#include "client.h"
#include "scripts.h"
#include "perf_counter.h"
#include "configs.h"
#include "plugins.h"
#include "custom.h"
#include "developer.h"

#include <UtlVector.h>

extern "C"
{
#include "tcc/tcc.h"
#include "tcc/libtcc.h"
}

#define EVENTSCRIPT_INTERFACE_VERSION "EventScript001"

extern int g_iShotsFired;
extern bool *g_bGunSmoke;

extern int tracerCount[32];

cl_entity_t *EVS_GetViewEntity(void)
{
	return GetViewEntity();
}

void EVS_VectorCopy(float *in, float *out)
{
	VectorCopy(in, out);
}

void EVS_AngleVectors(const float *vecAngles, float *forward, float *right, float *up)
{
	gEngfuncs.pfnAngleVectors(vecAngles, forward, right, up);
}

int EVS_FindModelIndex(const char *pmodel)
{
	return gEngfuncs.pEventAPI->EV_FindModelIndex(pmodel);
}

bool EVS_IsLocal(int idx)
{
	return EV_IsLocal(idx);
}

bool EVS_UseLeftHand(void)
{
	return cl_righthand->value != 0;
}

void EVS_ShotsFired(void)
{
	g_iShotsFired++;
}

void EVS_MuzzleFlash(void)
{
	EV_MuzzleFlash();
}

void EVS_WeaponAnimation(int sequence)
{
	gEngfuncs.pEventAPI->EV_WeaponAnimation(sequence, 2);
}

void EVS_CreateSmoke(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate)
{
	if (*g_bGunSmoke)
		EV_HLDM_CreateSmoke(origin, dir, speed, scale, r, g, b, iSmokeType, base_velocity, bWind, framerate);
}

void EVS_GetDefaultShellInfo(event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection)
{
	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, forwardScale, upScale, rightScale, bReverseDirection);
}

void EVS_VectorScale(float *in, float scale, float *out)
{
	VectorScale(in, scale, out);
}

void EVS_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity)
{
	EV_EjectBrass(origin, velocity, rotation, model, soundtype, idx, angle_velocity);
}

void EVS_PlaySound(int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch)
{
	gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, channel, sample, volume, attenuation, fFlags, pitch);
}

void EVS_GetGunPosition(event_args_t *args, float *pos, float *origin)
{
	EV_GetGunPosition(args, pos, origin);
}

void EVS_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iPenetration)
{
	EV_HLDM_FireBullets(idx, forward, right, up, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, 0, &tracerCount[idx - 1], iPenetration);
}

long EVS_RandomLong(long lLow, long lHigh)
{
	return gEngfuncs.pfnRandomLong(lLow, lHigh);
}

float EVS_RandomFloat(float flLow, float flHigh)
{
	return gEngfuncs.pfnRandomFloat(flLow, flHigh);
}

void EVS_ConsolePrintf(char *fmt, ...)
{
	va_list argptr;
	static char text[1024];

	va_start(argptr, fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	gEngfuncs.Con_Printf(text);
}

void EVS_ConsoleDPrintf(char *fmt, ...)
{
	va_list argptr;
	static char text[1024];

	va_start(argptr, fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	gEngfuncs.Con_DPrintf(text);
}

particle_t *EVS_AllocParticle(void (*callback)(struct particle_s *particle, float frametime))
{
	return gEngfuncs.pEfxAPI->R_AllocParticle(callback);
}

void EVS_BlobExplosion(float *org)
{
	gEngfuncs.pEfxAPI->R_BlobExplosion(org);
}

void EVS_Blood(float *org, float *dir, int pcolor, int speed)
{
	gEngfuncs.pEfxAPI->R_Blood(org, dir, pcolor, speed);
}

void EVS_BloodSprite(float *org, int colorindex, int modelIndex, int modelIndex2, float size)
{
	gEngfuncs.pEfxAPI->R_BloodSprite(org, colorindex, modelIndex, modelIndex2, size);
}

void EVS_BloodStream(float *org, float *dir, int pcolor, int speed)
{
	gEngfuncs.pEfxAPI->R_BloodStream(org, dir, pcolor, speed);
}

void EVS_BreakModel(float *pos, float *size, float *dir, float random, float life, int count, int modelIndex, char flags)
{
	gEngfuncs.pEfxAPI->R_BreakModel(pos, size, dir, random, life, count, modelIndex, flags);
}

void EVS_Bubbles(float *mins, float *maxs, float height, int modelIndex, int count, float speed)
{
	gEngfuncs.pEfxAPI->R_Bubbles(mins, maxs, height, modelIndex, count, speed);
}

void EVS_BubbleTrail(float *start, float *end, float height, int modelIndex, int count, float speed)
{
	gEngfuncs.pEfxAPI->R_BubbleTrail(start, end, height, modelIndex, count, speed);
}

void EVS_BulletImpactParticles(float *pos)
{
	gEngfuncs.pEfxAPI->R_BulletImpactParticles(pos);
}

void EVS_EntityParticles(struct cl_entity_s *ent)
{
	gEngfuncs.pEfxAPI->R_EntityParticles(ent);
}

void EVS_Explosion(float *pos, int model, float scale, float framerate, int flags)
{
	gEngfuncs.pEfxAPI->R_Explosion(pos, model, scale, framerate, flags);
}

void EVS_FizzEffect(struct cl_entity_s *pent, int modelIndex, int density)
{
	gEngfuncs.pEfxAPI->R_FizzEffect(pent, modelIndex, density);
}

void EVS_FireField(float *org, int radius, int modelIndex, int count, int flags, float life)
{
	gEngfuncs.pEfxAPI->R_FireField(org, radius, modelIndex, count, flags, life);
}

void EVS_FlickerParticles(float *org)
{
	gEngfuncs.pEfxAPI->R_FlickerParticles(org);
}

void EVS_FunnelSprite(float *org, int modelIndex, int reverse)
{
	gEngfuncs.pEfxAPI->R_FunnelSprite(org, modelIndex, reverse);
}

void EVS_Implosion(float *end, float radius, int count, float life)
{
	gEngfuncs.pEfxAPI->R_Implosion(end, radius, count, life);
}

void EVS_LargeFunnel(float *org, int reverse)
{
	gEngfuncs.pEfxAPI->R_LargeFunnel(org, reverse);
}

void EVS_LavaSplash(float *org)
{
	gEngfuncs.pEfxAPI->R_LavaSplash(org);
}

void EVS_MultiGunshot(float *org, float *dir, float *noise, int count, int decalCount, int *decalIndices)
{
	gEngfuncs.pEfxAPI->R_MultiGunshot(org, dir, noise, count, decalCount, decalIndices);
}

void EVS_ParticleBox(float *mins, float *maxs, unsigned char r, unsigned char g, unsigned char b, float life)
{
	gEngfuncs.pEfxAPI->R_ParticleBox(mins, maxs, r, r, b, life);
}

void EVS_ParticleBurst(float *pos, int size, int color, float life)
{
	gEngfuncs.pEfxAPI->R_ParticleBurst(pos, size, color, life);
}

void EVS_ParticleExplosion(float *org)
{
	gEngfuncs.pEfxAPI->R_ParticleExplosion(org);
}

void EVS_ParticleExplosion2(float *org, int colorStart, int colorLength)
{
	gEngfuncs.pEfxAPI->R_ParticleExplosion2(org, colorStart, colorLength);
}

void EVS_ParticleLine(float *start, float *end, unsigned char r, unsigned char g, unsigned char b, float life)
{
	gEngfuncs.pEfxAPI->R_ParticleLine(start, end, r, g, b, life);
}

void EVS_PlayerSprites(int client, int modelIndex, int count, int size)
{
	gEngfuncs.pEfxAPI->R_PlayerSprites(client, modelIndex, count, size);
}

void EVS_Projectile(float *origin, float *velocity, int modelIndex, int life, int owner, void (*hitcallback)(struct tempent_s *ent, struct pmtrace_s *ptr))
{
	gEngfuncs.pEfxAPI->R_Projectile(origin, velocity, modelIndex, life, owner, hitcallback);
}

void EVS_RicochetSound(float *pos)
{
	gEngfuncs.pEfxAPI->R_RicochetSound(pos);
}

void EVS_RicochetSprite(float *pos, struct model_s *pmodel, float duration, float scale)
{
	gEngfuncs.pEfxAPI->R_RicochetSprite(pos, pmodel, duration, scale);
}

void EVS_RocketFlare(float *pos)
{
	gEngfuncs.pEfxAPI->R_RocketFlare(pos);
}

void EVS_RocketTrail(float *start, float *end, int type)
{
	gEngfuncs.pEfxAPI->R_RocketTrail(start, end, type);
}

void EVS_RunParticleEffect(float *org, float *dir, int color, int count)
{
	gEngfuncs.pEfxAPI->R_RunParticleEffect(org, dir, color, count);
}

void EVS_ShowLine(float *start, float *end)
{
	gEngfuncs.pEfxAPI->R_ShowLine(start, end);
}

void EVS_SparkEffect(float *pos, int count, int velocityMin, int velocityMax)
{
	gEngfuncs.pEfxAPI->R_SparkEffect(pos, count, velocityMin, velocityMax);
}

void EVS_SparkShower(float *pos)
{
	gEngfuncs.pEfxAPI->R_SparkShower(pos);
}

void EVS_SparkStreaks(float *pos, int count, int velocityMin, int velocityMax)
{
	gEngfuncs.pEfxAPI->R_SparkStreaks(pos, count, velocityMin, velocityMax);
}

void EVS_Spray(float *pos, float *dir, int modelIndex, int count, int speed, int spread, int rendermode)
{
	gEngfuncs.pEfxAPI->R_Spray(pos, dir, modelIndex, count, speed, spread, rendermode);
}

void EVS_Sprite_Explode(TEMPENTITY *pTemp, float scale, int flags)
{
	gEngfuncs.pEfxAPI->R_Sprite_Explode(pTemp, scale, flags);
}

void EVS_Sprite_Smoke(TEMPENTITY *pTemp, float scale)
{
	gEngfuncs.pEfxAPI->R_Sprite_Smoke(pTemp, scale);
}

void EVS_Sprite_Spray(float *pos, float *dir, int modelIndex, int count, int speed, int iRand)
{
	gEngfuncs.pEfxAPI->R_Sprite_Spray(pos, dir, modelIndex, count, speed, iRand);
}

void EVS_Sprite_Trail(int type, float *start, float *end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed)
{
	gEngfuncs.pEfxAPI->R_Sprite_Trail(type, start, end, modelIndex, count, life, size, amplitude, renderamt, speed);
}

void EVS_Sprite_WallPuff(TEMPENTITY *pTemp, float scale)
{
	gEngfuncs.pEfxAPI->R_Sprite_WallPuff(pTemp, scale);
}

void EVS_StreakSplash(float *pos, float *dir, int color, int count, float speed, int velocityMin, int velocityMax)
{
	gEngfuncs.pEfxAPI->R_StreakSplash(pos, dir, color, count, speed, velocityMin, velocityMax);
}

void EVS_TracerEffect(float *start, float *end)
{
	gEngfuncs.pEfxAPI->R_TracerEffect(start, end);
}

void EVS_UserTracerParticle(float *org, float *vel, float life, int colorIndex, float length, unsigned char deathcontext, void (*deathfunc)(struct particle_s *particle))
{
	gEngfuncs.pEfxAPI->R_UserTracerParticle(org, vel, life, colorIndex, length, deathcontext, deathfunc);
}

particle_t *EVS_TracerParticles(float *org, float *vel, float life)
{
	return gEngfuncs.pEfxAPI->R_TracerParticles(org, vel, life);
}

void EVS_TeleportSplash(float *org)
{
	gEngfuncs.pEfxAPI->R_TeleportSplash(org);
}

void EVS_TempSphereModel(float *pos, float speed, float life, int count, int modelIndex)
{
	gEngfuncs.pEfxAPI->R_TempSphereModel(pos, speed, life, count, modelIndex);
}

TEMPENTITY *EVS_TempModel(float *pos, float *dir, float *angles, float life, int modelIndex, int soundtype)
{
	return gEngfuncs.pEfxAPI->R_TempModel(pos, dir, angles, life, modelIndex, soundtype);
}

TEMPENTITY *EVS_DefaultSprite(float *pos, int spriteIndex, float framerate)
{
	return gEngfuncs.pEfxAPI->R_DefaultSprite(pos, spriteIndex, framerate);
}

TEMPENTITY *EVS_TempSprite(float *pos, float *dir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags)
{
	return gEngfuncs.pEfxAPI->R_TempSprite(pos, dir, scale, modelIndex, rendermode, renderfx, a, life, flags);
}

int EVS_DecalIndex(int id)
{
	return gEngfuncs.pEfxAPI->Draw_DecalIndex(id);
}

int EVS_DecalIndexFromName(char *name)
{
	return gEngfuncs.pEfxAPI->Draw_DecalIndexFromName(name);
}

void EVS_DecalShoot(int textureIndex, int entity, int modelIndex, float *position, int flags)
{
	gEngfuncs.pEfxAPI->R_DecalShoot(textureIndex, entity, modelIndex, position, flags);
}

void EVS_AttachTentToPlayer(int client, int modelIndex, float zoffset, float life)
{
	gEngfuncs.pEfxAPI->R_AttachTentToPlayer(client, modelIndex, zoffset, life);
}

void EVS_KillAttachedTents(int client)
{
	gEngfuncs.pEfxAPI->R_KillAttachedTents(client);
}

BEAM *EVS_BeamCirclePoints(int type, float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b)
{
	return gEngfuncs.pEfxAPI->R_BeamCirclePoints(type, start, end, modelIndex, life, width, amplitude, brightness, speed, startFrame, framerate, r, g, b);
}

BEAM *EVS_BeamEntPoint(int startEnt, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b)
{
	return gEngfuncs.pEfxAPI->R_BeamEntPoint(startEnt, end, modelIndex, life, width, amplitude, brightness, speed, startFrame, framerate, r, g, b);
}

BEAM *EVS_BeamEnts(int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b)
{
	return gEngfuncs.pEfxAPI->R_BeamEnts(startEnt, endEnt, modelIndex, life, width, amplitude, brightness, speed, startFrame, framerate, r, g, b);
}

BEAM *EVS_BeamFollow(int startEnt, int modelIndex, float life, float width, float r, float g, float b, float brightness)
{
	return gEngfuncs.pEfxAPI->R_BeamFollow(startEnt, modelIndex, life, width, r, g, b, brightness);
}

void EVS_BeamKill(int deadEntity)
{
	gEngfuncs.pEfxAPI->R_BeamKill(deadEntity);
}

BEAM *EVS_BeamLightning(float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed)
{
	return gEngfuncs.pEfxAPI->R_BeamLightning(start, end, modelIndex, life, width, amplitude, brightness, speed);
}

BEAM *EVS_BeamPoints(float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b)
{
	return gEngfuncs.pEfxAPI->R_BeamPoints(start, end, modelIndex, life, width, amplitude, brightness, speed, startFrame, framerate, r, g, b);
}

BEAM *EVS_BeamRing(int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b)
{
	return gEngfuncs.pEfxAPI->R_BeamRing(startEnt, endEnt, modelIndex, life, width, amplitude, brightness, speed, startFrame, framerate, r, g, b);
}

dlight_t *EVS_AllocDlight(int key)
{
	return gEngfuncs.pEfxAPI->CL_AllocDlight(key);
}

dlight_t *EVS_AllocElight(int key)
{
	return gEngfuncs.pEfxAPI->CL_AllocElight(key);
}

TEMPENTITY *EVS_TempEntAlloc(float *org, struct model_s *model)
{
	return gEngfuncs.pEfxAPI->CL_TempEntAlloc(org, model);
}

TEMPENTITY *EVS_TempEntAllocNoModel(float *org)
{
	return gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(org);
}

TEMPENTITY *EVS_TempEntAllocHigh(float *org, struct model_s *model)
{
	return gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(org, model);
}

TEMPENTITY *EVS_TentEntAllocCustom(float *origin, struct model_s *model, int high, void (*callback)(struct tempent_s *ent, float frametime, float currenttime))
{
	return gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, model, high, callback);
}

void EVS_GetPackedColor(short *packed, short color)
{
	gEngfuncs.pEfxAPI->R_GetPackedColor(packed, color);
}

short EVS_LookupColor(unsigned char r, unsigned char g, unsigned char b)
{
	return gEngfuncs.pEfxAPI->R_LookupColor(r, g, b);
}

void EVS_DecalRemoveAll(int textureIndex)
{
	return gEngfuncs.pEfxAPI->R_DecalRemoveAll(textureIndex);
}

DWORD EVS_GetProcAddress(char *pszProcName)
{
#define DESCRIBE_PROC(a) \
	if (!strcmp(pszProcName, #a)) \
		return (DWORD)EVS_##a; \

	DESCRIBE_PROC(GetViewEntity);
	DESCRIBE_PROC(VectorCopy);
	DESCRIBE_PROC(AngleVectors);
	DESCRIBE_PROC(FindModelIndex);
	DESCRIBE_PROC(IsLocal);
	DESCRIBE_PROC(UseLeftHand);
	DESCRIBE_PROC(ShotsFired);
	DESCRIBE_PROC(MuzzleFlash);
	DESCRIBE_PROC(WeaponAnimation);
	DESCRIBE_PROC(CreateSmoke);
	DESCRIBE_PROC(GetDefaultShellInfo);
	DESCRIBE_PROC(VectorScale);
	DESCRIBE_PROC(EjectBrass);
	DESCRIBE_PROC(PlaySound);
	DESCRIBE_PROC(GetGunPosition);
	DESCRIBE_PROC(FireBullets);
	DESCRIBE_PROC(RandomLong);
	DESCRIBE_PROC(RandomFloat);
	DESCRIBE_PROC(ConsolePrintf);
	DESCRIBE_PROC(ConsoleDPrintf);

	DESCRIBE_PROC(AllocParticle);
	DESCRIBE_PROC(BlobExplosion);
	DESCRIBE_PROC(Blood);
	DESCRIBE_PROC(BloodSprite);
	DESCRIBE_PROC(BloodStream);
	DESCRIBE_PROC(BreakModel);
	DESCRIBE_PROC(Bubbles);
	DESCRIBE_PROC(BubbleTrail);
	DESCRIBE_PROC(BulletImpactParticles);
	DESCRIBE_PROC(EntityParticles);
	DESCRIBE_PROC(Explosion);
	DESCRIBE_PROC(FizzEffect);
	DESCRIBE_PROC(FireField);
	DESCRIBE_PROC(FlickerParticles);
	DESCRIBE_PROC(FunnelSprite);
	DESCRIBE_PROC(Implosion);
	DESCRIBE_PROC(LargeFunnel);
	DESCRIBE_PROC(LavaSplash);
	DESCRIBE_PROC(MultiGunshot);
	DESCRIBE_PROC(ParticleBox);
	DESCRIBE_PROC(ParticleBurst);
	DESCRIBE_PROC(ParticleExplosion);
	DESCRIBE_PROC(ParticleExplosion2);
	DESCRIBE_PROC(ParticleLine);
	DESCRIBE_PROC(PlayerSprites);
	DESCRIBE_PROC(Projectile);
	DESCRIBE_PROC(RicochetSound);
	DESCRIBE_PROC(RicochetSprite);
	DESCRIBE_PROC(RocketFlare);
	DESCRIBE_PROC(RocketTrail);
	DESCRIBE_PROC(RunParticleEffect);
	DESCRIBE_PROC(ShowLine);
	DESCRIBE_PROC(SparkEffect);
	DESCRIBE_PROC(SparkShower);
	DESCRIBE_PROC(SparkStreaks);
	DESCRIBE_PROC(Spray);
	DESCRIBE_PROC(Sprite_Explode);
	DESCRIBE_PROC(Sprite_Smoke);
	DESCRIBE_PROC(Sprite_Spray);
	DESCRIBE_PROC(Sprite_Trail);
	DESCRIBE_PROC(Sprite_WallPuff);
	DESCRIBE_PROC(StreakSplash);
	DESCRIBE_PROC(TracerEffect);
	DESCRIBE_PROC(UserTracerParticle);
	DESCRIBE_PROC(TracerParticles);
	DESCRIBE_PROC(TeleportSplash);
	DESCRIBE_PROC(TempSphereModel);
	DESCRIBE_PROC(TempModel);
	DESCRIBE_PROC(DefaultSprite);
	DESCRIBE_PROC(TempSprite);
	DESCRIBE_PROC(DecalIndexFromName);
	DESCRIBE_PROC(DecalShoot);
	DESCRIBE_PROC(AttachTentToPlayer);
	DESCRIBE_PROC(KillAttachedTents);
	DESCRIBE_PROC(BeamCirclePoints);
	DESCRIBE_PROC(BeamEntPoint);
	DESCRIBE_PROC(BeamEnts);
	DESCRIBE_PROC(BeamFollow);
	DESCRIBE_PROC(BeamKill);
	DESCRIBE_PROC(BeamLightning);
	DESCRIBE_PROC(BeamPoints);
	DESCRIBE_PROC(BeamRing);
	DESCRIBE_PROC(AllocDlight);
	DESCRIBE_PROC(AllocElight);
	DESCRIBE_PROC(TempEntAlloc);
	DESCRIBE_PROC(TempEntAllocNoModel);
	DESCRIBE_PROC(TempEntAllocHigh);
	DESCRIBE_PROC(TentEntAllocCustom);
	DESCRIBE_PROC(GetPackedColor);
	DESCRIBE_PROC(LookupColor);
	DESCRIBE_PROC(DecalRemoveAll);
	return 0;
}

IBaseInterface *EVS_CreateInterface(void)
{
	return (IBaseInterface *)EVS_GetProcAddress;
}

EXPOSE_INTERFACE_FN((InstantiateInterfaceFn)EVS_CreateInterface, IEventScript, EVENTSCRIPT_INTERFACE_VERSION);

typedef struct script_object_item_s
{
	char szName[24];
	Script *pObject;
	double loadtime;
}
script_object_item_t;

CUtlVector<script_object_item_t> gEVScriptObjItems;

void EVS_LoadErrorCallBack(void *opaque, const char *msg)
{
	gEngfuncs.Con_DPrintf("EV_LoadScript: %s\n", msg);
}

void EVS_CompileErrorCallBack(void *opaque, const char *msg)
{
	gEngfuncs.Con_Printf("EV_CompileScript: %s\n", msg);
}

Script *EVS_GetScript(const char *name)
{
	for (int i = 0; i < gEVScriptObjItems.Count(); i++)
	{
		if (!strcmp(gEVScriptObjItems[i].szName, name))
			return gEVScriptObjItems[i].pObject;
	}

	return NULL;
}

static byte event_script_buffer[0x10000];

Script *EVS_LoadScript(const char *name)
{
	if (!gConfigs.bEnableEventScript)
		return NULL;

	Script *pScript;
	size_t len = 0;
	bool debugTime = ((int)developer->value > 2) ? true : false;
	double startTime = gPerformanceCounter.GetCurTime();

	char fullPath[MAX_PATH];
	char path[MAX_QPATH];
	byte *buffer = event_script_buffer;
	byte **ppbuf = (byte **)&event_script_buffer;
	sprintf(path, "events/%s.sc", name);

	if (!g_pFileSystem->GetLocalPath(path, fullPath, sizeof(fullPath)))
		return NULL;

	char targetDir[MAX_PATH];
	strcpy(targetDir, fullPath);

	char *filebase = strrchr(targetDir, '\\');

	if (filebase)
		*filebase = '\0';

	FILE *fp = fopen(fullPath, "rb");

	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, len, 1, fp);
	buffer[len] = '\0';
	fclose(fp);

	if (!(buffer[0] == 'E' && buffer[1] == 'V' && buffer[2] == 'S'))
	{
		if (buffer[0] == '/')
			return NULL;

		TCCState *s = tcc_new();
		tcc_set_options(s, "-nostdinc");
		tcc_set_output_type(s, TCC_OUTPUT_DLL);
		tcc_set_error_func(s, NULL, EVS_LoadErrorCallBack);
		tcc_add_sysinclude_path(s, targetDir);
		tcc_compile_string_ex(s, (const char *)buffer, name);

		if (tcc_output_buffer(s, ppbuf, &len))
		{
			gEngfuncs.Con_DPrintf("EV_LoadScript: compile %s failed.\n", name);
			tcc_delete(s);
			return NULL;
		}

		tcc_delete(s);
	}
	else
	{
		buffer += 3;
		len -= 3;
	}

	pScript = new Script(buffer, len, EVENTSCRIPT_INTERFACE_VERSION);

	if (!pScript)
		return NULL;

	double endTime = gPerformanceCounter.GetCurTime();
	int i = gEVScriptObjItems.AddToTail();
	gEVScriptObjItems[i].pObject = pScript;
	strcpy(gEVScriptObjItems[i].szName, name);
	gEVScriptObjItems[i].loadtime = endTime - startTime;

	if (debugTime)
		gEngfuncs.Con_Printf("EV_LoadScript: loaded script %s in time %.4f sec.\n", name, gEVScriptObjItems[i].loadtime);

	return pScript;
}

bool EV_CompileScript(const char *sourcefile, const char *targetfile = NULL)
{
	size_t len = 0;

	byte *buffer = event_script_buffer;
	byte **ppbuf = (byte **)&event_script_buffer;

	char fullPath[MAX_PATH];
	char path[MAX_QPATH];
	sprintf(path, "events/%s.sc", sourcefile);

	if (!g_pFileSystem->GetLocalPath(path, fullPath, sizeof(fullPath)))
	{
		gEngfuncs.Con_Printf("EV_CompileScript: open %s failed.\n", sourcefile);
		return false;
	}

	char targetPath[MAX_PATH], targetDir[MAX_PATH];
	strcpy(targetDir, fullPath);

	char *filebase = strrchr(targetDir, '\\');

	if (filebase)
	{
		*filebase = '\0';

		if (!targetfile)
			sprintf(targetPath, "%s/compiled/%s.sc", targetDir, sourcefile);
		else
			sprintf(targetPath, "%s/compiled/%s.sc", targetDir, targetfile);
	}

	double startTime = gPerformanceCounter.GetCurTime();
	FILE *fp = fopen(fullPath, "rb");

	if (!fp)
	{
		gEngfuncs.Con_Printf("EV_CompileScript: open %s failed.\n", sourcefile);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, len, 1, fp);
	buffer[len] = '\0';
	fclose(fp);

	if (buffer[0] == '/')
	{
		gEngfuncs.Con_Printf("EV_CompileScript: invalid %s.\n", sourcefile);
		return false;
	}

	if (buffer[0] == 'E' && buffer[1] == 'V' && buffer[2] == 'S')
	{
		gEngfuncs.Con_Printf("EV_CompileScript: ignore %s.\n", sourcefile);
		return false;
	}

	TCCState *s = tcc_new();
	tcc_set_options(s, "-nostdinc");
	tcc_set_output_type(s, TCC_OUTPUT_DLL);
	tcc_set_error_func(s, NULL, EVS_CompileErrorCallBack);
	tcc_add_sysinclude_path(s, targetDir);
	tcc_compile_string_ex(s, (const char *)buffer, sourcefile);

	if (tcc_output_buffer(s, ppbuf, &len))
	{
		gEngfuncs.Con_Printf("EV_CompileScript: compile %s failed.\n", sourcefile);
		tcc_delete(s);
		return false;
	}

	tcc_delete(s);

	gEngfuncs.Con_Printf("Compiled %s time %.4f..\n", sourcefile, gPerformanceCounter.GetCurTime() - startTime);

	fp = fopen(targetPath, "wb");

	if (fp)
	{
		fwrite("EVS", 3, 1, fp);
		fwrite(buffer, len, 1, fp);
		fclose(fp);
	}
	else
	{
		gEngfuncs.Con_Printf("EV_CompileScript: Please create folder \"compiled\"!\n");
	}

	return true;
}

void EVS_ReleaseAllScript(void)
{
	for (int i = 0; i < gEVScriptObjItems.Count(); i++)
		delete gEVScriptObjItems[i].pObject;

	gEVScriptObjItems.RemoveAll();
}

void EVS_CompileAllScript(void)
{
	FileFindHandle_t findHandle;
	const char *result = g_pFileSystem->FindFirst("events/*.sc", &findHandle, "GAME");

	while (result)
	{
		char filename[MAX_QPATH];
		strcpy(filename, result);
		filename[strlen(filename) - 3] = 0;

		if (!EV_CompileScript(filename))
			break;

		result = g_pFileSystem->FindNext(findHandle);
	}

	g_pFileSystem->FindClose(findHandle);
}

void EVS_CompileTest(event_args_t *args)
{
}

void EVS_About(void)
{
	gEngfuncs.Con_Printf("Event script system using the Tiny C Compiler!\n");
	gEngfuncs.Con_Printf("Tiny C Compiler Website: http://bellard.org/tcc\n");
}

void EVS_List(void)
{
	double totalTime = 0;
	gEngfuncs.Con_Printf("%-12s%-5s\n", "name", "loadtime(s)");

	for (int i = 0; i < gEVScriptObjItems.Count(); i++)
	{
		totalTime += gEVScriptObjItems[i].loadtime;
		gEngfuncs.Con_Printf("%-12s%-.4f\n", gEVScriptObjItems[i].szName, gEVScriptObjItems[i].loadtime);
	}

	gEngfuncs.Con_Printf("%d scripts loaded, %.4f sec total..\n", gEVScriptObjItems.Count(), totalTime);
}

void EVS_Compile(void)
{
	if (gEngfuncs.Cmd_Argc() <= 1)
	{
		gEngfuncs.Con_Printf("Usage: evs_compile <script>\n");
		return;
	}

	char *sourcefile = gEngfuncs.Cmd_Argv(1);

	if (gEngfuncs.Cmd_Argc() == 3)
	{
		char *targetfile = gEngfuncs.Cmd_Argv(2);

		if (strlen(targetfile) > 0)
			EV_CompileScript(sourcefile, targetfile);
	}
	else
	{
		EV_CompileScript(sourcefile, NULL);
	}
}