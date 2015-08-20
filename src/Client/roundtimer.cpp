#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include "CounterStrikeViewport.h"

pfnUserMsgHook g_pfnMSG_RoundTime;

DECLARE_MESSAGE(m_roundTimer, RoundTime)

int CHudRoundTimer::Init(void)
{
	g_pfnMSG_RoundTime = HOOK_MESSAGE(RoundTime);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudRoundTimer::VidInit(void)
{
	m_HUD_stopwatch = gHUD.GetSpriteIndex("stopwatch");
	m_HUD_colon = gHUD.GetSpriteIndex("colon");
	return 1;
}

void CHudRoundTimer::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
	m_fFade = 0;
	m_flTimeEnd = 0;
	m_flNewPeriod = 0;
	m_flNextToggle = 0;
	m_bPanicColor = 0;
	m_closestRight = ScreenWidth;
}

bool IsBombPlanted(void)
{
	return g_PlayerExtraInfo[33].playerclass && !gHUD.m_bShowTimer && g_PlayerExtraInfo[33].radarflash != -1;
}

int CHudRoundTimer::Draw(float flTime)
{
	int r, g, b;

	return 0;

	if (m_flNewPeriod)
	{
		m_flTimeEnd = m_flNewPeriod + flTime;
		m_flNewPeriod = 0;
	}

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_TIMER))
		return 1;

	if ((gViewPortInterface && gViewPortInterface->IsSpectatorGUIVisible()))
		return 1;

	if (IsBombPlanted())
		return 1;

	int timer = m_flTimeEnd - flTime;

	if (timer <= 0)
	{
		m_bPanicColor = true;
	}
	else if (timer <= 2)
	{
		if (flTime > m_flNextToggle)
		{
			m_flNextToggle = flTime + 0.05;
			m_bPanicColor = !m_bPanicColor;
		}
	}
	else if (timer <= 5)
	{
		if (flTime > m_flNextToggle)
		{
			m_flNextToggle = flTime + 0.1;
			m_bPanicColor = !m_bPanicColor;
		}
	}
	else if (timer <= 10)
	{
		if (flTime > m_flNextToggle)
		{
			m_flNextToggle = flTime + 0.2;
			m_bPanicColor = !m_bPanicColor;
		}
	}
	else if (timer <= 20)
	{
		if (flTime > m_flNextToggle)
		{
			m_flNextToggle = flTime + 0.4;
			m_bPanicColor = !m_bPanicColor;
		}
	}
	else if (timer <= 30)
	{
		if (flTime > m_flNextToggle)
		{
			m_flNextToggle = flTime + 0.8;
			m_bPanicColor = !m_bPanicColor;
		}
	}

	int timer_m, timer_s;

	if (timer > 0)
	{
		timer_m = timer / 60;
		timer_s = timer % 60;
	}
	else
	{
		timer_m = 0;
		timer_s = 0;
	}

	if (!m_bPanicColor)
	{
		UnpackRGB(r, g, b, RGB_YELLOWISH);
		ScaleColors(r, g, b, 100);
	}
	else
	{
		UnpackRGB(r, g, b, RGB_REDISH);
	}

	int x, y, w, yc2;

	if (ScreenWidth < 640)
		w = ScreenWidth - (5 * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));
	else
		w = ScreenWidth - (4 * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));

	x = w / 2;

	if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
		y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	else
		y = 42;

	if (ScreenWidth >= 400)
	{
		SPR_Set(gHUD.GetSprite(m_HUD_stopwatch), r, g, b);
		SPR_DrawAdditive(0, gHUD.GetSpriteRect(m_HUD_stopwatch).left + x - gHUD.GetSpriteRect(m_HUD_stopwatch).right, y, &gHUD.GetSpriteRect(m_HUD_stopwatch));
	}

	yc2 = y - (gHUD.m_iFontEngineHeight - gHUD.m_iFontHeight) / 2 - 1;
	x = gHUD.DrawHudNumber(x, y, DHN_2DIGITS | DHN_DRAWZERO, timer_m, r, g, b);

	if (m_HUD_colon != -1)
	{
		SPR_Set(gHUD.GetSprite(m_HUD_colon), r, g, b);
		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_colon));

		x += gHUD.GetSpriteRect(m_HUD_colon).right - gHUD.GetSpriteRect(m_HUD_colon).left;
	}
	else
	{
		x = gHUD.DrawHudString(x, yc2, ScreenWidth - 1, ":", r, g, b);
	}

	x = gHUD.DrawHudNumber(x, y, DHN_2DIGITS | DHN_DRAWZERO | DHN_FILLZERO, timer_s, r, g, b);

	m_closestRight = x + (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left) * 1.5;
	return 1;
}

int CHudRoundTimer::MsgFunc_RoundTime(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_bPanicColor = false;
	m_flNextToggle = 0;
	m_flNewPeriod = READ_SHORT();
	m_flTimeEnd = m_flNewPeriod + gHUD.m_flTime;

	return 1;
}