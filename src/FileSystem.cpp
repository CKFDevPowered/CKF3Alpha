#include "metahook.h"
#include "tier0/dbg.h"
#include "FileSystem.h"
#include "plugins.h"

IFileSystem *g_pFullFileSystem;

static void (__fastcall *m_pfnFileSystem_Mount)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnFileSystem_Unmount)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnFileSystem_RemoveAllSearchPaths)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnFileSystem_AddSearchPath)(void *pthis, int, const char *pPath, const char *pathID) = NULL;
static bool (__fastcall *m_pfnFileSystem_RemoveSearchPath)(void *pthis, int, const char *pPath) = NULL;
static void (__fastcall *m_pfnFileSystem_RemoveFile)(void *pthis, int, const char *pRelativePath, const char *pathID) = NULL;
static void (__fastcall *m_pfnFileSystem_CreateDirHierarchy)(void *pthis, int, const char *path, const char *pathID) = NULL;
static bool (__fastcall *m_pfnFileSystem_FileExists)(void *pthis, int, const char *pFileName) = NULL;
static bool (__fastcall *m_pfnFileSystem_IsDirectory)(void *pthis, int, const char *pFileName) = NULL;
static FileHandle_t (__fastcall *m_pfnFileSystem_Open)(void *pthis, int, const char *pFileName, const char *pOptions, const char *pathID) = NULL;
static void (__fastcall *m_pfnFileSystem_Close)(void *pthis, int, FileHandle_t file) = NULL;
static void (__fastcall *m_pfnFileSystem_Seek)(void *pthis, int, FileHandle_t file, int pos, FileSystemSeek_t seekType) = NULL;
static unsigned (__fastcall *m_pfnFileSystem_Tell)(void *pthis, int, FileHandle_t file) = NULL;
static unsigned (__fastcall *m_pfnFileSystem_Size)(void *pthis, int, FileHandle_t file) = NULL;
static unsigned (__fastcall *m_pfnFileSystem_Size2)(void *pthis, int, const char *pFileName) = NULL;
static long (__fastcall *m_pfnFileSystem_GetFileTime)(void *pthis, int, const char *pFileName) = NULL;
static void (__fastcall *m_pfnFileSystem_FileTimeToString)(void *pthis, int, char *pStrip, int maxCharsIncludingTerminator, long fileTime) = NULL;
static bool (__fastcall *m_pfnFileSystem_IsOk)(void *pthis, int, FileHandle_t file) = NULL;
static void (__fastcall *m_pfnFileSystem_Flush)(void *pthis, int, FileHandle_t file) = NULL;
static bool (__fastcall *m_pfnFileSystem_EndOfFile)(void *pthis, int, FileHandle_t file) = NULL;
static int (__fastcall *m_pfnFileSystem_Read)(void *pthis, int, void *pOutput, int size, FileHandle_t file) = NULL;
static int (__fastcall *m_pfnFileSystem_Write)(void *pthis, int, void const *pInput, int size, FileHandle_t file) = NULL;
static char *(__fastcall *m_pfnFileSystem_ReadLine)(void *pthis, int, char *pOutput, int maxChars, FileHandle_t file) = NULL;
static int (__fastcall *m_pfnFileSystem_FPrintf)(void *pthis, int, FileHandle_t file, char *pFormat, ...) = NULL;
static char *(__fastcall *m_pfnFileSystem_GetReadBuffer)(void *pthis, int, FileHandle_t file, char *pBuffer) = NULL;
static void (__fastcall *m_pfnFileSystem_ReleaseReadBuffer)(void *pthis, int, FileHandle_t file, char *pBuffer) = NULL;
static const char *(__fastcall *m_pfnFileSystem_FindFirst)(void *pthis, int, const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID) = NULL;
static const char *(__fastcall *m_pfnFileSystem_FindNext)(void *pthis, int, FileFindHandle_t handle) = NULL;
static bool (__fastcall *m_pfnFileSystem_FindIsDirectory)(void *pthis, int, FileFindHandle_t handle) = NULL;
static void (__fastcall *m_pfnFileSystem_FindClose)(void *pthis, int, FileFindHandle_t handle) = NULL;
static void (__fastcall *m_pfnFileSystem_GetLocalCopy)(void *pthis, int, const char *pFileName) = NULL;
static const char *(__fastcall *m_pfnFileSystem_GetLocalPath)(void *pthis, int, const char *pFileName, char *pLocalPath, int maxlen) = NULL;
static char *(__fastcall *m_pfnFileSystem_ParseFile)(void *pthis, int, char *data, char *token, bool *wasquoted) = NULL;
static bool (__fastcall *m_pfnFileSystem_FullPathToRelativePath)(void *pthis, int, const char *pFullpath, char *pRelative) = NULL;
static bool (__fastcall *m_pfnFileSystem_GetCurrentDirectory)(void *pthis, int, char *pDirectory, int maxlen) = NULL;
static void (__fastcall *m_pfnFileSystem_PrintOpenedFiles)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnFileSystem_SetWarningFunc)(void *pthis, int, void (*pfnWarning)(const char *fmt, ...)) = NULL;
static void (__fastcall *m_pfnFileSystem_SetWarningLevel)(void *pthis, int, FileWarningLevel_t level) = NULL;
static void (__fastcall *m_pfnFileSystem_LogLevelLoadStarted)(void *pthis, int, const char *name) = NULL;
static void (__fastcall *m_pfnFileSystem_LogLevelLoadFinished)(void *pthis, int, const char *name) = NULL;
static int (__fastcall *m_pfnFileSystem_HintResourceNeed)(void *pthis, int, const char *hintlist, int forgetEverything) = NULL;
static int (__fastcall *m_pfnFileSystem_PauseResourcePreloading)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnFileSystem_ResumeResourcePreloading)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnFileSystem_SetVBuf)(void *pthis, int, FileHandle_t stream, char *buffer, int mode, long size) = NULL;
static void (__fastcall *m_pfnFileSystem_GetInterfaceVersion)(void *pthis, int, char *p, int maxlen) = NULL;
static bool (__fastcall *m_pfnFileSystem_IsFileImmediatelyAvailable)(void *pthis, int, const char *path) = NULL;
static void *(__fastcall *m_pfnFileSystem_WaitForResources)(void *pthis, int, const char *pFileName) = NULL;
static bool (__fastcall *m_pfnFileSystem_GetWaitForResourcesProgress)(void *pthis, int, WaitForResourcesHandle_t handle, float *progress, bool *complete) = NULL;
static void (__fastcall *m_pfnFileSystem_CancelWaitForResources)(void *pthis, int, WaitForResourcesHandle_t handle) = NULL;
static bool (__fastcall *m_pfnFileSystem_IsAppReadyForOfflinePlay)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnFileSystem_AddPackFile)(void *pthis, int, const char *pPath, const char *pathID) = NULL;
static void *(__fastcall *m_pfnFileSystem_OpenFromCacheForRead)(void *pthis, int, const char *pFileName, const char *pOptions, const char *pathID) = NULL;

