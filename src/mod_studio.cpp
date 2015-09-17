#include <metahook.h>
#include "model.h"
#include "IceKey.h"
#include <studio.h>
#include <cvardef.h>
#include "plugins.h"
#include "configs.h"
#include "perf_counter.h"
#include "developer.h"
#include "zone.h"
#include "tier1/strtools.h"
#include "textures.h"
#include "console.h"

static CONST BYTE g_pDecryptorKey_20[32] =
{
	0x32, 0xA6, 0x21, 0xE0, 0xAB, 0x6B, 0xF4, 0x2C,
	0x93, 0xC6, 0xF1, 0x96, 0xFB, 0x38, 0x75, 0x68,
	0xBA, 0x70, 0x13, 0x86, 0xE0, 0xB3, 0x71, 0xF4,
	0xE3, 0x9B, 0x07, 0x22, 0x0C, 0xFE, 0x88, 0x3A
};

static CONST BYTE g_pDecryptorKey_21[32] =
{
	0x22, 0x7A, 0x19, 0x6F, 0x7B, 0x86, 0x7D, 0xE0,
	0x8C, 0xC6, 0xF1, 0x96, 0xFB, 0x38, 0x75, 0x68,
	0x88, 0x7A, 0x78, 0x86, 0x78, 0x86, 0x67, 0x70,
	0xD9, 0x91, 0x07, 0x3A, 0x14, 0x74, 0xFE, 0x22
};

static CIceKey g_Decryptor(4);

static VOID DecryptChunk(BYTE *pData, SIZE_T uDataSize)
{
	if (!uDataSize)
		return;

	SIZE_T uCount = (uDataSize + 7) >> 3;

	while (uCount)
	{
		g_Decryptor.Decrypt(pData, pData);
		pData += 8;
		uCount--;
	}
}

static VOID DecryptData(BYTE *pData, SIZE_T uDataSize)
{
	if (!uDataSize)
		return;

	do
	{
		SIZE_T uTempSize = uDataSize;

		if (uTempSize > 1024)
			uTempSize = 1024;

		if (uTempSize & 7)
			return;

		DecryptChunk(pData, uTempSize);
		pData += uTempSize;
		uDataSize -= uTempSize;
	}
	while (uDataSize);
}

typedef struct
{
	int numseq;
	int seqindex;
}
studioseqpack_t;

#define MAX_SEQUENCEPACKS 256

FileHandle_t g_hSequencePack[MAX_SEQUENCEPACKS];
unsigned int g_nSequenceSize[MAX_SEQUENCEPACKS];
int g_iSequenceNums = 0;

#define STUDIO_VERSION 10

#pragma warning(push)
#pragma warning(disable: 4244)

//void Mod_LoadStudioModel(model_t *mod, byte *buffer)
//{
//	studiohdr_t *phdr = (studiohdr_t *)buffer;
//	bool debugTime = ((int)developer->value > 2) ? true : false;
//	double startTime;
//
//	if (debugTime)
//		startTime = gPerformanceCounter.GetCurTime();
//
//	if (!strncmp(mod->name, "models/player", 13))
//	{
//		if (phdr->numhitboxes == 21)
//			phdr->numhitboxes = 20;
//	}
//
//	int version = LittleLong(phdr->version);
//
//	if (version == 20 || version == 21)
//	{
//		if (version == 20)
//			g_Decryptor.SetKey(g_pDecryptorKey_20);
//		else if (version == 21)
//			g_Decryptor.SetKey(g_pDecryptorKey_21);
//
//		mstudiotexture_t *ptexture = (mstudiotexture_t *)(buffer + phdr->textureindex);
//
//		for (int i = 0; i < phdr->numtextures; i++)
//			DecryptData(buffer + ptexture[i].index, (ptexture[i].width * ptexture[i].height) + (256 * 3));
//
//		mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)(buffer + phdr->bodypartindex);
//
//		for (int i = 0; i < phdr->numbodyparts; i++)
//		{
//			mstudiomodel_t *pmodel = (mstudiomodel_t *)(buffer + pbodypart[i].modelindex);
//
//			for (int j = 0; j < pbodypart[i].nummodels; j++)
//			{
//				if (pmodel[j].numverts > 0)
//					DecryptData(buffer + pmodel[j].vertindex, pmodel[j].numverts * sizeof(vec3_t));
//			}
//		}
//
//		version = STUDIO_VERSION;
//	}
//
//	if (version != STUDIO_VERSION)
//	{
//		Q_memset(phdr, 0, sizeof(studiohdr_t));
//		Q_strcpy(phdr->name, "bogus");
//
//		phdr->length = sizeof(studiohdr_t);
//		phdr->texturedataindex = sizeof(studiohdr_t);
//	}
//
//	mod->type = mod_studio;
//	mod->flags = phdr->flags;
//
//	if (phdr->textureindex != 0)
//	{
//		Cache_Alloc(&mod->cache, phdr->texturedataindex, mod->name);
//	}
//	else
//	{
//		Cache_Alloc(&mod->cache, phdr->length, mod->name);
//	}
//
//	if (mod->cache.data)
//	{
//		mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)mod->cache.data + phdr->textureindex);
//
//		if (phdr->textureindex != 0)
//		{
//			Q_memcpy(mod->cache.data, buffer, phdr->texturedataindex);
//
//			for (int i = 0; i < phdr->numtextures; i++)
//			{
//				if (ptexture->name[0] == '#')
//				{
//					int cwidth, cheight;
//
//					if (sscanf(ptexture->name + 1, "%3d%3d", &cwidth, &cheight) == 2)
//					{
//						ptexture->width = cwidth;
//						ptexture->height = cheight;
//					}
//				}
//				else
//				{
//					qboolean mipmap;
//					char name[256];
//					byte *pal = (byte *)buffer + ptexture->index + (ptexture->height * ptexture->width);
//					Q_snprintf(name, sizeof(name), "%s%s", mod->name, ptexture->name);
//
//					if (ptexture->flags & STUDIO_NF_NOMIPS)
//						mipmap = false;
//					else
//						mipmap = true;
//
//					ptexture->index = GL_LoadTexture(name, GLT_STUDIO, ptexture->width, ptexture->height, (byte *)buffer + ptexture->index, mipmap, (ptexture->flags & STUDIO_NF_MASKED) ? TEX_TYPE_ALPHA : TEX_TYPE_NONE, pal);
//				}
//
//				ptexture++;
//			}
//		}
//		else
//		{
//			Q_memcpy(mod->cache.data, buffer, phdr->length);
//		}
//	}
//
//	if (debugTime)
//		Con_Printf("Mod_LoadStudioModel: loaded model %s in time %.4f sec.\n", mod->name, gPerformanceCounter.GetCurTime() - startTime);
//}

#pragma warning(pop)