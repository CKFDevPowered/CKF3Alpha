// DemoEditDialog.h: interface for the CDemoEditDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined DEMOEDITDIALOG_H
#define DEMOEDITDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "DirectorCmd.h"
#include "LabeledCommandComboBox.h"

class IDemoPlayer;
class IDirector;
class IEngineWrapper;


#define MAX_DATA_FIELDS		8


class CDemoEditDialog : public vgui::Frame
{

public:
	CDemoEditDialog(vgui::Panel *parent, const char *name, IEngineWrapper * engine,
		IDemoPlayer * demoPlayer );
	virtual ~CDemoEditDialog();
	
	void	SetCommand(DirectorCmd * cmd);

	typedef vgui::Frame BaseClass;

protected:
	// virtual overrides

	virtual void	OnCommand(const char *command);

	void	OnOK();
	void	OnGetView();
	void	OnGetTime();
	void	OnUpdate();
	void	OnTypeChanged();
	void	UpdateDrcCmd();
	void	ClearDataFields();
	void	SetDataField( int index, char * label, char * text);
	char *	GetDataFields( int index );
	int		GetDataFieldx( int index );
	int		GetDataFieldi( int index );
	float	GetDataFieldf( int index );
	void	GetDataField3f( int index, float * f1, float * f2, float * f3 );
	void	GetDataField2f( int index, float * f1, float * f2 );
	

	
	IEngineWrapper			*m_Engine;
	IDemoPlayer				*m_DemoPlayer;
	DirectorCmd				m_DrcCmd;
	bool					m_ChangingData;

	CLabeledCommandComboBox *m_TypeBox;
	vgui::TextEntry			*m_TimeStamp;
	vgui::Button			*m_GetViewButton;

	vgui::TextEntry			*m_DataEntries[MAX_DATA_FIELDS];
	vgui::Label				*m_DataLabels[MAX_DATA_FIELDS];

	DECLARE_PANELMAP();
};

#endif // !defined DEMOEDITDIALOG_H