class CFileSystem : public IFileSystem
{
public:
	virtual void Mount(void);
	virtual void Unmount(void);
	virtual void RemoveAllSearchPaths(void);
	virtual void AddSearchPath(const char *pPath, const char *pathID = 0);
	virtual bool RemoveSearchPath(const char *pPath);
	virtual void RemoveFile(const char *pRelativePath, const char *pathID = 0);
	virtual void CreateDirHierarchy(const char *path, const char *pathID = 0);
	virtual bool FileExists(const char *pFileName);
	virtual bool IsDirectory(const char *pFileName);
	virtual FileHandle_t Open(const char *pFileName, const char *pOptions, const char *pathID = 0);
	virtual void Close(FileHandle_t file);
	virtual void Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType);
	virtual unsigned Tell(FileHandle_t file);
	virtual unsigned Size(FileHandle_t file);
	virtual unsigned Size(const char *pFileName);
	virtual long GetFileTime(const char *pFileName);
	virtual void FileTimeToString(char *pStrip, int maxCharsIncludingTerminator, long fileTime);
	virtual bool IsOk(FileHandle_t file);
	virtual void Flush(FileHandle_t file);
	virtual bool EndOfFile(FileHandle_t file);
	virtual int Read(void *pOutput, int size, FileHandle_t file);
	virtual int Write(void const *pInput, int size, FileHandle_t file);
	virtual char *ReadLine(char *pOutput, int maxChars, FileHandle_t file);
	virtual int FPrintf(FileHandle_t file, char *pFormat, ...);
	virtual char *GetReadBuffer(FileHandle_t file, char *pBuffer);
	virtual void ReleaseReadBuffer(FileHandle_t file, char *pBuffer);
	virtual const char *FindFirst(const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID = 0);
	virtual const char *FindNext(FileFindHandle_t handle);
	virtual bool FindIsDirectory(FileFindHandle_t handle);
	virtual void FindClose(FileFindHandle_t handle);
	virtual void GetLocalCopy(const char *pFileName);
	virtual const char *GetLocalPath(const char *pFileName, char *pLocalPath, int maxlen);
	virtual char *ParseFile(char *data, char *token, bool *wasquoted);
	virtual bool FullPathToRelativePath(const char *pFullpath, char *pRelative);
	virtual bool GetCurrentDirectory(char *pDirectory, int maxlen);
	virtual void PrintOpenedFiles(void);
	virtual void SetWarningFunc(void (*pfnWarning)(const char *fmt, ...));
	virtual void SetWarningLevel(FileWarningLevel_t level);
	virtual void LogLevelLoadStarted(const char *name);
	virtual void LogLevelLoadFinished(const char *name);
	virtual int HintResourceNeed(const char *hintlist, int forgetEverything);
	virtual int PauseResourcePreloading(void);
	virtual int ResumeResourcePreloading(void);
	virtual int SetVBuf(FileHandle_t stream, char *buffer, int mode, long size);
	virtual void GetInterfaceVersion(char *p, int maxlen);
	virtual bool IsFileImmediatelyAvailable(const char *path);
	virtual void *WaitForResources(const char *pFileName);
	virtual bool GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float *progress, bool *complete);
	virtual void CancelWaitForResources(WaitForResourcesHandle_t handle);
	virtual bool IsAppReadyForOfflinePlay(void);
	virtual void AddPackFile(const char *pPath, const char *pathID = 0);
	virtual void *OpenFromCacheForRead(const char *pFileName, const char *pOptions, const char *pathID = 0);
};

