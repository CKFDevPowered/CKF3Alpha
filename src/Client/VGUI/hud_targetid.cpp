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

#include "hud_targetid.h"
#include "cdll_int.h"
#include "player_info.h"

#include "CounterStrikeViewport.h"

int GetBuffedMaxHealth( int iMaxHealth );

namespace vgui
{

const char *g_aPlayerClassNames[] =
{
	"#CKF3_Undefined",
	"#CKF3_Scout",
	"#CKF3_Heavy",
	"#CKF3_Soldier",
	"#CKF3_Pyro",
	"#CKF3_Sniper",
	"#CKF3_Medic",
	"#CKF3_Engineer",
	"#CKF3_Demoman",
	"#CKF3_Spy"
};

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudTargetID::CTFHudTargetID(const char *pPanelName) : CHudElement(), BaseClass( NULL, pPanelName ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits( 0 );

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	m_pTargetNameLabel = NULL;
	m_pTargetDataLabel = NULL;
	m_pBGPanel = NULL;
	m_pTargetHealth = new CTFSpectatorGUIHealth( this, "SpectatorGUIHealth" );
	m_pMoveableSubPanel = NULL;
	m_pMoveableIconBG = NULL;
	m_pMoveableIcon = NULL;
	m_pMoveableSymbolIcon = NULL;
	m_pMoveableKeyLabel = NULL;

	m_bLayoutOnUpdate = false;
	m_nTeam = 0;
}

void CTFHudTargetID::Reset(void)
{
	m_pTargetHealth->Reset();
}

void CTFHudTargetID::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	LoadControlSettings( "resource/UI/TargetID.res" );

	BaseClass::ApplySchemeSettings(pScheme);

	m_pTargetNameLabel = dynamic_cast<Label *>(FindChildByName("TargetNameLabel"));
	m_pTargetDataLabel = dynamic_cast<Label *>(FindChildByName("TargetDataLabel"));
	m_pBGPanel = dynamic_cast<TFHudPanel *>(FindChildByName("TargetIDBG"));
	m_pMoveableSubPanel = dynamic_cast<EditablePanel *>(FindChildByName("MoveableSubPanel"));
	if(m_pMoveableSubPanel)
	{
		m_pMoveableIconBG = dynamic_cast<ImagePanel *>(m_pMoveableSubPanel->FindChildByName("MoveableIconBG"));
		m_pMoveableIcon = dynamic_cast<ImagePanel *>(m_pMoveableSubPanel->FindChildByName("MoveableIcon"));
		m_pMoveableSymbolIcon = dynamic_cast<ImagePanel *>(m_pMoveableSubPanel->FindChildByName("MoveableSymbolIcon"));
		m_pMoveableKeyLabel = dynamic_cast<Label *>(m_pMoveableSubPanel->FindChildByName("MoveableKeyLabel"));
	
		const char *key = engine->Key_LookupBinding( "attack2" );
		if ( !key )
		{
			key = "< not bound >";
		}
		m_pMoveableSubPanel->SetDialogVariable("movekey", key );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudTargetID::OnThink(void)
{
	if((*gCKFVars.g_iTeam) != m_nTeam)
	{
		m_nTeam = (*gCKFVars.g_iTeam);
		if(m_pBGPanel)
		{
			m_pBGPanel->SetVisible(true);
			m_pBGPanel->SetColorIndex( (m_nTeam >= 3 || m_nTeam < 0) ? 0 : m_nTeam );
		}
	}
}

void CTFHudTargetID::Init(void)
{
	SetVisible(true);
}

void CTFHudTargetID::VidInit(void)
{
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
	m_nTeam = 0;
}

bool CTFHudTargetID::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ))
		return false;

	// Get our target's ent index

	m_iTargetEntIndex = CalculateTargetIndex();
	if ( !m_iTargetEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && ( engine->GetAbsoluteTime() > m_flLastChangeTime ) )
		{
			m_flLastChangeTime = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			m_iTargetEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = engine->GetAbsoluteTime();
	}

	bool bReturn = false;
	if ( m_iTargetEntIndex )
	{
		cl_entity_t *pEnt = engine->GetEntityByIndex(m_iTargetEntIndex);
		if(pEnt)
		{
			if ( pEnt->player )
			{
				bool bDisguisedEnemy = false;
				int iDisguiseTarget = pEnt->curstate.endpos[0];
				if ( iDisguiseTarget && !(pEnt->curstate.renderfx == kRenderFxCloak && pEnt->curstate.renderamt < 255)) // they're not cloaked
				{
					if( (*gCKFVars.g_iTeam) == pEnt->curstate.team )
						bDisguisedEnemy = true;
				}

				bReturn = ( (*gCKFVars.g_iTeam) == TEAM_SPECTATOR || (*gCKFVars.g_iTeam) == gCKFVars.g_PlayerInfo[m_iTargetEntIndex].iTeam || bDisguisedEnemy );
			}
			else if ( pEnt->curstate.playerclass == CLASS_BUILDABLE )
			{
				if((*gCKFVars.g_iTeam) == pEnt->curstate.team || (*gCKFVars.g_iTeam) == TEAM_SPECTATOR || (*gCKFVars.g_iClass) == CLASS_SPY )
					bReturn = true;
			}
		}
	}

	if ( bReturn )
	{
		if ( !IsVisible() || (m_iTargetEntIndex != m_iLastEntIndex) )
		{
			m_iLastEntIndex = m_iTargetEntIndex;
			m_bLayoutOnUpdate = true;
		}

		UpdateID();
	}

	return bReturn;
}

