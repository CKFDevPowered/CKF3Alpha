#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "game.h"

extern DLL_GLOBAL BOOL g_fGameOver;

CHalfLifeTraining::CHalfLifeTraining(void)
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

void CHalfLifeTraining::HostageDied(void)
{
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(1);

	if (pPlayer)
		pPlayer->pev->radsuit_finished = gpGlobals->time + 3;
}

edict_t *CHalfLifeTraining::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	CBaseEntity *pSpawnSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");

	if (!pSpawnSpot)
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	edict_t *pentSpawnSpot = ENT(pSpawnSpot->pev);
	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
	pPlayer->pev->v_angle = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;
	return pentSpawnSpot;
}

extern int gmsgStatusIcon;
extern int gmsgBlinkAcct;

void CHalfLifeTraining::PlayerThink(CBasePlayer *pPlayer)
{
	if (pPlayer->pev->radsuit_finished && gpGlobals->time > pPlayer->pev->radsuit_finished)
		SERVER_COMMAND("reload\n");

	if (!pPlayer->m_iAccount)
	{
		if (pPlayer->pev->scale)
			pPlayer->m_iAccount = (int)pPlayer->pev->scale;
	}

	if (pPlayer->m_iTeam == TEAM_UNASSIGNED)
	{
		pPlayer->SetProgressBarTime(0);
		pPlayer->m_bHasDefuseKit = pPlayer->pev->ideal_yaw != 0;
	}

	g_fGameOver = FALSE;

	pPlayer->m_iTeam = TEAM_CT;
	pPlayer->m_bAllowAttack = TRUE;
	pPlayer->m_fLastMovement = gpGlobals->time;

	if (pPlayer->m_pActiveItem)
		pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
	else
		pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS;

	if (pPlayer->HasNamedPlayerItem("weapon_c4"))
	{
		if (pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("C4")] <= 0)
		{
			pPlayer->m_bIsBombGuy = FALSE;

			if (FClassnameIs(pPlayer->m_pActiveItem->pev, "weapon_c4"))
			{
				//pPlayer->pev->weapons &= ~(1 << WEAPON_C4);
				pPlayer->RemovePlayerItem(pPlayer->m_pActiveItem);
				pPlayer->m_pActiveItem->Drop();
			}
		}
		else
			pPlayer->m_bIsBombGuy = TRUE;
	}

	if (!pPlayer->m_bVGUIMenus)
	{
		if (m_bVGUIMenus)
			pPlayer->m_bVGUIMenus = m_bVGUIMenus;
	}

	CGrenade *pGrenade = NULL;

/*	while ((pGrenade = (CGrenade *)UTIL_FindEntityByClassname(pGrenade, "grenade")) != NULL)
	{
		if (pGrenade->m_pentCurBombTarget != NULL)
			pGrenade->m_bStartDefuser = TRUE;
	}*/

	if (pPlayer->m_iClientMapZone & MAPZONE_RESUPPLYROOM)
	{
		m_flNextGiveMoney = 1;

		if (m_fShowCanBuy == FALSE)
		{
			if (m_fShowBuyTip == FALSE)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
				WRITE_BYTE(STATUSICON_FLASH);
				WRITE_STRING("buyzone");
				WRITE_BYTE(0);
				WRITE_BYTE(160);
				WRITE_BYTE(0);
				MESSAGE_END();
			}
		}

		m_fShowCanBuy = TRUE;

		if (pPlayer->m_iAccount < 16000 && m_flNextGiveMoney == 0)
			m_flNextGiveMoney = gpGlobals->time + 5;

		if (m_flNextGiveMoney != 0 && gpGlobals->time > m_flNextGiveMoney)
		{
			if (m_fShowBuyTip == FALSE)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, pPlayer->pev);
				WRITE_BYTE(3);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
				WRITE_BYTE(STATUSICON_SHOW);
				WRITE_STRING("buyzone");
				WRITE_BYTE(0);
				WRITE_BYTE(160);
				WRITE_BYTE(0);
				MESSAGE_END();

				m_fShowBuyTip = TRUE;
			}

			pPlayer->AddAccount(16000 - pPlayer->m_iAccount, TRUE);
			m_flNextGiveMoney = 0;
		}
	}
	else
	{
		if (m_fShowCanBuy != FALSE && m_fShowBuyTip != FALSE)
			m_fShowCanBuy = FALSE;
	}

	pPlayer->pev->scale = pPlayer->m_iAccount;
	pPlayer->pev->ideal_yaw = pPlayer->m_bHasDefuseKit;
}

void CHalfLifeTraining::PlayerSpawn(CBasePlayer *pPlayer)
{
	if (pPlayer->m_bNotKilled)
		return;

	m_fShowBuyTip = FALSE;
	m_fShowCanBuy = TRUE;
	m_flNextGiveMoney = 0;

	pPlayer->m_iTeam = TEAM_CT;
	pPlayer->m_iJoiningState = JOINED;
	pPlayer->m_bNotKilled = TRUE;
	pPlayer->pev->body = 0;
	pPlayer->m_iClass = CLASS_URBAN;

	m_bVGUIMenus = pPlayer->m_bVGUIMenus;
	SET_MODEL(ENT(pPlayer->pev), "models/player.mdl");

	CBaseEntity *pWeaponEntity = NULL;

	while (pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip"))
		pWeaponEntity->Touch(pPlayer);

	pPlayer->SetPlayerModel();//(FALSE);
	pPlayer->Spawn();
	pPlayer->m_iHideHUD |= (HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);
}

BOOL CHalfLifeTraining::IsMultiplayer(void)
{
	return FALSE;
}

BOOL CHalfLifeTraining::IsDeathmatch(void)
{
	return FALSE;
}

int CHalfLifeTraining::ItemShouldRespawn(CItem *pItem)
{
	return GR_ITEM_RESPAWN_NO;
}

BOOL CHalfLifeTraining::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	return TRUE;
}

void CHalfLifeTraining::InitHUD(CBasePlayer *pl)
{
}

BOOL CHalfLifeTraining::PlayerCanBuy(CBasePlayer *pPlayer)
{
	return pPlayer->m_iClientMapZone & MAPZONE_RESUPPLYROOM;
}

void CHalfLifeTraining::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	SET_VIEW(ENT(pVictim->pev), ENT(pVictim->pev));
	FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);
}

void CHalfLifeTraining::CheckMapConditions(void)
{
}

void CHalfLifeTraining::CheckWinConditions(void)
{
}