#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"

#include "pm_defs.h"
#include "pm_shared\pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"
#include "client.h"
#include "ammohistory.h"
#include "weapons.h"

extern int g_iShotsFired;
extern bool *g_bGunSmoke;

extern engine_studio_api_t IEngineStudio;

int tracerCount[32];

extern void EVS_CompileTest(event_args_t *args);
extern "C" char PM_FindTextureType(char *name);

void V_PunchAxis(int axis, float punch);
void VectorAngles(const float *forward, float *angles);

extern cvar_t *cl_lw;

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

#define VECTOR_CONE_1DEGREES Vector(0.00873, 0.00873, 0.00873)
#define VECTOR_CONE_2DEGREES Vector(0.01745, 0.01745, 0.01745)
#define VECTOR_CONE_3DEGREES Vector(0.02618, 0.02618, 0.02618)
#define VECTOR_CONE_4DEGREES Vector(0.03490, 0.03490, 0.03490)
#define VECTOR_CONE_5DEGREES Vector(0.04362, 0.04362, 0.04362)
#define VECTOR_CONE_6DEGREES Vector(0.05234, 0.05234, 0.05234)
#define VECTOR_CONE_7DEGREES Vector(0.06105, 0.06105, 0.06105)
#define VECTOR_CONE_8DEGREES Vector(0.06976, 0.06976, 0.06976)
#define VECTOR_CONE_9DEGREES Vector(0.07846, 0.07846, 0.07846)
#define VECTOR_CONE_10DEGREES Vector(0.08716, 0.08716, 0.08716)
#define VECTOR_CONE_15DEGREES Vector(0.13053, 0.13053, 0.13053)
#define VECTOR_CONE_20DEGREES Vector(0.17365, 0.17365, 0.17365)

void EV_HLDM_CreateLight(float *origin, float radius, int r, int g, int b, float time)
{
	dlight_t *te = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
	VectorCopy(origin, te->origin);

	te->radius = radius;
	te->color.r = r;
	te->color.g = g;
	te->color.b = b;
	te->die = gHUD.m_flTime + time;
}

void EV_HLDM_CreateSmoke(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate)
{
	g_pfnEV_HLDM_CreateSmoke(origin, dir, speed, scale, r, g, b, iSmokeType, base_velocity, bWind, framerate);
}

float EV_HLDM_PlayTextureSound(int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType)
{
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[64];
	char szbuffer[64];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace(ptr);
	chTextureType = 0;

	if (entity >= 1 && entity <= gEngfuncs.GetMaxClients())
	{
		chTextureType = CHAR_TEX_FLESH;
	}
	else if (entity == 0)
	{
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture(ptr->ent, vecSrc, vecEnd);

		if (pTextureName)
		{
			strcpy(texname, pTextureName);
			pTextureName = texname;

			if (*pTextureName == '-' || *pTextureName == '+')
				pTextureName += 2;

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
				pTextureName++;

			strcpy(szbuffer, pTextureName);
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;
			chTextureType = PM_FindTextureType(szbuffer);
		}
	}

	switch (chTextureType)
	{
		default:
		case CHAR_TEX_CONCRETE:
		{
			fvol = 0.9; fvolbar = 0.6;
			rgsz[0] = "player/pl_step1.wav";
			rgsz[1] = "player/pl_step2.wav";
			cnt = 2;
			break;
		}

		case CHAR_TEX_METAL:
		{
			fvol = 0.9; fvolbar = 0.3;
			rgsz[0] = "player/pl_metal1.wav";
			rgsz[1] = "player/pl_metal2.wav";
			cnt = 2;
			break;
		}

		case CHAR_TEX_DIRT:
		{
			fvol = 0.9; fvolbar = 0.1;
			rgsz[0] = "player/pl_dirt1.wav";
			rgsz[1] = "player/pl_dirt2.wav";
			rgsz[2] = "player/pl_dirt3.wav";
			cnt = 3;
			break;
		}

		case CHAR_TEX_VENT:
		{
			fvol = 0.5; fvolbar = 0.3;
			rgsz[0] = "player/pl_duct1.wav";
			rgsz[1] = "player/pl_duct1.wav";
			cnt = 2;
			break;
		}

		case CHAR_TEX_GRATE:
		{
			fvol = 0.9; fvolbar = 0.5;
			rgsz[0] = "player/pl_grate1.wav";
			rgsz[1] = "player/pl_grate4.wav";
			cnt = 2;
			break;
		}

		case CHAR_TEX_TILE:
		{
			fvol = 0.8; fvolbar = 0.2;
			rgsz[0] = "player/pl_tile1.wav";
			rgsz[1] = "player/pl_tile3.wav";
			rgsz[2] = "player/pl_tile2.wav";
			rgsz[3] = "player/pl_tile4.wav";
			cnt = 4;
			break;
		}

		case CHAR_TEX_SLOSH:
		{
			fvol = 0.9; fvolbar = 0.0;
			rgsz[0] = "player/pl_slosh1.wav";
			rgsz[1] = "player/pl_slosh3.wav";
			rgsz[2] = "player/pl_slosh2.wav";
			rgsz[3] = "player/pl_slosh4.wav";
			cnt = 4;
			break;
		}

		case CHAR_TEX_SNOW:
		{
			fvol = 0.7; fvolbar = 0.4;
			rgsz[0] = "debris/pl_snow1.wav";
			rgsz[1] = "debris/pl_snow2.wav";
			rgsz[2] = "debris/pl_snow3.wav";
			rgsz[3] = "debris/pl_snow4.wav";
			cnt = 4;
			break;
		}

		case CHAR_TEX_WOOD:
		{
			fvol = 0.9; fvolbar = 0.2;
			rgsz[0] = "debris/wood1.wav";
			rgsz[1] = "debris/wood2.wav";
			rgsz[2] = "debris/wood3.wav";
			cnt = 3;
			break;
		}

		case CHAR_TEX_GLASS:
		case CHAR_TEX_COMPUTER:
		{
			fvol = 0.8; fvolbar = 0.2;
			rgsz[0] = "debris/glass1.wav";
			rgsz[1] = "debris/glass2.wav";
			rgsz[2] = "debris/glass3.wav";
			cnt = 3;
			break;
		}

		case CHAR_TEX_FLESH:
		{
			if (iBulletType == BULLET_PLAYER_CROWBAR)
				return 0.0;

			fvol = 1.0; fvolbar = 0.2;
			rgsz[0] = "weapons/bullet_hit1.wav";
			rgsz[1] = "weapons/bullet_hit2.wav";
			fattn = 1.0;
			cnt = 2;
			break;
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0, cnt - 1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0, 0xf));
	return fvolbar;
}

