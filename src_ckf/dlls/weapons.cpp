#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "hltv.h"

DLL_GLOBAL short g_sModelIndexLaser;
DLL_GLOBAL const char *g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL short g_sModelIndexLaserDot;
DLL_GLOBAL short g_sModelIndexFireball;
DLL_GLOBAL short g_sModelIndexFireball2;
DLL_GLOBAL short g_sModelIndexFireball3;
DLL_GLOBAL short g_sModelIndexFireball4;
DLL_GLOBAL short g_sModelIndexSmoke;
DLL_GLOBAL short g_sModelIndexSmokePuff;
DLL_GLOBAL short g_sModelIndexBubbles;
DLL_GLOBAL short g_sModelIndexBloodDrop;
DLL_GLOBAL short g_sModelIndexBloodSpray;
DLL_GLOBAL short g_sModelIndexRadio;

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

extern int gEvilImpulse101;
extern int gmsgCurWeapon;
extern int gmsgWeaponAnimEx;

MULTIDAMAGE gMultiDamage;

int MaxAmmoCarry(int iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;

		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}

void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount = 0;
	gMultiDamage.type = 0;
}

void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	if (!gMultiDamage.pEntity)
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type, gMultiDamage.crit);
}

void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType, int iCrit)
{
	if (!pEntity)
		return;

	if(!gMultiDamage.pEntity)
	{
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
		gMultiDamage.type = 0;
	}
	else if(pEntity != gMultiDamage.pEntity || gMultiDamage.crit != iCrit)
	{
		ApplyMultiDamage(pevInflictor, pevInflictor);
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
		gMultiDamage.type = 0;
	}

	gMultiDamage.crit = iCrit;
	gMultiDamage.amount += flDamage;
	gMultiDamage.type |= bitsDamageType;
}

void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, flDamage);
}

void DecalGunshot(TraceResult *pTrace, int iBulletType, BOOL bShowUnEffect, entvars_t *pevAttacker, BOOL bShowSparks)
{
	// HZ Fixed
	if ( !UTIL_IsValidEntity(pTrace->pHit) ) return;

	if ( VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		if ( !FNullEnt(pTrace->pHit) ) pEntity = CBaseEntity::Instance(pTrace->pHit);
		if ( !pEntity )
		{
			iBulletType == BULLET_PLAYER_CROWBAR ? UTIL_DecalTrace( pTrace, DECAL_BIGSHOT1 ) : UTIL_GunshotDecalTrace( pTrace, RANDOM_LONG(DECAL_GUNSHOT1, DECAL_GUNSHOT5) );
			return;
		}

		switch( iBulletType )
		{
			case BULLET_PLAYER_9MM:
			case BULLET_MONSTER_9MM:
			case BULLET_PLAYER_MP5:
			case BULLET_MONSTER_MP5:
			case BULLET_PLAYER_BUCKSHOT:
			case BULLET_PLAYER_357:
			case BULLET_MONSTER_12MM:
				UTIL_GunshotDecalTrace( pTrace, pEntity->DamageDecal( DMG_BULLET ) );
				break;
			case BULLET_PLAYER_CROWBAR:
				UTIL_DecalTrace( pTrace, pEntity->DamageDecal( DMG_CLUB ) );
				break;
			default:
				UTIL_GunshotDecalTrace( pTrace, pEntity->DamageDecal( DMG_BULLET ) );
				break;
		}
	}
}

void EjectBrass(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(soundtype);
	WRITE_BYTE(25);
	MESSAGE_END();
}

void EjectBrass2(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype, entvars_t *pev)
{
	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pev);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(0);
	WRITE_BYTE(5);
	MESSAGE_END();
}

int giAmmoIndex = 0;

void AddAmmoNameToAmmoRegistry(const char *szAmmoName)
{
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (!stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoName))
			return;
	}

	giAmmoIndex++;

	if (giAmmoIndex >= MAX_AMMO_SLOTS)
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoName;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;
}

