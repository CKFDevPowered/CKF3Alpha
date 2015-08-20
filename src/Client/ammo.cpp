#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "ammohistory.h"
#include "pm_shared.h"
#include "shared_util.h"
#include "weapons.h"
#include "configs.h"
#include "CounterStrikeViewport.h"
#include <ICKFClient.h>

extern ICKFClient *g_pCKFClient;

WEAPON *gpActiveSel;
WEAPON *gpLastSel;

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_iShotsFired;
int g_weaponselect = 0;

void WeaponsResource::LoadAllWeaponSprites(void)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iId)
			LoadWeaponSprites(&rgWeapons[i]);
	}
}

int WeaponsResource::CountAmmo(int iId)
{
	if (iId < 0)
		return 0;

	return riAmmo[iId];
}

int WeaponsResource::HasAmmo(WEAPON *p)
{
	if (!p)
		return FALSE;

	if (p->iMax1 == -1)
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType) || CountAmmo(p->iAmmo2Type) || (p->iFlags & WEAPON_FLAGS_SELECTONEMPTY);
}

void WeaponsResource::LoadWeaponSprites(WEAPON *pWeapon)
{
	int i, iRes;

	if (ScreenWidth < 640)
		iRes = 320;
	else
		iRes = 640;

	char sz[128];

	if (!pWeapon)
		return;

	memset(&pWeapon->rcActive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcInactive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo2, 0, sizeof(wrect_t));

	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	if (strlen(pWeapon->szExtraName))
		sprintf(sz, "sprites/%s.txt", pWeapon->szExtraName);
	else
		sprintf(sz, "sprites/%s.txt", pWeapon->szName);

	client_sprite_t *pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t *p = GetSpriteList(pList, "crosshair", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = NULL;

	p = GetSpriteList(pList, "autoaim", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList(pList, "zoom", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair;
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);

	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo2 = 0;
}

WEAPON *WeaponsResource::GetFirstPos(int iSlot)
{
	WEAPON *pret = NULL;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if (rgSlots[iSlot][i])
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}

WEAPON *WeaponsResource::GetNextActivePos(int iSlot, int iSlotPos)
{
	if (iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS)
		return NULL;

	WEAPON *p = gWR.rgSlots[iSlot][iSlotPos + 1];

	if (!p)
		return GetNextActivePos(iSlot, iSlotPos + 1);

	return p;
}

int giBucketHeight, giBucketWidth, giABHeight, giABWidth;
HSPRITE ghsprBuckets;

DECLARE_MESSAGE(m_Ammo, CurWeapon);
DECLARE_MESSAGE(m_Ammo, CurWeapon2);
DECLARE_MESSAGE(m_Ammo, WeaponList);
DECLARE_MESSAGE(m_Ammo, AmmoX);
DECLARE_MESSAGE(m_Ammo, AmmoX2);
DECLARE_MESSAGE(m_Ammo, AmmoPickup);
DECLARE_MESSAGE(m_Ammo, WeapPickup);
DECLARE_MESSAGE(m_Ammo, WeapPickup2);
DECLARE_MESSAGE(m_Ammo, HideWeapon);
DECLARE_MESSAGE(m_Ammo, Crosshair);
DECLARE_MESSAGE(m_Ammo, ItemPickup);

DECLARE_COMMAND(m_Ammo, Slot1);
DECLARE_COMMAND(m_Ammo, Slot2);
DECLARE_COMMAND(m_Ammo, Slot3);
DECLARE_COMMAND(m_Ammo, Slot4);
DECLARE_COMMAND(m_Ammo, Slot5);
DECLARE_COMMAND(m_Ammo, Slot6);
DECLARE_COMMAND(m_Ammo, Slot7);
DECLARE_COMMAND(m_Ammo, Slot8);
DECLARE_COMMAND(m_Ammo, Slot9);
DECLARE_COMMAND(m_Ammo, Slot10);
DECLARE_COMMAND(m_Ammo, Close);
DECLARE_COMMAND(m_Ammo, NextWeapon);
DECLARE_COMMAND(m_Ammo, PrevWeapon);
DECLARE_COMMAND(m_Ammo, Adjust_Crosshair);

#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME "5"

pfnUserMsgHook g_pfnMSG_CurWeapon;
pfnUserMsgHook g_pfnMSG_HideWeapon;
pfnUserMsgHook g_pfnMSG_WeaponList;
pfnUserMsgHook g_pfnMSG_Crosshair;

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(AmmoX);
	HOOK_MESSAGE(AmmoX2);

	HOOK_MESSAGE(CurWeapon2);
	HOOK_MESSAGE(WeapPickup2);

	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);
	HOOK_COMMAND("adjust_crosshair", Adjust_Crosshair);

	//g_pfnMSG_CurWeapon = HOOK_MESSAGE(CurWeapon);
	//g_pfnMSG_HideWeapon = HOOK_MESSAGE(HideWeapon);
	g_pfnMSG_WeaponList = HOOK_MESSAGE(WeaponList);
	g_pfnMSG_Crosshair = HOOK_MESSAGE(Crosshair);

	Reset();

	m_iFlags |= (HUD_ACTIVE | HUD_INTERMISSION);
	m_bObserverCrosshair = false;
	m_flCrosshairDistance = 0;
	m_iAmmoLastCheck = 0;
	m_iAlpha = 200;
	m_R = 50;
	m_G = 250;
	m_B = 50;
	m_cvarR = -1;
	m_cvarG = -1;
	m_cvarB = -1;
	m_iCurrentCrosshair = 0;
	m_bAdditive = 1;
	m_iCrosshairScaleBase = 1024;
	m_szLastCrosshairColor[0] = 0;
	m_flLastCalcTime = 0;

	gWR.Init();
	gHR.Init();
	return 1;
};

