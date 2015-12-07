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

#include "cmd.h"

void SVC_Init(void)
{
	DWORD addr_svc_bad = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, "svc_bad", sizeof("svc_bad")-1);
	DWORD addr_svc_nop = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr_svc_bad, 0x100, "svc_nop", sizeof("svc_nop")-1);

	int svc[6] = {0, addr_svc_bad, 0, 1, addr_svc_nop, 0};
	char *sig_svc = (char *)svc;

	DWORD addr_cl_parsefuncs = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig_svc, sizeof(svc) );

	if (addr_cl_parsefuncs)
	{
		cl_parsefuncs = (svc_func_t *)addr_cl_parsefuncs;

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

	DWORD addr_cl_parse_director = (DWORD)cl_parsefuncs[51].pfnParse;
	if (addr_cl_parse_director)
	{
		msg_readcount = (int *)(*(DWORD *)(addr_cl_parse_director + 8));
		net_message = (sizebuf_t *)(*(DWORD *)(addr_cl_parse_director + 15) - 8);
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