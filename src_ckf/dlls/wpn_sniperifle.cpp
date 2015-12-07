#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum sniperifle_e
{
	SNIPERIFLE_IDLE1,
	SNIPERIFLE_IDLE2,
	SNIPERIFLE_DRAW,
	SNIPERIFLE_FIRE
};

LINK_ENTITY_TO_CLASS(weapon_sniperifle, CSniperifle);

void CSniperifle::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sniperifle");

	Precache();
	m_iId = WEAPON_SNIPERIFLE;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 20;
	m_iDefaultAmmo = SNIPERIFLE_DEFAULT_GIVE;
	m_pSpot = m_pDSpot = NULL;
	FallInit();
}

void CSniperifle::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_sniperifle.mdl");
	PRECACHE_SOUND("CKF_III/sniperifle_shoot.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_sniperifle.sc");
}

int CSniperifle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _SNIPER_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SNIPERIFLE;
	p->iFlags = 0;
	p->iWeight = SNIPERIFLE_WEIGHT;
	return 1;
}

BOOL CSniperifle::Deploy(void)
{
	m_fCharge = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fSpotActive = 0;

	return GroupDeploy("models/CKF_III/v_sniperifle.mdl", "models/CKF_III/wp_group_rf.mdl", SNIPERIFLE_DRAW, 0, 0, "shotgun", 57);
}

BOOL CSniperifle::CanHolster(void)
{
	if(m_iResumeZoom == 1)
		return FALSE;
	return TRUE;
}

void CSniperifle::Holster( int skiplocal)
{
	m_pPlayer->m_iFOV = m_pPlayer->m_iDefaultFOV;

	m_iResumeZoom = 0;
	m_fCharge = 0;
	m_fSpotActive = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;

	UpdateSpot();
	m_pPlayer->ResetMaxSpeed();
}

void CSniperifle::PrimaryAttack(void)
{
	SniperifleFire();
}

void CSniperifle::StartZoom(void)
{
	m_pPlayer->m_iFOV = 32 * m_pPlayer->m_iDefaultFOV / 90;
	m_fChargeTimer = UTIL_WeaponTimeBase() + 4.0;
	m_fCharge = 0;
	
	m_iWeaponState |= WEAPONSTATE_CHARGING;

	m_fSpotActive = 1;
	UpdateSpot();

	m_pPlayer->ResetMaxSpeed();

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 1, 0, 0);
	//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "CKF_III/zoom.wav", 0.8, 2.4);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_iResumeZoom = 0;
}

void CSniperifle::StopZoom(void)
{
	m_pPlayer->m_iFOV = m_pPlayer->m_iDefaultFOV;
	m_fCharge = 0;

	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fSpotActive = 0;
	UpdateSpot();

	m_pPlayer->ResetMaxSpeed();

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 1, 0, 0);
	//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "CKF_III/zoom.wav", 0.8, 2.4);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_iResumeZoom = 0;
}

void CSniperifle::SecondaryAttack(void)
{
	if(m_iResumeZoom)
		return;

	if ( !(m_iWeaponState & WEAPONSTATE_CHARGING) && (m_pPlayer->pev->flags & FL_ONGROUND) )
	{
		StartZoom();
	}
	else if ( (m_iWeaponState & WEAPONSTATE_CHARGING) )
	{
		StopZoom();
	}
}

void CSniperifle::SniperifleFire(void)
{
	int iCrit = m_pPlayer->GetCriticalFire(-1, m_pPlayer->random_seed);

	if (m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		m_iResumeZoom = 1;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.35;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, 0.0, 8192, (m_fCharge > 10) ? BULLET_PLAYER_SNIPER : BULLET_PLAYER_SNIPER_NOHS, (m_fCharge > 25) ? int((1+2.0f*m_fCharge/100.0f)*50.0f) : 50, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, 0, 0, iCrit, 0, 0, 0);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_fSpotActive = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	UpdateSpot();

	SendWeaponAnim(SNIPERIFLE_FIRE, UseDecrement() != FALSE);
}

