#ifndef DIALOGSERVERPASSWORD_H
#define DIALOGSERVERPASSWORD_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

class CDialogServerPassword : public vgui::Frame
{
public:
	typedef vgui::Frame BaseClass;

public:
	CDialogServerPassword(vgui::Panel *parent);
	~CDialogServerPassword(void);

public:
	void Activate(const char *serverName, unsigned int serverID);

private:
	virtual void OnCommand(const char *command);

private:
	vgui::Label *m_pInfoLabel;
	vgui::Label *m_pGameLabel;
	vgui::TextEntry *m_pPasswordEntry;
	vgui::Button *m_pConnectButton;

	int m_iServerID;
};

#endif