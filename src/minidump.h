#include "tier0/minidump.h"

DWORD GetLastMiniDumpFileSize(void);
FnMiniDump SetMiniDumpFunction(FnMiniDump pfn);
void WriteMiniDump(void);
void CatchAndWriteMiniDump(FnWMain pfn, int argc, tchar *argv[]);
void SetupExceptionHandle(void);