#ifndef BUY_PRESET_DEBUG_H
#define BUY_PRESET_DEBUG_H

#ifdef _WIN32
#pragma once
#endif

bool IsPresetDebuggingEnabled(void);
bool IsPresetFullCostDebuggingEnabled(void);
bool IsPresetCurrentCostDebuggingEnabled(void);

#define PRESET_DEBUG if (IsPresetDebuggingEnabled()) DevMsg
#define FULLCOST_DEBUG if (IsPresetFullCostDebuggingEnabled()) DevMsg
#define CURRENTCOST_DEBUG if (IsPresetCurrentCostDebuggingEnabled()) DevMsg

#endif