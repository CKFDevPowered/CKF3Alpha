#include "hud.h"
#include "tfteammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
TFTeamButton::TFTeamButton( vgui::Panel *parent, const char *panelName ) : Button( parent, panelName, "" )
{
	m_szModelPanel[0] = '\0';
	m_iTeam = TEAM_UNASSIGNED;
	m_flHoverTimeToWait = -1;
	m_flHoverTime = -1;
	m_bMouseEntered = false;
	m_bTeamDisabled = false;

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szModelPanel, inResourceData->GetString( "associated_model", "" ), sizeof( m_szModelPanel ) );
	m_iTeam = inResourceData->GetInt( "team", TEAM_UNASSIGNED );
	m_flHoverTimeToWait = inResourceData->GetFloat( "hover", -1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetDefaultColor( GetFgColor(), Color( 0, 0, 0, 0 ) );
	SetArmedColor( GetButtonFgColor(), Color( 0, 0, 0, 0 ) );
	SetDepressedColor( GetButtonFgColor(), Color( 0, 0, 0, 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::SendAnimation( const char *pszAnimation )
{
	Panel *pParent = GetParent();
	if ( pParent )
	{
		ModelPanel *pModel = dynamic_cast< ModelPanel* >(pParent->FindChildByName( m_szModelPanel ));
		if ( pModel )
		{
			pModel->SetAnimation(pszAnimation);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool TFTeamButton::IsDisabled()
{
	bool bRetVal = false;

	if ( ( m_iTeam > TEAM_UNASSIGNED ) && GetParent() )
	{
		CTFTeamMenu *pTeamMenu = dynamic_cast< CTFTeamMenu* >( GetParent() );
		if ( pTeamMenu )
		{
			bRetVal = pTeamMenu->IsTeamDisabled(m_iTeam);
		}
	}

	return bRetVal;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	SetMouseEnteredState( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	SetMouseEnteredState( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::SetMouseEnteredState( bool state )
{
	if ( state )
	{
		m_bMouseEntered = true;

		if ( m_flHoverTimeToWait > 0 )
		{
			m_flHoverTime = gEngfuncs.GetClientTime() + m_flHoverTimeToWait;
		}
		else
		{
			m_flHoverTime = -1;
		}

		if ( m_bTeamDisabled )
		{
			SendAnimation( "enter_disabled" );
		}
		else
		{
			SendAnimation( "enter_enabled" );
		}
	}
	else
	{
		m_bMouseEntered = false;
		m_flHoverTime = -1;

		if ( m_bTeamDisabled )
		{
			SendAnimation( "exit_disabled" );
		}
		else
		{
			SendAnimation( "exit_enabled" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFTeamButton::OnTick()
{
	// check to see if our state has changed
	bool bDisabled = IsDisabled();

	if ( bDisabled != m_bTeamDisabled )
	{
		m_bTeamDisabled = bDisabled;

		if ( m_bMouseEntered )
		{
			// something has changed, so reset our state
			SetMouseEnteredState( true );
		}
		else
		{
			// the mouse isn't currently over the button, but we should update the status
			if ( m_bTeamDisabled )
			{
				SendAnimation( "idle_disabled" );
			}
			else
			{
				SendAnimation( "idle_enabled" );
			}
		}
	}

	if ( ( m_flHoverTime > 0 ) && ( m_flHoverTime < gEngfuncs.GetClientTime() ) )
	{
		m_flHoverTime = -1;

		if ( m_bTeamDisabled )
		{
			SendAnimation( "hover_disabled" );
		}
		else
		{
			SendAnimation( "hover_enabled" );
		}
	}
}

CTFTeamMenu::CTFTeamMenu(void) : CTeamMenu()
{
	m_pBlueTeamButton = new TFTeamButton( this, "teambutton0" );
	m_pRedTeamButton = new TFTeamButton( this, "teambutton1" );
	m_pAutoTeamButton = new TFTeamButton( this, "teambutton2" );
	m_pSpecTeamButton = new TFTeamButton( this, "teambutton3" );
	m_pSpecLabel = new Label( this, "TeamMenuSpectate", "" );

	m_pCancelButton = new Button( this, "CancelButton", "#CKF3_Cancel" );

	m_pModelPanel[0] = new ModelPanel( this, "autodoor");
	m_pModelPanel[1] = new ModelPanel( this, "bluedoor");
	m_pModelPanel[2] = new ModelPanel( this, "reddoor");
	m_pSpectateImage = new ImagePanel( this, "spectate");

	m_bRedDisabled = false;
	m_bBlueDisabled = false;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	LoadControlSettings("Resource/UI/TFTeamMenu.res");
	InvalidateLayout();
}

CTFTeamMenu::~CTFTeamMenu(void)
{
}

bool CTFTeamMenu::IsTeamDisabled(int iTeam)
{
	if(iTeam == TEAM_RED && g_iTeamNumber == TEAM_RED)
	{
		return true;
	}
	if(iTeam == TEAM_BLUE && g_iTeamNumber == TEAM_BLUE)
	{
		return true;
	}
	return false;
}

void CTFTeamMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;

		m_pModelPanel[0]->SetModel();
		m_pModelPanel[1]->SetModel();
		m_pModelPanel[2]->SetModel();
	}

	BaseClass::ShowPanel(bShow);
}

void CTFTeamMenu::Update(void)
{
	if ( g_iTeamNumber != TEAM_UNASSIGNED )
	{
		if ( m_pCancelButton )
		{
			m_pCancelButton->SetVisible( true );
		}
	}
	else
	{
		if ( m_pCancelButton && m_pCancelButton->IsVisible() )
		{
			m_pCancelButton->SetVisible( false );
		}
	}
}

void CTFTeamMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

void CTFTeamMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		engine->pfnClientCmd(const_cast<char *>(command));
	}

	BaseClass::OnCommand(command);
}

void CTFTeamMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CTFTeamMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CTFTeamMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	Update();
}