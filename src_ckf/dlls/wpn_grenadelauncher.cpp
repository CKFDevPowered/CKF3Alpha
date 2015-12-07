#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "buildable.h"

#define CHAN_WEAPON	1

enum grenadelauncher_e
{
	GRENADELAUNCHER_IDLE,
	GRENADELAUNCHER_START_RELOAD,
	GRENADELAUNCHER_RELOAD,
	GRENADELAUNCHER_AFTER_RELOAD,
	GRENADELAUNCHER_DRAW,
	GRENADELAUNCHER_SHOOT	
};

LINK_ENTITY_TO_CLASS(weapon_grenadelauncher, CGrenadeLauncher);

void CGrenadeLauncher::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_grenadelauncher");

	Precache();
	m_iId = WEAPON_GRENADELAUNCHER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_2bone.mdl");
	pev->sequence = 0;
	m_iDefaultAmmo = GRENADE_DEFAULT_GIVE;
	FallInit();
}

void CGrenadeLauncher::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_grenadelauncher.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_2bone.mdl");
	PRECACHE_SOUND("CKF_III/grenadelauncher_shoot.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

int CGrenadeLauncher::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _GRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GRENADE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_GRENADELAUNCHER;
	p->iFlags = 0;
	p->iWeight = GRENADELAUNCHER_WEIGHT;

	return 1;
}

BOOL CGrenadeLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;
	return GroupDeploy("models/CKF_III/v_grenadelauncher.mdl", "models/CKF_III/wp_group_2bone.mdl", GRENADELAUNCHER_DRAW, 0, 0, "shotgun", 0);
}

void CGrenadeLauncher::PrimaryAttack(void)
{
	GrenadeLauncherFire();
}

void CGrenadeLauncher::SecondaryAttack(void)
{
	CBasePlayerItem *pWeapon = m_pPlayer->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY];
	if(pWeapon && pWeapon->m_iId == WEAPON_STICKYLAUNCHER)
	{
		CStickyLauncher *pLauncher = (CStickyLauncher *)pWeapon;
		if( pLauncher->m_flNextSecondaryAttack < UTIL_WeaponTimeBase())
			pLauncher->SecondaryAttack();
	}
}

void CGrenadeLauncher::GrenadeLauncherFire()
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

	CDemoGrenade *pGrenade = CDemoGrenade::CreateDemoGrenade( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);

	pGrenade->pev->velocity = gpGlobals->v_forward * 1065 + gpGlobals->v_up * 150;

	pGrenade->m_iCrit = iCrit;
	pGrenade->m_iTeam = m_pPlayer->m_iTeam;
	pGrenade->pev->frame = (iCrit>=2) ? 1 : 0;
	pGrenade->pev->skin = m_pPlayer->m_iTeam-1;
	pGrenade->m_iPjFlags = PJ_AIRBLAST_DEFLECTABLE | PJ_AIRBLAST_UPWARD | PJ_AIRBLAST_OWNER | PJ_AIRBLAST_ROTATE;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_GRENADETRAIL);
	WRITE_SHORT(ENTINDEX( pGrenade->edict() ));
	MESSAGE_END();

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = false;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(GRENADELAUNCHER_SHOOT, UseDecrement() != FALSE);
}

void CGrenadeLauncher::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == GRENADE_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		SendWeaponAnim(GRENADELAUNCHER_START_RELOAD, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;

		m_flNextReload = UTIL_WeaponTimeBase() + 0.64;
		m_fInSpecialReload = 1;
	}
}

void CGrenadeLauncher::Reloaded(void)
{
	if (m_iClip == GRENADE_MAX_CLIP || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)//out of ammo or full of clip, stop reloading
	{
		SendWeaponAnim(GRENADELAUNCHER_AFTER_RELOAD, UseDecrement() != FALSE);
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

		SendWeaponAnim(GRENADELAUNCHER_RELOAD, UseDecrement() != FALSE);
		m_flNextReload = UTIL_WeaponTimeBase() + 0.60;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	}
}

void CGrenadeLauncher::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(GRENADELAUNCHER_IDLE, UseDecrement() != FALSE);
	}
}

LINK_ENTITY_TO_CLASS( pj_grenade, CDemoGrenade );

