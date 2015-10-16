#include <metahook.h>
#include "configs.h"
#include "LoadTGA.h"
#include "plugins.h"
#include "hooks.h"
#include "perf_counter.h"
#include "cvardef.h"
#include "developer.h"
#include "console.h"
#include "ref_int.h"

int LoadImageGeneric(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	return gRefExports.LoadImageGeneric(szFilename, buffer, bufferSize, width, height);
}