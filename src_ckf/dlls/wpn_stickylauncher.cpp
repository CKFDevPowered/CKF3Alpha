#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum stickylauncher_e
{
	STICKYLAUNCHER_IDLE,
	STICKYLAUNCHER_START_RELOAD,
	STICKYLAUNCHER_RELOAD,
	STICKYLAUNCHER_AFTER_RELOAD,
	STICKYLAUNCHER_DRAW,
	STICKYLAUNCHER_SHOOT	
};

LINK_ENTITY_TO_CLASS(weapon_stickylauncher, CStickyLauncher);

void CStickyLauncher::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_stickylauncher");

	Precache();
	m_iId = WEAPON_STICKYLAUNCHER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 6;
	m_iDefaultAmmo = STICKY_DEFAULT_GIVE;
	m_iMaxDeploy = 8;
	m_iStickyNum = 0;
	m_fCharge = 0;
	m_fChargeTimer = 0;
	FallInit();
}

void CStickyLauncher::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_stickylauncher.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/stickylauncher_shoot.wav");
	PRECACHE_SOUND("CKF_III/stickylauncher_charge.wav");
	PRECACHE_SOUND("CKF_III/stickylauncher_det.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

int CStickyLauncher::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SecondaryAmmo";
	p->iMaxAmmo1 = _STICKY_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = STICKY_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 10;
	p->iId = m_iId = WEAPON_STICKYLAUNCHER;
	p->iFlags = 0;
	p->iWeight = STICKYLAUNCHER_WEIGHT;

	return 1;
}

BOOL CStickyLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;

	return GroupDeploy("models/CKF_III/v_stickylauncher.mdl", "models/CKF_III/wp_group_rf.mdl", STICKYLAUNCHER_DRAW, 0, 0, "onehanded", 24);
}

void CStickyLauncher::Holster(int skiplocal)
{
	StopCharge();
}

void CStickyLauncher::StopCharge(void)
{
	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 1, 0, 0);
	}
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fCharge = 0;
}

void CStickyLauncher::PrimaryAttack(void)
{
	if(m_iClip && !(m_iWeaponState & WEAPONSTATE_CHARGING))
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 2, 0, 0);
		m_fChargeTimer = UTIL_WeaponTimeBase() + 4.0;
		m_fCharge = 0;
		m_iWeaponState |= WEAPONSTATE_CHARGING;
	}
}

void CStickyLauncher::ItemPostFrame(void)
{
	if(!m_iClip && (m_iWeaponState & WEAPONSTATE_CHARGING) )
	{
		StopCharge();
	}
	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		if(m_pPlayer->pev->button & IN_ATTACK)
		{
			m_fCharge = min(max((UTIL_WeaponTimeBase() + 4.0 - m_fChargeTimer) * (100.0 / 4.0), 0), 100);
		}
		if(!(m_pPlayer->pev->button & IN_ATTACK))
		{
			StickyLauncherFire();
		}
		if(m_fCharge >= 100)
		{
			StickyLauncherFire();
		}
	}

	CBasePlayerWeapon::ItemPostFrame();
}

int CStickyLauncher::RecountSticky(void)
{
	int iCount = 0;

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "pj_sticky")) != NULL)
	{
		if(pEntity->pev->owner == m_pPlayer->edict())
			iCount ++;
	}
	m_iStickyNum = iCount;
	return iCount;
}

void CStickyLauncher::StickyFucker(void)
{
	CBaseEntity *pEntity = NULL;
	CSticky *pSticky = NULL;
	CSticky *pFind = NULL;
	float flMostEarly = 0;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "pj_sticky")) != NULL)
	{
		if(pEntity->pev->owner == m_pPlayer->edict())
		{
			pSticky = (CSticky *)pEntity;
			if(!flMostEarly || pSticky->m_fDeploy < flMostEarly)
			{
				flMostEarly = ((CSticky *)pEntity)->m_fDeploy;
				pFind = pSticky;
			}
		}
	}

	if(pFind)
	{
		if(pFind->m_iPjFlags & PJ_AIRBLAST_DEFLECTED)
		{
			pFind->CKFDetonate();
		}
		else
		{
			pFind->SetThink(&CSticky::CKFDetonate);
			pFind->pev->nextthink = gpGlobals->time + 0.1;
		}

		EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/stickylauncher_det.wav", 1.0, 1.0);
	}
}

