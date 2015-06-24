#include "tier0/dbg.h"
#include "FileSystem.h"

bool IFileSystem::FileExists(const char *pFileName, const char *pPathID)
{
	if (pPathID)
	{
		Assert(0);
		return false;
	}

	return FileExists(pFileName);
}

bool IFileSystem::IsFileWritable(char const *pFileName, const char *pPathID)
{
	Assert(0);
	return false;
}

bool IFileSystem::SetFileWritable(char const *pFileName, bool writable, const char *pPathID)
{
	Assert(0);
	return false;
}

bool IFileSystem::IsDirectory(const char *pFileName, const char *pathID)
{
	Assert(0);
	return false;
}

bool IFileSystem::GetFileTypeForFullPath(char const *pFullPath, wchar_t *buf, size_t bufSizeInBytes)
{
	Assert(0);
	return false;
}