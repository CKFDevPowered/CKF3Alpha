#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

enum medigun_e
{
	MEDIGUN_IDLE1,
	MEDIGUN_DRAW,
	MEDIGUN_HEAL,
	MEDIGUN_UBER
};

LINK_ENTITY_TO_CLASS(weapon_medigun, CMedigun);

void CMedigun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_medigun");

	Precache();
	m_iId = WEAPON_MEDIGUN;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 10;

	m_iDefaultAmmo = -1;
	m_pTarget = NULL;
	m_fNextHeal = 0;
	m_fNextBeam = 0;
	m_fNextCheck = 0;
	m_bFullyCharged = false;
	m_bHealing = false;
	FallInit();
}

void CMedigun::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_medigun.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/medigun_no_target.wav");
	PRECACHE_SOUND("CKF_III/medigun_heal.wav");
	PRECACHE_SOUND("CKF_III/medigun_charged.wav");
	PRECACHE_SOUND("CKF_III/invulnerable_on.wav");
	PRECACHE_SOUND("CKF_III/invulnerable_off.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_medigun.sc");
}

int CMedigun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_MEDIGUN;
	p->iFlags = 0;
	p->iWeight = MEDIGUN_WEIGHT;
	return 1;
}

BOOL CMedigun::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_medigun.mdl", "models/CKF_III/wp_group_rf.mdl", MEDIGUN_DRAW, 0, 0, "shotgun", 30);
}

void CMedigun::Holster(int skiplocal)
{
	StopHeal();
}

void CMedigun::PrimaryAttack(void)
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	CBaseEntity *pEntity = NULL;
	TraceResult tr;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecSpot = vecSrc + gpGlobals->v_forward * 320;
	UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, edict(), &tr);

	if(tr.pHit)
	{
		pEntity = CBaseEntity::Instance(tr.pHit);
	}

	if(pEntity && pEntity->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
		if((pPlayer->m_iTeam == m_pPlayer->m_iTeam || (pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_pPlayer->m_iTeam)) && pPlayer->m_iCloak == CLOAK_NO && pPlayer != m_pTarget)
		{
			StopHeal();

			m_pTarget = pPlayer;
			m_bHealing = TRUE;
			m_fNextHeal = gpGlobals->time + 0.1;
			m_fNextBeam = gpGlobals->time;

			EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/medigun_heal.wav", VOL_NORM, ATTN_NORM);
		}
	}

	if(!m_bDelayedFire)
	{
		SendWeaponAnim(MEDIGUN_HEAL, UseDecrement() != FALSE);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 0, 0, FALSE, FALSE);
		m_bDelayedFire = TRUE;
	}
}

void CMedigun::SecondaryAttack(void)
{
	if(m_pPlayer->m_iUbercharge)
		return;
	if(m_pPlayer->m_fUbercharge < 100)
		return;

	m_pPlayer->m_iUbercharge |= UC_INVULNERABLE;
	m_bFullyCharged = FALSE;

	m_pPlayer->AddPoints(1, TRUE);
	m_pPlayer->m_Stats.iUbercharge ++;
	m_pPlayer->SendStatsInfo(STATS_UBERCHARGE);

	SendWeaponAnim(MEDIGUN_UBER);
	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/invulnerable_on.wav", 1.0, ATTN_NORM);
}

