#ifndef TFCLASSTIPS_H
#define TFCLASSTIPS_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>


namespace vgui
{

class ImagePanel;
class Label;

class TFClassTipsItemPanel : public EditablePanel
{
public:
	typedef EditablePanel BaseClass;

	TFClassTipsItemPanel( vgui::Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	void SetText(const char *text);
	void SetText(const wchar_t *text);

private:
	Label		*m_pLabel;
	ImagePanel	*m_pIcon;
};

}

#endif