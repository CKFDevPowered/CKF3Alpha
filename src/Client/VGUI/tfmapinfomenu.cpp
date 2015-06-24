#include "hud.h"
#include "cl_util.h"
#include "tfmapinfomenu.h"
#include "tftextwindow.h"
#include "configs.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <tier1/byteswap.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ScalableImagePanel.h>

#include <game_controls/TFRichText.h>

#define MAX_MAP_NAME 40

using namespace vgui;

const char *GetMapDisplayName( const char *mapName );
const char *GetMapDescription( const char *mapName );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFMapInfoMenu::CTFMapInfoMenu() : Frame( NULL, PANEL_MAPINFO )
{
	// load the new scheme early!!
	SetScheme( "ClientScheme" );
	
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );
	SetProportional( true );
	SetVisible( false );
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );
	SetPaintBackgroundEnabled(false);

	m_pTitle = new Label( this, "MapInfoTitle", " " );

	m_pContinue = new Button( this, "MapInfoContinue", "#CKF3_Continue" );
	m_pBack = new Button( this, "MapInfoBack", "#CKF3_Back" );

	// info window about this map
	m_pMapInfo = new TFRichText( this, "MapInfoText" );
	m_pMapImage = new ImagePanel( this, "MapImage" );

	m_szMapName[0] = 0;

	LoadControlSettings("Resource/UI/TFMapInfoMenu.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFMapInfoMenu::~CTFMapInfoMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	CheckBackContinueButtons();

	char mapname[MAX_MAP_NAME];

	Q_FileBase( engine->pfnGetLevelName(), mapname, sizeof(mapname) );

	// Save off the map name so we can re-load the page in ApplySchemeSettings().
	Q_strncpy( m_szMapName, mapname, sizeof( m_szMapName ) );
	Q_strupr( m_szMapName );

	LoadMapPage( m_szMapName );
	SetMapTitle();

	if ( m_pContinue )
	{
		m_pContinue->RequestFocus();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::ShowPanel( bool bShow )
{
	if ( IsVisible() == bShow )
		return;

	m_KeyRepeat.Reset();

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::CheckBackContinueButtons()
{
	if ( m_pBack && m_pContinue )
	{
		if ( g_iTeamNumber == TEAM_UNASSIGNED )
		{
			m_pBack->SetVisible( true );
			m_pContinue->SetText( "#CKF3_Continue" );
		}
		else
		{
			m_pBack->SetVisible( false );
			m_pContinue->SetText( "#CKF3_Close" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::OnCommand( const char *command )
{
	m_KeyRepeat.Reset();

	if ( !Q_strcmp( command, "back" ) )
	{
		 // only want to go back to the Welcome menu if we're not already on a team
		if ( g_iTeamNumber == TEAM_UNASSIGNED )
		{
			g_pViewPort->ShowPanel( PANEL_INFO, true );
			g_pViewPort->ShowPanel(this, false);
		}
	}
	else if ( !Q_strcmp( command, "continue" ) )
	{
		g_pViewPort->ShowPanel(this, false);
		ServerCmd("chooseteam\n");
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::Update()
{ 
	InvalidateLayout( false, true );
}

void CTFMapInfoMenu::Init(void)
{

}

void CTFMapInfoMenu::VidInit(void)
{

}

//-----------------------------------------------------------------------------
// Purpose: chooses and loads the text page to display that describes mapName map
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::LoadMapPage( const char *mapName )
{
	char szMapImage[MAX_PATH];
	// load the map image (if it exists for the current map)
	if ( m_pMapImage )
	{
		if ( !m_pMapImage->IsVisible() )
		{
			m_pMapImage->SetVisible( true );
		}

		m_pMapImage->SetBgColor(Color(0, 0, 0, 0));

		// take off the vgui/ at the beginning when we set the image
		Q_snprintf( szMapImage, sizeof( szMapImage ), "gfx/vgui/maps/menu_photos_%s", mapName );
		Q_strlower( szMapImage );

		m_pMapImage->SetImage( szMapImage );
	}

	// load the map description files
	char mapRES[ MAX_PATH ];

	Q_snprintf( mapRES, sizeof( mapRES ), "maps/%s_%s.txt", mapName, gConfigs.szLanguage );

	// try English if the file doesn't exist for our language
	if( !g_pFullFileSystem->FileExists( mapRES ) )
	{
		Q_snprintf( mapRES, sizeof( mapRES ), "maps/%s_english.txt", mapName );

		// if the file doesn't exist for English either, try the filename without any language extension
		if( !g_pFullFileSystem->FileExists( mapRES ) )
		{
			Q_snprintf( mapRES, sizeof( mapRES ), "maps/%s.txt", mapName );
		}
	}

	// if no map specific description exists, load default text
	if( !g_pFullFileSystem->FileExists( mapRES ) )
	{
		m_pMapInfo->SetText( GetMapDescription(mapName) );
		// we haven't loaded a valid map image for the current map
		if ( m_pMapImage && !m_pMapImage->IsVisible() )
		{
			if ( m_pMapInfo )
			{
				m_pMapInfo->SetWide( m_pMapInfo->GetWide() + ( m_pMapImage->GetWide() * 0.75 ) ); // add in the extra space the images would have taken 
			}
		}
		return;
	}

	FileHandle_t f = g_pFullFileSystem->Open( mapRES, "rb" );

	// read into a memory block
	int fileSize = g_pFullFileSystem->Size(f);
	int dataSize = fileSize + sizeof( wchar_t );
	if ( dataSize % 2 )
		++dataSize;
	wchar_t *memBlock = (wchar_t *)malloc(dataSize);
	memset( memBlock, 0x0, dataSize);
	int bytesRead = g_pFullFileSystem->Read(memBlock, fileSize, f);
	if ( bytesRead < fileSize )
	{
		// NULL-terminate based on the length read in, since Read() can transform \r\n to \n and
		// return fewer bytes than we were expecting.
		char *data = reinterpret_cast<char *>( memBlock );
		data[ bytesRead ] = 0;
		data[ bytesRead+1 ] = 0;
	}

	// null-terminate the stream (redundant, since we memset & then trimmed the transformed buffer already)
	memBlock[dataSize / sizeof(wchar_t) - 1] = 0x0000;

	// check the first character, make sure this a little-endian unicode file

#if defined( _X360 )
	if ( memBlock[0] != 0xFFFE )
#else
	if ( memBlock[0] != 0xFEFF )
#endif
	{
		// its a ascii char file
		m_pMapInfo->SetText( reinterpret_cast<char *>( memBlock ) );
	}
	else
	{
		// ensure little-endian unicode reads correctly on all platforms
		CByteswap byteSwap;
		byteSwap.SetTargetBigEndian( false );
		byteSwap.SwapBufferToTargetEndian( memBlock, memBlock, dataSize/sizeof(wchar_t) );

		m_pMapInfo->SetText( memBlock+1 );
	}
	// go back to the top of the text buffer
	m_pMapInfo->GotoTextStart();

	g_pFullFileSystem->Close( f );
	free(memBlock);

	// we haven't loaded a valid map image for the current map
	if ( m_pMapImage && !m_pMapImage->IsVisible() )
	{
		if ( m_pMapInfo )
		{
			m_pMapInfo->SetWide( m_pMapInfo->GetWide() + ( m_pMapImage->GetWide() * 0.75 ) ); // add in the extra space the images would have taken 
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::SetMapTitle()
{
	m_pTitle->SetText( GetMapDisplayName(m_szMapName) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::OnKeyCodePressed( KeyCode code )
{
	m_KeyRepeat.KeyDown( code );

	if ( code == KEY_C )
	{
		OnCommand( "continue" );
	}
	else if ( code == KEY_B )
	{
		OnCommand( "back" );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::OnKeyCodeReleased( vgui::KeyCode code )
{
	m_KeyRepeat.KeyUp( code );

	BaseClass::OnKeyCodeReleased( code );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMapInfoMenu::OnThink()
{
	vgui::KeyCode code = m_KeyRepeat.KeyRepeated();
	if ( code )
	{
		OnKeyCodePressed( code );
	}

	BaseClass::OnThink();
}

struct s_MapInfo
{
	const char	*pDiskName;
	const char	*pDisplayName;
	const char	*pMapDesc;
};

static s_MapInfo s_Maps[] = {
	"ctf_2fort",	"2Fort",		"#CKF3_MapDesc_CP",
	"cp_dustbowl",	"Dustbowl",		"#CKF3_MapDesc_AttackDefend",
	"cp_granary",	"Granary",		"#CKF3_MapDesc_CP",
	"cp_well",		"Well (CP)",	"#CKF3_MapDesc_CP",
	"cp_gravelpit", "Gravel Pit",	"#CKF3_MapDesc_AttackDefend",
	"cp_orangesdk", "Orange SDK",	"#CKF3_MapDesc_CP",
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *GetMapDisplayName( const char *mapName )
{
	static char szDisplayName[256];
	char szTempName[256];
	const char *pszSrc = NULL;

	szDisplayName[0] = '\0';

	if ( !mapName )
		return szDisplayName;

	// check our lookup table
	Q_strncpy( szTempName, mapName, sizeof( szTempName ) );
	Q_strlower( szTempName );

	for ( int i = 0; i < ARRAYSIZE( s_Maps ); ++i )
	{
		if ( !Q_stricmp( s_Maps[i].pDiskName, szTempName ) )
		{
			return s_Maps[i].pDisplayName;
		}
	}

	// we haven't found a "friendly" map name, so let's just clean up what we have
	if ( !Q_strncmp( szTempName, "cp_", 3 ) ||
		 !Q_strncmp( szTempName, "tc_", 3 ) ||
		 !Q_strncmp( szTempName, "ad_", 3 ) )
	{
		pszSrc = szTempName + 3;
	}
	else if ( !Q_strncmp( szTempName, "ctf_", 4 ) )
	{
		pszSrc = szTempName + 4;
	}
	else
	{
		pszSrc = szTempName;
	}

	Q_strncpy( szDisplayName, pszSrc, sizeof( szDisplayName ) );
	Q_strupr( szDisplayName );

	return szDisplayName;
}

const char *GetMapDescription( const char *mapName )
{
	char szTempName[256];
	const char *pszSrc = NULL;

	if ( !mapName )
		return "";

	// check our lookup table
	Q_strncpy( szTempName, mapName, sizeof( szTempName ) );
	Q_strlower( szTempName );

	for ( int i = 0; i < ARRAYSIZE( s_Maps ); ++i )
	{
		if ( !Q_stricmp( s_Maps[i].pDiskName, szTempName ) )
		{
			return s_Maps[i].pMapDesc;
		}
	}

	// we haven't found a "friendly" map name, so let's just clean up what we have
	if ( !Q_strncmp( szTempName, "cp_", 3 ) )
		return "#CKF3_MapDesc_CP";
	else if ( !Q_strncmp( szTempName, "ctf_", 4 ) )
		return "#CKF3_MapDesc_CTF";
	
	return "";
}