#include "EngineInterface.h"
#include "ModInfo.h"
#include "KeyValues.h"
#include "vgui_controls/Controls.h"
#include "FileSystem.h"
#include "configs.h"

CModInfo &ModInfo(void)
{
	static CModInfo s_ModInfo;
	return s_ModInfo;
}

CModInfo::CModInfo(void)
{
	m_pModData = new KeyValues("ModData");
}

CModInfo::~CModInfo(void)
{
}

void CModInfo::FreeModInfo(void)
{
	if (m_pModData)
	{
		m_pModData->deleteThis();
		m_pModData = NULL;
	}
}

const char *CModInfo::GetStartMap(void)
{
	return m_pModData->GetString("startmap", "c0a0");
}

const char *CModInfo::GetTrainMap(void)
{
	return m_pModData->GetString("trainmap", "t0a0");
}

bool CModInfo::IsMultiplayerOnly(void)
{
	return (stricmp(m_pModData->GetString("type", ""), "multiplayer_only") == 0);
}

bool CModInfo::IsSinglePlayerOnly(void)
{
	return (stricmp(m_pModData->GetString("type", ""), "singleplayer_only") == 0);
}

bool CModInfo::NoModels(void)
{
	return (stricmp(m_pModData->GetString("nomodels", "0"), "1") == 0);
}

bool CModInfo::NoHiModel(void)
{
	return (stricmp(m_pModData->GetString("nohimodel", "0"), "1") == 0);
}

const char *CModInfo::GetGameDescription(void)
{
	return m_pModData->GetString("game", "Half-Life");
}

void CModInfo::LoadCurrentGameInfo(void)
{
	char const *filename = "liblist.gam";

	FileHandle_t fh = g_pFullFileSystem->Open(filename, "rb");

	if (fh != FILESYSTEM_INVALID_HANDLE)
	{
		int len = g_pFullFileSystem->Size(fh);

		if (len > 0)
		{
			char *buf = (char *)_alloca(len + 1);
			g_pFullFileSystem->Read(buf, len, fh);
			buf[len] = 0;
			LoadGameInfoFromBuffer(buf, len);
		}

		g_pFullFileSystem->Close(fh);
	}
}

void CModInfo::LoadGameInfoFromBuffer(const char *buffer, int bufferSize)
{
	char token[1024];
	bool done = false;
	char *p = (char *)buffer;

	while (!done && p)
	{
		char key[256];
		char value[256];

		p = engine->COM_ParseFile(p, token);

		if (strlen(token) <= 0)
			break;

		strcpy(key, token);

		p = engine->COM_ParseFile(p, token);
		strcpy(value, token);

		m_pModData->SetString(key, value);
	}
}