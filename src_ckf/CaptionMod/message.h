#pragma once

typedef int (*pfnMF_DrawFX)(void);

#define READ_ENTITY() int entindex = READ_SHORT();\
	cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex(entindex);\
	if(!pEntity)\
		return 1;

#define READ_TARGETENT() int targetent = READ_SHORT();\
	cl_entity_t *pTarget = gEngfuncs.GetEntityByIndex(targetent);\
	if(!pTarget)\
		return 1;
