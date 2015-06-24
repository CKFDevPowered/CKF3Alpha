#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

#include <winsani_in.h>
#include <windows.h>
#include <winsani_out.h>

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

typedef float vec_t;
typedef float vec2_t[2];

#ifndef vec3_t
typedef float vec3_t[3];
#endif

#include <wrect.h>
#include <interface.h>
#include <cdll_int.h>
#include <cvardef.h>

extern cl_enginefunc_t *engine;

namespace vgui
{
	class ISurface;
};

class IServerBrowser;

#include <IBaseUI.h>
#include <VGUI/KeyCode.h>
#include <IGameUIFuncs.h>
#include <IEngineVGui.h>

extern IBaseUI *baseuifuncs;
extern IGameUIFuncs *gameuifuncs;

#endif