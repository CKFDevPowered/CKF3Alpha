#include <metahook.h>
#include "client.h"
#include "exportfuncs.h"
#include "eventscripts.h"
#include "configs.h"
#include "plugins.h"
#include "hooks.h"
#include "cmd.h"
#include "scripts.h"
#include <progs.h>
#include "modules.h"
#include <cvardef.h>

extern engine_studio_api_t IEngineStudio;

#define MAX_LIGHTSTYLES 256
#define MAX_MODELS 512
#define MAX_SOUNDS 512
#define MAX_EVENTS 256
#define MAX_RESOURCES (MAX_MODELS + MAX_SOUNDS + MAX_EVENTS)
#define MAX_GENERIC 512

char g_szModelPrecache[512][MAX_QPATH];
int g_iModelPrecacheNums;

void CL_ClearCaches(void)
{
	for (int i = 0; i < MAX_MODELS; i++)
		g_szModelPrecache[i][0] = 0;

	g_iModelPrecacheNums = 0;
}

int CL_FindModelIndex(const char *pmodel)
{
	if (strlen(pmodel) > 12 && pmodel[8] == '_' && (pmodel[7] == 'p' || pmodel[7] == 'v'))
	{
		if (!strncmp(pmodel, "models/", 7))
		{
			for (int i = 0; i < 512; i++)
			{
				if (!strcmp(g_szModelPrecache[i], pmodel))
					return MAX_MODELS + i;
			}
		}
	}

	return 0;
}

struct model_s *CL_GetModelByIndex(int index)
{
	if (index == -1)
		return NULL;

	if (index >= MAX_MODELS)
		return IEngineStudio.Mod_ForName(g_szModelPrecache[index - MAX_MODELS], false);

	return g_pfnCL_GetModelByIndex(index);
}

void CL_AddToResourceList(resource_t *pResource, resource_t *pList)
{
	if (pResource->type == t_model)
	{
		if (strlen(pResource->szFileName) > 12 && pResource->szFileName[8] == '_' && (pResource->szFileName[7] == 'p' || pResource->szFileName[7] == 'v'))
		{
			if (!strncmp(pResource->szFileName, "models/", 7))
			{
				for (int i = 0; i < MAX_MODELS; i++)
				{
					if (!strcmp(g_szModelPrecache[i], pResource->szFileName))
						return;
				}

				IEngineStudio.Mod_ForName(pResource->szFileName, false);
				strcpy(g_szModelPrecache[g_iModelPrecacheNums++], pResource->szFileName);
				return;
			}
		}
	}
	else if (pResource->type == t_eventscript)
	{
		char name[32];
		sprintf(name, "%s", &pResource->szFileName[7]);
		name[strlen(name) - 3] = 0;

		Script *pScript;
		void (*pfnEvent)(struct event_args_s *args);

		if (!strcmp(name, "elite_left") || !strcmp(name, "elite_right"))
		{
			pScript = EVS_GetScript("elite");

			if (!pScript)
			{
				pScript = EVS_LoadScript("elite");

				if (pScript)
				{
					pfnEvent = (void (*)(struct event_args_s *))pScript->GetProcAddress("main");

					g_pfnEV_FireElite = pfnEvent;
					gEngfuncs.pfnHookEvent("events/elite_left.sc", EV_FireEliteLeft);
					gEngfuncs.pfnHookEvent("events/elite_right.sc", EV_FireEliteRight);
				}
			}
		}
		else if (!strcmp(pResource->szFileName, "decal_reset") || !strcmp(pResource->szFileName, "createsmoke") || !strcmp(pResource->szFileName, "createexplo"))
		{
		}
		else
		{
			pScript = EVS_GetScript(name);

			if (!pScript)
			{
				pScript = EVS_LoadScript(name);

				if (pScript)
				{
					pfnEvent = (void (*)(struct event_args_s *))pScript->GetProcAddress("main");

					if (pfnEvent)
						gEngfuncs.pfnHookEvent(pResource->szFileName, pfnEvent);
				}
			}
		}
	}

	g_pfnCL_AddToResourceList(pResource, pList);
}

typedef struct event_hook_s
{
	event_hook_s *next;
	char *name;
	void (*pfnEvent)(struct event_args_s *args);
}
event_hook_t;

event_hook_t *CL_FindEventHook(char *name)
{
	return g_pfnCL_FindEventHook(name);
}

void CL_FireEvents(void)
{
	g_pfnCL_FireEvents();
}

int CL_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
	return 0;
}

void CL_Init(void)
{
	cvar_t *cl_name = gEngfuncs.pfnGetCvarPointer("name");

	if (cl_name)
	{
		cl_name->flags &= ~FCVAR_PRINTABLEONLY;
	}

}

void CL_VidInit(void)
{
	CL_ClearCaches();
}

void CL_Frame(void)
{
}

void CL_Shutdown(void)
{
}