#ifndef VGUI_HUD_SCREENOVERLAY_H
#define VGUI_HUD_SCREENOVERLAY_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include "hud_element.h"

namespace vgui
{

class CTFHudScreenOverlay : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudScreenOverlay, EditablePanel );

public:
	CTFHudScreenOverlay();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf);
	virtual bool ShouldDraw( void );

	virtual void Paint(void);

	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	int								m_nTeam;
	bool							m_bInvuln;
	int								m_iInvulnTeam;
	int								m_iInvulnMaterial[2];
	int								m_iSniperScopeMaterial;
};
}

#endif