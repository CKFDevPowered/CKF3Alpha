#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include <com_model.h>
#include "configs.h"

pfnUserMsgHook g_pfnMSG_ItemStatus;

DECLARE_MESSAGE(m_itemStatus, ItemStatus)

int CHudItemStatus::Init(void)
{
	g_pfnMSG_ItemStatus = HOOK_MESSAGE(ItemStatus);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudItemStatus::VidInit(void)
{
	return 1;
}

void CHudItemStatus::Reset(void)
{
}

int CHudItemStatus::MsgFunc_ItemStatus(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int flags = READ_BYTE();

	m_hasDefuser = flags & ITEMSTATE_HASDEFUSER;
	m_hasNightvision = flags & ITEMSTATE_HASNIGHTVISION;

	return g_pfnMSG_ItemStatus(pszName, iSize, pbuf);
}