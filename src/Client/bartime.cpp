#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"

pfnUserMsgHook g_pfnMSG_BarTime;
pfnUserMsgHook g_pfnMSG_BarTime2;

DECLARE_MESSAGE(m_progressBar, BarTime)
DECLARE_MESSAGE(m_progressBar, BarTime2)

int CHudProgressBar::Init(void)
{
	g_pfnMSG_BarTime = HOOK_MESSAGE(BarTime);
	g_pfnMSG_BarTime2 = HOOK_MESSAGE(BarTime2);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudProgressBar::VidInit(void)
{
	return 1;
}

void CHudProgressBar::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
	m_tmEnd = 0;
	m_tmNewPeriod = 0;

	m_height = 10;
	m_x0 = m_width / 2;
	m_width = (ScreenWidth * 0.5);
	m_x1 = m_width + m_width / 2 - 1;
	m_y0 = (ScreenHeight * 0.5) + 64;
	m_y1 = m_y0 + 9;
}

int CHudProgressBar::Draw(float flTime)
{
	int slot, pos;

	if (m_tmNewPeriod)
	{
		m_tmEnd = m_tmNewPeriod + flTime - m_tmNewElapsed;
		m_tmStart = flTime - m_tmNewElapsed;
		m_tmNewPeriod = 0;
		m_tmNewElapsed = 0;
	}

	if (flTime > m_tmEnd)
		return 1;

	if (m_tmStart > flTime)
		return 1;

	int r, g, b;
	UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(m_x0, m_y0, 1, m_height, r, g, b, 80);
	FillRGBA(m_x1, m_y0, 1, m_height, r, g, b, 80);
	FillRGBA(m_x0 + 1, m_y0, m_width - 2, 1, r, g, b, 80);
	FillRGBA(m_x0 + 1, m_y1, m_width - 2, 1, r, g, b, 80);

	pos = m_x0 + 2;
	slot = (m_width - 4) * (flTime - m_tmStart) / (m_tmEnd - m_tmStart);

	if (slot >= m_width - 3)
		slot = m_width - 4;

	FillRGBA(pos, m_y0 + 2, slot, m_height - 4, r, g, b, 160);
	return 1;
}

int CHudProgressBar::MsgFunc_BarTime(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	float val = READ_SHORT();

	if (val)
		m_tmNewPeriod = val;
	else
		m_tmNewPeriod = -1;

	return 1;
}

int CHudProgressBar::MsgFunc_BarTime2(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int total = READ_SHORT();
	int elapsedPercent = READ_SHORT();

	if (total)
	{
		m_tmNewPeriod = total;
		m_tmNewElapsed = total * (elapsedPercent / 100.0);
	}
	else
	{
		m_tmNewPeriod = -1;
		m_tmNewElapsed = 0;
	}

	return 1;
}