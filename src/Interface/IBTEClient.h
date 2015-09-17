#ifndef IBTECLIENT_H
#define IBTECLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include <metahook.h>

typedef void (*xcommand_t)(void);

class IBTEClient : public IBaseInterface
{
public:
	virtual int GetHUDFov(void);
	virtual float *GetPunchAngles(void);
	virtual pfnUserMsgHook HookUserMsg(char *pMsgName, pfnUserMsgHook pfnHook);
	virtual xcommand_t HookCmd(char *cmd_name, xcommand_t newfuncs);
	virtual HWND GetMainHWND(void);
};

extern IBTEClient *g_pBTEClient;

#define BTECLIENT_API_VERSION "BTECLIENT_API_VERSION001"
#endif