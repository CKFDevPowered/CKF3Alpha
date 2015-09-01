#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "util.h"
#include "cvar.h"
#include "event.h"

#include "pm_materials.h"

extern vec3_t g_vecZero;
extern float *ev_punchangle;

void R_BeginTracer(int iTracerColor, int iNumTracer);
void R_BeginTracerDelayed(int iTracerColor, int iNumTracer, float flDelay);
void R_BeginTracerAttachment(int iTracerColor, int iNumTracer, cl_entity_t *pEntity, int iAttachIndex);
void R_EmitTracer(vec3_t vecSrc, vec3_t vecDst);
void R_PistolMuzzle(cl_entity_t *pEntity, int attachment);
void R_ShotgunMuzzle(cl_entity_t *pEntity, int attachment);
void R_MinigunMuzzle(cl_entity_t *pEntity, int attachment);
void R_ScattergunMuzzle(cl_entity_t *pEntity, int iAttachment);
void R_FlameThrow(cl_entity_t *pEntity, int iTeam);
void R_BulletImpact(vec3_t vecStart, vec3_t vecNormal);

float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

void V_PunchAxis( int axis, float punch )
{
	ev_punchangle[axis] += punch;
}

void EV_EjectBrass( float *origin, float *velocity, float rotation, int model, int soundtype )
{
	vec3_t endpos;
	VectorClear( endpos );
	endpos[1] = rotation;
	gEngfuncs.pEfxAPI->R_TempModel( origin, velocity, endpos, 2.5, model, soundtype );
}

qboolean EV_IsPlayer(int idx)
{
	if (idx >= 1 && idx <= gEngfuncs.GetMaxClients())
		return true;

	return false;
}

qboolean EV_IsLocal( int idx )
{
	// check if we are in some way in first person spec mode
	if ( CL_IsFirstPersonSpec()  )
		return (g_iUser2 == idx);
	else
		return gEngfuncs.pEventAPI->EV_IsLocal( idx - 1 ) ? true : false;
}

void EV_MuzzleFlash( void )
{
	// Add muzzle flash to current weapon model
	cl_entity_t *ent = gEngfuncs.GetViewModel();
	if ( !ent )
		return;

	// Or in the muzzle flash
	ent->curstate.effects |= EF_MUZZLEFLASH;
}

void EV_SetupArgs(event_args_t *args, float *origin, float *tracer_origin, float *angles, float *forward, float *right, float *up)
{
	int idx;

	idx = args->entindex;

	if (EV_IsPlayer(idx))
	{
		VectorCopy(args->origin, origin);
		VectorCopy(args->angles, angles);
		if(angles[1] < 0)
			angles[1] += 360;

		gEngfuncs.pfnAngleVectors(angles, forward, right, up);

		VectorCopy(origin, tracer_origin);
		VectorMA(tracer_origin, 16, forward, tracer_origin);
		if(EV_IsLocal(idx))
		{
			if(cl_righthand->value > 0)
				VectorMA(tracer_origin, 4, right, tracer_origin);
			else
				VectorMA(tracer_origin, -4, right, tracer_origin);
		}
		VectorMA(tracer_origin, -4, up, tracer_origin);
	}
}

void EV_SetupArgsMinigun(event_args_t *args, float *origin, float *tracer_origin, float *angles, float *forward, float *right, float *up)
{
	int idx;

	idx = args->entindex;

	if (EV_IsPlayer(idx))
	{
		VectorCopy(args->origin, origin);
		VectorCopy(args->angles, angles);
		if(angles[1] < 0)
			angles[1] += 360;

		gEngfuncs.pfnAngleVectors(angles, forward, right, up);

		VectorCopy(origin, tracer_origin);
		VectorMA(tracer_origin, 16, forward, tracer_origin);
		if(EV_IsLocal(idx))
		{
			if(cl_righthand->value > 0)
				VectorMA(tracer_origin, 1.3, right, tracer_origin);
			else
				VectorMA(tracer_origin, -1.3, right, tracer_origin);
		}
		VectorMA(tracer_origin, -4, up, tracer_origin);
	}
}

void EV_GetDefaultShellInfo(event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection)
{
	int i;
	vec3_t view_ofs;
	float fR, fU;

	int idx;

	idx = args->entindex;

	VectorClear(view_ofs);
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx))
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		else if (args->ducking == 1)
			view_ofs[2] = VEC_DUCK_VIEW;
	}

	fR = gEngfuncs.pfnRandomFloat(50, 70);
	fU = gEngfuncs.pfnRandomFloat(100, 150);

	VectorScale(velocity, 0.5);

	if (bReverseDirection)
	{
		for (i = 0; i < 3; i++)
		{
			ShellVelocity[i] = velocity[i] - right[i] * fR + up[i] * fU + forward[i];
			ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale + (velocity[i] / 5.0);
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			ShellVelocity[i] = velocity[i] + right[i] * fR + up[i] * fU + forward[i];
			ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale + (velocity[i] / 5.0);
		}
	}
}

