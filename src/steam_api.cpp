#include <metahook.h>
#include "plugins.h"
#include "configs.h"
#include "steam_api.h"

HMODULE g_hSteamAPI = NULL;

ISteamUser *(*g_pfnSteamUser)(void) = NULL;
ISteamFriends *(*g_pfnSteamFriends)(void) = NULL;
ISteamUtils *(*g_pfnSteamUtils)(void) = NULL;
ISteamMatchmaking *(*g_pfnSteamMatchmaking)(void) = NULL;
ISteamUserStats *(*g_pfnSteamUserStats)(void) = NULL;
ISteamApps *(*g_pfnSteamApps)(void) = NULL;
ISteamNetworking *(*g_pfnSteamNetworking)(void) = NULL;
ISteamMatchmakingServers *(*g_pfnSteamMatchmakingServers)(void) = NULL;
ISteamRemoteStorage *(*g_pfnSteamRemoteStorage)(void) = NULL;
ISteamScreenshots *(*g_pfnSteamScreenshots)(void) = NULL;
ISteamHTTP *(*g_pfnSteamHTTP)(void) = NULL;
ISteamUnifiedMessages *(*g_pfnSteamUnifiedMessages)(void) = NULL;

int (*g_pfnSteamAPI_Init)(void) = NULL;
bool (*g_pfnSteamAPI_IsSteamRunning)(void) = NULL;
void (*g_pfnSteamAPI_RegisterCallback)(class CCallbackBase *pCallback, int iCallback) = NULL;
void (*g_pfnSteamAPI_UnregisterCallback)(class CCallbackBase *pCallback) = NULL;

ISteamUser *SteamUser(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamUser)
		g_pfnSteamUser = (ISteamUser *(*)(void))GetProcAddress(g_hSteamAPI, "SteamUser");

	if (!g_pfnSteamUser)
		return NULL;

	return g_pfnSteamUser();
}

ISteamFriends *SteamFriends(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamFriends)
		g_pfnSteamFriends = (ISteamFriends *(*)(void))GetProcAddress(g_hSteamAPI, "SteamFriends");

	if (!g_pfnSteamFriends)
		return NULL;

	return g_pfnSteamFriends();
}

ISteamUtils *SteamUtils(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamUtils)
		g_pfnSteamUtils = (ISteamUtils *(*)(void))GetProcAddress(g_hSteamAPI, "SteamUtils");

	if (!g_pfnSteamUtils)
		return NULL;

	return g_pfnSteamUtils();
}

ISteamMatchmaking *SteamMatchmaking(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamMatchmaking)
		g_pfnSteamMatchmaking = (ISteamMatchmaking *(*)(void))GetProcAddress(g_hSteamAPI, "SteamMatchmaking");

	if (!g_pfnSteamMatchmaking)
		return NULL;

	return g_pfnSteamMatchmaking();
}

ISteamUserStats *SteamUserStats(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamUserStats)
		g_pfnSteamUserStats = (ISteamUserStats *(*)(void))GetProcAddress(g_hSteamAPI, "SteamUserStats");

	if (!g_pfnSteamUserStats)
		return NULL;

	return g_pfnSteamUserStats();
}

ISteamApps *SteamApps(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamApps)
		g_pfnSteamApps = (ISteamApps *(*)(void))GetProcAddress(g_hSteamAPI, "SteamApps");

	if (!g_pfnSteamApps)
		return NULL;

	return g_pfnSteamApps();
}

ISteamNetworking *SteamNetworking(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamNetworking)
		g_pfnSteamNetworking = (ISteamNetworking *(*)(void))GetProcAddress(g_hSteamAPI, "SteamNetworking");

	if (!g_pfnSteamNetworking)
		return NULL;

	return g_pfnSteamNetworking();
}

