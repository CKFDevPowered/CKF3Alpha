#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "skill.h"
#include "game.h"

extern edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);

DLL_GLOBAL CHalfLifeMultiplay *g_pGameRules = NULL;
extern DLL_GLOBAL BOOL g_fGameOver;
extern int gmsgDeathMsg;
extern int gmsgMOTD;

BOOL CGameRules::CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry)
{
	if (pszAmmoName)
	{
		int iAmmoIndex = pPlayer->GetAmmoIndex(pszAmmoName);

		if (iAmmoIndex > -1)
		{
			if (pPlayer->AmmoInventory(iAmmoIndex) < iMaxCarry)
				return TRUE;
		}
	}

	return FALSE;
}

edict_t *CGameRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	edict_t *pentSpawnSpot = EntSelectSpawnPoint(pPlayer);

	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
	pPlayer->pev->v_angle = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;
	return pentSpawnSpot;
}

BOOL CGameRules::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (pPlayer->pev->deadflag != DEAD_NO)
		return FALSE;

	if (pWeapon->pszAmmo1())
	{
		if (!CanHaveAmmo(pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1()))
		{
			if (pPlayer->HasPlayerItem(pWeapon))
				return FALSE;
		}
	}
	else
	{
		if (pPlayer->HasPlayerItem(pWeapon))
			return FALSE;
	}

	return TRUE;
}

