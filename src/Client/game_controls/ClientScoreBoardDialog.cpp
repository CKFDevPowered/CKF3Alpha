#include "hud.h"
#include "ClientScoreBoardDialog.h"
#include "cl_util.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>
#include <cl_entity.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/SectionedListPanel.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

bool AvatarIndexLessFunc(const int &lhs, const int &rhs)
{
	return lhs < rhs;
}

CClientScoreBoardDialog::CClientScoreBoardDialog(void) : BaseClass(NULL, PANEL_SCOREBOARD)
{
	m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");
	m_nCloseKey = KEY_NONE;

	TrackerImage = 0;

	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetSizeable(false);
	SetCloseButtonVisible(false);

	SetScheme("ClientScheme");

	m_pPlayerList = new SectionedListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	LoadControlSettings("Resource/UI/ScoreBoard.res", "GAME");

	m_iDesiredHeight = GetTall();
	m_pPlayerList->SetVisible(false);

	m_HLTVSpectators = 0;

	m_pImageList = NULL;

	m_mapAvatarsToImageList.SetLessFunc(AvatarIndexLessFunc);
	m_mapAvatarsToImageList.RemoveAll();
	memset(&m_iImageAvatars, 0, sizeof(m_iImageAvatars));
}

void CClientScoreBoardDialog::SetServerName(const char *hostname)
{
	Panel *control = FindChildByName("ServerName");

	if (control)
	{
		PostMessage(control, new KeyValues("SetText", "text", hostname));
		control->MoveToFront();
	}

	if (IsVisible())
		Update();
}

CClientScoreBoardDialog::~CClientScoreBoardDialog(void)
{
	if (m_pImageList)
		delete m_pImageList;
}

void CClientScoreBoardDialog::OnThink(void)
{
	BaseClass::OnThink();

	if (m_nCloseKey != KEY_NONE)
	{
		if (!input()->IsKeyDown(m_nCloseKey))
		{
			m_nCloseKey = KEY_NONE;

			g_pViewPort->ShowPanel(this, false);
		}
	}
}

void CClientScoreBoardDialog::OnPollHideCode(int code)
{
	m_nCloseKey = (KeyCode)code;
}

void CClientScoreBoardDialog::VidInit(void)
{
	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();

	m_iSectionId = 0;
	m_fNextUpdateTime = 0;

	InitScoreboardSections();
}

void CClientScoreBoardDialog::Reset(void)
{
}

void CClientScoreBoardDialog::InitScoreboardSections(void)
{
}

void CClientScoreBoardDialog::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (m_pImageList)
		delete m_pImageList;

	m_pImageList = new ImageList(false);

	m_mapAvatarsToImageList.RemoveAll();
	memset(&m_iImageAvatars, 0, sizeof(m_iImageAvatars));

	PostApplySchemeSettings(pScheme);
}

void CClientScoreBoardDialog::PostApplySchemeSettings(vgui::IScheme *pScheme)
{
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		int wide, tall;
		m_pImageList->GetImage(i)->GetSize(wide, tall);
		m_pImageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValueEx(GetScheme(), wide), scheme()->GetProportionalScaledValueEx(GetScheme(), tall));
	}

	m_pPlayerList->SetImageList(m_pImageList, false);
	m_pPlayerList->SetVisible(true);

	SetBgColor(Color(0, 0, 0, 0));
}

void CClientScoreBoardDialog::ShowPanel(bool bShow)
{
	if (m_pImageList == NULL)
	{
		InvalidateLayout(false, true);
	}

	if (!bShow)
	{
		m_nCloseKey = KEY_NONE;
	}

	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Reset();
		Update();
		SetVisible(true);
		MoveToFront();
	}
	else
	{
		SetVisible(false);
	}
}

bool CClientScoreBoardDialog::NeedsUpdate(void)
{
	return (m_fNextUpdateTime < g_pViewPort->GetCurrentTime());
}

void CClientScoreBoardDialog::Update(void)
{
	m_pPlayerList->DeleteAllItems();

	FillScoreBoard();

	int wide, tall;
	m_pPlayerList->GetContentSize(wide, tall);
	tall += GetAdditionalHeight();
	wide = GetWide();

	if (m_iDesiredHeight < tall)
	{
		SetSize(wide, tall);
		m_pPlayerList->SetSize(wide, tall);
	}
	else
	{
		SetSize(wide, m_iDesiredHeight);
		m_pPlayerList->SetSize(wide, m_iDesiredHeight);
	}

	MoveToCenterOfScreen();

	m_fNextUpdateTime = g_pViewPort->GetCurrentTime() + 1.0f;
}

void CClientScoreBoardDialog::UpdateTeamInfo(void)
{
}

