#include <windows.h>
#include "interface.h"
#include "memdll.h"

CreateInterfaceFn MEM_GetFactory(HINTERFACEMODULE hModule)
{
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

	if (pDOSHeader->e_magic == *(DWORD *)"ML")
		return (CreateInterfaceFn)MEM_GetProcAddress((HMODULE)hModule, CREATEINTERFACE_PROCNAME);

	return (CreateInterfaceFn)GetProcAddress((HMODULE)hModule, CREATEINTERFACE_PROCNAME);
}

void MEM_CopySection(PBYTE pSrc, PBYTE pDest)
{
	PIMAGE_DOS_HEADER pSrcDOSHeader = (PIMAGE_DOS_HEADER)pSrc;
	PIMAGE_NT_HEADERS pSrcNTHeader = (PIMAGE_NT_HEADERS)(pSrc + pSrcDOSHeader->e_lfanew);
	PIMAGE_DOS_HEADER pDstDOSHeader = (PIMAGE_DOS_HEADER)pDest;
	PIMAGE_NT_HEADERS pDstNTHeader = (PIMAGE_NT_HEADERS)(pDest + pDstDOSHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pDstNTHeader);

	for (DWORD i = 0; i < pDstNTHeader->FileHeader.NumberOfSections; i++, pSection++)
	{
		if (pSection->SizeOfRawData == 0)
		{
			DWORD dwSize = pSrcNTHeader->OptionalHeader.SectionAlignment;

			if (dwSize > 0)
			{
				pSection->Misc.PhysicalAddress = pSection->VirtualAddress + (DWORD)pDest;
				memset((PVOID)pSection->Misc.PhysicalAddress, 0, dwSize);
			}

			continue;
		}

		pSection->Misc.PhysicalAddress = pSection->VirtualAddress + (DWORD)pDest;
		memcpy((PVOID)pSection->Misc.PhysicalAddress, (PVOID)((DWORD)pSrc + pSection->PointerToRawData), pSection->SizeOfRawData);
	}
}

void MEM_LoadRelocation(PBYTE pSrc, PBYTE pDest)
{
	PIMAGE_DOS_HEADER pSrcDOSHeader = (PIMAGE_DOS_HEADER)pSrc;
	PIMAGE_NT_HEADERS pSrcNTHeader = (PIMAGE_NT_HEADERS)(pSrc + pSrcDOSHeader->e_lfanew);
	PIMAGE_DOS_HEADER pDstDOSHeader = (PIMAGE_DOS_HEADER)pDest;
	PIMAGE_NT_HEADERS pDstNTHeader = (PIMAGE_NT_HEADERS)(pDest + pDstDOSHeader->e_lfanew);
	PIMAGE_DATA_DIRECTORY pDirectory = (PIMAGE_DATA_DIRECTORY)&pDstNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	if (!pDirectory->VirtualAddress)
		return;

	PIMAGE_BASE_RELOCATION pRelocation = (PIMAGE_BASE_RELOCATION)(pDirectory->VirtualAddress + (DWORD)pDest);
	DWORD dwOriginAddress = pSrcNTHeader->OptionalHeader.ImageBase;
	DWORD dwBaseDelta = (DWORD)pDest - dwOriginAddress;

	while (pRelocation->VirtualAddress != 0)
	{
		DWORD dwSize = (pRelocation->SizeOfBlock - sizeof(IMAGE_DATA_DIRECTORY)) / 2;
		PWORD pData = (PWORD)((DWORD)pRelocation + 8);

		for (DWORD i = 0; i < dwSize; i++)
		{
			int iType = pData[i] >> 12;
			DWORD *dwRelocationPointer =  (DWORD *)((DWORD)pDest + ((pData[i] & 0x0FFF) + pRelocation->VirtualAddress));

			switch (iType)
			{
				case IMAGE_REL_BASED_ABSOLUTE:
				{
					break;
				}

				case IMAGE_REL_BASED_HIGH:
				{
					*(PWORD)dwRelocationPointer = (WORD)(((dwBaseDelta + *(PWORD)dwRelocationPointer) >> 16) & 0xFFFF);
					break;
				}

				case IMAGE_REL_BASED_LOW:
				{
					*(PWORD)dwRelocationPointer = (WORD)((dwBaseDelta + *(PWORD)dwRelocationPointer) & 0xFFFF);
					break;
				}

				case IMAGE_REL_BASED_HIGHLOW:
				{
					*dwRelocationPointer = *dwRelocationPointer + dwBaseDelta;
					break;
				}

				default:
				{
					break;
				}
			}
		}

		pRelocation = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocation + pRelocation->SizeOfBlock);		
	}
}

