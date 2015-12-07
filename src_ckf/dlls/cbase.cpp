#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "client.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "MemPool.h"
#include "enginedef.h"

sv_funcs_t gSVFuncs;

model_t **sv_models;
model_t **sv_worldmodel;

DWORD g_dwEngineBase = 0;
DWORD g_dwEngineSize = 0;
DWORD g_dwEngineBuildnum = 0;

int ShouldCollide(edict_t *pentTouched, edict_t *pentOther);

#define SIG_NOT_FOUND(name) Sys_ErrorEx("无法定位: %s\n引擎版本：%d", name, g_dwEngineBuildnum);

void InitExceptionFilter(void);
void InitCrashHandler(void);
void InitPrefCounter(void);

void EntvarsKeyvalue(entvars_t *pev, KeyValueData *pkvd);

extern "C" void PM_Move(struct playermove_s *ppmove, int server);
extern "C" void PM_Init(struct playermove_s *ppmove);
extern "C" char PM_FindTextureType(char *name);

Vector VecBModelOrigin(entvars_t *pevBModel);
extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL int g_iSkillLevel;

//cs16nd start

CUtlVector<hash_item_t> stringsHashTable;
CMemoryPool hashItemMemPool(sizeof(hash_item_t), 64);

int CaseInsensitiveHash(const char *string, int iBounds)
{
	unsigned int hash = 0;

	if (!*string)
		return 0;

	while (*string)
	{
		if (*string < 'A' || *string > 'Z')
			hash = *string + 2 * hash;
		else
			hash = *string + 2 * hash + ' ';

		string++;
	}

	return (hash % iBounds);
}

void EmptyEntityHashTable(void)
{
	int i;
	hash_item_t *item;
	hash_item_t *temp;
	hash_item_t *free;

	for (i = 0; i < stringsHashTable.Count(); i++)
	{
		item = &stringsHashTable[i];
		temp = item->next;
		item->pev = NULL;
		item->pevIndex = 0;
		item->lastHash = NULL;
		item->next = NULL;

		while (temp)
		{
			free = temp;
			temp = temp->next;
			hashItemMemPool.Free(free);
		}
	}
}

void AddEntityHashValue(struct entvars_s *pev, const char *value, hash_types_e fieldType)
{
	int count;
	hash_item_t *item;
	hash_item_t *next;
	hash_item_t *temp;
	hash_item_t *newp;
	unsigned int hash = 0;
	int pevIndex;
	entvars_t *pevtemp;

	if (fieldType == CLASSNAME)
	{
		if (!FStringNull(pev->classname))
		{
			count = stringsHashTable.Count();
			hash = CaseInsensitiveHash(value, count);
			pevIndex = ENTINDEX(ENT(pev));
			item = &stringsHashTable[hash];

			while (item->pev)
			{
				if (!strcmp(STRING(item->pev->classname), STRING(pev->classname)))
					break;

				hash = (hash + 1) % count;
				item = &stringsHashTable[hash];
			}

			if (item->pev)
			{
				next = item->next;

				while (next)
				{
					if (item->pev == pev)
						break;

					if (item->pevIndex >= pevIndex)
						break;

					item = next;
					next = next->next;
				}

				if (pevIndex < item->pevIndex)
				{
					pevtemp = item->pev;
					item->pev = pev;
					item->lastHash = NULL;
					item->pevIndex = pevIndex;
					pevIndex = ENTINDEX(ENT(pevtemp));
				}
				else
					pevtemp = pev;

				if (item->pev != pevtemp)
				{
					temp = item->next;
					newp = (hash_item_t *)hashItemMemPool.Alloc(sizeof(hash_item_t));
					item->next = newp;
					newp->pev = pevtemp;
					newp->lastHash = NULL;
					newp->pevIndex = pevIndex;

					if (temp)
						newp->next = temp;
					else
						newp->next = NULL;
				}
			}
			else
			{
				item->pev = pev;
				item->lastHash = NULL;
				item->pevIndex = ENTINDEX(ENT(pev));
			}
		}
	}
}