CFileSystem g_FileSystem;

void CFileSystem::Mount(void)
{
	return m_pfnFileSystem_Mount(this, 0);
}

void CFileSystem::Unmount(void)
{
	return m_pfnFileSystem_Unmount(this, 0);
}

void CFileSystem::RemoveAllSearchPaths(void)
{
	return m_pfnFileSystem_RemoveAllSearchPaths(this, 0);
}

void CFileSystem::AddSearchPath(const char *pPath, const char *pathID)
{
	if (!g_bIsUseSteam)
	{
		if (!strcmp(pathID, "PLATFORM"))
			return;
	}

	return m_pfnFileSystem_AddSearchPath(this, 0, pPath, pathID);
}

bool CFileSystem::RemoveSearchPath(const char *pPath)
{
	return m_pfnFileSystem_RemoveSearchPath(this, 0, pPath);
}

void CFileSystem::RemoveFile(const char *pRelativePath, const char *pathID)
{
	return m_pfnFileSystem_RemoveFile(this, 0, pRelativePath, pathID);
}

void CFileSystem::CreateDirHierarchy(const char *path, const char *pathID)
{
	return m_pfnFileSystem_CreateDirHierarchy(this, 0, path, pathID);
}

bool CFileSystem::FileExists(const char *pFileName)
{
	return m_pfnFileSystem_FileExists(this, 0, pFileName);
}