void CClientScoreBoardDialog::UpdatePlayerInfo(void)
{
	m_iSectionId = 0;

	int selectedRow = -1;

	for (int i = 1; i <= gEngfuncs.GetMaxClients(); ++i)
	{
		hud_player_info_t info;
		engine->pfnGetPlayerInfo(i, &info);

		if (info.name != NULL && info.name[0] != '\0')
		{
			KeyValues *playerData = new KeyValues("data");
			GetPlayerScoreInfo(i, playerData);

			const char *oldName = playerData->GetString("name", "");
			int bufsize = strlen(oldName) * 2 + 1;
			char *newName = (char *)_alloca(bufsize);

			gViewPortInterface->MakeSafeName(oldName, newName, bufsize);
			playerData->SetString("name", newName);

			int itemID = FindItemIDForPlayerIndex(i);
			int sectionID = g_PlayerExtraInfo[i].teamnumber;

			if (i == gHUD.m_iPlayerNum)
			{
				selectedRow = itemID;
			}

			if (itemID == -1)
				itemID = m_pPlayerList->AddItem(sectionID, playerData);
			else
				m_pPlayerList->ModifyItem(itemID, sectionID, playerData);

			m_pPlayerList->SetItemFgColor(itemID, g_pViewPort->GetTeamColor(sectionID));

			playerData->deleteThis();
		}
		else
		{
			int itemID = FindItemIDForPlayerIndex(i);

			if (itemID != -1)
			{
				m_pPlayerList->RemoveItem(itemID);
			}
		}
	}

	if (selectedRow != -1)
		m_pPlayerList->SetSelectedItem(selectedRow);
}

void CClientScoreBoardDialog::AddHeader(void)
{
	m_pPlayerList->AddSection(m_iSectionId, "");
	m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
	m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "#PlayerName", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
	m_pPlayerList->AddColumnToSection(m_iSectionId, "frags", "#PlayerScore", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "#PlayerDeath", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
}

void CClientScoreBoardDialog::AddSection(int teamType, int teamNumber)
{
	if (teamType == TYPE_TEAM)
	{
		wchar_t *teamName = g_pVGuiLocalize->Find(g_pViewPort->GetTeamName(teamNumber));
		wchar_t name[64];
		wchar_t string1[1024];

		if (!teamName)
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(g_pViewPort->GetTeamName(teamNumber), name, sizeof(name));
			teamName = name;
		}

		g_pVGuiLocalize->ConstructString(string1, sizeof(string1), g_pVGuiLocalize->Find("#Player"), 2, teamName);

		m_pPlayerList->AddSection(m_iSectionId, "", StaticPlayerSortFunc);

		if (ShowAvatars())
			m_pPlayerList->AddColumnToSection(m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth);

		m_pPlayerList->AddColumnToSection(m_iSectionId, "name", string1, 0, scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH) - m_iAvatarWidth);
		m_pPlayerList->AddColumnToSection(m_iSectionId, "frags", "", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
		m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
	}
	else if (teamType == TYPE_SPECTATORS)
	{
		m_pPlayerList->AddSection(m_iSectionId, "");

		if (ShowAvatars())
			m_pPlayerList->AddColumnToSection(m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth);

		m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "#Spectators", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH) - m_iAvatarWidth);
		m_pPlayerList->AddColumnToSection(m_iSectionId, "frags", "", 0, scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
	}
}

bool CClientScoreBoardDialog::StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);
	Assert(it1 && it2);

	int v1 = it1->GetInt("frags");
	int v2 = it2->GetInt("frags");

	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	v1 = it1->GetInt("deaths");
	v2 = it2->GetInt("deaths");

	if (v1 > v2)
		return false;
	else if (v1 < v2)
		return true;

	return itemID1 < itemID2;
}

bool CClientScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	hud_player_info_t info;
	engine->pfnGetPlayerInfo(playerIndex, &info);

	if (info.name == NULL || info.name[0] == '\0')
		return false;

	kv->SetInt("deaths", g_PlayerExtraInfo[playerIndex].deaths);
	kv->SetInt("frags", g_PlayerExtraInfo[playerIndex].frags);

	if (info.ping <= 0)
	{
		const char *bot = engine->PlayerInfo_ValueForKey(playerIndex, "*bot");

		if (bot && atoi(bot) > 0)
			kv->SetString("ping", "BOT");
		else
			kv->SetString("ping", "");
	}
	else
		kv->SetInt("ping", info.ping);

	kv->SetString("name", info.name);
	return true;
}

void CClientScoreBoardDialog::UpdatePlayerAvatar(int playerIndex, KeyValues *kv)
{

}

void CClientScoreBoardDialog::UpdatePlayerClass(int playerIndex, KeyValues *kv)
{

}

void CClientScoreBoardDialog::FillScoreBoard(void)
{
	UpdateTeamInfo();
	UpdatePlayerInfo();
}

int CClientScoreBoardDialog::FindItemIDForPlayerIndex(int playerIndex)
{
	for (int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++)
	{
		if (m_pPlayerList->IsItemIDValid(i))
		{
			KeyValues *kv = m_pPlayerList->GetItemData(i);
			kv = kv->FindKey(m_iPlayerIndexSymbol);

			if (kv && kv->GetInt() == playerIndex)
				return i;
		}
	}

	return -1;
}

void CClientScoreBoardDialog::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->MoveToFront();
}

void CClientScoreBoardDialog::MoveToCenterOfScreen(void)
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}