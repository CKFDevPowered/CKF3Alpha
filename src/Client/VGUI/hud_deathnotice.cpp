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

#include "hud_deathnotice.h"
#include <ICKFClient.h>

#include "CounterStrikeViewport.h"

cvar_t *hud_deathnotice_time;

static int DEATHNOTICE_DISPLAY_TIME = 6;

namespace vgui
{



// Must match resource/tf_objects.txt!!!
const char *szLocalizedObjectNames[] =
{
	"#CKF3_Object_Sentry",
	"#CKF3_Object_Dispenser",
	"#CKF3_Object_Tele_Entrance",
	"#CKF3_Object_Tele_Exit",	
	"#CKF3_Object_Sapper"
};

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudDeathNotice::CTFHudDeathNotice() : CHudElement(), BaseClass( NULL, "HudDeathNotice" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");	

	SetHiddenBits( 0 );
}

DeathNoticeIcon *CTFHudDeathNotice::FindDeathIcon(const char *szName)
{
	for(int i = 0; i < m_Icons.Count(); i++)
	{
		if(!stricmp(m_Icons[i].szName, szName))
			return &m_Icons[i];
	}
	return NULL;
}

void CTFHudDeathNotice::LoadDeathIcon(const char *szName)
{
	DeathNoticeIcon &icon = m_Icons[m_Icons.AddToTail()];

	strncpy(icon.szName, szName, ARRAYSIZE(icon.szName));

	char szFullpath[256];
	sprintf(szFullpath, "resource/tga/d_%s", szName);

	icon.iTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( icon.iTextureID, szFullpath, true, false );
	surface()->DrawGetTextureSize( icon.iTextureID, icon.iWide, icon.iTall );

	int iNameLength = strlen(szName);
	if((szName[iNameLength-1] == 'b' || szName[iNameLength-1] == 'd') && szName[iNameLength-2] == '_')
		icon.bColored = true;
}

void CTFHudDeathNotice::Reset(void)
{

}

void CTFHudDeathNotice::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundEnabled( false );

	CalcRoundedCorners();
}

void CTFHudDeathNotice::OnThink(void)
{

}

void CTFHudDeathNotice::Init(void)
{
	SetVisible(true);

	hud_deathnotice_time = engine->pfnRegisterVariable("hud_deathnotice_time", "6", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

	LoadDeathIcon("suicide");
	LoadDeathIcon("critical");
	LoadDeathIcon("backstab");
	LoadDeathIcon("bat");
	LoadDeathIcon("blucap_b");
	LoadDeathIcon("blucap_d");
	LoadDeathIcon("bludef_b");
	LoadDeathIcon("bludef_d");
	LoadDeathIcon("bonesaw");
	LoadDeathIcon("bottle");
	LoadDeathIcon("butterfly");	
	LoadDeathIcon("defgrenade");
	LoadDeathIcon("defrocket");
	LoadDeathIcon("defsticky");
	LoadDeathIcon("dom_b");
	LoadDeathIcon("dom_d");
	LoadDeathIcon("fireaxe");
	LoadDeathIcon("fist");
	LoadDeathIcon("flame_b");
	LoadDeathIcon("flame_d");
	LoadDeathIcon("grenade");
	LoadDeathIcon("headshot");
	LoadDeathIcon("kukri");
	LoadDeathIcon("minigun");
	LoadDeathIcon("pistol");
	LoadDeathIcon("redcap_b");
	LoadDeathIcon("redcap_d");
	LoadDeathIcon("reddef_b");
	LoadDeathIcon("reddef_d");
	LoadDeathIcon("revolver");
	LoadDeathIcon("rocket");
	LoadDeathIcon("sapper_b");
	LoadDeathIcon("sapper_d");
	LoadDeathIcon("scattergun");
	LoadDeathIcon("sentry1");
	LoadDeathIcon("sentry2");
	LoadDeathIcon("sentry3");
	LoadDeathIcon("shotgun");
	LoadDeathIcon("shovel");
	LoadDeathIcon("smg");
	LoadDeathIcon("sniperifle");
	LoadDeathIcon("sticky");
	LoadDeathIcon("syringe");
	LoadDeathIcon("wrench");

	m_CritIcon = FindDeathIcon("critical");
}

void CTFHudDeathNotice::VidInit(void)
{
	m_DeathNotices.RemoveAll();
}

bool CTFHudDeathNotice::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ))
		return false;
	if( !m_DeathNotices.Count() )
		return false;
	return true;
}

