#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "cstrikechatdialog.h"

using namespace vgui;

CCSChatDialog::CCSChatDialog(void) : BaseClass()
{
	SetProportional(true);
	SetSizeable(false);
}

void CCSChatDialog::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("Resource/UI/ChatDialog.res");
	GetPos(m_iSaveX, m_iSaveY);
	SetVisible(false);
}

void CCSChatDialog::CreateChatInputLine(void)
{
	//BaseClass::CreateChatInputLine();
	m_pChatInput = new CChatDialogInputLine(this, "ChatInputLine");
	m_pChatInput->SetVisible(false);
}

void CCSChatDialog::CreateChatLines(void)
{
	//BaseClass::CreateChatLines();
	m_ChatLine = new CChatDialogLine(this, "ChatLine");
	m_ChatLine->SetVisible(false);
}

void CCSChatDialog::Init(void)
{
	BaseClass::Init();
}

void CCSChatDialog::VidInit(void)
{
	BaseClass::VidInit();
}

void CCSChatDialog::Reset(void)
{
	BaseClass::Reset();
}

void CCSChatDialog::OnThink(void)
{
	if (g_iUser1 || gEngfuncs.IsSpectateOnly())
	{
		int bottomBarHeight = g_pViewPort->GetSpectatorBottomBarHeight();
		SetPos(m_iSaveX, ScreenHeight - bottomBarHeight - GetWide() + (ScreenHeight - m_iSaveY));
	}
	else
		SetPos(m_iSaveX, m_iSaveY);

	if (gHUD.m_iIntermission && IsMouseInputEnabled())
		StopMessageMode();

	BaseClass::OnThink();
}

int CCSChatDialog::GetChatInputOffset(void)
{
	if (m_pChatInput->IsVisible())
		return m_iFontHeight;

	return 0;
}

void CCSChatDialog::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}