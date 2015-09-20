#include <metahook.h>
#include "LoadBMP.h"
#include "plugins.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"
#include "ref_int.h"

int LoadDDS(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadDDS(szFilename, buffer, bufferSize, width, height);
}

int LoadPNG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadPNG(szFilename, buffer, bufferSize, width, height);
}

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadBMP(szFilename, buffer, bufferSize, width, height);
}

int LoadJPEG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadJPEG(szFilename, buffer, bufferSize, width, height);
}

void GL_UploadDXT(byte *data, int width, int height, qboolean mipmap, qboolean ansio)
{
	return gRefExports.GL_UploadDXT(data, width, height, mipmap, ansio);
}