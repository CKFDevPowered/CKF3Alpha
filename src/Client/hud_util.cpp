#include "hud.h"

bool IsConnected(int playerIndex)
{
	return (g_PlayerInfoList[playerIndex].name && g_PlayerInfoList[playerIndex].name[0] != 0);
}

int GetTeamCounts(short teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		if (!IsConnected(i))
			continue;

		if (g_PlayerExtraInfo[i].teamnumber == teamnumber)
			count++;
	}

	return count;
}

int GetTeamAliveCounts(short teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		if (!IsConnected(i))
			continue;

		if (g_PlayerExtraInfo[i].teamnumber == teamnumber && g_PlayerExtraInfo[i].dead == false)
			count++;
	}

	return count;
}