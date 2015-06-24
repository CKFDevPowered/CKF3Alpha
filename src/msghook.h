#pragma once

void MSG_Init(void);
void MSG_HookMessage(void);
pfnUserMsgHook MSG_HookUserMsg(char *szMsgName, pfnUserMsgHook pfn);
pfnUserMsgHook MSG_UnHookUserMsg(char *szMsgName);
void MSG_WriteChar(int iValue);
void MSG_WriteByte(int iValue);
void MSG_WriteShort(int iValue);
void MSG_WriteWord(int iValue);
void MSG_WriteLong(int iValue);
void MSG_WriteFloat(float flValue);
void MSG_WriteString(const char *sz);
void MSG_WriteAngle(float flValue);
void MSG_WriteHiresAngle(float flValue);
void MSG_BeginWrite(char *szMsgName);
void MSG_EndWrite(void);
void MSG_BeginWrite(void);
void MSG_EndWrite(const char *pszName, pfnUserMsgHook pfn);