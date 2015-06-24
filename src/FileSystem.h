#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifndef NULL
#define NULL 0
#endif

#include "interface.h"
#include "IFileSystem.h"

extern IFileSystem *g_pFileSystem;
extern IFileSystem *g_pFullFileSystem;

void FileSystem_InstallHook(IFileSystem *pFileSystem);

#endif