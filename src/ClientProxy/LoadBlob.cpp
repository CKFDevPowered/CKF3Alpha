#include <windows.h>
#include <stdio.h>
#include "LoadBlob.h"
#include <interface.h>
#include "IFileSystem.h"

BlobHeader_t g_BlobHeader;

BlobHeader_t *GetBlobHeader(void)
{
	return &g_BlobHeader;
}

BOOL FIsBlob(const char *pstFileName)
{
	FILE *file = fopen(pstFileName, "rb");

	if (file == NULL)
		return FALSE;

	BlobInfo_t info;
	fread(&info, sizeof(BlobInfo_t), 1, file);
	fclose(file);

	if (info.m_dwAlgorithm != BLOB_ALGORITHM)
		return FALSE;

	return TRUE;
}

DWORD NLoadBlobFile(const char *pstFileName, BlobFootprint_t *pblobfootprint, void **pv)
{
	FILE *file = fopen(pstFileName, "rb");

	DWORD dwSize;
	BYTE *pBuffer;
	DWORD dwAddress;

	fseek(file, 0, SEEK_END);
	dwSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	pBuffer = (BYTE *)malloc(dwSize);
	fread(pBuffer, dwSize, 1, file);

	dwAddress = LoadBlobFile(pBuffer, pblobfootprint, pv, dwSize);
	free(pBuffer);
	fclose(file);
	return dwAddress;
}

DWORD LoadBlobFile(BYTE *pBuffer, BlobFootprint_t *pblobfootprint, void **pv, DWORD dwSize)
{
	BYTE bXor = 0x57;
	BlobHeader_t *pHeader;
	BlobSection_t *pSection;
	DWORD dwAddress = 0;

	for (size_t i = sizeof(BlobInfo_t); i < dwSize; i++)
	{
		pBuffer[i] ^= bXor;
		bXor += pBuffer[i] + 0x57;
	}

	pHeader = (BlobHeader_t *)(pBuffer + sizeof(BlobInfo_t));
	pHeader->m_dwExportPoint ^= 0x7A32BC85;
	pHeader->m_dwImageBase ^= 0x49C042D1;
	pHeader->m_dwEntryPoint -= 12;
	pHeader->m_dwImportTable ^= 0x872C3D47;
	pSection = (BlobSection_t *)(pBuffer + sizeof(BlobInfo_t) + sizeof(BlobHeader_t));

	memcpy(&g_BlobHeader, pHeader, sizeof(BlobHeader_t));

	for (WORD j = 0; j <= pHeader->m_wSectionCount; j++)
	{
		if (pSection[j].m_bIsSpecial)
			dwAddress = pSection[j].m_dwDataAddress;

		if (pSection[j].m_dwVirtualSize > pSection[j].m_dwDataSize)
			memset((BYTE *)(pSection[j].m_dwVirtualAddress + pSection[j].m_dwDataSize), NULL, pSection[j].m_dwVirtualSize - pSection[j].m_dwDataSize);

		memcpy((BYTE *)pSection[j].m_dwVirtualAddress, pBuffer + pSection[j].m_dwDataAddress, pSection[j].m_dwDataSize);
	}

	PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)pHeader->m_dwImportTable;

	while (pImport->Name)
	{
		HMODULE hPorcDll = LoadLibrary((char *)(pHeader->m_dwImageBase + pImport->Name));
		PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pHeader->m_dwImageBase + pImport++->FirstThunk);

		while (pThunk->u1.Function)
		{
			const char *pszProcName = IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal) ? (char *)((LONG)pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32 - 1) : (char *)(pHeader->m_dwImageBase + ((IMAGE_IMPORT_BY_NAME *)((LONG)pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32 - 1))->Name);
			pThunk++->u1.AddressOfData = (DWORD)GetProcAddress(hPorcDll, pszProcName);
		}
	}

	((BOOL (WINAPI *)(HINSTANCE, DWORD, void *))(pHeader->m_dwEntryPoint))(0, DLL_PROCESS_ATTACH, 0);
	((void (*)(void **))(pHeader->m_dwExportPoint))(pv);
	return dwAddress;
}

void FreeBlob(BlobFootprint_t *pblobfootprint)
{
	FreeLibrary(pblobfootprint->m_hDll);
}