char *EV_HLDM_DamageDecal(physent_t *pe)
{
	static char decalname[32];
	int idx;

	if (pe->classnumber == 1)
	{
		idx = gEngfuncs.pfnRandomLong(0, 2);
		sprintf(decalname, "{break%i", idx + 1);
	}
	else if (pe->rendermode != kRenderNormal)
	{
		sprintf(decalname, "{bproof1");
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong(0, 4);
		sprintf(decalname, "{shot%i", idx + 1);
	}

	return decalname;
}

void EV_HLDM_GunshotDecalTrace(pmtrace_t *pTrace, char *decalName)
{
	int iRand;
	physent_t *pe;

	gEngfuncs.pEfxAPI->R_BulletImpactParticles(pTrace->endpos);

	iRand = gEngfuncs.pfnRandomLong(0, 0x7FFF);

	if (iRand < (0x7fff / 2))
	{
		switch (iRand % 5)
		{
			case 0:	gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
			case 1:	gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
			case 2:	gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
			case 3:	gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
			case 4:	gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (decalName && decalName[0] && pe && (pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP))
	{
		if (CVAR_GET_FLOAT("r_decals"))
			gEngfuncs.pEfxAPI->R_DecalShoot(gEngfuncs.pEfxAPI->Draw_DecalIndex(gEngfuncs.pEfxAPI->Draw_DecalIndexFromName(decalName)), gEngfuncs.pEventAPI->EV_IndexFromTrace(pTrace), 0, pTrace->endpos, 0);
	}
}

void EV_HLDM_DecalGunshot(pmtrace_t *pTrace, int iBulletType, float scale, int r, int g, int b, bool bCreateSparks, char cTextureType)
{
	g_pfnEV_HLDM_DecalGunshot(pTrace, iBulletType, scale, r, g, b, bCreateSparks, cTextureType);
}

void EV_HLDM_DecalGunshot(pmtrace_t *pTrace, int iBulletType)
{
	physent_t *pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (pe && pe->solid == SOLID_BSP)
	{
		switch (iBulletType)
		{
			case BULLET_PLAYER_9MM:
			case BULLET_MONSTER_9MM:
			case BULLET_PLAYER_MP5:
			case BULLET_MONSTER_MP5:
			case BULLET_PLAYER_BUCKSHOT:
			case BULLET_PLAYER_357:
			default:
			{
				EV_HLDM_GunshotDecalTrace(pTrace, EV_HLDM_DamageDecal(pe));
				break;
			}
		}
	}
}

int EV_HLDM_CheckTracer(int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount)
{
	int tracer = 0;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;

	if (iTracerFreq != 0 && ((*tracerCount)++ % iTracerFreq) == 0)
	{
		vec3_t vecTracerSrc;

		if (player)
		{
			vec3_t offset(0, 0, -4);

			for (i = 0; i < 3; i++)
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
		}
		else
			VectorCopy(vecSrc, vecTracerSrc);

		if (iTracerFreq != 1)
			tracer = 1;

		switch (iBulletType)
		{
			case BULLET_PLAYER_MP5:
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:

			default:
			{
				EV_CreateTracer(vecTracerSrc, end);
				break;
			}
		}
	}

	return tracer;
}

void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, int iPenetration)
{
	g_pfnEV_HLDM_FireBullets(idx, forward, right, up, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, tracerCount, iPenetration);
}

void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY)
{
	int i;
	pmtrace_t tr;
	int iShot;
	int tracer;

	for (iShot = 1; iShot <= cShots; iShot++)
	{
		vec3_t vecDir, vecEnd;
		float x, y, z;

		if (iBulletType == BULLET_PLAYER_BUCKSHOT)
		{
			do
			{
				x = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				y = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				z = x * x + y * y;
			}
			while (z > 1);

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + x * flSpreadX * right[i] + y * flSpreadY * up [i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}
		else
		{
			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + flSpreadX * right[i] + flSpreadY * up [i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);

		gEngfuncs.pEventAPI->EV_PushPMStates();

		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

		tracer = EV_HLDM_CheckTracer(idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount);

		if (tr.fraction != 1.0)
		{
			switch (iBulletType)
			{
				default:
				case BULLET_PLAYER_9MM:
				{
					EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
					EV_HLDM_DecalGunshot(&tr, iBulletType);
					break;
				}

				case BULLET_PLAYER_MP5:
				{
					if (!tracer)
					{
						EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
						EV_HLDM_DecalGunshot(&tr, iBulletType);
					}

					break;
				}

				case BULLET_PLAYER_BUCKSHOT:
				{
					EV_HLDM_DecalGunshot(&tr, iBulletType);
					break;
				}

				case BULLET_PLAYER_357:
				{
					EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
					EV_HLDM_DecalGunshot(&tr, iBulletType);
					break;
				}
			}
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

void EV_FireGlock1(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = args->bparam1 == false;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 2);
		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, 0, args->fparam1, args->fparam2);
}

void EV_FireGlock2(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GLOCK_SHOOT, 2);
		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

void EV_FireShotGunDouble(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	int j;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE2, 2);
		V_PunchAxis(0, -10.0);
	}

	for (j = 0; j < 2; j++)
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);
		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dbarrel1.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
		EV_HLDM_FireBullets(idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.17365, 0.04362);
	else
		EV_HLDM_FireBullets(idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
}

void EV_FireShotGunSingle(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE, 2);

		V_PunchAxis(0, -5.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
		EV_HLDM_FireBullets(idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.04362);
	else
		EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
}

void EV_FireMP5(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_FIRE1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-2, 2));
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
		case 0:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		}

		case 1:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		}
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 2, &tracerCount[idx - 1], args->fparam1, args->fparam2);
	else
		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 2, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

