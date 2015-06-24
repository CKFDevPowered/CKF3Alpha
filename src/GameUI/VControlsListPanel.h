#if !defined (VCONTROLSLISTPANEL_H)
#define VCONTROLSLISTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/SectionedListPanel.h>

class VControlsListPanel : public vgui::SectionedListPanel
{
public:
	VControlsListPanel(vgui::Panel *parent, const char *listName);
	virtual ~VControlsListPanel(void);

public:
	virtual void StartCaptureMode(vgui::HCursor hCursor = NULL);
	virtual void EndCaptureMode(vgui::HCursor hCursor = NULL);
	virtual bool IsCapturing(void);
	virtual void SetItemOfInterest(int itemID);
	virtual int GetItemOfInterest(void);
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseDoublePressed(vgui::MouseCode code);

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	class CInlineEditPanel *m_pInlineEditPanel;

private:
	bool m_bCaptureMode;
	int m_nClickRow;
	vgui::HFont m_hFont;
	int m_iMouseX, m_iMouseY;
	typedef vgui::SectionedListPanel BaseClass;
};

#endif