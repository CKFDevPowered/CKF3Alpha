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

#include "hud_menu_engy_build.h"

#include "CounterStrikeViewport.h"

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudMenuEngyBuild::CTFHudMenuEngyBuild() : CHudElement(), BaseClass( NULL, "HudMenuEngyBuild" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits( 0 );

	char buf[64];

	for ( int i = 0; i < 4; i++ )
	{
		Q_snprintf( buf, sizeof(buf), "active_item_%d", i+1 );
		m_pAvailableObjects[i] = new EditablePanel( this, buf );

		Q_snprintf( buf, sizeof(buf), "already_built_item_%d", i+1 );
		m_pAlreadyBuiltObjects[i] = new EditablePanel( this, buf );

		Q_snprintf( buf, sizeof(buf), "cant_afford_item_%d", i+1 );
		m_pCantAffordObjects[i] = new EditablePanel( this, buf );

		Q_snprintf( buf, sizeof(buf), "unavailable_item_%d", i+1 );
		m_pUnavailableObjects[i] = new EditablePanel( this, buf );
	}

	// load control settings...
	LoadControlSettings( "resource/UI/build_menu/HudMenuEngyBuild.res" );

	// Load the already built images, not destroyable
	const char build_name[4][16] = {"sentry", "dispenser", "tele_entrance", "tele_exit"};
	for ( int i = 0; i < 4; i++ )
	{
		Q_snprintf( buf, sizeof(buf), "resource/UI/build_menu/%s_already_built.res", build_name[i] );
		m_pAlreadyBuiltObjects[i]->LoadControlSettings( "resource/UI/build_menu/base_already_built.res" );
		m_pAlreadyBuiltObjects[i]->LoadControlSettings( buf );

		Q_snprintf( buf, sizeof(buf), "resource/UI/build_menu/%s_active.res", build_name[i] );
		m_pAvailableObjects[i]->LoadControlSettings( "resource/UI/build_menu/base_active.res" );
		m_pAvailableObjects[i]->LoadControlSettings( buf );

		Q_snprintf( buf, sizeof(buf), "resource/UI/build_menu/%s_cant_afford.res", build_name[i] );
		m_pCantAffordObjects[i]->LoadControlSettings( "resource/UI/build_menu/base_cant_afford.res" );
		m_pCantAffordObjects[i]->LoadControlSettings( buf );

		Q_snprintf( buf, sizeof(buf), "resource/UI/build_menu/%s_unavailable.res", build_name[i] );
		m_pUnavailableObjects[i]->LoadControlSettings( "resource/UI/build_menu/base_unavailable.res" );
		m_pUnavailableObjects[i]->LoadControlSettings( buf );		
	}

	// Set the cost label
	for ( int i = 0; i < 4; i ++ )
	{
		int iCost = GetBuildCost(i + 1);

		m_pAvailableObjects[i]->SetDialogVariable( "metal", iCost );
		m_pAlreadyBuiltObjects[i]->SetDialogVariable( "metal", iCost );
		m_pCantAffordObjects[i]->SetDialogVariable( "metal", iCost );
		m_pUnavailableObjects[i]->SetDialogVariable( "metal", iCost );
	}
}

bool CTFHudMenuEngyBuild::KeyInput( int down, KeyCode keynum, const char *pszCurrentBinding )
{
	int iSlot = 0;

	switch( keynum )
	{
	case KEY_1:
		iSlot = 1;
		break;
	case KEY_2:
		iSlot = 2;
		break;
	case KEY_3:
		iSlot = 3;
		break;
	case KEY_4:
		iSlot = 4;
		break;

	case KEY_5:
	case KEY_6:
	case KEY_7:
	case KEY_8:
	case KEY_9:
		// Eat these keys
		return 0;

	case KEY_0:
	case KEY_XBUTTON_B:
		// cancel, close the menu
		engine->pfnClientCmd( "lastinv\n" );
		return 0;

	default:
		return 1;	// key not handled
	}

	if ( iSlot > 0 )
	{
		SendBuildMessage( iSlot );
		return 0;
	}

	return 1;
}

void CTFHudMenuEngyBuild::Reset(void)
{
}

void CTFHudMenuEngyBuild::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CTFHudMenuEngyBuild::Init(void)
{
	SetVisible(true);
}

void CTFHudMenuEngyBuild::VidInit(void)
{

}

