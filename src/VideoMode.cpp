#define _WIN32_WINNT 0x0501
#include <metahook.h>
#include "plugins.h"
#include <UtlVector.h>
#include "qgl.h"
#include "hooks.h"
#include "FileSystem.h"
#include "VideoMode.h"
#include "Video.h"
#include "LoadTGA.h"
#include "LoadBMP.h"
#include <IRenderer.h>

struct bimage_t
{
	byte *buffer;
	int x, y, width, height;
	int srcwidth, srcheight;
	int layer;
	bool scaled;
	//only for DrawStartupGraphic_GL
	GLuint glt;
};

typedef CUtlVector<bimage_t> bimagegroup_t;

static CUtlVector<bimagegroup_t> g_ImageLayers;

void __fastcall LoadStartupGraphic(void *pthis, int)
{
	int texBufSize;
	int width, height;
	char token[512];
	char fileExtension[32];

	g_ImageLayers.RemoveAll();

	FileHandle_t file = g_pFileSystem->Open( "resource/LoadingLayout.txt", "rb" );

	if ( !file )
		return;

	int fileSize = g_pFileSystem->Size( file );
	char *buffer = (char *)malloc( fileSize+1 );

	if(!buffer)
		return;

	char *filebuffer = buffer;
	memset( buffer, 0, fileSize );

	g_pFileSystem->Read( buffer, fileSize, file );
	g_pFileSystem->Close( file );

	byte *texBuffer = gRefExports.R_GetTexLoaderBuffer(&texBufSize);

	buffer[fileSize] = '\0';

	double scaleH, scaleW;

	scaleW = (double)g_iVideoWidth / (double)640;
	scaleH = (double)g_iVideoHeight / (double)480;

	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	while ( buffer && *buffer )
	{
		buffer = g_pFileSystem->ParseFile( buffer, token, NULL );

		if ( !buffer || !buffer[0] )
			break;

		int (*pfnLoadImage)(const char *, byte *, int , int *, int *);

		V_ExtractFileExtension(token, fileExtension, sizeof(fileExtension));

		if(!Q_stricmp(fileExtension, "tga"))
			pfnLoadImage = LoadTGA;
		else if(!Q_stricmp(fileExtension, "png"))
			pfnLoadImage = LoadPNG;
		else if(!Q_stricmp(fileExtension, "bmp"))
			pfnLoadImage = LoadBMP;
		else//unsupported image format
			break;

		if(pfnLoadImage(token, texBuffer, texBufSize, &width, &height))
		{
			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );

			if ( !buffer || !buffer[0] )
				break;

			int layer = atoi(token);
			if(layer < 0 || layer > 9)
				layer = 0;

			g_ImageLayers.EnsureCount(layer + 1);
			bimagegroup_t &group = g_ImageLayers[layer];

			bimage_t &bimage = group[group.AddToTail()];

			bimage.buffer = (byte *)malloc(width * height * 4);
			memcpy(bimage.buffer, texBuffer, width * height * 4);

			bimage.x = 0;
			bimage.y = 0;
			bimage.width = 0;
			bimage.height = 0;
			bimage.srcwidth = width;
			bimage.srcheight = height;
			bimage.glt = 0;
			bimage.layer = layer;

			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );
			bimage.scaled = ( stricmp(token, "scaled") == 0 );

			if ( !buffer || !buffer[0] )
				break;

			double whscale = (bimage.scaled) ? scale : 1;

			if ( !buffer || !buffer[0] )
				break;

			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );
			
			if(token[0] == 'c')
			{
				bimage.x = floor(320 * scaleW + atof( token + 1 ) * whscale);
			}
			else if(token[0] == 'r' || token[0] == 'f')
			{
				bimage.x = floor(640 * scaleW + atof( token + 1 ) * whscale);
			}
			else
			{
				bimage.x = floor(atof( token ) * whscale);
			}

			if ( !buffer || !buffer[0] )
				break;

			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );

			if(token[0] == 'c')
			{
				bimage.y = 240 * scaleH + atof( token + 1 ) * whscale;
			}
			else if(token[0] == 'r' || token[0] == 'f')
			{
				bimage.y = 480 * scaleH + atof( token + 1 ) * whscale;
			}
			else
			{
				bimage.y = atof( token ) * whscale;
			}

			if ( !buffer || !buffer[0] )
				break;

			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );

			if(token[0] == 'r' || token[0] == 'f')
			{
				bimage.width = ceil(640 * scaleW + atof( token + 1 ) * whscale);
			}
			else
			{
				bimage.width = ceil(atof( token ) * whscale);
			}

			if ( !buffer || !buffer[0] )
				break;

			buffer = g_pFileSystem->ParseFile( buffer, token, NULL );

			if(token[0] == 'r' || token[0] == 'f')
			{
				bimage.height = 480 * scaleH + atof( token + 1 ) * whscale;
			}
			else
			{
				bimage.height = atof( token ) * whscale;
			}
		}
	}

	if(filebuffer)
		free(filebuffer);
}

