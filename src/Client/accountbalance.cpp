#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"

#define MONEY_FADE_TIME 5.0
#define MONEY_FASTFADE_TIME (MONEY_FADE_TIME / 2)

pfnUserMsgHook g_pfnMSG_Money;
pfnUserMsgHook g_pfnMSG_BlinkAcct;

DECLARE_MESSAGE(m_accountBalance, Money)
DECLARE_MESSAGE(m_accountBalance, BlinkAcct)

int CHudAccountBalance::Init(void)
{
	g_pfnMSG_Money = HOOK_MESSAGE(Money);
	g_pfnMSG_BlinkAcct = HOOK_MESSAGE(BlinkAcct);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudAccountBalance::VidInit(void)
{
	m_HUD_dollar = gHUD.GetSpriteIndex("dollar");
	m_HUD_minus = gHUD.GetSpriteIndex("minus");
	m_HUD_plus = gHUD.GetSpriteIndex("plus");

	return 1;
}

void CHudAccountBalance::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
	m_iAccount = 0;
	m_iAccountDelta = 0;
	m_fFade = 0;
	m_fFadeFast = 0;
	m_iBlinkCount = 0;
	m_fBlinkTime = 0;
}

int CHudAccountBalance::Draw(float flTime)
{
	if (gEngfuncs.IsSpectateOnly())
		return 1;

	int r, g, b, a;
	UnpackRGB(r, g, b, RGB_YELLOWISH);

	int r1, g1, b1, a1;
	int x, y;

	if (m_fFade)
	{
		m_fFade -= gHUD.m_flTimeDelta;

		if (m_fFade <= 0)
		{
			a = MIN_ALPHA;
			m_fFade = 0;
			m_iAccountDelta = 0;
		}

		a = MIN_ALPHA + (m_fFade / MONEY_FADE_TIME) * 128;

		if (m_iAccountDelta > 0)
		{
			r1 = 0;
			g1 = 255;
			b1 = 0;
		}
		else if (m_iAccountDelta < 0)
		{
			r1 = 255;
			g1 = 0;
			b1 = 0;
		}

		assert(m_fFade <= MONEY_FADE_TIME);

		UnpackRGB(r, g, b, RGB_YELLOWISH);
		ScaleColors(r, g, b, ((MONEY_FADE_TIME - m_fFade) / MONEY_FADE_TIME) * 255.0);
		ScaleColors(r1, g1, b1, (m_fFade / MONEY_FADE_TIME) * 255.0);

		r += r1;
		g += g1;
		b += b1;
	}
	else
	{
		a = MIN_ALPHA;
	}

	ScaleColors(r, g, b, a);

	if (m_bShowDelta && m_fFadeFast)
	{
		m_fFadeFast -= gHUD.m_flTimeDelta;

		if (m_fFadeFast > 0)
		{
			r1 = r;
			g1 = g;
			b1 = b;
			a1 = (m_fFadeFast / MONEY_FASTFADE_TIME) * 255.0;
			ScaleColors(r1, g1, b1, a1);
		}
		else
		{
			m_fFadeFast = 0;
			m_bShowDelta = false;
		}
	}

	if (m_iBlinkCount > 0)
	{
		if (flTime > m_fBlinkTime)
		{
			m_fBlinkTime = flTime + 0.1;
			m_iBlinkCount--;
		}

		if (m_iBlinkCount & 1)
		{
			UnpackRGB(r, g, b, RGB_YELLOWISH);
			ScaleColors(r, g, b, 255);
		}
	}

	assert(a >= 0 && g >= 0 && b >= 0);
	assert(a <= 255 && g <= 255 && b <= 255);

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_MONEY) || g_iUser1)
		return 1;

	if (ScreenWidth >= 640)
		x = ScreenWidth - 16 - (max(5, gHUD.GetNumBits(m_iAccount)) * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));
	else
		x = ScreenWidth - 12 - (max(5, gHUD.GetNumBits(m_iAccount)) * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));

	if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
		y = ScreenHeight + -(gHUD.m_iFontHeight * 2) - gHUD.m_iFontHeight;
	else
		y = ScreenHeight + -(gHUD.m_iFontHeight * 5);

	SPR_Set(gHUD.GetSprite(m_HUD_dollar), r, g, b);
	SPR_DrawAdditive(0, x - (gHUD.GetSpriteRect(m_HUD_dollar).right - gHUD.GetSpriteRect(m_HUD_dollar).left), y, &gHUD.GetSpriteRect(m_HUD_dollar));
	gHUD.DrawHudNumber(x, y, m_iAccount, r, g, b);

	if (m_bShowDelta)
	{
		int nSpr;

		if (m_iAccountDelta < 0)
			nSpr = m_HUD_minus;
		else
			nSpr = m_HUD_plus;

		y -= gHUD.m_iFontHeight + (gHUD.m_iFontHeight / 2);

		SPR_Set(gHUD.GetSprite(nSpr), r1, g1, b1);
		SPR_DrawAdditive(0, gHUD.GetSpriteRect(nSpr).left + x - gHUD.GetSpriteRect(nSpr).right, y, &gHUD.GetSpriteRect(nSpr));

		if (ScreenWidth >= 640)
			x = ScreenWidth - 16 - (max(5, gHUD.GetNumBits(m_iAccountDelta)) * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));
		else
			x = ScreenWidth - 12 - (max(5, gHUD.GetNumBits(m_iAccountDelta)) * (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left));

		gHUD.DrawHudNumber(x, y, abs(m_iAccountDelta), r1, g1, b1);
	}

	return 1;
}

int CHudAccountBalance::MsgFunc_Money(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int lastAccount = m_iAccount;
	int lastAccountDelta = m_iAccountDelta;

	m_iAccount = READ_LONG();
	m_bShowDelta = READ_BYTE();
	m_iAccountDelta = m_iAccount - lastAccount;

	if (m_iAccount == lastAccount)
	{
		if (m_bShowDelta)
			m_bShowDelta = false;
	}
	else
	{
		if ((m_fFade / MONEY_FADE_TIME) == 1.0)
			m_iAccountDelta += lastAccountDelta;

		m_fFade = MONEY_FADE_TIME;
		m_fFadeFast = MONEY_FASTFADE_TIME;
	}

	return g_pfnMSG_Money(pszName, iSize, pbuf);
}

int CHudAccountBalance::MsgFunc_BlinkAcct(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_fBlinkTime = 0;
	m_iBlinkCount = READ_BYTE() * 2;

	return 1;
}