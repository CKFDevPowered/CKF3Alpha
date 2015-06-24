#include "hud.h"
#include "BuyMenu.h"
#include "BuySubMenu.h"
#include "mouseoverpanelbutton.h"

using namespace vgui;

CBuyMenu::CBuyMenu(void) : WizardPanel(NULL, PANEL_BUY)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetTitleBarVisible(false);
	SetAutoDelete(false);

	LoadControlSettings("Resource/UI/BuyMenu.res", "GAME");
	ShowButtons(false);

	m_pMainMenu = new CBuySubMenu(this, "mainmenu");
	m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res", "GAME");
	m_pMainMenu->SetVisible(false);
}

CBuyMenu::~CBuyMenu(void)
{
	if (m_pMainMenu)
		m_pMainMenu->DeleteSubPanels();
}

void CBuyMenu::Init(void)
{
}

void CBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Update();
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	g_pViewPort->ShowBackGround(bShow);
}

void CBuyMenu::Update(void)
{
	NULL;
}

void CBuyMenu::OnClose(void)
{
	BaseClass::OnClose();

	ResetHistory();

	g_pViewPort->ShowBackGround(false);
}