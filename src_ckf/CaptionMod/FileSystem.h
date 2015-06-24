#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifndef NULL
#define NULL 0
#endif

#include "interface.h"
#include "IFileSystem.h"

#define IBaseFileSystem IFileSystem

#include <cvardef.h>
extern cvar_t *cl_hud_minmode;
#endif