void BlitGraphicToHDCWithAlpha(HDC hdc, byte *rgba, int imageWidth, int imageHeight, int x, int y, int w, int h)
{
	int wide = x+w;
	int tall = y+h;

	for ( int y0 = y; y0 < tall; y0++ )
	{
		if(y0 < 0 || y0 > g_iVideoHeight)
			continue;
		byte *ysrc = (byte *)(rgba + imageWidth * 4 * ((y0-y) * imageHeight / h) );
		for ( int x0 = x; x0 < wide; x0++ )
		{
			if(x0 < 0 || x0 > g_iVideoWidth)
				continue;
			byte *xsrc = (byte *)(ysrc + 4 * ((x0-x) * imageWidth / w) );
			if (xsrc[3])
			{
				::SetPixel(hdc, x0, y0, RGB(xsrc[0], xsrc[1], xsrc[2]));
			}
		}
	}
}

void __fastcall DrawStartupGraphic_GDI(void *pthis, int, HWND hWnd)
{
	int x, y, winW, winH;

	(*g_pGame)->GetWindowRect(&x, &y, &winW, &winH);

	int videoW = g_iVideoWidth;
	int videoH = g_iVideoHeight;

	float videoAspect = (float)videoW / videoH;
	float windowAspect = (float)winW / winH;

	int realX = 0;
	int realY = 0;
	int realX2 = winW;
	int realY2 = winH;

	if ( CommandLine()->CheckParm("-stretchaspect") == NULL )
	{
		if ( windowAspect < videoAspect )
		{
			realY = (winH - 1.0 / videoAspect * winW) / 2.0;
			realY2 = winH - realY;
		}
		else
		{
			realX = (winW - winH * videoAspect) / 2.0;
			realX2 = winW - realX;
		}
	}

	HDC dc = ::GetDC( hWnd );

	HBITMAP bm = ::CreateCompatibleBitmap( dc, videoW, videoH );
	HDC memdc = ::CreateCompatibleDC( dc );
	SelectObject(memdc, bm);

	for ( int i = 0; i < g_ImageLayers.Size(); i++ )
	{
		bimagegroup_t &group = g_ImageLayers[i];
		int index = g_pMetaSave->pEngineFuncs->pfnRandomLong(0, group.Size()-1);
		bimage_t &bimage = group[index];

		BlitGraphicToHDCWithAlpha(memdc, bimage.buffer, bimage.srcwidth, bimage.srcheight, bimage.x, bimage.y, bimage.width, bimage.height);
	}

	SetViewportOrgEx( dc, 0, 0, NULL );
	//BitBlt( dc, 0, 0, videoW, videoH, memdc, 0, 0, SRCCOPY );
	StretchBlt( dc, realX, realY, realX2-realX, realY2-realY, memdc, 0, 0, videoW, videoH, SRCCOPY);
	DeleteObject( bm );
	DeleteDC( memdc );
	ReleaseDC( hWnd, dc );

	for ( int i = 0; i < g_ImageLayers.Size(); i++ )
	{
		bimagegroup_t &group = g_ImageLayers[i];
		for(int j = 0; j < group.Size(); ++j)
		{
			bimage_t &bimage = group[j];
			free(bimage.buffer);
		}
	}
	g_ImageLayers.RemoveAll();
}

