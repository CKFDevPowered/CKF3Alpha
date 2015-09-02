#ifndef TFSCOREBOARDDIALOG_H
#define TFSCOREBOARDDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\ClientScoreBoardDialog.h"

namespace vgui
{
class ImagePanel;
class TFHudPanel;
};

class CTFScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE(CTFScoreBoardDialog, CClientScoreBoardDialog);

public:
	CTFScoreBoardDialog(void);
	~CTFScoreBoardDialog(void);

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
	void UpdatePlayerDetails(void);
	void ClearPlayerDetails(void);

private:
	static bool CSPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

private:
	vgui::SectionedListPanel *m_pRedPlayerList;
	vgui::SectionedListPanel *m_pBluePlayerList;

	vgui::Label *m_pPingLabel_T;
	vgui::Label *m_pPingLabel_CT;

	vgui::TFHudPanel *m_pMainPanel;
	vgui::TFHudPanel *m_pBlueTeamPanel;
	vgui::TFHudPanel *m_pRedTeamPanel;

	vgui::Label *m_pBlueTeamLabel;
	vgui::Label *m_pBlueTeamScoreLabel[2];
	vgui::Label *m_pBlueTeamPlayerCountLabel;
	vgui::Label *m_pRedTeamLabel;
	vgui::Label *m_pRedTeamScoreLabel[2];
	vgui::Label *m_pRedTeamPlayerCountLabel;
	vgui::Label *m_pSpectators;
	vgui::ImagePanel *m_pVerticalLine;
	vgui::ImagePanel *m_pShadedBar;
	vgui::ImagePanel *m_pClassImage;
	vgui::ImagePanel *m_pHorizontalLine;
	vgui::Label *m_pPlayerNameLabel;
	vgui::Label *m_pPlayerScoreLabel;
	vgui::EditablePanel *m_pLocalPlayerStatsPanel;
	vgui::Label *m_pServerLabel;

	int m_iImageDead;
	int m_iImageDominated;
	int m_iImageNemesis;
	int m_iImageClassIcon[9];
	int m_iImageDominates[16];

	vgui::IImage *m_pImageClassImage[2][9];
};

#endif