void EV_FireMP52(event_args_t *args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_LAUNCH, 2);
		V_PunchAxis(0, -10);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
		case 0:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		}

		case 1:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		}
	}
}

void EV_FirePython(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		int multiplayer = gEngfuncs.GetMaxClients() == 1 ? 0 : 1;

		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(PYTHON_FIRE1, multiplayer ? 1 : 0);
		V_PunchAxis(0, -10.0);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
		case 0:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot1.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
			break;
		}

		case 1:
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot2.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_357, 0, 0, args->fparam1, args->fparam2);
}

#define SND_CHANGE_PITCH (1<<7)

void EV_SpinGauss(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	pitch = args->iparam1;

	iSoundState = args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch);
}

void EV_StopPreviousGauss(int idx)
{
	gEngfuncs.pEventAPI->EV_KillEvents(idx, "events/gaussspin.sc");
	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_WEAPON, "ambience/pulsemachine.wav");
}

void EV_FireGauss(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage = args->fparam1;
	int primaryfire = args->bparam1;

	int m_fPrimaryFire = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	edict_t *pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	int nTotal = 0;
	int fHasPunched = 0;
	int fFirstBeam = 1;
	int nMaxHits = 10;
	physent_t *pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	if (args->bparam2)
	{
		EV_StopPreviousGauss(idx);
		return;
	}

	EV_GetGunPosition(args, vecSrc, origin);

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	AngleVectors(angles, forward, right, up);

	VectorMA(vecSrc, 8192, forward, vecDest);

	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, -2.0);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GAUSS_FIRE2, 2);
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);

		gEngfuncs.pEventAPI->EV_PushPMStates();

		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecDest, PM_STUDIO_BOX, -1, &tr);

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if (tr.allsolid)
			break;

		if (fFirstBeam)
		{
			if (EV_IsLocal(idx))
				EV_MuzzleFlash();

			fFirstBeam = 0;

			gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, m_iBeam, 0.1, m_fPrimaryFire ? 1.0 : 2.5, 0.0, m_fPrimaryFire ? 128.0 : flDamage, 0, 0, 0, m_fPrimaryFire ? 255 : 255, m_fPrimaryFire ? 128 : 255, m_fPrimaryFire ? 0 : 255);
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, tr.endpos, m_iBeam, 0.1, m_fPrimaryFire ? 1.0 : 2.5, 0.0, m_fPrimaryFire ? 128.0 : flDamage, 0, 0, 0, m_fPrimaryFire ? 255 : 255, m_fPrimaryFire ? 128 : 255, m_fPrimaryFire ? 0 : 255);
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);

		if (pEntity == NULL)
			break;

		if (pEntity->solid == SOLID_BSP)
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct(tr.plane.normal, forward);

			if (n < 0.5)
			{
				vec3_t r;
				VectorMA(forward, 2.0 * n, tr.plane.normal, r);

				flMaxFrac = flMaxFrac - tr.fraction;

				VectorCopy(r, forward);

				VectorMA(tr.endpos, 8.0, forward, vecSrc);
				VectorMA(vecSrc, 8192.0, forward, vecDest);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT);

				vec3_t fwd;
				VectorAdd(tr.endpos, tr.plane.normal, fwd);

				gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100, 255, 100);

				if (n == 0)
					n = 0.1;

				flDamage = flDamage * (1 - n);
			}
			else
			{
				EV_HLDM_DecalGunshot(&tr, BULLET_MONSTER_12MM);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

				if (fHasPunched)
					break;

				fHasPunched = 1;

				if (!m_fPrimaryFire)
				{
					vec3_t start;
					VectorMA(tr.endpos, 8.0, forward, start);

					gEngfuncs.pEventAPI->EV_PushPMStates();

					gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

					gEngfuncs.pEventAPI->EV_SetTraceHull(2);
					gEngfuncs.pEventAPI->EV_PlayerTrace(start, vecDest, PM_STUDIO_BOX, -1, &beam_tr);

					if (!beam_tr.allsolid)
					{
						vec3_t delta;
						float n;

						gEngfuncs.pEventAPI->EV_PlayerTrace(beam_tr.endpos, tr.endpos, PM_STUDIO_BOX, -1, &beam_tr);

						VectorSubtract(beam_tr.endpos, tr.endpos, delta);

						n = Length(delta);

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;

							flDamage -= n;

							vec3_t fwd;
							VectorSubtract(tr.endpos, forward, fwd);
							gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100, 255, 100);

							EV_HLDM_DecalGunshot(&beam_tr, BULLET_MONSTER_12MM);

							gEngfuncs.pEfxAPI->R_TempSprite(beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

							VectorSubtract(beam_tr.endpos, forward, fwd);
							gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.3), 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 200, 255, 40);

							VectorAdd(beam_tr.endpos, forward, vecSrc);
						}
					}
					else
						flDamage = 0;

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if (m_fPrimaryFire)
					{
						gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT);

						vec3_t fwd;
						VectorAdd(tr.endpos, tr.plane.normal, fwd);
						gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100, 255, 200);
					}

					flDamage = 0;
				}
			}
		}
		else
			VectorAdd(tr.endpos, forward, vecSrc);
	}
}

