#ifndef ICKFCLIENT_H
#define ICKFCLIENT_H

#ifdef _WIN32
#pragma once
#endif

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
	virtual int SwitchWeapon(int slot);
	virtual void Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h);
};

#define CKFCLIENT_API_VERSION "CKFCLIENT_API_VERSION001"

#endif