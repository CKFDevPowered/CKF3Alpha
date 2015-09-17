#include <metahook.h>

float scr_fov_value;
int glwidth, glheight;

void SCR_Init(void)
{
	SCREENINFO scrinfo;
	scrinfo.iSize = sizeof(scrinfo);
	g_pMetaSave->pEngineFuncs->pfnGetScreenInfo(&scrinfo);

	glwidth = scrinfo.iWidth;
	glheight = scrinfo.iHeight;
}

void SCR_VidInit(void)
{
}