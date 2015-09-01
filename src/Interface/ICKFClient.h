#ifndef ICKFCLIENT_H
#define ICKFCLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include <metahook.h>
#include "ckfvars.h"

typedef void (*xcommand_t)(void);

typedef struct
{
	int (*GetHUDFov)(void);
	float *(*GetPunchAngles)(void);
	pfnUserMsgHook (*HookUserMsg)(char *pMsgName, pfnUserMsgHook pfnHook);
	xcommand_t (*HookCmd)(char *cmd_name, xcommand_t newfuncs);
}bte_funcs_t;

class ICKFClient : public IBaseInterface
{
public:
	virtual void Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave);
	virtual void Shutdown(void);
	virtual void LoadEngine(void);
	virtual void LoadClient(cl_exportfuncs_t *pExportFunc);
	virtual void ExitGame(int iResult);

	virtual void GetClientFuncs(cl_exportfuncs_t *pExportfuncs);
	virtual void GetBTEFuncs(bte_funcs_t *pBTEFuncs);
	virtual void ShowScoreBoard(bool state);
	virtual bool IsScoreBoardVisible(void);
	virtual int SwitchWeapon(int slot);
	virtual void Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h, qboolean bLocalXY);
	virtual void GetCKFVars(ckf_vars_t *pCKFVars);
	virtual void DrawHudMask(int col, int x, int y, int w, int h);
};

#define CKFCLIENT_API_VERSION "CKFCLIENT_API_VERSION001"

#endif