void UTIL_PrecacheOtherWeapon(const char *szClassname)
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
		return;
	}

	CBaseEntity *pEntity = CBaseEntity::Instance(VARS(pent));

	if (pEntity)
	{
		ItemInfo II;
		pEntity->Precache();
		memset(&II, 0, sizeof II);

		if (((CBasePlayerItem *)pEntity)->GetItemInfo(&II))
		{
			CBasePlayerItem::ItemInfoArray[II.iId] = II;

			if (II.pszAmmo1 && *II.pszAmmo1)
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);

			if (II.pszAmmo2 && *II.pszAmmo2)
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);

			memset(&II, 0, sizeof II);
		}
	}

	REMOVE_ENTITY(pent);
}

void W_Precache(void)
{
	memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
	giAmmoIndex = 0;

	//ckfprecacheother
	UTIL_PrecacheOther("item_healthkit");
	UTIL_PrecacheOther("item_ammobox");

	UTIL_PrecacheOther("ammo_primary");
	UTIL_PrecacheOther("ammo_secondary");
	UTIL_PrecacheOtherWeapon("weapon_rocketlauncher");
	UTIL_PrecacheOther("pj_rocket");
	UTIL_PrecacheOtherWeapon("weapon_sniperifle");
	UTIL_PrecacheOther("laser_spot");
	UTIL_PrecacheOtherWeapon("weapon_smg");
	UTIL_PrecacheOtherWeapon("weapon_kukri");
	UTIL_PrecacheOtherWeapon("weapon_shotgun");
	UTIL_PrecacheOtherWeapon("weapon_shovel");
	UTIL_PrecacheOtherWeapon("weapon_stickylauncher");
	UTIL_PrecacheOther("pj_sticky");
	UTIL_PrecacheOtherWeapon("weapon_grenadelauncher");
	UTIL_PrecacheOther("pj_grenade");
	UTIL_PrecacheOtherWeapon("weapon_bottle");
	UTIL_PrecacheOtherWeapon("weapon_scattergun");
	UTIL_PrecacheOtherWeapon("weapon_pistol");
	UTIL_PrecacheOtherWeapon("weapon_bat");
	UTIL_PrecacheOtherWeapon("weapon_minigun");
	UTIL_PrecacheOtherWeapon("weapon_fist");
	UTIL_PrecacheOtherWeapon("weapon_flamethrower");
	UTIL_PrecacheOtherWeapon("weapon_fireaxe");
	UTIL_PrecacheOtherWeapon("weapon_revolver");
	UTIL_PrecacheOtherWeapon("weapon_sapper");
	UTIL_PrecacheOtherWeapon("weapon_butterfly");
	UTIL_PrecacheOtherWeapon("weapon_syringegun");
	UTIL_PrecacheOther("pj_syringe");
	UTIL_PrecacheOtherWeapon("weapon_medigun");
	UTIL_PrecacheOtherWeapon("weapon_bonesaw");
	UTIL_PrecacheOtherWeapon("weapon_disguisekit");
	UTIL_PrecacheOtherWeapon("weapon_wrench");
	UTIL_PrecacheOtherWeapon("weapon_buildpda");
	UTIL_PrecacheOther("buildable_sentry");
	UTIL_PrecacheOther("buildable_dispenser");
	UTIL_PrecacheOther("pj_sapper");
	UTIL_PrecacheOtherWeapon("weapon_destroypda");
	UTIL_PrecacheOther("buildable_teleporter");

	if (g_pGameRules->IsDeathmatch())
		UTIL_PrecacheOther("weaponbox");

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");
	g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
	g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
	g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
	g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
	g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");
	g_sModelIndexLaser = PRECACHE_MODEL((char *)g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/CKF_III/sniperdot.spr");

	g_sModelIndexPlayerClass[CLASS_SCOUT-1] = PRECACHE_MODEL("models/player/ckf_scout/ckf_scout.mdl");
	g_sModelIndexPlayerClass[CLASS_SOLDIER-1] = PRECACHE_MODEL("models/player/ckf_soldier/ckf_soldier.mdl");
	g_sModelIndexPlayerClass[CLASS_PYRO-1] = PRECACHE_MODEL("models/player/ckf_pyro/ckf_pyro.mdl");
	g_sModelIndexPlayerClass[CLASS_DEMOMAN-1] = PRECACHE_MODEL("models/player/ckf_demoman/ckf_demoman.mdl");
	g_sModelIndexPlayerClass[CLASS_HEAVY-1] = PRECACHE_MODEL("models/player/ckf_heavy/ckf_heavy.mdl");
	g_sModelIndexPlayerClass[CLASS_ENGINEER-1] = PRECACHE_MODEL("models/player/ckf_engineer/ckf_engineer.mdl");
	g_sModelIndexPlayerClass[CLASS_MEDIC-1] = PRECACHE_MODEL("models/player/ckf_medic/ckf_medic.mdl");
	g_sModelIndexPlayerClass[CLASS_SNIPER-1] = PRECACHE_MODEL("models/player/ckf_sniper/ckf_sniper.mdl");
	g_sModelIndexPlayerClass[CLASS_SPY-1] = PRECACHE_MODEL("models/player/ckf_spy/ckf_spy.mdl");

	//Sentry
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv1.mdl");
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv2.mdl");
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv3.mdl");
	PRECACHE_SOUND("CKF_III/build_deploy.wav");
	PRECACHE_SOUND("CKF_III/sentry_shoot.wav");

	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_2bone.mdl");
	PRECACHE_SOUND("weapons/debris1.wav");
	PRECACHE_SOUND("weapons/debris2.wav");
	PRECACHE_SOUND("weapons/debris3.wav");
	PRECACHE_SOUND("weapons/bullet_hit1.wav");
	PRECACHE_SOUND("weapons/bullet_hit2.wav");
	PRECACHE_SOUND("items/weapondrop1.wav");
	PRECACHE_SOUND("weapons/generic_reload.wav");

	PRECACHE_SOUND("CKF_III/explode1.wav");
	PRECACHE_SOUND("CKF_III/explode2.wav");
	PRECACHE_SOUND("CKF_III/explode3.wav");
	PRECACHE_SOUND("CKF_III/draw_melee.wav");
	PRECACHE_SOUND("CKF_III/melee_swing.wav");
	PRECACHE_SOUND("CKF_III/null.wav");
	PRECACHE_SOUND("CKF_III/multijump.wav");
	PRECACHE_SOUND("CKF_III/spy_cloak.wav");
	PRECACHE_SOUND("CKF_III/spy_uncloak.wav");
	PRECACHE_SOUND("CKF_III/spy_disguise.wav");
	PRECACHE_SOUND("CKF_III/regenerate.wav");
	PRECACHE_SOUND("CKF_III/zoom.wav");
}

void CBasePlayerItem::SetObjectCollisionBox(void)
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

void CBasePlayerItem::FallInit(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(&CBasePlayerItem::FallThink);

	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayerItem::FallThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		if (!FNullEnt(pev->owner))
		{
			int pitch = 95 + RANDOM_LONG(0, 29);
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", VOL_NORM, ATTN_NORM, 0, pitch);
		}

		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize();
	}
}

void CBasePlayerItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		if (g_pGameRules->IsMultiplayer())
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);

		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(pev, pev->origin);
	SetTouch(&CBasePlayerItem::DefaultTouch);

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 1;
	}
	else
		SetThink(NULL);
}

void CBasePlayerItem::AttemptToMaterialize(void)
{
	float time = g_pGameRules->FlWeaponTryRespawn(this);

	if (!time)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

void CBasePlayerItem::CheckRespawn(void)
{
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
		case GR_WEAPON_RESPAWN_YES: return;
		case GR_WEAPON_RESPAWN_NO: return;
	}
}

CBaseEntity *CBasePlayerItem::Respawn(void)
{
	CBaseEntity *pNewWeapon = CBaseEntity::Create((char *)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

	if (pNewWeapon)
	{
		pNewWeapon->pev->effects |= EF_NODRAW;
		pNewWeapon->SetTouch(NULL);
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
	}
	else
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));

	return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		if (gEvilImpulse101)
			UTIL_Remove(this);

		return;
	}

	PreAttachPlayer(pPlayer);

	if (pOther->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

//void CBasePlayerWeapon::EjectBrassLate(void)
//{
//	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
//	Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(-50, -70) + gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;
//	EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -9 + gpGlobals->v_forward * 16 + gpGlobals->v_right * 9, vecShellVelocity, pev->angles.y, m_iShellId, TE_BOUNCE_SHELL);
//}

//valvesb
void CBasePlayerWeapon::ItemPostFrame(void)
{
	/*if (m_pPlayer->m_flEjectBrass)
	{
		if (gpGlobals->time > m_pPlayer->m_flEjectBrass)
		{
			m_pPlayer->m_flEjectBrass = 0;
			//client predicted
			EjectBrassLate();
		}
	}*/

	if ((m_fInReload || m_fInSpecialReload) && m_flNextReload < UTIL_WeaponTimeBase())
	{
		Reloaded();
	}

	if(m_bMeleeAttack && m_flMeleeAttack <= UTIL_WeaponTimeBase())
	{
		m_bMeleeAttack = FALSE;
		Swing();		
	}
	else if((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= UTIL_WeaponTimeBase())
	{
		if (pszAmmo1() && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()])
			m_fFireOnEmpty = TRUE;

		SecondaryAttack();
		//m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() && m_pPlayer->PlayerCanAttack())
	{
		if ((!m_iClip && pszAmmo1()) || (iMaxClip() == WEAPON_NOCLIP && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
			m_fFireOnEmpty = TRUE;

		PrimaryAttack();
	}
	else if ((m_pPlayer->pev->button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && !m_fInSpecialReload)
	{
		Reload();
	}
	else if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		m_bDelayedFire = FALSE;
		m_fFireOnEmpty = FALSE;

		if ( IsUseable() && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && (!m_iClip || (m_iClip < iMaxClip() && m_pPlayer->m_bAutoReload)) && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
		{
			Reload();
		}
		// If we don't have any ammo, switch to the next best weapon
		if ( !IsUseable() && m_flNextPrimaryAttack < UTIL_WeaponTimeBase() && !(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon( m_pPlayer, this ))
		{
		}
		else
		{
			WeaponIdle();
		}
	}
}

void CBasePlayerItem::DestroyItem(void)
{
	if (m_pPlayer)
		m_pPlayer->RemovePlayerItem(this);

	Kill();
}

int CBasePlayerItem::AddToPlayer(CBasePlayer *pPlayer)
{
	m_pPlayer = pPlayer;
	return TRUE;
}

void CBasePlayerItem::Drop(void)
{
	SetTouch(NULL);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayerItem::Kill(void)
{
	SetTouch(NULL);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayerItem::Holster(int skiplocal)
{
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer *pPlayer)
{
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW;
	pev->modelindex = 0;
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + 0.1;
	SetTouch(NULL);
}

int CBasePlayerWeapon::AddDuplicate(CBasePlayerItem *pOriginal)
{
	if (m_iDefaultAmmo)
		return ExtractAmmo((CBasePlayerWeapon *)pOriginal);
	else
		return ExtractClipAmmo((CBasePlayerWeapon *)pOriginal);
}

int CBasePlayerWeapon::AddToPlayer(CBasePlayer *pPlayer)
{
	int bResult = CBasePlayerItem::AddToPlayer(pPlayer);
	pPlayer->pev->weapons |= (1 << m_iId);

	if (!m_iPrimaryAmmoType)
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo1());
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo2());
	}

	if (bResult)
		bResult = AddWeapon();

	if (bResult)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
	}

	return bResult;
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer *pPlayer)
{
	BOOL bSend = FALSE;
	int state = 0;

	if (pPlayer->m_pActiveItem == this)
	{
		if (pPlayer->m_fOnTarget)
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	if (!pPlayer->m_fWeapon)
		bSend = TRUE;

	if (this == pPlayer->m_pActiveItem || this == pPlayer->m_pClientActiveItem)
	{
		if (pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem)
			bSend = TRUE;
	}

	if (m_iClip != m_iClientClip || state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV)
		bSend = TRUE;

	if (bSend)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev);
		WRITE_BYTE(state);
		WRITE_BYTE(m_iId);
		WRITE_BYTE((m_iClip == -1) ? 0xFF : m_iClip);
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if (m_pNext)
		m_pNext->UpdateClientData(pPlayer);

	return 1;
}

void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int skiplocal)
{
	m_pPlayer->pev->weaponanim = iAnim;

#ifdef CLIENT_WEAPONS
	if (skiplocal && ENGINE_CANSKIP(ENT(m_pPlayer->pev)))
		return;
#endif

	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim);
	WRITE_BYTE(pev->body);
	MESSAGE_END();
}

void CBasePlayerWeapon::SendWeaponAnimEx(int iAnim, int iBody, int iSkin, int skiplocal)
{
	m_pPlayer->pev->weaponanim = iAnim;

#ifdef CLIENT_WEAPONS
	if (skiplocal && ENGINE_CANSKIP(ENT(m_pPlayer->pev)))
		return;
#endif

	MESSAGE_BEGIN(MSG_ONE, gmsgWeaponAnimEx, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim);
	WRITE_BYTE(iBody);
	WRITE_BYTE(iSkin);
	MESSAGE_END();
}

BOOL CBasePlayerWeapon::AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry)
{
	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}
	else if (!m_iClip)
	{
		int i = min(m_iClip + iCount, iMaxClip) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, szName, iMaxCarry);
	}
	else
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;

		if (m_pPlayer->HasPlayerItem(this))
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}

