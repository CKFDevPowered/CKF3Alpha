#include <IRenderer.h>
#include <ICKFClient.h>

extern IRenderer *g_pRenderer;
extern ICKFClient *g_pCKFClient;

void Renderer_Init(void);
void CKF_Init(void);
void Renderer_Shutdown(void);
void CKF_Shutdown(void);