void EV_HLDM_GunshotDecalTrace( int idx, pmtrace_t *pTrace, int iBulletType, char *decalName )
{
	int iRand, iChance;
	physent_t *pe;

	float vol = 1.0;

	if(EV_IsLocal(idx))
		vol *= 0.8;

	if(iBulletType == BULLET_PLAYER_SHOTGUN || iBulletType == BULLET_PLAYER_MINIGUN)
		vol *= 0.8;

	if(iBulletType == BULLET_PLAYER_SHOTGUN)
	{
		iRand = gEngfuncs.pfnRandomLong(0,0x7FFF);
		iChance = 0x7FFF/4;
		if ( iRand < iChance )// not every bullet makes a impact effect.
		{
			R_BulletImpact(pTrace->endpos, pTrace->plane.normal);
		}
	}

	iRand = gEngfuncs.pfnRandomLong(0,0x7FFF);
	iChance = (iBulletType == BULLET_PLAYER_SHOTGUN || iBulletType == BULLET_PLAYER_MINIGUN) ? 0x7FFF/4 : 0x7FFF/2;
	if ( iRand < iChance )// not every bullet makes a sound.
	{
		switch( iRand % 5)
		{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, CHAN_STATIC, "weapons/ric1.wav", vol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, CHAN_STATIC, "weapons/ric2.wav", vol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, CHAN_STATIC, "weapons/ric3.wav", vol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, CHAN_STATIC, "weapons/ric4.wav", vol, ATTN_NORM, 0, PITCH_NORM ); break;
		case 4:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, CHAN_STATIC, "weapons/ric5.wav", vol, ATTN_NORM, 0, PITCH_NORM ); break;
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	// Only decal brush models such as the world etc.
	if (  decalName && decalName[0] && pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{
		if ( r_decals->value )
		{
			gEngfuncs.pEfxAPI->R_DecalShoot( 
				gEngfuncs.pEfxAPI->Draw_DecalIndex( gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalName ) ), 
				gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace ), 0, pTrace->endpos, 0 );
		}
	}
}

char *EV_HLDM_DamageDecal( physent_t *pe )
{
	static char decalname[ 32 ];
	int idx;

	if ( pe->classnumber == 1 )
	{
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else if ( pe->rendermode != kRenderNormal )
	{
		strcpy( decalname, "{bproof1" );
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong( 0, 4 );
		sprintf( decalname, "{shot%i", idx + 1 );
	}
	return decalname;
}

void EV_HLDM_DecalGunshot( int idx, pmtrace_t *pTrace, int iBulletType )
{
	physent_t *pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	if ( pe && pe->solid == SOLID_BSP )
	{
		switch( iBulletType )
		{
		default:
			// smoke and decal
			EV_HLDM_GunshotDecalTrace( idx, pTrace, iBulletType, EV_HLDM_DamageDecal( pe ) );
			break;
		}
	}
}

float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[ 64 ];
	char szbuffer[ 64 ];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if ( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if ( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );
		
		if ( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}
			
			// '}}'
			strcpy( szbuffer, pTextureName );
			szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;
				
			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );	
		}
	}
	
	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	if(EV_IsLocal(idx))
		fvol *= 0.8;

	if(iBulletType == BULLET_PLAYER_SHOTGUN)
		fvol *= 0.8;

	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0,cnt-1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0,0xf) );
	return fvolbar;
}

