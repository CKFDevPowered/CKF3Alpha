inline void Cvar_DirectSet(cvar_t *var, char *value)
{
	g_pMetaSave->pEngineFuncs->Cvar_Set(var->name, value);
}