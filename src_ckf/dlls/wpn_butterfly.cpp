#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define	KNIFE_BODYHIT_VOLUME 128
#define	KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_butterfly, CButterfly);

enum butterfly_e
{
	BUTTERFLY_IDLE,
	BUTTERFLY_HANDUP,
	BUTTERFLY_HANDIDLE,
	BUTTERFLY_DRAW,
	BUTTERFLY_IDLE2,
	BUTTERFLY_IDLE3,
	BUTTERFLY_SLASH1,
	BUTTERFLY_SLASH2,	
	BUTTERFLY_HANDDOWN,
	BUTTERFLY_STAB
};

void CButterfly::Spawn(void)
{
	Precache();
	m_iId = WEAPON_BUTTERFLY;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 14;
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CButterfly::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_butterfly.mdl");
	PRECACHE_SOUND("CKF_III/butterfly_swing.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

int CButterfly::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_BUTTERFLY;
	p->iFlags = 0;
	p->iWeight = BUTTERFLY_WEIGHT;

	return 1;
}

BOOL CButterfly::Deploy(void)
{
	m_iShotsFired = 0;

	return GroupDeploy("models/CKF_III/v_butterfly.mdl", "models/CKF_III/wp_group_rf.mdl", BUTTERFLY_DRAW, 0, 0, "knife", 42);
}

void CButterfly::Holster(int skiplocal)
{
	m_bMeleeAttack = FALSE;
}

void CButterfly::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.05;
	m_iMeleeCrit = m_pPlayer->GetCriticalFire(-1, m_pPlayer->random_seed);
	m_bMeleeAttack = TRUE;

	SendWeaponAnim(BUTTERFLY_SLASH1 + ((m_iShotsFired++) % 2), UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;

	m_pPlayer->Disguise_Stop();
}

void CButterfly::SecondaryAttack(void)
{
	if(m_pPlayer->m_iCloak == CLOAK_NO)
		m_pPlayer->Cloak_Begin();
	else if(m_pPlayer->m_iCloak == CLOAK_YES)
		m_pPlayer->Cloak_Stop();
}

BOOL IsBackFace(Vector &anglesAttacker, Vector &anglesVictim)
{
	float flAngles = anglesAttacker.y - anglesVictim.y;
	if(flAngles < -180.0) flAngles += 360.0;
	if(flAngles <= 90.0 && flAngles >= -90.0) return TRUE;
	return FALSE;
}

void CButterfly::Swing(void)
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_iMeleeCrit, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		float flVol = 1;
		int fHitWorld = TRUE;

		if(pEntity)
		{
			m_iMeleeCrit += pEntity->GetCriticalHit();

			float fDmg = 40;

			if(m_iMeleeCrit >= 2) fDmg *= 3;
			else if(m_iMeleeCrit) fDmg *= 1.35f;
			else fDmg += RANDOM_LONG(-12, 14);

			ClearMultiDamage();

			BOOL bBackStab = FALSE;

			if(pEntity->IsPlayer())
			{
				CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
				if( pPlayer->m_iTeam == 3-m_pPlayer->m_iTeam)
				{
					if(IsBackFace(m_pPlayer->pev->angles, pPlayer->pev->angles))
					{
						bBackStab = TRUE;
						fDmg = pPlayer->pev->health * 6;					
						m_iMeleeCrit += 2;
					}
					else
						pPlayer->m_bLastHitBackStab = TRUE;
				}
			}
			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, (int)fDmg, gpGlobals->v_forward, &tr, (bBackStab) ? DMG_BACKSTAB | DMG_NEVERGIB | DMG_SLASH : DMG_NEVERGIB | DMG_SLASH, m_iMeleeCrit);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MOVEABLE)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_STATIC, "CKF_III/kukri_hit.wav", VOL_NORM, ATTN_NORM);

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return;

				flVol = 0.1;
				fHitWorld = FALSE;
			}
		}

		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, "CKF_III/kukri_hit.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}

		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
		DecalGunshot(&tr, BULLET_PLAYER_CROWBAR, FALSE, m_pPlayer->pev, FALSE);
	}

	return;
}