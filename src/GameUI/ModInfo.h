#ifndef MODINFO_H
#define MODINFO_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>

class KeyValues;

class CModInfo
{
public:
	CModInfo(void);
	~CModInfo(void);

public:
	void FreeModInfo(void);
	void LoadCurrentGameInfo(void);
	void LoadGameInfoFromBuffer(const char *buffer, int bufferSize);
	const char *GetStartMap(void);
	const char *GetTrainMap(void);
	bool IsMultiplayerOnly(void);
	bool IsSinglePlayerOnly(void);
	bool NoModels(void);
	bool NoHiModel(void);
	const char *GetGameDescription(void);

private:
	KeyValues *m_pModData;
};

extern CModInfo &ModInfo(void);

#endif