bool CTFHudMenuEngyBuild::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;

	if(*gCKFVars.g_iClass != CLASS_ENGINEER)
		return false;
	
	if(gCKFVars.g_Player->m_pActiveItem && gCKFVars.g_Player->m_pActiveItem->m_iId == TF_WEAPON_PDA_ENGINEER_BUILD
		return true;

	return false;
}

int CTFHudMenuEngyBuild::GetBuildingIDFromSlot( int iSlot )
{
	int iBuilding = 0;
	switch( iSlot )
	{
	case 1:
		iBuilding = BUILDABLE_SENTRYGUN;
		break;
	case 2:
		iBuilding = BUILDABLE_DISPENSER;
		break;
	case 3:
		iBuilding = BUILDABLE_ENTRANCE;
		break;
	case 4:
		iBuilding = BUILDABLE_EXIT;
		break;

	default:
		break;
	}

	return iBuilding;
}

int CTFHudMenuEngyBuild::GetBuildingCost(int iBuildID)
{
	switch(iBuildClass)
	{
	case BUILDABLE_SENTRY:
		return 130;
	case BUILDABLE_DISPENSER:
		return 100;
	case BUILDABLE_ENTRANCE:
		return 125;
	case BUILDABLE_EXIT:
		return 125;
	}
	return 0;
}

void CTFHudMenuEngyBuild::SendBuildMessage( int iSlot )
{
	int iBuilding = GetBuildingIDFromSlot( iSlot );

	int iCost = GetBuildingCost( iBuilding );

	if ( !gCKFVars.g_Player->GetBuildingInfo( iBuilding ) && gCKFVars.g_Player->m_iMetal >= iCost )
	{
		char szCmd[128];
		Q_snprintf( szCmd, sizeof(szCmd), "build %d", iBuilding );
		engine->ClientCmd( szCmd );
	}
	else
	{
		engine->pEventAPI->EV_PlaySound(iLocalPlayerIndex, engine->GetLocalPlayer()->origin, CHAN_STATIC, "common/wpn_denyselect.wav", 1, 1, 0, 100);
	}
}

//bool CTFHudMenuEngyBuild::SendDestroyMessage( int iSlot )
//{
//	bool bSuccess = false;
//
//	int iBuilding = GetBuildingCost( iSlot );
//
//	if ( gCKFVars.g_Player->GetBuildingInfo( iBuilding ) )
//	{
//		char szCmd[128];
//		Q_snprintf( szCmd, sizeof(szCmd), "destroy %d", iBuilding );
//		engine->ClientCmd( szCmd );
//		bSuccess = true; 
//	}
//	else
//	{
//		engine->pEventAPI->EV_PlaySound(iLocalPlayerIndex, engine->GetLocalPlayer()->origin, CHAN_STATIC, "common/wpn_denyselect.wav", 1, 1, 0, 100);
//	}
//
//	return bSuccess;
//}

void CTFHudMenuEngyBuild::OnThink( void )
{
	int iAccount = gCKFVars.g_Player->m_iMetal;

	for ( int i = 0;i < 4; i ++ )
	{
		int iRemappedObjectID = GetBuildingIDFromSlot( i + 1 );

		// update this slot

		m_pAvailableObjects[i]->SetVisible( false );
		m_pAlreadyBuiltObjects[i]->SetVisible( false );
		m_pCantAffordObjects[i]->SetVisible( false );

		// If the building is already built
		if ( pObj != NULL && !pObj->IsPlacing() )
		{
			m_pAlreadyBuiltObjects[i]->SetVisible( true );
		}
		// See if we can afford it
		else if ( iAccount < GetBuildingCost( iRemappedObjectID ) )
		{
			m_pCantAffordObjects[i]->SetVisible( true );
		}
		else
		{
			// we can buy it
			m_pAvailableObjects[i]->SetVisible( true );
		}
	}
}

void CHudMenuEngyBuild::SetVisible( bool state )
{
	if ( state == true )
	{
		// close the weapon selection menu
		engine->ClientCmd( "cancelselect\n" );

		// set the %lastinv% dialog var to our binding
		const char *key = engine->Key_LookupBinding( "lastinv" );
		if ( !key )
		{
			key = "< not bound >";
		}

		SetDialogVariable( "lastinv", key );
	}

	BaseClass::SetVisible( state );
}

}