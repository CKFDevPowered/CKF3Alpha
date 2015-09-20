#include "hud.h"
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
//#include "hud_radar.h"
#include "hud_playerstatus.h"
#include "hud_ammostatus.h"
#include "hud_mediccharge.h"
#include "hud_demopipes.h"
#include "hud_cloakmeter.h"
#include "hud_metalmeter.h"
#include "hud_targetid.h"

CHudLayer::CHudLayer(Panel *parent) : EditablePanel(parent, "HudLayer")
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
	m_pMenu = (CHudNewMenu *)AddNewPanel(new CHudNewMenu);
	//m_pRadar = (CHudNewRadar *)AddNewPanel(new CHudNewRadar);
	m_pPlayerStatus = (CTFHudPlayerStatus *)AddNewPanel(new CTFHudPlayerStatus);
	m_pWeaponAmmo = (CTFHudWeaponAmmo *)AddNewPanel(new CTFHudWeaponAmmo);
	m_pMedicChargeMeter = (CTFHudMedicChargeMeter *)AddNewPanel(new CTFHudMedicChargeMeter);
	m_pDemomanPipes = (CTFHudDemomanPipes *)AddNewPanel(new CTFHudDemomanPipes);
	m_pCloakMeter = (CTFHudCloakMeter *)AddNewPanel(new CTFHudCloakMeter);
	m_pMetalMeter = (CTFHudMetalMeter *)AddNewPanel(new CTFHudMetalMeter);

	m_pMainTargetID = (CMainTargetID *)AddNewPanel(new CMainTargetID);
	m_pSpectatorTargetID = (CSpectatorTargetID *)AddNewPanel(new CSpectatorTargetID);
	m_pSedondaryTargetID = (CSecondaryTargetID *)AddNewPanel(new CSecondaryTargetID);

	// reload the .res file from disk
	LoadControlSettings( "resource/UI/HudLayout.res" );

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
		bool visible = m_Panels[i]->ShouldDraw();

		// If it's a vgui panel, hide/show as appropriate
		vgui::Panel *pPanel = dynamic_cast<vgui::Panel *>(m_Panels[i]);
		if ( pPanel && pPanel->IsVisible() != visible )
		{
			pPanel->SetVisible( visible );
		}
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
	//m_pRadar->UpdateMapSprite();
}

void CHudLayer::RenderMapSprite(void)
{
	//m_pRadar->RenderMapSprite();
}

void CHudLayer::CalcRefdef(struct ref_params_s *pparams)
{
	//m_pRadar->CalcRefdef(pparams);
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