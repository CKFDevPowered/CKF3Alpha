#include <metahook.h>
#include "scripts.h"
#include "modules.h"
#include <UtlVector.h>

#pragma pack(push, 1)

typedef struct
{
	union
	{
		DWORD Characteristics;
		DWORD OriginalFirstThunk;
	};

	DWORD FirstThunk;
}
IMAGE_IMPORT_DESCRIPTOR2;

#undef IMAGE_DIRECTORY_ENTRY_EXPORT
#undef IMAGE_DIRECTORY_ENTRY_IMPORT
#undef IMAGE_DIRECTORY_ENTRY_BASERELOC
#undef IMAGE_NUMBEROF_DIRECTORY_ENTRIES

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 2
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 3

typedef struct
{
	WORD NumberOfSections;
	DWORD ImageBase;
	DWORD SizeOfImage;
	DWORD SizeOfHeaders;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
}
IMAGE_NT_HEADERS2;

typedef struct
{
	DWORD NumberOfFunctions;
	DWORD NumberOfNames;
	DWORD AddressOfFunctions;
	DWORD AddressOfNames;
	DWORD AddressOfNameOrdinals;
}
IMAGE_EXPORT_DIRECTORY2;

#pragma pack(pop)

Script::Script(byte *buffer, size_t len, const char *interfaceName)
{
	PVOID pData = (PVOID)buffer;
	IMAGE_NT_HEADERS2 *pHeader = (IMAGE_NT_HEADERS2 *)pData;

	m_pExportList = NULL;
	m_pszInterfaceName = interfaceName;
	m_dwImageSize = pHeader->SizeOfImage;
	m_pImageBase = VirtualAlloc(NULL, m_dwImageSize, MEM_RESERVE, PAGE_NOACCESS);

	m_dwSectionSize = pHeader->SizeOfHeaders;
	m_pSectionBase = VirtualAlloc(m_pImageBase, m_dwSectionSize, MEM_COMMIT, PAGE_READWRITE);

	MoveMemory(m_pSectionBase, pData, m_dwSectionSize);

	DWORD dwFlags;
	VirtualProtect(m_pSectionBase, m_dwSectionSize, PAGE_READONLY, &dwFlags);

	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((PBYTE)pHeader + sizeof(IMAGE_NT_HEADERS2));

	m_dwSectionCount = pHeader->NumberOfSections;
	m_pSectionsBase = (PVOID *)malloc(m_dwSectionCount * sizeof(PVOID));
	m_pSectionsSize = (DWORD *)malloc(m_dwSectionCount * sizeof(DWORD));

	for (DWORD i = 0; i < m_dwSectionCount; ++i)
	{
		m_pSectionsSize[i] = pSectionHeader[i].SizeOfRawData;

		if (m_pSectionsSize[i] < pSectionHeader[i].Misc.VirtualSize)
			m_pSectionsSize[i] = pSectionHeader[i].Misc.VirtualSize;

		m_pSectionsBase[i] = VirtualAlloc((PBYTE)m_pImageBase + pSectionHeader[i].VirtualAddress, m_pSectionsSize[i], MEM_COMMIT, PAGE_READWRITE);

		if (pSectionHeader[i].PointerToRawData)
			MoveMemory(m_pSectionsBase[i], (PBYTE)pData + pSectionHeader[i].PointerToRawData, pSectionHeader[i].SizeOfRawData);
	}

#define CONVERT_POINTER(addr, ret) \
	ret = 0; \
	for (DWORD i = 0; i < m_dwSectionCount; i++) \
	{ \
		if (addr < pSectionHeader[i].VirtualAddress + m_pSectionsSize[i] && addr >= pSectionHeader[i].VirtualAddress) \
		{ \
			ret = (addr - pSectionHeader[i].VirtualAddress) + (DWORD)m_pSectionsBase[i]; \
			break; \
		} \
	}

	if (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
	{
		DWORD dwRelocations = 0;

		for (DWORD i = 0; i < m_dwSectionCount; ++i)
		{
			if (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress < pSectionHeader[i].VirtualAddress + m_pSectionsSize[i] && pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress >= pSectionHeader[i].VirtualAddress)
			{
				dwRelocations = (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress - pSectionHeader[i].VirtualAddress) + (DWORD)m_pSectionsBase[i];
				break;
			}
		}

		DWORD dwBaseDelta = (DWORD)m_pImageBase - pHeader->ImageBase;

		if (dwRelocations)
		{
			for (DWORD i = 0; i < pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size; )
			{
				PIMAGE_BASE_RELOCATION pBaseRelocation = (PIMAGE_BASE_RELOCATION)dwRelocations;
				DWORD dwBase = 0;

				for (DWORD j = 0; j < m_dwSectionCount; ++j)
				{
					if (pBaseRelocation->VirtualAddress < pSectionHeader[j].VirtualAddress + m_pSectionsSize[j] && pBaseRelocation->VirtualAddress >= pSectionHeader[j].VirtualAddress)
					{
						dwBase = (pBaseRelocation->VirtualAddress - pSectionHeader[j].VirtualAddress) + (DWORD)m_pSectionsBase[j];
						break;
					}
				}

				if (!dwBase)
					break;

				DWORD dwNumberOfRelocations = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
				PWORD pRelocation = (PWORD)(dwRelocations + sizeof(IMAGE_BASE_RELOCATION));

				for (DWORD j = 0; j < dwNumberOfRelocations; ++j)
				{
					DWORD dwRelocationPointer = dwBase + (pRelocation[j] & 0xFFF);
					DWORD dwRelocationType = pRelocation[j] >> 12;

					switch (dwRelocationType)
					{
						case IMAGE_REL_BASED_ABSOLUTE: break;
						case IMAGE_REL_BASED_HIGH: *(PWORD)dwRelocationPointer = (WORD)(((dwBaseDelta + *(PWORD)dwRelocationPointer) >> 16) & 0xFFFF); break;
						case IMAGE_REL_BASED_LOW: *(PWORD)dwRelocationPointer = (WORD)((dwBaseDelta + *(PWORD)dwRelocationPointer) & 0xFFFF); break;
						case IMAGE_REL_BASED_HIGHLOW: *(PDWORD)dwRelocationPointer = dwBaseDelta + *(PDWORD)dwRelocationPointer; break;
					}
				}

				i += pBaseRelocation->SizeOfBlock;
				dwRelocations += pBaseRelocation->SizeOfBlock;
			}
		}
	}

	if (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
	{
		DWORD dwImportDescriptor = 0;

		for (DWORD i = 0; i < m_dwSectionCount; ++i)
		{
			if (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress < pSectionHeader[i].VirtualAddress + m_pSectionsSize[i] && pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress >= pSectionHeader[i].VirtualAddress)
			{
				dwImportDescriptor = (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress - pSectionHeader[i].VirtualAddress) + (DWORD)m_pSectionsBase[i];
				break;
			}
		}

		CreateInterfaceFn fnCreateInterface = Sys_GetFactoryThis();
		DWORD (*pfnGetProcAddress)(char *pszProcName) = (DWORD (*)(char *))fnCreateInterface(m_pszInterfaceName, NULL);
		IMAGE_IMPORT_DESCRIPTOR2 *pImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR2 *)dwImportDescriptor;

		if (pImportDescriptor)
		{
			HMODULE hModule = g_hThisModule;
			PDWORD pThunkData;
			DWORD dwThunkData;
			CONVERT_POINTER(pImportDescriptor->FirstThunk, dwThunkData);
			pThunkData = (PDWORD)dwThunkData;

			while (pThunkData && *pThunkData)
			{
				DWORD dwFunctionPointer;
				DWORD dwFuncName;
				DWORD dwTmp = (DWORD)(&PIMAGE_IMPORT_BY_NAME(*pThunkData)->Name);

				CONVERT_POINTER(dwTmp, dwFuncName);
				char *pszFuncName = (char *)dwFuncName;
				dwFunctionPointer = pfnGetProcAddress(pszFuncName);
				*pThunkData = dwFunctionPointer;
				pThunkData++;
			}
		}
	}

	for (DWORD i = 0; i < m_dwSectionCount; ++i)
	{
		DWORD dwCharacteristics = pSectionHeader[i].Characteristics;
		DWORD dwFlags = 0;

		if (dwCharacteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			if (dwCharacteristics & IMAGE_SCN_MEM_READ)
			{
				if (dwCharacteristics & IMAGE_SCN_MEM_WRITE)
					dwFlags |= PAGE_EXECUTE_READWRITE;
				else
					dwFlags |= PAGE_EXECUTE_READ;
			}
			else if (dwCharacteristics & IMAGE_SCN_MEM_WRITE)
				dwFlags |= PAGE_EXECUTE_WRITECOPY;
			else
				dwFlags |= PAGE_EXECUTE;
		}
		else if (dwCharacteristics & IMAGE_SCN_MEM_READ)
		{
			if (dwCharacteristics & IMAGE_SCN_MEM_WRITE)
				dwFlags |= PAGE_READWRITE;
			else
				dwFlags |= PAGE_READONLY;
		}
		else if (dwCharacteristics & IMAGE_SCN_MEM_WRITE)
			dwFlags |= PAGE_WRITECOPY;
		else
			dwFlags |= PAGE_NOACCESS;

		if (dwCharacteristics & IMAGE_SCN_MEM_NOT_CACHED)
			dwFlags |= PAGE_NOCACHE;

		DWORD dwProtect;
		VirtualProtect(m_pSectionsBase[i], m_pSectionsSize[i], dwFlags, &dwProtect);
	}

	if (pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
	{
		DWORD dwExportDirectory;
		CONVERT_POINTER(pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, dwExportDirectory);
		IMAGE_EXPORT_DIRECTORY2 *pExportDirectory = (IMAGE_EXPORT_DIRECTORY2 *)dwExportDirectory;

		if (pExportDirectory)
		{
			DWORD dwAddressOfNames;
			CONVERT_POINTER(pExportDirectory->AddressOfNames, dwAddressOfNames);
			PDWORD pAddressOfNames = (PDWORD)dwAddressOfNames;

			DWORD dwAddressOfNameOrdinals;
			CONVERT_POINTER(pExportDirectory->AddressOfNameOrdinals, dwAddressOfNameOrdinals);
			PWORD pAddressOfNameOrdinals = (PWORD)dwAddressOfNameOrdinals;

			DWORD dwAddressOfFunctions;
			CONVERT_POINTER(pExportDirectory->AddressOfFunctions, dwAddressOfFunctions);
			PDWORD pAddressOfFunctions = (PDWORD)dwAddressOfFunctions;

			DWORD dwExportDirectorySize = pHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

			for (DWORD j = 0; j < pExportDirectory->NumberOfNames; j++)
			{
				DWORD dwFuncName;
				CONVERT_POINTER(pAddressOfNames[j], dwFuncName);

				DWORD dwFuncIndex = pAddressOfNameOrdinals[j];
				DWORD dwFuncAddr;
				CONVERT_POINTER(pAddressOfFunctions[dwFuncIndex], dwFuncAddr);

				ExportFuncs *pFunItem = new ExportFuncs;
				pFunItem->dwFuncIndex = dwFuncIndex;
				pFunItem->pFuncName = (char *)dwFuncName;
				pFunItem->pFuncAddr = (FARPROC)dwFuncAddr;
				pFunItem->pNext = m_pExportList;
				m_pExportList = pFunItem;
			}
		}
	}
}

Script::~Script(void)
{
	while (m_pExportList)
	{
		ExportFuncs *pFunc = m_pExportList->pNext;
		delete m_pExportList;
		m_pExportList = pFunc;
	}

	if (m_pSectionBase)
	{
		VirtualFree(m_pSectionBase, m_dwSectionSize, MEM_DECOMMIT);
		m_pSectionBase = NULL;
	}

	if (m_dwSectionCount && m_pSectionsBase && m_pSectionsSize)
	{
		for (DWORD i = 0; i < m_dwSectionCount; i++)
			VirtualFree(m_pSectionsBase[i], m_pSectionsSize[i], MEM_DECOMMIT);
	}

	if (m_pSectionsBase)
	{
		free(m_pSectionsBase);
		m_pSectionsBase = NULL;
	}

	if (m_pSectionsSize)
	{
		free(m_pSectionsSize);
		m_pSectionsSize = NULL;
	}

	m_dwSectionCount = 0;
	m_dwSectionSize = 0;

	if (m_pImageBase && m_dwImageSize)
		VirtualFree(m_pImageBase, 0, MEM_RELEASE);

	m_pImageBase = NULL;
	m_dwImageSize = 0;
}

DWORD Script::GetProcAddress(const char *name)
{
	ExportFuncs *pFunc = m_pExportList;

	while (pFunc)
	{
		if (!strcmp(name, pFunc->pFuncName))
			return (DWORD)pFunc->pFuncAddr;

		pFunc = pFunc->pNext;
	}

	return 0;
}