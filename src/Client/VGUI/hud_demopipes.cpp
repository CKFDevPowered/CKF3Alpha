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

#include "hud_demopipes.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudDemomanPipes::CTFHudDemomanPipes() : CHudElement(), BaseClass( NULL, "HudDemomanPipes" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits(HIDEHUD_HEALTH);

	m_pPipesPresent = new EditablePanel( this, "PipesPresentPanel" );
	m_pNoPipesPresent = new EditablePanel( this, "NoPipesPresentPanel" );
	m_pImageBG = new ImagePanel(this, "HudDemomanPipesBG");
	m_pChargeLabel = new Label(this, "ChargeLabel", "");
	m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");

	// load control settings...
	LoadControlSettings( "resource/UI/HudDemomanPipes.res" );

	m_nTeam = 0;
	m_nActivePipes = -1;
}

void CTFHudDemomanPipes::Reset(void)
{
	//force to update
	m_nTeam = 0;
	m_nActivePipes = -1;

	m_pChargeLabel->SetVisible(false);
	m_pChargeMeter->SetVisible(false);
}

void CTFHudDemomanPipes::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudDemomanPipes::OnThink(void)
{
	if((*gCKFVars.g_iTeam) != m_nTeam)
	{
		m_nTeam = (*gCKFVars.g_iTeam);
		if(m_pImageBG)
		{
			m_pImageBG->SetVisible(true);
			m_pImageBG->SetImage( (m_nTeam == TEAM_RED) ? "resource/tga/mask_demo_red" : "resource/tga/mask_demo_blu");
		}
	}

	int iPipes = gCKFVars.g_Player->GetNumActivePipebombs();

	if(iPipes != m_nActivePipes)
	{
		m_nActivePipes = iPipes;

		m_pPipesPresent->SetDialogVariable( "activepipes", iPipes );
		m_pNoPipesPresent->SetDialogVariable( "activepipes", iPipes );

		m_pPipesPresent->SetVisible( iPipes > 0 );
		m_pNoPipesPresent->SetVisible( iPipes <= 0 );
	}
}

void CTFHudDemomanPipes::Init(void)
{
	SetVisible(true);
}

void CTFHudDemomanPipes::VidInit(void)
{
	//m_pHudPlayerClass->VidInit();
}

bool CTFHudDemomanPipes::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	//if(!(*gCKFVars.g_iMaxHealth) || !(*gCKFVars.g_iClass))
	//	return false;
	//if((*gCKFVars.g_iTeam) != TEAM_RED && (*gCKFVars.g_iTeam) != TEAM_BLUE)
	//	return false;
	if(*gCKFVars.g_iClass != CLASS_DEMOMAN)
		return false;
	return true;
}

}