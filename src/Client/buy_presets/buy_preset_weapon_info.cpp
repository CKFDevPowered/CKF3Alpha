#include "hud.h"
#include "cdll_dll.h"
#include "ammohistory.h"

#include "buy_preset_debug.h"
#include "buy_presets.h"
#include "weapon_csbase.h"
#include "shared_util.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>

struct WeaponDisplayNameInfo
{
	WeaponIdType id;
	const char *displayName;
};

static WeaponDisplayNameInfo weaponDisplayNameInfo[] =
{
	{ WEAPON_P228, "#Cstrike_TitlesTXT_P228" },
	{ WEAPON_GLOCK, "#Cstrike_TitlesTXT_Glock18" },
	{ WEAPON_SCOUT, "#Cstrike_TitlesTXT_Scout" },
	{ WEAPON_XM1014, "#Cstrike_TitlesTXT_AutoShotgun" },
	{ WEAPON_MAC10, "#Cstrike_TitlesTXT_Mac10_Short" },
	{ WEAPON_AUG, "#Cstrike_TitlesTXT_Aug" },
	{ WEAPON_ELITE, "#Cstrike_TitlesTXT_Beretta96G" },
	{ WEAPON_FIVESEVEN, "#Cstrike_TitlesTXT_ESFiveSeven" },
	{ WEAPON_UMP45, "#Cstrike_TitlesTXT_KMUMP45" },
	{ WEAPON_SG550, "#Cstrike_TitlesTXT_SG550" },
	{ WEAPON_GALIL, "#Cstrike_TitlesTXT_Galil" },
	{ WEAPON_FAMAS, "#Cstrike_TitlesTXT_Famas" },
	{ WEAPON_USP, "#Cstrike_TitlesTXT_USP45" },
	{ WEAPON_AWP, "#Cstrike_TitlesTXT_ArcticWarfareMagnum" },
	{ WEAPON_MP5N, "#Cstrike_TitlesTXT_mp5navy" },
	{ WEAPON_M249, "#Cstrike_TitlesTXT_ESM249" },
	{ WEAPON_M3, "#Cstrike_TitlesTXT_Leone12" },
	{ WEAPON_M4A1, "#Cstrike_TitlesTXT_M4A1_Short" },
	{ WEAPON_TMP, "#Cstrike_TitlesTXT_tmp" },
	{ WEAPON_G3SG1, "#Cstrike_TitlesTXT_G3SG1" },
	{ WEAPON_DEAGLE, "#Cstrike_TitlesTXT_DesertEagle" },
	{ WEAPON_SG552, "#Cstrike_TitlesTXT_SG552" },
	{ WEAPON_AK47, "#Cstrike_TitlesTXT_AK47" },
	{ WEAPON_P90, "#Cstrike_TitlesTXT_ESC90" },
	{ WEAPON_SHIELDGUN, "#Cstrike_TitlesTXT_TactShield" },
	{ WEAPON_NONE, "#Cstrike_CurrentWeapon" }
};

const wchar_t *WeaponIDToDisplayName(WeaponIdType weaponID)
{
	for (int i = 0; weaponDisplayNameInfo[i].displayName; ++i)
	{
		if (weaponDisplayNameInfo[i].id == weaponID)
			return g_pVGuiLocalize->Find(weaponDisplayNameInfo[i].displayName);
	}

	return NULL;
}

BuyPresetWeapon::BuyPresetWeapon(void)
{
	m_name = NULL;
	m_weaponID = WEAPON_NONE;
	m_ammoType = AMMO_CLIPS;
	m_ammoAmount = 0;
	m_fillAmmo = true;
}

BuyPresetWeapon::BuyPresetWeapon(WeaponIdType weaponID)
{
	m_name = WeaponIDToDisplayName(weaponID);
	m_weaponID = weaponID;
	m_ammoType = AMMO_CLIPS;
	m_ammoAmount = (weaponID == WEAPON_NONE) ? 0 : 1;
	m_fillAmmo = true;
}

BuyPresetWeapon &BuyPresetWeapon::operator = (const BuyPresetWeapon &other)
{
	m_name = other.m_name;
	m_weaponID = other.m_weaponID;
	m_ammoType = other.m_ammoType;
	m_ammoAmount = other.m_ammoAmount;
	m_fillAmmo = other.m_fillAmmo;

	return *this;
}

void BuyPresetWeapon::SetWeaponID(WeaponIdType weaponID)
{
	m_name = WeaponIDToDisplayName(weaponID);
	m_weaponID = weaponID;
}

bool CanBuyWeapon(WeaponIdType currentPrimaryID, WeaponIdType currentSecondaryID, WeaponIdType weaponID)
{
	if (currentPrimaryID == WEAPON_SHIELDGUN && weaponID == WEAPON_ELITE)
		return false;

	if (currentSecondaryID == WEAPON_ELITE && weaponID == WEAPON_SHIELDGUN)
		return false;

	return true;
}

static bool IsPrimaryWeaponID(WeaponIdType id)
{
	WEAPON *p = gWR.GetWeapon(id);

	if (p->iSlot == 0)
		return true;

	return false;
}

static bool IsSecondaryWeaponID(WeaponIdType id)
{
	WEAPON *p = gWR.GetWeapon(id);

	if (p->iSlot == 1)
		return true;

	return false;
}