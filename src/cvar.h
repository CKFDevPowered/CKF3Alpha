#ifndef CVAR_H
#define CVAR_H
#pragma once

#include "cvardef.h"

extern cvar_t *cvar_vars;

void Cvar_Init(void);
void Cvar_Shutdown(void);
cvar_t *Cvar_FindVar(const char *var_name);
cvar_t *Cvar_FindPrevVar(char *var_name);
float Cvar_VariableValue(char *var_name);
char *Cvar_VariableString(char *var_name);
char *Cvar_CompleteVariable(char *partial, qboolean next);
void Cvar_DirectSet(cvar_t *var, char *value);
void Cvar_Set(char *var_name, char *value);
void Cvar_SetValue(char *var_name, float value);
cvar_t *Cvar_RegisterVariable(char *szName, char *szValue, int flags);
void Cvar_RegisterVariable(cvar_t *variable);
void Cvar_RemoveHudCvars(void);
const char *Cvar_IsMultipleTokens(const char *varname);
qboolean Cvar_Command(void);
int Cvar_CountServerVariables(void);
void Cvar_UnlinkExternals(void);
void Cvar_CmdInit(void);

#endif