void CHudAmmo::Reset(void)
{
	m_fFade = 0;
	m_iAlpha = 120;
	m_iFlags |= HUD_ACTIVE;

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();
}

int CHudAmmo::VidInit(void)
{
	m_HUD_bucket0 = gHUD.GetSpriteIndex("bucket1");
	m_HUD_selection = gHUD.GetSpriteIndex("selection");

	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	gHR.iHistoryGap = max(gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

	int i;
	client_sprite_t *pList = SPR_GetList("sprites/observer.txt", &i);

	if (pList)
	{
		int iRes;

		if (ScreenWidth < 640)
			iRes = 320;
		else
			iRes = 640;

		char sz[128];
		client_sprite_t *p = GetSpriteList(pList, "crosshair", iRes, i);

		if (p)
		{
			sprintf(sz, "sprites/%s.spr", p->szSprite);
			m_hObserverCrosshair = SPR_Load(sz);
			m_rcObserverCrosshair = p->rc;
		}
		else
			m_hObserverCrosshair = NULL;
	}

	return 1;
}

void CHudAmmo::Think(void)
{
	return;

	if (gHUD.m_fPlayerDead)
		return;

	if (gHUD.m_iWeaponBits != gWR.iOldWeaponBits)
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS - 1; i > 0; i--)
		{
			WEAPON *p = gWR.GetWeapon(i);

			if (p)
			{
				if (gHUD.m_iWeaponBits & (1 << p->iId))
					gWR.PickupWeapon(p);
				else
					gWR.DropWeapon(p);
			}
		}
	}

	if (!gpActiveSel)
		return;

	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (gpActiveSel != (WEAPON *)1)
		{
			if (strlen(gpActiveSel->szExtraName))
				ServerCmd(gpActiveSel->szExtraName);
			else
				ServerCmd(gpActiveSel->szName);

			g_weaponselect = gpActiveSel->iId;
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	}
}

HSPRITE *WeaponsResource::GetAmmoPicFromWeapon(int iAmmoId, wrect_t &rect)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iAmmoType == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if (rgWeapons[i].iAmmo2Type == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}

void WeaponsResource::SelectSlot(int iSlot, int fAdvance, int iDirection)
{
	if (gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1))
	{
		gHUD.m_Menu.SelectMenuItem(iSlot + 1);
		return;
	}

	if (iSlot > MAX_WEAPON_SLOTS)
		return;

	if (gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
		return;

	g_pCKFClient->SwitchWeapon(iSlot);

	/*if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	WEAPON *p = NULL;
	bool fastSwitch = CVAR_GET_FLOAT("hud_fastswitch") != 0;

	if ((gpActiveSel == NULL) || (gpActiveSel == (WEAPON *)1) || (iSlot != gpActiveSel->iSlot))
	{
		PlaySound("common/wpn_hudon.wav", 1);
		p = GetFirstPos(iSlot);

		if (p && fastSwitch)
		{
			WEAPON *p2 = GetNextActivePos(p->iSlot, p->iSlotPos);

			if (!p2)
			{
				ServerCmd(p->szName);
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);

		if (gpActiveSel)
			p = GetNextActivePos(gpActiveSel->iSlot, gpActiveSel->iSlotPos);

		if (!p)
			p = GetFirstPos(iSlot);
	}

	if (!p)
	{
		if (!fastSwitch)
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else
		gpActiveSel = p;*/
}