CDemoGrenade *CDemoGrenade::CreateDemoGrenade( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CDemoGrenade *pGrenade = GetClassPtr( (CDemoGrenade *)NULL );

	UTIL_SetOrigin( pGrenade->pev, vecOrigin );
	vecAngles.x *= -1;
	pGrenade->pev->angles = vecAngles;

	pGrenade->Spawn();
	pGrenade->SetTouch(&CDemoGrenade::GrenadeTouch);
	pGrenade->pev->owner = pOwner->edict();

	CBasePlayer *pPlayer = (CBasePlayer *)pOwner;

	return pGrenade;
}

void CDemoGrenade::Spawn( void )
{
	Precache();
	// motor
	SET_MODEL(ENT(pev), "models/CKF_III/pj_grenade.mdl");
	UTIL_SetSize(pev, Vector(-3,-3,-3), Vector(3,3,3));
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->avelocity = Vector(RANDOM_FLOAT(0, 720), RANDOM_FLOAT(0, 720), RANDOM_FLOAT(0, 720));
	pev->takedamage = DAMAGE_YES;
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("pj_grenade");
	//fix for cs16nd
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetThink(&CDemoGrenade::GrenadeThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetTouch(&CDemoGrenade::GrenadeTouch);
	pev->dmgtime = gpGlobals->time + 2.28;
	pev->team = -1;
	pev->health = 999999;

	pev->gravity = 1.0;

	m_fDmg = 120;
	m_fDmgRadius = 128;
	m_fForce = 450;
	m_fForceRadius = 128;
	m_bFall = FALSE;
	m_bRoll = FALSE;
	m_iJumpTimes = 0;
	m_iPjFlags = 0;	
	m_pDeflecter = NULL;
}

void CDemoGrenade::GrenadeTouch(CBaseEntity *pOther)
{
	if(pOther->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pOther;
		if(pPlayer->m_iTeam == m_iTeam) return;
		if(m_bFall && pev->velocity.Length() < 80) return;
		pev->enemy = pOther->edict();

		TraceResult tr;
		Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
		UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);
		CKFExplode(&tr, DMG_BLAST|DMG_ALWAYSGIB);
	}
	else if(pOther->Classify() == CLASS_BUILDABLE)
	{
		CBaseBuildable *pBuild = (CBaseBuildable *)pOther;
		if(pBuild->m_iTeam == m_iTeam) return;
		if(m_bFall && pev->velocity.Length() < 80) return;
		pev->enemy = pOther->edict();

		TraceResult tr;
		Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
		UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);
		CKFExplode(&tr, DMG_BLAST|DMG_ALWAYSGIB);
	}
	else
	{
		if(m_bRoll)
		{
			pev->velocity = pev->movedir;
			return;
		}
		else if(pev->velocity.Length() <= 10 && pev->flags & FL_ONGROUND)
		{
			pev->velocity = Vector(RANDOM_FLOAT(-32, 32), RANDOM_FLOAT(-32, 32), 32);
		}
		pev->velocity = pev->velocity * 0.5;

		TraceResult tr;
		Vector vecSpot = pev->origin;
		if(pev->velocity.Length() > 10)
			UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 128, ignore_monsters, ENT(pev), &tr);
		else
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -30), ignore_monsters, ENT(pev), &tr);
		if(tr.flFraction != 1)
		{
			//物理修正开始
			BOOL bFall = FALSE;
			float flAngle = M_PI/2;
			if(tr.vecPlaneNormal.z)//判断是否落到角度小于60度的斜面（平面上）
			{
				Vector vecSB = Vector(0, 0, 1);
				flAngle = acos( DotProduct(vecSB, tr.vecPlaneNormal) / ( tr.vecPlaneNormal.Length() ) );
				if(flAngle < M_PI/3) { m_bFall = bFall = TRUE; }
			}

			if(bFall && pev->velocity.Length() < 128)
			{
				UTIL_MakeVectors(pev->angles);
				Vector vecAngles = gpGlobals->v_forward;

				float flAnglesDiff = asin( fabs(DotProduct(tr.vecPlaneNormal, vecAngles))/(vecAngles.Length()) );

				if(( flAnglesDiff < M_PI/4 && !m_bRoll ) || m_iJumpTimes > 3)//与平面之间的角度差小于45度
				{
					vecAngles = ProjectionProduct(tr.vecPlaneNormal, vecAngles);//暴力计算平面内的投影向量
					pev->angles = UTIL_VecToAngles(vecAngles);//水平放置
					pev->velocity = g_vecZero;
					pev->avelocity = g_vecZero;
					float flRollSpeed = 32;
					if(tr.vecPlaneNormal.Length2D() > 0)
					{
						Vector vecMaxRoll = ProjectionProduct(tr.vecPlaneNormal, Vector(0, 0, 1));
						float flCosRollAngles = fabs( DotProduct(vecMaxRoll, vecAngles) )/(vecMaxRoll.Length()*vecAngles.Length()) ;//榴弹朝向d与最大速度滚动方向的夹角cos值
						flRollSpeed = max(512 * (1-flCosRollAngles) * flAngle/(M_PI/2), 32);
					}
					pev->velocity = CrossProduct(tr.vecPlaneNormal, vecAngles).Normalize() * flRollSpeed;//叉乘计算角度指向向量与面法向量的法向量用作滚动方向
					if(pev->velocity.z > 0) pev->velocity = -pev->velocity;//往高处滚动，不科学
					
					pev->movedir = pev->velocity;
					m_bRoll = TRUE;
				}
				else
				{
					pev->velocity = pev->velocity * 1.5;					
					pev->avelocity = pev->avelocity * 1.5;
					m_iJumpTimes ++;
				}
			}
			//物理修正结束
		}
	}
}

