#ifndef NAVPROGRESS_H
#define NAVPROGRESS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ProgressBar.h>

#define PANEL_NAV_PROGRESS "nav_progress"

class CNavProgress : public vgui::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CNavProgress, vgui::Frame);

public:
	CNavProgress(void);
	virtual ~CNavProgress(void);

public:
	virtual const char *GetName(void) { return PANEL_NAV_PROGRESS; }
	virtual void SetData(KeyValues *data);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout(void);

public:
	void Init(const char *title, int numTicks, int currentTick);
	void Update(const char *statusText, int tick);

protected:
	int m_numTicks;
	int m_currentTick;

	vgui::Label *m_pTitle;
	vgui::Label *m_pText;
	vgui::Panel *m_pProgressBarBorder;
	vgui::Panel *m_pProgressBar;
	vgui::Panel *m_pProgressBarSizer;
};

#endif