int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo(iIndex, abs(iCount));
	return 1;
}

int CHudAmmo::MsgFunc_AmmoX2(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = READ_SHORT();

	gWR.SetAmmo(iIndex, abs(iCount));
	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gHR.AddToHistory(HISTSLOT_AMMO, iIndex, abs(iCount));
	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	WEAPON *p = gWR.GetWeapon(iIndex);

	gHR.AddToHistory(HISTSLOT_WEAP, iIndex);
	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup2(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	char szExtraName[32];
	strcpy(szExtraName, READ_STRING());
	WEAPON *p = gWR.GetWeapon(iIndex);

	gHR.AddToHistory(HISTSLOT_WEAP, iIndex);
	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const char *szName = READ_STRING();

	gHR.AddToHistory(HISTSLOT_ITEM, szName);
	return 1;
}

int CHudAmmo::MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf)
{
	static wrect_t nullrc;

	BEGIN_READ(pbuf, iSize);

	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if (!gEngfuncs.IsSpectateOnly())
	{
		if ((gHUD.m_iHideHUDDisplay & HIDEHUD_CROSSHAIR) || gHUD.m_iHideHUDDisplay == 0)
		{
			SetCrosshair(0, nullrc, 0, 0, 0);
		}
		else if (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
		{
			if (!m_bObserverCrosshair)
			{
				gpActiveSel = NULL;
				SetCrosshair(0, nullrc, 0, 0, 0);
			}
		}
		else
		{
			if (m_pWeapon)
				SetCrosshair(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);

			m_bObserverCrosshair = false;
		}
	}

	MSG_BeginWrite();
	MSG_WriteByte(gHUD.m_iHideHUDDisplay);
	MSG_EndWrite(pszName, g_pfnMSG_HideWeapon);
	return 1;
}

int CHudAmmo::MsgFunc_Crosshair(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	static wrect_t nullrc;
	bool drawn = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if (gEngfuncs.pfnGetCvarFloat("cl_observercrosshair") == 0)
		return 1;

	if (drawn)
	{
		SetCrosshair(m_hObserverCrosshair, m_rcObserverCrosshair, 255, 255, 255);
	}
	else
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
	}

	m_bObserverCrosshair = true;
	return 1;
}

extern int g_iUser1;

int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf)
{
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	BEGIN_READ(pbuf, iSize);

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_CHAR();

	if (iState > 1)
		fOnTarget = TRUE;

	if (iId < 1 && !m_bObserverCrosshair)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);
		}

		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon(iId);

	if (!pWeapon)
		return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);

	if (iClip < -1)
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;

	if (iState == 0)
		return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);

	m_pWeapon = pWeapon;

	if (gHUD.m_iFOV >= 90)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
	}
	else
	{
		if (fOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
	}

	m_fFade = 200.0f;
	m_iFlags |= HUD_ACTIVE;
	m_bObserverCrosshair = false;

	return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);
}

int CHudAmmo::MsgFunc_CurWeapon2(const char *pszName, int iSize, void *pbuf)
{
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	BEGIN_READ(pbuf, iSize);

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_SHORT();

	if (iState > 1)
		fOnTarget = TRUE;

	if (iId < 1 && !m_bObserverCrosshair)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);
		}

		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon(iId);

	if (!pWeapon)
		return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);

	if (iClip < -1)
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;

	if (iState == 0)
		return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);

	m_pWeapon = pWeapon;

	if (gHUD.m_iFOV >= 90)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
	}
	else
	{
		if (fOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
	}

	m_fFade = 200.0f;
	m_iFlags |= HUD_ACTIVE;
	m_bObserverCrosshair = false;

	return g_pfnMSG_CurWeapon(pszName, iSize, pbuf);
}

