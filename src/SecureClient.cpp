#include <metahook.h>
#include "SecureClient.h"
#include "plugins.h"

#define ISSECURECLIENT_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x54\x24\x04\x83\xEC\x44\x8B\x01\x56\x6A\x00\x68\x2A\x2A\x2A\x2A\x52\xFF\x50\x28\x8B\x0D"

int (*g_pfnIsSecureClient)(char *pszDllName);
int (*g_pfnLoadSecureClient)(char *pszDllName);

int IsSecureClient(char *pszDllName)
{
	strcpy(pszDllName, "cl_dlls\\blob_client.dll");
	return g_pfnIsSecureClient(pszDllName);
}

void SecureClient_InstallHook(void)
{
	if (g_dwEngineBuildnum >= 5953)
		return;

	g_pfnIsSecureClient = (int (*)(char *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, ISSECURECLIENT_SIG, sizeof(ISSECURECLIENT_SIG) - 1);

	if (g_pfnIsSecureClient)
		g_pMetaHookAPI->InlineHook(g_pfnIsSecureClient, IsSecureClient, (void *&)g_pfnIsSecureClient);
}