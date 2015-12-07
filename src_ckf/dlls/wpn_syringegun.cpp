#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define CHAN_WEAPON	1

enum syringegun_e
{
	SYRINGEGUN_IDLE,
	SYRINGEGUN_RELOAD,
	SYRINGEGUN_DRAW,
	SYRINGEGUN_SHOOT
};

LINK_ENTITY_TO_CLASS(weapon_syringegun, CSyringeGun);

void CSyringeGun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_syringegun");

	Precache();
	m_iId = WEAPON_SYRINGEGUN;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 9;
	m_iDefaultAmmo = SYRINGEGUN_DEFAULT_GIVE;
	FallInit();
}

void CSyringeGun::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_syringegun.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/syringegun_shoot.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_syringegun.sc");
}

int CSyringeGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _SYRINGE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SYRINGE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_SYRINGEGUN;
	p->iFlags = 0;
	p->iWeight = SYRINGEGUN_WEIGHT;

	return 1;
}

BOOL CSyringeGun::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_syringegun.mdl", "models/CKF_III/wp_group_rf.mdl", SYRINGEGUN_DRAW, 0, 0, "onehanded", 27);
}

void CSyringeGun::PrimaryAttack(void)
{
	SyringeGunFire();
}

void CSyringeGun::SyringeGunFire(void)
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

	int iCrit = m_pPlayer->GetCriticalFire(1, m_pPlayer->random_seed);

	m_iClip--;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	SendWeaponAnim(SYRINGEGUN_SHOOT, UseDecrement() != FALSE);

	//Launch Function
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

	CSyringe *pSyringe = CSyringe::CreatePjSyringe( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);

	pSyringe->pev->velocity = gpGlobals->v_forward * 990 + gpGlobals->v_up * RANDOM_FLOAT(-10, 10) + gpGlobals->v_right * RANDOM_FLOAT(-10, 10);

	pSyringe->m_iCrit = iCrit;
	pSyringe->m_iTeam = m_pPlayer->m_iTeam;
	if(iCrit >= 2)
	{
		pSyringe->pev->rendermode = kRenderNormal;
		pSyringe->pev->renderfx = kRenderFxGlowShell;
		pSyringe->pev->renderamt = 5;
		pSyringe->pev->rendercolor.x = (2-m_pPlayer->m_iTeam) * 255;
		pSyringe->pev->rendercolor.y = 0;
		pSyringe->pev->rendercolor.z = (m_pPlayer->m_iTeam-1) * 255;
	}
	pSyringe->pev->skin = m_pPlayer->m_iTeam-1;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInReload = false;
}

void CSyringeGun::Reload(void)
{
	if (DefaultReload(SYRINGE_MAX_CLIP, SYRINGEGUN_RELOAD, 1.6))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
	}
}

void CSyringeGun::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(SYRINGEGUN_IDLE, UseDecrement() != FALSE);
	}
}

LINK_ENTITY_TO_CLASS( pj_syringe, CSyringe );

CSyringe *CSyringe::CreatePjSyringe( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CSyringe *pSyringe = GetClassPtr( (CSyringe *)NULL );

	UTIL_SetOrigin( pSyringe->pev, vecOrigin );
	vecAngles.x *= -1;
	pSyringe->pev->angles = vecAngles;

	pSyringe->Spawn();
	pSyringe->SetTouch(&CSyringe::SyringeTouch);
	pSyringe->pev->owner = pOwner->edict();

	CBasePlayer *pPlayer = (CBasePlayer *)pOwner;

	return pSyringe;
}

void CSyringe::Spawn( void )
{
	Precache();
	// motor
	SET_MODEL(ENT(pev), "models/CKF_III/pj_syringe.mdl");
	UTIL_SetSize(pev, Vector(-1,-1,-1), Vector(1,1,1));
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_NO;
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("pj_syringe");
	//fix for cs16nd
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetTouch(&CSyringe::SyringeTouch);
	SetThink(&CSyringe::SyringeThink);
	pev->nextthink = gpGlobals->time + 0.1;

	pev->health = 99999;

	pev->gravity = 0.5;

	m_fDmg = 10;
	m_fDmgRadius = 0;
	m_fForce = 0;
	m_fForceRadius = 0;
	m_iPjFlags = 0;
	m_pDeflecter = NULL;
}

void CSyringe::SyringeThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}
	pev->nextthink = gpGlobals->time + 0.1;
	Vector vecSpot = pev->origin + pev->velocity.Normalize()*32;
	UTIL_TraceLine(pev->origin, vecSpot, dont_ignore_monsters, ENT(pev), &m_iTrace);
	if(m_iTrace.flFraction < 1.0)
	{
		if(m_iTrace.pHit)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(m_iTrace.pHit);
			SyringeTouch(pEntity);
		}
	}	
}

void CSyringe::SyringeTouch(CBaseEntity *pOther)
{
	if(!pOther) return;
	if(pOther->IsBSPModel())
	{
		SetTouch(NULL);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->movetype = MOVETYPE_NONE;
		pev->nextthink = gpGlobals->time + 3;
		return;
	}
	if(pOther->pev->takedamage < DAMAGE_YES) return;
	if(!pev->owner) return;
	CBasePlayer *pAttacker = (CBasePlayer *)CBaseEntity::Instance(pev->owner);
	if(!pAttacker) return;

	if(pOther->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pOther;
		if(!pPlayer) return;

		if(pPlayer->m_iTeam != 3-m_iTeam) return;
		int iCrit = m_iCrit + pOther->GetCriticalHit();
		
		float fDmg = m_fDmg;

		float flDamageModifier = 1.0;

		float flDistance = (pPlayer->Center() - pAttacker->Center()).Length();

		if(flDistance < DIST_MEDIUM) flDamageModifier = -0.2*flDistance/DIST_MEDIUM+1.2;
		else if(flDistance < DIST_LONG) flDamageModifier = sin(M_PI/2*flDistance/DIST_LONG + M_PI/2) + 0.5;
		else flDamageModifier = 0.5;
		if(m_iCrit == 1 && flDistance >= DIST_MEDIUM) flDamageModifier = 1.0;

		if(m_iCrit >= 2) flDamageModifier = 3;
		else if(m_iCrit) flDamageModifier *= 1.35f;
		else fDmg += RANDOM_LONG(-1, 2);

		fDmg *= flDamageModifier;

		ClearMultiDamage();

		pPlayer->TraceAttack(pAttacker->pev, fDmg, gpGlobals->v_forward, &m_iTrace, DMG_NEVERGIB | DMG_BULLET, iCrit);

		ApplyMultiDamage(pev, pAttacker->pev);
	}
	UTIL_Remove(this);
}

void CSyringe::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/pj_syringe.mdl");
}