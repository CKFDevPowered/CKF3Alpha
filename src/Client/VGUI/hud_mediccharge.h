#ifndef VGUI_HUD_MEDICCHARGE_H
#define VGUI_HUD_MEDICCHARGE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/AnimationController.h>
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

class CTFHudMedicChargeMeter : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudMedicChargeMeter, EditablePanel );

public:
	CTFHudMedicChargeMeter();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	int								m_nTeam;

	ContinuousProgressBar			*m_pChargeMeter;

	bool							m_bCharged;
	float							m_flLastChargeValue;

	ImagePanel						*m_pImageBG;
	Label							*m_pChargeLabel;
	Label							*m_pIndividualChargesLabel;
};
}

#endif