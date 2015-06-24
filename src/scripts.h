class Script
{
public:
	Script(byte *buffer, size_t len, const char *interfaceName);
	~Script(void);

public:
	DWORD GetProcAddress(const char *name);

private:
	PVOID m_pImageBase;
	DWORD m_dwImageSize;
	PVOID m_pSectionBase;
	DWORD m_dwSectionSize;
	PVOID *m_pSectionsBase;
	DWORD *m_pSectionsSize;
	DWORD m_dwSectionCount;

	struct ExportFuncs
	{
		FARPROC pFuncAddr;
		DWORD dwFuncIndex;
		char *pFuncName;
		struct ExportFuncs *pNext;
	};

	ExportFuncs *m_pExportList;
	const char *m_pszInterfaceName;
};

Script *LoadScript(const char *name);
void ReleaseAllScript(void);