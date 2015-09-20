#ifndef VGUI_HUD_DEMOPIPES_H
#define VGUI_HUD_DEMOPIPES_H

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

class CTFHudDemomanPipes : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudDemomanPipes, EditablePanel );

public:
	CTFHudDemomanPipes();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	vgui::EditablePanel *m_pPipesPresent;
	vgui::EditablePanel *m_pNoPipesPresent;

	vgui::ImagePanel *m_pImageBG;

	vgui::ContinuousProgressBar *m_pChargeMeter;
	vgui::Label *m_pChargeLabel;

	int	m_nTeam;
	int m_nActivePipes;
};
}

#endif