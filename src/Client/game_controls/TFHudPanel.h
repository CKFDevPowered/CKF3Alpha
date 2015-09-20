#ifndef TFCLASSTIPS_H
#define TFCLASSTIPS_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

namespace vgui
{

class TFHudPanel : public Panel
{
public:
	DECLARE_CLASS_SIMPLE( TFHudPanel, Panel );

	TFHudPanel( vgui::Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Paint(void);

	void SetColorIndex( int iColorIndex );
	void SetColorAlpha( int iColorAlpha );

private:
	int m_iColorIndex;
	int m_iColorAlpha;
};

}

#endif