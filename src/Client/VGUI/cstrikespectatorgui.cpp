#include "hud.h"
#include "cstrikespectatorgui.h"
#include "cstrikeclientscoreboard.h"
#include "game_controls\commandmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>

#include "pm_shared.h"
#include "cdll_dll.h"
#include "shared_util.h"

CCSSpectatorGUI::CCSSpectatorGUI(void) : CSpectatorGUI()
{
	m_pCTLabel = NULL;
	m_pCTScore = NULL;
	m_pTerLabel = NULL;
	m_pTerScore = NULL;

	m_pTimer = NULL;
	m_pTimerLabel = NULL;

	m_pDivider = NULL;

	m_pExtraInfo = NULL;

	m_scoreWidth = 0;
	m_extraInfoWidth = 0;

	LoadControlSettings("Resource/UI/Spectator.res");
	InvalidateLayout();
}

void CCSSpectatorGUI::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	char text[4096];
	int temp;

	BaseClass::ApplySchemeSettings(pScheme);

	m_pCTLabel = dynamic_cast<Label *>(FindChildByName("CTScoreLabel"));
	m_pCTScore = dynamic_cast<Label *>(FindChildByName("CTScoreValue"));
	m_pTerLabel = dynamic_cast<Label *>(FindChildByName("TerScoreLabel"));
	m_pTerScore = dynamic_cast<Label *>(FindChildByName("TerScoreValue"));

	m_pTimer = dynamic_cast<Panel *>(FindChildByName("timerimage"));
	m_pTimerLabel = dynamic_cast<Label *>(FindChildByName("timerlabel"));

	m_pDivider = dynamic_cast<Panel *>(FindChildByName("DividerBar"));
	m_pExtraInfo = dynamic_cast<Label *>(FindChildByName("extrainfo"));

	m_pCTScore->GetPos(m_CTScoreX, temp);
	m_pCTLabel->GetPos(m_CTLabelX, temp);
	m_pTerScore->GetPos(m_TerScoreX, temp);
	m_pTerLabel->GetPos(m_TerLabelX, temp);

	m_pCTScore->GetText(text, sizeof(text));
	m_pCTScore->SetText("0");
	m_pCTScore->GetContentSize(m_scoreOffset, temp);
	m_pCTScore->SetText(text);

	m_scoreWidth = m_pCTScore->GetWide();
	m_extraInfoWidth = m_pExtraInfo->GetWide();
	m_scoreOffset = m_scoreWidth - m_scoreOffset;
}

void CCSSpectatorGUI::UpdateSpectatorPlayerList(void)
{
	m_nTeamScoreT = g_pViewPort->GetTeamScore(TEAM_CT);
	m_nTeamScoreCT = g_pViewPort->GetTeamScore(TEAM_TERRORIST);

	SetLabelText("CTScoreValue", SharedVarArgs("%i", m_nTeamScoreT));
	SetLabelText("TerScoreValue", SharedVarArgs("%i", m_nTeamScoreCT));
}

bool CCSSpectatorGUI::NeedsUpdate(void)
{
	if (m_nLastAccount != gHUD.m_accountBalance.m_iAccount)
		return true;

	if (m_nLastTime != (int)(gHUD.m_roundTimer.m_flTimeEnd - gHUD.m_flTime))
		return true;

	if (m_nLastSpecMode != g_iUser1)
		return true;

	if (m_nLastSpecTarget != g_iUser2)
		return true;

	if (m_nTeamScoreT != g_pViewPort->GetTeamScore(TEAM_CT) || m_nTeamScoreCT != g_pViewPort->GetTeamScore(TEAM_TERRORIST))
		return true;

	return BaseClass::NeedsUpdate();
}

void CCSSpectatorGUI::UpdateTimer(void)
{
	m_nLastTime = (int)(gHUD.m_roundTimer.m_flTimeEnd - gHUD.m_flTime);

	if (m_nLastTime < 0)
		m_nLastTime = 0;

	wchar_t szText[63];
	_snwprintf(szText, sizeof(szText), L"%d:%02d", (m_nLastTime / 60), (m_nLastTime % 60));
	szText[62] = 0;

	SetLabelText("timerlabel", szText);
}

void CCSSpectatorGUI::UpdateAccount(void)
{
	m_nLastAccount = gHUD.m_accountBalance.m_iAccount;

	if ((g_iTeamNumber == TEAM_TERRORIST) || (g_iTeamNumber == TEAM_CT))
	{
		wchar_t szText[63];
		_snwprintf(szText, sizeof(szText), L"$%i", m_nLastAccount);
		szText[62] = 0;

		SetLabelText("extrainfo", szText);
	}
}

void CCSSpectatorGUI::Update(void)
{
	BaseClass::Update();

	m_nLastSpecMode = g_iUser1;
	m_nLastSpecTarget = g_iUser2;

	UpdateTimer();
	UpdateAccount();
	UpdateSpectatorPlayerList();

	ResizeControls();
}

void CCSSpectatorGUI::StoreWidths(void)
{
	if (!ControlsPresent())
		return;
}

void CCSSpectatorGUI::ResizeControls(void)
{
	if (!ControlsPresent())
		return;

	int x1, y1, w1, t1;
	int x2, y2, w2, t2;
	int diff;

	StoreWidths();

	int wCT, hCT, wTer, hTer;
	m_pCTScore->GetBounds(x1, y1, w1, t1);
	m_pCTScore->GetContentSize(wCT, hCT);
	m_pTerScore->GetBounds(x2, y2, w2, t2);
	m_pTerScore->GetContentSize(wTer, hTer);

	int maxWidth = max(wCT, wTer) + m_scoreOffset;

	if (maxWidth > m_scoreWidth)
	{
		diff = maxWidth - m_scoreWidth;

		if (diff != 0)
		{
			m_pCTScore->GetBounds(x1, y1, w1, t1);
			m_pCTScore->SetBounds(m_CTScoreX - diff, y1, maxWidth, t1);

			m_pTerScore->GetBounds(x1, y1, w1, t1);
			m_pTerScore->SetBounds(m_TerScoreX - diff, y1, maxWidth, t1);

			m_pCTLabel->GetPos(x1, y1);
			m_pCTLabel->SetPos(m_CTLabelX - diff, y1);

			m_pTerLabel->GetPos(x1, y1);
			m_pTerLabel->SetPos(m_TerLabelX - diff, y1);
		}
	}
}

bool CCSSpectatorGUI::ControlsPresent(void) const
{
	return (m_pCTLabel != NULL && m_pCTScore != NULL && m_pTerLabel != NULL && m_pTerScore != NULL && m_pTimer != NULL && m_pTimerLabel != NULL && m_pDivider != NULL && m_pExtraInfo != NULL);
}

CCSSpectatorMenu::CCSSpectatorMenu(void) : CSpectatorMenu()
{
	LoadControlSettings("Resource/UI/BottomSpectator.res");
	SetPaintBackgroundEnabled(false);
}

void CCSSpectatorMenu::Reset(void)
{
	ShowPanel(false);
}