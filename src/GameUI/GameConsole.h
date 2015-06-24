#ifndef GAMECONSOLE_H
#define GAMECONSOLE_H

#ifdef _WIN32
#pragma once
#endif

#include "IGameConsole.h"

class CGameConsoleDialog;

class CGameConsole : public IGameConsole
{
public:
	CGameConsole(void);
	~CGameConsole(void);

public:
	virtual void Activate(void);
	virtual void Initialize(void);
	virtual void Hide(void);
	virtual void Clear(void);
	virtual bool IsConsoleVisible(void);
	virtual void Printf(const char *format, ...);
	virtual void DPrintf(const char *format, ...);
	virtual void SetParent(int parent);

public:
	static void OnCmdCondump(void);

private:
	bool m_bInitialized;
	CGameConsoleDialog *m_pConsole;
};

extern CGameConsole &GameConsole(void);
#endif