void CTFHudTargetID::PerformLayout( void )
{
	int iX,iY;
	int iPanelWidth, iPanelHeight;
	int iXIndent = XRES(5);
	int iXPostdent = XRES(10);
	int iWidth = m_pTargetHealth->GetWide() + iXIndent + iXPostdent;

	int iTextW, iTextH;
	int iDataW, iDataH;

	//an initalization just in case
	iPanelWidth = 0;

	//perform layout for moveable panel
	//if(m_pMoveableSubPanel->IsVisible())
	//{
		m_pMoveableKeyLabel->GetContentSize( iTextW, iTextH );
		m_pMoveableSymbolIcon->GetSize( iDataW, iDataH );
		m_pMoveableSubPanel->GetSize( iPanelWidth, iPanelHeight );

		iPanelHeight = GetTall() + YRES(8);
		iPanelWidth = iXPostdent + XRES(2);
		iPanelWidth += max(iTextW, iDataW);

		//we have (71pixels width of this image filled)
		int iFixedPanelWidth = iPanelWidth * 128 / 72;

		m_pMoveableSubPanel->SetSize( iFixedPanelWidth, iPanelHeight );
		m_pMoveableIconBG->SetSize( iFixedPanelWidth, iPanelHeight );

		m_pMoveableKeyLabel->GetPos( iX, iY );
		m_pMoveableKeyLabel->SetPos( (iPanelWidth - iTextW) * 0.5 + iXIndent, iY );

		m_pMoveableSymbolIcon->GetPos( iX, iY );
		m_pMoveableSymbolIcon->SetPos( (iPanelWidth - iDataW) * 0.5 + iXIndent, iY );

		m_pMoveableIcon->GetBounds( iX, iY, iDataW, iDataH );
		m_pMoveableIcon->SetPos( (iPanelWidth - iDataW) * 0.5 + iXIndent, iY );
	//}

	//perform layout for targetid panel
	m_pTargetNameLabel->GetContentSize( iTextW, iTextH );
	m_pTargetDataLabel->GetContentSize( iDataW, iDataH );

	iWidth += max(iTextW, iDataW);

	iPanelHeight = GetTall();

	SetSize( iWidth + iPanelWidth, iPanelHeight );

	GetPos( iX, iY );
	SetPos( (ScreenWidth - iWidth) * 0.5, iY );

	m_pBGPanel->SetSize( iWidth , iPanelHeight );
	
	//if(m_pMoveableSubPanel->IsVisible())
	//{
		m_pMoveableSubPanel->GetPos( iX, iY );
		m_pMoveableSubPanel->SetPos( iWidth - iXIndent , iY );
	//}

	//Reset position NameLabel
	int iHealthX, iHealthY;
	m_pTargetHealth->GetPos(iHealthX, iHealthY);

	int iNewX = iHealthX + m_pTargetHealth->GetWide() + iXIndent;	

	m_pTargetNameLabel->GetPos(iX, iY);	
	m_pTargetNameLabel->SetPos(iNewX, iY);

	m_pTargetDataLabel->GetPos(iX, iY);
	m_pTargetDataLabel->SetPos(iNewX, iY);
};

