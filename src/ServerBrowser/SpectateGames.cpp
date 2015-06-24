#include "SpectateGames.h"

CSpectateGames::CSpectateGames(vgui::Panel *parent) : CInternetGames(parent, "SpectateGames", eSpectatorServer)
{
}

void CSpectateGames::GetNewServerList(void)
{
	m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("proxy", "1"));

	BaseClass::GetNewServerList();
}