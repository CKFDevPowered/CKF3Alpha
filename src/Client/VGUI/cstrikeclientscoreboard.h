#ifndef CSTRIKECLIENTSCOREBOARDDIALOG_H
#define CSTRIKECLIENTSCOREBOARDDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\ClientScoreBoardDialog.h"

class CCSClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE(CCSClientScoreBoardDialog, CClientScoreBoardDialog);

public:
	CCSClientScoreBoardDialog(void);
	~CCSClientScoreBoardDialog(void);

public:
	virtual void Reset(void);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Update(void);
	virtual void PaintBackground(void);
	virtual void PaintBorder(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void ShowPanel(bool bShow);

public:
	void SetServerName(const char *hostname);
	void SetMapName(const char *mapname);
	void UpdatePlayerClass(int playerIndex, KeyValues *kv);

private:
	void InitPlayerList(vgui::SectionedListPanel *pPlayerList, int teamNumber);
	void UpdateTeamInfo(void);
	void UpdatePlayerList(void);
	void UpdateSpectatorList(void);
	void UpdateRoundTimer(void);
	bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	bool ShouldShowAsSpectator(int iPlayerIndex);

private:
	static bool CSPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

private:
	vgui::SectionedListPanel *m_pPlayerListT;
	vgui::SectionedListPanel *m_pPlayerListCT;

	vgui::Label *m_pPlayerCountLabel_T;
	vgui::Label *m_pScoreLabel_T;
	vgui::Label *m_pPingLabel_T;
	vgui::Label *m_pPlayerCountLabel_CT;
	vgui::Label *m_pScoreLabel_CT;
	vgui::Label *m_pPingLabel_CT;

	int m_iListWidth;
};

#endif