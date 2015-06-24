#include "hud.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "ammohistory.h"

#include <cl_entity.h>

#include "buy_preset_debug.h"
#include "buy_presets.h"
#include "weapon_csbase.h"
#include "shared_util.h"
#include "vgui/ILocalize.h"
#include <vgui_controls/Controls.h>

static bool CanBuyDefuser(void)
{
	return (g_iTeamNumber == TEAM_CT && g_pViewPort->IsBombDefuseMap());
}

void BuyPresetManager::GetCurrentLoadout(WeaponSet *weaponSet)
{
	if (!weaponSet)
		return;

	WEAPON *smokeGrenade = gWR.GetWeapon(WEAPON_SMOKEGRENADE);
	WEAPON *HEGrenade = gWR.GetWeapon(WEAPON_HEGRENADE);
	WEAPON *flashBang = gWR.GetWeapon(WEAPON_FLASHBANG);
	WEAPON *primaryWeapon = gWR.GetFirstPos(0);
	WEAPON *secondaryWeapon = gWR.GetFirstPos(1);
	WEAPON *knifeWeapon = gWR.GetFirstPos(2);

	weaponSet->Reset();

	if (smokeGrenade)
		weaponSet->m_smokeGrenade = gWR.CountAmmo(smokeGrenade->iAmmoType);

	if (HEGrenade)
		weaponSet->m_HEGrenade = gWR.CountAmmo(HEGrenade->iAmmoType);

	if (flashBang)
		weaponSet->m_flashbangs = gWR.CountAmmo(flashBang->iAmmoType);

	if (primaryWeapon)
	{
		weaponSet->m_primaryWeapon.SetWeaponID((WeaponIdType)primaryWeapon->iId);
		weaponSet->m_primaryWeapon.SetAmmoType(AMMO_CLIPS);

		if (gWR.CountAmmo(primaryWeapon->iAmmoType) > 0)
			weaponSet->m_primaryWeapon.SetFillAmmo(true);
		else
			weaponSet->m_primaryWeapon.SetFillAmmo(false);
	}

	if (secondaryWeapon)
	{
		weaponSet->m_secondaryWeapon.SetWeaponID((WeaponIdType)secondaryWeapon->iId);
		weaponSet->m_secondaryWeapon.SetAmmoType(AMMO_CLIPS);

		if (gWR.CountAmmo(secondaryWeapon->iAmmoType) > 0)
			weaponSet->m_secondaryWeapon.SetFillAmmo(true);
		else
			weaponSet->m_secondaryWeapon.SetFillAmmo(false);
	}

	if (knifeWeapon)
	{
		weaponSet->m_knifeWeapon.SetWeaponID((WeaponIdType)knifeWeapon->iId);
		weaponSet->m_knifeWeapon.SetAmmoType(AMMO_CLIPS);

		if (gWR.CountAmmo(knifeWeapon->iAmmoType) > 0)
			weaponSet->m_knifeWeapon.SetFillAmmo(true);
		else
			weaponSet->m_knifeWeapon.SetFillAmmo(false);
	}

	weaponSet->m_armor = gHUD.m_Battery.m_iBat;
	weaponSet->m_helmet = gHUD.m_Battery.m_iArmorType;

	weaponSet->m_defuser = gHUD.m_itemStatus.m_hasDefuser;
	weaponSet->m_nightvision = gHUD.m_itemStatus.m_hasNightvision;
}

WeaponSet::WeaponSet(void)
{
	Reset();
}

void WeaponSet::Reset(void)
{
	static BuyPresetWeapon m_blankWeapon;

	m_primaryWeapon = m_blankWeapon;
	m_secondaryWeapon = m_blankWeapon;
	m_knifeWeapon = m_blankWeapon;

	m_armor = 0;
	m_helmet = false;
	m_smokeGrenade = false;
	m_HEGrenade = false;
	m_flashbangs = 0;
	m_defuser = false;
	m_nightvision = false;
}

void WeaponSet::GetCurrent(int &cost, WeaponSet &ws) const
{
	cost = -1;
	ws.Reset();
}

void WeaponSet::GetFromScratch(int &cost, WeaponSet &ws) const
{
	cost = 0;
	ws.Reset();
}

int WeaponSet::FullCost(void) const
{
	WeaponSet fullSet;
	int cost = 0;
	GetFromScratch(cost, fullSet);
	return cost;
}