enum crowbar_e
{
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};

int g_iSwing;

void EV_Crowbar(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 1);

		switch ((g_iSwing++) % 3)
		{
			case 0: gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 1); break;
			case 1: gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK2MISS, 1); break;
			case 2: gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK3MISS, 1); break;
		}
	}
}

enum crossbow_e
{
	CROSSBOW_IDLE1 = 0,
	CROSSBOW_IDLE2,
	CROSSBOW_FIDGET1,
	CROSSBOW_FIDGET2,
	CROSSBOW_FIRE1,
	CROSSBOW_FIRE2,
	CROSSBOW_FIRE3,
	CROSSBOW_RELOAD,
	CROSSBOW_DRAW1,
	CROSSBOW_DRAW2,
	CROSSBOW_HOLSTER1,
	CROSSBOW_HOLSTER2,
};

void EV_BoltCallback(struct tempent_s *ent, float frametime, float currenttime)
{
	VectorCopy(ent->entity.baseline.vuser1, ent->entity.origin);
	VectorCopy(ent->entity.baseline.vuser2, ent->entity.angles);
}

void EV_FireCrossbow2(event_args_t *args)
{
	vec3_t vecSrc, vecEnd;
	vec3_t up, right, forward;
	pmtrace_t tr;

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	VectorMA(vecSrc, 8192, forward, vecEnd);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	if (EV_IsLocal(idx))
	{
		if (args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 1);
		else if (args->iparam2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 1);
	}

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

	if (tr.fraction < 1.0)
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);

		if (pe->solid != SOLID_BSP)
		{
			switch (gEngfuncs.pfnRandomLong(0, 1))
			{
				case 0: gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1: gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}
		else if (pe->rendermode == kRenderNormal)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, PITCH_NORM);

			if (gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_WATER)
				gEngfuncs.pEfxAPI->R_SparkShower(tr.endpos);

			vec3_t vBoltAngles;
			int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("models/crossbow_bolt.mdl");

			VectorAngles(forward, vBoltAngles);

			TEMPENTITY *bolt = gEngfuncs.pEfxAPI->R_TempModel(tr.endpos - forward * 10, Vector(0, 0, 0), vBoltAngles, 5, iModelIndex, TE_BOUNCE_NULL);

			if (bolt)
			{
				bolt->flags |= (FTENT_CLIENTCUSTOM);

				VectorSubtract(tr.endpos, forward, bolt->entity.baseline.vuser1);
				VectorScale(bolt->entity.baseline.vuser1, 10, bolt->entity.baseline.vuser1);
				VectorCopy(vBoltAngles, bolt->entity.baseline.vuser2);

				bolt->callback = EV_BoltCallback;
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

void EV_FireCrossbow(event_args_t *args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	if (EV_IsLocal(idx))
	{
		if (args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 1);
		else if (args->iparam2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 1);

		V_PunchAxis(0, -2.0);
	}
}

enum rpg_e
{
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,
	RPG_FIRE2,
	RPG_HOLSTER1,
	RPG_DRAW1,
	RPG_HOLSTER2,
	RPG_DRAW_UL,
	RPG_IDLE_UL,
	RPG_FIDGET_UL,
};

void EV_FireRpg(event_args_t *args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM);
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(RPG_FIRE2, 1);

		V_PunchAxis(0, -5.0);
	}
}

enum egon_e
{
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};

int g_fireAnims1[] = { EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4 };
int g_fireAnims2[] = { EGON_ALTFIRECYCLE };

enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE };

#define EGON_PRIMARY_VOLUME 450
#define EGON_BEAM_SPRITE "sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE "sprites/XSpark1.spr"
#define EGON_SOUND_OFF "weapons/egon_off1.wav"
#define EGON_SOUND_RUN "weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP "weapons/egon_windup2.wav"

BEAM *pBeam;
BEAM *pBeam2;

void EV_EgonFire(event_args_t *args)
{
	int idx, iFireState, iFireMode;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	iFireState = args->iparam1;
	iFireMode = args->iparam2;
	int iStartup = args->bparam1;

	if (iStartup)
	{
		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100);
	}
	else
	{
		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100);
	}

	if (EV_IsLocal(idx))
		gEngfuncs.pEventAPI->EV_WeaponAnimation(g_fireAnims1[gEngfuncs.pfnRandomLong(0, 3)], 1);

	if (iStartup == 1 && EV_IsLocal(idx) && !pBeam && !pBeam2 && cl_lw->value)
	{
		vec3_t vecSrc, vecEnd, origin, angles, forward, right, up;
		pmtrace_t tr;

		cl_entity_t *pl = gEngfuncs.GetEntityByIndex(idx);

		if (pl)
		{
			VectorCopy(gHUD.m_vecAngles, angles);

			AngleVectors(angles, forward, right, up);

			EV_GetGunPosition(args, vecSrc, pl->origin);

			VectorMA(vecSrc, 2048, forward, vecEnd);

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);

			gEngfuncs.pEventAPI->EV_PushPMStates();

			gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

			gEngfuncs.pEventAPI->EV_SetTraceHull(2);
			gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);

			gEngfuncs.pEventAPI->EV_PopPMStates();

			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex(EGON_BEAM_SPRITE);

			float r = 50.0f;
			float g = 50.0f;
			float b = 125.0f;

			if (IEngineStudio.IsHardware())
			{
				r /= 100.0f;
				g /= 100.0f;
			}

			pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint (idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b);

			if (pBeam)
				pBeam->flags |= (FBEAM_SINENOISE);

			pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint (idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b);
		}
	}
}

