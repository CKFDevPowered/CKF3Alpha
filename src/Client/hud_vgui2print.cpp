#include "hud.h"
#include "cl_util.h"

int CHudVGUI2Print::Init(void)
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;
	m_hudfont = 0;
	return 1;
}

int CHudVGUI2Print::VidInit(void)
{
	m_flVGUI2StringTime = 0;
	m_hudfont = vgui::scheme()->GetIScheme(vgui::scheme()->GetDefaultScheme())->GetFont("EngineFont", true);
	return 1;
}

int CHudVGUI2Print::Draw(float flTime)
{
	if (m_flVGUI2StringTime > gEngfuncs.GetClientTime())
		DrawVGUI2String(m_wCharBuf, m_iX, m_iY, m_fR, m_fG, m_fB);

	return 1;
}

int CHudVGUI2Print::DrawVGUI2StringReverse(wchar_t *msg, int x, int y, float r, float g, float b)
{
	int len;
	int i;
	int w1, w2, w3;
	int fontheight;

	if (!m_hudfont)
		return 0;

	len = wcslen(msg);

	if (y == -1)
	{
		fontheight = vgui::surface()->GetFontTall(m_hudfont);
		y = (ScreenHeight - fontheight) / 2;
	}

	gEngfuncs.pfnDrawSetTextColor(r, g, b);

	for (i = len - 1; i >= 0; i--)
	{
		gEngfuncs.pfnVGUI2DrawCharacter(x, y, msg[i], m_hudfont);
		vgui::surface()->GetCharABCwide(m_hudfont, msg[i], w1, w2, w3);

		x -= w1 + w2 + w3;
	}

	return 1;
}

int CHudVGUI2Print::DrawVGUI2StringReverse(char *charMsg, int x, int y, float r, float g, float b)
{
	wchar_t *msg = vgui::localize()->Find(charMsg);

	if (!msg)
	{
		wchar_t buf[512];
		vgui::localize()->ConvertANSIToUnicode(charMsg, buf, sizeof(buf));
		msg = buf;
	}

	return DrawVGUI2StringReverse(msg, x, y, r, g, b);
}

void CHudVGUI2Print::VGUI2LocalizeArg(char *pSrc, wchar_t *pDest, int unicodeBufferSizeInBytes)
{
	if (!pSrc || !pDest)
		return;

	if (pSrc[0] == '#')
	{
		wchar_t *temp = vgui::localize()->Find(pSrc);

		if (temp)
		{
			int iCount = (unicodeBufferSizeInBytes / sizeof(wchar_t)) - 1;

			wcsncpy(pDest, temp, iCount);
			pDest[iCount] = 0;
			return;
		}
	}

	vgui::localize()->ConvertANSIToUnicode(pSrc, pDest, unicodeBufferSizeInBytes);
}

void CHudVGUI2Print::VGUI2HudPrintArgs(char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b)
{
	wchar_t *msg;
	wchar_t wstr1[64];
	wchar_t wstr2[64];
	wchar_t wstr3[64];
	wchar_t wstr4[64];
	float flCenterTime;

	VGUI2LocalizeArg(sstr1, wstr1, sizeof(wstr1));
	VGUI2LocalizeArg(sstr2, wstr2, sizeof(wstr2));
	VGUI2LocalizeArg(sstr3, wstr3, sizeof(wstr3));
	VGUI2LocalizeArg(sstr4, wstr4, sizeof(wstr4));

	m_iX = x;
	m_iY = y;
	m_fR = r;
	m_fG = g;
	m_fB = b;

	flCenterTime = gEngfuncs.pfnGetCvarFloat("scr_centertime");

	m_flVGUI2StringTime = gEngfuncs.GetClientTime() + flCenterTime;

	msg = vgui::localize()->Find(charMsg);

	if (msg)
	{
		vgui::localize()->ConstructString(m_wCharBuf, sizeof(m_wCharBuf), msg, 4, wstr1, wstr2, wstr3, wstr4);
	}
	else
	{
		vgui::localize()->ConvertANSIToUnicode(charMsg, m_wCharBuf, sizeof(m_wCharBuf));
	}
}