void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecTracerSrc, float *vecSpread, float flDistance, int iBulletType, int iTracerColor, int iRandSeed)
{
	int i;
	pmtrace_t *tr;
	int iShot;

	for (iShot = 1; iShot <= cShots; iShot++)
	{
		vec3_t vecDir, vecEnd;
		float x, y;

		if ((iBulletType == BULLET_PLAYER_SHOTGUN || iBulletType == BULLET_PLAYER_MINIGUN) && cShots > 1)
		{
			x = UTIL_SharedRandomFloat( iRandSeed + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( iRandSeed + ( 1 + iShot ) , -0.5, 0.5 );
			y = UTIL_SharedRandomFloat( iRandSeed + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( iRandSeed + ( 3 + iShot ), -0.5, 0.5 );

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = forward[i] + x * vecSpread[0] * right[i] + y * vecSpread[1] * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}
		else
		{
			x = UTIL_SharedRandomFloat( iRandSeed , -0.5, 0.5 ) + UTIL_SharedRandomFloat( iRandSeed + ( 1 ) , -0.5, 0.5 );
			y = UTIL_SharedRandomFloat( iRandSeed + ( 2 ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( iRandSeed + ( 3 ), -0.5, 0.5 );

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = forward[i] + x * vecSpread[0] * right[i] + y * vecSpread[1] * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}

		tr = cl_pmove->PM_TraceLineEx(vecSrc, vecEnd, PM_NORMAL, 2, CL_TraceEntity_Ignore );

		if(iBulletType != BULLET_PLAYER_SNIPERIFLE)
		{
			if(iBulletType == BULLET_PLAYER_SHOTGUN)
			{
				if(RANDOM_LONG(0, 0x7FFF) < 0x7FFF/2)
					R_EmitTracer(vecTracerSrc, tr->endpos);
			}
			else if(iBulletType == BULLET_PLAYER_MINIGUN)
			{
				if(RANDOM_LONG(0, 0x7FFF) < 0x7FFF/3)
					R_EmitTracer(vecTracerSrc, tr->endpos);
			}
			else
			{
				R_EmitTracer(vecTracerSrc, tr->endpos);
			}
		}

		if (tr->fraction != 1.0)
		{
			switch (iBulletType)
			{
				case BULLET_PLAYER_SHOTGUN:
				case BULLET_PLAYER_MINIGUN:
				{
					EV_HLDM_DecalGunshot(idx, tr, iBulletType);
					break;
				}
				case BULLET_PLAYER_SNIPERIFLE:
				case BULLET_PLAYER_TF2:
				{
					EV_HLDM_PlayTextureSound(idx, tr, vecSrc, vecEnd, iBulletType);
					EV_HLDM_DecalGunshot(idx, tr, iBulletType);
					break;
				}
				default:
					break;
			}
		}
	}
}

void EV_FireScattergun( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		if(!CL_IsThirdPerson())
		{
			V_PunchAxis(0, 4.5);
		}

		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/scattergun_shoot.wav", 1.0, 0.5, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	if(args->iparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_ShotgunMuzzle(ent, 0);
	R_BeginTracer(args->iparam1, 10);

	if(args->bparam1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, g_vecZero, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, 0);
		EV_HLDM_FireBullets(idx, forward, right, up, 9, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, args->iparam2);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 10, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, args->iparam2);
	}
	CL_FinishPMTrace();
}

void EV_FirePistol( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/pistol_shoot.wav", 1.0, 0.8, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	
	if(args->iparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_PistolMuzzle(ent, 0);
	R_BeginTracer(args->iparam1, 1);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_TF2, args->iparam1, args->iparam2);
	CL_FinishPMTrace();
}

void EV_FireShotgun( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if(EV_IsLocal(idx))
	{
		if(!CL_IsThirdPerson())
		{
			if(g_iClass == CLASS_HEAVY || g_iClass == CLASS_SOLDIER)
				V_PunchAxis(0, 2.0);
			else
				V_PunchAxis(0, 3.0);
		}
		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/shotgun_shoot.wav", 1.0, 0.5, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	if(args->iparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_ShotgunMuzzle(ent, 0);
	R_BeginTracer(args->iparam1, 10);

	if(args->bparam1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, g_vecZero, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, 0);
		EV_HLDM_FireBullets(idx, forward, right, up, 9, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, args->iparam2);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 10, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_SHOTGUN, args->iparam1, args->iparam2);
	}

	CL_FinishPMTrace();
}

void EV_FireRevolver( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/revolver_shoot.wav", 1.0, 0.8, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	
	if(args->iparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_PistolMuzzle(ent, 0);
	R_BeginTracer(args->iparam1, 1);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_TF2, args->iparam1, args->iparam2);

	CL_FinishPMTrace();
}

void EV_FireSMG( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/smg_shoot.wav", 1.0, 0.8, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	if(args->iparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_PistolMuzzle(ent, 0);
	R_BeginTracer(args->iparam1, 1);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_TF2, args->iparam1, args->iparam2);

	CL_FinishPMTrace();
}