void EV_EgonStop(event_args_t *args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

	if (args->iparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100);

	if (EV_IsLocal(idx))
	{
		if (pBeam)
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}

		if (pBeam2)
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}
	}
}

enum hgun_e
{
	HGUN_IDLE1 = 0,
	HGUN_FIDGETSWAY,
	HGUN_FIDGETSHAKE,
	HGUN_DOWN,
	HGUN_UP,
	HGUN_SHOOT
};

void EV_HornetGunFire(event_args_t *args)
{
	int idx, iFireMode;
	vec3_t origin, angles, vecSrc, forward, right, up;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	iFireMode = args->iparam1;

	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, gEngfuncs.pfnRandomLong(0, 2));
		gEngfuncs.pEventAPI->EV_WeaponAnimation(HGUN_SHOOT, 1);
	}

	switch (gEngfuncs.pfnRandomLong(0, 2))
	{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100); break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100); break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100); break;
	}
}

enum tripmine_e
{
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND,
};

void EV_TripmineFire(event_args_t *args)
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	vecSrc = vecSrc + view_ofs;

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + forward * 128, PM_NORMAL, -1, &tr);

	if (tr.fraction < 1.0)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(TRIPMINE_DRAW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

enum squeak_e
{
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

#define VEC_HULL_MIN Vector(-16, -16, -36)
#define VEC_DUCK_HULL_MIN Vector(-16, -16, -18)

void EV_SnarkFire(event_args_t *args)
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	if (args->ducking)
		vecSrc = vecSrc - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc + forward * 20, vecSrc + forward * 64, PM_NORMAL, -1, &tr);

	if (tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SQUEAK_THROW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

extern void (*g_pfnEV_VehiclePitchAdjust)(event_args_t *args);

void EV_VehiclePitchAdjust(event_args_t *args)
{
	return g_pfnEV_VehiclePitchAdjust(args);
}

void EV_TrainPitchAdjust(event_args_t *args)
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;

	char sz[256];

	idx = args->entindex;

	VectorCopy(args->origin, origin);

	us_params = (unsigned short)args->iparam1;
	stop = args->bparam1;

	m_flVolume = (float)(us_params & 0x003f) / 40.0;
	noise = (int)(((us_params) >> 12) & 0x0007);
	pitch = (int)(10.0 * (float)((us_params >> 6) & 0x003f));

	switch (noise)
	{
		case 1: strcpy(sz, "plats/ttrain1.wav"); break;
		case 2: strcpy(sz, "plats/ttrain2.wav"); break;
		case 3: strcpy(sz, "plats/ttrain3.wav"); break;
		case 4: strcpy(sz, "plats/ttrain4.wav"); break;
		case 5: strcpy(sz, "plats/ttrain6.wav"); break;
		case 6: strcpy(sz, "plats/ttrain7.wav"); break;

		default:
		{
			strcpy(sz, "");
			return;
		}
	}

	if (stop)
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, sz);
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch);
}

int EV_TFC_IsAllyTeam(int iTeam1, int iTeam2)
{
	return 0;
}

enum usp_e
{
	USP_IDLE,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOT_EMPTY,
	USP_RELOAD,
	USP_DRAW,
	USP_ATTACH_SILENCER,
	USP_UNSIL_IDLE,
	USP_UNSIL_SHOOT1,
	USP_UNSIL_SHOOT2,
	USP_UNSIL_SHOOT3,
	USP_UNSIL_SHOOT_EMPTY,
	USP_UNSIL_RELOAD,
	USP_UNSIL_DRAW,
	USP_DETACH_SILENCER
};

void EV_FireUSP(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	int silencer_on;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;
	vec3_t smoke_origin;
	float base_scale;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	silencer_on = args->bparam2;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;

		if (silencer_on)
		{
			if (!empty)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(USP_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(USP_SHOOT_EMPTY, 2);
		}
		else
		{
			EV_MuzzleFlash();

			if (!empty)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(USP_UNSIL_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(USP_UNSIL_SHOOT_EMPTY, 2);
		}

		if (*g_bGunSmoke)
		{
			VectorCopy(ent->attachment[0], smoke_origin);

			smoke_origin[0] -= forward[0] * 3;
			smoke_origin[1] -= forward[1] * 3;
			smoke_origin[2] -= forward[2] * 3;

			base_scale = gEngfuncs.pfnRandomFloat(0.1, 0.25);

			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, smoke_origin[2], 7, 7, 7, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 20, base_scale + 0.1, 10, 10, 10, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, base_scale, 13, 13, 13, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, -14.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, 14.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.5, ShellVelocity);
		ShellVelocity[2] += 45.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 5);

	if (!silencer_on)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/usp_unsil-1.wav", 1.0, ATTN_NORM, 0, 87 + gEngfuncs.pfnRandomLong(0, 0x12));
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/usp1.wav", 1.0, 2.0, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 4096, BULLET_PLAYER_45ACP, 0, &tracerCount[idx - 1], 2);
}

enum mp5n_e
{
	MP5N_IDLE1,
	MP5N_RELOAD,
	MP5N_DRAW,
	MP5N_SHOOT1,
	MP5N_SHOOT2,
	MP5N_SHOOT3
};

void EV_FireMP5N(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5N_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.25, 10, 10, 10, 3, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, -11.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 13);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/mp5-1.wav", 1.0, 0.64, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], 2);
}

