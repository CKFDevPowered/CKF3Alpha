#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>

#define STEAM_OFFLINE_MODE "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\Offline"
#define STEAM_AFS_MODE "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\OfflineAFS"
#define OFFLINE_FILE "Steam\\cached\\offline_"

inline bool IsSteamInOfflineMode(void)
{
	int offline = 0;
	vgui::system()->GetRegistryInteger(STEAM_OFFLINE_MODE, offline);
	return (offline == 1);
}

inline bool IsSteamInAuthenticationFailSafeMode(void)
{
	int offline = 0;
	vgui::system()->GetRegistryInteger(STEAM_AFS_MODE, offline);
	return (offline == 1);
}

inline bool IsSteamGameServerBrowsingEnabled(void)
{
	return (IsSteamInAuthenticationFailSafeMode() || !IsSteamInOfflineMode());
}