void __fastcall DrawStartupGraphic_GL(void *pthis, int, HWND hWnd)
{
	int winW, winH;
	int x, y;

	if(g_dwEngineBuildnum >= 5953)
		gHookFuncs.SDL_GetWindowSize(hWnd, &winW, &winH);
	else
		(*g_pGame)->GetWindowRect(&x, &y, &winW, &winH);

	Msg("DrawStartupGraphic_GL: winW %d winH %d\n", winW, winH);

	int videoW = g_iVideoWidth;
	int videoH = g_iVideoHeight;

	qglDisable(GL_LIGHTING);
	qglDisable(GL_ALPHA_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_DEPTH_TEST);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0, videoW, videoH, 0, -1, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	qglClearColor(0.0, 0.0, 0.0, 0.0);
	qglClear(GL_COLOR_BUFFER_BIT);

	qglEnable(GL_TEXTURE_2D);

	float videoAspect = (float)videoW / videoH;
	float windowAspect = (float)winW / winH;

	int realX = 0;
	int realY = 0;
	int realX2 = winW;
	int realY2 = winH;

	if ( CommandLine()->CheckParm("-stretchaspect") == NULL )
	{
		if ( windowAspect < videoAspect )
		{
			realY = (winH - 1.0 / videoAspect * winW) / 2.0;
			realY2 = winH - realY;
		}
		else
		{
			realX = (winW - winH * videoAspect) / 2.0;
			realX2 = winW - realX;
		}
	}

	qglViewport(realX, realY, realX2-realX, realY2-realY);

	for ( int i = 0; i < g_ImageLayers.Size(); i++ )
	{
		bimagegroup_t &group = g_ImageLayers[i];
		int index = g_pMetaSave->pEngineFuncs->pfnRandomLong(0, group.Size()-1);
		bimage_t &bimage = group[index];

		qglGenTextures(1, &bimage.glt);
		qglBindTexture(GL_TEXTURE_2D, bimage.glt);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bimage.srcwidth, bimage.srcheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bimage.buffer);

		qglColor4f(1, 1, 1, 1);

		int dstX, dstY, dstX2, dstY2;
		dstX = bimage.x;
		dstY = bimage.y;
		dstX2 = dstX + bimage.width;
		dstY2 = dstY + bimage.height;

		qglBegin(GL_QUADS);
		qglTexCoord2f(0, 0);
		qglVertex3f(dstX, dstY, 0);

		qglTexCoord2f(1, 0);
		qglVertex3f(dstX2, dstY, 0);

		qglTexCoord2f(1, 1);
		qglVertex3f(dstX2, dstY2, 0);

		qglTexCoord2f(0, 1);
		qglVertex3f(dstX, dstY2, 0);
		qglEnd();

		Msg("DrawStartupGraphic_GL image at %d %d %d %d\n", dstX, dstY, dstX2, dstY2);
	}

	qglMatrixMode(GL_MODELVIEW);
	qglBindTexture(GL_TEXTURE_2D, 0);
	qglDisable(GL_TEXTURE_2D);

	gRefExports.RefAPI.GL_SwapBuffer();

	for ( int i = 0; i < g_ImageLayers.Size(); i++ )
	{
		bimagegroup_t &group = g_ImageLayers[i];
		for(int j = 0; j < group.Size(); ++j)
		{
			bimage_t &bimage = group[j];
			qglDeleteTextures(1, &bimage.glt);
			free(bimage.buffer);
		}
	}
	g_ImageLayers.RemoveAll();
}

CVideoMode_Common *VideoMode_Create(void)
{
	CVideoMode_Common *videomode = gHookFuncs.VideoMode_Create();
	
	if(videomode != NULL)
	{
	#define IGAME_SIG "\x8B\x0D"
		DWORD *pVFTable = *(DWORD **)videomode;
		DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pVFTable[10], 0x20, IGAME_SIG, sizeof(IGAME_SIG) - 1);
		g_pGame = *(IGame ***)(addr + 2);
	}
	//if (g_pVideoMode)
	//{
	//	DWORD *pVFTable = *(DWORD **)&g_VideoMode_Hook;

	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 0, (void *)pVFTable[0], (void *&)m_pfnVideoMode_GetName);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 1, (void *)pVFTable[1], (void *&)m_pfnVideoMode_Init);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 2, (void *)pVFTable[2], (void *&)m_pfnVideoMode_Shutdown);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 3, (void *)pVFTable[3], (void *&)m_pfnVideoMode_AddMode);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 4, (void *)pVFTable[4], (void *&)m_pfnVideoMode_GetCurrentMode);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 5, (void *)pVFTable[5], (void *&)m_pfnVideoMode_GetMode);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 6, (void *)pVFTable[6], (void *&)m_pfnVideoMode_GetModeCount);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 7, (void *)pVFTable[7], (void *&)m_pfnVideoMode_IsWindowedMode);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 8, (void *)pVFTable[8], (void *&)m_pfnVideoMode_GetInitialized);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 9, (void *)pVFTable[9], (void *&)m_pfnVideoMode_SetInitialized);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 10, (void *)pVFTable[10], (void *&)m_pfnVideoMode_UpdateWindowPosition);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 11, (void *)pVFTable[11], (void *&)m_pfnVideoMode_FlipScreen);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 12, (void *)pVFTable[12], (void *&)m_pfnVideoMode_RestoreVideo);
	//	g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 13, (void *)pVFTable[13], (void *&)m_pfnVideoMode_ReleaseVideo);
	//}

	return videomode;
}