enum ak47_e
{
	AK47_IDLE1,
	AK47_RELOAD,
	AK47_DRAW,
	AK47_SHOOT1,
	AK47_SHOOT2,
	AK47_SHOOT3
};

void EV_FireAK47(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(AK47_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.35, 20, 20, 20, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 75;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 9);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/ak47-1.wav", 1.0, 0.4, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_762MM, 0, &tracerCount[idx - 1], 2);
}

enum aug_e
{
	AUG_IDLE1,
	AUG_RELOAD,
	AUG_DRAW,
	AUG_SHOOT1,
	AUG_SHOOT2,
	AUG_SHOOT3
};

void EV_FireAug(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(AUG_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.26, 15, 15, 15, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 17.0, -8.0, -14.0, true);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 17.0, -8.0, 14.0, true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 122;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 8);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/aug-1.wav", 1.0, 0.48, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

enum deagle_e
{
	DEAGLE_IDLE1,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,
	DEAGLE_DRAW
};

void EV_FireDeagle(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (empty)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(DEAGLE_SHOOT_EMPTY, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(DEAGLE_SHOOT1 + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] += 25;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 5);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/deagle-1.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
}

enum g3sg1_e
{
	G3SG1_IDLE,
	G3SG1_SHOOT,
	G3SG1_SHOOT2,
	G3SG1_RELOAD,
	G3SG1_DRAW
};

void EV_FireG3SG1(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(G3SG1_SHOOT + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 35, 35, 35, 4, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 35, 0.35, 30, 30, 30, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 70, 0.3, 25, 25, 25, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.5, ShellVelocity);
		ShellVelocity[2] -= 50.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 17);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/g3sg1-1.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_762MM, 0, &tracerCount[idx - 1], 2);
}

enum sg550_e
{
	SG550_IDLE,
	SG550_SHOOT,
	SG550_SHOOT2,
	SG550_RELOAD,
	SG550_DRAW
};

void EV_FireSG550(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(G3SG1_SHOOT + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 35, 35, 35, 4, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 35, 0.35, 30, 30, 30, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 70, 0.3, 25, 25, 25, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.5, ShellVelocity);
		ShellVelocity[2] -= 50.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 17);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sg550-1.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

enum glock18_e
{
	GLOCK18_IDLE1,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,
	GLOCK18_DRAW,
	GLOCK18_HOLSTER,
	GLOCK18_ADD_SILENCER,
	GLOCK18_DRAW2,
	GLOCK18_RELOAD2
};

void EV_FireGlock18(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (empty)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(GLOCK18_SHOOT_EMPTY, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(GLOCK18_SHOOT3, 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.2, 10, 10, 10, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.4, 20, 20, 20, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, -14.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, 14.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.65, ShellVelocity);
		ShellVelocity[2] += 25.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 4);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glock18-2.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 4096, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], 2);
}

enum m249_e
{
	M249_IDLE1,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_RELOAD,
	M249_DRAW
};

void EV_FireM249(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(M249_SHOOT1 + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.26, 15, 15, 15, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, true);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.2, ShellVelocity);
		ShellVelocity[2] -= 75;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 10);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m249-1.wav", 1.0, 0.52, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

enum m3_e
{
	M3_IDLE,
	M3_FIRE1,
	M3_FIRE2,
	M3_RELOAD,
	M3_PUMP,
	M3_START_RELOAD,
	M3_DRAW,
	M3_HOLSTER
};

void EV_FireM3(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(M3_FIRE1 + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.5, 25, 25, 25, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.5, 20, 20, 20, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.45, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 75, 0.35, 7, 7, 7, 2, velocity, false, 35);
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m3-1.wav", 1.0, 0.48, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = 0.0675;
	vSpread[1] = 0.0675;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 9, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 1);
}

enum m4a1_e
{
	M4A1_IDLE,
	M4A1_SHOOT1,
	M4A1_SHOOT2,
	M4A1_SHOOT3,
	M4A1_RELOAD,
	M4A1_DRAW,
	M4A1_ATTACH_SILENCER,
	M4A1_UNSIL_IDLE,
	M4A1_UNSIL_SHOOT1,
	M4A1_UNSIL_SHOOT2,
	M4A1_UNSIL_SHOOT3,
	M4A1_UNSIL_RELOAD,
	M4A1_UNSIL_DRAW,
	M4A1_DETACH_SILENCER
};

void EV_FireM4A1(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int silencer_on;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	silencer_on = args->bparam1;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (silencer_on)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M4A1_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M4A1_UNSIL_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
		{
			if (silencer_on)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 16, 16, 16, 3, velocity, false, 35);
			else
				EV_HLDM_CreateSmoke(ent->attachment[2], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 45;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 10);

	if (silencer_on)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4a1-1.wav", 1.0, 1.4, 0.52, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	}
	else
	{
		if (gEngfuncs.pfnRandomLong(0, 1))
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4a1_unsil-1.wav", 1.0, 0.52, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/m4a1_unsil-2.wav", 1.0, 0.52, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	}

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

enum mac10_e
{
	MAC10_IDLE1,
	MAC10_RELOAD,
	MAC10_DRAW,
	MAC10_SHOOT1,
	MAC10_SHOOT2,
	MAC10_SHOOT3
};

void EV_FireMac10(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MAC10_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 2, 0.25, 8, 8, 8, 3, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32.0, -9.0, -11.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32.0, -9.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] -= 65;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 7);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/mac10-1.wav", 1.0, 0.72, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], 2);
}

