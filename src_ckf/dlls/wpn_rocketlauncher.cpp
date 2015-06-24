#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "buildable.h"

#define CHAN_WEAPON	1

enum rocketlauncher_e
{
	ROCKETLAUNCHER_IDLE,
	ROCKETLAUNCHER_START_RELOAD,
	ROCKETLAUNCHER_RELOAD,
	ROCKETLAUNCHER_AFTER_RELOAD,
	ROCKETLAUNCHER_DRAW,
	ROCKETLAUNCHER_SHOOT	
};

LINK_ENTITY_TO_CLASS(weapon_rocketlauncher, CRocketLauncher);

void CRocketLauncher::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_rocketlauncher");

	Precache();
	m_iId = WEAPON_ROCKETLAUNCHER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 3;

	m_iDefaultAmmo = ROCKET_DEFAULT_GIVE;
	FallInit();
}

void CRocketLauncher::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_rocketlauncher.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/rocketlauncher_shoot.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

int CRocketLauncher::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _ROCKET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = ROCKET_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 10;
	p->iId = m_iId = WEAPON_ROCKETLAUNCHER;
	p->iFlags = 0;
	p->iWeight = ROCKETLAUNCHER_WEIGHT;

	return 1;
}

BOOL CRocketLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;
	
	return GroupDeploy("models/CKF_III/v_rocketlauncher.mdl", "models/CKF_III/wp_group_rf.mdl", ROCKETLAUNCHER_DRAW, 0, 0, "shotgun", 6);
}

void CRocketLauncher::PrimaryAttack(void)
{
	RocketLauncherFire();
}

void CRocketLauncher::RocketLauncherFire()
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(0, m_pPlayer->random_seed);

	m_iClip--;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	SendWeaponAnim(ROCKETLAUNCHER_SHOOT, UseDecrement() != FALSE);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -2;

	CRocket *pRocket = CRocket::CreatePjRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);

	pRocket->m_iCrit = iCrit;
	pRocket->m_iTeam = m_pPlayer->m_iTeam;
	pRocket->pev->frame = (iCrit>=2) ? 1 : 0;
	pRocket->pev->skin = m_pPlayer->m_iTeam-1;
	pRocket->m_iPjFlags = PJ_AIRBLAST_DEFLECTABLE | PJ_AIRBLAST_OWNER;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_ROCKETTRAIL);
	WRITE_SHORT(ENTINDEX( pRocket->edict() ));
	MESSAGE_END();

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	//EMIT_SOUND(ENT(pev) , CHAN_WEAPON, "CKF_III/rocketlauncher_shoot.wav", 1.0, 0.80);
	//if(iCrit >= 2) EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/crit_shot.wav", VOL_NORM, ATTN_NORM);

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;

	//if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	//	m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = 0;
}

void CRocketLauncher::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == ROCKET_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		SendWeaponAnim(ROCKETLAUNCHER_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.50;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.50;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		SendWeaponAnim(ROCKETLAUNCHER_RELOAD, UseDecrement() != FALSE);
		m_pPlayer->SetAnimation(PLAYER_RELOAD);//anim fix

		m_flNextReload = UTIL_WeaponTimeBase() + 0.70;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.70;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
	}
}

void CRocketLauncher::WeaponIdle(void)
{
	ResetEmptySound();
	//m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_iClip && !m_fInSpecialReload && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload)
		{
			if (m_iClip == ROCKET_MAX_CLIP || !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				SendWeaponAnim(ROCKETLAUNCHER_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(ROCKETLAUNCHER_IDLE, UseDecrement() != FALSE);
	}
}

LINK_ENTITY_TO_CLASS(pj_rocket, CRocket);

CRocket *CRocket::CreatePjRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner)
{
	CRocket *pRocket = GetClassPtr( (CRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	vecAngles.x *= -1;
	pRocket->pev->angles = vecAngles;

	pRocket->Spawn();
	pRocket->SetTouch(&CRocket::RocketTouch);
	pRocket->pev->owner = pOwner->edict();

	TraceResult tr;
	UTIL_TraceLine(vecOrigin, vecOrigin + gpGlobals->v_forward * 8192, dont_ignore_monsters, pOwner->edict(), &tr);
	float flDistance = (tr.vecEndPos - vecOrigin).Length();
	if(flDistance > 880)
	{
		pRocket->pev->velocity = gpGlobals->v_forward * 50;
		pRocket->SetThink(&CRocket::RocketLaunch);
		pRocket->pev->nextthink = gpGlobals->time + 0.15;
	}
	else if(flDistance > 220)
	{
		pRocket->pev->velocity = gpGlobals->v_forward * 50;
		pRocket->SetThink(&CRocket::RocketLaunch);
		pRocket->pev->nextthink = gpGlobals->time + 0.05+0.10*(flDistance-220)/880;
	}
	else
	{
		pRocket->pev->velocity = gpGlobals->v_forward * 50;
		pRocket->SetThink(&CRocket::RocketLaunch);
		pRocket->pev->nextthink = gpGlobals->time + 0.05;
	}

	return pRocket;
}

void CRocket::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/CKF_III/pj_rocket.mdl");
	UTIL_SetSize(pev, Vector(-2,-2,-2), Vector(2,2,2));	
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_TOSS;	
	pev->takedamage = DAMAGE_AIM;
	pev->gravity = 0.000001;
	pev->health = 80;

	pev->classname = MAKE_STRING("pj_rocket");

	SetTouch(&CRocket::RocketTouch);

	m_fDmg = 90;
	m_fDmgRadius = 128;
	m_fForce = 450;
	m_fForceRadius = 128;
	m_iPjFlags = 0;
	m_pDeflecter = NULL;
}

void CRocket::RocketTouch(CBaseEntity *pOther)
{
	if(pOther->IsPlayer())
	{
		if(pOther->edict() == pev->owner)
			return;
		if(((CBasePlayer *)pOther)->m_iTeam == m_iTeam)
			return;
	}
	if(pOther)
		pev->enemy = pOther->edict();

	TraceResult tr;
	Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);

	CKFExplode(&tr, DMG_BLAST|DMG_ALWAYSGIB|DMG_RANGE);
}

void CRocket::Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce)
{
	CBasePlayer *pPlayer = NULL;
	if(pAttacker->IsPlayer()) pPlayer = (CBasePlayer *)pAttacker;
	pev->velocity = vecDirShooting.Normalize() * (pev->velocity.Length());
	pev->owner = pAttacker->edict();
	if(pPlayer) m_iTeam = pPlayer->m_iTeam;
	pev->skin = m_iTeam - 1;
	m_iPjFlags |= PJ_AIRBLAST_DEFLECTED;
	if(!m_iCrit) m_iCrit ++;
	pev->classname = MAKE_STRING("pj_defrocket");
}

void CRocket::RocketLaunch(void)
{
	pev->velocity = pev->velocity.Normalize() * 1100;
	pev->solid = SOLID_BBOX;
}

void CRocket::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/pj_rocket.mdl");
	PRECACHE_SOUND("CKF_III/explode.wav");
}

void CRocket::Killed(entvars_t *pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;

	TraceResult tr;
	Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);

	CKFExplode(&tr, DMG_BLAST|DMG_ALWAYSGIB|DMG_RANGE);
}