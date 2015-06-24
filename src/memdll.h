CreateInterfaceFn MEM_GetFactory(HINTERFACEMODULE hModule);
FARPROC WINAPI MEM_GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE WINAPI MEM_LoadLibrary(PBYTE pBuffer, BOOL bCallEntry, LPVOID lpvReserved);