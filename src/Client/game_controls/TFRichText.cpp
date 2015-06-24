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

#include <vgui_controls/RichText.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ScrollBarSlider.h>
#include <vgui_controls/Controls.h>
#include <game_controls/TFRichText.h>

using namespace vgui;

TFRichText::TFRichText( Panel *parent, const char *name ) : RichText( parent, name )
{
	m_szFont[0] = '\0';
	m_szColor[0] = '\0';

	SetCursor(dc_arrow);

	m_pUpArrow = new ImagePanel( this, "UpArrow" );
	if ( m_pUpArrow )
	{
		m_pUpArrow->SetShouldScaleImage( true );
		m_pUpArrow->SetImage( "gfx\\vgui\\chalkboard_scroll_up" );
		m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pUpArrow->SetAlpha( 255 );
		m_pUpArrow->SetVisible( false );
	}

	m_pLine = new ImagePanel( this, "Line" );
	if ( m_pLine )
	{
		m_pLine->SetShouldScaleImage( true );
		m_pLine->SetImage( "gfx\\vgui\\chalkboard_scroll_line" );
		m_pLine->SetVisible( false );
	}

	m_pDownArrow = new ImagePanel( this, "DownArrow" );
	if ( m_pDownArrow )
	{
		m_pDownArrow->SetShouldScaleImage( true );
		m_pDownArrow->SetImage( "gfx\\vgui\\chalkboard_scroll_down" );
		m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pDownArrow->SetAlpha( 255 );
		m_pDownArrow->SetVisible( false );
	}

	m_pBox = new ImagePanel( this, "Box" );
	if ( m_pBox )
	{
		m_pBox->SetShouldScaleImage( true );
		m_pBox->SetImage( "gfx\\vgui\\chalkboard_scroll_box" );
		m_pBox->SetVisible( false );
	}

	ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFRichText::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szFont, inResourceData->GetString( "font", "Default" ), sizeof( m_szFont ) );
	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "RichText.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFRichText::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFont( pScheme->GetFont( m_szFont, true ) );
	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );

	SetBorder( pScheme->GetBorder( "NoBorder" ) );
	SetBgColor( pScheme->GetColor( "Blank", Color( 0,0,0,0 ) ) );
	SetPanelInteractive( false );
	SetUnusedScrollbarInvisible( true );

	if ( m_pDownArrow  )
	{
		m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
	}

	if ( m_pUpArrow  )
	{
		m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
	}

	SetScrollBarImagesVisible( false );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFRichText::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( _vertScrollBar && _vertScrollBar->IsVisible() )
	{
		int nMin, nMax;
		_vertScrollBar->GetRange( nMin, nMax );
		_vertScrollBar->SetValue( nMin );

		int nScrollbarWide = _vertScrollBar->GetWide();

		int wide, tall;
		GetSize( wide, tall );

		if ( m_pUpArrow )
		{
			m_pUpArrow->SetBounds( wide - nScrollbarWide, 0, nScrollbarWide, nScrollbarWide );
		}

		if ( m_pLine )
		{
			m_pLine->SetBounds( wide - nScrollbarWide, nScrollbarWide, nScrollbarWide, tall - ( 2 * nScrollbarWide ) );
		}

		if ( m_pBox )
		{
			m_pBox->SetBounds( wide - nScrollbarWide, nScrollbarWide, nScrollbarWide, nScrollbarWide );
		}

		if ( m_pDownArrow )
		{
			m_pDownArrow->SetBounds( wide - nScrollbarWide, tall - nScrollbarWide, nScrollbarWide, nScrollbarWide );
		}

		SetScrollBarImagesVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFRichText::SetScrollBarImagesVisible( bool visible )
{
	if ( m_pDownArrow && m_pDownArrow->IsVisible() != visible )
	{
		m_pDownArrow->SetVisible( visible );
	}

	if ( m_pUpArrow && m_pUpArrow->IsVisible() != visible )
	{
		m_pUpArrow->SetVisible( visible );
	}

	if ( m_pLine && m_pLine->IsVisible() != visible )
	{
		m_pLine->SetVisible( visible );
	}

	if ( m_pBox && m_pBox->IsVisible() != visible )
	{
		m_pBox->SetVisible( visible );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFRichText::OnTick()
{
	if ( !IsVisible() )
		return;

	if ( m_pDownArrow && m_pUpArrow && m_pLine && m_pBox )
	{
		if ( _vertScrollBar && _vertScrollBar->IsVisible() )
		{
			_vertScrollBar->SetZPos( 500 );

			// turn off painting the vertical scrollbar
			_vertScrollBar->SetPaintBackgroundEnabled( false );
			_vertScrollBar->SetPaintBorderEnabled( false );
			_vertScrollBar->SetPaintEnabled( false );
			_vertScrollBar->SetScrollbarButtonsVisible( false );
			_vertScrollBar->GetButton(0)->SetPaintBorderEnabled(false);
			_vertScrollBar->GetButton(1)->SetPaintBorderEnabled(false);

			// turn on our own images
			SetScrollBarImagesVisible ( true );

			// set the alpha on the up arrow
			int nMin, nMax;
			_vertScrollBar->GetRange( nMin, nMax );
			int nScrollPos = _vertScrollBar->GetValue();
			int nRangeWindow = _vertScrollBar->GetRangeWindow();
			int nBottom = nMax - nRangeWindow;
			if ( nBottom < 0 )
			{
				nBottom = 0;
			}

			// set the alpha on the up arrow
			int nAlpha = ( nScrollPos - nMin <= 0 ) ? 90 : 255;
			m_pUpArrow->SetAlpha( nAlpha );

			// set the alpha on the down arrow
			nAlpha = ( nScrollPos >= nBottom ) ? 90 : 255;
			m_pDownArrow->SetAlpha( nAlpha );

			ScrollBarSlider *pSlider = _vertScrollBar->GetSlider();
			if ( pSlider && pSlider->GetRangeWindow() > 0 )
			{
				int x, y, w, t, min, max;
				m_pLine->GetBounds( x, y, w, t );
				pSlider->GetNobPos( min, max );

				m_pBox->SetBounds( x, y + min, w, ( max - min ) );
			}
		}
		else
		{
			// turn off our images
			SetScrollBarImagesVisible ( false );
		}
	}
}