void EV_FireMelee(event_args_t *args)
{
	int idx;
	char *sample = NULL;

	idx = args->entindex;

	switch(args->iparam1)
	{
	case WEAPON_BOTTLE:
		{
			if(args->iparam2 == 1)
			{
				if(*cls_viewmodel_sequence == BOTTLE_SLASH1_BROKEN || *cls_viewmodel_sequence == BOTTLE_SLASH2_BROKEN)
					HudWeaponAnimEx(BOTTLE_SLASH1 + (*cls_viewmodel_sequence) - BOTTLE_SLASH1_BROKEN, 0, 0, *cls_viewmodel_starttime);
				else if(*cls_viewmodel_sequence == BOTTLE_DRAW_BROKEN)
					HudWeaponAnimEx(BOTTLE_DRAW, 0, 0, *cls_viewmodel_starttime);
				else
					HudWeaponAnim(BOTTLE_IDLE);
				return;
			}
			sample = "CKF_III/melee_swing.wav";
			break;
		}
	case WEAPON_BUTTERFLY:
		{
			sample = "CKF_III/butterfly_swing.wav";
			break;
		}
	case WEAPON_WRENCH:
		{
			sample = "CKF_III/wrench_swing.wav";
			break;
		}
	default:
		{
			sample = "CKF_III/melee_swing.wav";
			break;
		}
	}

	if(sample)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_WEAPON, sample, 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
	if(args->bparam1)//crit shot
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}

void EV_FireLauncher(event_args_t *args)
{
	int idx;
	char *sample = NULL;

	idx = args->entindex;

	switch(args->iparam1)
	{
	case WEAPON_ROCKETLAUNCHER:
		sample = "CKF_III/rocketlauncher_shoot.wav";break;
	case WEAPON_GRENADELAUNCHER:
		if(EV_IsLocal(idx) && !CL_IsThirdPerson())
		{
			V_PunchAxis(0, 2.0);
		}
		sample = "CKF_III/grenadelauncher_shoot.wav";break;
	case WEAPON_STICKYLAUNCHER:
		if(EV_IsLocal(idx) && !CL_IsThirdPerson())
		{
			V_PunchAxis(0, 2.0);
		}
		sample = "CKF_III/stickylauncher_shoot.wav";break;
	default:
		break;
	}
	if(sample)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_WEAPON, sample, 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
	if(args->bparam1)//crit shot
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}

void EV_StickyLauncher(event_args_t *args)
{
	if(args->iparam2 == 1)
	{
		gHookFuncs.S_StopSound(args->entindex, CHAN_WEAPON);
	}
	else if(args->iparam2 == 2)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(args->entindex, args->origin, CHAN_WEAPON, "CKF_III/stickylauncher_charge.wav", 1.0, 1.0, 0, PITCH_NORM);
	}
}

void EV_Launcher(event_args_t *args)
{
	if(args->iparam2 == 0)
	{
		EV_FireLauncher(args);
		return;
	}
	switch(args->iparam1)
	{
	case WEAPON_STICKYLAUNCHER:
		EV_StickyLauncher(args);
		break;
	default:
		break;
	}
}

void EV_FireMinigun(event_args_t *args)
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;
	vec3_t vSpread;
	idx = args->entindex;

	EV_SetupArgsMinigun(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
	}

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam1;

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = cl_viewent;
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_MinigunMuzzle(ent, 0);
	R_BeginTracerDelayed(args->iparam1, 10, 0.02);

	if(args->bparam1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, g_vecZero, 8192, BULLET_PLAYER_MINIGUN, args->iparam1, 0);
		EV_HLDM_FireBullets(idx, forward, right, up, 9, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_MINIGUN, args->iparam1, args->iparam2);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 10, origin, tracer_origin, vSpread, 8192, BULLET_PLAYER_MINIGUN, args->iparam1, args->iparam2);
	}

	CL_FinishPMTrace();
}

void EV_Minigun(event_args_t *args)
{
	int idx;
	char *sample = NULL;

	idx = args->entindex;

	if(!args->bparam2)
	{
		switch(args->iparam1)
		{
		case STATE_MINIGUN_NONE:
			break;
		case STATE_MINIGUN_WINDUP:
			sample = "CKF_III/minigun_wind_up.wav";break;
		case STATE_MINIGUN_SPIN:
			sample = "CKF_III/minigun_spin.wav";break;
		case STATE_MINIGUN_WINDDOWN:
			sample = "CKF_III/minigun_wind_down.wav";break;
		case STATE_MINIGUN_FIRE:
			sample = "CKF_III/minigun_shoot.wav";break;
		case STATE_MINIGUN_FIRECRIT:
			sample = "CKF_III/minigun_shoot_crit.wav";break;
		default:
			break;
		}
		if(sample)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_WEAPON, sample, 1.0, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{
			gHookFuncs.S_StopSound(idx, CHAN_WEAPON);
		}
	}
	else
	{
		EV_FireMinigun(args);
	}
}

