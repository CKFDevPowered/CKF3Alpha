#include "EngineInterface.h"
#include "CreateMultiplayerGameDialog.h"
#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"

#include "ModInfo.h"
#include "GameUI_Interface.h"

#include <stdio.h>

using namespace vgui;

#include <vgui/ILocalize.h>

#include "FileSystem.h"
#include <KeyValues.h>

CCreateMultiplayerGameDialog::CCreateMultiplayerGameDialog(vgui::Panel *parent) : PropertyDialog(parent, "CreateMultiplayerGameDialog")
{
	m_bBotsEnabled = false;
	SetSize(348, 460);

	SetTitle("#GameUI_CreateServer", true);
	SetOKButtonText("#GameUI_Start");

	m_bBotsEnabled = true;

	m_pServerPage = new CCreateMultiplayerGameServerPage(this, "ServerPage");
	m_pGameplayPage = new CCreateMultiplayerGameGameplayPage(this, "GameplayPage");
	m_pBotPage = NULL;

	AddPage(m_pServerPage, "#GameUI_Server");
	AddPage(m_pGameplayPage, "#GameUI_Game");

	m_pSavedData = new KeyValues("ServerConfig");

	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/ServerConfig.vdf");

		const char *startMap = m_pSavedData->GetString("map", "");

		if (startMap[0])
			m_pServerPage->SetMap(startMap);
	}

	if (m_bBotsEnabled)
	{
		m_pBotPage = new CCreateMultiplayerGameBotPage(this, "BotPage", m_pSavedData);
		AddPage(m_pBotPage, "#GameUI_CPUPlayerOptions");
		m_pServerPage->EnableBots(m_pSavedData);
	}
}

CCreateMultiplayerGameDialog::~CCreateMultiplayerGameDialog(void)
{
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

bool CCreateMultiplayerGameDialog::OnOK(bool applyOnly)
{
	BaseClass::OnOK(applyOnly);

	char szMapName[64], szHostName[64], szPassword[64];
	const char *pszMapName = m_pServerPage->GetMapName();

	if (!pszMapName)
		return false;

	strncpy(szMapName, pszMapName, sizeof(szMapName));
	strncpy(szHostName, m_pGameplayPage->GetHostName(), sizeof(szHostName));
	strncpy(szPassword, m_pGameplayPage->GetPassword(), sizeof(szPassword));

	if (m_pSavedData)
	{
		if (m_pServerPage->IsRandomMapSelected())
			m_pSavedData->SetString("map", "");
		else
			m_pSavedData->SetString("map", szMapName);

		m_pSavedData->SaveToFile(g_pFullFileSystem, "cfg/ServerConfig.vdf");
	}

	SetVisible(false);

	char szMapCommand[1024];
	Q_snprintf(szMapCommand, sizeof(szMapCommand), "disconnect\nsv_lan 1\nsetmaster disable\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\ncd fadeout\nmap %s\n", m_pGameplayPage->GetMaxPlayers(), szPassword, szHostName, szMapName);
	engine->pfnClientCmd(szMapCommand);

	char *botCmdBuf = m_pServerPage->GetBOTCommandBuffer();

	if (botCmdBuf)
		engine->pfnClientCmd(botCmdBuf);

	if (m_pBotPage)
		engine->pfnClientCmd(m_pBotPage->GetBOTCommandBuffer());

	m_pGameplayPage->SaveValues();
	return true;
}