BOOL CBasePlayerWeapon::AddSecondaryAmmo(int iCount, char *szName, int iMax)
{
	int iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMax);

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}

BOOL CBasePlayerWeapon::IsUseable(void)
{
	if (m_iClip <= 0)
		if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1)
			return FALSE;

	return TRUE;
}

BOOL CBasePlayerWeapon::CanDeploy(void)
{
	if(!IsUseable())
		return FALSE;

	return TRUE;
}

BOOL CBasePlayerWeapon::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int body)
{
	if (!CanDeploy())
		return FALSE;

	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	SendWeaponAnim(iAnim, UseDecrement() != FALSE);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.75;
	m_pPlayer->m_iFOV = m_pPlayer->m_iDefaultFOV;//Reset Fov
	m_pPlayer->SetAnimation(PLAYER_IDLE);
	m_pPlayer->ResetMaxSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.75;
	m_bMeleeAttack = FALSE;
	return TRUE;
}

BOOL CBasePlayerWeapon::GroupDeploy(char *szViewModel, char *szWeaponModel, int iViewAnim, int iViewBody, int iViewSkin, char *szAnimExt, int iWpnBody)
{
	if (!CanDeploy())
		return FALSE;
	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	m_pPlayer->pev->scale = iWpnBody;
	strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	if(!iViewBody && !iViewBody)
		SendWeaponAnim(iViewAnim, UseDecrement() != FALSE);
	else
		SendWeaponAnimEx(iViewAnim, iViewBody, iViewSkin, UseDecrement() != FALSE);
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.75;
	m_pPlayer->m_iFOV = m_pPlayer->m_iDefaultFOV;
	m_pPlayer->SetAnimation(PLAYER_IDLE);
	m_pPlayer->ResetMaxSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	m_flDecreaseShotsFired = UTIL_WeaponTimeBase();
	m_bMeleeAttack = FALSE;
	return TRUE;
}