void RemoveEntityHashValue(struct entvars_s *pev, const char *value, hash_types_e fieldType)
{
	int hash = 0;
	hash_item_t *item;
	hash_item_t *last;
	int pevIndex;
	int count;

	count = stringsHashTable.Count();
	hash = CaseInsensitiveHash(value, count);
	pevIndex = ENTINDEX(ENT(pev));

	if (fieldType == CLASSNAME)
	{
		hash = hash % count;
		item = &stringsHashTable[hash];

		while (item->pev)
		{
			if (!strcmp(STRING(item->pev->classname), STRING(pev->classname)))
				break;

			hash = (hash + 1) % count;
			item = &stringsHashTable[hash];
		}

		if (item->pev)
		{
			last = item;

			while (item->next)
			{
				if (item->pev == pev)
					break;

				last = item;
				item = item->next;
			}

			if (item->pev == pev)
			{
				if (last == item)
				{
					if (item->next)
					{
						item->pev = item->next->pev;
						item->pevIndex = item->next->pevIndex;
						item->lastHash = NULL;
						item->next = item->next->next;
					}
					else
					{
						item->pev = NULL;
						item->lastHash = NULL;
						item->pevIndex = 0;
					}
				}
				else
				{
					if (stringsHashTable[hash].lastHash == item)
						stringsHashTable[hash].lastHash = NULL;

					last->next = item->next;
					hashItemMemPool.Free(item);
				}
			}
		}
	}
}

edict_t *CREATE_NAMED_ENTITY(int iClass)
{
	edict_t *named = g_engfuncs.pfnCreateNamedEntity(iClass);

	if (named)
		AddEntityHashValue(&named->v, STRING(iClass), CLASSNAME);

	return named;
}

void REMOVE_ENTITY(edict_t *e)
{
	if (e)
		g_engfuncs.pfnRemoveEntity(e);
}

//cs16nd finished

static DLL_FUNCTIONS gFunctionTable =
{
	GameDLLInit,
	DispatchSpawn,
	DispatchThink,
	DispatchUse,
	DispatchTouch,
	DispatchBlocked,
	DispatchKeyValue,
	DispatchSave,
	DispatchRestore,
	DispatchObjectCollsionBox,
	SaveWriteFields,
	SaveReadFields,
	SaveGlobalState,
	RestoreGlobalState,
	ResetGlobalState,
	ClientConnect,
	ClientDisconnect,
	ClientKill,
	ClientPutInServer,
	ClientCommand,
	ClientUserInfoChanged,
	ServerActivate,
	ServerDeactivate,
	PlayerPreThink,
	PlayerPostThink,
	StartFrame,
	ParmsNewLevel,
	ParmsChangeLevel,
	GetGameDescription,
	PlayerCustomization,
	SpectatorConnect,
	SpectatorDisconnect,
	SpectatorThink,
	Sys_Error,
	PM_Move,
	PM_Init,
	PM_FindTextureType,
	SetupVisibility,
	UpdateClientData,
	AddToFullPack,
	CreateBaseline,
	RegisterEncoders,
	GetWeaponData,
	CmdStart,
	CmdEnd,
	ConnectionlessPacket,
	GetHullBounds,
	CreateInstancedBaselines,
	InconsistentFile,
	AllowLagCompensation
};

int GetEntityAPI(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion)
{
	if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
		return 0;

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));

	//cs16nd added
	stringsHashTable.SetSize(2048);

	for (int i = 0; i < stringsHashTable.Count(); i++)
		stringsHashTable[i].next = NULL;

	EmptyEntityHashTable();
	//cs16nd added

	return 1;
}

//cs16nd added
void OnFreeEntPrivateData(edict_t *pEnt)
{
	CBaseEntity *pEntity = CBaseEntity::Instance(pEnt);

	if (!pEntity)
		return;

	pEntity->UpdateOnRemove();
	RemoveEntityHashValue(pEntity->pev, STRING(pEntity->pev->classname), CLASSNAME);
}
//cs16nd added

static NEW_DLL_FUNCTIONS gNewDLLFunctions = 
{
	OnFreeEntPrivateData,
	NULL,
	ShouldCollide
};

//error when sig not found
void Sys_ErrorEx(const char *fmt, ...)
{
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	MessageBox(NULL, msg, "错误", MB_ICONERROR);
	exit(0);
}

