#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"
#include "util.h"

extern vec3_t g_vecHullMin;
extern vec3_t g_vecHullMax;

pmtrace_t *FindHullIntersection(float *vecSrc, float *vecEnd, pmtrace_t *tr, float *pflMins, float *pfkMaxs);
BOOL IsBackFace(vec3_t anglesAttacker, vec3_t anglesVictim);

void CClientButterfly::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

BOOL CClientButterfly::Deploy(void)
{
	m_iShotsFired = 0;

	//Close disguise kit menu here
	//ShowHudMenu(0, 0);

	return GroupDeploy("models/CKF_III/v_butterfly.mdl", "models/CKF_III/wp_group_rf.mdl", BUTTERFLY_DRAW, 0, 0, "knife");
}

void CClientButterfly::Holster(void)
{
	m_bMeleeAttack = FALSE;
}

void CClientButterfly::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.05;
	m_iMeleeCrit = g_Player.GetCriticalFire(-1, g_Player.random_seed);
	m_bMeleeAttack = TRUE;
	
	SendWeaponAnim(BUTTERFLY_SLASH1 + (m_iShotsFired++) % 2);

	m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CClientButterfly::ItemPostFrame(void)
{
	vec3_t vecSrc, vecViewHeight, vecEnd, vecForward, vecRight, vecUp;

	qboolean handdown = true;

	if(!g_Player.m_iCloak)
	{
		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(vecViewHeight);
		VectorAdd(g_finalstate->playerstate.origin, vecViewHeight, vecSrc);

		gEngfuncs.pfnAngleVectors(refparams.viewangles, vecForward, vecRight, vecUp);

		VectorMA(vecSrc, 48, vecForward, vecEnd);

		pmtrace_t *tr, tr2;
		physent_t *physent;
		tr = cl_pmove->PM_TraceLineEx(vecSrc, vecEnd, PM_NORMAL, 0, CL_TraceEntity_Ignore );

		if(tr->fraction == 1)
		{
			tr = cl_pmove->PM_TraceLineEx(vecSrc, vecEnd, PM_NORMAL, 3, CL_TraceEntity_Ignore );
			if (tr->fraction < 1)
			{
				physent = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
				if (!physent || (physent->model && physent->model->name[0] == '*'))
				{
					memcpy(&tr2, tr, sizeof(pmtrace_t));
					tr = FindHullIntersection(vecSrc, tr2.endpos, &tr2, g_vecHullMin, g_vecHullMax);
				}

				VectorCopy(tr->endpos, vecEnd);
			}
		}

		if (tr->fraction < 1)
		{
			physent = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
			cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(physent->info);

			float flVol = 1;
			int fHitWorld = TRUE;

			if(pEntity && pEntity->player)
			{
				if(pEntity->curstate.team == 3-g_iTeam && IsBackFace(gEngfuncs.GetLocalPlayer()->angles, pEntity->angles))
				{
					if( !((*cls_viewmodel_sequence == BUTTERFLY_SLASH1 || *cls_viewmodel_sequence == BUTTERFLY_SLASH2 || *cls_viewmodel_sequence == BUTTERFLY_STAB) && *cls_viewmodel_starttime != 0 && g_flClientTime - *cls_viewmodel_starttime < 0.5) &&
						*cls_viewmodel_sequence != BUTTERFLY_HANDUP && *cls_viewmodel_sequence != BUTTERFLY_HANDIDLE)
					{
						SendWeaponAnim(BUTTERFLY_HANDUP);
					}
					handdown = false;
				}
			}
		}
	}

	if(*cls_viewmodel_sequence == BUTTERFLY_HANDUP && *cls_viewmodel_starttime != 0 && g_flClientTime - (*cls_viewmodel_starttime) > 0.5)
	{
		SendWeaponAnim(BUTTERFLY_HANDIDLE);
	}
	else if(*cls_viewmodel_sequence == BUTTERFLY_HANDIDLE && handdown)
	{
		SendWeaponAnim(BUTTERFLY_HANDDOWN);
	}
	CClientWeapon::ItemPostFrame();
}

void CClientButterfly::Swing(void)
{
	vec3_t vecSrc, vecViewHeight, vecEnd, vecForward, vecRight, vecUp;

	qboolean stab = false;

	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(vecViewHeight);
	VectorAdd(g_finalstate->playerstate.origin, vecViewHeight, vecSrc);

	gEngfuncs.pfnAngleVectors(refparams.viewangles, vecForward, vecRight, vecUp);

	VectorMA(vecSrc, 48, vecForward, vecEnd);

	pmtrace_t *tr, tr2;
	physent_t *physent;
	tr = cl_pmove->PM_TraceLineEx(vecSrc, vecEnd, PM_NORMAL, 0, CL_TraceEntity_Ignore );

	if(tr->fraction == 1)
	{
		tr = cl_pmove->PM_TraceLineEx(vecSrc, vecEnd, PM_NORMAL, 3, CL_TraceEntity_Ignore );
		if (tr->fraction < 1)
		{
			physent = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
			if (!physent || (physent->model && physent->model->name[0] == '*'))
			{
				memcpy(&tr2, tr, sizeof(pmtrace_t));
				tr = FindHullIntersection(vecSrc, tr2.endpos, &tr2, g_vecHullMin, g_vecHullMax);
			}

			VectorCopy(tr->endpos, vecEnd);
		}
	}

	if (tr->fraction < 1)
	{
		physent = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
		cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(physent->info);

		float flVol = 1;
		int fHitWorld = TRUE;

		if(pEntity && pEntity->player)
		{
			if(pEntity->curstate.team == 3-g_iTeam && IsBackFace(gEngfuncs.GetLocalPlayer()->angles, pEntity->angles))
			{
				SendWeaponAnim(BUTTERFLY_STAB);
			}
		}
	}

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);
}