void CBasePlayerWeapon::ReloadSound(void)
{
	CBasePlayer *pPlayer = NULL;

	while ((pPlayer = (CBasePlayer *)UTIL_FindEntityByClassname(pPlayer, "player")))
	{
		if (pPlayer->pev->flags == FL_DORMANT)
			break;

		if (pPlayer == m_pPlayer)
			continue;

		float flDistance = m_pPlayer->pev->origin.Length() - pPlayer->pev->origin.Length();

		if (flDistance <= 512)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgReloadSound, NULL, ENT(m_pPlayer->pev));
			WRITE_BYTE((1.0 - (flDistance / 512.0)) * 255.0);

			const char *classname = STRING(pev->classname);

			if (!strcmp(classname, "weapon_m3") || !strcmp(classname, "weapon_xm1014"))
				WRITE_BYTE(0);
			else
				WRITE_BYTE(1);

			MESSAGE_END();
		}
	}
}

BOOL CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay, int body)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	if(m_fInReload)
		return FALSE;

	int j = min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (!j)
		return FALSE;

	m_flNextReload = UTIL_WeaponTimeBase() + fDelay;
	m_flTimeWeaponIdle = m_flNextReload + 0.5;

	ReloadSound();
	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0);

	m_fInReload = true;
	return TRUE;
}

