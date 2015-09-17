#include <IRenderer.h>
#include <ICKFClient.h>
#include <IBTEClient.h>
#include <IPlugins.h>

#define METAHOOK_PLUGIN_API_VERSION "METAHOOK_PLUGIN_API_VERSION002"

extern IPlugins *g_pRendererPlugins;
extern IPlugins *g_pCKFClientPlugins;

void Renderer_Init(void);
void CKF_Init(void);
void Renderer_Shutdown(void);
void CKF_Shutdown(void);