int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	WEAPON Weapon;
	strcpy(Weapon.szName, READ_STRING());
	Weapon.iAmmoType = (int)READ_CHAR();
	Weapon.iMax1 = READ_BYTE();

	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = READ_CHAR();
	Weapon.iMax2 = READ_BYTE();

	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = READ_CHAR();
	Weapon.iSlotPos = READ_CHAR();
	Weapon.iId = READ_CHAR();
	Weapon.iFlags = READ_BYTE();
	Weapon.iClip = 0;
	Weapon.szExtraName[0] = '\0';

	gWR.AddWeapon(&Weapon);

	return g_pfnMSG_WeaponList(pszName, iSize, pbuf);
}

void CHudAmmo::SlotInput(int iSlot)
{
	if (gHUD.m_iIntermission)
		return;

	if (gViewPortInterface && gViewPortInterface->SlotInput(iSlot))
		return;

	gWR.SelectSlot(iSlot, FALSE, 1);
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput(0);
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput(1);
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput(2);
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput(3);
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput(4);
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput(5);
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput(6);
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput(7);
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput(8);
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput(9);
}

void CHudAmmo::UserCmd_Close(void)
{
	if (gpActiveSel)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
	{
		ClientCmd("escape");
	}
}

void CHudAmmo::UserCmd_NextWeapon(void)
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (!gpActiveSel || gpActiveSel == (WEAPON *)1)
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;

	if (gpActiveSel)
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++)
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++)
			{
				WEAPON *wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp)
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;
	}

	gpActiveSel = NULL;
}

void CHudAmmo::UserCmd_PrevWeapon(void)
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (!gpActiveSel || gpActiveSel == (WEAPON *)1)
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS - 1;
	int slot = MAX_WEAPON_SLOTS - 1;

	if (gpActiveSel)
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for ( ; slot >= 0; slot--)
		{
			for ( ; pos >= 0; pos--)
			{
				WEAPON *wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp)
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS - 1;
		}

		slot = MAX_WEAPON_SLOTS - 1;
	}

	gpActiveSel = NULL;
}

int CHudAmmo::Draw(float flTime)
{
	static wrect_t nullrc;
	int a, x, y, r, g, b;
	int AmmoWidth;

	return 0;

	if (gEngfuncs.IsSpectateOnly())
	{
		if (g_iUser1 != OBS_IN_EYE)
		{
			SetCrosshair(0, nullrc, 0, 0, 0);
			return 1;
		}

		WEAPON *pw = gWR.GetWeapon(WEAPON_AWP);

		if (pw->hZoomedCrosshair)
		{
			SetCrosshair(pw->hZoomedCrosshair, pw->rcZoomedCrosshair, 255, 255, 255);
			return 1;
		}

		SetCrosshair(0, nullrc, 0, 0, 0);
		DrawCrosshair(flTime, 0);
		return 1;
	}

	if (gHUD.m_iIntermission)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 1;
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
			return 1;

		if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
			return 1;

		DrawWList(flTime);

		gHR.DrawAmmoHistory(flTime);

		if (!(m_iFlags & HUD_ACTIVE))
			return 0;
	}

	if (!m_pWeapon)
		return 0;

	WEAPON *pw = m_pWeapon;

	if (gHUD.m_iFOV >= 55)
	{
		if (pw->iId != WEAPON_G3SG1 && pw->iId != WEAPON_AWP && pw->iId != WEAPON_SCOUT && pw->iId != WEAPON_SG550)
		{
			if (!(gHUD.m_iHideHUDDisplay & HIDEHUD_CROSSHAIR))
				DrawCrosshair(flTime, pw->iId);
		}
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
			return 0;

		int iFlags = DHN_DRAWZERO;

		AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

		a = (int)max(MIN_ALPHA, m_fFade);

		if (m_fFade > 0)
			m_fFade -= (gHUD.m_flTimeDelta * 20);

		UnpackRGB(r, g, b, RGB_YELLOWISH);
		ScaleColors(r, g, b, a);

		y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

		if (m_pWeapon->iAmmoType > 0)
		{
			int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;

			if (pw->iClip >= 0)
			{
				x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;
				x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, pw->iClip, r, g, b);

				wrect_t rc;
				rc.top = 0;
				rc.left = 0;
				rc.right = AmmoWidth;
				rc.bottom = 100;

				int iBarWidth = AmmoWidth / 10;
				x += AmmoWidth / 2;

				UnpackRGB(r, g, b, RGB_YELLOWISH);
				FillRGBA(x, y, iBarWidth, gHUD.m_iFontHeight, r, g, b, a);

				x += iBarWidth + AmmoWidth / 2;

				ScaleColors(r, g, b, a);
				x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
			}
			else
			{
				x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
				x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
			}

			int iOffset = (m_pWeapon->rcAmmo.bottom - m_pWeapon->rcAmmo.top) / 8;
			SPR_Set(m_pWeapon->hAmmo, r, g, b);
			SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo);
		}

		if (pw->iAmmo2Type > 0)
		{
			int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;

			if ((pw->iAmmo2Type != 0) && (gWR.CountAmmo(pw->iAmmo2Type) > 0))
			{
				y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;
				x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
				x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmo2Type), r, g, b);

				SPR_Set(m_pWeapon->hAmmo2, r, g, b);

				int iOffset = (m_pWeapon->rcAmmo2.bottom - m_pWeapon->rcAmmo2.top) / 8;
				SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo2);
			}
		}
	}

	return 1;
}

