#include "hud.h"
#include "buy_preset_debug.h"

#ifdef _DEBUG
#define BUY_PRESET_DEBUGGING 1
#else
#define BUY_PRESET_DEBUGGING 0
#endif

#if BUY_PRESET_DEBUGGING

bool IsPresetDebuggingEnabled(void)
{
	return true;
}

bool IsPresetFullCostDebuggingEnabled(void)
{
	return true;
}

bool IsPresetCurrentCostDebuggingEnabled(void)
{
	return true;
}

#else

bool IsPresetDebuggingEnabled(void)
{
	return false;
}

bool IsPresetFullCostDebuggingEnabled(void)
{
	return false;
}

bool IsPresetCurrentCostDebuggingEnabled(void)
{
	return false;
}

#endif