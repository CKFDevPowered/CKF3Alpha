#ifndef TFMAPINFOMENU_H
#define TFMAPINFOMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/keyrepeat.h>

#include <vgui/KeyCode.h>
#include <UtlVector.h>

#define PANEL_MAPINFO "mapinfo"

namespace vgui
{
	class Label;
	class Button;
	class TFRichText;
	class ImagePanel;
	class CKeyRepeatHandler;
}

class CTFMapInfoMenu : public vgui::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CTFMapInfoMenu, vgui::Frame );

public:
	CTFMapInfoMenu( );
	virtual ~CTFMapInfoMenu();

	virtual const char *GetName( void ){ return PANEL_MAPINFO; }
	virtual void SetData( KeyValues *data ){}
	virtual void Reset(){ Update(); }
	virtual void Update();
	virtual bool NeedsUpdate( void ){ return false; }
	virtual bool HasInputElements( void ){ return true; }
	virtual void ShowPanel( bool bShow );

	static const char *GetMapType( const char *mapName );

	DECLARE_VIEWPORT_PANEL_SIMPLE();
protected:
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );
	virtual void OnKeyCodeReleased( vgui::KeyCode code );
	virtual void OnThink();
	
	virtual void Init(void);
	virtual void VidInit(void);

private:
	// helper functions
	void LoadMapPage( const char *mapName );
	void SetMapTitle();
	void CheckBackContinueButtons();

protected:
	vgui::Label			*m_pTitle;
	vgui::TFRichText		*m_pMapInfo;

	vgui::Button			*m_pContinue;
	vgui::Button			*m_pBack;

	vgui::ImagePanel		*m_pMapImage;

	char				m_szMapName[MAX_PATH];

	vgui::CKeyRepeatHandler	m_KeyRepeat;
};

#endif