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

#include <vgui_controls/Image.h>
#include <game_controls/TFImageButton.h>

using namespace vgui;

DECLARE_BUILD_FACTORY_PANEL(TFImageButton)

TFImageButton::TFImageButton(Panel *parent, const char *name, EditablePanel *panel) : BaseClass(parent, name, panel)
{
	m_szDefaultImage = NULL;
	m_szArmedImage = NULL;
	m_szSelectedImage = NULL;
	m_pszDrawColorName = NULL;
	m_pszArmedColorName = NULL;
	m_pszSelectedColorName = NULL;

	m_pDefaultImage = NULL;
	m_pArmedImage = NULL;
	m_pSelectedImage = NULL;

	m_DrawColor = Color(255,255,255,255);
	m_ArmedColor = Color(255,255,255,255);
	m_SelectedColor = Color(255,255,255,255);

	REGISTER_COLOR_AS_OVERRIDABLE( m_DrawColor, "drawcolor_override" );
}

TFImageButton::~TFImageButton()
{
	if(m_szDefaultImage)
		delete [] m_szDefaultImage;
	if(m_szArmedImage)
		delete [] m_szArmedImage;
	if(m_szSelectedImage)
		delete [] m_szSelectedImage;
	if(m_pszDrawColorName)
		delete [] m_pszDrawColorName;
	if(m_pszArmedColorName)
		delete [] m_pszArmedColorName;
	if(m_pszSelectedColorName)
		delete [] m_pszSelectedColorName;
}

void TFImageButton::SetDefaultImage(IImage *image)
{
	m_pDefaultImage = image;
	Repaint();
}

void TFImageButton::SetDefaultImage(const char *imageName)
{
	if ( imageName && m_szDefaultImage && V_stricmp( imageName, m_szDefaultImage ) == 0 )
		return;

	int len = Q_strlen(imageName) + 1;
	if(m_szDefaultImage)
		delete [] m_szDefaultImage;
	m_szDefaultImage = new char[ len ];
	Q_strncpy(m_szDefaultImage, imageName, len );

	InvalidateLayout(false, true);
}

void TFImageButton::SetArmedImage(IImage *image)
{
	m_pArmedImage = image;
	Repaint();
}

void TFImageButton::SetArmedImage(const char *imageName)
{
	if ( imageName && m_szArmedImage && V_stricmp( imageName, m_szArmedImage ) == 0 )
		return;

	int len = Q_strlen(imageName) + 1;
	if(m_szArmedImage)
		delete [] m_szArmedImage;
	m_szArmedImage = new char[ len ];
	Q_strncpy(m_szArmedImage, imageName, len );

	InvalidateLayout(false, true);
}

void TFImageButton::SetSelectedImage(IImage *image)
{
	m_pSelectedImage = image;
	Repaint();
}

void TFImageButton::SetSelectedImage(const char *imageName)
{
	if ( imageName && m_szSelectedImage && V_stricmp( imageName, m_szSelectedImage ) == 0 )
		return;

	int len = Q_strlen(imageName) + 1;
	if(m_szSelectedImage)
		delete [] m_szSelectedImage;
	m_szSelectedImage = new char[ len ];
	Q_strncpy(m_szSelectedImage, imageName, len );

	InvalidateLayout(false, true);
}

IImage *TFImageButton::GetDefaultImage()
{
	return m_pDefaultImage;
}

IImage *TFImageButton::GetArmedImage()
{
	return m_pArmedImage;
}

IImage *TFImageButton::GetSelectedImage()
{
	return m_pSelectedImage;
}

Color TFImageButton::GetDrawColor( void )
{
	return m_DrawColor;
}

void TFImageButton::SetDrawColor( Color drawColor )
{
	m_DrawColor = drawColor;
}

Color TFImageButton::GetArmedColor( void )
{
	return m_ArmedColor;
}

void TFImageButton::SetArmedColor( Color armedColor )
{
	m_ArmedColor = armedColor;
}

Color TFImageButton::GetSelectedColor( void )
{
	return m_SelectedColor;
}

void TFImageButton::SetSelectedColor( Color selectedColor )
{
	m_SelectedColor = selectedColor;
}

void TFImageButton::PaintBackground()
{
	IImage *pImage = m_pDefaultImage;
	Color col = m_DrawColor;

	if(IsArmed() && m_pArmedImage)
	{
		pImage = m_pArmedImage;
		col = m_ArmedColor;
	}
	else if(IsSelected() && m_pSelectedImage)
	{
		pImage = m_pSelectedImage;
		col = m_SelectedColor;
	}

	if ( pImage )
	{
		pImage->SetColor( col );

		int imageWide, imageTall;
		pImage->GetSize( imageWide, imageTall );

		int wide, tall;
		GetSize( wide, tall );
		pImage->SetSize( wide, tall );

		pImage->Paint();

		pImage->SetSize( imageWide, imageTall );
	}
}