void WeaponSet::GenerateBuyCommands(char command[BUY_PRESET_COMMAND_LEN]) const
{
	command[0] = 0;

	char *tmp = command;
	int remainder = BUY_PRESET_COMMAND_LEN;

	if (m_primaryWeapon.GetWeaponID() != WEAPON_NONE)
	{
		tmp = BufPrintf(tmp, remainder, "%s\n", WeaponIDToAlias(m_primaryWeapon.GetWeaponID()));

		if (m_primaryWeapon.GetFillAmmo())
			tmp = BufPrintf(tmp, remainder, "primammo\n");
	}

	if (m_secondaryWeapon.GetWeaponID() != WEAPON_NONE)
	{
		tmp = BufPrintf(tmp, remainder, "%s\n", WeaponIDToAlias(m_secondaryWeapon.GetWeaponID()));

		if (m_secondaryWeapon.GetFillAmmo())
			tmp = BufPrintf(tmp, remainder, "secammo\n");
	}

	if (m_armor)
	{
		if (m_helmet)
			tmp = BufPrintf(tmp, remainder, "vesthelm\n");
		else
			tmp = BufPrintf(tmp, remainder, "vest\n");
	}

	if (m_smokeGrenade)
		tmp = BufPrintf(tmp, remainder, "sgren\n");

	if (m_HEGrenade)
		tmp = BufPrintf(tmp, remainder, "hegren\n");

	for (int i = 0; i < m_flashbangs; ++i)
		tmp = BufPrintf(tmp, remainder, "flash\n");

	if (m_defuser)
		tmp = BufPrintf(tmp, remainder, "defuser\n");

	if (m_nightvision)
		tmp = BufPrintf(tmp, remainder, "nvgs\n");
}

const char *ImageFnameFromWeaponID(WeaponIdType weaponID, bool isPrimary)
{
	switch (weaponID)
	{
		case WEAPON_NONE: return "gfx/vgui/defaultweapon";
		case WEAPON_SCOUT: return "gfx/vgui/scout";
		case WEAPON_XM1014: return "gfx/vgui/xm1014";
		case WEAPON_MAC10: return "gfx/vgui/mac10";
		case WEAPON_AUG: return "gfx/vgui/aug";
		case WEAPON_UMP45: return "gfx/vgui/ump45";
		case WEAPON_SG550: return "gfx/vgui/sg550";
		case WEAPON_GALIL: return "gfx/vgui/galil";
		case WEAPON_FAMAS: return "gfx/vgui/famas";
		case WEAPON_AWP: return "gfx/vgui/awp";
		case WEAPON_MP5N: return "gfx/vgui/mp5";
		case WEAPON_M249: return "gfx/vgui/m249";
		case WEAPON_M3: return "gfx/vgui/m3";
		case WEAPON_M4A1: return "gfx/vgui/m4a1";
		case WEAPON_TMP: return "gfx/vgui/tmp";
		case WEAPON_G3SG1: return "gfx/vgui/g3sg1";
		case WEAPON_SG552: return "gfx/vgui/sg552";
		case WEAPON_AK47: return "gfx/vgui/ak47";
		case WEAPON_P90: return "gfx/vgui/p90";
		case WEAPON_SHIELDGUN: return "gfx/vgui/shield";

		case WEAPON_USP: return "gfx/vgui/usp45";
		case WEAPON_GLOCK18: return "gfx/vgui/glock18";
		case WEAPON_DEAGLE: return "gfx/vgui/deserteagle";
		case WEAPON_ELITE: return "gfx/vgui/elites";
		case WEAPON_P228: return "gfx/vgui/p228";
		case WEAPON_FIVESEVEN: return "gfx/vgui/fiveseven";
	}

	return "";
}

BuyPreset::BuyPreset(void)
{
}

BuyPreset::BuyPreset(const BuyPreset &other)
{
	*this = other;
}

BuyPreset::~BuyPreset(void)
{
}

static void ParseWeaponString(const char *str, BuyPresetWeaponList &weapons, bool isPrimary)
{
	weapons.RemoveAll();

	if (!str)
		return;

	const char *remainder = SharedParse(str);
	const char *token;

	const int BufLen = 32;
	char tmpBuf[BufLen];
	char weaponBuf[BufLen];
	char clipModifier;

	while (remainder)
	{
		token = SharedGetToken();

		if (!token || strlen(token) >= BufLen)
			return;

		Q_strncpy(tmpBuf, token, BufLen);
		tmpBuf[BufLen - 1] = 0;
		char *tmp = tmpBuf;

		while (*tmp)
		{
			if (*tmp == '/')
				*tmp = ' ';

			tmp++;
		}

		if (sscanf(tmpBuf, "%s %c", weaponBuf, &clipModifier) != 2)
			return;

		WeaponIdType weaponID = (WeaponIdType)AliasToWeaponID(weaponBuf);

		BuyPresetWeapon weapon(weaponID);
		weapon.SetAmmoType(AMMO_CLIPS);
		weapon.SetFillAmmo((clipModifier == '+'));
		weapons.AddToTail(weapon);

		remainder = SharedParse(remainder);
	}
}

