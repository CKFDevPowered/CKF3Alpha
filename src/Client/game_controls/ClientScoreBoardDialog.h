#ifndef CLIENTSCOREBOARDDIALOG_H
#define CLIENTSCOREBOARDDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#define PANEL_SCOREBOARD "scores"

#include <vgui_controls/EditablePanel.h>
#include <tier1/UtlMap.h>

#define TYPE_NOTEAM 0
#define TYPE_TEAM 1
#define TYPE_PLAYERS 2
#define TYPE_SPECTATORS 3
#define TYPE_BLANK 4

class CClientScoreBoardDialog : public vgui::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CClientScoreBoardDialog, vgui::Frame);

protected:
	enum { NAME_WIDTH = 160, SCORE_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0 };

public:
	CClientScoreBoardDialog(void);
	~CClientScoreBoardDialog(void);

public:
	virtual const char *GetName(void) { return PANEL_SCOREBOARD; }
	virtual void SetData(KeyValues *data) {}
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual bool NeedsUpdate(void);
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);
	virtual bool ShowAvatars(void) { return false; }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void SetServerName(const char *hostname);
	virtual void UpdatePlayerAvatar(int playerIndex, KeyValues *kv);
	virtual void UpdatePlayerClass(int playerIndex, KeyValues *kv);

protected:
	MESSAGE_FUNC_INT(OnPollHideCode, "PollHideCode", code);

protected:
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void InitScoreboardSections(void);
	virtual void UpdateTeamInfo(void);
	virtual void UpdatePlayerInfo(void);
	virtual void OnThink(void);
	virtual void AddHeader(void);
	virtual void AddSection(int teamType, int teamNumber);
	virtual int GetAdditionalHeight(void) { return 0; }

protected:
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PostApplySchemeSettings(vgui::IScheme *pScheme);

protected:
	int FindItemIDForPlayerIndex(int playerIndex);
	void MoveLabelToFront(const char *textEntryName);
	void MoveToCenterOfScreen(void);

protected:
	int m_iNumTeams;

	vgui::SectionedListPanel *m_pPlayerList;
	int m_iSectionId;

	int s_VoiceImage[5];
	int TrackerImage;
	int m_HLTVSpectators;
	float m_fNextUpdateTime;

	vgui::ImageList *m_pImageList;
	int m_iImageAvatars[32 + 1];
	CUtlMap<int, int> m_mapAvatarsToImageList;

protected:
	CPanelAnimationVarAliasType(int, m_iAvatarWidth, "avatar_width", "34", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iNameWidth, "name_width", "136", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iClassWidth, "class_width", "35", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iScoreWidth, "score_width", "35", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iDeathWidth, "death_width", "35", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iPingWidth, "ping_width", "23", "proportional_int");

private:
	int m_iPlayerIndexSymbol;
	int m_iDesiredHeight;
	vgui::KeyCode m_nCloseKey;

private:
	void FillScoreBoard(void);
};

#endif