void CHudAmmo::UserCmd_Adjust_Crosshair(void)
{
	if (gEngfuncs.Cmd_Argc() <= 1)
		m_iCurrentCrosshair = (m_iCurrentCrosshair + 1) % 5;
	else
		m_iCurrentCrosshair = atoi(gEngfuncs.Cmd_Argv(1)) % 10;

	switch (m_iCurrentCrosshair)
	{
		default:
		{
			m_R = 50;
			m_G = 250;
			m_B = 50;
			m_bAdditive = true;
			break;
		}

		case 8:
		{
			m_R = 250;
			m_G = 250;
			m_B = 50;
			m_bAdditive = false;
			break;
		}

		case 9:
		{
			m_R = 50;
			m_G = 250;
			m_B = 250;
			m_bAdditive = false;
			break;
		}

		case 1:
		{
			m_R = 250;
			m_G = 50;
			m_B = 50;
			m_bAdditive = true;
			break;
		}

		case 2:
		{
			m_R = 50;
			m_G = 50;
			m_B = 250;
			m_bAdditive = true;
			break;
		}

		case 3:
		{
			m_R = 250;
			m_G = 250;
			m_B = 50;
			m_bAdditive = true;
			break;
		}

		case 4:
		{
			m_R = 50;
			m_G = 250;
			m_B = 250;
			m_bAdditive = true;
			break;
		}

		case 5:
		{
			m_R = 50;
			m_G = 250;
			m_B = 50;
			m_bAdditive = false;
			break;
		}

		case 6:
		{
			m_R = 250;
			m_G = 50;
			m_B = 50;
			m_bAdditive = false;
			break;
		}

		case 7:
		{
			m_R = 50;
			m_G = 50;
			m_B = 250;
			m_bAdditive = false;
			break;
		}
	}

	char cmd[256] = "";
	sprintf(cmd, "%d %d %d", m_R, m_G, m_B);
	gEngfuncs.Cvar_Set("cl_crosshair_color", cmd);

	if (m_bAdditive)
		gEngfuncs.Cvar_Set("cl_crosshair_translucent", "1");
	else
		gEngfuncs.Cvar_Set("cl_crosshair_translucent", "0");
}

void CHudAmmo::CalculateCrosshairColor(void)
{
	char *value = cl_crosshair_color->string;

	if (value && strcmp(value, m_szLastCrosshairColor))
	{
		int cvarR, cvarG, cvarB;
		char *token;
		char *data = value;

		data = SharedParse(data);
		token = SharedGetToken();

		if (token)
		{
			cvarR = atoi(token);

			data = SharedParse(data);
			token = SharedGetToken();

			if (token)
			{
				cvarG = atoi(token);

				data = SharedParse(data);
				token = SharedGetToken();

				if (token)
				{
					cvarB = atoi(token);

					if (m_cvarR != cvarR || m_cvarG != cvarG || m_cvarB != cvarB)
					{
						int r, g, b;

						r = min(max(0, cvarR), 255);
						g = min(max(0, cvarG), 255);
						b = min(max(0, cvarB), 255);

						m_R = r;
						m_G = g;
						m_B = b;
						m_cvarR = cvarR;
						m_cvarG = cvarG;
						m_cvarB = cvarB;
					}

					strcpy(m_szLastCrosshairColor, value);
				}
			}
		}
	}
}

void CHudAmmo::CalculateCrosshairDrawMode(void)
{
	float value = cl_crosshair_translucent->value;

	if (value == 0)
		m_bAdditive = false;
	else if (value == 1)
		m_bAdditive = true;
	else
		gEngfuncs.Con_Printf("usage: cl_crosshair_translucent <1|0>\n");
}

