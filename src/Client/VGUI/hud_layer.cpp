#include "cl_dll.h"
#include <vgui_controls/Panel.h>
#include <VGUI/ISurface.h>
#include "ViewPort.h"
#include "parsemsg.h"
#include "cdll_dll.h"
#include "player.h"
#include "vgui_int.h"

#include "CSBackGroundPanel.h"
#include "CounterStrikeViewport.h"

using namespace vgui;

#include "hud_layer.h"
#include "hud_menu.h"
#include "hud_radar.h"

CHudLayer::CHudLayer(Panel *parent) : Panel(parent, "HudLayer")
{
	int swide, stall;
	surface()->GetScreenSize(swide, stall);

	SetScheme("ClientScheme");
	SetBounds(0, 0, swide, stall);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	m_Panels.RemoveAll();
}

CHudLayer::~CHudLayer(void)
{
	RemoveAllPanels();
}

void CHudLayer::Start(void)
{
	m_pMenu = (CHudMenu *)AddNewPanel(new CHudMenu);
	m_pRadar = (CHudRadar *)AddNewPanel(new CHudRadar);

	SetVisible(false);
}

void CHudLayer::SetParent(VPANEL parent)
{
	Panel::SetParent(parent);

	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->SetParent(this);

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
}

void CHudLayer::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

void CHudLayer::Init(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->Init();
}

void CHudLayer::VidInit(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->VidInit();
}

void CHudLayer::Reset(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->Reset();
}

void CHudLayer::Think(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
	{
		if (!m_Panels[i]->IsVisible())
			m_Panels[i]->Think();
	}
}

CHudElement *CHudLayer::AddNewPanel(CHudElement *pPanel, char const *pchDebugName)
{
	if (!pPanel)
	{
		if (pchDebugName)
			DevMsg("CHudLayer::AddNewPanel(%s): NULL panel.\n", pchDebugName);
		else
			Assert(0);

		delete pPanel;
		return NULL;
	}

	m_Panels.AddToTail(pPanel);

	ipanel()->SetVisible(pPanel->GetVPanel(), false);
	pPanel->SetParent(this);
	return pPanel;
}

void CHudLayer::RemoveAllPanels(void)
{
	for (int i = 0; i < m_Panels.Count(); i++)
	{
		VPANEL vPanel = m_Panels[i]->GetVPanel();
		ipanel()->DeletePanel(vPanel);
	}

	m_Panels.Purge();
}

bool CHudLayer::SelectMenuItem(int menu_item)
{
	return m_pMenu->SelectMenuItem(menu_item);
}

void CHudLayer::UpdateMapSprite(void)
{
	m_pRadar->UpdateMapSprite();
}

void CHudLayer::RenderMapSprite(void)
{
	m_pRadar->RenderMapSprite();
}

void CHudLayer::CalcRefdef(struct ref_params_s *pparams)
{
	m_pRadar->CalcRefdef(pparams);
}

int CHudLayer::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	for (int i = 0; i < m_Panels.Count(); i++)
	{
		if (m_Panels[i]->FireMessage(pszName, iSize, pbuf))
			return 1;
	}

	return 0;
}