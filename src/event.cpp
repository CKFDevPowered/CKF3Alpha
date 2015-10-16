#include <metahook.h>
#include <event_api.h>
#include "client.h"

int (*g_pfnEV_FindModelIndex)(const char *pmodel);

int EV_FindModelIndex(const char *pmodel)
{
	int index = g_pfnEV_FindModelIndex(pmodel);

	if (index == 0)
		return CL_FindModelIndex(pmodel);

	return index;
}

void EV_InstallHook(void)
{
	//g_pMetaHookAPI->InlineHook(g_pMetaSave->pEngineFuncs->pEventAPI->EV_FindModelIndex, EV_FindModelIndex, (void *&)g_pfnEV_FindModelIndex);
}