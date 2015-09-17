#include "gl_local.h"
#include "screen.h"

byte *scrcapture_buffer = NULL;
int scrcapture_bufsize = 0;

char scrcapture_ext[32];

void R_CaptureScreen(const char *szExt)
{
	int iFileIndex = 0;
	char *pLevelName;
	char szLevelName[64];
	char szFileName[260];
	typeSaveImage pfnSaveImage;

	if(scrcapture_ext[0] != '\0')
		szExt = scrcapture_ext;

	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		int current_readfbo;
		qglGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &current_readfbo);
		if(current_readfbo != s_BackBufferFBO.s_hBackBufferFBO)
		{
			R_GLBindFrameBuffer(GL_READ_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
			qglReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, scrcapture_buffer);
			R_GLBindFrameBuffer(GL_READ_FRAMEBUFFER, current_readfbo);
		}
		else
		{
			qglReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, scrcapture_buffer);
		}
	}
	else
	{
		qglReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, scrcapture_buffer);
	}

	const char *pLevel = g_pMetaSave->pEngineFuncs->pfnGetLevelName();
	if(!pLevel || !pLevel[0])
	{
		strcpy(szLevelName, "screenshot");
	}
	else
	{
		pLevelName = (char *)pLevel + strlen(pLevel) - 1;
		while( *pLevelName != '/' && *pLevelName != '\\')
			pLevelName --;
		strcpy(szLevelName, pLevelName+1);
		szLevelName[strlen(szLevelName)-4] = 0;
	}

	if(!stricmp(szExt, "png"))
	{
		pfnSaveImage = (typeSaveImage)SavePNG;
	}
	else if(!stricmp(szExt, "tga"))
	{
		pfnSaveImage = (typeSaveImage)SaveTGA;
	}
	else if(!stricmp(szExt, "bmp"))
	{
		pfnSaveImage = (typeSaveImage)SaveBMP;
	}
	else
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("Unsupported image format: \".%s\"\n", szExt);
		return;
	}

	do
	{
		sprintf(szFileName, "%s%.4d.%s", szLevelName, iFileIndex, szExt);
		++iFileIndex;
	}while(g_pFileSystem->FileExists(szFileName) == true);

	if(pfnSaveImage(szFileName, glwidth, glheight, scrcapture_buffer))
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("Screenshot %s saved.\n", szFileName);
	}
}

void CL_ScreenShot_f(void)
{
	if(scrcapture_bufsize < glwidth*glheight*3)
	{
		if(scrcapture_buffer)
			delete []scrcapture_buffer;
		scrcapture_bufsize = glwidth*glheight*3;
		scrcapture_buffer = new byte[scrcapture_bufsize];
	}

	char *szExt = "png";
	if(g_pMetaSave->pEngineFuncs->Cmd_Argc() > 1)
	{
		szExt = g_pMetaSave->pEngineFuncs->Cmd_Argv(1);
	}

	R_CaptureScreen(szExt);
}

byte *R_GetSCRCaptureBuffer(int *bufsize)
{
	if(bufsize)
		*bufsize = scrcapture_bufsize;
	return scrcapture_buffer;
}