void CHudAmmo::CalculateCrosshairSize(void)
{
	char *value = cl_crosshair_size->string;

	if (!value)
		return;

	int size = atoi(value);

	if (size)
	{
		if (size > 3)
			size = -1;
	}
	else
	{
		if (strcmp(value, "0"))
			size = -1;
	}

	if (!stricmp(value, "auto"))
		size = 0;
	else if (!stricmp(value, "small"))
		size = 1;
	else if (!stricmp(value, "medium"))
		size = 2;
	else if (!stricmp(value, "large"))
		size = 3;

	if (size == -1)
	{
		gEngfuncs.Con_Printf("usage: cl_crosshair_size <auto|small|medium|large>\n");
		return;
	}

	switch (size)
	{
		case 0:
		{
			if (ScreenWidth < 640)
				m_iCrosshairScaleBase = 1024;
			else if (ScreenWidth < 1024)
				m_iCrosshairScaleBase = 800;
			else
				m_iCrosshairScaleBase = 640;

			break;
		}

		case 1:
		{
			m_iCrosshairScaleBase = 1024;
			break;
		}

		case 2:
		{
			m_iCrosshairScaleBase = 800;
			break;
		}

		case 3:
		{
			m_iCrosshairScaleBase = 640;
			break;
		}
	}
}

int GetWeaponAccuracyFlags(int iWeaponID)
{
	int flags;

	switch (iWeaponID)
	{
		case WEAPON_USP:
		{
			flags = (g_iWeaponFlags & 1) < 1 ? 7 : 15;
			break;
		}

		case WEAPON_GLOCK18:
		{
			flags = (g_iWeaponFlags & 2) < 1 ? 7 : 23;
			break;
		}

		case WEAPON_M4A1:
		{
			flags = (g_iWeaponFlags & 4) < 1 ? 3 : 11;
			break;
		}

		case WEAPON_MAC10:
		case WEAPON_UMP45:
		case WEAPON_MP5N:
		case WEAPON_TMP:
		{
			flags = 1;
			break;
		}

		case WEAPON_AUG:
		case WEAPON_GALIL:
		case WEAPON_M249:
		case WEAPON_SG552:
		case WEAPON_AK47:
		case WEAPON_P90:
		{
			flags = 3;
			break;
		}

		case WEAPON_P228:
		case WEAPON_FIVESEVEN:
		case WEAPON_DEAGLE:
		{
			flags = 7;
			break;
		}

		case WEAPON_FAMAS:
		{
			flags = (g_iWeaponFlags & 0x10) < 1 ? 3 : 19;
			break;
		}

		default:
		{
			flags = 0;
			break;
		}
	}

	return flags;
}

