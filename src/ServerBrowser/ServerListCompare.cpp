#include "ServerListCompare.h"
#include "isteammatchmaking.h"
#include "ServerBrowserDialog.h"

#include <KeyValues.h>
#include <vgui_controls/ListPanel.h>

int __cdecl PasswordCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	if (s1->m_bPassword < s2->m_bPassword)
		return 1;
	else if (s1->m_bPassword > s2->m_bPassword)
		return -1;

	return 0;
}

int __cdecl BotsCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	if (s1->m_nBotPlayers < s2->m_nBotPlayers)
		return 1;
	else if (s1->m_nBotPlayers > s2->m_nBotPlayers)
		return -1;

	return 0;
}

int __cdecl SecureCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	if (s1->m_bSecure < s2->m_bSecure)
		return 1;
	else if (s1->m_bSecure > s2->m_bSecure)
		return -1;

	return 0;
}

int __cdecl IPAddressCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	if (s1->m_NetAdr < s2->m_NetAdr)
		return -1;
	else if (s2->m_NetAdr < s1->m_NetAdr)
		return 1;

	return 0;
}

int __cdecl PingCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	int ping1 = s1->m_nPing;
	int ping2 = s2->m_nPing;

	if (ping1 < ping2)
		return -1;
	else if (ping1 > ping2)
		return 1;

	return 0;
}

int __cdecl MapCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->m_szMap, s2->m_szMap);
}

int __cdecl GameCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	Assert(p1.userData != p2.userData);

	return Q_stricmp(s1->m_szGameDescription, s2->m_szGameDescription);
}

int __cdecl ServerNameCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->GetName(), s2->GetName());
}

int __cdecl PlayersCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	int s1p = max(0, s1->m_nPlayers - s1->m_nBotPlayers);
	int s1m = max(0, s1->m_nMaxPlayers - s1->m_nBotPlayers);
	int s2p = max(0, s2->m_nPlayers - s2->m_nBotPlayers);
	int s2m = max(0, s2->m_nMaxPlayers - s2->m_nBotPlayers);

	if (s1p > s2p)
		return -1;

	if (s1p < s2p)
		return 1;

	if (s1m > s2m)
		return -1;

	if (s1m < s2m)
		return 1;

	return 0;
}


int __cdecl LastPlayedCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	if (s1->m_ulTimeLastPlayed > s2->m_ulTimeLastPlayed)
		return -1;

	if (s1->m_ulTimeLastPlayed < s2->m_ulTimeLastPlayed)
		return 1;

	return 0;
}

int __cdecl TagsCompare(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	gameserveritem_t *s1 = ServerBrowserDialog().GetServer(p1.userData);
	gameserveritem_t *s2 = ServerBrowserDialog().GetServer(p2.userData);

	if (!s1 && s2)
		return -1;

	if (!s2 && s1)
		return 1;

	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->m_szGameTags, s2->m_szGameTags);
}