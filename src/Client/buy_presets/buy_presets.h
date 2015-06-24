#ifndef BUY_PRESETS_H
#define BUY_PRESETS_H
#ifdef _WIN32
#pragma once
#endif

#define USE_BUY_PRESETS 1

#include "weapon_csbase.h"
#include <KeyValues.h>
#include <UtlVector.h>

class BuyPreset;
class CCSWeaponInfo;

enum BuyPresetStringSizes
{
	BUY_PRESET_COMMAND_LEN = 256,
	MaxBuyPresetName = 64,
	MaxBuyPresetImageFname = 64,
};

enum AmmoSizeType
{
	AMMO_PERCENT,
	AMMO_CLIPS,
	AMMO_ROUNDS
};

enum { NUM_PRESETS = 5 };

class BuyPresetWeapon
{
public:
	BuyPresetWeapon(void);
	BuyPresetWeapon(WeaponIdType weaponID);
	BuyPresetWeapon &operator = (const BuyPresetWeapon &other);

public:
	const wchar_t *GetName(void) const { return m_name; }
	WeaponIdType GetWeaponID(void) const { return m_weaponID; }
	void SetWeaponID(WeaponIdType weaponID);

	void SetAmmoType(AmmoSizeType ammoType) { m_ammoType = ammoType; }
	void SetFillAmmo(bool fill) { m_fillAmmo = fill; }

	AmmoSizeType GetAmmoType(void) const { return m_ammoType; }
	bool GetFillAmmo(void) const { return m_fillAmmo; }

protected:
	const wchar_t *m_name;
	WeaponIdType m_weaponID;
	AmmoSizeType m_ammoType;
	int m_ammoAmount;
	bool m_fillAmmo;
};

typedef CUtlVector<BuyPresetWeapon> BuyPresetWeaponList;

class WeaponSet
{
public:
	WeaponSet(void);

public:
	void GetCurrent(int &cost, WeaponSet &ws) const;
	void GetFromScratch(int &cost, WeaponSet &ws) const;

	void GenerateBuyCommands(char command[BUY_PRESET_COMMAND_LEN]) const;
	int FullCost(void) const;

	void Reset(void);

	const BuyPresetWeapon &GetPrimaryWeapon(void) const { return m_primaryWeapon; }
	const BuyPresetWeapon &GetSecondaryWeapon(void) const { return m_secondaryWeapon; }
	const BuyPresetWeapon &GetKnifeWeapon(void) const { return m_knifeWeapon; }

public:
	BuyPresetWeapon m_primaryWeapon;
	BuyPresetWeapon m_secondaryWeapon;
	BuyPresetWeapon m_knifeWeapon;

	int m_armor;
	bool m_helmet;
	bool m_smokeGrenade;
	bool m_HEGrenade;
	int m_flashbangs;
	bool m_defuser;
	bool m_nightvision;
};

typedef CUtlVector<WeaponSet> WeaponSetList;

class BuyPreset
{
public:
	BuyPreset(void);
	~BuyPreset(void);
	BuyPreset(const BuyPreset &other);

public:
	void Parse(KeyValues *data);
	void Save(KeyValues *data);

	int GetNumSets(void) const { return m_weaponList.Count(); }
	const WeaponSet *GetSet(int index) const;

	int FullCost(void) const;

	void DeleteSet(int index);
	void SwapSet(int firstIndex, int secondIndex);
	void ReplaceSet(int index, const WeaponSet &weaponSet);

private:
	WeaponSetList m_weaponList;
};

typedef CUtlVector<BuyPreset> BuyPresetList;

class BuyPresetManager
{
public:
	BuyPresetManager(void);

public:
	void Save(void);
	void SetPurchasePreset(int presetIndex);
	void PurchasePreset(int presetIndex);

	int GetNumPresets(void) { VerifyLoadedTeam(); return m_presets.Count(); }
	const BuyPreset *GetPreset(int index) const;
	void SetPreset(int index, const BuyPreset *preset);
	void SetPresets(const BuyPresetList &presets) { m_presets = presets; }

	void SetEditPresets(const BuyPresetList &presets) { m_editPresets = presets; }
	int GetNumEditPresets(void) const { return m_editPresets.Count(); }

	BuyPreset *GetEditPreset(int index);
	const CUtlVector<BuyPreset> &GetEditPresets(void) const { return m_editPresets; }

	void ResetEditPresets(void) { m_editPresets = m_presets; }
	void ResetEditToDefaults(void);
	void GetCurrentLoadout(WeaponSet *weaponSet);

private:
	BuyPresetList m_presets;
	BuyPresetList m_editPresets;

	int m_loadedTeam;

private:
	void VerifyLoadedTeam(void);
};

extern BuyPresetManager *TheBuyPresets;

const char *ImageFnameFromWeaponID(WeaponIdType weaponID, bool isPrimary);
BuyPresetWeaponList CareerWeaponList(const BuyPresetWeaponList &source, bool isPrimary, WeaponIdType currentClientID);

class CCSWeaponInfo;

int CalcClipsNeeded(const BuyPresetWeapon *pWeapon, const CCSWeaponInfo *pInfo, const int ammo[MAX_AMMO_TYPES]);
void FillClientAmmo(int ammo[MAX_AMMO_TYPES]);
bool CanBuyWeapon(WeaponIdType currentPrimaryID, WeaponIdType currentSecondaryID, WeaponIdType weaponID);
const wchar_t *WeaponIDToDisplayName(WeaponIdType weaponID);

#endif
