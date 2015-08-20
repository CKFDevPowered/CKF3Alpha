#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

pfnUserMsgHook g_pfnMSG_Battery;
pfnUserMsgHook g_pfnMSG_ArmorType;

DECLARE_MESSAGE(m_Battery, Battery)
DECLARE_MESSAGE(m_Battery, ArmorType)

int CHudBattery::Init(void)
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;
	m_iArmorType = 0;

	//g_pfnMSG_Battery = HOOK_MESSAGE(Battery);
	//g_pfnMSG_ArmorType = HOOK_MESSAGE(ArmorType);

	gHUD.AddHudElem(this);

	return 1;
}

int CHudBattery::VidInit(void)
{
	m_HUD_suit_empty = gHUD.GetSpriteIndex("suit_empty");
	m_HUD_suit_full = gHUD.GetSpriteIndex("suit_full");

	m_HUD_suithelmet_empty = gHUD.GetSpriteIndex("suithelmet_empty");
	m_HUD_suithelmet_full = gHUD.GetSpriteIndex("suithelmet_full");

	m_fFade = 0;
	return 1;
}

int CHudBattery::MsgFunc_Battery(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT();

	if (x != m_iBat)
	{
		m_fFade = FADE_TIME;
		m_iBat = x;

		if (x <= 0)
			m_iArmorType = 0;
	}

	return 1;
}

int CHudBattery::MsgFunc_ArmorType(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iArmorType = READ_BYTE();

	return 1;
}

int CHudBattery::Draw(float flTime)
{
	return 0;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return 1;

	int sprite1, sprite2;

	if (m_iArmorType == 1)
	{
		sprite1 = m_HUD_suithelmet_empty;
		sprite2 = m_HUD_suithelmet_full;
	}
	else
	{
		sprite1 = m_HUD_suit_empty;
		sprite2 = m_HUD_suit_full;
	}

	HSPRITE hSprite1 = gHUD.GetSprite(sprite1);
	HSPRITE hSprite2 = gHUD.GetSprite(sprite2);
	wrect_t *prc1 = &gHUD.GetSpriteRect(sprite1);
	wrect_t *prc2 = &gHUD.GetSpriteRect(sprite2);

	int r, g, b, x, y, a;
	wrect_t rc = *prc2;

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 1;

	if (m_fFade)
	{
		if (m_fFade > FADE_TIME)
			m_fFade = FADE_TIME;

		m_fFade -= (gHUD.m_flTimeDelta * 20);

		if (m_fFade <= 0)
		{
			a = 100;
			m_fFade = 0;
		}

		a = MIN_ALPHA + (m_fFade / FADE_TIME) * 128;

	}
	else
		a = MIN_ALPHA;

	ScaleColors(r, g, b, a);

	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	x = ScreenWidth / 5;

	SPR_Set(hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y, prc1);

	if (rc.bottom > rc.top)
	{
		SPR_Set(hSprite2, r, g, b);
		SPR_DrawAdditive(0, x, y + (rc.top - prc2->top), &rc);
	}

	x += (prc1->right - prc1->left);
	x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b);

	return 1;
}