#include <metahook.h>
#include "plugins.h"
#include "sizebuf.h"
#include "svchook.h"

int SVC_LASTMSG = 50;

typedef struct svc_func_s
{
	unsigned char opcode;
	char *pszname;
	pfnSVC_Parse pfnParse;
}
svc_func_t;

static svc_func_t *cl_parsefuncs = NULL;
static sizebuf_t *net_message = NULL;
static int *msg_readcount = NULL;

void SVC_Init(void)
{
	DWORD address = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, "\x55\x8B\xEC\x83\xE4\xF8\x51\x53\x56\x57\x68", 11);

	if (address)
	{
		cl_parsefuncs = (svc_func_t *)(*(DWORD *)(address + 0x1C) - 0x4);

		while (SVC_LASTMSG)
		{
			if (cl_parsefuncs[SVC_LASTMSG].opcode == 0xFF)
			{
				SVC_LASTMSG -= 1;
				break;
			}

			SVC_LASTMSG++;
		}
	}

	address = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, "\xA1\x2A\x2A\x2A\x2A\x8B\x15\x2A\x2A\x2A\x2A\x8D\x48\x01\x3B\xCA\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x01\x00\x00\x00\x83\xC8\xFF\xC3", 32);

	if (address)
	{
		msg_readcount = (int *)(*(DWORD *)(address + 0x1));
		net_message = (sizebuf_t *)(*(DWORD *)(address + 0x7) - 0x10);
	}
}

pfnSVC_Parse SVC_HookFunc(int opcode, pfnSVC_Parse pfnParse)
{
	pfnSVC_Parse pfnResult = cl_parsefuncs[opcode].pfnParse;

	cl_parsefuncs[opcode].pfnParse = pfnParse;
	return pfnResult;
}

void *SVC_GetBuffer(void)
{
	return net_message->data + *msg_readcount;
}

int SVC_GetBufferSize(void)
{
	return net_message->cursize - *msg_readcount;
}