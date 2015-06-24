#ifndef SERVERCONTEXTMENU_H
#define SERVERCONTEXTMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>

class CServerContextMenu : public vgui::Menu
{
public:
	CServerContextMenu(vgui::Panel *parent);
	~CServerContextMenu();

public:
	void ShowMenu(vgui::Panel *target, unsigned int serverID, bool showConnect, bool showViewGameInfo, bool showRefresh, bool showAddToFavorites);
};

#endif