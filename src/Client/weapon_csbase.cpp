#include "hud.h"
#include "in_buttons.h"
#include "weapon_csbase.h"

static const char *s_WeaponAliasInfo[] =
{
	"none",		// WEAPON_NONE
	"p228",		// WEAPON_P228
	"",			// WEAPON_GLOCK
	"scout",	// WEAPON_SCOUT
	"hegren",	// WEAPON_HEGRENADE
	"xm1014",	// WEAPON_XM1014
	"c4",		// WEAPON_C4
	"mac10",	// WEAPON_MAC10
	"aug",		// WEAPON_AUG
	"sgren",	// WEAPON_SMOKEGRENADE
	"elites",	// WEAPON_ELITE
	"fiveseven",// WEAPON_FIVESEVEN
	"ump45",	// WEAPON_UMP45
	"sg550",	// WEAPON_SG550
	"galil",	// WEAPON_GALIL
	"famas",	// WEAPON_FAMAS
	"usp",		// WEAPON_USP
	"glock",	// WEAPON_GLOCK18
	"awp",		// WEAPON_AWP
	"mp5",		// WEAPON_MP5N
	"m249",		// WEAPON_M249
	"m3",		// WEAPON_M3
	"m4a1",		// WEAPON_M4A1
	"tmp",		// WEAPON_TMP
	"g3sg1",	// WEAPON_G3SG1
	"flash",	// WEAPON_FLASHBANG
	"deagle",	// WEAPON_DEAGLE
	"sg552",	// WEAPON_SG552
	"ak47",		// WEAPON_AK47
	"knife",	// WEAPON_KNIFE
	"p90",		// WEAPON_P90
	"shield",	// WEAPON_SHIELDGUN
};

int AliasToWeaponID(const char *alias)
{
	if (alias)
	{
		for (int i = 0; s_WeaponAliasInfo[i] != NULL; ++i)
		{
			if (!Q_stricmp(s_WeaponAliasInfo[i], alias))
				return i;
		}
	}

	return WEAPON_NONE;
}

const char *WeaponIDToAlias(int id)
{
	if ((id >= WEAPON_MAX) || (id < 0))
		return NULL;

	return s_WeaponAliasInfo[id];
}