void BuyPreset::Parse(KeyValues *data)
{
	m_weaponList.RemoveAll();

	if (!data)
		return;

	const char *primaryString = data->GetString("Primary", NULL);
	const char *secondaryString = data->GetString("Secondary", NULL);
	const char *equipmentString = data->GetString("Equipment", NULL);

	CUtlVector<BuyPresetWeapon> weapons;
	ParseWeaponString(primaryString, weapons, true);

	WeaponSet ws;

	if (weapons.Count())
		ws.m_primaryWeapon = weapons[0];

	weapons.RemoveAll();
	ParseWeaponString(secondaryString, weapons, false);

	if (weapons.Count())
		ws.m_secondaryWeapon = weapons[0];

	if (ws.m_knifeWeapon.GetWeaponID() == WEAPON_NONE)
		ws.m_knifeWeapon.SetWeaponID(WEAPON_KNIFE);

	if (equipmentString)
	{
		const char *remainder = SharedParse(equipmentString);
		const char *token;

		const int BufLen = 32;
		char tmpBuf[BufLen];
		char itemBuf[BufLen];
		int intVal;

		while (remainder)
		{
			token = SharedGetToken();

			if (!token || Q_strlen(token) >= BufLen)
				break;

			Q_strncpy(tmpBuf, token, BufLen);
			tmpBuf[BufLen - 1] = 0;

			char *tmp = tmpBuf;

			while (*tmp)
			{
				if (*tmp == '/')
					*tmp = ' ';

				tmp++;
			}

			if (sscanf(tmpBuf, "%s %d", itemBuf, &intVal) != 2)
				break;

			if (!strcmp(itemBuf, "vest"))
			{
				ws.m_armor = (intVal > 0) ? 100 : 0;
				ws.m_helmet = false;
			}
			else if (!strcmp(itemBuf, "vesthelm"))
			{
				ws.m_armor = (intVal > 0) ? 100 : 0;
				ws.m_helmet = true;
			}
			else if (!strcmp(itemBuf, "defuser"))
			{
				ws.m_defuser = (intVal > 0);
			}
			else if (!strcmp(itemBuf, "nvgs"))
			{
				ws.m_nightvision = (intVal > 0);
			}
			else if (!strcmp(itemBuf, "sgren"))
			{
				ws.m_smokeGrenade = (intVal > 0);
			}
			else if (!strcmp(itemBuf, "hegren"))
			{
				ws.m_HEGrenade = (intVal > 0);
			}
			else if (!strcmp(itemBuf, "flash"))
			{
				ws.m_flashbangs = min(2, max(0, intVal));
			}

			remainder = SharedParse(remainder);
		}

		m_weaponList.AddToTail(ws);
	}
}

static const char *ConstructWeaponString(const BuyPresetWeapon &weapon)
{
	const int WeaponLen = 1024;
	static char weaponString[WeaponLen];
	weaponString[0] = 0;
	int remainder = WeaponLen;
	char *tmp = weaponString;
	tmp = BufPrintf(tmp, remainder, "%s/%c", WeaponIDToAlias(weapon.GetWeaponID()), (weapon.GetFillAmmo()) ? '+' : '=');
	return weaponString;
}

void BuyPreset::Save(KeyValues *data)
{
	KeyValues *presetKey = data->CreateNewKey();

	if (m_weaponList.Count() > 0)
	{
		const WeaponSet &ws = m_weaponList[0];

		presetKey->SetString("Primary", ConstructWeaponString(ws.m_primaryWeapon));
		presetKey->SetString("Secondary", ConstructWeaponString(ws.m_secondaryWeapon));
		presetKey->SetString("Equipment", SharedVarArgs("vest%s/%d flash/%d sgren/%d hegren/%d defuser/%d nvgs/%d", (ws.m_helmet) ? "helm" : "", ws.m_armor, ws.m_flashbangs, ws.m_smokeGrenade, ws.m_HEGrenade, ws.m_defuser, ws.m_nightvision));
	}
	else
	{
		presetKey->SetString("Primary", "");
		presetKey->SetString("Secondary", "");
		presetKey->SetString("Equipment", "");
	}
}

int BuyPreset::FullCost(void) const
{
	if (m_weaponList.Count() == 0)
		return 0;

	return m_weaponList[0].FullCost();
}

const WeaponSet *BuyPreset::GetSet(int index) const
{
	if (index < 0 || index >= m_weaponList.Count())
		return NULL;

	return &(m_weaponList[index]);
}

void BuyPreset::DeleteSet(int index)
{
	if (index < 0 || index >= m_weaponList.Count())
		return;

	m_weaponList.Remove(index);
}

void BuyPreset::SwapSet(int firstIndex, int secondIndex)
{
	if (firstIndex < 0 || firstIndex >= m_weaponList.Count())
		return;

	if (secondIndex < 0 || secondIndex >= m_weaponList.Count())
		return;

	WeaponSet tmpSet = m_weaponList[firstIndex];
	m_weaponList[firstIndex] = m_weaponList[secondIndex];
	m_weaponList[secondIndex] = tmpSet;
}

void BuyPreset::ReplaceSet(int index, const WeaponSet &weaponSet)
{
	if (index < 0 || index > m_weaponList.Count())
		return;

	if (index == m_weaponList.Count())
		m_weaponList.AddToTail(weaponSet);
	else
		m_weaponList[index] = weaponSet;
}