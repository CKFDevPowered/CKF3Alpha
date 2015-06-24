#include <metahook.h>
#include "engfuncs.h"

#define MAXPRINTMSG 4096

void Con_Printf(char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	g_pEngfuncs->Con_Printf(msg);
}

void Con_DPrintf(char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	g_pEngfuncs->Con_DPrintf(msg);
}

void Con_NPrintf(int idx, char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	g_pEngfuncs->Con_NPrintf(idx, msg);
}

void Con_NXPrintf(struct con_nprint_s *info, char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	g_pEngfuncs->Con_NXPrintf(info, msg);
}