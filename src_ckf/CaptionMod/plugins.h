#pragma once

extern DWORD g_dwEngineBase, g_dwEngineSize;
extern DWORD g_dwClientBase, g_dwClientSize;
extern DWORD g_dwEngineBuildnum;
extern DWORD g_iVideoMode;
extern int g_iVideoWidth, g_iVideoHeight;
extern bool g_bWindowed;
extern IFileSystem *g_pFileSystem;
extern HMODULE g_hThisModule, g_hEngineModule;
extern bool g_bIsUseSteam;