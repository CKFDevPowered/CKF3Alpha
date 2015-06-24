#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "event_api.h"
#include "configs.h"

DECLARE_MESSAGE(m_GraphicMenu, TeamMenu)
DECLARE_MESSAGE(m_GraphicMenu, ClassMenu)
DECLARE_MESSAGE(m_GraphicMenu, BuyMenu)
DECLARE_MESSAGE(m_GraphicMenu, GraphicMenu)

int CHudGraphicMenu::Init(void)
{
	m_iFlags = 0;

	HOOK_MESSAGE(TeamMenu);
	HOOK_MESSAGE(ClassMenu);
	HOOK_MESSAGE(BuyMenu);
	HOOK_MESSAGE(GraphicMenu);

	return 1;
}

int CHudGraphicMenu::VidInit(void)
{
	return 1;
}

int CHudGraphicMenu::MsgFunc_TeamMenu(const char *pszName, int iSize, void *pbuf)
{
	const char *vgui_menus = gEngfuncs.LocalPlayerInfo_ValueForKey("_vgui_menus");

	if (gConfigs.bEnableClientUI && vgui_menus && strlen(vgui_menus) && atoi(vgui_menus) != 0)
	{
		g_pViewPort->FireMessage(pszName, iSize, pbuf);
		return 1;
	}

	return 1;
}

int CHudGraphicMenu::MsgFunc_ClassMenu(const char *pszName, int iSize, void *pbuf)
{
	const char *vgui_menus = gEngfuncs.LocalPlayerInfo_ValueForKey("_vgui_menus");

	if (gConfigs.bEnableClientUI && vgui_menus && strlen(vgui_menus) && atoi(vgui_menus) != 0)
	{
		g_pViewPort->FireMessage(pszName, iSize, pbuf);
		return 1;
	}

	return 1;
}

int CHudGraphicMenu::MsgFunc_BuyMenu(const char *pszName, int iSize, void *pbuf)
{
	const char *vgui_menus = gEngfuncs.LocalPlayerInfo_ValueForKey("_vgui_menus");

	if (gConfigs.bEnableClientUI && vgui_menus && strlen(vgui_menus) && atoi(vgui_menus) != 0)
	{
		g_pViewPort->FireMessage(pszName, iSize, pbuf);
		return 1;
	}

	return 1;
}

int CHudGraphicMenu::MsgFunc_GraphicMenu(const char *pszName, int iSize, void *pbuf)
{
	const char *vgui_menus = gEngfuncs.LocalPlayerInfo_ValueForKey("_vgui_menus");

	if (gConfigs.bEnableClientUI && vgui_menus && strlen(vgui_menus) && atoi(vgui_menus) != 0)
	{
		g_pViewPort->FireMessage(pszName, iSize, pbuf);
		return 1;
	}

	return 1;
}