void CSniperifle::ItemPostFrame(void)
{
	if (m_iResumeZoom && m_flNextReload <= UTIL_WeaponTimeBase())
	{
		if (m_iResumeZoom == 1)
		{
			m_pPlayer->m_iFOV = m_pPlayer->m_iDefaultFOV;
			m_fCharge = 0;
			m_flNextReload = m_flNextPrimaryAttack;
			m_pPlayer->ResetMaxSpeed();
			m_iResumeZoom = 2;
			m_iWeaponState &= ~WEAPONSTATE_CHARGING;
		}
		else if (m_iResumeZoom == 2)
		{
			StartZoom();
		}
	}

	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		if((m_pPlayer->pev->oldbuttons & IN_JUMP) && (m_pPlayer->pev->fuser2 > 1300.0f))
		{
			StopZoom();
		}
		else
		{
			if (m_fSpotActive && m_pSpot && m_pDSpot)//Set spot
			{
				UTIL_MakeVectors(m_pPlayer->pev->v_angle);
				Vector vecSrc = m_pPlayer->GetGunPosition();
				Vector vecAiming = gpGlobals->v_forward;

				TraceResult tr;
				UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
				UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
				UTIL_SetOrigin( m_pDSpot->pev, tr.vecEndPos );
				m_pSpot->pev->renderamt = min(1.5*m_fCharge+150, 255);
				m_pSpot->pev->scale = 0.05+0.05*m_fCharge/100;
			}
			if(m_fChargeTimer > UTIL_WeaponTimeBase())
			{
				m_fCharge = (4.0 + UTIL_WeaponTimeBase() - m_fChargeTimer) / 4.0 * 100;;
			}
			else
			{
				m_fCharge = 100;
			}
		}
	}
	CBasePlayerWeapon::ItemPostFrame();
}

void CSniperifle::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(SNIPERIFLE_IDLE1, UseDecrement() != FALSE);
	}
}

float CSniperifle::GetMaxSpeed(void)
{
	if (!(m_iWeaponState & WEAPONSTATE_CHARGING))
		return 1;

	return 0.27;
}

LINK_ENTITY_TO_CLASS( laser_spot, CLaserSpot );

CLaserSpot *CLaserSpot::CreateSpot(CBaseEntity * pOwner)
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();
	pSpot->pev->owner = pOwner->edict();
	return pSpot;
}

void CLaserSpot::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), "sprites/CKF_III/sniperdot.spr");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	pev->rendermode = kRenderGlow;//kRenderTransAdd;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 149;
	pev->rendercolor.x = pev->rendercolor.y = pev->rendercolor.z = 0;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->scale = 0.1;

	pev->classname = MAKE_STRING("laser_spot");
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
}

void CLaserSpot::Precache(void)
{
	PRECACHE_MODEL("sprites/CKF_III/sniperdot.spr");
}

void CSniperifle::UpdateSpot(void)
{
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot((CBaseEntity *)m_pPlayer);
			(m_pPlayer->m_iTeam == 1) ? m_pSpot->pev->rendercolor.x = 255 : m_pSpot->pev->rendercolor.z = 255;
		}
		if (!m_pDSpot)
		{
			m_pDSpot = CLaserSpot::CreateSpot((CBaseEntity *)m_pPlayer);
			(m_pPlayer->m_iTeam == 1) ? m_pDSpot->pev->rendercolor.x = 255 : m_pDSpot->pev->rendercolor.z = 255;
		}
	}
	else
	{
		if (m_pSpot)
		{
			m_pSpot->Killed( NULL, GIB_NEVER );
			m_pSpot = NULL;
		}
		if (m_pDSpot)
		{
			m_pDSpot->Killed( NULL, GIB_NEVER );
			m_pDSpot = NULL;
		}
	}
}