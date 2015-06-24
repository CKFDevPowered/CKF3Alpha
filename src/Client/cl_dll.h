typedef unsigned char byte;
typedef unsigned short word;
typedef float vec_t;
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include "util_vector.h"

#ifndef EXPORT
#ifdef _WIN32
#define EXPORT _declspec(dllexport)
#else
#define EXPORT __attribute__ ((visibility("default")))
#endif
#endif

#include "wrect.h"
#include <cdll_int.h>
#include "cdll_dll.h"

extern cl_enginefunc_t gEngfuncs;