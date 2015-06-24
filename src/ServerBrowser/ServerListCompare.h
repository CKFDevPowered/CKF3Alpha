#ifndef SERVERLISTCOMPARE_H
#define SERVERLISTCOMPARE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ListPanel.h>

using vgui::ListPanel;
using vgui::ListPanelItem;

int __cdecl PasswordCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl BotsCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl PingCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl PlayersCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl MapCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl GameCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl ServerNameCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl SecureCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl IPAddressCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl LastPlayedCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);
int __cdecl TagsCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);

#endif