void EV_Sniperifle(event_args_t *args)
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;

	if(args->iparam2 == 1)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(args->entindex, args->origin, CHAN_WEAPON, "CKF_III/zoom.wav", 0.8, 2.4, 0, PITCH_NORM);
		return;
	}

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/sniperifle_shoot.wav", 1.0, 0.6, 0, PITCH_NORM);
	if(args->iparam1 >= 2)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	CL_SetupPMTrace(idx);

	cl_entity_t *ent;
	if(EV_IsLocal(idx) && !CL_IsThirdPerson())
	{
		ent = gEngfuncs.GetViewModel();
	}
	else
	{
		ent = gEngfuncs.GetEntityByIndex(idx);
	}
	R_PistolMuzzle(ent, 0);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, origin, tracer_origin, g_vecZero, 8192, BULLET_PLAYER_SNIPERIFLE, 0, 0);

	CL_FinishPMTrace();
}

void EV_Flamethrower(event_args_t *args)
{
	int idx;
	char *sample = NULL;

	idx = args->entindex;

	switch(args->iparam1)
	{
	case STATE_FLAMETHROWER_IDLE:
		{
			R_FlameThrow(gEngfuncs.GetEntityByIndex(idx), 0);
			break;
		}
	case STATE_FLAMETHROWER_START:
		{
			R_FlameThrow(gEngfuncs.GetEntityByIndex(idx), 3);
			sample = "CKF_III/flamethrower_start.wav";
			break;
		}
	case STATE_FLAMETHROWER_LOOP:
		{
			R_FlameThrow(gEngfuncs.GetEntityByIndex(idx), 3);
			sample = "CKF_III/flamethrower_loop.wav";
			break;
		}
	case STATE_FLAMETHROWER_LOOP_CRIT:
		{
			R_FlameThrow(gEngfuncs.GetEntityByIndex(idx), args->iparam2);
			sample = "CKF_III/flamethrower_loop_crit.wav";
			break;
		}
	case STATE_FLAMETHROWER_AIRBLAST:
		{
			R_FlameThrow(gEngfuncs.GetEntityByIndex(idx), 0);
			sample = "CKF_III/flamethrower_airblast.wav";
			break;
		}
	default:
		break;
	}
	if(sample)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_WEAPON, sample, 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
	else
	{
		gHookFuncs.S_StopSound(idx, CHAN_WEAPON);
	}
}

void EV_Medigun(event_args_t *args)
{
	int idx = args->entindex;

	if(args->iparam1 == 0)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_WEAPON, "CKF_III/medigun_no_target.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}

void EV_FireSyringegun( event_args_t *args )
{
	int idx;
	vec3_t origin, tracer_origin, angles;
	vec3_t up, right, forward;

	idx = args->entindex;

	EV_SetupArgs(args, origin, tracer_origin, angles, forward, right, up);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "CKF_III/syringegun_shoot.wav", 1.0, 0.8, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	
	if(args->bparam1)//Crit?
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "CKF_III/crit_shot.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
}

void EV_HookEvents(void)
{
	gEngfuncs.pfnHookEvent( "events/ckf_scattergun.sc", EV_FireScattergun );
	gEngfuncs.pfnHookEvent( "events/ckf_pistol.sc", EV_FirePistol );
	gEngfuncs.pfnHookEvent( "events/ckf_shotgun.sc", EV_FireShotgun );
	gEngfuncs.pfnHookEvent( "events/ckf_revolver.sc", EV_FireRevolver);
	gEngfuncs.pfnHookEvent( "events/ckf_smg.sc", EV_FireSMG);
	gEngfuncs.pfnHookEvent( "events/ckf_melee.sc", EV_FireMelee);
	gEngfuncs.pfnHookEvent( "events/ckf_launcher.sc", EV_Launcher);
	gEngfuncs.pfnHookEvent( "events/ckf_minigun.sc", EV_Minigun);
	gEngfuncs.pfnHookEvent( "events/ckf_sniperifle.sc", EV_Sniperifle);
	gEngfuncs.pfnHookEvent( "events/ckf_flamethrower.sc", EV_Flamethrower);
	gEngfuncs.pfnHookEvent( "events/ckf_medigun.sc", EV_Medigun);
	gEngfuncs.pfnHookEvent( "events/ckf_syringegun.sc", EV_FireSyringegun);
}