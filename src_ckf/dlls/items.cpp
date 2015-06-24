#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;
extern int gmsgArmorType;
extern int gmsgStatusIcon;

void CItem::Spawn(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->sequence = 0;
	pev->framerate = 1.0f;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		UTIL_Remove(this);
		return;
	}
}

extern int gEvilImpulse101;

void CItem::ItemTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if (!g_pGameRules->CanHaveItem(pPlayer, this))
		return;

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		SetTouch(NULL);
		g_pGameRules->PlayerGotItem(pPlayer, this);

		if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES)
			Respawn();
		else
			UTIL_Remove(this);
	}
	else if (gEvilImpulse101)
		UTIL_Remove(this);
}

CBaseEntity *CItem::Respawn(void)
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin(pev, g_pGameRules->VecItemRespawnSpot(this));
	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->FlItemRespawnTime(this);
	return this;
}

void CItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}

void CItem::Restart(void)
{
	SetThink(&CItem::Materialize);
	pev->nextthink = gpGlobals->time + 1.0;
}

class CItemAmmoBox : public CItem
{
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);
	BOOL MyTouch(CBasePlayer *pPlayer);
public:
	float m_flGivePercent;
};

LINK_ENTITY_TO_CLASS(item_ammobox, CItemAmmoBox);

void CItemAmmoBox::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "giveammo"))
	{
		m_flGivePercent = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "type"))
	{
		pev->body = min(2,max(0, atoi(pkvd->szValue))) * 3;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CItemAmmoBox::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/CKF_III/w_ammo.mdl");
	CItem::Spawn();
	if(m_flGivePercent == 0)
	{
		if(pev->body == 6)
			m_flGivePercent = 100;
		else if(pev->body == 3)
			m_flGivePercent = 50;
		else
			m_flGivePercent = 20.5;
	}
}

void CItemAmmoBox::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/w_ammo.mdl");
}

BOOL CItemAmmoBox::MyTouch(CBasePlayer *pPlayer)
{
	if (!(pev->flags & FL_ONGROUND))
		return FALSE;

	if (!pPlayer->IsAlive())
		return FALSE;

	if (pPlayer->EatAmmoBox(m_flGivePercent))
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
		return TRUE;
	}
	return FALSE;
}

class CItemHealthBox : public CItem
{
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);
	BOOL MyTouch(CBasePlayer *pPlayer);
public:
	float m_flGivePercent;
};

LINK_ENTITY_TO_CLASS(item_healthbox, CItemHealthBox);

void CItemHealthBox::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "givehealth"))
	{
		m_flGivePercent = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "type"))
	{
		pev->body = min(2,max(0, atoi(pkvd->szValue))) * 3;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CItemHealthBox::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/CKF_III/w_health.mdl");
	CItem::Spawn();
	if(m_flGivePercent == 0)
	{
		if(pev->body == 6)
			m_flGivePercent = 100;
		else if(pev->body == 3)
			m_flGivePercent = 50;
		else
			m_flGivePercent = 20.5;
	}
}

void CItemHealthBox::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/w_health.mdl");
}

BOOL CItemHealthBox::MyTouch(CBasePlayer *pPlayer)
{
	if (!(pev->flags & FL_ONGROUND))
		return FALSE;

	if (!pPlayer->IsAlive())
		return FALSE;

	int iAteShit = 0;
	int iShit = 0;
	
	//Add Spy disguise health here...
	if(pPlayer->m_iDisguise == DISGUISE_YES)
	{
		if(pPlayer->m_iDisguiseHealth < pPlayer->m_iDisguiseMaxHealth)
		{
			int iOldShit = pPlayer->m_iDisguiseHealth;
			pPlayer->m_iDisguiseHealth = min((pPlayer->m_iDisguiseHealth + pPlayer->m_iDisguiseMaxHealth * m_flGivePercent / 100.0f), pPlayer->m_iDisguiseMaxHealth);
			iAteShit ++;
		}
	}

	if(pPlayer->pev->health < pPlayer->pev->max_health)
	{
		int iOldShit = pPlayer->pev->health;
		pPlayer->pev->health = min((pPlayer->pev->health + pPlayer->pev->max_health * m_flGivePercent / 100.0f), pPlayer->pev->max_health);
		iShit = pPlayer->pev->health - iOldShit;
		iAteShit ++;
	}

	if (iAteShit > 0)
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
		if(iShit)//Really ate shit?
		{
			pPlayer->SendAddHealth(iShit);
			pPlayer->m_Cond.AfterBurn.Remove();
		}
		return TRUE;
	}
	return FALSE;
}