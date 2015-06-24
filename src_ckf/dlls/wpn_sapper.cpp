#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum sapper_e
{
	SAPPER_IDLE1,
	SAPPER_DRAW,
	SAPPER_DRAW2
};

LINK_ENTITY_TO_CLASS(weapon_sapper, CSapper);

void CSapper::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sapper");

	Precache();
	m_iId = WEAPON_SAPPER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 17;
	m_iDefaultAmmo = -1;
	FallInit();
}

void CSapper::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_sapper.mdl");
	PRECACHE_SOUND("CKF_III/sapper_plant.wav");
}

int CSapper::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SAPPER;
	p->iFlags = 0;
	p->iWeight = SAPPER_WEIGHT;
	return 1;
}

BOOL CSapper::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_sapper.mdl", "models/CKF_III/wp_group_rf.mdl", (m_pPlayer->m_iCloak == CLOAK_NO) ? SAPPER_DRAW : SAPPER_DRAW2, 0, 0, "onehanded", 51);
}

void CSapper::PrimaryAttack(void)
{
	if(m_bDelayedFire)
		return;

	m_bDelayedFire = TRUE;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 8 + gpGlobals->v_right * 4 + gpGlobals->v_up * -2;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + gpGlobals->v_forward * 80, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if(!tr.pHit)
		return;

	CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

	if(pEntity->Classify() != CLASS_BUILDABLE)
		return;

	CBaseBuildable *pBuild = (CBaseBuildable *)pEntity;

	if(!pBuild->m_pPlayer)//Killed
		return;

	if(pBuild->m_iTeam == m_pPlayer->m_iTeam)
		return;

	if(pBuild->m_pSapper)
		return;

	CBuildSapper::CreateBuildSapper(pBuild, m_pPlayer);
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "CKF_III/sapper_plant.wav", VOL_NORM, ATTN_NORM);
}

void CSapper::SecondaryAttack(void)
{
	if(m_pPlayer->m_iCloak == CLOAK_NO)
		m_pPlayer->Cloak_Begin();
	else if(m_pPlayer->m_iCloak == CLOAK_YES)
		m_pPlayer->Cloak_Stop();
}

void CSapper::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(SAPPER_IDLE1, UseDecrement() != FALSE);
}

//PjSapper

LINK_ENTITY_TO_CLASS(pj_sapper, CBuildSapper);

CBuildSapper *CBuildSapper::CreateBuildSapper(CBaseBuildable *pBuild, CBaseEntity *pOwner)
{
	CBuildSapper *pSapper = GetClassPtr( (CBuildSapper *)NULL );

	UTIL_SetOrigin( pSapper->pev, pBuild->pev->origin );

	//pSapper->pev->angles = pBuild->pev->angles;

	pSapper->Spawn();

	pSapper->pev->aiment = pBuild->edict();
	pSapper->pev->owner = pOwner->edict();
	pSapper->m_pBuild = pBuild;
	pSapper->pev->sequence = pBuild->GetSapperSequence();
	pBuild->m_pSapper = pSapper;
	pBuild->m_iFlags |= BUILD_SAPPERED;
	pBuild->pev->framerate = 0;
	pBuild->Sapped();

	pSapper->SetThink(&CBuildSapper::SapperThink);
	pSapper->pev->nextthink = gpGlobals->time + 0.1;

	return pSapper;
}

void CBuildSapper::Spawn( void )
{
	Precache();

	SET_MODEL(ENT(pev), "models/CKF_III/w_sapper.mdl");
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_AIM;
	pev->body = 0; 

	pev->max_health = pev->health = 100;

	pev->classname = MAKE_STRING("pj_sapper");

	m_fDmg = 12.5;
	m_iPjFlags = 0;
	m_pDeflecter = NULL;
}

void CBuildSapper::SapperThink(void)
{
	if(!m_pBuild)
		return;

	if (!pev->owner)
		return;

	entvars_t *pevOwner = NULL;
	pevOwner = VARS(pev->owner);

	m_pBuild->TakeDamage(pev, pevOwner, m_fDmg, DMG_SAPPER, 0);

	if(pev->dmgtime <= gpGlobals->time)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "CKF_III/sapper_timer.wav", 0.5, ATTN_NORM);
		pev->dmgtime = gpGlobals->time + 2;
	}

	pev->nextthink = gpGlobals->time + 0.5;
}

void CBuildSapper::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/w_sapper.mdl");
	PRECACHE_SOUND("CKF_III/sapper_removed.wav");
	PRECACHE_SOUND("CKF_III/sapper_timer.wav");
}

void CBuildSapper::Killed(entvars_t *pevAttacker, int iGib)
{
	if(m_pBuild)
	{
		m_pBuild->pev->framerate = 1;
		m_pBuild->m_pSapper = NULL;
		m_pBuild->m_iFlags &= ~BUILD_SAPPERED;
		m_pBuild->UpdateHUD();
	}
	m_pBuild = NULL;
	pev->owner = NULL;
	pev->effects |= EF_NODRAW;

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "CKF_III/null.wav", 0.5, ATTN_NORM);
	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/sapper_removed.wav", VOL_NORM, ATTN_NORM);

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.05;
}

int CBuildSapper::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

	if(pInflictor)
	{
		if(pInflictor->Classify() == CLASS_PROJECTILE)
			return 0;
	}

	if(!(bitsDamageType & DMG_SLASH))
		return 0;

	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);
		return 0;
	}

	return 1;
}