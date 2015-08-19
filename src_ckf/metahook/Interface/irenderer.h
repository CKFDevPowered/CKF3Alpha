#ifndef IRENDERER_H
#define IRENDERER_H

#ifdef _WIN32
#pragma once
#endif

class IRenderer : public IBaseInterface
{
public:
	virtual void Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave);
	virtual void Shutdown(void);
	virtual void LoadEngine(void);
	virtual void LoadClient(cl_exportfuncs_t *pExportFunc);
	virtual void ExitGame(int iResult);

	virtual void GetClientFuncs(cl_exportfuncs_t *pExportfuncs);
	virtual void GetInterface(ref_export_t **ppRefExports, ref_funcs_t **ppRefFuncs, studio_funcs_t **ppStudioFuncs);
};

#define RENDERER_API_VERSION "RENDER_API_VERSION001"

#endif