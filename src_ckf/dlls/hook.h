#pragma once

#pragma pack(push, 1)

struct tagIATDATA
{
	void *pAPIInfoAddr;
};

struct tagCLASS
{
	DWORD *pVMT;
};

struct tagVTABLEDATA
{
	tagCLASS *pInstance;
	void *pVFTInfoAddr;
};

#pragma pack(pop)

typedef struct hook_s
{
	void *pOldFuncAddr;
	void *pNewFuncAddr;
	void *pClass;
	int iTableIndex;
	int iFuncIndex;
	HMODULE hModule;
	LPCWSTR  pszModuleName;
	LPCWSTR  pszFuncName;
	struct hook_s *pNext;
	void *pInfo;
}hook_t;

hook_t *MH_NewHook(void);
void MH_FreeHook(hook_t *pHook);
void MH_FreeAllHook(void);
BOOL MH_UnHook(hook_t *pHook);
hook_t *MH_InlineHook(void *pOldFuncAddr, void *pNewFuncAddr, void *&pCallBackFuncAddr);
hook_t *MH_VFTHook(void *pClass, int iTableIndex, int iFuncIndex, void *pNewFuncAddr, void *&pCallBackFuncAddr);
DWORD MH_WriteMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD MH_ReadMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD MH_GetModuleBase(HMODULE hModule);
DWORD MH_GetModuleSize(HMODULE hModule);
DWORD MH_SIGFind(DWORD dwStartAddr, DWORD dwFindLen, char *sig, int len);