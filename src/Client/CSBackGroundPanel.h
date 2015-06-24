#include "BackGroundPanel.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/BitmapImagePanel.h>

class CCSBackGroundPanel : public CBackGroundPanel
{
	typedef CBackGroundPanel BaseClass;

public:
	CCSBackGroundPanel(vgui::Panel *parent);

public:
	void SetTitleText(const wchar_t *text);
	void SetTitleText(const char *text);

public:
	void PaintBackground(void);
	void Activate(void);
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void PerformLayout(void);

private:
	vgui::CBitmapImagePanel *m_pTopLeftPanel;
	vgui::CBitmapImagePanel *m_pTopRightPanel;
	vgui::CBitmapImagePanel *m_pBottomLeftPanel;
	vgui::CBitmapImagePanel *m_pBottomRightPanel;
	vgui::Label *m_pTitleLabel;
	vgui::Panel *m_pGapPanel;
	vgui::CBitmapImagePanel *m_pExclamationPanel;
	Color m_bgColor;
	Color m_titleColor;
	Color m_borderColor;
	int m_offsetX;
	int m_offsetY;
	bool m_enabled;
};