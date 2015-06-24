#ifndef VGUI_HUD_MENU_H
#define VGUI_HUD_MENU_H

#include "hud_element.h"
#include "UtlVector.h"
#include "vgui_controls/Panel.h"

namespace vgui
{
class CHudMenu : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudMenu, Panel);

public:
	CHudMenu(void);

public:
	void Init(void);
	void VidInit(void);
	void Think(void);
	void Reset(void);
	int FireMessage(const char *pszName, int iSize, void *pbuf);
	void HideMenu(void);
	void ShowMenu(const wchar_t *menuName, int keySlot);
	bool IsMenuOpen(void);
	bool SelectMenuItem(int menu_item);

public:
	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void ProcessText(void);
	void PaintString(const wchar_t *text, int textlen, vgui::HFont &font, int x, int y);

private:
	struct ProcessedLine
	{
		int menuitem;
		int startchar;
		int length;
		int pixels;
		int height;
	};

	CUtlVector<ProcessedLine> m_Processed;

	int m_nMaxPixels;
	int m_nHeight;

	bool m_bMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
	int m_nSelectedItem;
	bool m_bMenuTakesInput;

private:
	float m_flOpenCloseTime;
	float m_flBlur;
	float m_flTextScan;
	float m_flAlphaOverride;
	float m_flSelectionAlphaOverride;
	vgui::HFont m_hTextFont;
	vgui::HFont m_hItemFont;
	vgui::HFont m_hItemFontPulsing;
	Color m_MenuColor;
	Color m_ItemColor;
	Color m_BoxColor;
};
}

#endif