void CStickyLauncher::SecondaryAttack(void)
{
	int iDetonate = 0;

	CBaseEntity *pEntity = NULL;
	CSticky *pSticky = NULL;

	if(!m_iStickyNum)
		return;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "pj_sticky")) != NULL)
	{
		if(pEntity->pev->owner != m_pPlayer->edict())
			continue;
		pSticky = (CSticky *)pEntity;
		if(pSticky->m_bDetonating || pSticky->m_fDeploy > gpGlobals->time)
			continue;

		pSticky->m_bDetonating = true;
		pSticky->SetThink(&CSticky::CKFDetonate);
		pSticky->pev->nextthink = gpGlobals->time + 0.125f;
		iDetonate ++;
	}

	if(iDetonate)
	{
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/stickylauncher_det.wav", 1.0, 1.0);
		m_iStickyNum -= iDetonate;
	}
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4f;
}

void CStickyLauncher::StickyLauncherFire(void)
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

	//Launch Function
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

	CSticky *pSticky = CSticky::CreatePjSticky( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);
	pSticky->m_iTeam = m_pPlayer->m_iTeam;
	pSticky->m_iCrit = iCrit;
	pSticky->m_fDeploy = gpGlobals->time + 0.6;
	pSticky->m_fGCharge = m_fCharge;
	pSticky->pev->frame = (iCrit>=2) ? 1 : 0;
	pSticky->pev->skin = m_pPlayer->m_iTeam-1;
	if(iCrit >= 2) pSticky->pev->skin += 2;
	pSticky->m_iPjFlags = PJ_AIRBLAST_DEFLECTABLE | PJ_AIRBLAST_UPWARD | PJ_AIRBLAST_ROTATE | PJ_AIRBLAST_TEMPOWNER;

	MESSAGE_BEGIN(MSG_PVS, gmsgDrawFX, pSticky->Center());
	WRITE_BYTE(FX_STICKYTRAIL);
	WRITE_SHORT(ENTINDEX( pSticky->edict() ));
	MESSAGE_END();

	float fVel = 805 * (1+1.3*pSticky->m_fGCharge/100.0f);
	pSticky->pev->velocity = gpGlobals->v_forward * fVel + gpGlobals->v_up * 100;

	if(m_iStickyNum >= m_iMaxDeploy)
	{
		StickyFucker();
	}
	else
	{
		m_iStickyNum ++;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = 0;

	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fCharge = 0;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(STICKYLAUNCHER_SHOOT, UseDecrement() != FALSE);
}

void CStickyLauncher::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == STICKY_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if((m_iWeaponState & WEAPONSTATE_CHARGING))
		return;

	if (!m_fInSpecialReload)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		SendWeaponAnim(STICKYLAUNCHER_START_RELOAD, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;

		m_fInSpecialReload = 1;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.42;
	}
}

void CStickyLauncher::Reloaded(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == STICKY_MAX_CLIP)//out of ammo or full of clip, stop reloading
	{
		SendWeaponAnim(STICKYLAUNCHER_AFTER_RELOAD, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		m_fInSpecialReload = 0;
	}
	else if (m_fInSpecialReload == 2)
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_fInSpecialReload = 1;//go back to start stage
		Reloaded();//have the next try now so weapon anim will be played immediately 
	}
	else
	{
		m_fInSpecialReload = 2;//reloading stage

		SendWeaponAnim(STICKYLAUNCHER_RELOAD, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.67;		
	}
}

void CStickyLauncher::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		SendWeaponAnim(STICKYLAUNCHER_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}

LINK_ENTITY_TO_CLASS( pj_sticky, CSticky );

CSticky *CSticky::CreatePjSticky( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CSticky *pSticky = GetClassPtr( (CSticky *)NULL );

	UTIL_SetOrigin( pSticky->pev, vecOrigin );
	vecAngles.x *= -1;
	pSticky->pev->angles = vecAngles;

	pSticky->Spawn();
	pSticky->SetTouch(&CSticky::StickyTouch);
	pSticky->pev->owner = pOwner->edict();

	return pSticky;
}

void CSticky::Spawn(void)
{
	Precache();
	// motor
	SET_MODEL(ENT(pev), "models/CKF_III/pj_stickybomb.mdl");
	UTIL_SetSize(pev, Vector(-2,-2,-2), Vector(2,2,2));
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;

	UTIL_SetOrigin( pev, pev->origin );
	
	pev->avelocity = Vector(RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720));
	pev->takedamage = DAMAGE_YES;
	pev->health = 50;
	
	pev->classname = MAKE_STRING("pj_sticky");
	//fix for cs16nd
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetTouch(&CSticky::StickyTouch);
	SetThink(&CSticky::StickyThink);
	pev->nextthink = gpGlobals->time + 0.90f;

	pev->gravity = 1.0f;	

	m_fDmg = 120;
	m_fDmgRadius = 128;
	m_fForce = 600;
	m_fForceRadius = 128;
	m_iPjFlags = 0;
	m_pDeflecter = NULL;
}

