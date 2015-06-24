#include <metahook.h>

ICommandLine *CommandLine(void)
{
	return g_pInterface->CommandLine;
}