void CHudVGUI2Print::VGUI2HudPrint(char *charMsg, int x, int y, float r, float g, float b)
{
	wchar_t *temp = vgui::localize()->Find(charMsg);

	if (temp)
	{
		int iCount = (sizeof(m_wCharBuf) / sizeof(wchar_t)) - 1;

		wcsncpy(m_wCharBuf, temp, iCount);
		m_wCharBuf[iCount] = 0;

		m_iX = x;
		m_iY = y;
		m_fR = r;
		m_fG = g;
		m_fB = b;

		m_flVGUI2StringTime = gEngfuncs.GetClientTime() + 4;
	}
}

int CHudVGUI2Print::GetHudFontHeight(void)
{
	if (!m_hudfont)
		return 0;

	return vgui::surface()->GetFontTall(m_hudfont);
}

void CHudVGUI2Print::GetStringSize(const wchar_t *string, int *width, int *height)
{
	int i;
	int len;
	int a, b, c;

	if (width)
		*width = 0;

	if (height)
		*height = 0;

	if (!m_hudfont)
		return;

	len = wcslen(string);

	if (width)
	{
		for (i = 0; i < len; i++)
		{
			vgui::surface()->GetCharABCwide(m_hudfont, string[i], a, b, c);
			*width += a + b + c;
		}
	}

	if (height)
	{
		*height = GetHudFontHeight();
	}
}

int CHudVGUI2Print::DrawVGUI2String(wchar_t *msg, int x, int y, float r, float g, float b)
{
	int i;
	int iOriginalX;
	int iTotalLines;
	int iCurrentLine;
	int w1, w2, w3;
	bool bHorzCenter;
	int len;
	wchar_t *strTemp;
	int fontheight;

	if (!m_hudfont)
		return 0;

	iCurrentLine = 0;
	iOriginalX = x;
	iTotalLines = 1;
	bHorzCenter = false;
	len = wcslen(msg);

	for (strTemp = msg; *strTemp; strTemp++)
	{
		if (*strTemp == '\r')
			iTotalLines++;
	}

	if (x == -1)
	{
		bHorzCenter = true;
	}

	if (y == -1)
	{
		fontheight = vgui::surface()->GetFontTall(m_hudfont);
		y = (ScreenHeight - fontheight) / 2;
	}

	for (i = 0; i < iTotalLines; i++)
	{
		wchar_t line[1024];
		int iWidth;
		int iHeight;
		int iTempCount;
		int j;
		int shadow_x;

		iTempCount = 0;
		iWidth = 0;
		iHeight = 0;

		for (strTemp = &msg[iCurrentLine]; *strTemp; strTemp++, iCurrentLine++)
		{
			if (*strTemp == '\r')
				break;

			if (*strTemp != '\n')
				line[iTempCount++] = *strTemp;
		}

		line[iTempCount] = 0;

		GetStringSize(line, &iWidth, &iHeight);

		if (bHorzCenter)
			x = (ScreenWidth - iWidth) / 2;
		else
			x = iOriginalX;

		gEngfuncs.pfnDrawSetTextColor(0, 0, 0);

		shadow_x = x;

		for (j = 0; j < iTempCount; j++)
		{
			gEngfuncs.pfnVGUI2DrawCharacter(shadow_x, y, line[j], m_hudfont);
			vgui::surface()->GetCharABCwide(m_hudfont, line[j], w1, w2, w3);

			shadow_x += w1 + w2 + w3;
		}

		gEngfuncs.pfnDrawSetTextColor(r, g, b);

		for (j = 0; j < iTempCount; j++)
		{
			gEngfuncs.pfnVGUI2DrawCharacter(x, y, line[j], m_hudfont);
			vgui::surface()->GetCharABCwide(m_hudfont, line[j], w1, w2, w3);

			x += w1 + w2 + w3;
		}

		y += iHeight;
		iCurrentLine++;
	}

	return x;
}

int CHudVGUI2Print::DrawVGUI2String(char *charMsg, int x, int y, float r, float g, float b)
{
	wchar_t *msg = vgui::localize()->Find(charMsg);

	if (!msg)
	{
		wchar_t buf[512];
		vgui::localize()->ConvertANSIToUnicode(charMsg, buf, sizeof(buf));
		msg = buf;
	}

	return DrawVGUI2String(msg, x, y, r, g, b);
}