#include <dbghelp.h>
#include <tchar.h>

typedef char tchar;

typedef void (*FnMiniDump)(unsigned int uStructuredExceptionCode, _EXCEPTION_POINTERS *pExceptionInfo);

typedef void (*FnWMain)(int , tchar *[]);
typedef int (*FnWMainIntRet)(int , tchar *[]);
typedef void (*FnVoidPtrFn)(void *);

enum ECatchAndWriteMinidumpAction
{
	k_ECatchAndWriteMiniDumpAbort = 0,
	k_ECatchAndWriteMiniDumpReThrow = 1,
	k_ECatchAndWriteMiniDumpIgnore = 2,
};

bool WriteMiniDumpUsingExceptionInfo(unsigned int uStructuredExceptionCode, _EXCEPTION_POINTERS * pExceptionInfo, MINIDUMP_TYPE minidumpType, tchar *ptchMinidumpFileNameBuffer = NULL);
void CatchAndWriteMiniDump(FnWMain pfn, int argc, tchar *argv[]);
void CatchAndWriteMiniDumpForVoidPtrFn(FnVoidPtrFn pfn, void *pv, bool bExitQuietly);
void CatchAndWriteMiniDumpEx(FnWMain pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
int CatchAndWriteMiniDumpExReturnsInt(FnWMainIntRet pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
void CatchAndWriteMiniDumpExForVoidPtrFn(FnVoidPtrFn pfn, void *pv, ECatchAndWriteMinidumpAction eAction);