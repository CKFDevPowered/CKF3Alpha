#ifndef VGUI_INTERNAL_H
#define VGUI_INTERNAL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "interface.h"
#include "tier3/tier3.h"

namespace vgui
{

bool VGui_InternalLoadInterfaces(CreateInterfaceFn *factoryList, int numFactories);

extern class IInputInternal *g_pInput;
extern class ISchemeManager *g_pScheme;
extern class ISurface *g_pSurface;
extern class ISystem *g_pSystem;
extern class IVGui *g_pIVgui;
extern class ILocalize *g_pLocalize;
extern class IPanel *g_pIPanel;

void vgui_strcpy(char *dst, int dstLen, const char *src);

}

#endif