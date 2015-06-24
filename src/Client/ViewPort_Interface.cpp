#include <metahook.h>
#include <vgui_controls/Panel.h>
#include <VGUI/ISurface.h>
#include <IGameUIFuncs.h>
#include "ViewPort.h"
#include "Configs.h"

IEngineVGui *enginevgui;

class CClientVGUI : public IClientVGUI
{
public:
	virtual void Initialize(CreateInterfaceFn *factories, int count);
	virtual void Start(void);
	virtual void SetParent(vgui::VPANEL parent);
	virtual bool UseVGUI1(void);
	virtual void HideScoreBoard(void);
	virtual void HideAllVGUIMenu(void);
	virtual void ActivateClientUI(void);
	virtual void HideClientUI(void);
};

IClientVGUI *g_pClientVGUI;

EXPOSE_SINGLE_INTERFACE(CClientVGUI, IClientVGUI, CLIENTVGUI_INTERFACE_VERSION);

void CClientVGUI::Initialize(CreateInterfaceFn *factories, int count)
{
	if (!vgui::VGui_InitInterfacesList("ClientUI", factories, count))
		return;

	enginevgui = (IEngineVGui *)factories[0](VENGINE_VGUI_VERSION, NULL);
	gameuifuncs = (IGameUIFuncs *)factories[0](VENGINE_GAMEUIFUNCS_VERSION, NULL);

	g_pClientVGUI = (IClientVGUI *)((CreateInterfaceFn)gExportfuncs.ClientFactory())(CLIENTVGUI_INTERFACE_VERSION, NULL);
	g_pClientVGUI->Initialize(factories, count);
}

void CClientVGUI::Start(void)
{
	g_pClientVGUI->Start();
	g_pViewPort = new CViewport();
	g_pViewPort->Start();
}

void CClientVGUI::SetParent(vgui::VPANEL parent)
{
	g_pClientVGUI->SetParent(parent);
	g_pViewPort->SetParent(parent);
}

bool CClientVGUI::UseVGUI1(void)
{
	return g_pClientVGUI->UseVGUI1();
}

void CClientVGUI::HideScoreBoard(void)
{
	g_pViewPort->HideScoreBoard();
	g_pClientVGUI->HideScoreBoard();
}

void CClientVGUI::HideAllVGUIMenu(void)
{
	g_pViewPort->HideAllVGUIMenu();
	g_pClientVGUI->HideAllVGUIMenu();
}

void CClientVGUI::ActivateClientUI(void)
{
	g_pViewPort->ActivateClientUI();
	g_pClientVGUI->HideClientUI();
}

void CClientVGUI::HideClientUI(void)
{
	g_pViewPort->HideClientUI();
	g_pClientVGUI->HideClientUI();
}