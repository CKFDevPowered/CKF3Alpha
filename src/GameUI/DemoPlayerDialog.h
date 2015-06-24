//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
//
// Purpose: DemoPlayerDialog.h: interface for the CDemoPlayerDialog class.
//
// $NoKeywords: $
//=============================================================================

#if !defined DEMOPLAYERDIALOG_H
#define DEMOPLAYERDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
//#include "BaseSystemModule.h"

class IDemoPlayer;
class IWorld;
class IBaseSystem;
class IEngineWrapper;
class ISystemModule;

class CDemoPlayerDialog  : public vgui::Frame
{
public:
	
	CDemoPlayerDialog(vgui::Panel *parent);
	virtual ~CDemoPlayerDialog();

protected:

	// virtual overrides
	virtual void	OnThink();
	virtual void	OnClose();
	virtual void	OnCommand(const char *command);
	virtual void	OnMessage(KeyValues *params, vgui::VPANEL fromPanel);

	virtual void	ReceiveSignal(ISystemModule * module, unsigned int signal);

	typedef vgui::Frame BaseClass;

	void Update();	// updates all visible data fields
	bool LoadModules();	// get other modules (world, demo player etc)

	
	// main panel button events
	void OnPause();	// pause playback
	void OnPlay();	// rsume playback
	void OnStart();	// go to start
	void OnEnd();	// go to end
	void OnSlower();	// faster playback speed
	void OnFaster();	// slower playback speed
	void OnLoad();	// load new demo dialog
	void OnStop();	// stop demo playback completely
	void OnNextFrame(int direction);	// next/last frame
	void OnEvents(); // open demo events editor
	void OnSave();	// save demo file again
	

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

	vgui::Label				*m_pLableTimeCode;
	vgui::Slider			*m_pTimeSlider;	
	vgui::ToggleButton		*m_MasterButton;

	
	vgui::Button			*m_pButtonPlay;		
	vgui::Button			*m_pButtonStepF;		// |>
	vgui::Button			*m_pButtonFaster;		// >>
	vgui::Button			*m_pButtonSlower;		// <<
	vgui::Button			*m_pButtonStepB;		// <|
	vgui::Button			*m_pButtonPause;		// ||
	vgui::Button			*m_pButtonStart;		// |<
	vgui::Button			*m_pButtonEnd;			// >|
	vgui::Button			*m_pButtonLoad;			// ^
	vgui::Button			*m_pButtonStop;			// x
	
	
	vgui::DHANDLE<vgui::Frame>	m_hDemoPlayerFileDialog;
	vgui::DHANDLE<vgui::Frame>	m_hDemoEventsDialog;
	
	IEngineWrapper			*m_Engine;
	IDemoPlayer				*m_DemoPlayer;
	IBaseSystem				*m_System;
	IWorld					*m_World;
	
	float					m_NextTimeScale;
	int						m_lastSliderTime;
};

#endif // !defined DEMOPLAYERDIALOG_H
