#define MAX_WEAPON_POSITIONS 21

class WeaponsResource
{
private:
	WEAPON rgWeapons[MAX_WEAPONS];
	WEAPON *rgSlots[MAX_WEAPON_SLOTS + 1][MAX_WEAPON_POSITIONS + 1];
	int riAmmo[MAX_AMMO_TYPES];

public:
	void Init(void)
	{
		memset(rgWeapons, 0, sizeof rgWeapons);
		Reset();
	}

	void Reset(void)
	{
		iOldWeaponBits = 0;
		memset(rgSlots, 0, sizeof rgSlots);
	}

	int iOldWeaponBits;
	WEAPON *GetWeapon(int iId) { return &rgWeapons[iId]; }

	void AddWeapon(WEAPON *wp)
	{
		rgWeapons[wp->iId] = *wp;
		LoadWeaponSprites(&rgWeapons[wp->iId]);
	}

	void PickupWeapon(WEAPON *wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = wp;
	}

	void DropWeapon(WEAPON *wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = NULL;
	}

	void DropAllWeapons(void)
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (rgWeapons[i].iId)
				DropWeapon(&rgWeapons[i]);
		}
	}

	WEAPON *GetWeaponSlot(int slot, int pos) { return rgSlots[slot][pos]; }
	void LoadWeaponSprites(WEAPON *wp);
	void LoadAllWeaponSprites(void);
	WEAPON *GetFirstPos(int iSlot);
	void SelectSlot(int iSlot, int fAdvance, int iDirection);
	WEAPON *GetNextActivePos(int iSlot, int iSlotPos);

	int HasAmmo(WEAPON *p);
	AMMO GetAmmo(int iId) { return riAmmo[iId]; }
	void SetAmmo(int iId, int iCount) { riAmmo[iId] = iCount; }
	int CountAmmo(int iId);
	HSPRITE *GetAmmoPicFromWeapon(int iAmmoId, wrect_t &rect);
};

extern WeaponsResource gWR;

#define MAX_HISTORY 12

enum
{
	HISTSLOT_EMPTY,
	HISTSLOT_AMMO,
	HISTSLOT_WEAP,
	HISTSLOT_ITEM,
};

class HistoryResource
{
private:
	struct HIST_ITEM
	{
		int type;
		float DisplayTime;
		int iCount;
		int iId;
		HSPRITE hSpr;
		wrect_t rc;
	};

	HIST_ITEM rgAmmoHistory[MAX_HISTORY];

public:
	void Init(void)
	{
		Reset();
	}

	void Reset(void)
	{
		memset(rgAmmoHistory, 0, sizeof rgAmmoHistory);
	}

	int iHistoryGap;
	int iCurrentHistorySlot;

	void AddToHistory(int iType, int iId, int iCount = 0);
	void AddToHistory(int iType, const char *szName, int iCount = 0);
	void CheckClearHistory(void);
	int DrawAmmoHistory(float flTime);
};

extern HistoryResource gHR;