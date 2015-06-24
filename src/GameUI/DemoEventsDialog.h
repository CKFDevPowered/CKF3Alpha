// DemoEventsDialog.h: interface for the CDemoEventsDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined DEMOEVENTSDIALOG_H
#define DEMOEVENTSDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "DemoEditDialog.h"
#include "DirectorCmd.h"

class IDemoPlayer;
class IDirector;
class IEngineWrapper;

class CDemoEventsDialog : public vgui::Frame
{

public:
	CDemoEventsDialog(vgui::Panel *parent, const char *name, IEngineWrapper * engine,
		IDemoPlayer * demoPlayer );
	virtual ~CDemoEventsDialog();

	typedef vgui::Frame BaseClass;

protected:
	// virtual overrides

	virtual void	OnCommand(const char *command);
	// virtual void	OnMessage(vgui::KeyValues *params, vgui::VPANEL fromPanel);

	void			OnUpdate();	// update director command list
	void			GetCurrentCmd();
	void			OnAdd();
	void			OnGoto();
	void			OnRemove();
	void			OnModify();
	
	vgui::ListPanel				*m_EventList;
	IEngineWrapper				*m_Engine;
	IDemoPlayer					*m_DemoPlayer;
	DirectorCmd					*m_CurrentCmd;
	CDemoEditDialog				*m_EditDialog;

	DECLARE_PANELMAP();

};

#endif // !defined DEMOEVENTSDIALOG_H