BOOL CBasePlayerWeapon::PlayEmptySound(void)
{
	#ifdef CLIENT_WEAPONS
		if (UseDecrement() != FALSE && ENGINE_CANSKIP(ENT(m_pPlayer->pev)))
			return FALSE;
	#endif

	if(m_iPlayEmptySound)
	{
		if (m_iPlayEmptySound >= 2)
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dryfire_rifle.wav", 0.8, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dryfire_rifle.wav", 0.8, ATTN_NORM);
		return TRUE;
	}

	return FALSE;
}

void CBasePlayerWeapon::ResetEmptySound(void)
{
	m_iPlayEmptySound = 1;
}

int CBasePlayerWeapon::PrimaryAmmoIndex(void)
{
	return m_iPrimaryAmmoType;
}

int CBasePlayerWeapon::SecondaryAmmoIndex(void)
{
	return -1;
}

void CBasePlayerWeapon::Holster(int skiplocal)
{
	m_fInReload = FALSE;
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerAmmo::Spawn(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(pev, pev->origin);

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 1;
	}
}

int CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	int iReturn = 0;

	if (pszAmmo1())
	{
		iReturn = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1());
		m_iDefaultAmmo = 0;
	}

	if (pszAmmo2())
		iReturn = AddSecondaryAmmo(0, (char *)pszAmmo2(), iMaxAmmo2());

	return iReturn;
}

BOOL CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon *pWeapon)
{
	int iAmmo;

	if (m_iClip == WEAPON_NOCLIP)
		iAmmo = 0;
	else
		iAmmo = m_iClip;

	return pWeapon->m_pPlayer->GiveAmmo(iAmmo, (char *)pszAmmo1(), iMaxAmmo1());
}

void CBasePlayerWeapon::RetireWeapon(void)
{
	m_pPlayer->pev->viewmodel = NULL;
	m_pPlayer->pev->weaponmodel = NULL;
	g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
}

//bool CBasePlayerWeapon::HasAnyAmmo( void )
//{
//	// If I don't use ammo of any kind, I can always fire
//	if ( iMaxClip() <= 0 && iMaxAmmo1() <= 0 )
//		return true;
//
//	// Otherwise, I need ammo of either type
//	return ( (m_iClip > 0) || (iMaxAmmo1() > 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0) );
//}

//default reloaded
void CBasePlayerWeapon::Reloaded(void)
{
	if(m_iPrimaryAmmoType < 0)
		return;

	int j = min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	m_iClip += j;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
	m_fInReload = false;
}

LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox);

void CWeaponBox::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/w_ammo.mdl");
}

void CWeaponBox::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "giveammo"))
	{
		m_flGivePercent = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CWeaponBox::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_NO;
	m_flGivePercent = 50;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	SET_MODEL(ENT(pev), "models/CKF_III/w_ammo.mdl");
}

extern int gmsgBombPickup;

void CWeaponBox::Touch(CBaseEntity *pOther)
{
	if (!(pev->flags & FL_ONGROUND))
		return;

	if (!pOther->IsPlayer())
		return;

	if (!pOther->IsAlive())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if (pPlayer->EatAmmoBox(m_flGivePercent))
	{
		EMIT_SOUND(ENT(pOther->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
		SetTouch(NULL);
		UTIL_Remove(this);
	}
}

void CWeaponBox::SetObjectCollisionBox(void)
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16);
}