int CHudAmmo::DrawCrosshair(float flTime, int weaponid)
{
	int iDistance;
	int iDeltaDistance;
	int iWeaponAccuracyFlags;
	int iBarSize;
	float flCrosshairDistance;

	switch (weaponid)
	{
		case WEAPON_P228:
		case WEAPON_HEGRENADE:
		case WEAPON_SMOKEGRENADE:
		case WEAPON_FIVESEVEN:
		case WEAPON_USP:
		case WEAPON_GLOCK18:
		case WEAPON_AWP:
		case WEAPON_FLASHBANG:
		case WEAPON_DEAGLE:
		{
			iDistance = 8;
			iDeltaDistance = 3;
			break;
		}

		case WEAPON_MP5N:
		{
			iDistance = 6;
			iDeltaDistance = 2;
			break;
		}

		case WEAPON_M3:
		{
			iDistance = 8;
			iDeltaDistance = 6;
			break;
		}

		case WEAPON_G3SG1:
		{
			iDistance = 6;
			iDeltaDistance = 4;
			break;
		}

		case WEAPON_AK47:
		{
			iDistance = 4;
			iDeltaDistance = 4;
			break;
		}

		case WEAPON_TMP:
		case WEAPON_KNIFE:
		case WEAPON_P90:
		{
			iDistance = 7;
			iDeltaDistance = 3;
			break;
		}

		case WEAPON_XM1014:
		{
			iDistance = 9;
			iDeltaDistance = 4;
			break;
		}

		case WEAPON_MAC10:
		{
			iDistance = 9;
			iDeltaDistance = 3;
			break;
		}

		case WEAPON_AUG:
		{
			iDistance = 3;
			iDeltaDistance = 3;
			break;
		}

		case WEAPON_C4:
		case WEAPON_UMP45:
		case WEAPON_M249:
		{
			iDistance = 6;
			iDeltaDistance = 3;
			break;
		}

		case WEAPON_SCOUT:
		case WEAPON_SG550:
		case WEAPON_SG552:
		{
			iDistance = 5;
			iDeltaDistance = 3;
			break;
		}

		default:
		{
			iDistance = 4;
			iDeltaDistance = 3;
			break;
		}
	}

	iWeaponAccuracyFlags = GetWeaponAccuracyFlags(weaponid);

	if (iWeaponAccuracyFlags != 0 && cl_dynamiccrosshair && cl_dynamiccrosshair->value != 0.0 && !(gHUD.m_iHideHUDDisplay & HIDEHUD_CROSSHAIR))
	{
		if ((g_iPlayerFlags & FL_ONGROUND) || !(iWeaponAccuracyFlags & 1))
		{
			if ((g_iPlayerFlags & FL_DUCKING) && (iWeaponAccuracyFlags & 4))
			{
				iDistance *= 0.5;
			}
			else
			{
				float flLimitSpeed;

				switch (weaponid)
				{
					case WEAPON_MAC10:
					case WEAPON_SG550:
					case WEAPON_GALIL:
					case WEAPON_MP5N:
					case WEAPON_M3:
					case WEAPON_DEAGLE:
					case WEAPON_SG552:
					{
						flLimitSpeed = 140;
						break;
					}

					case WEAPON_KNIFE:
					{
						flLimitSpeed = 170;
						break;
					}

					default:
					{
						flLimitSpeed = 0;
						break;
					}
				}

				if (g_flPlayerSpeed > flLimitSpeed && (iWeaponAccuracyFlags & 2))
					iDistance *= 1.5;
			}
		}
		else
			iDistance *= 2;

		if (iWeaponAccuracyFlags & 8)
			iDistance *= 1.4;

		if (iWeaponAccuracyFlags & 0x10)
			iDistance *= 1.4;
	}

	if (g_iShotsFired > m_iAmmoLastCheck)
	{
		m_flCrosshairDistance += iDeltaDistance;
		m_iAlpha -= 40;

		if (m_flCrosshairDistance > 15)
			m_flCrosshairDistance = 15;

		if (m_iAlpha < 120)
			m_iAlpha = 120;
	}
	else
	{
		m_flCrosshairDistance -= (0.013 * m_flCrosshairDistance) + 0.1;
		m_iAlpha += 2;
	}

	if (g_iShotsFired > 600)
		g_iShotsFired = 1;

	m_iAmmoLastCheck = g_iShotsFired;

	if (iDistance > m_flCrosshairDistance)
		m_flCrosshairDistance = iDistance;

	if (m_iAlpha > 255)
		m_iAlpha = 255;

	iBarSize = (int)((m_flCrosshairDistance - (float)iDistance) * 0.5) + 5;

	if (gHUD.m_flTime > m_flLastCalcTime + 1)
	{
		CalculateCrosshairColor();
		CalculateCrosshairDrawMode();
		CalculateCrosshairSize();

		m_flLastCalcTime = gHUD.m_flTime;
	}

	flCrosshairDistance = m_flCrosshairDistance;

	if (m_iCrosshairScaleBase != ScreenWidth)
	{
		flCrosshairDistance *= (float)(ScreenWidth) / m_iCrosshairScaleBase;
		iBarSize = (float)(ScreenWidth * iBarSize) / m_iCrosshairScaleBase;
	}

	if (gHUD.m_NightVision.m_fOn)
		DrawCrosshairEx(flTime, weaponid, iBarSize, flCrosshairDistance, false, 250, 50, 50, m_iAlpha);
	else
		DrawCrosshairEx(flTime, weaponid, iBarSize, flCrosshairDistance, m_bAdditive, m_R, m_G, m_B, m_iAlpha);

	return 1;
}

