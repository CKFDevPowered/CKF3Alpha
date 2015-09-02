//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <UtlVector.h>

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include <vgui/Cursor.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/Controls.h>
#include <game_controls/TFClassTips.h>

using namespace vgui;

TFClassTipsItemPanel::TFClassTipsItemPanel( Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_pIcon = new ImagePanel( this, "TipIcon" );
	m_pIcon->SetImage( "gfx\\vgui\\class\\chalf_circle" );		

	m_pLabel = new Label( this, "TipLabel", "");

	LoadControlSettings("classes/classtipsitem.res");

	m_bShouldSkipAutoResize = true;
}

void TFClassTipsItemPanel::SetText(const char *text)
{
	m_pLabel->SetText(text);
}

void TFClassTipsItemPanel::SetText(const wchar_t *text)
{
	m_pLabel->SetText(text);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFClassTipsItemPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFClassTipsItemPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( pScheme->GetBorder( "NoBorder" ) );
	SetBgColor( pScheme->GetColor( "Blank", Color( 0,0,0,0 ) ) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFClassTipsItemPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}