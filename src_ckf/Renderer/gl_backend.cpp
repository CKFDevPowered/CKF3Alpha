#include "gl_local.h"
#include "screen.h"

byte *scrcapture_buffer = NULL;
int scrcapture_bufsize = 0;

void R_CaptureScreen(void)
{
	if(scrcapture_bufsize < glwidth*glheight*3)
	{
		if(scrcapture_buffer)
			delete []scrcapture_buffer;
		scrcapture_bufsize = glwidth*glheight*3;
		scrcapture_buffer = new byte[scrcapture_bufsize];
	}

	qglReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, scrcapture_buffer);
}

void CL_ScreenShot_f(void)
{
	char *szExt = "png";
	int iFileIndex = 0;
	char *pLevelName;
	char szLevelName[64];
	char szFileName[260];	
	typeSaveImage pfnSaveImage;

	R_CaptureScreen();

	if(gEngfuncs.Cmd_Argc() > 1)
		szExt = gEngfuncs.Cmd_Argv(1);

	const char *pLevel = gEngfuncs.pfnGetLevelName();
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
		pfnSaveImage = (typeSaveImage)GL_SavePNG;
	}
	else if(!stricmp(szExt, "tga"))
	{
		pfnSaveImage = (typeSaveImage)GL_SaveTGA;
	}
	else if(!stricmp(szExt, "bmp"))
	{
		pfnSaveImage = (typeSaveImage)GL_SaveBMP;
	}
	else
	{
		gEngfuncs.Con_Printf("Unsupported image format: \".%s\"\n", szExt);
		return;
	}

	do
	{
		sprintf(szFileName, "%s%.4d.%s", szLevelName, iFileIndex, szExt);
		++iFileIndex;
	}while(g_pFileSystem->FileExists(szFileName) == true);

	if(pfnSaveImage(szFileName, glwidth, glheight, scrcapture_buffer))
	{
		gEngfuncs.Con_Printf("Screenshot %s saved.\n", szFileName);
	}
}

byte *R_GetSCRCaptureBuffer(int *bufsize)
{
	if(bufsize)
		*bufsize = scrcapture_bufsize;
	return scrcapture_buffer;
}