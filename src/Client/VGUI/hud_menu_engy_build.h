#ifndef VGUI_HUD_MENU_ENGY_BUILD_H
#define VGUI_HUD_MENU_ENGY_BUILD_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "hud_element.h"

namespace vgui
{

class EditablePanel;
class ImagePanel;
class Label;
class ContinuousProgressBar;

class CTFHudMenuEngyBuild : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudMenuEngyBuild, EditablePanel );

public:
	CTFHudMenuEngyBuild();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual void OnThink( void );
	virtual void SetVisible( bool state );

	virtual bool KeyInput( int down, KeyCode keynum, const char *pszCurrentBinding );

	DECLARE_HUD_ELEMENT_SIMPLE();

	int GetBuildingCost(int iBuildID);
	int GetBuildingIDFromSlot( int iSlot );
	void SendBuildMessage( int iSlot );
	bool SendDestroyMessage( int iSlot );

private:

	EditablePanel *m_pAvailableObjects[4];
	EditablePanel *m_pAlreadyBuiltObjects[4];
	EditablePanel *m_pCantAffordObjects[4];
	EditablePanel *m_pUnavailableObjects[4];

	Label *m_pBuildLabelBright;
	Label *m_pBuildLabelDim;

	Label *m_pDestroyLabelBright;
	Label *m_pDestroyLabelDim;
};
}

#endif