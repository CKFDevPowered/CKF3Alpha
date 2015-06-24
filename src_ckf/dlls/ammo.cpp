#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

class CSecondaryAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(ammo_secondary, CSecondaryAmmo);

void CSecondaryAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSecondaryAmmo::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

class CPrimaryAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(ammo_primary, CPrimaryAmmo);

void CPrimaryAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");//Valve SB
	CBasePlayerAmmo::Spawn();
}

void CPrimaryAmmo::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}