FARPROC WINAPI MEM_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	if (!hModule)
		return NULL;

	PBYTE pBuffer = (PBYTE)hModule;
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pBuffer;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(pBuffer + pDOSHeader->e_lfanew);
	PIMAGE_DATA_DIRECTORY pDirectory = &pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

	if (pDirectory->Size == 0)
		return NULL;

	PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pBuffer + pDirectory->VirtualAddress);

	if (pExport->NumberOfNames == 0 || pExport->NumberOfFunctions == 0)
		return NULL;

	int iIndex = -1;
	WORD *pOrdinal = (WORD *)((DWORD)pBuffer + pExport->AddressOfNameOrdinals);

	if (!HIWORD(lpProcName))
	{
		if ((DWORD)lpProcName >= pExport->NumberOfFunctions + pExport->Base || (DWORD)lpProcName < pExport->Base)
			return NULL;

		iIndex = (int)lpProcName - pExport->Base;
	}
	else
	{
		DWORD *pNameRef = (DWORD *)((DWORD)pBuffer + pExport->AddressOfNames);

		for (DWORD i = 0; i < pExport->NumberOfNames; i++, pNameRef++, pOrdinal++)
		{
			if (strcmp(lpProcName, (const char *)((DWORD)pBuffer + (*pNameRef))) == 0)
			{
				iIndex = *pOrdinal;
				break;
			}
		}
	}

	if (iIndex == -1)
		return NULL;

	if ((DWORD)iIndex > pExport->NumberOfFunctions)
		return NULL;

	FARPROC proc =  (FARPROC)((DWORD)hModule + (*(DWORD *)((DWORD)hModule + pExport->AddressOfFunctions + (iIndex * 4))));

	if (proc)
	{
		if (((PBYTE)proc)[0] == 'N' && ((PBYTE)proc)[1] == 'T' && ((PBYTE)proc)[2] == 'D' && ((PBYTE)proc)[3] == 'L' && ((PBYTE)proc)[4] == 'L')
			return GetProcAddress(GetModuleHandle("ntdll"), (LPCSTR)&((PBYTE)proc)[6]);
	}

	return proc;
}

void MEM_GetImportInfo(DWORD dwImageBase, DWORD dwImportOffset)
{
	PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)(dwImageBase + dwImportOffset);

	for (DWORD i = 0; pImport[i].Characteristics != 0; i++)
	{
		HMODULE hModule = LoadLibrary((LPCSTR)(pImport[i].Name + dwImageBase));
		PIMAGE_THUNK_DATA32 pImportHunk = (PIMAGE_THUNK_DATA32)(pImport[i].OriginalFirstThunk + dwImageBase);
		FARPROC *pImportWrite = (FARPROC *)(pImport[i].FirstThunk + dwImageBase);

		for (DWORD p = 0; pImportHunk[p].u1.AddressOfData != 0; p++)
		{
			PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pImportHunk[p].u1.AddressOfData + dwImageBase);

			if (IMAGE_SNAP_BY_ORDINAL32(pImportHunk[p].u1.AddressOfData))
				pImportWrite[p] = MEM_GetProcAddress(hModule, (LPCSTR)IMAGE_ORDINAL(pImportHunk[p].u1.AddressOfData));
			else
				pImportWrite[p] = MEM_GetProcAddress(hModule, (LPCSTR)&pImportName->Name);

			if (!pImportWrite[p])
				DebugBreak();
		}
	}
}

void MEM_LoadImport(PBYTE pSrc, PBYTE pDest)
{
	PIMAGE_DOS_HEADER pSrcDOSHeader = (PIMAGE_DOS_HEADER)pSrc;
	PIMAGE_NT_HEADERS pSrcNTHeader = (PIMAGE_NT_HEADERS)(pSrc + pSrcDOSHeader->e_lfanew);
	PIMAGE_DOS_HEADER pDstDOSHeader = (PIMAGE_DOS_HEADER)pDest;
	PIMAGE_NT_HEADERS pDstNTHeader = (PIMAGE_NT_HEADERS)(pDest + pDstDOSHeader->e_lfanew);
	PIMAGE_DATA_DIRECTORY pDirectory = &pDstNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!pDirectory->VirtualAddress)
		return;

	MEM_GetImportInfo((DWORD)pDest, pDirectory->VirtualAddress);
}

HMODULE WINAPI MEM_LoadLibrary(PBYTE pBuffer, BOOL bCallEntry, LPVOID lpvReserved)
{
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pBuffer;

	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(pBuffer + pDOSHeader->e_lfanew);

	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	PBYTE pNewBuf = (PBYTE)VirtualAlloc(NULL, pNTHeader->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(pNewBuf, pBuffer, pDOSHeader->e_lfanew + pNTHeader->OptionalHeader.SizeOfHeaders);

	pDOSHeader = (PIMAGE_DOS_HEADER)pNewBuf;
	pNTHeader = (PIMAGE_NT_HEADERS)(pNewBuf + pDOSHeader->e_lfanew);
	pNTHeader->OptionalHeader.ImageBase = (DWORD)pNewBuf;
	pDOSHeader->e_magic = *(WORD *)"ML";

	MEM_CopySection(pBuffer, pNewBuf);
	MEM_LoadRelocation(pBuffer, pNewBuf);
	MEM_LoadImport(pBuffer, pNewBuf);

	if (bCallEntry)
	{
		if (pNTHeader->OptionalHeader.AddressOfEntryPoint)
		{
			BOOL (WINAPI *pfnDllMain)(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved) = (BOOL (WINAPI *)(HINSTANCE, DWORD, LPVOID))(pNTHeader->OptionalHeader.AddressOfEntryPoint + pNewBuf);
			pfnDllMain((HINSTANCE)pNewBuf, DLL_PROCESS_ATTACH, lpvReserved);
		}
	}

	return (HMODULE)pNewBuf;
}