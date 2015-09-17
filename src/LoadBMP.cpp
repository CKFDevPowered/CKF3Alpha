#include <metahook.h>
#include "LoadBMP.h"
#include "plugins.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"
#include "ref_int.h"

int LoadPNG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadPNG(szFilename, buffer, bufferSize, width, height);
}

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadBMP(szFilename, buffer, bufferSize, width, height);
}