#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "pmtrace.h"
#include "ev_hldm.h"

void CHudNightVision::Reset(void)
{
	m_fOn = FALSE;
	m_iAlpha = 110;
}

int CHudNightVision::VidInit(void)
{
	return 1;
}

DECLARE_COMMAND(m_NightVision, NVGAdjustUp);
DECLARE_COMMAND(m_NightVision, NVGAdjustDown);
DECLARE_MESSAGE(m_NightVision, NVGToggle);

int CHudNightVision::Init(void)
{
	Reset();

	HOOK_MESSAGE(NVGToggle);

	HOOK_COMMAND("+nvgadjust", NVGAdjustUp);
	HOOK_COMMAND("-nvgadjust", NVGAdjustDown);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudNightVision::Draw(float flTime)
{
	if (m_fOn && !gEngfuncs.IsSpectateOnly())
	{
		gEngfuncs.pfnFillRGBA(0, 0, ScreenWidth, ScreenHeight, 50, 225, 50, m_iAlpha);
		EV_HLDM_CreateLight(gHUD.m_vecOrigin, gEngfuncs.pfnRandomLong(0, 50) + 750, 1, 20, 1, 0.1);
	}

	return 1;
}

int CHudNightVision::MsgFunc_NVGToggle(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_fOn = READ_BYTE();
	return 1;
}

void CHudNightVision::UserCmd_NVGAdjustUp(void)
{
	m_iAlpha += 10;

	if (m_iAlpha > 220)
		m_iAlpha = 220;
}

void CHudNightVision::UserCmd_NVGAdjustDown(void)
{
	m_iAlpha -= 10;

	if (m_iAlpha < 30)
		m_iAlpha = 30;
}