void CGameRules::RefreshSkillData(void)
{
	int iSkill = (int)CVAR_GET_FLOAT("skill");

	if (iSkill < 1)
		iSkill = 1;
	else if (iSkill > 3)
		iSkill = 3;

	gSkillData.iSkillLevel = iSkill;
	ALERT(at_console, "\nGAME SKILL LEVEL:%d\n", iSkill);

	gSkillData.agruntHealth = GetSkillCvar("sk_agrunt_health");;
	gSkillData.agruntDmgPunch = GetSkillCvar("sk_agrunt_dmg_punch");
	gSkillData.apacheHealth = GetSkillCvar("sk_apache_health");
	gSkillData.barneyHealth = GetSkillCvar("sk_barney_health");
	gSkillData.bigmommaHealthFactor = GetSkillCvar("sk_bigmomma_health_factor");
	gSkillData.bigmommaDmgSlash = GetSkillCvar("sk_bigmomma_dmg_slash");
	gSkillData.bigmommaDmgBlast = GetSkillCvar("sk_bigmomma_dmg_blast");
	gSkillData.bigmommaRadiusBlast = GetSkillCvar("sk_bigmomma_radius_blast");
	gSkillData.bullsquidHealth = GetSkillCvar("sk_bullsquid_health");
	gSkillData.bullsquidDmgBite = GetSkillCvar("sk_bullsquid_dmg_bite");
	gSkillData.bullsquidDmgWhip = GetSkillCvar("sk_bullsquid_dmg_whip");
	gSkillData.bullsquidDmgSpit = GetSkillCvar("sk_bullsquid_dmg_spit");
	gSkillData.gargantuaHealth = GetSkillCvar("sk_gargantua_health");
	gSkillData.gargantuaDmgSlash = GetSkillCvar("sk_gargantua_dmg_slash");
	gSkillData.gargantuaDmgFire = GetSkillCvar("sk_gargantua_dmg_fire");
	gSkillData.gargantuaDmgStomp = GetSkillCvar("sk_gargantua_dmg_stomp");
	gSkillData.hassassinHealth = GetSkillCvar("sk_hassassin_health");
	gSkillData.headcrabHealth = GetSkillCvar("sk_headcrab_health");
	gSkillData.headcrabDmgBite = GetSkillCvar("sk_headcrab_dmg_bite");
	gSkillData.hgruntHealth = GetSkillCvar("sk_hgrunt_health");
	gSkillData.hgruntDmgKick = GetSkillCvar("sk_hgrunt_kick");
	gSkillData.hgruntShotgunPellets = GetSkillCvar("sk_hgrunt_pellets");
	gSkillData.hgruntGrenadeSpeed = GetSkillCvar("sk_hgrunt_gspeed");
	gSkillData.houndeyeHealth = GetSkillCvar("sk_houndeye_health");
	gSkillData.houndeyeDmgBlast = GetSkillCvar("sk_houndeye_dmg_blast");
	gSkillData.slaveHealth = GetSkillCvar("sk_islave_health");
	gSkillData.slaveDmgClaw = GetSkillCvar("sk_islave_dmg_claw");
	gSkillData.slaveDmgClawrake = GetSkillCvar("sk_islave_dmg_clawrake");
	gSkillData.slaveDmgZap = GetSkillCvar("sk_islave_dmg_zap");
	gSkillData.ichthyosaurHealth = GetSkillCvar("sk_ichthyosaur_health");
	gSkillData.ichthyosaurDmgShake = GetSkillCvar("sk_ichthyosaur_shake");
	gSkillData.leechHealth = GetSkillCvar("sk_leech_health");
	gSkillData.leechDmgBite = GetSkillCvar("sk_leech_dmg_bite");
	gSkillData.controllerHealth = GetSkillCvar("sk_controller_health");
	gSkillData.controllerDmgZap = GetSkillCvar("sk_controller_dmgzap");
	gSkillData.controllerSpeedBall = GetSkillCvar("sk_controller_speedball");
	gSkillData.controllerDmgBall = GetSkillCvar("sk_controller_dmgball");
	gSkillData.nihilanthHealth = GetSkillCvar("sk_nihilanth_health");
	gSkillData.nihilanthZap = GetSkillCvar("sk_nihilanth_zap");
	gSkillData.scientistHealth = GetSkillCvar("sk_scientist_health");
	gSkillData.snarkHealth = GetSkillCvar("sk_snark_health");
	gSkillData.snarkDmgBite = GetSkillCvar("sk_snark_dmg_bite");
	gSkillData.snarkDmgPop = GetSkillCvar("sk_snark_dmg_pop");
	gSkillData.zombieHealth = GetSkillCvar("sk_zombie_health");
	gSkillData.zombieDmgOneSlash = GetSkillCvar("sk_zombie_dmg_one_slash");
	gSkillData.zombieDmgBothSlash = GetSkillCvar("sk_zombie_dmg_both_slash");
	gSkillData.turretHealth = GetSkillCvar("sk_turret_health");
	gSkillData.miniturretHealth = GetSkillCvar("sk_miniturret_health");
	gSkillData.sentryHealth = GetSkillCvar("sk_sentry_health");
	gSkillData.plrDmgCrowbar = GetSkillCvar("sk_plr_crowbar");
	gSkillData.plrDmg9MM = GetSkillCvar("sk_plr_9mm_bullet");
	gSkillData.plrDmg357 = GetSkillCvar("sk_plr_357_bullet");
	gSkillData.plrDmgMP5 = GetSkillCvar("sk_plr_9mmAR_bullet");
	gSkillData.plrDmgM203Grenade = GetSkillCvar("sk_plr_9mmAR_grenade");
	gSkillData.plrDmgBuckshot = GetSkillCvar("sk_plr_buckshot");
	gSkillData.plrDmgCrossbowClient = GetSkillCvar("sk_plr_xbow_bolt_client");
	gSkillData.plrDmgCrossbowMonster = GetSkillCvar("sk_plr_xbow_bolt_monster");
	gSkillData.plrDmgRPG = GetSkillCvar("sk_plr_rpg");
	gSkillData.plrDmgGauss = GetSkillCvar("sk_plr_gauss");
	gSkillData.plrDmgEgonNarrow = GetSkillCvar("sk_plr_egon_narrow");
	gSkillData.plrDmgEgonWide = GetSkillCvar("sk_plr_egon_wide");
	gSkillData.plrDmgHandGrenade = GetSkillCvar("sk_plr_hand_grenade");
	gSkillData.plrDmgSatchel = GetSkillCvar("sk_plr_satchel");
	gSkillData.plrDmgTripmine = GetSkillCvar("sk_plr_tripmine");
	gSkillData.monDmg12MM = GetSkillCvar("sk_12mm_bullet");
	gSkillData.monDmgMP5 = GetSkillCvar("sk_9mmAR_bullet");
	gSkillData.monDmg9MM = GetSkillCvar("sk_9mm_bullet");
	gSkillData.monDmgHornet = GetSkillCvar("sk_hornet_dmg");
	gSkillData.plrDmgHornet = 7;
	gSkillData.suitchargerCapacity = GetSkillCvar("sk_suitcharger");
	gSkillData.batteryCapacity = GetSkillCvar("sk_battery");
	gSkillData.healthchargerCapacity = GetSkillCvar("sk_healthcharger");
	gSkillData.healthkitCapacity = GetSkillCvar("sk_healthkit");
	gSkillData.scientistHeal = GetSkillCvar("sk_scientist_heal");
	gSkillData.monHead = GetSkillCvar("sk_monster_head");
	gSkillData.monChest = GetSkillCvar("sk_monster_chest");
	gSkillData.monStomach = GetSkillCvar("sk_monster_stomach");
	gSkillData.monLeg = GetSkillCvar("sk_monster_leg");
	gSkillData.monArm = GetSkillCvar("sk_monster_arm");
	gSkillData.plrHead = GetSkillCvar("sk_player_head");
	gSkillData.plrChest = GetSkillCvar("sk_player_chest");
	gSkillData.plrStomach = GetSkillCvar("sk_player_stomach");
	gSkillData.plrLeg = GetSkillCvar("sk_player_leg");
	gSkillData.plrArm = GetSkillCvar("sk_player_arm");
}

CGameRules *InstallGameRules(void)
{
	SERVER_COMMAND("exec game.cfg\n");
	SERVER_EXECUTE();

	//if (!gpGlobals->deathmatch)
	//	return new CHalfLifeTraining;

	return new CHalfLifeMultiplay;
}