enum p90_e
{
	P90_IDLE1,
	P90_RELOAD,
	P90_DRAW,
	P90_SHOOT1,
	P90_SHOOT2,
	P90_SHOOT3
};

void EV_FireP90(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(P90_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.16, 7, 7, 7, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -16.0, -22.0, true);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -16.0, 22.0, true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 10);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/p90-1.wav", 1.0, 0.64, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_57MM, 0, &tracerCount[idx - 1], 2);
}

enum p228_e
{
	P228_IDLE,
	P228_SHOOT1,
	P228_SHOOT2,
	P228_SHOOT3,
	P228_SHOOT_EMPTY,
	P228_RELOAD,
	P228_DRAW
};

void EV_FireP228(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (empty)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(P228_SHOOT_EMPTY, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(P228_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 20, 20, 20, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, -14.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, 14.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] += 35;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 4);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/p228-1.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_357SIG, 0, &tracerCount[idx - 1], 2);
}

enum awp_e
{
	AWP_IDLE,
	AWP_SHOOT1,
	AWP_SHOOT2,
	AWP_SHOOT3,
	AWP_RELOAD,
	AWP_DRAW
};

void EV_FireAWP(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(AWP_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.5, 20, 20, 20, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.5, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 80, 0.5, 10, 10, 10, 2, velocity, false, 35);
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/awp1.wav", 1.0, 0.28, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_338MAG, 0, &tracerCount[idx - 1], 3);
}

enum scout_e
{
	SCOUT_IDLE,
	SCOUT_SHOOT,
	SCOUT_SHOOT2,
	SCOUT_RELOAD,
	SCOUT_DRAW
};

void EV_FireScout(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SCOUT_SHOOT + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 25, 25, 25, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 35, 0.45, 20, 20, 20, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 70, 0.45, 15, 15, 15, 2, velocity, false, 35);
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/scout_fire-1.wav", 1.0, 1.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	vSpread[0] = args->fparam1 / 1000.0;
	vSpread[1] = args->fparam2 / 1000.0;
	vSpread[2] = 0;

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_762MM, 0, &tracerCount[idx - 1], 3);
}

enum sg552_e
{
	SG552_IDLE1,
	SG552_RELOAD,
	SG552_DRAW,
	SG552_SHOOT1,
	SG552_SHOOT2,
	SG552_SHOOT3
};

void EV_FireSG552(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SG552_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 18, 18, 18, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.65, ShellVelocity);
		ShellVelocity[2] -= 120;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 15);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sg552-1.wav", 1.0, 0.4, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

enum tmp_e
{
	TMP_IDLE1,
	TMP_RELOAD,
	TMP_DRAW,
	TMP_SHOOT1,
	TMP_SHOOT2,
	TMP_SHOOT3
};

void EV_FireTMP(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(TMP_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 10, 10, 10, 3, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32.0, -6.0, -11.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32.0, -6.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.2, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 15);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/tmp-1.wav", 1.0, 1.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], 2);
}

enum fiveseven_e
{
	FIVESEVEN_IDLE,
	FIVESEVEN_SHOOT1,
	FIVESEVEN_SHOOT2,
	FIVESEVEN_SHOOT_EMPTY,
	FIVESEVEN_RELOAD,
	FIVESEVEN_DRAW
};

void EV_FireFiveSeven(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (empty)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(FIVESEVEN_SHOOT_EMPTY, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(FIVESEVEN_SHOOT1 + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 20, 20, 20, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, -14.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, 14.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] -= 35.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 4);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/fiveseven-1.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_57MM, 0, &tracerCount[idx - 1], 2);
}

enum ump45_e
{
	UMP45_IDLE1,
	UMP45_RELOAD,
	UMP45_DRAW,
	UMP45_SHOOT1,
	UMP45_SHOOT2,
	UMP45_SHOOT3
};

void EV_FireUMP45(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(UMP45_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.25, 10, 10, 10, 3, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, -11.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 13);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/ump45-1.wav", 1.0, 0.64, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_45ACP, 0, &tracerCount[idx - 1], 2);
}

enum xm1014_e
{
	XM1014_IDLE,
	XM1014_FIRE1,
	XM1014_FIRE2,
	XM1014_RELOAD,
	XM1014_PUMP,
	XM1014_START_RELOAD,
	XM1014_DRAW
};

void EV_FireXM1014(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(XM1014_FIRE1 + gEngfuncs.pfnRandomLong(0, 1), 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;

		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, -11.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL, idx, 3);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xm1014-1.wav", 1.0, 0.52, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = 0.0725;
	vSpread[1] = 0.0725;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 1);
}

enum elite_e
{
	ELITE_IDLE,
	ELITE_IDLE_LEFTEMPTY,
	ELITE_SHOOTLEFT1,
	ELITE_SHOOTLEFT2,
	ELITE_SHOOTLEFT3,
	ELITE_SHOOTLEFT4,
	ELITE_SHOOTLEFT5,
	ELITE_SHOOTLEFTLAST,
	ELITE_SHOOTRIGHT1,
	ELITE_SHOOTRIGHT2,
	ELITE_SHOOTRIGHT3,
	ELITE_SHOOTRIGHT4,
	ELITE_SHOOTRIGHT5,
	ELITE_SHOOTRIGHTLAST,
	ELITE_RELOAD,
	ELITE_DRAW
};

