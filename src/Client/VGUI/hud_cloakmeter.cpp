#include "qgl.h"
#include "hud.h"

#include "cl_util.h"

#include "parsemsg.h"
#include "player.h"
#include "client.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <event_api.h>
#include <cdll_dll.h>

#include <ref_params.h>

#include <VGUI/VGUI.h>
#include <VGUI/ISurface.h>
#include <VGUI/Vector.h>
#include <KeyValues.h>

#include "hud_cloakmeter.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Engineer Metal Panel
//-----------------------------------------------------------------------------

CTFHudCloakMeter::CTFHudCloakMeter() : CHudElement(), BaseClass( NULL, "HudCloakMeter" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits(HIDEHUD_HEALTH);

	m_pImageBG = new ImagePanel(this, "HudCloakMeterBG");
	m_pCloakLabel = new Label(this, "CloakMeterLabel", "");
	m_pCloakMeter = new ContinuousProgressBar(this, "CloakMeterMeter");

	// load control settings...
	LoadControlSettings( "resource/UI/HudCloakMeters.res" );

	m_nTeam = 0;
	m_flCloakEnergy = 0;
}

void CTFHudCloakMeter::Reset(void)
{
	//force to update
	m_nTeam = 0;
	m_flCloakEnergy = 0;
}

void CTFHudCloakMeter::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pCloakMeter->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudCloakMeter::OnThink(void)
{
	if((*gCKFVars.g_iTeam) != m_nTeam)
	{
		m_nTeam = (*gCKFVars.g_iTeam);
		if(m_pImageBG)
		{
			m_pImageBG->SetVisible(true);
			m_pImageBG->SetImage( (m_nTeam == TEAM_RED) ? "resource/tga/mask_spy_red" : "resource/tga/mask_spy_blu");
		}
	}

	float flCloakEnergy = gCKFVars.g_Player->m_flCloakEnergy;

	if(flCloakEnergy != m_flCloakEnergy)
	{
		m_flCloakEnergy = flCloakEnergy;

		SetDialogVariable( "cloakenergy", (int)flCloakEnergy );
		m_pCloakMeter->SetProgress( flCloakEnergy / 100 );
	}
}

void CTFHudCloakMeter::Init(void)
{
	SetVisible(true);
}

void CTFHudCloakMeter::VidInit(void)
{
	//m_pHudPlayerClass->VidInit();
}

bool CTFHudCloakMeter::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	//if(!(*gCKFVars.g_iMaxHealth) || !(*gCKFVars.g_iClass))
	//	return false;
	//if((*gCKFVars.g_iTeam) != TEAM_RED && (*gCKFVars.g_iTeam) != TEAM_BLUE)
	//	return false;
	if(*gCKFVars.g_iClass != CLASS_SPY)
		return false;
	return true;
}

}