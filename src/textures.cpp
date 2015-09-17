#include <metahook.h>
#include "textures.h"
#include "hooks.h"
#include "configs.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"

//gltexture_t gltextures[MAX_GLTEXTURES];
//int numgltextures;
//
//int GL_LoadTexture(char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal)
//{
//	if (textureType != GLT_STUDIO)
//		return g_pfnGL_LoadTexture(identifier, textureType, width, height, data, mipmap, iType, pPal);
//
//	size_t len = strlen(identifier);
//	char *n_identifier = strstr(identifier, ".mdl");
//	bool debugTime = (developer && (int)developer->value > 2);
//	double startTime;
//
//	if (n_identifier)
//	{
//		n_identifier += 4;
//
//		if (n_identifier[0] == '#')
//		{
//			char *check = &n_identifier[7];
//
//			if (!strncmp(&n_identifier[7], "pw_", 3))
//				check += 3;
//
//			for (int i = 0; i < numgltextures; i++)
//			{
//				if (!strcmp(gltextures[i].identifier, check))
//					return gltextures[i].texnum;
//			}
//
//			char models[32];
//			size_t mlen = n_identifier - identifier;
//			strncpy(models, identifier, mlen);
//			models[mlen] = 0;
//			Con_DPrintf("GL_LoadTexture: texture %s missing from %s\n", n_identifier, models);
//			return 0;
//		}
//	}
//	else
//	{
//		n_identifier = identifier;
//	}
//
//	if (debugTime)
//		startTime = gPerformanceCounter.GetCurTime();
//
//	int result = g_pfnGL_LoadTexture(identifier, textureType, width, height, data, mipmap, iType, pPal);
//	strcpy(gltextures[result].identifier, n_identifier);
//	gltextures[result].width = width;
//	gltextures[result].height = height;
//	gltextures[result].texnum = result;
//	numgltextures = result + 1;
//
//	if (debugTime)
//		Con_Printf("GL_LoadTexture: loaded texture %s in time %.4f sec.\n", identifier ? identifier : "*", gPerformanceCounter.GetCurTime() - startTime);
//
//	return result;
//}