#pragma once

void Config_Init(void);

typedef struct config_s
{
	bool bInitialied;

	char szLanguage[128];
	char szGameDir[MAX_PATH];
	char szGameLangDir[MAX_PATH];
	char szFallbackDir[MAX_PATH];
	char szGameVersion[12];
	char szDownloadURL[256];
	char szGameName[128];

	bool bEnableRenderer;
	bool bEnableGameUI;
	bool bEnableClientUI;
	bool bEnableEventScript;
	bool bEnableMuzzleflash;

	bool bDisableSteamClient;
}
config_t;

extern config_t gConfigs;