#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define	KNIFE_BODYHIT_VOLUME 128
#define	KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_shovel, CShovel);

enum shovel_e
{
	SHOVEL_IDLE,
	SHOVEL_IDLE1,
	SHOVEL_IDLE2,
	SHOVEL_DRAW,
	SHOVEL_IDLE3,
	SHOVEL_IDLE4,
	SHOVEL_SLASH1,
	SHOVEL_SLASH2
};

void CShovel::Spawn(void)
{
	Precache();
	m_iId = WEAPON_SHOVEL;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 0;

	m_iClip = WEAPON_NOCLIP;

	FallInit();
}

void CShovel::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_shovel.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/cbar_hit1.wav");
	PRECACHE_SOUND("CKF_III/cbar_hit2.wav");
	PRECACHE_SOUND("CKF_III/cbar_hitbod.wav");
	PRECACHE_SOUND("CKF_III/draw_shovel.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

int CShovel::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_SHOVEL;
	p->iFlags = 0;
	p->iWeight = SHOVEL_WEIGHT;

	return 1;
}

BOOL CShovel::Deploy(void)
{
	m_iShotsFired = 0;

	return GroupDeploy("models/CKF_III/v_shovel.mdl", "models/CKF_III/wp_group_rf.mdl", SHOVEL_DRAW, 0, 0, "knife", 0);
}

void CShovel::Holster(int skiplocal)
{
	m_bMeleeAttack = FALSE;
}

void CShovel::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.3;
	m_iMeleeCrit = m_pPlayer->GetCriticalFire(2, m_pPlayer->random_seed);
	m_bMeleeAttack = TRUE;
	SendWeaponAnim(SHOVEL_SLASH1 + (m_iShotsFired++) % 2, UseDecrement() != FALSE);
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CShovel::Swing(void)
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
			m_iMeleeCrit += m_pPlayer->GetCriticalHit();

			float fDmg = 65;

			if(m_iMeleeCrit >= 2) fDmg *= 3;
			else if(m_iMeleeCrit) fDmg *= 1.35f;
			else fDmg += RANDOM_LONG(-6, 7);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, fDmg, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_SLASH, m_iMeleeCrit);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MOVEABLE)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "CKF_III/cbar_hitbod.wav", VOL_NORM, ATTN_NORM);

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
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0, 1) ? "CKF_III/cbar_hit1.wav" : "CKF_III/cbar_hit2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}

		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
		DecalGunshot(&tr, BULLET_PLAYER_CROWBAR, FALSE, m_pPlayer->pev, FALSE);
	}

	return;
}