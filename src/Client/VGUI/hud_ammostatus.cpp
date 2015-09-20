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

#include "hud_ammostatus.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudWeaponAmmo::CTFHudWeaponAmmo() : CHudElement(), BaseClass( NULL, "HudWeaponAmmo" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits(HIDEHUD_WEAPONS);

	m_pInClip = new Label(this, "AmmoInClip", "");
	m_pInClipShadow = new Label(this, "AmmoInClipShadow", "");

	m_pInReserve = new Label(this, "AmmoInReserve", "");
	m_pInReserveShadow = new Label(this, "AmmoInReserveShadow", "");

	m_pNoClip = new Label(this, "AmmoNoClip", "");
	m_pNoClipShadow = new Label(this, "AmmoNoClipShadow", "");

	m_pImageBG = new ImagePanel(this, "HudWeaponAmmoBG");

	// load control settings...
	LoadControlSettings( "resource/UI/HudAmmoWeapons.res" );

	m_flNextThink = 0.0f;
}

void CTFHudWeaponAmmo::Reset(void)
{
	m_flNextThink = engine->GetAbsoluteTime() + 0.05f;

	//force to update
	m_nAmmo	= 0;
	m_nAmmo2 = 0;
	m_hCurrentWeapon = NULL;

	UpdateAmmoLabels( false, false, false );
}

void CTFHudWeaponAmmo::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudWeaponAmmo::UpdateAmmoLabels( bool bPrimary, bool bReserve, bool bNoClip )
{
	if ( m_pInClip && m_pInClipShadow )
	{
		if ( m_pInClip->IsVisible() != bPrimary )
		{
			m_pInClip->SetVisible( bPrimary );
			m_pInClipShadow->SetVisible( bPrimary );
		}
	}

	if ( m_pInReserve && m_pInReserveShadow )
	{
		if ( m_pInReserve->IsVisible() != bReserve )
		{
			m_pInReserve->SetVisible( bReserve );
			m_pInReserveShadow->SetVisible( bReserve );
		}
	}

	if ( m_pNoClip && m_pNoClipShadow )
	{
		if ( m_pNoClip->IsVisible() != bNoClip )
		{
			m_pNoClip->SetVisible( bNoClip );
			m_pNoClipShadow->SetVisible( bNoClip );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudWeaponAmmo::OnThink(void)
{
	CKFClientWeapon *pWeapon = gCKFVars.g_Player->m_pActiveItem;
	if ( m_flNextThink < engine->GetAbsoluteTime() )
	{
		if ( !pWeapon || (pWeapon->iMaxClip() <= 0 && pWeapon->iMaxAmmo() <= 0) )
		{
			// turn off our ammo counts
			UpdateAmmoLabels( false, false, false );

			if(m_pImageBG)
			{
				m_pImageBG->SetVisible(false);
			}

			m_nAmmo = 0;
			m_nAmmo2 = 0;
		}
		else
		{
			// Get the ammo in our clip.
			int nAmmo1 = pWeapon->m_iClip;
			int nAmmo2 = 0;
			// Clip ammo not used, get total ammo count.
			if ( nAmmo1 < 0 )
			{
				nAmmo1 = pWeapon->m_iAmmo;
			}
			// Clip ammo, so the second ammo is the total ammo.
			else
			{
				nAmmo2 = pWeapon->m_iAmmo;
			}

			if ( m_nAmmo != nAmmo1 || m_nAmmo2 != nAmmo2 || m_hCurrentWeapon != pWeapon || m_nTeam != (*gCKFVars.g_iTeam) )
			{
				m_nAmmo = nAmmo1;
				m_nAmmo2 = nAmmo2;
				m_hCurrentWeapon = pWeapon;
				m_nTeam = (*gCKFVars.g_iTeam);

				if ( m_hCurrentWeapon->iMaxClip() > 0 )
				{
					UpdateAmmoLabels( true, true, false );

					SetDialogVariable( "Ammo", m_nAmmo );
					SetDialogVariable( "AmmoInReserve", m_nAmmo2 );
				}
				else
				{
					UpdateAmmoLabels( false, false, true );
					SetDialogVariable( "Ammo", m_nAmmo );
				}
			}

			if(m_pImageBG)
			{
				m_pImageBG->SetVisible(true);
				m_pImageBG->SetImage( (m_nTeam == TEAM_RED) ? "resource/tga/mask_weapon_red" : "resource/tga/mask_weapon_blu");
			}
		}

		m_flNextThink = engine->GetAbsoluteTime() + 0.1f;
	}
}

void CTFHudWeaponAmmo::Init(void)
{
	SetVisible(true);
}

void CTFHudWeaponAmmo::VidInit(void)
{
	//m_pHudPlayerClass->VidInit();
}

bool CTFHudWeaponAmmo::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	//if(!(*gCKFVars.g_iMaxHealth) || !(*gCKFVars.g_iClass))
	//	return false;
	//if((*gCKFVars.g_iTeam) != TEAM_RED && (*gCKFVars.g_iTeam) != TEAM_BLUE)
	//	return false;
	if(!(gCKFVars.g_Player->m_pActiveItem))
		return false;
	return true;
}

}