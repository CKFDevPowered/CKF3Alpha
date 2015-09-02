#ifndef VGUI_HUD_PLAYERSTATUS_H
#define VGUI_HUD_PLAYERSTATUS_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/controls.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "hud_element.h"

namespace vgui
{

class EditablePanel;
class ImagePanel;
class Label;

//-----------------------------------------------------------------------------
// Purpose: Display class images
//-----------------------------------------------------------------------------
class TFClassImage : public ImagePanel
{
public:
	DECLARE_CLASS_SIMPLE( TFClassImage, ImagePanel );

	TFClassImage( vgui::Panel *parent, const char *name );

	void SetClass( int iTeam, int iClass );

private:
	IImage	*m_pImageClassImage[2][9];
};

//-----------------------------------------------------------------------------
// Purpose:  Clips the health image to the appropriate percentage
//-----------------------------------------------------------------------------
class TFHealthPanel : public ImagePanel
{
public:
	DECLARE_CLASS_SIMPLE( TFHealthPanel, ImagePanel );

	TFHealthPanel( vgui::Panel *parent, const char *name );
	virtual void Paint();

	void SetHealth( float flHealth ){ m_flHealth = ( flHealth <= 1.0 ) ? flHealth : 1.0f; }

private:

	float	m_flHealth; // percentage from 0.0 -> 1.0
	int		m_iMaterialIndex;
	int		m_iDeadMaterialIndex;
};

//-----------------------------------------------------------------------------
// Purpose:  Displays player class data
//-----------------------------------------------------------------------------
class CTFHudPlayerClass : public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CTFHudPlayerClass, EditablePanel );

public:

	CTFHudPlayerClass( Panel *parent, const char *name );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Reset();
	virtual void OnThink();

	virtual int FireMessage(const char *pszName, int iSize, void *pbuf);
	

private:

	float				m_flNextThink;

	TFClassImage		*m_pClassImage;
	ImagePanel			*m_pSpyImage; // used when spies are disguised
	ImagePanel			*m_pSpyOutlineImage;
	ImagePanel			*m_pClassImageBG;
	Label				*m_pHealthValue;

	int					m_nTeam;
	int					m_nClass;
	int					m_nDisguise;
	int					m_nDisguiseTeam;
	int					m_nDisguiseClass;
};

//-----------------------------------------------------------------------------
// Purpose:  Displays player health data
//-----------------------------------------------------------------------------
class CTFHudPlayerHealth : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CTFHudPlayerHealth, EditablePanel );

public:

	CTFHudPlayerHealth( Panel *parent, const char *name );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Reset();

	void	SetHealth( int iNewHealth, int iMaxHealth, int iMaxBuffedHealth );
	void	HideHealthBonusImage( void );

protected:

	virtual void OnThink();

protected:
	float				m_flNextThink;

private:
	TFHealthPanel		*m_pHealthImage;
	ImagePanel			*m_pHealthBonusImage;
	ImagePanel			*m_pHealthImageBG;

	int					m_nHealth;
	int					m_nMaxHealth;

	int					m_nBonusHealthOrigX;
	int					m_nBonusHealthOrigY;
	int					m_nBonusHealthOrigW;
	int					m_nBonusHealthOrigH;

	CPanelAnimationVar( int, m_nHealthBonusPosAdj, "HealthBonusPosAdj", "25" );
	CPanelAnimationVar( float, m_flHealthDeathWarning, "HealthDeathWarning", "0.49" );
	CPanelAnimationVar( Color, m_clrHealthDeathWarningColor, "HealthDeathWarningColor", "HUDDeathWarning" );
};

class CTFHudPlayerStatus : public EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CTFHudPlayerStatus, vgui::EditablePanel );

public:
	CTFHudPlayerStatus();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual int FireMessage(const char *pszName, int iSize, void *pbuf);

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void Think(void);
	virtual bool ShouldDraw( void );

	DECLARE_HUD_ELEMENT_SIMPLE();

private:

	CTFHudPlayerClass	*m_pHudPlayerClass;
	CTFHudPlayerHealth	*m_pHudPlayerHealth;
};
}

#endif