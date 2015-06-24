#ifndef CSSPECTATORGUI_H
#define CSSPECTATORGUI_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\spectatorgui.h"

class CCSSpectatorGUI : public CSpectatorGUI
{
private:
	DECLARE_CLASS_SIMPLE(CCSSpectatorGUI, CSpectatorGUI);

public:
	CCSSpectatorGUI(void);

public:
	void Init(void) {}
	void VidInit(void) {}

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void UpdateSpectatorPlayerList(void);
	virtual void Update(void);
	virtual bool NeedsUpdate(void);

protected:
	void UpdateTimer(void);
	void UpdateAccount(void);

protected:
	int m_nLastAccount;
	int m_nLastTime;
	int m_nLastSpecMode;
	int m_nLastSpecTarget;

protected:
	int m_nTeamScoreT, m_nTeamScoreCT;

protected:
	void StoreWidths(void);
	void ResizeControls(void);
	bool ControlsPresent(void) const;

protected:
	vgui::Label *m_pCTLabel;
	vgui::Label *m_pCTScore;
	vgui::Label *m_pTerLabel;
	vgui::Label *m_pTerScore;
	vgui::Panel *m_pTimer;
	vgui::Label *m_pTimerLabel;
	vgui::Panel *m_pDivider;
	vgui::Label *m_pExtraInfo;

	int m_scoreWidth;
	int m_extraInfoWidth;
	int m_scoreOffset;

	int m_CTScoreX, m_TerScoreX;
	int m_CTLabelX, m_TerLabelX;
};

class CCSSpectatorMenu : public CSpectatorMenu
{
	DECLARE_CLASS_SIMPLE(CCSSpectatorMenu, CSpectatorMenu);

public:
	CCSSpectatorMenu(void);

public:
	void Init(void) {}
	void VidInit(void) {}
	void Reset(void);
};

#endif