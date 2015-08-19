#ifndef BASEPANEL_H
#define BASEPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "vgui_controls/PHandle.h"
#include "vgui_controls/MenuItem.h"
#include "KeyValues.h"
#include "UtlVector.h"
#include "tier1/CommandBuffer.h"

class CGameMenu;
class CBackgroundMenuButton;
class CBackGroundPanel;

class CBasePanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CBasePanel, vgui::Panel);

public:
	CBasePanel(void);
	~CBasePanel(void);

public:
	void RunFrame(void);
	void SetMenuItemBlinkingState(const char *itemName, bool state);

public:
	void OnCommand(const char *command);
	void OnLevelLoadingStarted(const char *levelName);
	void OnLevelLoadingFinished(void);
	void OnGameUIActivated(void);
	void OnOpenServerBrowser(void);
	void OnOpenCreateMultiplayerGameDialog(void);
	void OnOpenQuitConfirmationDialog(void);
	void OnOpenOptionsDialog(void);
	void OnOpenDemoDialog(void);
	void OnSizeChanged(int newWide, int newTall);
	void OnGameUIHidden(void);

public:
	void PositionDialog(vgui::PHandle dlg);

protected:
	virtual void PaintBackground(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	enum EBackgroundState
	{
		BACKGROUND_INITIAL,
		BACKGROUND_LOADING,
		BACKGROUND_MAINMENU,
		BACKGROUND_LEVEL,
		BACKGROUND_DISCONNECTED,
		BACKGROUND_EXITING,
	};

	struct coord
	{
		int x;
		int y;
	};

	struct bimage_t
	{
		int imageID;
		int width, height;
	};

	enum { BACKGROUND_ROWS = 3, BACKGROUND_COLUMNS = 4 };

public:
	void SetBackgroundRenderState(EBackgroundState state);
	void UpdateBackgroundState(void);
	void SetMenuAlpha(int alpha);
	void CreateGameMenu(void);
	void UpdateGameMenus(void);
	CGameMenu *RecursiveLoadGameMenu(KeyValues *datafile);
	void StartExitingProcess(void);
	void DrawBackgroundImage(void);

private:
	virtual void PerformLayout(void);

private:
	MESSAGE_FUNC_INT(OnActivateModule, "ActivateModule", moduleIndex);

private:
	MESSAGE_FUNC_CHARPTR(RunEngineCommand, "RunEngineCommand", command);
	MESSAGE_FUNC_CHARPTR(RunMenuCommand, "RunMenuCommand", command);
	MESSAGE_FUNC(FinishDialogClose, "FinishDialogClose");

private:
	CBackgroundMenuButton *m_pGameMenuButton;
	CGameMenu *m_pGameMenu;
	int m_iGameMenuInset;
	vgui::DHANDLE<vgui::PropertyDialog> m_hOptionsDialog;
	vgui::DHANDLE<vgui::Frame> m_hCreateMultiplayerGameDialog;
	vgui::DHANDLE<vgui::QueryBox> m_hQuitQueryBox;
	EBackgroundState m_eBackgroundState;
	//bimage_t m_ImageID[BACKGROUND_ROWS][BACKGROUND_COLUMNS];
	bimage_t m_ImageID;
	int m_ExitingFrameCount;
	int m_iLoadingImageID;
	bool m_bLevelLoading;
	bool m_bEverActivated;
	bool m_bFadingInMenus;
	float m_flFadeMenuStartTime;
	float m_flFadeMenuEndTime;
	bool m_bRenderingBackgroundTransition;
	float m_flTransitionStartTime;
	float m_flTransitionEndTime;
	bool m_bHaveDarkenedBackground;
	bool m_bHaveDarkenedTitleText;
	bool m_bForceTitleTextUpdate;
	float m_flFrameFadeInTime;
	Color m_BackdropColor;

private:
	CPanelAnimationVar(float, m_flBackgroundFillAlpha, "m_flBackgroundFillAlpha", "0");
};

extern CBasePanel *BasePanel(void);

#endif