void TFImageButton::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	if (m_szDefaultImage)
	{
		outResourceData->SetString("defaultimage", m_szDefaultImage);
	}
	if (m_szArmedImage)
	{
		outResourceData->SetString("armedimage", m_szArmedImage);
	}
	if (m_szSelectedImage)
	{
		outResourceData->SetString("selectedimage", m_szSelectedImage);
	}
	if (m_pszDrawColorName)
	{
		outResourceData->SetString("drawcolor", m_pszDrawColorName);
	}
	if (m_pszArmedColorName)
	{
		outResourceData->SetString("armedcolor", m_pszArmedColorName);
	}
	if (m_pszSelectedColorName)
	{
		outResourceData->SetString("selectedcolor", m_pszSelectedColorName);
	}
}

void TFImageButton::ApplySettings(KeyValues *inResourceData)
{
	if(m_szDefaultImage)
		delete [] m_szDefaultImage;
	if(m_szArmedImage)
		delete [] m_szArmedImage;
	if(m_szSelectedImage)
		delete [] m_szSelectedImage;

	m_szDefaultImage = NULL;
	m_szArmedImage = NULL;
	m_szSelectedImage = NULL;

	const char *defaultImageName = inResourceData->GetString("defaultimage", "");
	if ( *defaultImageName )
	{
		SetDefaultImage( defaultImageName );
	}

	const char *armedImageName = inResourceData->GetString("armedimage", "");
	if ( *armedImageName )
	{
		SetArmedImage( armedImageName );
	}

	const char *selectedImageName = inResourceData->GetString("selectedimage", "");
	if ( *selectedImageName )
	{
		SetSelectedImage( selectedImageName );
	}

	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
	const char *pszDrawColor = inResourceData->GetString("drawcolor", "");
	if (*pszDrawColor)
	{
		int r = 255, g = 255, b = 255, a = 255;
		int len = Q_strlen(pszDrawColor) + 1;
		m_pszDrawColorName = new char[ len ];
		Q_strncpy( m_pszDrawColorName, pszDrawColor, len );

		if (sscanf(pszDrawColor, "%d %d %d %d", &r, &g, &b, &a) >= 3)
		{
			// it's a direct color
			m_DrawColor = Color(r, g, b, a);
		}
		else
		{
			m_DrawColor = pScheme->GetColor(pszDrawColor, Color(255, 255, 255, 255));
		}
	}
	const char *pszArmedColor = inResourceData->GetString("armedcolor", "");
	if (*pszArmedColor)
	{
		int r = 255, g = 255, b = 255, a = 255;
		int len = Q_strlen(pszArmedColor) + 1;
		m_pszArmedColorName = new char[ len ];
		Q_strncpy( m_pszArmedColorName, pszArmedColor, len );

		if (sscanf(pszArmedColor, "%d %d %d %d", &r, &g, &b, &a) >= 3)
		{
			// it's a direct color
			m_ArmedColor = Color(r, g, b, a);
		}
		else
		{			
			m_ArmedColor = pScheme->GetColor(pszArmedColor, Color(255, 255, 255, 255));
		}
	}
	const char *pszSelectedColor = inResourceData->GetString("selectedcolor", "");
	if (*pszSelectedColor)
	{
		int r = 255, g = 255, b = 255, a = 255;
		int len = Q_strlen(pszSelectedColor) + 1;
		m_pszSelectedColorName = new char[ len ];
		Q_strncpy( m_pszSelectedColorName, pszSelectedColor, len );

		if (sscanf(pszSelectedColor, "%d %d %d %d", &r, &g, &b, &a) >= 3)
		{
			// it's a direct color
			m_SelectedColor = Color(r, g, b, a);
		}
		else
		{			
			m_SelectedColor = pScheme->GetColor(pszSelectedColor, Color(255, 255, 255, 255));
		}
	}
	//=============================================================================
	// HPE_END
	//=============================================================================

	const char *pszBorder = inResourceData->GetString("border", "");
	if (*pszBorder)
	{
		IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
		SetBorder(pScheme->GetBorder(pszBorder));
	}

	BaseClass::ApplySettings(inResourceData);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFImageButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
	if ( m_szDefaultImage && strlen( m_szDefaultImage ) > 0 )
	{
		SetDefaultImage(scheme()->GetImage(m_szDefaultImage, true));
	}
	if ( m_szArmedImage && strlen( m_szArmedImage ) > 0 )
	{
		SetArmedImage(scheme()->GetImage(m_szArmedImage, true));
	}
	if ( m_szSelectedImage && strlen( m_szSelectedImage ) > 0 )
	{
		SetSelectedImage(scheme()->GetImage(m_szSelectedImage, true));
	}
}