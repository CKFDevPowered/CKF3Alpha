#ifndef IKEYVALUESSYSTEM_H
#define IKEYVALUESSYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "vstdlib/vstdlib.h"

typedef int HKeySymbol;
#define INVALID_KEY_SYMBOL (-1)

class IKeyValuesSystem : public IBaseInterface
{
public:
	virtual void RegisterSizeofKeyValues(int size) = 0;
	virtual void *AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void *pMem) = 0;
	virtual HKeySymbol GetSymbolForString(const char *name) = 0;
	virtual const char *GetStringForSymbol(HKeySymbol symbol) = 0;
	virtual void GetLocalizedFromANSI(const char *ansi, wchar_t *outBuf, int unicodeBufferSizeInBytes) = 0;
	virtual void GetANSIFromLocalized(const wchar_t *wchar, char *outBuf, int ansiBufferSizeInBytes) = 0;
	virtual void AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void *pMem) = 0;
};

IKeyValuesSystem *KeyValuesSystem(void);

#endif
