void Sys_GetRegKeyValueUnderRoot(const char *pszSubKey, const char *pszElement, char *pszReturnString, int nReturnLength, const char *pszDefaultValue);
void Sys_SetRegKeyValueUnderRoot(const char *pszSubKey, const char *pszElement, const char *pszValue);
void Sys_SplitPath(const char *path, char *drive, char *dir, char *fname, char *ext);
void Sys_InitMemory(void);