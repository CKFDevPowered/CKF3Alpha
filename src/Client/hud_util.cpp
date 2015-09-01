#include "hud.h"

bool IsConnected(int playerIndex)
{
	return (g_PlayerInfoList[playerIndex].name && g_PlayerInfoList[playerIndex].name[0] != 0);
}

int GetTeamCounts(int teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		if (!IsConnected(i))
			continue;

		if (gCKFVars.g_PlayerInfo[i].iTeam == teamnumber)
			count++;
	}

	return count;
}

int GetClassCounts(int iTeam, int iClass)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		if (!IsConnected(i))
			continue;

		if (gCKFVars.g_PlayerInfo[i].iTeam == iTeam && gCKFVars.g_PlayerInfo[i].iClass == iClass)
			count++;
	}

	return count;
}

int GetTeamAliveCounts(int teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		if (!IsConnected(i))
			continue;

		if (gCKFVars.g_PlayerInfo[i].iTeam == teamnumber && gCKFVars.g_PlayerInfo[i].bIsDead == false)
			count++;
	}

	return count;
}