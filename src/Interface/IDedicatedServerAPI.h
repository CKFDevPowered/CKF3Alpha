#ifndef IDEDICATEDSERVERAPI_H
#define IDEDICATEDSERVERAPI_H

#ifdef _WIN32
#pragma once
#endif

#include <interface.h>

class IDedicatedServerAPI : public IBaseInterface
{
public:
	virtual bool Init(char *basedir, char *cmdline, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory);
	virtual int Shutdown(void);
	virtual bool RunFrame(void);
	virtual void AddConsoleText(char *text);
	virtual void UpdateStatus(float *fps, int *nActive, int *nMaxPlayers, char *pszMap);
};

#define VENGINE_HLDS_API_VERSION "VENGINE_HLDS_API_VERSION002"

#endif