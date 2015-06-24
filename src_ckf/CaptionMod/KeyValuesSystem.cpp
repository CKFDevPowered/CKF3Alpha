#include <metahook.h>
#include "interface.h"
#include <KeyValues.h>
#include "FileSystem.h"
#include "IKeyValuesSystem.h"

void (__fastcall *g_pfnCKeyValuesSystem_RegisterSizeofKeyValues)(void *pthis, int, int size);
void *(__fastcall *g_pfnCKeyValuesSystem_AllocKeyValuesMemory)(void *pthis, int, int size);
void (__fastcall *g_pfnCKeyValuesSystem_FreeKeyValuesMemory)(void *pthis, int, void *pMem);
HKeySymbol (__fastcall *g_pfnCKeyValuesSystem_GetSymbolForString)(void *pthis, int, const char *name);
const char *(__fastcall *g_pfnCKeyValuesSystem_GetStringForSymbol)(void *pthis, int, HKeySymbol symbol);
HLocalized (__fastcall *g_pfnCKeyValuesSystem_GetLocalizedFromANSI)(void *pthis, int, const char *string);
const char *(__fastcall *g_pfnCKeyValuesSystem_GetANSIFromLocalized)(void *pthis, int, HLocalized l);
void (__fastcall *g_pfnCKeyValuesSystem_AddKeyValuesToMemoryLeakList)(void *pthis, int, void *pMem, HKeySymbol name);
void (__fastcall *g_pfnCKeyValuesSystem_RemoveKeyValuesFromMemoryLeakList)(void *pthis, int, void *pMem);

class CKeyValuesSystem : public IKeyValuesSystem
{
public:
	virtual void RegisterSizeofKeyValues(int size);
	virtual void *AllocKeyValuesMemory(int size);
	virtual void FreeKeyValuesMemory(void *pMem);
	virtual HKeySymbol GetSymbolForString(const char *name);
	virtual const char *GetStringForSymbol(HKeySymbol symbol);
	virtual HLocalized GetLocalizedFromANSI(const char *string);
	virtual const char *GetANSIFromLocalized(HLocalized l);
	virtual void AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name);
	virtual void RemoveKeyValuesFromMemoryLeakList(void *pMem);
};

void CKeyValuesSystem::RegisterSizeofKeyValues(int size)
{
	size = sizeof(KeyValues);
	g_pfnCKeyValuesSystem_RegisterSizeofKeyValues(this, 0, size);
}

void *CKeyValuesSystem::AllocKeyValuesMemory(int size)
{
	return g_pfnCKeyValuesSystem_AllocKeyValuesMemory(this, 0, size);
}

void CKeyValuesSystem::FreeKeyValuesMemory(void *pMem)
{
	g_pfnCKeyValuesSystem_FreeKeyValuesMemory(this, 0, pMem);
}

HKeySymbol CKeyValuesSystem::GetSymbolForString(const char *name)
{
	return g_pfnCKeyValuesSystem_GetSymbolForString(this, 0, name);
}

const char *CKeyValuesSystem::GetStringForSymbol(HKeySymbol symbol)
{
	return g_pfnCKeyValuesSystem_GetStringForSymbol(this, 0, symbol);
}

HLocalized CKeyValuesSystem::GetLocalizedFromANSI(const char *string)
{
	return g_pfnCKeyValuesSystem_GetLocalizedFromANSI(this, 0, string);
}

const char *CKeyValuesSystem::GetANSIFromLocalized(HLocalized l)
{
	return g_pfnCKeyValuesSystem_GetANSIFromLocalized(this, 0, l);
}

void CKeyValuesSystem::AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name)
{
	g_pfnCKeyValuesSystem_AddKeyValuesToMemoryLeakList(this, 0, pMem, name);
}

void CKeyValuesSystem::RemoveKeyValuesFromMemoryLeakList(void *pMem)
{
	g_pfnCKeyValuesSystem_RemoveKeyValuesFromMemoryLeakList(this, 0, pMem);
}

void KeyValuesSystem_InstallHook(IKeyValuesSystem *pKeyValuesSystem)
{
	CKeyValuesSystem KeyValuesSystem;
	DWORD *pVFTable = *(DWORD **)&KeyValuesSystem;

	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 1, (void *)pVFTable[1], (void *&)g_pfnCKeyValuesSystem_RegisterSizeofKeyValues);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 2, (void *)pVFTable[2], (void *&)g_pfnCKeyValuesSystem_AllocKeyValuesMemory);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 3, (void *)pVFTable[3], (void *&)g_pfnCKeyValuesSystem_FreeKeyValuesMemory);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 4, (void *)pVFTable[4], (void *&)g_pfnCKeyValuesSystem_GetSymbolForString);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 5, (void *)pVFTable[5], (void *&)g_pfnCKeyValuesSystem_GetStringForSymbol);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 6, (void *)pVFTable[6], (void *&)g_pfnCKeyValuesSystem_GetLocalizedFromANSI);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 7, (void *)pVFTable[7], (void *&)g_pfnCKeyValuesSystem_GetANSIFromLocalized);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 8, (void *)pVFTable[8], (void *&)g_pfnCKeyValuesSystem_AddKeyValuesToMemoryLeakList);
	g_pMetaHookAPI->VFTHook(pKeyValuesSystem, 0, 9, (void *)pVFTable[9], (void *&)g_pfnCKeyValuesSystem_RemoveKeyValuesFromMemoryLeakList);
}