int	CTFHudTargetID::CalculateTargetIndex( void ) 
{ 
	int iIndex = 0; 

	if(*gCKFVars.g_pTraceEntity)
		iIndex = (*gCKFVars.g_pTraceEntity)->index;

	// If our target entity is already in our secondary ID, don't show it in primary.
	CSecondaryTargetID *pSecondaryID = dynamic_cast<CSecondaryTargetID *>(g_pViewPort->FindChildByName( "CSecondaryTargetID" ));
	if ( pSecondaryID && pSecondaryID != this && pSecondaryID->GetTargetIndex() == iIndex )
	{
		iIndex = 0;
	}

	return iIndex;
}

void CTFHudTargetID::UpdateID( void )
{
	wchar_t sIDString[ 256 ] = L"";
	wchar_t sDataString[ 256 ] = L"";

	// Get our target's ent index
	Assert( m_iTargetEntIndex );

	// Is this an entindex sent by the server?
	if ( m_iTargetEntIndex )
	{
		cl_entity_t *pEnt = engine->GetEntityByIndex( m_iTargetEntIndex );
		if ( !pEnt )
			return;

		bool bShowHealth = false;
		int iHealth = 0;
		int iMaxHealth = 1;
		int iMaxBuffedHealth = 0;

		//hide as default
		m_pTargetHealth->SetBuilding(false);
		m_pMoveableSubPanel->SetVisible(false);

		// Some entities we always want to check, cause the text may change
		// even while we're looking at it
		// Is it a player?
		if ( pEnt->player )
		{
			const char *printFormatString = NULL;
			wchar_t wszPlayerName[ 32 ];
			bool bDisguisedTarget = false;
			bool bDisguisedEnemy = false;

			if (gViewPortInterface)
				gViewPortInterface->GetAllPlayersInfo();

			if(!g_PlayerInfoList[m_iTargetEntIndex].name || !g_PlayerInfoList[m_iTargetEntIndex].name[0])
				return;

			g_pVGuiLocalize->ConvertANSIToUnicode( g_PlayerInfoList[m_iTargetEntIndex].name, wszPlayerName, sizeof(wszPlayerName) );

			int iDisguiseTarget = pEnt->curstate.endpos[0];
			if(iDisguiseTarget) iDisguiseTarget = (iDisguiseTarget>>1);

			// determine if the target is a disguised spy (either friendly or enemy)
			if ( iDisguiseTarget && // they're disguised
				!(pEnt->curstate.renderfx == kRenderFxCloak && pEnt->curstate.renderamt < 255) ) // they're not cloaked
			{
				bDisguisedTarget = true;
			}

			int iTargetTeam = gCKFVars.g_PlayerInfo[m_iTargetEntIndex].iTeam;

			if ( bDisguisedTarget )
			{
				// is the target a disguised enemy spy?
				if ( (*gCKFVars.g_iTeam) == 3 - iTargetTeam )
				{
					if ( iDisguiseTarget )
					{
						bDisguisedEnemy = true;
						// change the player name
						if(g_PlayerInfoList[iDisguiseTarget].name && g_PlayerInfoList[iDisguiseTarget].name[0])
							g_pVGuiLocalize->ConvertANSIToUnicode( g_PlayerInfoList[iDisguiseTarget].name, wszPlayerName, sizeof(wszPlayerName) );
						// change the team  / team color
					}
				}
				else
				{
					// The target is a disguised friendly spy.  They appear to the player with no disguise.  Add the disguise
					// team & class to the target ID element.
					bool bDisguisedAsEnemy = ( pEnt->curstate.team != iTargetTeam );
					const wchar_t *wszAlignment = g_pVGuiLocalize->Find( bDisguisedAsEnemy ? "#CKF3_enemy" : "#CKF3_friendly" );
					
					int classindex = pEnt->curstate.playerclass;
					const wchar_t *wszClassName = g_pVGuiLocalize->Find( g_aPlayerClassNames[classindex] );

					// build a string with disguise information
					g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find( "#CKF3_playerid_friendlyspy_disguise" ), 
						2, wszAlignment, wszClassName );
				}
			}

			if ( pEnt->curstate.playerclass == CLASS_MEDIC )
			{
				wchar_t wszChargeLevel[ 10 ];

				float flMedicChargeLevel = pEnt->curstate.startpos[2];
				_snwprintf( wszChargeLevel, ARRAYSIZE(wszChargeLevel) - 1, L"%.0f", flMedicChargeLevel );
				wszChargeLevel[ ARRAYSIZE(wszChargeLevel)-1 ] = '\0';
				g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find( "#CKF3_playerid_mediccharge" ), 1, wszChargeLevel );
			}
			
			if ( (*gCKFVars.g_iTeam) == TEAM_SPECTATOR || (*gCKFVars.g_iTeam) == iTargetTeam || bDisguisedEnemy )
			{
				printFormatString = "#CKF3_playerid_sameteam";
				bShowHealth = true;
			}
			else if ( gCKFVars.g_Player->pev.deadflag != DEAD_NO && g_iUser1 )
			{
				// We're looking at an enemy who killed us.
				printFormatString = "#CKF3_playerid_diffteam";
				bShowHealth = true;
			}

			if ( bShowHealth )
			{
				iMaxHealth = pEnt->curstate.startpos[1];
				iMaxBuffedHealth = GetBuffedMaxHealth( iMaxHealth );
				iHealth = pEnt->curstate.startpos[0];
			}

			if ( printFormatString )
			{
				wchar_t *pszPrepend = GetPrepend();
				if ( !pszPrepend || !pszPrepend[0] )
				{
					pszPrepend = L"";
				}
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 2, pszPrepend, wszPlayerName );
			}
		}
		else
		{
			// see if it is a buildable
			if ( pEnt->curstate.playerclass == CLASS_BUILDABLE )
			{
				int iTargetTeam = pEnt->curstate.team;
				int iBuildClass = pEnt->curstate.iuser3;
				int iBuildLevel = pEnt->curstate.iuser2;
				int iOwner = pEnt->curstate.iuser1;

				//format id string
				wchar_t wszBuilderName[ MAX_PLAYER_NAME_LENGTH ];
				wchar_t *wszObjectName = L"";

				if(iBuildClass == BUILDABLE_SENTRY)
					wszObjectName = g_pVGuiLocalize->Find( "#CKF3_Buildable_Sentry" );
				else if(iBuildClass == BUILDABLE_DISPENSER)
					wszObjectName = g_pVGuiLocalize->Find( "#CKF3_Buildable_Dispenser" );
				else if(iBuildClass == BUILDABLE_ENTRANCE)
					wszObjectName = g_pVGuiLocalize->Find( "#CKF3_Buildable_TeleEntrance" );
				else if(iBuildClass == BUILDABLE_EXIT)
					wszObjectName = g_pVGuiLocalize->Find( "#CKF3_Buildable_TeleExit" );

				//is it a valid owner?
				if ( iOwner >= 1 && iOwner <= engine->GetMaxClients() && g_PlayerInfoList[iOwner].name && g_PlayerInfoList[iOwner].name[0] )
				{
					g_pVGuiLocalize->ConvertANSIToUnicode( g_PlayerInfoList[iOwner].name, wszBuilderName, sizeof(wszBuilderName) );
				}
				else
				{
					wszBuilderName[0] = L'\0';
				}

				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find( ((*gCKFVars.g_iTeam) == 3 - iTargetTeam) ? "#CKF3_playerid_object_diffteam" : "#CKF3_playerid_object"), 2, wszObjectName, wszBuilderName );

				//format data string
				wchar_t *wszBuildLevel = L"1";

				if(iBuildLevel == 3)
					wszBuildLevel = L"3";
				else if(iBuildLevel == 2)
					wszBuildLevel = L"2";

				int iBuildUpgrade = pEnt->curstate.endpos[2];

				_snwprintf( wszBuilderName, ARRAYSIZE(wszBuilderName) - 1, L"%d / %d", iBuildUpgrade, 200 );
				wszBuilderName[ ARRAYSIZE(wszBuilderName)-1 ] = '\0';

				//only teammate can see upgrade progress
				if(iBuildLevel < 3 && (*gCKFVars.g_iTeam) == iTargetTeam)
				{
					g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find("#CKF3_playerid_object_upgrading_level"), 2, wszBuildLevel, wszBuilderName );
				}
				else
				{
					g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find("#CKF3_playerid_object_level"), 1, wszBuildLevel );
				}

				bShowHealth = true;
				iHealth = pEnt->curstate.startpos[0];
				iMaxHealth = pEnt->curstate.endpos[0];

				if(iMaxHealth < 150)
					iMaxHealth = 150;//a display fix

				m_pTargetHealth->SetBuilding(true);
				if( gCKFVars.g_Player->CanPickupBuilding(pEnt) )
				{
					//fix moveable panel
					if(iBuildClass == BUILDABLE_SENTRY)
					{
						if(iBuildLevel == 3)
						{
							m_pMoveableIcon->SetImage("resource/tga/ico_sentry3");
						}
						else if(iBuildLevel == 2)
						{
							m_pMoveableIcon->SetImage("resource/tga/ico_sentry2");
						}
						else
						{
							m_pMoveableIcon->SetImage("resource/tga/ico_sentry1");
						}
					}
					else if(iBuildClass == BUILDABLE_DISPENSER)
					{
						m_pMoveableIcon->SetImage("resource/tga/ico_dispenser");
					}
					else if(iBuildClass == BUILDABLE_ENTRANCE)
					{
						m_pMoveableIcon->SetImage("resource/tga/ico_telein");
					}
					else
					{
						m_pMoveableIcon->SetImage("resource/tga/ico_teleout");
					}

					m_pMoveableSubPanel->SetVisible(true);
				}
			}
		}

		// Setup health icon
		//if ( !pEnt->IsAlive() )
		//{
		//	flHealth = 0;	// fixup for health being 1 when dead
		//}

		m_pTargetHealth->SetHealth( iHealth, iMaxHealth, iMaxBuffedHealth );
		m_pTargetHealth->SetVisible( bShowHealth );

		int iNameW, iDataW, iIgnored;
		m_pTargetNameLabel->GetContentSize( iNameW, iIgnored );
		m_pTargetDataLabel->GetContentSize( iDataW, iIgnored );

		// Target name
		if ( sIDString[0] )
		{
			sIDString[ ARRAYSIZE(sIDString)-1 ] = '\0';
			m_pTargetNameLabel->SetVisible(true);

			// TODO: Support	if( hud_centerid.GetInt() == 0 )
			SetDialogVariable( "targetname", sIDString );
		}
		else
		{
			m_pTargetNameLabel->SetVisible(false);
			m_pTargetNameLabel->SetText("");
		}

		// Extra target data
		if ( sDataString[0] )
		{
			sDataString[ ARRAYSIZE(sDataString)-1 ] = '\0';
			m_pTargetDataLabel->SetVisible(true);
			SetDialogVariable( "targetdata", sDataString );
		}
		else
		{
			m_pTargetDataLabel->SetVisible(false);
			m_pTargetDataLabel->SetText("");
		}

		int iPostNameW, iPostDataW;
		m_pTargetNameLabel->GetContentSize( iPostNameW, iIgnored );
		m_pTargetDataLabel->GetContentSize( iPostDataW, iIgnored );

		if ( m_bLayoutOnUpdate || (iPostDataW != iDataW) || (iPostNameW != iNameW) )
		{
			InvalidateLayout( true );
			m_bLayoutOnUpdate = false;
		}
	}
}

