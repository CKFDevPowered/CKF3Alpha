#ifndef IGAMECONSOLE_H
#define IGAMECONSOLE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"

class Color;

class IGameConsole : public IBaseInterface
{
public:
	virtual void Activate(void) = 0;
	virtual void Initialize(void) = 0;
	virtual void Hide(void) = 0;
	virtual void Clear(void) = 0;
	virtual bool IsConsoleVisible(void) = 0;
	virtual void Printf(const char *format, ...) = 0;
	virtual void DPrintf(const char *format, ...) = 0;
	virtual void SetParent(int parent) = 0;
};

#define GAMECONSOLE_INTERFACE_VERSION "GameConsole003"

#endif