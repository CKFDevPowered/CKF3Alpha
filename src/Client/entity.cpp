#include <memory.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_types.h"
#include "studio_event.h"
#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "pm_shared.h"
#include "game_shared/voice_status.h"

int g_iAlive = 1;
int iOnTrain[MAX_PLAYERS + 1];

int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	switch (type)
	{
		case ET_NORMAL:
		case ET_PLAYER:
		{
			if (ent->player && iOnTrain[ent->index])
			{
				VectorCopy(ent->origin, ent->curstate.origin);
				VectorCopy(ent->angles, ent->curstate.angles);
				return 1;
			}

			break;
		}

		case ET_BEAM:
		case ET_TEMPENTITY:
		case ET_FRAGMENTED:
		default:
		{
			break;
		}
	}

	if (g_iUser1)
	{
		gHUD.m_Spectator.AddOverviewEntity(type, ent, modelname);

		if ((g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE) && ent->index == g_iUser2)
			return 0;
	}
	if(gRefClientFuncs.HUD_AddEntity)
		gRefClientFuncs.HUD_AddEntity(type, ent, modelname);
	if(gCkfClientFuncs.HUD_AddEntity)
		gCkfClientFuncs.HUD_AddEntity(type, ent, modelname);

	return 1;
}

void HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client)
{
	VectorCopy(client->origin, state->origin);

	state->iuser1 = client->iuser1;
	state->iuser2 = client->iuser2;

	state->iuser3 = client->iuser3;

	state->iuser4 = client->iuser4;

	if(gCkfClientFuncs.HUD_TxferLocalOverrides)
		gCkfClientFuncs.HUD_TxferLocalOverrides(state, client);
}

void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src)
{
	iOnTrain[src->number] = src->iuser4;

	if (dst->number == gEngfuncs.GetLocalPlayer()->index)
	{
		g_iTeamNumber = g_PlayerExtraInfo[dst->number].teamnumber;
		g_iUser1 = src->iuser1;
		g_iUser2 = src->iuser2;
		g_iUser3 = src->iuser3;
	}

	if(gCkfClientFuncs.HUD_ProcessPlayerState)
		gCkfClientFuncs.HUD_ProcessPlayerState(dst, src);

	gExportfuncs.HUD_ProcessPlayerState(dst, src);
}

void HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd)
{
	ps->oldbuttons = pps->oldbuttons;
	ps->flFallVelocity = pps->flFallVelocity;
	ps->iStepLeft = pps->iStepLeft;
	ps->playerclass = pps->playerclass;

	ps->iuser4 = pps->iuser4;

	pcd->viewmodel = ppcd->viewmodel;
	pcd->m_iId = ppcd->m_iId;
	pcd->ammo_shells = ppcd->ammo_shells;
	pcd->ammo_nails = ppcd->ammo_nails;
	pcd->ammo_cells = ppcd->ammo_cells;
	pcd->ammo_rockets = ppcd->ammo_rockets;
	pcd->m_flNextAttack = ppcd->m_flNextAttack;
	pcd->fov = ppcd->fov;
	pcd->weaponanim = ppcd->weaponanim;
	pcd->tfstate = ppcd->tfstate;
	pcd->maxspeed = ppcd->maxspeed;

	pcd->deadflag = ppcd->deadflag;

	g_iAlive = (ppcd->iuser1 || (pcd->deadflag == DEAD_NO)) ? 1 : 0;

	pcd->iuser1 = ppcd->iuser1;
	pcd->iuser2 = ppcd->iuser2;
	pcd->iuser3 = ppcd->iuser3;

	if (gEngfuncs.IsSpectateOnly())
	{
		pcd->iuser1 = g_iUser1;
		pcd->iuser2 = g_iUser2;
		pcd->iuser3 = g_iUser3;
	}

	pcd->iuser4 = ppcd->iuser4;

	VectorCopy(ppcd->vuser2, pcd->vuser2);
	VectorCopy(ppcd->vuser3, pcd->vuser3);
	VectorCopy(ppcd->vuser4, pcd->vuser4);

	memcpy(wd, pwd, 32 * sizeof(weapon_data_t));

	if(gCkfClientFuncs.HUD_TxferPredictionData)
		gCkfClientFuncs.HUD_TxferPredictionData(ps, pps, pcd, ppcd, wd, pwd);
}

void HUD_CreateEntities(void)
{
	GetClientVoiceHud()->CreateEntities();

	if(gRefClientFuncs.HUD_CreateEntities)
		gRefClientFuncs.HUD_CreateEntities();
	if(gCkfClientFuncs.HUD_CreateEntities)
		gCkfClientFuncs.HUD_CreateEntities();
}

void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity)
{
	if(gCkfClientFuncs.HUD_StudioEvent)
		gCkfClientFuncs.HUD_StudioEvent(event, entity);
	/*int iMuzzleFlash = 1;

	switch (event->event)
	{
		case 5001:
		{
			if (iMuzzleFlash)
				gEngfuncs.pEfxAPI->R_MuzzleFlash((float *)&entity->attachment[0], atoi(event->options));

			break;
		}

		case 5011:
		{
			if (iMuzzleFlash)
				gEngfuncs.pEfxAPI->R_MuzzleFlash((float *)&entity->attachment[1], atoi(event->options));

			break;
		}

		case 5021:
		{
			if (iMuzzleFlash)
				gEngfuncs.pEfxAPI->R_MuzzleFlash((float *)&entity->attachment[2], atoi(event->options));

			break;
		}

		case 5031:
		{
			if (iMuzzleFlash)
				gEngfuncs.pEfxAPI->R_MuzzleFlash((float *)&entity->attachment[3], atoi(event->options));

			break;
		}

		case 5002:
		{
			gEngfuncs.pEfxAPI->R_SparkEffect((float *)&entity->attachment[0], atoi(event->options), -100, 100);
			break;
		}

		case 5004:
		{
			gEngfuncs.pfnPlaySoundByNameAtLocation((char *)event->options, 1.0, (float *)&entity->attachment[0]);
			break;
		}

		default:
		{
			break;
		}
	}*/
}