ISteamMatchmakingServers *SteamMatchmakingServers(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamMatchmakingServers)
		g_pfnSteamMatchmakingServers = (ISteamMatchmakingServers *(*)(void))GetProcAddress(g_hSteamAPI, "SteamMatchmakingServers");

	if (!g_pfnSteamMatchmakingServers)
		return NULL;

	return g_pfnSteamMatchmakingServers();
}

ISteamRemoteStorage *SteamRemoteStorage(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamRemoteStorage)
		g_pfnSteamRemoteStorage = (ISteamRemoteStorage *(*)(void))GetProcAddress(g_hSteamAPI, "SteamRemoteStorage");

	if (!g_pfnSteamRemoteStorage)
		return NULL;

	return g_pfnSteamRemoteStorage();
}

ISteamScreenshots *SteamScreenshots(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamScreenshots)
		g_pfnSteamScreenshots = (ISteamScreenshots *(*)(void))GetProcAddress(g_hSteamAPI, "SteamScreenshots");

	if (!g_pfnSteamScreenshots)
		return NULL;

	return g_pfnSteamScreenshots();
}

ISteamHTTP *SteamHTTP(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamHTTP)
		g_pfnSteamHTTP = (ISteamHTTP *(*)(void))GetProcAddress(g_hSteamAPI, "SteamHTTP");

	if (!g_pfnSteamHTTP)
		return NULL;

	return g_pfnSteamHTTP();
}

ISteamUnifiedMessages *SteamUnifiedMessages(void)
{
	if (!g_hSteamAPI)
		g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_pfnSteamUnifiedMessages)
		g_pfnSteamUnifiedMessages = (ISteamUnifiedMessages *(*)(void))GetProcAddress(g_hSteamAPI, "SteamUnifiedMessages");

	if (!g_pfnSteamUnifiedMessages)
		return NULL;

	return g_pfnSteamUnifiedMessages();
}

bool SteamAPI_Init(void)
{
	if (!g_bIsUseSteam)
		return false;

	g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_hSteamAPI)
		return false;

	if (!g_pfnSteamAPI_Init)
		g_pfnSteamAPI_Init = (int (*)(void))GetProcAddress(g_hSteamAPI, "SteamAPI_Init");

	if (!g_pfnSteamAPI_Init)
		return false;

	return g_pfnSteamAPI_Init() != 0;
}

bool SteamAPI_IsSteamRunning(void)
{
	if (!g_bIsUseSteam)
		return false;

	g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_hSteamAPI)
		return false;

	if (!g_pfnSteamAPI_IsSteamRunning)
		g_pfnSteamAPI_IsSteamRunning = (bool (*)(void))GetProcAddress(g_hSteamAPI, "SteamAPI_IsSteamRunning");

	if (!g_pfnSteamAPI_IsSteamRunning)
		return false;

	return g_pfnSteamAPI_IsSteamRunning();
}

void SteamAPI_RegisterCallback(class CCallbackBase *pCallback, int iCallback)
{
	if (!g_bIsUseSteam)
		return;

	g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_hSteamAPI)
		return;

	if (!g_pfnSteamAPI_RegisterCallback)
		g_pfnSteamAPI_RegisterCallback = (void (*)(class CCallbackBase *, int))GetProcAddress(g_hSteamAPI, "SteamAPI_RegisterCallback");

	if (!g_pfnSteamAPI_RegisterCallback)
		return;

	return g_pfnSteamAPI_RegisterCallback(pCallback, iCallback);
}

void SteamAPI_UnregisterCallback(class CCallbackBase *pCallback)
{
	if (!g_bIsUseSteam)
		return;

	g_hSteamAPI = GetModuleHandle("steam_api.dll");

	if (!g_hSteamAPI)
		return;

	if (!g_pfnSteamAPI_UnregisterCallback)
		g_pfnSteamAPI_UnregisterCallback = (void (*)(class CCallbackBase *))GetProcAddress(g_hSteamAPI, "SteamAPI_UnregisterCallback");

	if (!g_pfnSteamAPI_UnregisterCallback)
		return;

	return g_pfnSteamAPI_UnregisterCallback(pCallback);
}