#include <metahook.h>
#include <math.h>
#include "engfuncs.h"
#include "cvar.h"

static void (*g_pfnCvar_RegisterVariable)(cvar_t *variable) = NULL;

cvar_t *Cvar_FindVar(const char *var_name)
{
	cvar_t *var;

	for (var = g_pEngfuncs->GetFirstCvarPtr(); var; var = var->next)
	{
		if (!strcmp(var_name, var->name))
			return var;
	}

	return NULL;
}

cvar_t *Cvar_FindPrevVar(char *var_name)
{
	cvar_t *var;

	for (var = g_pEngfuncs->GetFirstCvarPtr(); var->next; var = var->next)
	{
		if (!strcmp(var_name, var->next->name))
			return var;
	}

	return NULL;
}

void Cvar_Set(char *var_name, char *value)
{
	gEngfuncs.Cvar_Set(var_name, value);
}

void Cvar_SetValue(char *var_name, float value)
{
	char val[32];

	if (fabs(value - (int)value) < 0.000001)
		_snprintf(val, sizeof(val), "%d", (int)value);
	else
		_snprintf(val, sizeof(val), "%f", value);

	Cvar_Set(var_name, val);
}

cvar_t *Cvar_RegisterVariable(char *szName, char *szValue, int flags)
{
	return g_pEngfuncs->pfnRegisterVariable(szName, szValue, flags);
}

void Cvar_RegisterVariable(cvar_t *variable)
{
	g_pfnCvar_RegisterVariable(variable);
}

void Cvar_Init(void)
{
	DWORD dwBaseAddress = (DWORD)g_pEngfuncs->pfnRegisterVariable + 0x30;
	DWORD dwFunctionAddress = (DWORD)g_pMetaHookAPI->SearchPattern((void *)dwBaseAddress, 0x20, "\x89\x46\x08\xE8", 4);

	if (dwFunctionAddress)
	{
		dwFunctionAddress += 4;
		g_pfnCvar_RegisterVariable = (void (*)(cvar_t *))((dwFunctionAddress + *(DWORD *)dwFunctionAddress) + 0x4);
	}
}