#ifndef VGUI_INTERNAL_H
#define VGUI_INTERNAL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "interface.h"
#include "tier3/tier3.h"
#include <ICommandLine.h>

namespace vgui
{

bool VGui_InternalLoadInterfaces(CreateInterfaceFn *factoryList, int numFactories);
bool VGui_LoadEngineInterfaces(CreateInterfaceFn vguiFactory, CreateInterfaceFn engineFactory);

extern class IInputInternal *g_pInput;
extern class ISchemeManager *g_pScheme;
extern class ISurface *g_pSurface;
extern class ISystem *g_pSystem;
extern class IVGui *g_pIVgui;
extern class IPanel *g_pIPanel;

void vgui_strcpy(char *dst, int dstLen, const char *src);
}

#include <cvardef.h>

extern class IEngineVGui *g_pEngineVGui;

extern class IGameUIFuncs *g_pGameUIFuncs;

extern struct cl_enginefuncs_s *engine;
extern cvar_t *cl_hud_minmode;

#endif