char *Info_ValueForKey(char *s, char *key);
void Info_RemoveKey(char *s, char *key);
void Info_RemovePrefixedKeys(char *start, char prefix);
qboolean Info_IsKeyImportant(char *key);
char *Info_FindLargestKey(char *s);
void Info_SetValueForStarKey(char *s, char *key, char *value, int maxsize);
void Info_Print(char *s);
void Info_SetValueForKey(char *s, char *key, char *value, int maxsize);