void CDemoGrenade::Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce)
{
	CBasePlayer *pPlayer = NULL;
	if(pAttacker->IsPlayer())
		pPlayer = (CBasePlayer *)pAttacker;

	if(!m_bFall)
	{
		pev->velocity = vecDirShooting.Normalize() * (pev->velocity.Length());
	}
	else
	{
		vecDirShooting = vecDirShooting * flForce * 2;
		vecDirShooting.z = max(150, vecDirShooting.z);
		pev->velocity = vecDirShooting;
		m_bFall = FALSE;
		m_bRoll = FALSE;
		m_fDmg *= 2;
	}
	pev->avelocity = Vector(RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720), RANDOM_FLOAT(-720, 720));

	pev->owner = pAttacker->edict();
	if(pPlayer)
	{
		m_iTeam = pPlayer->m_iTeam;
		pev->skin = m_iTeam - 1;
	}
	m_iPjFlags |= PJ_AIRBLAST_DEFLECTED;

	if(!m_iCrit) m_iCrit ++;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_GRENADETRAIL);
	WRITE_SHORT(ENTINDEX( ENT(pev) ));
	MESSAGE_END();

	SetThink(&CDemoGrenade::GrenadeThink);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->dmgtime = gpGlobals->time + 2.28;
	pev->team = -1;

	//pev->classname = MAKE_STRING("pj_defgrenade");
}

void CDemoGrenade::GrenadeThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if(pev->dmgtime - gpGlobals->time > 0.20f) pev->nextthink = gpGlobals->time + 0.1;
	else pev->nextthink = gpGlobals->time + 0.01;

	pev->team ++;
	BOOL bDrawFX = FALSE;
	if(pev->team <= 16)
	{
		if(pev->team % 4 == 0)
			bDrawFX = TRUE;
	}
	else
	{
		if(pev->team == 18 || pev->team == 20 || pev->team == 25 || pev->team == 30)
			bDrawFX = TRUE;
	}

	if(bDrawFX)
	{
		if(pev->skin < 2)
			pev->skin = pev->skin+2;
		else
			pev->skin = pev->skin-2;
	}

	if(pev->dmgtime < gpGlobals->time)
	{
		if(m_bFall) m_fDmg *= 0.5;
		SetThink(&CGrenade::CKFDetonate);
		return;
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}

void CDemoGrenade::Killed(entvars_t *pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->owner = NULL;
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CDemoGrenade::CKFDetonate(void)
{
	TraceResult tr;
	Vector vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	CKFExplode(&tr, DMG_BLAST|DMG_ALWAYSGIB|DMG_SHAKE);
}

void CDemoGrenade::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/pj_grenade.mdl");
}