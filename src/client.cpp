#include <metahook.h>
#include "client.h"
#include "exportfuncs.h"
#include "eventscripts.h"
#include "configs.h"
#include "plugins.h"
#include "hooks.h"
#include "cmd.h"
#include "scripts.h"
#include "demo.h"
#include <progs.h>
#include "modules.h"
#include <cvardef.h>
#include "strtools.h"

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

qboolean CL_IsNewPrecachedModel(const char *pszFileName)
{
	//if( !Q_strncmp(pszFileName, "models/CKF_III/", sizeof("models/CKF_III/") - 1) || !Q_strncmp(pszFileName, "models\\CKF_III\\", sizeof("models\\CKF_III\\") - 1) )
	//{
	//	const char *p = pszFileName + sizeof("models/CKF_III/") - 1;
	//	if( p[0] == 'v' && p[1] == '_' )
	//		return true;
	//}
	return false;
}

int CL_FindModelIndex(const char *pmodel)
{
	if (CL_IsNewPrecachedModel(pmodel))
	{
		for (int i = 0; i < 512; i++)
		{
			if (!Q_strcmp(g_szModelPrecache[i], pmodel))
				return MAX_MODELS + i;
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

	return gHookFuncs.CL_GetModelByIndex(index);
}

void CL_AddToResourceList(resource_t *pResource, resource_t *pList)
{
	if (pResource->type == t_model)
	{
		//if ( CL_IsNewPrecachedModel(pResource->szFileName) && !(pResource->ucFlags & RES_CHECKFILE) && g_iModelPrecacheNums < 512 )
		//{
		//	for (int i = 0; i < MAX_MODELS; i++)
		//	{
		//		if (!Q_strcmp(g_szModelPrecache[i], pResource->szFileName))
		//			return;
		//	}

		//	IEngineStudio.Mod_ForName(pResource->szFileName, false);
		//	strcpy(g_szModelPrecache[g_iModelPrecacheNums++], pResource->szFileName);
		//	return;
		//}
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

	gHookFuncs.CL_AddToResourceList(pResource, pList);
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
	return gHookFuncs.CL_FindEventHook(name);
}

//void CL_FireEvents(void)
//{
//	g_pfnCL_FireEvents();
//}

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

void CL_ReadClientDLLData( void )
{
	static byte	data[0x8000];
	int		i;

	Q_memset( data, 0, sizeof(data) );
	g_pFileSystem->Read( &i, sizeof(int), *gHookFuncs.cls_demofile );
	i = min( LittleLong(i), sizeof(data) );
	g_pFileSystem->Read( data, i, *gHookFuncs.cls_demofile );
	Demo_ReadBuffer( i, data );
}

void CL_DemoParseSound( void )
{
	int		channel;
	char	sample[256];
	float	volume;
	float	attenuation;
	int		flags;
	int		pitch;
	int		i;

	g_pFileSystem->Read( &channel, sizeof(int), *gHookFuncs.cls_demofile );
	channel = LittleLong (channel);

	g_pFileSystem->Read( &i, sizeof(int), *gHookFuncs.cls_demofile );
	i = min(LittleLong(i), sizeof(sample)-1);

	g_pFileSystem->Read( sample, i, *gHookFuncs.cls_demofile );
	sample[i] = 0;

	g_pFileSystem->Read( &attenuation, sizeof(int), *gHookFuncs.cls_demofile );
	LittleFloat (&attenuation, &attenuation);

	g_pFileSystem->Read( &volume, sizeof(int), *gHookFuncs.cls_demofile );
	LittleFloat (&volume, &volume);

	g_pFileSystem->Read( &flags, sizeof(int), *gHookFuncs.cls_demofile );
	flags = LittleLong (flags);

	g_pFileSystem->Read( &pitch, sizeof(int), *gHookFuncs.cls_demofile );
	pitch = LittleLong (pitch);

	gHookFuncs.CL_DemoPlaySound(channel, sample, attenuation, volume, flags, pitch);
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