void InstallHook(void)
{
	DWORD addr;
	HMODULE hEngine;
	
	DWORD dwDataSize = 0x02FFFFFF-0x01D00000;

	hEngine = GetModuleHandle("swds.dll");
	if(!hEngine || hEngine == INVALID_HANDLE_VALUE)
	{
		hEngine = GetModuleHandle("hw.dll");
	}
	if(!hEngine || hEngine == INVALID_HANDLE_VALUE)
	{
		g_dwEngineBase = 0x1D01000;
		g_dwEngineSize = 0x1000000;
	}
	else
	{
		g_dwEngineBase = MH_GetModuleBase(hEngine);
		g_dwEngineSize = MH_GetModuleSize(hEngine);
	}

#define BUILD_NUMBER_SIG "\xA1\x2A\x2A\x2A\x2A\x83\xEC\x08\x2A\x33\x2A\x85\xC0"
#define BUILD_NUMBER_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\x53\x33\xDB\x8B\x04\x9D"

	addr = MH_SIGFind(g_dwEngineBase, g_dwEngineSize, "Protocol version %i\nExe version %s", sizeof("Protocol version %i\nExe version %s")-1);
	if(!addr)
		SIG_NOT_FOUND("buildnum");

	byte sig_buildnum[] = "\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x50";
	*(DWORD *)(sig_buildnum+1) = addr;

	addr = MH_SIGFind(g_dwEngineBase, g_dwEngineSize, (char *)sig_buildnum, sizeof(sig_buildnum)-1);
	if(!addr)
		SIG_NOT_FOUND("buildnum");

	gSVFuncs.buildnum = (int (*)(void))GetCallAddress(addr + 5);
	g_dwEngineBuildnum = gSVFuncs.buildnum();

	//Here is a bug in engine so we can not only just hook FM_ShouldCollide but need to fix some bug in engine:
	/*

	void SV_ClipToLinks(areanode_t *node, moveclip_t *clip)
	{
		....
		for (l = node->solid_edicts.next; l != &node->solid_edicts; l = next)
		{
		...
			if (gNewDLLFunctions.pfnShouldCollide)
			{
				if (gNewDLLFunctions.pfnShouldCollide(touch, clip->passedict) == 0)
					return;  <- this "return" caused some phys problems, and this "return" should be "continue".
			}
		...

	}

	*/

	//SV_ClipToLinks
	//We need to find "Trigger in clipping list" first;
	addr = MH_SIGFind(g_dwEngineBase, dwDataSize, "Trigger in clipping list", sizeof("Trigger in clipping list")-1);
	if(!addr)
		SIG_NOT_FOUND("\"Trigger in clipping list\"");

	//Setup the signature so we can find Sys_Error("Trigger in clipping list");
	//2A means indeterminate bytes
	//  68 2A 2A 2A 2A	push    offset aTriggerInClipp ; "Trigger in clipping list"
	//  E8 2A 2A 2A 2A	call    Sys_Error
	//  83 C4 04		add     esp, 4
	byte Sys_Error_SIG[14] = "\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04";

	*(DWORD *)(Sys_Error_SIG+1) = addr;

	//Let's go find
	addr = MH_SIGFind(g_dwEngineBase, g_dwEngineSize, (char *)Sys_Error_SIG, 13);
	if(!addr)
		SIG_NOT_FOUND("SV_ClipToLinks->Sys_Error(\"Trigger in clipping list\")");

	//Here we found Sys_Error("Trigger in clipping list");

	//FF D0					call    eax ; gNewDLLFunctions.pfnShouldCollide
	//83 C4 08				add     esp, 8
	//85 C0					test    eax, eax
	//0F 84 2A 2A 2A 2A		jz      loc_XXXXXX

#define SV_CLIPTOLINKS_SHOULDCOLLIDE_SIG "\xFF\xD0\x83\xC4\x08\x85\xC0\x0F\x84"
	addr = MH_SIGFind(addr, 0x200, SV_CLIPTOLINKS_SHOULDCOLLIDE_SIG, sizeof(SV_CLIPTOLINKS_SHOULDCOLLIDE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("SV_ClipToLinks->pfnShouldCollide");

	//Save the address of "0F 84 2A 2A 2A 2A jz loc_XXXXXX"
	//it will be modified by us later.

	DWORD addr_shouldcollide_jz = addr + 7;

	//Now we find a "continue" in this for() loop

	//D8 1D 2A 2A 2A 2A                                               fcomp   flt:xxxxxx (0.0)
	//DF E0                                                           fnstsw  ax
	//F6 C4 44                                                        test    ah, 44h
	//0F 8B 2A 2A 2A 2A                                               jnp     loc_xxxxxx

	//D8 9D 2A 2A 2A 2A                                               fcomp   dword ptr[ebp+n]
	//DF E0                                                           fnstsw  ax
	//F6 C4 41                                                        test    ah, 41h
	//0F 84 2A 2A 2A 2A                                               jz      loc_xxxxxx

#define SV_CLIPTOLINKS_CONTINUE_SIG "\xD8\x2A\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x2A\x0F"
	addr = MH_SIGFind(addr, 0x200, SV_CLIPTOLINKS_CONTINUE_SIG, sizeof(SV_CLIPTOLINKS_CONTINUE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("SV_ClipToLinks->continue");

	//This jnp loc_xxxxxx is a conditional jump code and we need to trace to it's destination address,
	//jnp's operand is an offset from it's next opcode's address,
	//which means opcode ".text:04000000 0F 8B 02 00 00 00 jnp loc_xxxxxx" jumps to 0x04000008 (0x04000000 + 6 + 0x02)
	
	//Now we calcluate the destination address

	//Move to "0F 8B 2A 2A 2A 2A jnp loc_xxxxxx" first
	DWORD addr_jnp = addr + 11;

	//Add the offset to the jnp's address, just like "0x04000000 + 6 + 0x02", this is the destionation address we need to jump to.
	DWORD addr_destloc = addr_jnp + (*(DWORD *)(addr_jnp + 2)) + 6;

	//And what is the offset? (jz_address + 6 + offset = jump_destination_address)
	//it's very easy to know that : offset = jump_destination_address - jz_address - 6;
	//just basic math problem.
	DWORD jz_offset = addr_destloc - addr_shouldcollide_jz - 6;

	//Write this offset back to "0F 84 2A 2A 2A 2A jz loc_XXXXXX"
	MH_WriteMemory((void *)(addr_shouldcollide_jz + 2), (BYTE *)&jz_offset, 4);

	//SV_HullPointContents

	addr = (DWORD)g_engfuncs.pfnPointContents;

	if(*(byte *)addr == 0xE9)
	{
		addr = GetCallAddress(addr);
	}
	else
	{
		#define SV_POINTCONTENTS_SIG "\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04"
		addr = MH_SIGFind(addr, 0x10, SV_POINTCONTENTS_SIG, sizeof(SV_POINTCONTENTS_SIG)-1);
		if(!addr)
			SIG_NOT_FOUND("PF_pointcontents_I->SV_PointContents");

		addr = GetCallAddress(addr);
	}

#define SV_HULLPOINTCONTENTS_SIG "\xE8\x2A\x2A\x2A\x2A\x8B\xF0"	
	addr = MH_SIGFind(addr, 0x100, SV_HULLPOINTCONTENTS_SIG, sizeof(SV_HULLPOINTCONTENTS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("SV_PointContents->SV_HullPointContents");

	gSVFuncs.SV_HullPointContents = (int (*)(hull_t *, int , const vec_t *))GetCallAddress(addr);

	//AddToFatPVS / PAS

#define SV_ADDTOFATPVS_SIG "\xE8\x2A\x2A\x2A\x2A\x83\xC4\x14"
	addr = MH_SIGFind((DWORD)g_engfuncs.pfnSetFatPVS, 0x50, SV_ADDTOFATPVS_SIG, sizeof(SV_ADDTOFATPVS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("SV_AddToFatPVS");

	gSVFuncs.SV_AddToFatPVS = (void (*)(float *, mnode_t *))GetCallAddress(addr);

#define SV_WORLDMODEL_SIG "\xE8\x2A\x2A\x2A\x2A\x8B\x0D"
	addr = MH_SIGFind((DWORD)g_engfuncs.pfnSetFatPVS, 0x50, SV_WORLDMODEL_SIG, sizeof(SV_WORLDMODEL_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("sv_worldmodel");

	sv_worldmodel = *(model_t ***)(addr + 7);

	addr = MH_SIGFind((DWORD)g_engfuncs.pfnSetFatPAS, 0x50, SV_ADDTOFATPVS_SIG, sizeof(SV_ADDTOFATPVS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("SV_AddToFatPAS");

	gSVFuncs.SV_AddToFatPAS = (void (*)(float *, mnode_t *))GetCallAddress(addr);

#define SV_MODELS_SIG "\x8B\x04\x85"
	addr = MH_SIGFind((DWORD)g_engfuncs.pfnModelFrames, 0x50, SV_MODELS_SIG, sizeof(SV_MODELS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("sv_models");
	sv_models = *(model_t ***)(addr + 3);

}

int GetNewDLLFunctions(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
	{
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return 0;
	}

	pFunctionTable->pfnOnFreeEntPrivateData = gNewDLLFunctions.pfnOnFreeEntPrivateData;
	pFunctionTable->pfnGameShutdown = gNewDLLFunctions.pfnGameShutdown;
	pFunctionTable->pfnShouldCollide = gNewDLLFunctions.pfnShouldCollide;

	//InitPrefCounter();
	//InitExceptionFilter();
	//InitCrashHandler();
	InstallHook();

	return 1;
}

int DispatchSpawn(edict_t *pent)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity)
	{
		pEntity->pev->absmin = pEntity->pev->origin - Vector(1, 1, 1);
		pEntity->pev->absmax = pEntity->pev->origin + Vector(1, 1, 1);
		pEntity->Spawn();
		pEntity = (CBaseEntity *)GET_PRIVATE(pent);

		if (pEntity)
		{
			if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
				return -1;

			if (pEntity->pev->flags & FL_KILLME)
				return -1;

			if (pEntity->pev->globalname)
			{
				const globalentity_t *pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);

				if (pGlobal)
				{
					if (pGlobal->state == GLOBAL_DEAD)
						return -1;

					if (strcmp(STRING(gpGlobals->mapname), pGlobal->levelName))
						pEntity->MakeDormant();
				}
				else
					gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
			}
		}
	}
	return 0;
}

void DispatchKeyValue(edict_t *pentKeyvalue, KeyValueData *pkvd)
{
	if (!pkvd || !pentKeyvalue)
		return;

	EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

	if (pkvd->fHandled || !pkvd->szClassName)
		return;

	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentKeyvalue);

	if (pEntity)
		pEntity->KeyValue(pkvd);
}

BOOL gTouchDisabled = FALSE;

void DispatchTouch(edict_t *pentTouched, edict_t *pentOther)
{
	if (gTouchDisabled)
		return;

	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentTouched);
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE(pentOther);

	if (pEntity && pOther && !((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME))
		pEntity->Touch(pOther);
}

void DispatchUse(edict_t *pentUsed, edict_t *pentOther)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentUsed);
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE(pentOther);

	if (pEntity && !(pEntity->pev->flags & FL_KILLME))
		pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

void DispatchThink(edict_t *pent)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity)
	{
		if (pEntity->pev->flags & FL_DORMANT)
			ALERT(at_console, "Dormant entity %s is trying to think (ignoring think)!!\n", STRING(pEntity->pev->classname));
		else
			pEntity->Think();
	}
}

void DispatchBlocked(edict_t *pentBlocked, edict_t *pentOther)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentBlocked);
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE(pentOther);

	if (pEntity)
		pEntity->Blocked(pOther);
}

void DispatchSave(edict_t *pent, SAVERESTOREDATA *pSaveData)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity && pSaveData)
	{
		ENTITYTABLE *pTable = &pSaveData->pTable[pSaveData->currentIndex];

		if (pTable->pent != pent)
			ALERT(at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n");

		if (pEntity->ObjectCaps() & FCAP_DONT_SAVE)
			return;

		if (pEntity->pev->movetype == MOVETYPE_PUSH)
		{
			float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
			pEntity->pev->ltime = gpGlobals->time;
			pEntity->pev->nextthink = pEntity->pev->ltime + delta;
		}

		pTable->location = pSaveData->size;
		pTable->classname = pEntity->pev->classname;

		CSave saveHelper(pSaveData);
		pEntity->Save(saveHelper);
		pTable->size = pSaveData->size - pTable->location;
	}
}

CBaseEntity *FindGlobalEntity(string_t classname, string_t globalname)
{
	edict_t *pent = FIND_ENTITY_BY_STRING(NULL, "globalname", STRING(globalname));
	CBaseEntity *pReturn = CBaseEntity::Instance(pent);

	if (!pReturn)
		return NULL;

	if (strcmp(STRING(pReturn->pev->classname), STRING(classname)))
	{
		ALERT(at_console, "Global entity found %s, wrong class %s\n", STRING(globalname), STRING(pReturn->pev->classname));
		return NULL;
	}

	return pReturn;
}

int DispatchRestore(edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity && pSaveData)
	{
		Vector oldOffset;
		CRestore restoreHelper(pSaveData);

		if (globalEntity)
		{
			entvars_t tmpVars;
			CRestore tmpRestore(pSaveData);
			tmpRestore.PrecacheMode(0);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

			if (strcmp(pSaveData->szCurrentMapName, pGlobal->levelName))
				return 0;

			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity *pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);

			if (!pNewEntity)
				return 0;

			restoreHelper.SetGlobalMode(1);
			pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
			pEntity = pNewEntity;
			pent = ENT(pEntity->pev);
			gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
		}

		if (pEntity->ObjectCaps() & FCAP_MUST_SPAWN)
		{
			pEntity->Restore(restoreHelper);
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore(restoreHelper);
			pEntity->Precache();
		}

		pEntity = (CBaseEntity *)GET_PRIVATE(pent);

		if (globalEntity)
		{
			pSaveData->vecLandmarkOffset = oldOffset;

			if (pEntity)
			{
				UTIL_SetOrigin(pEntity->pev, pEntity->pev->origin);
				pEntity->OverrideReset();
			}
		}
		else if (pEntity && pEntity->pev->globalname)
		{
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);

			if (pGlobal)
			{
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;

				if (strcmp(STRING(gpGlobals->mapname), pGlobal->levelName))
					pEntity->MakeDormant();
			}
			else
			{
				ALERT(at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname));
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
			}
		}
	}

	return 0;
}

