#ifndef VGUI_HUD_METALMETER_H
#define VGUI_HUD_METALMETER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ProgressBar.h>
#include "hud_element.h"

namespace vgui
{

class EditablePanel;
class ImagePanel;
class Label;
class ContinuousProgressBar;

class CTFHudMetalMeter : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudMetalMeter, EditablePanel );

public:
	CTFHudMetalMeter();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	vgui::ImagePanel *m_pImageBG;

	vgui::Label *m_pMetalLabel;

	int	m_nTeam;
	int m_nMetal;
};
}

#endif