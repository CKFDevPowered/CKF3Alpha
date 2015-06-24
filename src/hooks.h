extern void (*g_pfnMod_LoadStudioModel)(struct model_s *mod, byte *buffer);
extern struct model_s *(*g_pfnCL_GetModelByIndex)(int index);
extern void (*g_pfnCL_AddToResourceList)(struct resource_s *pResource, struct resource_s *pList);
extern void (*g_pfnCL_FireEvents)(void);
extern struct event_hook_s *(*g_pfnCL_FindEventHook)(char *name);
extern void (*g_pfnInfo_SetValueForKey)(char *s, char *key, char *value, int maxsize);
extern int (*g_pfnLoadTGA)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
extern int (*g_pfnGL_LoadTexture)(char *identifier, int textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPa);
extern int (*g_pfnGL_LoadTexture2)(char *identifier, int textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal, int filter);
extern void (*g_pfnGL_Bind)(int texnum);
extern HRESULT (CALLBACK *g_pVID_EnumDisplayModesProc)(void *lpDDSurfaceDesc, DWORD *pBPP);
extern class CVideoMode_Common *(*g_pfnVideoMode_Create)(void);

void InstallHook(void);