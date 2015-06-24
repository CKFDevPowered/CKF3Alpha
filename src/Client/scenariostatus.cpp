#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"

pfnUserMsgHook g_pfnMSG_Scenario;

DECLARE_MESSAGE(m_scenarioStatus, Scenario)

int CHudScenarioStatus::Init(void)
{
	g_pfnMSG_Scenario = HOOK_MESSAGE(Scenario);

	m_iFlags = 0;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudScenarioStatus::VidInit(void)
{
	m_alpha = 100;
	m_hSprite = NULL;

	return 1;
}

void CHudScenarioStatus::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_hSprite = NULL;
	m_nextFlash = 0;
}

int CHudScenarioStatus::Draw(float fTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return 1;

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if (!m_hSprite)
		return 1;

	if (m_iFlags & HUD_ACTIVE)
	{
		int r, g, b;
		UnpackRGB(r, g, b, RGB_YELLOWISH);

		int x = gHUD.m_roundTimer.m_closestRight;
		int y = ScreenHeight + (3 * gHUD.m_iFontHeight) / -2 - (m_rect.bottom - m_rect.top - gHUD.m_iFontHeight) / 2;

		ScaleColors(r, g, b, m_alpha);

		if (m_alpha > 100)
			m_alpha *= 0.9;

		SPR_Set(m_hSprite, r, g, b);
		SPR_DrawAdditive(0, x, y, &m_rect);
	}

	if (m_flashInterval && fTime >= m_nextFlash)
	{
		m_nextFlash = m_flashInterval + fTime;
		m_alpha = m_flashAlpha;
	}

	return 1;
}

int CHudScenarioStatus::MsgFunc_Scenario(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	bool wasActive = m_iFlags & HUD_ACTIVE;
	const char *spriteName;
	int sprIndex;
	int alpha;

	if (!READ_BYTE())
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 1;
	}

	m_iFlags |= HUD_ACTIVE;

	spriteName = READ_STRING();
	sprIndex = gHUD.GetSpriteIndex(spriteName);

	m_hSprite = gHUD.GetSprite(sprIndex);
	m_rect = gHUD.GetSpriteRect(sprIndex);

	alpha = READ_BYTE();

	if (alpha < MIN_ALPHA)
		alpha = MIN_ALPHA;

	m_flashAlpha = alpha;

	if (wasActive)
		alpha = m_alpha;
	else
		m_alpha = alpha;

	if (alpha == MIN_ALPHA)
		return 1;

	m_flashInterval = READ_SHORT() * 0.01;

	if (!m_nextFlash)
		m_nextFlash = gHUD.m_flTime;

	m_nextFlash += READ_SHORT() * 0.01;
	return 1;
}