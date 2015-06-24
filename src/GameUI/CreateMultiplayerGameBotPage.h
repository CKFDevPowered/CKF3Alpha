#ifndef CREATEMULTIPLAYERGAMEBOTPAGE_H
#define CREATEMULTIPLAYERGAMEBOTPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CPanelListPanel;
class CDescription;
class mpcontrol_t;
class CCvarToggleCheckButton;

class CCreateMultiplayerGameBotPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CCreateMultiplayerGameBotPage, vgui::PropertyPage);

public:
	CCreateMultiplayerGameBotPage(vgui::Panel *parent, const char *name, KeyValues *botKeys);
	~CCreateMultiplayerGameBotPage(void);

public:
	char *GetBOTCommandBuffer(void);

protected:
	virtual void OnResetChanges(void);
	virtual void OnApplyChanges(void);

private:
	void SetJoinTeamCombo(const char *team);
	void SetChatterCombo(const char *team);

private:
	CCvarToggleCheckButton *m_joinAfterPlayer;
	CCvarToggleCheckButton *m_allowRogues;
	CCvarToggleCheckButton *m_allowPistols;
	CCvarToggleCheckButton *m_allowShotguns;
	CCvarToggleCheckButton *m_allowSubmachineGuns;
	CCvarToggleCheckButton *m_allowMachineGuns;
	CCvarToggleCheckButton *m_allowRifles;
	CCvarToggleCheckButton *m_allowGrenades;
	CCvarToggleCheckButton *m_allowSnipers;
	CCvarToggleCheckButton *m_deferToHuman;

	vgui::ComboBox *m_joinTeamCombo;
	vgui::ComboBox *m_chatterCombo;
	vgui::TextEntry *m_prefixEntry;
	vgui::TextEntry *m_profileEntry;
	KeyValues *m_pSavedData;
};

#endif