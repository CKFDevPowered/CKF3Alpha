#ifndef VGUI_HUD_AMMOSTATUS_H
#define VGUI_HUD_AMMOSTATUS_H

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

class CTFHudWeaponAmmo : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudWeaponAmmo, EditablePanel );

public:
	CTFHudWeaponAmmo();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf){ return 0;};
	virtual bool ShouldDraw( void );

	virtual void OnThink(void);

	DECLARE_HUD_ELEMENT_SIMPLE();

private:

	void UpdateAmmoLabels( bool bPrimary, bool bReserve, bool bNoClip );

	float							m_flNextThink;
	int								m_nAmmo;
	int								m_nAmmo2;
	int								m_nTeam;
	CKFClientWeapon *				m_hCurrentWeapon;

	Label							*m_pInClip;
	Label							*m_pInClipShadow;
	Label							*m_pInReserve;
	Label							*m_pInReserveShadow;
	Label							*m_pNoClip;
	Label							*m_pNoClipShadow;

	ImagePanel						*m_pImageBG;
};
}

#endif