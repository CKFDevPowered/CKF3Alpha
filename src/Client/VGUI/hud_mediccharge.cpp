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

#include "hud_mediccharge.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudMedicChargeMeter::CTFHudMedicChargeMeter() : CHudElement(), BaseClass( NULL, "HudMedicCharge" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");

	SetHiddenBits(HIDEHUD_WEAPONS);

	m_pChargeMeter = new ContinuousProgressBar( this, "ChargeMeter" );

	m_pChargeLabel = new Label(this, "ChargeLabel", "");
	m_pIndividualChargesLabel = new Label(this, "IndividualChargesLabel", "");
	m_pImageBG = new ImagePanel(this, "Background");

	// load control settings...
	LoadControlSettings( "resource/UI/HudMedicCharge.res" );
}

void CTFHudMedicChargeMeter::Reset(void)
{
	//force meter to update
	m_flLastChargeValue = -1;
	m_nTeam = 0;
	m_bCharged = false;

	m_pIndividualChargesLabel->SetVisible(false);
}

void CTFHudMedicChargeMeter::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}


//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudMedicChargeMeter::OnThink(void)
{
	CKFClientWeapon *pWeapon = gCKFVars.g_Player->m_pActiveItem;

	if ( !pWeapon || pWeapon->m_iId != WEAPON_MEDIGUN )
		return;

	if(m_nTeam != (*gCKFVars.g_iTeam))
	{
		m_nTeam = (*gCKFVars.g_iTeam);
		m_pImageBG->SetImage( (m_nTeam == TEAM_RED) ? "resource/tga/mask_uber_red" : "resource/tga/mask_uber_blu" );
	}

	float flCharge = gCKFVars.g_Player->m_fUbercharge;

	if ( flCharge != m_flLastChargeValue )
	{
		if ( m_pChargeMeter )
		{
			m_pChargeMeter->SetProgress( flCharge / 100 );
		}

		SetDialogVariable( "charge", (int)flCharge );
		//SetDialogVariable( "charge_count", flCharge );

		if ( !m_bCharged )
		{
			if ( flCharge >= 100.0 )
			{
				GetAnimationController()->StartAnimationSequence( this, "HudMedicCharged" );
				m_bCharged = true;
			}
		}
		else
		{
			// we've got invuln charge or we're using our invuln
			if ( gCKFVars.g_Player->m_iUbercharge )
			{
				GetAnimationController()->StartAnimationSequence( this, "HudMedicChargedStop" );
				m_bCharged = false;
			}
		}
	}

	m_flLastChargeValue = flCharge;
}

void CTFHudMedicChargeMeter::Init(void)
{
	SetVisible(true);
}

void CTFHudMedicChargeMeter::VidInit(void)
{
}

bool CTFHudMedicChargeMeter::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	if((*gCKFVars.g_iClass) != CLASS_MEDIC)
		return false;
	if(!(gCKFVars.g_Player->m_pActiveItem))
		return false;
	if(gCKFVars.g_Player->m_pActiveItem->m_iId != WEAPON_MEDIGUN)
		return false;
	return true;
}

}