void CMedigun::MedigunHeal(void)
{
	//Effect
	if(m_pPlayer->m_iUbercharge & UC_INVULNERABLE)
	{
		m_pPlayer->Invulnerable_Add(1.0f, (m_pPlayer->m_fUbercharge < 25) ? true : false);
		m_pTarget->Invulnerable_Add(1.0f, (m_pPlayer->m_fUbercharge < 25) ? true : false);
	}

	int iHealerCount = m_pTarget->m_iHealerCount;
	/*for(int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer || !FNullEnt(pPlayer->pev))
			continue;
		if(pPlayer->m_iClass != CLASS_MEDIC)
			continue;
		if(!pPlayer->m_pActiveItem)
			continue;
		if(pPlayer->m_pActiveItem->iItemSlot() != WEAPON_SLOT_SECONDARY)
			continue;
		CMedigun *gun = (CMedigun *)pPlayer->m_pActiveItem;
		if(gun->m_pTarget == m_pTarget && gun->m_bHealing)
		{
			iHealerCount ++;
		}
	}*/

	float flHeal = 0;
	float flCharge = 0;
	float flDamageTime = gpGlobals->time - m_pTarget->m_flLastDamagedTime;
	if(flDamageTime < 10.0f)
		flHeal = 2.4;
	else if(flDamageTime < 15.0f)
		flHeal = flDamageTime*0.96+2.4;
	else
		flHeal = 7.2;

	if(!m_pPlayer->m_iUbercharge)
	{
		if(m_pTarget->pev->health / m_pTarget->pev->max_health > 1.425 && g_pGameRules->IsRoundSetup() )
			flCharge = 0.125;
		else
			flCharge = 0.25;
		flCharge /= max(iHealerCount, 1);
	}
	if(m_pTarget->m_iDisguise == DISGUISE_YES)
	{
		if(m_pTarget->m_iDisguiseHealth < m_pTarget->m_iDisguiseMaxHealth * 1.5)
		{
			if(m_pTarget->m_iDisguiseHealth + flHeal > m_pTarget->m_iDisguiseMaxHealth * 1.5)
				m_pTarget->m_iDisguiseHealth = m_pTarget->m_iDisguiseMaxHealth * 1.5;
			else
				m_pTarget->m_iDisguiseHealth += flHeal;
		}
	}
	float fLBoostMaxHealth = m_pTarget->GetBoostMaxHealth();
	if(m_pTarget->pev->health < fLBoostMaxHealth )
	{
		if(m_pTarget->pev->health + flHeal > fLBoostMaxHealth )
			flHeal = fLBoostMaxHealth - m_pTarget->pev->health;

		m_pTarget->pev->health += flHeal;
		m_pPlayer->m_flHealAmount += flHeal;
	}
	if(m_pPlayer->m_fUbercharge < 100)
	{
		if(m_pPlayer->m_fUbercharge + flCharge > 100)
			m_pPlayer->m_fUbercharge = 100;
		else
			m_pPlayer->m_fUbercharge += flCharge;
	}
	if(m_pPlayer->m_fUbercharge >= 100 && !m_bFullyCharged && !m_pPlayer->m_iUbercharge)
	{
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/medigun_charged.wav", 1.0, 0.3);
		SendWeaponAnim(MEDIGUN_UBER);
		m_bFullyCharged = TRUE;

		//Msg Call : I am fully charged!
	}

	m_fNextHeal = gpGlobals->time + 0.1;
}

bool CMedigun::PreHeal(void)
{
	if(!m_pTarget)
		return false;
	if(!m_pPlayer->IsAlive() || !m_pTarget->IsAlive())
		return false;
	if(!m_pPlayer->PlayerCanAttack())
		return false;
	if(m_pPlayer->m_iTeam != m_pTarget->m_iTeam && m_pTarget->m_iDisguise != DISGUISE_YES)
		return false;
	if(m_pTarget->m_iCloak != CLOAK_NO)
		return false;

	if(m_fNextCheck < gpGlobals->time)
	{
		m_fNextCheck = gpGlobals->time + 1.0;
		float flDistance = (m_pPlayer->Center() - m_pTarget->Center()).Length();
		if(flDistance > 400)
			return false;
		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->GetGunPosition(), m_pTarget->Center(), ignore_monsters, edict(), &tr);
		if(tr.flFraction != 1)
			return false;
	}
	return true;
}

void CMedigun::StopHeal(void)
{
	if(m_pTarget)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_KILLMEDIBEAM);
		WRITE_SHORT(ENTINDEX( m_pPlayer->edict() ) );
		WRITE_SHORT(ENTINDEX( m_pTarget->edict() ));
		MESSAGE_END();
	}

	m_bHealing = false;
	m_pTarget = NULL;
}

void CMedigun::MedigunBeam(void)
{
	if(m_fNextBeam > gpGlobals->time)
		return;
	m_fNextBeam = gpGlobals->time + 1.0f;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_MEDIBEAM);
	WRITE_SHORT(ENTINDEX( m_pPlayer->edict() ));
	WRITE_SHORT(ENTINDEX( m_pTarget->edict() ));
	WRITE_BYTE(m_pPlayer->m_iTeam-1);
	MESSAGE_END();
}

void CMedigun::ItemPostFrame(void)
{
	if(m_bHealing && m_fNextHeal < gpGlobals->time)
	{
		if(PreHeal())
		{
			MedigunHeal();
			MedigunBeam();
		}
		else
		{
			StopHeal();
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CMedigun::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(MEDIGUN_IDLE1, UseDecrement() != FALSE);
}