int CSecondaryTargetID::CalculateTargetIndex( void )
{
	// If we're a medic & we're healing someone, target him.
	int iHealTarget = gCKFVars.g_Player->GetMedigunHealingTarget();
	if ( iHealTarget )
	{
		cl_entity_t *pHealTarget = engine->GetEntityByIndex(iHealTarget);
		if(pHealTarget && pHealTarget->player)
		{
			if ( iHealTarget != m_iTargetEntIndex )
			{
				g_pVGuiLocalize->ConstructString( m_wszPrepend, sizeof(m_wszPrepend), g_pVGuiLocalize->Find("#CKF3_playerid_healtarget" ), 0 );
			}
			return iHealTarget;
		}
	}

	// If we have a healer, target him.
	int iHealer = gCKFVars.g_Player->m_iHealer;
	
	if ( iHealer )
	{
		cl_entity_t *pHealer = engine->GetEntityByIndex(iHealer);
		if(pHealer && pHealer->player)
		{
			if ( iHealer != m_iTargetEntIndex )
			{
				g_pVGuiLocalize->ConstructString( m_wszPrepend, sizeof(m_wszPrepend), g_pVGuiLocalize->Find("#CKF3_playerid_healer" ), 0 );
			}
			return iHealer;
		}
	}

	if ( m_iTargetEntIndex )
	{
		m_wszPrepend[0] = '\0';
	}
	return 0;
}

bool CMainTargetID::ShouldDraw( void )
{
	if ( g_iUser1 > OBS_NONE )
		return false;

	return BaseClass::ShouldDraw();
}

bool CSpectatorTargetID::ShouldDraw( void )
{
	if ( g_iUser1 <= OBS_NONE )
		return false;

	return BaseClass::ShouldDraw();
}

int	CSpectatorTargetID::CalculateTargetIndex( void ) 
{ 
	int iIndex = BaseClass::CalculateTargetIndex();

	if ( g_iUser1 == OBS_IN_EYE && g_iUser2 )
	{
		iIndex = g_iUser2;
	}

	return iIndex;
}



}