void SetObjectCollisionBox(entvars_t *pev);

void DispatchObjectCollsionBox(edict_t *pent)
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity)
		pEntity->SetObjectCollisionBox();
	else
		SetObjectCollisionBox(&pent->v);
}

void SaveWriteFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
	CSave saveHelper(pSaveData);
	saveHelper.WriteFields(pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount)
{
	CRestore restoreHelper(pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

edict_t *EHANDLE::Get(void)
{
	if (!m_pent)
		return NULL;

	if (m_pent->serialnumber != m_serialnumber)
		return NULL;

	return m_pent;
}

edict_t *EHANDLE::Set(edict_t *pent)
{
	m_pent = pent;

	if (pent)
		m_serialnumber = m_pent->serialnumber;

	return pent;
}

EHANDLE::operator CBaseEntity *(void)
{
	return (CBaseEntity *)GET_PRIVATE(Get());
}

CBaseEntity *EHANDLE::operator = (CBaseEntity *pEntity)
{
	if (pEntity)
	{
		m_pent = ENT(pEntity->pev);

		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = NULL;
		m_serialnumber = 0;
	}

	return pEntity;
}

EHANDLE::operator int(void)
{
	return Get() != NULL;
}

CBaseEntity *EHANDLE::operator ->(void)
{
	return (CBaseEntity *)GET_PRIVATE(Get());
}

void CBaseEntity::Activate( void )
{
	PostSpawn();
}

void CBaseEntity::PostSpawn(void)//hz added
{
	if(m_iFollowEntity)
	{
		CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iFollowEntity));

		if (pTarget)
		{
			pev->aiment = pTarget->edict();
			pev->movetype = MOVETYPE_FOLLOW;
		}
	}
}

int CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	if (pev->health >= pev->max_health)
		return 0;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return 1;
}

int CBaseEntity::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (!pev->takedamage)
		return 0;

	g_vecAttackDir = (pevInflictor->origin - VecBModelOrigin(pev)).Normalize();

	if (!FNullEnt(pevInflictor) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && pevAttacker->solid != SOLID_TRIGGER)
	{
		Vector vecDir = (pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5).Normalize();
		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000)
			flForce = 1000;

		pev->velocity = pev->velocity + vecDir * flForce;
	}

	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);
		return 0;
	}

	return 1;
}

void CBaseEntity::Killed(entvars_t *pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

CBaseEntity *CBaseEntity::GetNextTarget(void)
{
	if (FStringNull(pev->target))
		return NULL;

	edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));

	if (FNullEnt(pTarget))
		return NULL;

	return Instance(pTarget);
}

TYPEDESCRIPTION CBaseEntity::m_SaveData[] =
{
	DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),
	DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION)
};

int CBaseEntity::Save(CSave &save)
{
	if (save.WriteEntVars("ENTVARS", pev))
		return save.WriteFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	return 0;
}

int CBaseEntity::Restore(CRestore &restore)
{
	int status = restore.ReadEntVars("ENTVARS", pev);

	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
		Vector mins = pev->mins;
		Vector maxs = pev->maxs;

		PRECACHE_MODEL((char *)STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs);
	}

	return status;
}

