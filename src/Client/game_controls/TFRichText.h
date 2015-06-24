#ifndef TFRICHTEXT_H
#define TFRICHTEXT_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/RichText.h>
#include <vgui_controls/ImagePanel.h>

namespace vgui
{

class ImagePanel;
class RichText;

class TFRichText : public RichText
{
public:
	DECLARE_CLASS_SIMPLE( TFRichText, vgui::RichText );

	TFRichText( vgui::Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	//virtual void OnTick( void );
	void SetScrollBarImagesVisible( bool visible );

	MESSAGE_FUNC(OnTick, "Tick");

private:
	char		m_szFont[64];
	char		m_szColor[64];

	ImagePanel	*m_pUpArrow;
	ImagePanel	*m_pLine;
	ImagePanel	*m_pDownArrow;
	ImagePanel	*m_pBox;

};

}

#endif