void CSticky::StickyThink(void)
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}
}

void CSticky::StickyTouch( CBaseEntity *pOther )
{
	if(pev->movetype == MOVETYPE_NONE)
		return;

	if(pOther->IsPlayer())
		return;

	if(pev->velocity.Length() > 10)
	{
		if(pOther->Classify() == CLASS_MOVEABLE || pOther->Classify() == CLASS_PLAYER || pOther->Classify() == CLASS_PROJECTILE)
			return;
		if(pOther->pev->movetype == MOVETYPE_PUSH && pOther->pev->team != 0)
			return;
	}

	TraceResult tr;
	Vector vecSpot = pev->origin + pev->velocity*gpGlobals->frametime;
	UTIL_TraceLine(pev->origin, vecSpot, ignore_monsters, ENT(pev), &tr);

	if(tr.flFraction == 1) return;

	float flDistance = (tr.vecEndPos - pev->origin).Length();

	float flSin = DotProduct(tr.vecPlaneNormal, pev->velocity)/(pev->velocity.Length());

	if(flDistance*flSin < 3.4)
	{
		pev->origin = tr.vecEndPos + tr.vecPlaneNormal*3.4;
	}
	
	pev->enemy = pOther->edict();
	pev->movetype = MOVETYPE_NONE;
	pev->velocity = g_vecZero;
}

void CSticky::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/pj_stickybomb.mdl");
}

void CSticky::Killed(entvars_t *pevAttacker, int iGib)
{
	MESSAGE_BEGIN(MSG_PVS, gmsgDrawFX, pev->origin);
	WRITE_BYTE(FX_STICKYKILL);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(pev->skin);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_KILLTRAIL);
	WRITE_SHORT(ENTINDEX(edict()));
	MESSAGE_END();

	pev->takedamage = DAMAGE_NO;

	if(pev->owner)
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
		if(pOwner && pOwner->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pOwner;
			CBasePlayerWeapon *pSecondary = (CBasePlayerWeapon *)pPlayer->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY];
			if(pSecondary && pSecondary->m_iId == WEAPON_STICKYLAUNCHER)
			{
				CStickyLauncher *pLauncher = (CStickyLauncher *)pSecondary;
				pLauncher->m_iStickyNum --;
			}
		}
	}

	pev->owner = NULL;

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CSticky::Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce)
{
	CBasePlayer *pPlayer = NULL;
	if(pAttacker->IsPlayer()) pPlayer = (CBasePlayer *)pAttacker;

	if(pev->movetype != MOVETYPE_NONE)
	{
		pev->velocity = vecDirShooting.Normalize() * (pev->velocity.Length());
	}
	else
	{
		pev->movetype = MOVETYPE_TOSS;
		vecDirShooting = vecDirShooting * flForce * 2;
		vecDirShooting.z = max(150, vecDirShooting.z);
		pev->velocity = vecDirShooting;
	}
	pev->avelocity = Vector(RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720));

	if(pPlayer)
	{
		m_pDeflecter = pPlayer;
	}

	m_iPjFlags |= PJ_AIRBLAST_DEFLECTED;

	SetThink(&CSticky::CKFDeflectReset);
	pev->nextthink = gpGlobals->time + 1.5;
}

void CSticky::CKFDeflectReset(void)
{
	m_iPjFlags &= ~PJ_AIRBLAST_DEFLECTED;
	m_pDeflecter = NULL;

	/*MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_STICKYTRAIL);
	WRITE_SHORT(ENTINDEX( edict() ));
	MESSAGE_END();*/
}

void CSticky::CKFDetonate(void)
{
	TraceResult tr;
	Vector vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), ignore_monsters, ENT(pev), &tr);
	if(tr.flFraction == 1)
		UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, 16), ignore_monsters, ENT(pev), &tr);
	if(tr.flFraction == 1)
		UTIL_TraceLine(vecSpot, vecSpot + Vector(16, 0, 0), ignore_monsters, ENT(pev), &tr);
	if(tr.flFraction == 1)
		UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 16, 0), ignore_monsters, ENT(pev), &tr);
	if(tr.flFraction == 1)
		UTIL_TraceLine(vecSpot, vecSpot + Vector(-16, 0, 0), ignore_monsters, ENT(pev), &tr);
	if(tr.flFraction == 1)
		UTIL_TraceLine(vecSpot, vecSpot + Vector(0, -16, 0), ignore_monsters, ENT(pev), &tr);
	CKFExplode(&tr, (gpGlobals->time - m_fDeploy >= 5.0f) ? DMG_BLAST | DMG_ALWAYSGIB | DMG_SHAKE : DMG_BLAST | DMG_ALWAYSGIB | DMG_RANGE | DMG_SHAKE);
}