void (*g_pfnEV_FireElite)(struct event_args_s *args) = NULL;

void EV_FireEliteLeft(struct event_args_s *args)
{
	args->bparam1 = true;

	if (g_pfnEV_FireElite)
		return g_pfnEV_FireElite(args);

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int lefthand;
	int bullets_left;
	float time_diff;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = args->bparam1;
	bullets_left = args->iparam2;
	time_diff = args->fparam1;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (lefthand)
		{
			if (bullets_left <= 1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFTLAST, 2);
			else if (time_diff >= 0.5)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFT5, 2);
			else if (time_diff >= 0.4)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFT4, 2);
			else if (time_diff >= 0.3)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFT3, 2);
			else if (time_diff >= 0.2)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFT2, 2);
			else if (time_diff >= 0.0)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTLEFT1, 2);

			if (*g_bGunSmoke)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
			}
		}
		else
		{
			if (bullets_left <= 1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHTLAST, 2);
			else if (time_diff >= 0.5)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT5, 2);
			else if (time_diff >= 0.4)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT4, 2);
			else if (time_diff >= 0.3)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT3, 2);
			else if (time_diff >= 0.2)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT2, 2);
			else if (time_diff >= 0.0)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT1, 2);

			if (*g_bGunSmoke)
			{
				EV_HLDM_CreateSmoke(ent->attachment[1], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
			}
		}
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand)
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, false);
			VectorCopy(ent->attachment[2], ShellOrigin);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, false);
			VectorCopy(ent->attachment[3], ShellOrigin);
		}

		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] -= 25;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 5);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/elite_fire.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam2;
	vSpread[1] = args->fparam1 / 100.0;
	vSpread[2] = 0;

	if (lefthand)
	{
		vecSrc[0] -= right[0] * 5;
		vecSrc[1] -= right[1] * 5;
		vecSrc[2] -= right[2] * 5;
	}
	else
	{
		vecSrc[0] += right[0] * 5;
		vecSrc[1] += right[1] * 5;
		vecSrc[2] += right[2] * 5;
	}

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
}

void EV_FireEliteRight(struct event_args_s *args)
{
	args->bparam1 = false;

	if (g_pfnEV_FireElite)
		return g_pfnEV_FireElite(args);

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int bullets_left;
	float time_diff;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	bullets_left = args->iparam2;
	time_diff = args->fparam1;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (bullets_left <= 1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHTLAST, 2);
		else if (time_diff >= 0.5)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT5, 2);
		else if (time_diff >= 0.4)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT4, 2);
		else if (time_diff >= 0.3)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT3, 2);
		else if (time_diff >= 0.2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT2, 2);
		else if (time_diff >= 0.0)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ELITE_SHOOTRIGHT1, 2);

		if (*g_bGunSmoke)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
		}
	}

	if (EV_IsLocal(idx))
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, false);

		VectorCopy(ent->attachment[2], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] -= 25;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 5);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/elite_fire.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam2;
	vSpread[1] = args->fparam1 / 100.0;
	vSpread[2] = 0;

	vecSrc[0] += right[0] * 5;
	vecSrc[1] += right[1] * 5;
	vecSrc[2] += right[2] * 5;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
}

void EV_Knife(struct event_args_s *args)
{
	int idx;

	idx = args->entindex;

	if (EV_IsLocal(idx))
		gEngfuncs.pEventAPI->EV_WeaponAnimation(args->iparam1, 2);
}

enum galil_e
{
	GALIL_IDLE1,
	GALIL_RELOAD,
	GALIL_DRAW,
	GALIL_SHOOT1,
	GALIL_SHOOT2,
	GALIL_SHOOT3
};

void EV_FireGalil(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 10000000.0;
	angles[1] += args->iparam2 / 10000000.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GALIL_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);

		if (*g_bGunSmoke)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.26, 15, 15, 15, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, false);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 122;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 9);

	if (gEngfuncs.pfnRandomLong(0, 1))
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/galil-1.wav", 1.0, 0.4, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/galil-2.wav", 1.0, 0.4, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_762MM, 0, &tracerCount[idx - 1], 2);
}

enum famas_e
{
	FAMAS_IDLE1,
	FAMAS_RELOAD,
	FAMAS_DRAW,
	FAMAS_SHOOT1,
	FAMAS_SHOOT2,
	FAMAS_SHOOT3
};

void EV_FireFamas(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	int lefthand;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 10000000.0;
	angles[1] += args->iparam2 / 10000000.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(FAMAS_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 2);
	}

	if (EV_IsLocal(idx))
	{
		if (lefthand == 0)
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 17.0, -8.0, -14.0, true);
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 17.0, -8.0, 14.0, true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 122;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL, idx, 8);

	if (gEngfuncs.pfnRandomLong(0, 1))
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/famas-1.wav", 1.0, 0.48, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/famas-2.wav", 1.0, 0.48, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

void EV_RemoveAllDecals(struct event_args_s *args)
{
	g_pfnEV_RemoveAllDecals(args);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (g_DeadPlayerModels[i])
		{
			g_DeadPlayerModels[i]->die = 0;
			g_DeadPlayerModels[i] = 0;
		}
	}
}

void EV_CreateSmoke(struct event_args_s *args)
{
	return g_pfnEV_CreateSmoke(args);
}