int CHudAmmo::DrawCrosshairEx(float flTime, int weaponid, int iBarSize, float flCrosshairDistance, bool bAdditive, int r, int g, int b, int a)
{
	bool bDrawPoint = false;
	bool bDrawCircle = false;
	bool bDrawCross = false;

	void (*pfnFillRGBA)(int x, int y, int w, int h, int r, int g, int b, int a) = (bAdditive == false) ? gEngfuncs.pfnFillRGBABlend : gEngfuncs.pfnFillRGBA;

	switch ((int)cl_crosshair_type->value)
	{
		case 1:
		{
			bDrawPoint = true;
			bDrawCross = true;
			break;
		}

		case 2:
		{
			bDrawPoint = true;
			bDrawCircle = true;
			break;
		}

		case 3:
		{
			bDrawPoint = true;
			break;
		}

		default:
		{
			bDrawCross = true;
			break;
		}
	}

	if (bDrawCircle)
	{
		float radius = (iBarSize / 2) + flCrosshairDistance;
		int count = (int)((cos(M_PI / 4) * radius) + 0.5);

		for (int i = 0; i < count; i++)
		{
			int size = sqrt((radius * radius) - (float)(i * i));

			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
		}
	}

	if (bDrawPoint)
		pfnFillRGBA((ScreenWidth / 2) - 1, (ScreenHeight / 2) - 1, 3, 3, r, g, b, a);

	if (bDrawCross)
	{
		pfnFillRGBA((ScreenWidth / 2) - (int)flCrosshairDistance - iBarSize + 1, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA((ScreenWidth / 2) + (int)flCrosshairDistance, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) - (int)flCrosshairDistance - iBarSize + 1, 1, iBarSize, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) + (int)flCrosshairDistance, 1, iBarSize, r, g, b, a);
	}

	return 1;
}

int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	if (f < 0)
		f = 0;

	if (f > 1)
		f = 1;

	if (f)
	{
		int w = f * width;

		if (w <= 0)
			w = 1;

		UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	UnpackRGB(r, g, b, RGB_YELLOWISH);
	FillRGBA(x, y, width, height, r, g, b, 128);
	return (x + width);
}

void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	if (!p)
		return;

	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType) / (float)p->iMax1;
		x = DrawBar(x, y, width, height, f);

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type) / (float)p->iMax2;
			x += 5;

			DrawBar(x, y, width, height, f);
		}
	}
}

int CHudAmmo::DrawWList(float flTime)
{
	int r, g, b, x, y, a, i;

	if (!gpActiveSel)
		return 0;

	int iActiveSlot;

	if (gpActiveSel == (WEAPON *)1)
		iActiveSlot = -1;
	else
		iActiveSlot = gpActiveSel->iSlot;

	x = gHUD.m_Radar.GetRadarSize() + 10;
	y = 10;

	if (iActiveSlot > 0)
	{
		if (!gWR.GetFirstPos(iActiveSlot))
		{
			gpActiveSel = (WEAPON *)1;
			iActiveSlot = -1;
		}
	}

	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		int iWidth;

		UnpackRGB(r, g, b, RGB_YELLOWISH);

		if (iActiveSlot == i)
			a = 255;
		else
			a = 192;

		ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b);

		if (i == iActiveSlot)
		{
			WEAPON *p = gWR.GetFirstPos(iActiveSlot);

			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_bucket0 + i));
		x += iWidth + 5;
	}

	a = 128;
	x = gHUD.m_Radar.GetRadarSize() + 10;

	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		y = giBucketHeight + 10;

		if (i == iActiveSlot)
		{
			WEAPON *p = gWR.GetFirstPos(i);
			int iWidth = giBucketWidth;

			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				UnpackRGB(r, g, b, RGB_YELLOWISH);

				if (gWR.HasAmmo(p))
				{
					ScaleColors(r, g, b, 192);
				}
				else
				{
					UnpackRGB(r, g, b, RGB_REDISH);
					ScaleColors(r, g, b, 128);
				}

				if (gpActiveSel == p)
				{
					SPR_Set(p->hActive, r, g, b);
					SPR_DrawAdditive(0, x, y, &p->rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b);
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					SPR_Set(p->hInactive, r, g, b);
					SPR_DrawAdditive(0, x, y, &p->rcInactive);
				}

				DrawAmmoBar(p, x + giABWidth / 2, y, giABWidth, giABHeight);
				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;
		}
		else
		{
			UnpackRGB(r, g, b, RGB_YELLOWISH);

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				WEAPON *p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				if (gWR.HasAmmo(p))
				{
					UnpackRGB(r, g, b, RGB_YELLOWISH);
					a = 128;
				}
				else
				{
					UnpackRGB(r, g, b, RGB_REDISH);
					a = 96;
				}

				FillRGBA(x, y, giBucketWidth, giBucketHeight, r, g, b, a);
				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}

	return 1;
}

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while (i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;

		p++;
	}

	return NULL;
}