bool CFileSystem::IsDirectory(const char *pFileName)
{
	return m_pfnFileSystem_IsDirectory(this, 0, pFileName);
}

FileHandle_t CFileSystem::Open(const char *pFileName, const char *pOptions, const char *pathID)
{
	return m_pfnFileSystem_Open(this, 0, pFileName, pOptions, pathID);
}

void CFileSystem::Close(FileHandle_t file)
{
	return m_pfnFileSystem_Close(this, 0, file);
}

void CFileSystem::Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType)
{
	return m_pfnFileSystem_Seek(this, 0, file, pos, seekType);
}

unsigned CFileSystem::Tell(FileHandle_t file)
{
	return m_pfnFileSystem_Tell(this, 0, file);
}

unsigned CFileSystem::Size(FileHandle_t file)
{
	return m_pfnFileSystem_Size(this, 0, file);
}

unsigned CFileSystem::Size(const char *pFileName)
{
	return m_pfnFileSystem_Size2(this, 0, pFileName);
}

long CFileSystem::GetFileTime(const char *pFileName)
{
	return m_pfnFileSystem_GetFileTime(this, 0, pFileName);
}

void CFileSystem::FileTimeToString(char *pStrip, int maxCharsIncludingTerminator, long fileTime)
{
	return m_pfnFileSystem_FileTimeToString(this, 0, pStrip, maxCharsIncludingTerminator, fileTime);
}

bool CFileSystem::IsOk(FileHandle_t file)
{
	return m_pfnFileSystem_IsOk(this, 0, file);
}

void CFileSystem::Flush(FileHandle_t file)
{
	return m_pfnFileSystem_Flush(this, 0, file);
}

bool CFileSystem::EndOfFile(FileHandle_t file)
{
	return m_pfnFileSystem_EndOfFile(this, 0, file);
}

int CFileSystem::Read(void *pOutput, int size, FileHandle_t file)
{
	return m_pfnFileSystem_Read(this, 0, pOutput, size, file);
}

int CFileSystem::Write(void const *pInput, int size, FileHandle_t file)
{
	return m_pfnFileSystem_Write(this, 0, pInput, size, file);
}

char *CFileSystem::ReadLine(char *pOutput, int maxChars, FileHandle_t file)
{
	return m_pfnFileSystem_ReadLine(this, 0, pOutput, maxChars, file);
}

int CFileSystem::FPrintf(FileHandle_t file, char *pFormat, ...)
{
	__asm jmp m_pfnFileSystem_FPrintf;
}

char *CFileSystem::GetReadBuffer(FileHandle_t file, char *pBuffer)
{
	return m_pfnFileSystem_GetReadBuffer(this, 0, file, pBuffer);
}

void CFileSystem::ReleaseReadBuffer(FileHandle_t file, char *pBuffer)
{
	return m_pfnFileSystem_ReleaseReadBuffer(this, 0, file, pBuffer);
}

const char *CFileSystem::FindFirst(const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID)
{
	return m_pfnFileSystem_FindFirst(this, 0, pWildCard, pHandle, pathID);
}

const char *CFileSystem::FindNext(FileFindHandle_t handle)
{
	return m_pfnFileSystem_FindNext(this, 0, handle);
}

bool CFileSystem::FindIsDirectory(FileFindHandle_t handle)
{
	return m_pfnFileSystem_FindIsDirectory(this, 0, handle);
}

void CFileSystem::FindClose(FileFindHandle_t handle)
{
	return m_pfnFileSystem_FindClose(this, 0, handle);
}

void CFileSystem::GetLocalCopy(const char *pFileName)
{
	return m_pfnFileSystem_GetLocalCopy(this, 0, pFileName);
}

const char *CFileSystem::GetLocalPath(const char *pFileName, char *pLocalPath, int maxlen)
{
	return m_pfnFileSystem_GetLocalPath(this, 0, pFileName, pLocalPath, maxlen);
}

