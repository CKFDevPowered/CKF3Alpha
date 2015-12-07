#include <metahook.h>
#include "sys.h"
#include "Configs.h"
#include "plugins.h"
#include "common.h"
#include "steam_api.h"

config_t gConfigs = { 0 };
char g_szLanguage[128];

void Config_Init(void)
{
	char configPath[MAX_PATH], value[1024];

	FILE *fp;
	char line[128], key[64];
	char *data;

	const char *gamedir;
	CommandLine()->CheckParm("-game", &gamedir);

	strcpy(gConfigs.szGameDir, gamedir);
	strcpy(gConfigs.szGameVersion, "Unknown");
	strcpy(gConfigs.szDownloadURL, "");

	gConfigs.bEnableRenderer = true;
	gConfigs.bEnableGameUI = true;
	gConfigs.bEnableClientUI = true;
	gConfigs.bEnableEventScript = true;
	gConfigs.bEnableMuzzleflash = true;
	gConfigs.bDisableSteamClient = true;

	g_szLanguage[0] = 0;

	if (g_bIsUseSteam)
	{
		ISteamApps *app = SteamApps();
		if(app)
		{
			const char *pszLanguage = SteamApps()->GetCurrentGameLanguage();
			if (pszLanguage)
				strcpy(g_szLanguage, pszLanguage);
		}
	}
	
	if(!g_szLanguage[0])
	{
		Sys_GetRegKeyValueUnderRoot("Software\\Valve\\Steam", "Language", g_szLanguage, sizeof(g_szLanguage), "english");
	}

	strcpy(gConfigs.szLanguage, g_szLanguage);

	if (stricmp(g_szLanguage, "english"))
		sprintf(gConfigs.szGameLangDir, "%s_%s", gamedir, gConfigs.szLanguage);
	else
		strcpy(gConfigs.szGameLangDir, gamedir);

	sprintf(configPath, "%s/liblist.gam", gConfigs.szGameDir);

	fp = fopen(configPath, "r");

	if (fp)
	{
		while (!feof(fp))
		{
			fgets(line, sizeof(line), fp);

			data = COM_Parse(line);
			strcpy(key, COM_GetToken());
			data = COM_Parse(data);
			strcpy(value, COM_GetToken());

			if (!strcmp(key, "game"))
				strcpy(gConfigs.szGameName, value);
			else if (!strcmp(key, "version"))
				strcpy(gConfigs.szGameVersion, value);
			else if (!strcmp(key, "url_dl"))
				strcpy(gConfigs.szDownloadURL, value);
			else if (!strcmp(key, "fallback_dir"))
				strcpy(gConfigs.szFallbackDir, value);
		}

		fclose(fp);
	}

	sprintf(configPath, "%s/config.ini", gConfigs.szGameLangDir);

	fp = fopen(configPath, "r");

	if (!fp)
	{
		char originPath[MAX_PATH];
		sprintf(originPath, "%s/config.ini", gConfigs.szGameDir);
		CreateDirectory(gConfigs.szGameLangDir, NULL);
		CopyFile(originPath, configPath, TRUE);

		fp = fopen(originPath, "r");
	}

	if (fp)
	{
		while (!feof(fp))
		{
			fgets(line, sizeof(line), fp);

			data = COM_Parse(line);
			strcpy(key, COM_GetToken());
			data = COM_Parse(data);
			strcpy(value, COM_GetToken());

			if (!strcmp(key, "EnableRenderer"))
				gConfigs.bEnableRenderer = value[0] != '0';
			else if (!strcmp(key, "EnableGameUI"))
				gConfigs.bEnableGameUI = value[0] != '0';
			else if (!strcmp(key, "EnableClientUI"))
				gConfigs.bEnableClientUI = value[0] != '0';
			else if (!strcmp(key, "EnableEventScript"))
				gConfigs.bEnableEventScript = value[0] != '0';
			else if (!strcmp(key, "EnableMuzzleflash"))
				gConfigs.bEnableMuzzleflash = value[0] != '0';
			else if (!strcmp(key, "DisableSteamClient"))
				gConfigs.bDisableSteamClient = value[0] != '0';
		}

		fclose(fp);
	}

	gConfigs.bInitialied = true;
}