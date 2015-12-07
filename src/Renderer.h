#include <IRenderer.h>
#include <IAudio.h>
#include <ICKFClient.h>
#include <IBTEClient.h>
#include <IPlugins.h>

extern IPlugins *g_pAudioPlugins;
extern IPlugins *g_pRendererPlugins;
extern IPlugins *g_pCKFClientPlugins;

void Audio_Init(void);
void Audio_Shutdown(void);
void Renderer_Init(void);
void Renderer_Shutdown(void);
void CKF_Init(void);
void CKF_Shutdown(void);