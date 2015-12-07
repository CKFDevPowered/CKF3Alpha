#include <metahook.h>
#include <net_api.h>
#include <cvardef.h>
#include "mempatchs.h"
#include "plugins.h"

#define SYS_INITMEMORY_SIG "\x83\xEC\x24\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x85\xC0\x74\x2A\x8B\x0D\x2A\x2A\x2A\x2A\x49"

void MemPatch_HeapSizeLimitValue(void)
{
	if (g_dwEngineBuildnum >= 5953)
		return;

	int maxvalue = 128 * (1024 * 1024);
	DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, SYS_INITMEMORY_SIG, sizeof(SYS_INITMEMORY_SIG) - 1);

	if (!addr)
	{
		MessageBox(NULL, "HeapSizeLimitValue patch failed!", "Warning", MB_ICONWARNING);
		return;
	}

	DWORD addr2 = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, 0x150, "\x3D\x2A\x2A\x2A\x2A\x7E", 6);
	DWORD addr3 = (DWORD)g_pMetaHookAPI->SearchPattern((void *)(addr2 + 0x8), 0x150, "\x3D\x2A\x2A\x2A\x2A\x7E", 6);

	if (addr2 && addr3)
	{
		g_pMetaHookAPI->WriteDWORD((void *)(addr2 + 0x1), maxvalue);
		g_pMetaHookAPI->WriteDWORD((void *)(addr2 + 0x8), maxvalue);
		g_pMetaHookAPI->WriteDWORD((void *)(addr3 + 0x1), maxvalue);
		g_pMetaHookAPI->WriteDWORD((void *)(addr3 + 0xD), maxvalue);
	}
}

void MemPatch_WideScreenLimit(void)
{
	if (g_dwEngineBuildnum >= 5953)
		return;

	DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, "\x8B\x51\x08\x8B\x41\x0C\x8B\x71\x54\x8B\xFA\xC1\xE7\x04", 14);

	if (!addr)
	{
		MessageBox(NULL, "WideScreenLimit patch failed!", "Warning", MB_ICONWARNING);
		return;
	}

	DWORD addr2 = addr + 11;
	DWORD addr3 = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, 0x60, "\xB1\x01\x8B\x7C\x24\x14", 6);

	if (addr3)
	{
		g_pMetaHookAPI->WriteNOP((void *)addr2, addr3 - addr2);
	}
}

void MemPatch_PatchSteamCheckSubscribed(void)
{
	if (g_bIsUseSteam)
		return;

	unsigned char data[] = { 0x33, 0xC0, 0xC3 };
	DWORD addr = (DWORD)GetProcAddress(GetModuleHandle("steam.dll"), "SteamIsAppSubscribed");

	if (!addr)
		return;

	g_pMetaHookAPI->WriteMemory((void *)addr, data, sizeof(data));
}

void MemPatch_Start(MEMPATCH_STEP step)
{
	switch (step)
	{
		case MEMPATCH_STEP_LOADENGINE:
		{
			MemPatch_HeapSizeLimitValue();
			MemPatch_WideScreenLimit();
			MemPatch_PatchSteamCheckSubscribed();
			break;
		}

		case MEMPATCH_STEP_LOADCLIENT:
		{
			break;
		}

		case MEMPATCH_STEP_INITCLIENT:
		{
			break;
		}
	}
}