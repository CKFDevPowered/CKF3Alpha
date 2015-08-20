#ifndef TFTEAMMENU_H
#define TFTEAMMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <game_controls/teammenu.h>
#include <game_controls/ModelPanel.h>

namespace vgui
{
	class Label;
	class Button;
	class ImagePanel;
	class ModelPanel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class TFTeamButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( TFTeamButton, vgui::Button );

public:
	TFTeamButton( vgui::Panel *parent, const char *panelName );

	void ApplySettings( KeyValues *inResourceData );
	void ApplySchemeSettings( vgui::IScheme *pScheme );

	void OnCursorExited();
	void OnCursorEntered();

	MESSAGE_FUNC(OnTick, "Tick");

private:
	bool IsDisabled();
	void SendAnimation( const char * pszAnimation, bool reverse = false );
	void SetMouseEnteredState( bool state );

private:
	char	m_szModelPanel[64];		// the panel we'll send messages to
	int		m_iTeam;				// the team we're associated with (if any)

	float	m_flHoverTimeToWait;	// length of time to wait before reporting a "hover" message (-1 = no hover)
	float	m_flHoverTime;			// when should a "hover" message be sent?
	bool	m_bMouseEntered;		// used to track when the mouse is over a button
	bool	m_bTeamDisabled;
};

class CTFTeamMenu : public CTeamMenu
{
private:
	DECLARE_CLASS_SIMPLE(CTFTeamMenu, CTeamMenu);

public:
	CTFTeamMenu(void);

public:
	void Update(void);
	bool NeedsUpdate(void) { return true; }
	void ShowPanel(bool bShow);
	void SetVisible(bool state);
	bool IsTeamDisabled(int iTeam);
	void OnKeyCodePressed( vgui::KeyCode code );

private:
	enum { NUM_TEAMS = 3 };

private:
	void OnCommand(const char *command);

private:

public:
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	void SetVisibleButton(const char *textEntryName, bool state);

	TFTeamButton	*m_pBlueTeamButton;
	TFTeamButton	*m_pRedTeamButton;
	TFTeamButton	*m_pAutoTeamButton;
	TFTeamButton	*m_pSpecTeamButton;

	vgui::ImagePanel		*m_pBlueDisableImage;
	vgui::ImagePanel		*m_pRedDisableImage;
	vgui::ImagePanel		*m_pSpecDisableImage;

	vgui::ModelPanel		*m_pModelPanel[4];
	vgui::Label			*m_pSpecLabel;
	vgui::Button		*m_pCancelButton;

	vgui::KeyCode	m_iTeamMenuKey;
};

#endif