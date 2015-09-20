#ifndef VGUI_HUD_TARGETID_H
#define VGUI_HUD_TARGETID_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <game_controls/TFHudPanel.h>
#include "hud_element.h"

#include "hud_playerstatus.h"

#define MAX_PREPEND_STRING		32

namespace vgui
{

class EditablePanel;
class ImagePanel;
class Label;
class TFHudPanel;

class CTFSpectatorGUIHealth : public CTFHudPlayerHealth
{
public:
	CTFSpectatorGUIHealth( Panel *parent, const char *name ) : CTFHudPlayerHealth( parent, name )
	{
		m_pBuildingBG = new ImagePanel(this, "BuildingStatusHealthImageBG");
	}

	virtual const char *GetResFileName( void ) 
	{ 
		return "resource/UI/SpectatorGUIHealth.res"; 
	}

	void SetBuilding(bool isBuilding)
	{
		if(m_pBuildingBG)
			m_pBuildingBG->SetVisible(isBuilding);
	}

	virtual void OnThink()
	{
		// Do nothing. We're just preventing the base health panel from updating.
	}

	ImagePanel *m_pBuildingBG;
};

class CTFHudTargetID : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudTargetID, EditablePanel );

public:
	CTFHudTargetID( const char *pPanelName );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual int	CalculateTargetIndex( void );
	virtual wchar_t	*GetPrepend( void ) { return NULL; }
	void UpdateID( void );
	int GetTargetIndex( void ) { return m_iTargetEntIndex; }

	virtual void PerformLayout( void );
	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	Label				*m_pTargetNameLabel;
	Label				*m_pTargetDataLabel;
	TFHudPanel			*m_pBGPanel;
	CTFSpectatorGUIHealth	*m_pTargetHealth;
	EditablePanel		*m_pMoveableSubPanel;
	ImagePanel			*m_pMoveableIconBG;
	ImagePanel			*m_pMoveableIcon;
	ImagePanel			*m_pMoveableSymbolIcon;
	Label				*m_pMoveableKeyLabel;

protected:
	int					m_nTeam;
	float				m_flLastChangeTime;
	int					m_iTargetEntIndex;
	int					m_iLastEntIndex;
	bool				m_bLayoutOnUpdate;
};

class CMainTargetID : public CTFHudTargetID
{
	DECLARE_CLASS_SIMPLE( CMainTargetID, CTFHudTargetID );
public:
	CMainTargetID( ) : CTFHudTargetID( "CMainTargetID" ) {}

	virtual bool ShouldDraw( void );
};

class CSpectatorTargetID : public CTFHudTargetID
{
	DECLARE_CLASS_SIMPLE( CSpectatorTargetID, CTFHudTargetID );
public:
	CSpectatorTargetID( ) : CTFHudTargetID( "CSpectatorTargetID" ) {}

	virtual bool ShouldDraw( void );
	virtual int	CalculateTargetIndex( void );
};

//-----------------------------------------------------------------------------
// Purpose: Second target ID that's used for displaying a second target below the primary
//-----------------------------------------------------------------------------
class CSecondaryTargetID : public CTFHudTargetID
{
	DECLARE_CLASS_SIMPLE( CSecondaryTargetID, CTFHudTargetID );
public:
	CSecondaryTargetID::CSecondaryTargetID( ) : CTFHudTargetID( "CSecondaryTargetID" )
	{
		m_wszPrepend[0] = '\0';
	}

	virtual int		CalculateTargetIndex( void );
	virtual wchar_t	*GetPrepend( void ) { return m_wszPrepend; }

private:
	wchar_t		m_wszPrepend[ MAX_PREPEND_STRING ];
};

}

#endif