void SetObjectCollisionBox(entvars_t *pev)
{
	if (pev->solid == SOLID_BSP && (pev->angles.x || pev->angles.y || pev->angles.z))
	{
		float v, max = 0;
		int i;

		for (i = 0; i < 3; i++)
		{
			v = fabs(((float *)pev->mins)[i]);

			if (v > max)
				max = v;

			v = fabs(((float *)pev->maxs)[i]);

			if (v > max)
				max = v;
		}

		for (i = 0; i < 3; i++)
		{
			((float *)pev->absmin)[i] = ((float *)pev->origin)[i] - max;
			((float *)pev->absmax)[i] = ((float *)pev->origin)[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}

void CBaseEntity::SetObjectCollisionBox(void)
{
	::SetObjectCollisionBox(pev);
}

int CBaseEntity::Intersects(CBaseEntity *pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x || pOther->pev->absmin.y > pev->absmax.y || pOther->pev->absmin.z > pev->absmax.z || pOther->pev->absmax.x < pev->absmin.x || pOther->pev->absmax.y < pev->absmin.y || pOther->pev->absmax.z < pev->absmin.z)
		return 0;

	return 1;
}

void CBaseEntity::MakeDormant(void)
{
	pev->flags |= FL_DORMANT;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
	pev->nextthink = 0;
	UTIL_SetOrigin(pev, pev->origin);
}

int CBaseEntity::IsDormant(void)
{
	return pev->flags & FL_DORMANT;
}

BOOL CBaseEntity::IsInWorld(void)
{
	if (pev->origin.x >= 4096 || pev->origin.y >= 4096 || pev->origin.z >= 4096)
		return FALSE;

	if (pev->origin.x <= -4096 || pev->origin.y <= -4096 || pev->origin.z <= -4096)
		return FALSE;

	if (pev->velocity.x >= 12000 || pev->velocity.y >= 12000 || pev->velocity.z >= 12000)
		return FALSE;

	if (pev->velocity.x <= -12000 || pev->velocity.y <= -12000 || pev->velocity.z <= -12000)
		return FALSE;

	//if (pev->velocity.x >= 2000 || pev->velocity.y >= 2000 || pev->velocity.z >= 2000)
	//	return FALSE;

	//if (pev->velocity.x <= -2000 || pev->velocity.y <= -2000 || pev->velocity.z <= -2000)
	//	return FALSE;

	return TRUE;
}

int CBaseEntity::ShouldToggle(USE_TYPE useType, BOOL currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return 0;
	}

	return 1;
}

int CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (pev->rendermode == kRenderTransAlpha)
		return -1;

	if (pev->rendermode != kRenderNormal)
		return DECAL_BPROOF1;

	return RANDOM_LONG(DECAL_GUNSHOT4, DECAL_GUNSHOT5);
}

CBaseEntity *CBaseEntity::Create(char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner)
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return NULL;
	}

	CBaseEntity *pEntity = Instance(pent);
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;
	DispatchSpawn(pEntity->edict());
	return pEntity;
}