#include <windows.h>
#include "plugins.h"
#include "tier0\memalloc.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hThisModule = hinstDLL;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
#ifdef _CRTDBG_MAP_ALLOC
			_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
			break;
		}

		case DLL_PROCESS_DETACH:
		{
#ifdef _DEBUG
			g_pMemAlloc->DumpStats();
#endif
			break;
		}
	}

	return TRUE;
}