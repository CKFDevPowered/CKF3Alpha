#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"

#include <vgui/vgui.h>
#include <vgui/ISurface.h>
#include <vgui/ilocalize.h>
#include <KeyValues.h>

#include "hud_menu.h"

#include "client.h"
#include "CounterStrikeViewport.h"
#include "chatdialog.h"

#define MAX_MENU_STRING 1024
wchar_t g_szMenuString2[MAX_MENU_STRING];
wchar_t g_szPrelocalisedMenuString2[MAX_MENU_STRING];

wchar_t *ReadLocalizedString(wchar_t *pOut, int outSize, bool bStripNewline, char *originalString = NULL, int originalSize = 0);

namespace vgui
{
CHudMenu::CHudMenu(void) : BaseClass(NULL, "HudMenu")
{
	m_nSelectedItem = -1;
}

void CHudMenu::Init(void)
{
	m_bMenuTakesInput = false;
	m_bMenuDisplayed = false;
	m_bitsValidSlots = 0;
	m_Processed.RemoveAll();
	m_nMaxPixels = 0;
	m_nHeight = 0;

	m_flOpenCloseTime = 1;
	m_flBlur = 0;
	m_flTextScan = 1;
	m_flAlphaOverride = 255;
	m_flSelectionAlphaOverride = 255;

	m_MenuColor = Color(255, 255, 255, 255);
	m_ItemColor = Color(255, 255, 255, 255);
	m_BoxColor = Color(0, 0, 0, 128);

	m_hTextFont = vgui::INVALID_FONT;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	Reset();
	SetVisible(true);
}

void CHudMenu::Reset(void)
{
	g_szPrelocalisedMenuString2[0] = 0;
	m_fWaitingForMore = false;
}

bool CHudMenu::IsMenuOpen(void)
{
	return m_bMenuDisplayed && m_bMenuTakesInput;
}

void CHudMenu::VidInit(void)
{
}

void CHudMenu::Think(void)
{
	if (m_flShutoffTime > 0 && m_flShutoffTime <= g_pViewPort->GetCurrentTime())
		m_bMenuDisplayed = false;
}

void CHudMenu::PaintString(const wchar_t *text, int textlen, vgui::HFont &font, int x, int y)
{
	vgui::surface()->DrawSetTextFont(font);
	vgui::surface()->DrawSetTextPos(x, y);

	bool hit = false;

	for (int ch = 0; ch < textlen; ch++)
	{
		if (text[ch] == '\\')
		{
			hit = true;
			continue;
		}

		if (hit)
		{
			if (text[ch] == 'r')
			{
				vgui::surface()->DrawSetTextColor(210, 24, 0, m_flSelectionAlphaOverride);
			}
			else if (text[ch] == 'd')
			{
				vgui::surface()->DrawSetTextColor(100, 100, 100, m_flSelectionAlphaOverride);
			}
			else if (text[ch] == 'y')
			{
				vgui::surface()->DrawSetTextColor(255, 210, 64, m_flSelectionAlphaOverride);
			}
			else if (text[ch] == 'w')
			{
				vgui::surface()->DrawSetTextColor(255, 255, 255, m_flSelectionAlphaOverride);
			}
			else if (text[ch] == 'R')
			{
				int border = 20;
				int wide = m_nMaxPixels + border;

				x = wide;

				for (int j = textlen; j >= 0; j--)
				{
					if (text[j] == 'r')
					{
						if (j > 0 && text[j - 1] == '\\')
						{
							vgui::surface()->DrawSetTextColor(210, 24, 0, m_flSelectionAlphaOverride);
							j--;
							continue;
						}
					}
					else if (text[j] == 'd')
					{
						if (j > 0 && text[j - 1] == '\\')
						{
							vgui::surface()->DrawSetTextColor(100, 100, 100, m_flSelectionAlphaOverride);
							j--;
							continue;
						}
					}
					else if (text[j] == 'y')
					{
						if (j > 0 && text[j - 1] == '\\')
						{
							vgui::surface()->DrawSetTextColor(255, 210, 64, m_flSelectionAlphaOverride);
							j--;
							continue;
						}
					}
					else if (text[j] == 'w')
					{
						if (j > 0 && text[j - 1] == '\\')
						{
							vgui::surface()->DrawSetTextColor(255, 255, 255, m_flSelectionAlphaOverride);
							j--;
							continue;
						}
					}
					else if (text[j] == 'R')
					{
						if (j > 0 && text[j - 1] == '\\')
							break;
					}

					if (text[j] == 0x9 || text[j] == 0xA || text[j] == 0xD)
						continue;

					x -= vgui::surface()->GetCharacterWidth(font, text[j]);

					vgui::surface()->DrawSetTextPos(x, y);
					vgui::surface()->DrawUnicodeChar(text[j]);
				}

				break;
			}

			hit = false;
			continue;
		}

		if (text[ch] == 0x9 || text[ch] == 0xA || text[ch] == 0xD)
			continue;

		vgui::surface()->DrawUnicodeChar(text[ch]);
	}
}

void CHudMenu::Paint(void)
{
	if (!gHUD.m_pCvarDraw->value)
		return;

	if (g_pViewPort->IsScoreBoardVisible())
		return;

	if (!m_bMenuDisplayed)
		return;

	int x = 20;

	Color menuColor = m_MenuColor;
	Color itemColor = m_ItemColor;

	int c = m_Processed.Count();
	int border = 20;
	int wide = m_nMaxPixels + border;
	int tall = m_nHeight + border;

	int y = (ScreenHeight - tall) * 0.5f;

	if (cl_newmenu_drawbox->value)
		DrawBox(x - border / 2, y - border / 2, wide, tall, m_BoxColor, m_flSelectionAlphaOverride / 255.0f);

	vgui::surface()->DrawSetTextColor(255, 255, 255, m_flSelectionAlphaOverride);

	for (int i = 0; i < c; i++)
	{
		ProcessedLine *line = &m_Processed[i];
		Assert(line);

		bool canblur = false;

		if (line->menuitem != 0 && m_nSelectedItem >= 0 && (line->menuitem == m_nSelectedItem))
			canblur = true;

		int drawLen = line->length;

		if (line->menuitem != 0)
			drawLen *= m_flTextScan;

		vgui::surface()->DrawSetTextColor(255, 255, 255, m_flSelectionAlphaOverride);
		vgui::surface()->DrawSetTextFont(line->menuitem != 0 ? m_hItemFont : m_hTextFont);
		PaintString(&g_szMenuString2[line->startchar], drawLen, line->menuitem != 0 ? m_hItemFont : m_hTextFont, x, y);

		if (canblur)
		{
		}

		y += line->height;
	}
}

bool CHudMenu::SelectMenuItem(int menu_item)
{
	char szbuf[32];

	if ((menu_item > 0) && (m_bitsValidSlots & (1 << (menu_item - 1))))
	{
		Q_snprintf(szbuf, sizeof(szbuf), "menuselect %d\n", menu_item);
		engine->pfnClientCmd(szbuf);

		m_nSelectedItem = menu_item;
		m_bMenuTakesInput = false;
		m_flShutoffTime = g_pViewPort->GetCurrentTime() + 0.05;
		return true;
	}

	return false;
}

void CHudMenu::ProcessText(void)
{
	if (m_hTextFont == vgui::INVALID_FONT)
		return;

	m_Processed.RemoveAll();
	m_nMaxPixels = 0;
	m_nHeight = 0;

	int i = 0;
	int startpos = i;
	int menuitem = 0;

	while (i < MAX_MENU_STRING)
	{
		wchar_t ch = g_szMenuString2[i];

		if (ch == 0)
			break;

		if (i == startpos && (ch == L'-' && g_szMenuString2[i + 1] == L'>'))
		{
			swscanf(&g_szMenuString2[i + 2], L"%d", &menuitem);
			i += 2;
			startpos += 2;
			continue;
		}

		while (i < MAX_MENU_STRING && g_szMenuString2[i] != 0 && g_szMenuString2[i] != L'\n')
			i++;

		ProcessedLine line;
		line.menuitem = menuitem;
		line.startchar = startpos;
		line.length = i - startpos;
		line.pixels = 0;
		line.height = 0;

		m_Processed.AddToTail(line);

		menuitem = 0;

		if (g_szMenuString2[i] == '\n')
			i++;

		startpos = i;
	}

	if (i - startpos >= 1)
	{
		ProcessedLine line;
		line.menuitem = menuitem;
		line.startchar = startpos;
		line.length = i - startpos;
		line.pixels = 0;
		line.height = 0;

		m_Processed.AddToTail(line);
	}

	int c = m_Processed.Count();

	for (i = 0; i < c; i++)
	{
		ProcessedLine *l = &m_Processed[i];
		Assert(l);

		int pixels = 0;
		vgui::HFont font = l->menuitem != 0 ? m_hItemFont : m_hTextFont;

		for (int ch = 0; ch < l->length; ch++)
			pixels += vgui::surface()->GetCharacterWidth(font, g_szMenuString2[ch + l->startchar]);

		l->pixels = pixels;
		l->height = vgui::surface()->GetFontTall(font);

		if (pixels > m_nMaxPixels)
			m_nMaxPixels = pixels;

		m_nHeight += l->height;
	}
}

void CHudMenu::HideMenu(void)
{
	m_bMenuTakesInput = false;
	m_flShutoffTime = g_pViewPort->GetCurrentTime() + m_flOpenCloseTime;
}

void CHudMenu::ShowMenu(const wchar_t *menuName, int validSlots)
{
	m_flShutoffTime = -1;
	m_bitsValidSlots = validSlots;
	m_fWaitingForMore = 0;

	wcscpy_s(g_szPrelocalisedMenuString2, sizeof(g_szPrelocalisedMenuString2) / sizeof(wchar_t), menuName);

	m_nSelectedItem = -1;
	wcsncpy(g_szMenuString2, g_szPrelocalisedMenuString2, sizeof(g_szMenuString2) / sizeof(wchar_t));

	ProcessText();

	m_bMenuDisplayed = true;
	m_bMenuTakesInput = true;
}

int CHudMenu::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	if (!strcmp(pszName, "ShowMenu"))
	{
		int bitsValidSlots = READ_SHORT();
		int DisplayTime = READ_CHAR();
		int NeedMore = READ_BYTE();

		if (DisplayTime > 0)
			m_flShutoffTime = m_flOpenCloseTime + DisplayTime + g_pViewPort->GetCurrentTime();
		else
			m_flShutoffTime = -1;

		if (bitsValidSlots)
		{
			wchar_t szString[2048];
			ReadLocalizedString(szString, sizeof(szString), true);

			if (!m_fWaitingForMore)
				wcsncpy(g_szPrelocalisedMenuString2, szString, sizeof(g_szPrelocalisedMenuString2) / sizeof(wchar_t));
			else
				wcsncat(g_szPrelocalisedMenuString2, szString, sizeof(g_szPrelocalisedMenuString2) / sizeof(wchar_t));

			if (!NeedMore)
			{
				m_nSelectedItem = -1;
				wcsncpy(g_szMenuString2, g_szPrelocalisedMenuString2, sizeof(g_szMenuString2) / sizeof(wchar_t));

				ProcessText();
			}

			m_bMenuDisplayed = true;
			m_bMenuTakesInput = true;
		}
		else
		{
			HideMenu();
		}

		m_bitsValidSlots = bitsValidSlots;
		m_fWaitingForMore = NeedMore;
		return 1;
	}

	return 0;
}

void CHudMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	/*int x, y;
	GetPos(x, y);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, y, screenWide, screenTall - y);

	m_hTextFont = pScheme->GetFont("CreditsText");
	m_hItemFont = pScheme->GetFont("CreditsText");
	m_hItemFontPulsing = pScheme->GetFont("CreditsText");

	ProcessText();
	SetPaintBackgroundEnabled(false);*/
}
}