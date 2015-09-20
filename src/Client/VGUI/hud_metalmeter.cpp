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

#include "hud_metalmeter.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudMetalMeter::CTFHudMetalMeter() : CHudElement(), BaseClass( NULL, "HudMetalMeter" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits(HIDEHUD_HEALTH);

	m_pImageBG = new ImagePanel(this, "HudMetalMeterBG");
	m_pMetalLabel = new Label(this, "MetalMeterLabel", "");

	// load control settings...
	LoadControlSettings( "resource/UI/HudMetalMeters.res" );

	m_nTeam = 0;
	m_nMetal = 0;
}

void CTFHudMetalMeter::Reset(void)
{
	//force to update
	m_nTeam = 0;
	m_nMetal = 0;
}

void CTFHudMetalMeter::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudMetalMeter::OnThink(void)
{
	if((*gCKFVars.g_iTeam) != m_nTeam)
	{
		m_nTeam = (*gCKFVars.g_iTeam);
		if(m_pImageBG)
		{
			m_pImageBG->SetVisible(true);
			m_pImageBG->SetImage( (m_nTeam == TEAM_RED) ? "resource/tga/mask_weapon_small_red" : "resource/tga/mask_weapon_small_blu");
		}
	}

	int nMetal = gCKFVars.g_Player->m_iMetal;

	if(nMetal != m_nMetal)
	{
		m_nMetal = nMetal;

		SetDialogVariable( "metal", nMetal );
	}
}

void CTFHudMetalMeter::Init(void)
{
	SetVisible(true);
}

void CTFHudMetalMeter::VidInit(void)
{

}

bool CTFHudMetalMeter::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	//if(!(*gCKFVars.g_iMaxHealth) || !(*gCKFVars.g_iClass))
	//	return false;
	//if((*gCKFVars.g_iTeam) != TEAM_RED && (*gCKFVars.g_iTeam) != TEAM_BLUE)
	//	return false;
	if(*gCKFVars.g_iClass != CLASS_ENGINEER)
		return false;
	return true;
}

}