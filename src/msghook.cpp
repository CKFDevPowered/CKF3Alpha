#include <metahook.h>
#include "parsemsg.h"
#include "msghook.h"
#include "console.h"

typedef struct usermsg_s
{
	int index;
	int size;
	char name[16];
	struct usermsg_s *next;
	pfnUserMsgHook function;
}
usermsg_t;

usermsg_t **gClientUserMsgs = NULL;

byte gMsgData[512];
size_t gMsgDataSize;
pfnUserMsgHook gMsgFunction;
const char *gMsgName;

#define GetCallAddress(addr) (addr + (*(DWORD *)(addr+1)) + 5)

void MSG_Init(void)
{
	//DWORD address = (DWORD)g_pMetaSave->pEngineFuncs->pfnHookUserMsg;

	//if (*(BYTE *)(address + 0x1A) != 0xE8)
	//	address += 0x19;
	//else
	//	address += 0x1A;

	//address += 0x1;
	//address += *(DWORD *)address + 0x4;

	//if (*(BYTE *)(address + 0xC) != 0x35)
	//	address += 0x9;
	//else
	//	address += 0xC;

	//gClientUserMsgs = *(usermsg_t ***)(address + 0x1);

	DWORD address = (DWORD)g_pMetaSave->pEngineFuncs->pfnHookUserMsg;

	address = (DWORD)g_pMetaHookAPI->SearchPattern((void *)address, 0x50, "\xE8\x2A\x2A\x2A\x2A\x83\xC4", sizeof("\xE8\x2A\x2A\x2A\x2A\x83\xC4") - 1);

	address = GetCallAddress(address);

	address = (DWORD)g_pMetaHookAPI->SearchPattern((void *)address, 0x20, "\x8B\x35", sizeof("\x8B\x35") - 1);

	gClientUserMsgs = *(usermsg_t ***)(address + 0x2);
}

usermsg_t *MSG_FindUserMsgHook(char *szMsgName)
{
	for (usermsg_t *msg = *gClientUserMsgs; msg; msg = msg->next)
	{
		if (!strcmp(msg->name, szMsgName))
			return msg;
	}

	return NULL;
}

usermsg_t *MSG_FindUserMsgHookPrev(char *szMsgName)
{
	for (usermsg_t *msg = (*gClientUserMsgs)->next; msg->next; msg = msg->next)
	{
		if (!strcmp(msg->next->name, szMsgName))
			return msg;
	}

	return NULL;
}

pfnUserMsgHook MSG_HookUserMsg(char *szMsgName, pfnUserMsgHook pfn)
{
	usermsg_t *msg = MSG_FindUserMsgHook(szMsgName);

	if (msg)
	{
		pfnUserMsgHook result = msg->function;
		msg->function = pfn;
		return result;
	}

	gEngfuncs.pfnHookUserMsg(szMsgName, pfn);
	return pfn;
}

pfnUserMsgHook MSG_UnHookUserMsg(char *szMsgName)
{
	usermsg_t *msg = MSG_FindUserMsgHook(szMsgName);

	if (msg)
	{
		usermsg_t *prev = MSG_FindUserMsgHookPrev(szMsgName);

		if (prev)
		{
			prev->next = msg->next;
			return msg->function;
		}
	}

	return NULL;
}

void MSG_BeginWrite(void)
{
	gMsgFunction = NULL;
	gMsgDataSize = 0;
}

void MSG_BeginWrite(char *szMsgName)
{
	gMsgFunction = NULL;
	gMsgDataSize = 0;
	gMsgName = szMsgName;

	usermsg_t *msg = MSG_FindUserMsgHook(szMsgName);

	if (msg)
		gMsgFunction = msg->function;

	if (!gMsgFunction)
	{
		Con_DPrintf("MSG_BeginWrite: %s no found!\n", szMsgName);
		return;
	}
}

void MSG_WriteChar(int iValue)
{
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue;
}

void MSG_WriteByte(int iValue)
{
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue;
}

void MSG_WriteShort(int iValue)
{
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue & 0xFF;
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue >> 8;
}

void MSG_WriteWord(int iValue)
{
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue & 0xFF;
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue >> 8;
}

void MSG_WriteLong(int iValue)
{
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue & 0xFF;
	*(byte *)(&gMsgData[gMsgDataSize++]) = (iValue >> 8) & 0xFF;
	*(byte *)(&gMsgData[gMsgDataSize++]) = (iValue >> 16) & 0xFF;
	*(byte *)(&gMsgData[gMsgDataSize++]) = iValue >> 24;
}

void MSG_WriteFloat(float flValue)
{
	union
	{
		byte b[4];
		float f;
		int l;
	}
	dat;

	dat.f = flValue;

	*(byte *)(&gMsgData[gMsgDataSize++]) = dat.b[0];
	*(byte *)(&gMsgData[gMsgDataSize++]) = dat.b[1];
	*(byte *)(&gMsgData[gMsgDataSize++]) = dat.b[2];
	*(byte *)(&gMsgData[gMsgDataSize++]) = dat.b[3];
}

void MSG_WriteString(const char *sz)
{
	if (!sz)
	{
		MSG_WriteChar('\0');
		return;
	}

	strcpy((char *)&gMsgData[gMsgDataSize], sz);
	gMsgDataSize += strlen(sz) + 1;
}

void MSG_WriteAngle(float flValue)
{
	MSG_WriteByte(((int)(flValue * 256 / 360)) & 0xFF);
}

void MSG_WriteHiresAngle(float flValue)
{
	MSG_WriteShort(((int)(flValue * 65535 / 360)) & 0xFFFF);
}

void MSG_EndWrite(void)
{
	if (!gMsgFunction)
		return;

	gMsgFunction(gMsgName, (int)gMsgDataSize, gMsgData);
	gMsgFunction = NULL;
}

void MSG_EndWrite(const char *pszName, pfnUserMsgHook pfn)
{
	pfn(pszName, (int)gMsgDataSize, gMsgData);
}