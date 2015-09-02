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
#include <game_controls/TFHudPanel.h>

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include <ICKFClient.h>

extern ICKFClient *g_pCKFClient;

using namespace vgui;

TFHudPanel::TFHudPanel( Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_iColorIndex = 0;
	m_iColorAlpha = 255;

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFHudPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	SetColorIndex(inResourceData->GetInt( "colorindex", 0 ));
	SetColorAlpha(inResourceData->GetInt( "coloralpha", 255 ));
}

void TFHudPanel::SetColorIndex( int iColorIndex )
{
	m_iColorIndex = iColorIndex;
}

void TFHudPanel::SetColorAlpha( int iColorAlpha )
{
	m_iColorAlpha = iColorAlpha;
}

void TFHudPanel::Paint(void)
{
	if(m_iColorIndex == 0)
		surface()->DrawSetColor(Color( 41, 41, 41, m_iColorAlpha ));
	else
		surface()->DrawSetColor(Color( 255, 255, 255, m_iColorAlpha ));

	int w, h;
	GetSize(w, h);

	g_pCKFClient->DrawHudMask(m_iColorIndex, 0, 0, w, h);
}