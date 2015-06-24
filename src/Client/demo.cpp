#include "hud.h"
#include "cl_util.h"
#include "demo.h"
#include "demo_api.h"
#include <memory.h>

float g_demozoom;

void Demo_ReadBuffer(int size, unsigned char *buffer)
{
	int type = *(int *)buffer;
	int i = sizeof(int);

	switch (type)
	{
		case TYPE_ZOOM:
		{
			g_demozoom = *(float *)&buffer[i];
			i += sizeof(float);
			break;
		}
	}

	gExportfuncs.Demo_ReadBuffer(size, buffer);
}