char *CFileSystem::ParseFile(char *data, char *token, bool *wasquoted)
{
	return m_pfnFileSystem_ParseFile(this, 0, data, token, wasquoted);
}

bool CFileSystem::FullPathToRelativePath(const char *pFullpath, char *pRelative)
{
	return m_pfnFileSystem_FullPathToRelativePath(this, 0, pFullpath, pRelative);
}

bool CFileSystem::GetCurrentDirectory(char *pDirectory, int maxlen)
{
	return m_pfnFileSystem_GetCurrentDirectory(this, 0, pDirectory, maxlen);
}

void CFileSystem::PrintOpenedFiles(void)
{
	return m_pfnFileSystem_PrintOpenedFiles(this, 0);
}

void CFileSystem::SetWarningFunc(void (*pfnWarning)(const char *fmt, ...))
{
	return m_pfnFileSystem_SetWarningFunc(this, 0, pfnWarning);
}

void CFileSystem::SetWarningLevel(FileWarningLevel_t level)
{
	return m_pfnFileSystem_SetWarningLevel(this, 0, level);
}

void CFileSystem::LogLevelLoadStarted(const char *name)
{
	return m_pfnFileSystem_LogLevelLoadStarted(this, 0, name);
}

void CFileSystem::LogLevelLoadFinished(const char *name)
{
	return m_pfnFileSystem_LogLevelLoadFinished(this, 0, name);
}

int CFileSystem::HintResourceNeed(const char *hintlist, int forgetEverything)
{
	return m_pfnFileSystem_HintResourceNeed(this, 0, hintlist, forgetEverything);
}

int CFileSystem::PauseResourcePreloading(void)
{
	return m_pfnFileSystem_PauseResourcePreloading(this, 0);
}

int CFileSystem::ResumeResourcePreloading(void)
{
	return m_pfnFileSystem_ResumeResourcePreloading(this, 0);
}

int CFileSystem::SetVBuf(FileHandle_t stream, char *buffer, int mode, long size)
{
	return m_pfnFileSystem_SetVBuf(this, 0, stream, buffer, mode, size);
}

void CFileSystem::GetInterfaceVersion(char *p, int maxlen)
{
	return m_pfnFileSystem_GetInterfaceVersion(this, 0, p, maxlen);
}

bool CFileSystem::IsFileImmediatelyAvailable(const char *path)
{
	return m_pfnFileSystem_IsFileImmediatelyAvailable(this, 0, path);
}

void *CFileSystem::WaitForResources(const char *pFileName)
{
	return m_pfnFileSystem_WaitForResources(this, 0, pFileName);
}

bool CFileSystem::GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float *progress, bool *complete)
{
	return m_pfnFileSystem_GetWaitForResourcesProgress(this, 0, handle, progress, complete);
}

void CFileSystem::CancelWaitForResources(WaitForResourcesHandle_t handle)
{
	return m_pfnFileSystem_CancelWaitForResources(this, 0, handle);
}

bool CFileSystem::IsAppReadyForOfflinePlay(void)
{
	return m_pfnFileSystem_IsAppReadyForOfflinePlay(this, 0);
}

void CFileSystem::AddPackFile(const char *pPath, const char *pathID)
{
	return m_pfnFileSystem_AddPackFile(this, 0, pPath, pathID);
}

void *CFileSystem::OpenFromCacheForRead(const char *pFileName, const char *pOptions, const char *pathID)
{
	return m_pfnFileSystem_OpenFromCacheForRead(this, 0, pFileName, pOptions, pathID);
}

void FileSystem_InstallHook(IFileSystem *pFileSystem)
{
	DWORD *pVFTable = *(DWORD **)&g_FileSystem;

	g_pFullFileSystem = pFileSystem;
	g_pMetaHookAPI->VFTHook(pFileSystem, 0, 4, (void *)pVFTable[4], (void *&)m_pfnFileSystem_AddSearchPath);
}