void CTFHudDeathNotice::Paint()
{
	// Retire any death notices that have expired
	RetireExpiredDeathNotices();

	int yStart = YRES( 24 );

	surface()->DrawSetTextFont( m_hTextFont );

	int xMargin = XRES( 10 );
	int xSpacing = surface()->GetCharacterWidth( m_hTextFont, L' ' );

	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		DeathNoticeItem &msg = m_DeathNotices[i];
		
		DeathNoticeIcon *icon = msg.Icon;
						
		wchar_t victim[256]=L"";
		wchar_t killer[256]=L"";

		// TEMP - print the death icon name if we don't have a material for it

		g_pVGuiLocalize->ConvertANSIToUnicode( msg.Victim.szName, victim, sizeof( victim ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( msg.Killer.szName, killer, sizeof( killer ) );

		int iVictimTextWide, iVictimTextTall;
		surface()->GetTextSize(m_hTextFont, victim, iVictimTextWide, iVictimTextTall);
		iVictimTextWide += xSpacing;

		int iDeathInfoTextWide, iDeathInfoTextTall;
		
		if(msg.wzInfoText[0])
		{
			surface()->GetTextSize(m_hTextFont, msg.wzInfoText, iDeathInfoTextWide, iDeathInfoTextTall);
			iDeathInfoTextWide += xSpacing;
		}
		else
		{
			iDeathInfoTextWide = 0;
		}
		
		int iKillerTextWide, iKillerTextTall;
		if(killer[0])
		{
			surface()->GetTextSize(m_hTextFont, killer, iKillerTextWide, iKillerTextTall);
			iKillerTextWide += xSpacing;
		}
		else
		{
			iKillerTextWide = 0;
		}

		int iLineTall = m_flLineHeight;
		int iTextTall = surface()->GetFontTall( m_hTextFont );

		int iconWide = 0, iconTall = 0, iDeathInfoOffset = 0, iVictimTextOffset = 0, iconActualWide = 0;

		// Get the local position for this notice
		if ( icon )
		{			
			iconActualWide = icon->iWide;
			iconWide = iconActualWide + xSpacing;
			iconTall = icon->iTall;
			
			int iconTallDesired = iLineTall-YRES(2);
			Assert( 0 != iconTallDesired );
			float flScale = (float) iconTallDesired / (float) iconTall;

			iconActualWide *= flScale;
			iconTall *= flScale;
			iconWide *= flScale;
		}
		int iTotalWide = iKillerTextWide + iconWide + iVictimTextWide + iDeathInfoTextWide + ( xMargin * 2 );
		int y = yStart + ( ( iLineTall + m_flLineSpacing ) * i );				
		int yText = y + ( ( iLineTall - iTextTall ) / 2 );
		int yIcon = y + ( ( iLineTall - iconTall ) / 2 );

		int x=0;
		if ( m_bRightJustify )
		{
			x =	GetWide() - iTotalWide;
		}

		// draw a background panel for the message
		Vertex_t vert[NUM_BACKGROUND_COORD];

		GetBackgroundPolygonVerts( x, y+1, x+iTotalWide, y+iLineTall-1, ARRAYSIZE( vert ), vert );		
		surface()->DrawSetTexture( -1 );
		surface()->DrawSetColor( msg.bLocalPlayerInvolved ? m_clrLocalBGColor : m_clrBaseBGColor );
		surface()->DrawTexturedPolygon( ARRAYSIZE( vert ), (float *)vert );

		x += xMargin;
			
		if ( killer[0] )
		{
			// Draw killer's name
			DrawUnicodeText( x, yText, m_hTextFont, GetTeamColor( msg.Killer.iTeam ), killer );
			x += iKillerTextWide;
		}

		// Draw death icon
		if ( icon )
		{
			if( m_CritIcon && msg.bCritKilled )
			{
				surface()->DrawSetTexture( m_CritIcon->iTextureID );
				surface()->DrawSetColor( 255, 255, 255, 255 );
				surface()->DrawTexturedRect( x, yIcon, x + iconActualWide, yIcon + iconTall );
			}

			surface()->DrawSetTexture( icon->iTextureID );
			surface()->DrawSetColor( (msg.bLocalPlayerInvolved && !icon->bColored) ? m_clrLocalPlayer : m_clrIcon );
			surface()->DrawTexturedRect( x, yIcon, x + iconActualWide, yIcon + iconTall );

			x += iconWide;
		}

		// Draw additional info text next to death icon 
		if ( msg.wzInfoText[0] )
		{
			if ( msg.bSelfInflicted )
			{
				iDeathInfoOffset += iVictimTextWide;
				iVictimTextOffset -= iDeathInfoTextWide;
			}

			DrawUnicodeText( x + iDeathInfoOffset, yText, m_hTextFont, (msg.bLocalPlayerInvolved) ? m_clrLocalPlayer : m_clrIcon, msg.wzInfoText );
			x += iDeathInfoTextWide;
		}

		// Draw victims name
		DrawUnicodeText( x + iVictimTextOffset, yText, m_hTextFont, GetTeamColor( msg.Victim.iTeam ), victim );
		x += iVictimTextWide;
	}
}

//-----------------------------------------------------------------------------
// Purpose: draw text helper
//-----------------------------------------------------------------------------

void CTFHudDeathNotice::DrawUnicodeText( int x, int y, HFont hFont, Color clr, const wchar_t *szText )
{
	surface()->DrawSetTextPos( x, y );
	surface()->DrawSetTextColor( clr );
	surface()->DrawSetTextFont( hFont );	//reset the font, draw icon can change it
	surface()->DrawPrintText( szText, wcslen(szText) );
	surface()->DrawFlushText();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CTFHudDeathNotice::RetireExpiredDeathNotices()
{
	// Remove any expired death notices.  Loop backwards because we might remove one
	int iCount = m_DeathNotices.Count();
	for ( int i = iCount-1; i >= 0; i-- )
	{
		if ( engine->GetClientTime() > m_DeathNotices[i].GetExpiryTime() )
		{
			m_DeathNotices.Remove(i);
		}
	}

	// Do we have too many death messages in the queue?
	if ( m_DeathNotices.Count() > 0 &&
		m_DeathNotices.Count() > (int)m_flMaxDeathNotices )
	{		 
		// First, remove any notices not involving the local player, since they are lower priority.		
		iCount = m_DeathNotices.Count();
		int iNeedToRemove = iCount - (int)m_flMaxDeathNotices;
		// loop condition is iCount-1 because we won't remove the most recent death notice, otherwise
		// new non-local-player-involved messages would not appear if the queue was full of messages involving the local player
		for ( int i = 0; i < iCount-1 && iNeedToRemove > 0 ; i++ )
		{
			if ( !m_DeathNotices[i].bLocalPlayerInvolved )
			{
				m_DeathNotices.Remove( i );
				iCount--;
				iNeedToRemove--;
			}	
		}

		// Now that we've culled any non-local-player-involved messages up to the amount we needed to remove, see
		// if we've removed enough
		iCount = m_DeathNotices.Count();
		iNeedToRemove = iCount - (int)m_flMaxDeathNotices;
		if ( iNeedToRemove > 0 )		
		{
			// if we still have too many messages, then just remove however many we need, oldest first
			for ( int i = 0; i < iNeedToRemove; i++ )
			{
				m_DeathNotices.Remove( 0 );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new death notice to the queue
//-----------------------------------------------------------------------------
int CTFHudDeathNotice::AddDeathNoticeItem()
{
	int iMsg = m_DeathNotices.AddToTail();
	m_DeathNotices[iMsg].flCreationTime = engine->GetClientTime();
	return iMsg;
}

void CTFHudDeathNotice::GetBackgroundPolygonVerts( int x0, int y0, int x1, int y1, int iVerts, vgui::Vertex_t vert[] )
{
	Assert( iVerts == NUM_BACKGROUND_COORD );
	// use the offsets we generated for one corner and apply those to the passed-in dimensions to create verts for the poly
	for ( int i = 0; i < NUM_CORNER_COORD; i++ )
	{
		int j = ( NUM_CORNER_COORD-1 ) - i;
		// upper left corner
		vert[i].Init( Vector2D( x0 + m_CornerCoord[i].x, y0 + m_CornerCoord[i].y ) );
		// upper right corner
		vert[i+NUM_CORNER_COORD].Init( Vector2D( x1 - m_CornerCoord[j].x, y0 + m_CornerCoord[j].y ) );
		// lower right corner
		vert[i+(NUM_CORNER_COORD*2)].Init( Vector2D( x1 - m_CornerCoord[i].x, y1 - m_CornerCoord[i].y ) );
		// lower left corner
		vert[i+(NUM_CORNER_COORD*3)].Init( Vector2D( x0 + m_CornerCoord[j].x, y1 - m_CornerCoord[j].y) );
	}
}

void CTFHudDeathNotice::CalcRoundedCorners()
{
	// generate the offset geometry for upper left corner
	int iMax = ARRAYSIZE( m_CornerCoord );
	for ( int i = 0; i < iMax; i++ )
	{
		m_CornerCoord[i].x = m_flCornerRadius * ( 1 - cos( ( (float) i / (float) (iMax - 1 ) ) * ( M_PI / 2 ) ) );
		m_CornerCoord[i].y = m_flCornerRadius * ( 1 - sin( ( (float) i / (float) (iMax - 1 ) ) * ( M_PI / 2 ) ) );
	}
}

int CTFHudDeathNotice::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	if ( DEATHNOTICE_DISPLAY_TIME <= 0 )
	{
		return 0;
	}

	if(!strcmp(pszName, "DeathMsg"))
	{
		return MsgFunc_DeathMsg(pszName, iSize, pbuf);
	}
	if(!strcmp(pszName, "BuildDeath"))
	{
		return MsgFunc_BuildDeath(pszName, iSize, pbuf);
	}
	if(!strcmp(pszName, "ObjectMsg"))
	{
		return MsgFunc_ObjectMsg(pszName, iSize, pbuf);
	}
	if(!strcmp(pszName, "Dominate"))
	{
		return MsgFunc_Dominate(pszName, iSize, pbuf);
	}
	if(!strcmp(pszName, "Revenge"))
	{
		return MsgFunc_Revenge(pszName, iSize, pbuf);
	}

	return 0;
}

int CTFHudDeathNotice::MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iKillerID = READ_BYTE();
	int iAssisterID = READ_BYTE();
	int iVictimID = READ_BYTE();
	int iCritKill = READ_BYTE();
	char *szIconName = READ_STRING();

	int iLocalPlayerIndex = engine->GetLocalPlayer()->index;

	hud_player_info_t pi;

	DeathNoticeItem &msg = m_DeathNotices[AddDeathNoticeItem()];

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	bool bLocalPlayerInvolved = false;
	if ( iLocalPlayerIndex == iKillerID || iLocalPlayerIndex == iAssisterID || iLocalPlayerIndex == iVictimID )
	{
		bLocalPlayerInvolved = true;
	}
	msg.bLocalPlayerInvolved = bLocalPlayerInvolved;

	msg.bCritKilled = (iCritKill) ? true : false;

	msg.Killer.iTeam = gCKFVars.g_PlayerInfo[iKillerID].iTeam;
	msg.Victim.iTeam = gCKFVars.g_PlayerInfo[iVictimID].iTeam;

	if(iAssisterID)
	{
		char *pszKillerName = "";
		char *pszAssisterName = "";

		if(iKillerID)
		{
			engine->pfnGetPlayerInfo(iKillerID, &pi);
			if(pi.name && pi.name[0])
				pszKillerName = pi.name;
		}
		
		engine->pfnGetPlayerInfo(iAssisterID, &pi);
		if(pi.name && pi.name[0])
			pszAssisterName = pi.name;

		Q_snprintf( msg.Killer.szName, ARRAYSIZE( msg.Killer.szName ), "%s + %s", pszKillerName, pszAssisterName );
	}
	else
	{
		char *pszKillerName = "";

		if(iKillerID)
		{
			engine->pfnGetPlayerInfo(iKillerID, &pi);
			if(pi.name && pi.name[0])
				pszKillerName = pi.name;			
		}

		Q_strncpy( msg.Killer.szName, pszKillerName, ARRAYSIZE( msg.Killer.szName ) );
	}

	char *pszVictimName = "";

	if(iVictimID)
	{
		engine->pfnGetPlayerInfo(iVictimID, &pi);
		if(pi.name && pi.name[0])
			pszVictimName = pi.name;
	}

	Q_strncpy( msg.Victim.szName, pszVictimName, ARRAYSIZE( msg.Victim.szName ) );

	Q_strncpy( msg.szIcon, szIconName, ARRAYSIZE(msg.szIcon));

	if ( !iKillerID || iKillerID == iVictimID )
	{
		msg.bSelfInflicted = true;
		msg.Killer.szName[0] = 0;

		if ( !stricmp(szIconName, "worldspawn") )
		{
			// special case text for falling death
			V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#DeathMsg_Fall" ), sizeof( msg.wzInfoText ) );
		}
		else
		{
			V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#DeathMsg_Suicide" ), sizeof( msg.wzInfoText ) );
		}
	}

	msg.Icon = FindDeathIcon(msg.szIcon);

	if(!msg.Icon)
	{
		Q_snprintf(msg.szIcon, ARRAYSIZE( msg.szIcon ), (msg.bLocalPlayerInvolved) ? "%s_d" : "%s_b", szIconName );
		msg.Icon = FindDeathIcon(msg.szIcon);
	}

	if(!msg.Icon)
	{
		msg.Icon = FindDeathIcon("suicide");
	}

	return 1;
}


int CTFHudDeathNotice::MsgFunc_BuildDeath(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iKillerID = READ_BYTE();
	int iAssisterID = READ_BYTE();
	int iVictimID = READ_BYTE();
	int iBuildClass = READ_BYTE();
	char *szIconName = READ_STRING();

	int iLocalPlayerIndex = engine->GetLocalPlayer()->index;

	hud_player_info_t pi;

	DeathNoticeItem &msg = m_DeathNotices[AddDeathNoticeItem()];

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	bool bLocalPlayerInvolved = false;
	if ( iLocalPlayerIndex == iKillerID || iLocalPlayerIndex == iAssisterID || iLocalPlayerIndex == iVictimID )
	{
		bLocalPlayerInvolved = true;
	}
	msg.bLocalPlayerInvolved = bLocalPlayerInvolved;

	msg.bCritKilled = false;

	msg.Killer.iTeam = gCKFVars.g_PlayerInfo[iKillerID].iTeam;
	msg.Victim.iTeam = gCKFVars.g_PlayerInfo[iVictimID].iTeam;

	if(iAssisterID)
	{
		char *pszKillerName = "";
		char *pszAssisterName = "";

		if(iKillerID)
		{
			engine->pfnGetPlayerInfo(iKillerID, &pi);
			if(pi.name && pi.name[0])
				pszKillerName = pi.name;
		}
		
		engine->pfnGetPlayerInfo(iAssisterID, &pi);
		if(pi.name && pi.name[0])
			pszAssisterName = pi.name;

		Q_snprintf( msg.Killer.szName, ARRAYSIZE( msg.Killer.szName ), "%s + %s", pszKillerName, pszAssisterName );
	}
	else
	{
		char *pszKillerName = "";

		if(iKillerID)
		{
			engine->pfnGetPlayerInfo(iKillerID, &pi);
			if(pi.name && pi.name[0])
				pszKillerName = pi.name;			
		}

		Q_strncpy( msg.Killer.szName, pszKillerName, ARRAYSIZE( msg.Killer.szName ) );
	}

	char *pszVictimName = "";

	if(iVictimID)
	{
		engine->pfnGetPlayerInfo(iVictimID, &pi);
		if(pi.name && pi.name[0])
			pszVictimName = pi.name;
	}

	// get the localized name for the object
	char szLocalizedObjectName[MAX_PLAYER_NAME_LENGTH];
	szLocalizedObjectName[ 0 ] = 0;
	if(iBuildClass >= 1 && iBuildClass <= 5)
	{
		const wchar_t *wszLocalizedObjectName = g_pVGuiLocalize->Find( szLocalizedObjectNames[iBuildClass-1] );
		if ( wszLocalizedObjectName )
		{
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedObjectName, szLocalizedObjectName, ARRAYSIZE( szLocalizedObjectName ) );
		}
		else
		{
			Q_strncpy( szLocalizedObjectName, szLocalizedObjectNames[iBuildClass-1], sizeof( szLocalizedObjectName ) );
		}
	}

	Q_snprintf( msg.Victim.szName, ARRAYSIZE( msg.Victim.szName ), "%s (%s)", szLocalizedObjectName, pszVictimName );

	Q_strncpy( msg.szIcon, szIconName, ARRAYSIZE(msg.szIcon));

	if ( !iKillerID || iKillerID == iVictimID )
	{
		msg.bSelfInflicted = true;
		msg.Killer.szName[0] = 0;
	}

	msg.Icon = FindDeathIcon(msg.szIcon);

	if(!msg.Icon)
	{
		Q_snprintf(msg.szIcon, ARRAYSIZE( msg.szIcon ), (msg.bLocalPlayerInvolved) ? "%s_d" : "%s_b", szIconName );
		msg.Icon = FindDeathIcon(msg.szIcon);
	}

	if(!msg.Icon)
	{
		msg.Icon = FindDeathIcon("suicide");
	}

	return 1;
}

int CTFHudDeathNotice::MsgFunc_ObjectMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iObjectIndex = READ_BYTE();
	int iObjectAction = READ_BYTE();
	int iKillerTeam = READ_BYTE();
	int iVictimTeam = READ_BYTE();
	int iCapturerNum = READ_BYTE();
	int iCapturersID[5];
	if(iCapturerNum > 100)
	{
		iCapturersID[0] = iCapturerNum - 100;
		iCapturerNum = 1;
	}
	else
	{
		for(int i = 0; i < iCapturerNum; ++i)
		{
			iCapturersID[i] = READ_BYTE();
		}
	}

	char *szIconName = "";

	if(iObjectIndex < 1 || iObjectIndex > g_pCKFClient->GetControlPointCount())
		return 1;

	controlpoint_t *point = g_pCKFClient->GetControlPoint(iObjectIndex - 1);

	int iLocalPlayerIndex = engine->GetLocalPlayer()->index;

	hud_player_info_t pi;

	DeathNoticeItem &msg = m_DeathNotices[AddDeathNoticeItem()];

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	bool bLocalPlayerInvolved = false;
	if ( gCKFVars.g_PlayerInfo[iLocalPlayerIndex].iTeam == iKillerTeam || gCKFVars.g_PlayerInfo[iLocalPlayerIndex].iTeam == iKillerTeam )
	{
		bLocalPlayerInvolved = true;
	}
	msg.bLocalPlayerInvolved = bLocalPlayerInvolved;

	msg.bCritKilled = false;

	msg.Killer.iTeam = iKillerTeam;
	msg.Victim.iTeam = iVictimTeam;

	for(int i = 0; i < iCapturerNum; ++i)
	{
		char *pszCapturerName = "";

		engine->pfnGetPlayerInfo(iCapturersID[i], &pi);
		if(pi.name && pi.name[0])
			pszCapturerName = pi.name;

		if(i != 0)
		{
			Q_strncat( msg.Killer.szName, " + ", ARRAYSIZE( msg.Killer.szName ));
		}
		Q_strncat( msg.Killer.szName, pszCapturerName, ARRAYSIZE( msg.Killer.szName ));
	}

	// get the localized name for the object
	char szLocalizedObjectName[MAX_PLAYER_NAME_LENGTH];
	szLocalizedObjectName[ 0 ] = 0;
	if(iObjectIndex)
	{
		const wchar_t *wszLocalizedObjectName = g_pVGuiLocalize->Find( point->szName );
		if ( wszLocalizedObjectName )
		{
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedObjectName, szLocalizedObjectName, ARRAYSIZE( szLocalizedObjectName ) );
		}
		else
		{
			Q_strncpy( szLocalizedObjectName, point->szName, sizeof( szLocalizedObjectName ) );
		}
	}

	if(iObjectAction == 0)
	{
		szIconName = (iKillerTeam == TEAM_RED) ? "redcap" : "blucap";

		V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#Msg_Captured" ), sizeof( msg.wzInfoText ) );
	}
	else if(iObjectAction == 1)
	{
		szIconName = (iKillerTeam == TEAM_RED) ? "reddef" : "bludef";

		V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#Msg_Defended" ), sizeof( msg.wzInfoText ) );
	}

	Q_strncpy( msg.Victim.szName, szLocalizedObjectName, ARRAYSIZE( msg.Victim.szName ) );

	Q_strncpy( msg.szIcon, szIconName, ARRAYSIZE(msg.szIcon));

	msg.Icon = FindDeathIcon(msg.szIcon);

	if(!msg.Icon)
	{
		Q_snprintf(msg.szIcon, ARRAYSIZE( msg.szIcon ), (msg.bLocalPlayerInvolved) ? "%s_d" : "%s_b", szIconName );
		msg.Icon = FindDeathIcon(msg.szIcon);
	}

	if(!msg.Icon)
	{
		msg.Icon = FindDeathIcon("suicide");
	}

	return 1;
}

int CTFHudDeathNotice::MsgFunc_Dominate(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iKillerID = READ_BYTE();
	int iVictimID = READ_BYTE();
	char *szIconName = "dom";

	int iLocalPlayerIndex = engine->GetLocalPlayer()->index;

	hud_player_info_t pi;

	DeathNoticeItem &msg = m_DeathNotices[AddDeathNoticeItem()];

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	bool bLocalPlayerInvolved = false;
	if ( iLocalPlayerIndex == iKillerID || iLocalPlayerIndex == iVictimID )
	{
		bLocalPlayerInvolved = true;
	}
	msg.bLocalPlayerInvolved = bLocalPlayerInvolved;

	msg.bCritKilled = false;

	msg.Killer.iTeam = gCKFVars.g_PlayerInfo[iKillerID].iTeam;
	msg.Victim.iTeam = gCKFVars.g_PlayerInfo[iVictimID].iTeam;

	char *pszKillerName = "";

	if(iKillerID)
	{
		engine->pfnGetPlayerInfo(iKillerID, &pi);
		if(pi.name && pi.name[0])
			pszKillerName = pi.name;			
	}

	Q_strncpy( msg.Killer.szName, pszKillerName, ARRAYSIZE( msg.Killer.szName ) );

	char *pszVictimName = "";

	if(iVictimID)
	{
		engine->pfnGetPlayerInfo(iVictimID, &pi);
		if(pi.name && pi.name[0])
			pszVictimName = pi.name;
	}

	Q_strncpy( msg.Victim.szName, pszVictimName, ARRAYSIZE( msg.Victim.szName ) );

	Q_strncpy( msg.szIcon, szIconName, ARRAYSIZE(msg.szIcon));

	V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#Msg_Dominating" ), sizeof( msg.wzInfoText ) );

	msg.Icon = FindDeathIcon(msg.szIcon);

	if(!msg.Icon)
	{
		Q_snprintf(msg.szIcon, ARRAYSIZE( msg.szIcon ), (msg.bLocalPlayerInvolved) ? "%s_d" : "%s_b", szIconName );
		msg.Icon = FindDeathIcon(msg.szIcon);
	}

	if(!msg.Icon)
	{
		msg.Icon = FindDeathIcon("suicide");
	}

	if ( iLocalPlayerIndex == iKillerID )
		engine->pEventAPI->EV_PlaySound(iLocalPlayerIndex, engine->GetLocalPlayer()->origin, CHAN_STATIC, "CKF_III/tf_domination.wav", 1, 1, 0, 100);

	if ( iLocalPlayerIndex == iVictimID )
		engine->pEventAPI->EV_PlaySound(iLocalPlayerIndex, engine->GetLocalPlayer()->origin, CHAN_STATIC, "CKF_III/tf_nemesis.wav", 1, 1, 0, 100);

	return 1;
}

int CTFHudDeathNotice::MsgFunc_Revenge(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iKillerID = READ_BYTE();
	int iVictimID = READ_BYTE();
	char *szIconName = "dom";

	int iLocalPlayerIndex = engine->GetLocalPlayer()->index;

	hud_player_info_t pi;

	DeathNoticeItem &msg = m_DeathNotices[AddDeathNoticeItem()];

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	bool bLocalPlayerInvolved = false;
	if ( iLocalPlayerIndex == iKillerID || iLocalPlayerIndex == iVictimID )
	{
		bLocalPlayerInvolved = true;
	}
	msg.bLocalPlayerInvolved = bLocalPlayerInvolved;

	msg.bCritKilled = false;

	msg.Killer.iTeam = gCKFVars.g_PlayerInfo[iKillerID].iTeam;
	msg.Victim.iTeam = gCKFVars.g_PlayerInfo[iVictimID].iTeam;

	char *pszKillerName = "";

	if(iKillerID)
	{
		engine->pfnGetPlayerInfo(iKillerID, &pi);
		if(pi.name && pi.name[0])
			pszKillerName = pi.name;			
	}

	Q_strncpy( msg.Killer.szName, pszKillerName, ARRAYSIZE( msg.Killer.szName ) );

	char *pszVictimName = "";

	if(iVictimID)
	{
		engine->pfnGetPlayerInfo(iVictimID, &pi);
		if(pi.name && pi.name[0])
			pszVictimName = pi.name;
	}

	Q_strncpy( msg.Victim.szName, pszVictimName, ARRAYSIZE( msg.Victim.szName ) );

	Q_strncpy( msg.szIcon, szIconName, ARRAYSIZE(msg.szIcon));

	V_wcsncpy( msg.wzInfoText, g_pVGuiLocalize->Find( "#Msg_Revenge" ), sizeof( msg.wzInfoText ) );

	msg.Icon = FindDeathIcon(msg.szIcon);

	if(!msg.Icon)
	{
		Q_snprintf(msg.szIcon, ARRAYSIZE( msg.szIcon ), (msg.bLocalPlayerInvolved) ? "%s_d" : "%s_b", szIconName );
		msg.Icon = FindDeathIcon(msg.szIcon);
	}

	if(!msg.Icon)
	{
		msg.Icon = FindDeathIcon("suicide");
	}

	if(bLocalPlayerInvolved)
	{
		engine->pEventAPI->EV_PlaySound(iLocalPlayerIndex, engine->GetLocalPlayer()->origin, CHAN_STATIC, "CKF_III/tf_revenge.wav", 1, 1, 0, 100);
	}
	return 1;
}

Color CTFHudDeathNotice::GetTeamColor( int iTeamNumber )
{
	switch ( iTeamNumber )
	{
	case TEAM_BLUE:
		return m_clrBlueText;
		break;
	case TEAM_RED:
		return m_clrRedText;
		break;
	case TEAM_UNASSIGNED:		
		return Color( 255, 255, 255, 255 );
		break;
	default:
		return Color( 255, 255, 255, 255 );
		break;
	}
}

float DeathNoticeItem::GetExpiryTime()
{
	float flDuration = max(DEATHNOTICE_DISPLAY_TIME, 0);
	if ( bLocalPlayerInvolved )
	{
		// if the local player is involved, make the message last longer
		flDuration *= 2;
	}
	return flCreationTime + flDuration;
}

}