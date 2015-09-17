#ifndef ICKFCLIENT_H
#define ICKFCLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include <metahook.h>
#include "ckfvars.h"

class ICKFClient : public IBaseInterface
{
public:
	virtual void GetClientFuncs(cl_exportfuncs_t *pExportfuncs);
	virtual int SwitchWeapon(int slot);
	virtual void Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h, qboolean bLocalXY);
	virtual void GetCKFVars(ckf_vars_t *pCKFVars);
	virtual void DrawHudMask(int col, int x, int y, int w, int h);
	virtual void BaseUI_Initalize(CreateInterfaceFn *factories, int count);
};

extern ICKFClient *g_pCKFClient;

#define CKFCLIENT_API_VERSION "CKFCLIENT_API_VERSION001"
#endif