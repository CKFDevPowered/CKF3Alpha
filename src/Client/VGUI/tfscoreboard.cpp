#include "hud.h"
#include "hud_util.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "perf_counter.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>
#include <cl_entity.h>

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/SectionedListPanel.h>
#include <game_controls/TFHudPanel.h>

#include "tfscoreboard.h"

#include "CounterStrikeViewport.h"
#include "client.h"
#include "Configs.h"

#include <ICKFClient.h>

using namespace vgui;

static const char *g_sClassName[] = {
	"scout",
	"heavy",
	"soldier",
	"pyro",
	"sniper",
	"medic",
	"engineer",
	"demoman",	
	"spy"
};

static const char *g_sTeamNameDialogVariables[] = {
	"",
	"redteamname",
	"blueteamname",
	"",
};

static const char *g_sTeamScoreDialogVariables[] = {
	"",
	"redteamscore",
	"blueteamscore",
	"",
};

static const char *g_sTeamPlayerCountDialogVariables[] = {
	"",
	"redteamplayercount",
	"blueteamplayercount",
	"",
};

CTFScoreBoardDialog::CTFScoreBoardDialog(void) : CClientScoreBoardDialog()
{
	m_pRedPlayerList = new SectionedListPanel(this, "RedPlayerList");
	m_pBluePlayerList = new SectionedListPanel(this, "BluePlayerList");

	m_pPingLabel_T = new Label(this, "TPlayerLatencyLabel", "");
	m_pPingLabel_CT = new Label(this, "CTPlayerLatencyLabel", "");

	m_pMainPanel = new TFHudPanel(this, "MainPanel");
	m_pBlueTeamPanel = new TFHudPanel(this, "BlueTeamPanel");
	m_pRedTeamPanel = new TFHudPanel(this, "RedTeamPanel");

	m_pBlueTeamLabel = new Label(this, "BlueTeamLabel", "");
	m_pBlueTeamScoreLabel[0] = new Label(this, "BlueTeamScoreLabel", "");
	m_pBlueTeamScoreLabel[1] = new Label(this, "BlueTeamScoreLabelDropShadow", "");
	m_pBlueTeamPlayerCountLabel = new Label(this, "BlueTeamPlayerCountLabel", "");
	m_pRedTeamLabel = new Label(this, "RedTeamLabel", "");
	m_pRedTeamScoreLabel[0] = new Label(this, "RedTeamScoreLabel", "");
	m_pRedTeamScoreLabel[1] = new Label(this, "RedTeamScoreLabelDropShadow", "");
	m_pRedTeamPlayerCountLabel = new Label(this, "RedTeamPlayerCountLabel", "");
	m_pVerticalLine = new ImagePanel(this, "VerticalLine");
	m_pShadedBar = new ImagePanel(this, "ShadedBar");
	m_pSpectators = new Label(this, "Spectators", "");
	m_pClassImage = new ImagePanel(this, "ClassImage");
	m_pPlayerNameLabel = new Label(this, "PlayerNameLabel", "");
	m_pHorizontalLine = new ImagePanel(this, "HorizontalLine");
	m_pPlayerScoreLabel = new Label(this, "PlayerScoreLabel", "");
	m_pLocalPlayerStatsPanel = new EditablePanel(this, "LocalPlayerStatsPanel");
	m_pServerLabel = new Label(this, "ServerLabel", "");

	SetVisible(false);
	SetPaintBackgroundEnabled(false);

	LoadControlSettings("Resource/UI/TFScoreBoard.res");

	SetServerName(gConfigs.szGameName);
}

CTFScoreBoardDialog::~CTFScoreBoardDialog(void)
{
}

void CTFScoreBoardDialog::SetServerName(const char *hostname)
{
	wchar_t wzServerLabel[256], wzServerName[64];
	g_pVGuiLocalize->ConvertANSIToUnicode(hostname, wzServerName, sizeof(wzServerName));
	g_pVGuiLocalize->ConstructString(wzServerLabel, sizeof(wzServerLabel), g_pVGuiLocalize->Find("#Scoreboard_Server"), 1, wzServerName);
	SetDialogVariable("server", wzServerLabel);

	//if (IsVisible())
	//	Update();
}

void CTFScoreBoardDialog::SetMapName(const char *mapname)
{
	SetDialogVariable("mapname", mapname);

	//if (IsVisible())
	//	Update();
}

void CTFScoreBoardDialog::ShowPanel(bool bShow)
{
	if (gHUD.m_iIntermission && !bShow)
		return;

	BaseClass::ShowPanel(bShow);
}

void CTFScoreBoardDialog::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CTFScoreBoardDialog::PaintBorder(void)
{
	BaseClass::PaintBorder();
}

void CTFScoreBoardDialog::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetProportional(false);
	SetVisible(false);
	SetBgColor(Color(0, 0, 0, 0));

	char imageName[64];
	if ( m_pImageList )
	{
		m_iImageDead = m_pImageList->AddImage( scheme()->GetImage( "gfx/vgui/scoreboard/leaderboard_dead", true ) );
		m_iImageDominated = m_pImageList->AddImage( scheme()->GetImage( "gfx/vgui/scoreboard/leaderboard_dominated", true ) );
		m_iImageNemesis = m_pImageList->AddImage( scheme()->GetImage( "gfx/vgui/scoreboard/leaderboard_nemesis", true ) );
		
		for (int i = 0; i < 9; i++ )
		{
			Q_snprintf(imageName, sizeof(imageName), "gfx/vgui/scoreboard/leaderboard_class_%s", g_sClassName[i]);
			m_iImageClassIcon[i] = m_pImageList->AddImage( scheme()->GetImage( imageName , true ) );
		}
		for (int i = 0; i < 16; i++ )
		{
			Q_snprintf(imageName, sizeof(imageName), "gfx/vgui/scoreboard/leaderboard_dom%d", i+1);
			m_iImageDominates[i] = m_pImageList->AddImage( scheme()->GetImage( imageName , true ) );
		}		
		// resize the images to our resolution
		for (int i = 1; i < m_pImageList->GetImageCount(); i++ )
		{
			int wide = 13, tall = 13;
			int w, h;
			w = scheme()->GetProportionalScaledValueEx( GetScheme(), wide );
			h = scheme()->GetProportionalScaledValueEx( GetScheme(), tall );
			m_pImageList->GetImage(i)->SetSize(w, h);
		}
	}
	for (int i = 0; i < 2; i++ )
	{
		for(int j = 0; j < 9; ++j)
		{
			Q_snprintf(imageName, sizeof(imageName), "resource/tga/class_%s_%s", g_sClassName[j], (!i) ? "red" : "blu");
			m_pImageClassImage[i][j] = scheme()->GetImage( imageName , true );
		}
	}

	if (m_pRedPlayerList)
	{
		m_pRedPlayerList->SetImageList(m_pImageList, false);
		m_pRedPlayerList->SetBgColor(Color(0, 0, 0, 0));
		m_pRedPlayerList->SetBorder(NULL);
	}

	if (m_pBluePlayerList)
	{
		m_pBluePlayerList->SetImageList(m_pImageList, false);
		m_pBluePlayerList->SetBgColor(Color(0, 0, 0, 0));
		m_pBluePlayerList->SetBorder(NULL);
	}

	if (m_pPlayerList)
		m_pPlayerList->SetVisible(false);

	InitPlayerList(m_pRedPlayerList, TEAM_RED);
	InitPlayerList(m_pBluePlayerList, TEAM_BLUE);
}

void CTFScoreBoardDialog::Init(void)
{

}

void CTFScoreBoardDialog::VidInit(void)
{
}

void CTFScoreBoardDialog::Reset(void)
{
	const char *mapname = gEngfuncs.pfnGetLevelName();

	if (strlen(mapname) > 5)
	{
		char newname[32];
		strcpy(newname, &mapname[5]);
		newname[strlen(newname) - 4] = 0;
		SetMapName(newname);
	}
	else
		SetMapName(mapname);
}

bool CTFScoreBoardDialog::CSPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
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

	int iPlayerIndex1 = it1->GetInt("playerIndex");
	int iPlayerIndex2 = it2->GetInt("playerIndex");

	return (iPlayerIndex1 > iPlayerIndex2);
}

void CTFScoreBoardDialog::InitPlayerList(SectionedListPanel *pPlayerList, int teamNumber)
{
	pPlayerList->SetVerticalScrollbar(false);
	pPlayerList->RemoveAll();
	pPlayerList->RemoveAllSections();
	pPlayerList->AddSection(0, "Players", CSPlayerSortFunc);
	pPlayerList->SetSectionAlwaysVisible(0, true);
	
	vgui::IScheme *ischeme = scheme()->GetIScheme(GetScheme());
	pPlayerList->SetSectionFgColor(0, ischeme->GetColor("HudWhite", Color(255, 255, 255, 255)));
	pPlayerList->SetSectionDividerColor(0, ischeme->GetColor("HudWhite", Color(255, 255, 255, 255)));
	pPlayerList->SetFontSection(0, ischeme->GetFont("ScoreboardColumns"));
	pPlayerList->SetBgColor(Color(0, 0, 0, 0));
	pPlayerList->SetBorder(NULL);

	pPlayerList->AddColumnToSection(0, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth);
	pPlayerList->AddColumnToSection(0, "status", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iStatusWidth );
	pPlayerList->AddColumnToSection(0, "name", "#CKF3_ScoreBoard_Name", 0, m_iNameWidth);	
	pPlayerList->AddColumnToSection(0, "dominate", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iDominateWidth );
	pPlayerList->AddColumnToSection(0, "class", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iClassWidth);
	pPlayerList->AddColumnToSection(0, "nemesis", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iNemesisWidth );
	pPlayerList->AddColumnToSection(0, "frags", "#CKF3_ScoreBoard_Score", SectionedListPanel::COLUMN_CENTER, m_iScoreWidth);
	pPlayerList->AddColumnToSection(0, "deaths", "#CKF3_ScoreBoard_Death", SectionedListPanel::COLUMN_CENTER, m_iDeathWidth);
	pPlayerList->AddColumnToSection(0, "ping", "#CKF3_ScoreBoard_Ping", SectionedListPanel::COLUMN_CENTER, m_iPingWidth);
}

void CTFScoreBoardDialog::UpdateRoundTimer(void)
{
	int timer = (int)(gHUD.m_flTimeLeft - gHUD.m_flTime);

	if(timer < 0) timer = 0;

	int hours = (timer / 3600);
	int mins = (timer % 3600) / 60;
	int secs = (timer % 60);

	char szText[48];
	if(hours)
		sprintf(szText, "%0d:%02d:%02d", hours, mins, secs);
	else
		sprintf(szText, "%02d:%02d", mins, secs);
	
	wchar_t wzClockTimer[48];
	wchar_t wzClockLabel[128];
	g_pVGuiLocalize->ConvertANSIToUnicode(szText, wzClockTimer, sizeof(wzClockTimer));
	g_pVGuiLocalize->ConstructString(wzClockLabel, sizeof(wzClockLabel), g_pVGuiLocalize->Find("#Scoreboard_TimeLeft"), 1, wzClockTimer);

	if(timer == 0)
	{
		wchar_t wzClockLabel2[128];
		_swprintf(wzClockLabel2, L"%s%s", g_pVGuiLocalize->Find("#Scoreboard_ChangeOnRoundEnd"), wzClockLabel);
		SetDialogVariable("servertimeleft", wzClockLabel2);
	}
	else
	{
		SetDialogVariable("servertimeleft", wzClockLabel);
	}
}

void CTFScoreBoardDialog::Update(void)
{
	UpdateTeamInfo();
	UpdatePlayerList();
	UpdateSpectatorList();
	UpdateRoundTimer();
	UpdatePlayerDetails();

	if (NeedsUpdate())
		m_fNextUpdateTime = g_pViewPort->GetCurrentTime() + 1.0f;
}

void CTFScoreBoardDialog::UpdateTeamInfo(void)
{
	for (int teamIndex = TEAM_TERRORIST; teamIndex <= TEAM_CT; teamIndex++)
	{
		wchar_t *teamName = NULL;
		int teamCounts = GetTeamCounts(teamIndex);
		int teamAliveCounts = GetTeamAliveCounts(teamIndex);

		SectionedListPanel *pTeamList = NULL;
		Label *pPingLabel = NULL;

		wchar_t name[64];
		wchar_t string1[1024];

		if (!teamName)
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(g_pViewPort->GetTeamName(teamIndex), name, sizeof(name));
			teamName = name;
		}

		switch (teamIndex)
		{
			case TEAM_RED:
			{
				teamName = g_pVGuiLocalize->Find("#CKF3_ScoreBoard_RedTeam");

				pTeamList = m_pRedPlayerList;
				pPingLabel = m_pPingLabel_T;

				_swprintf(string1, L"%s %d/%d", g_pVGuiLocalize->Find("#Cstrike_Scoreboard_PlayersAlive"), teamAliveCounts, teamCounts);
				break;
			}

			case TEAM_CT:
			{
				teamName = g_pVGuiLocalize->Find("#CKF3_ScoreBoard_BlueTeam");

				pTeamList = m_pBluePlayerList;
				pPingLabel = m_pPingLabel_CT;

				_swprintf(string1, L"%d/%d %s", teamAliveCounts, teamCounts, g_pVGuiLocalize->Find("#Cstrike_Scoreboard_PlayersAlive"));
				break;
			}
		}

		int pingsum = 0;
		int numcounted = 0;

		for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
		{
			if (IsConnected(playerIndex) && g_PlayerExtraInfo[playerIndex].teamnumber == teamIndex)
			{
				int ping = g_PlayerInfoList[playerIndex].ping;

				if (ping >= 1)
				{
					pingsum += ping;
					numcounted++;
				}
			}
		}

		if (numcounted > 0)
		{
			int ping = (int)((float)pingsum / (float)numcounted);

			_swprintf(string1, L"%d", ping);
			pPingLabel->SetText(string1);
		}
		else
		{
			pPingLabel->SetText("");
		}

		SetDialogVariable(g_sTeamNameDialogVariables[teamIndex], teamName);

		SetDialogVariable(g_sTeamScoreDialogVariables[teamIndex], g_pViewPort->GetTeamScore(teamIndex));

		wchar_t wzPlayerCount[16];
		_swprintf(wzPlayerCount, L"%d", teamCounts);
		g_pVGuiLocalize->ConstructString(string1, sizeof(string1), g_pVGuiLocalize->Find((teamCounts == 1) ? "#CKF3_ScoreBoard_PlayerNum" : "#CKF3_ScoreBoard_PlayersNum"), 1, wzPlayerCount);
		SetDialogVariable(g_sTeamPlayerCountDialogVariables[teamIndex], string1);
	}
}

void CTFScoreBoardDialog::UpdatePlayerList(void)
{
	m_pRedPlayerList->RemoveAll();
	m_pBluePlayerList->RemoveAll();

	for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
	{
		if (IsConnected(playerIndex))
		{
			SectionedListPanel *pPlayerList = NULL;

			switch (gCKFVars.g_PlayerInfo[playerIndex].iTeam)
			{
				case TEAM_RED:
				{
					pPlayerList = m_pRedPlayerList;
					break;
				}

				case TEAM_BLUE:
				{
					pPlayerList = m_pBluePlayerList;
					break;
				}
			}

			if (pPlayerList == NULL)
			{
				continue;
			}

			KeyValues *pKeyValues = new KeyValues("data");
			GetPlayerScoreInfo(playerIndex, pKeyValues);

			int itemID = pPlayerList->AddItem(0, pKeyValues);

			Color clr = g_pViewPort->GetTeamColor(gCKFVars.g_PlayerInfo[playerIndex].iTeam);
			pPlayerList->SetItemFgColor(itemID, clr);

			if (playerIndex == gHUD.m_iPlayerNum)
				pPlayerList->SetItemBgColor(itemID, Color(64, 64, 64, 64));

			pKeyValues->deleteThis();
		}
	}
}

void CTFScoreBoardDialog::UpdateSpectatorList(void)
{
	char szSpectatorList[512] = "";
	int nSpectators = 0;

	for (int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++)
	{
		if (ShouldShowAsSpectator(playerIndex))
		{
			if (nSpectators > 0)
				Q_strncat(szSpectatorList, ", ", ARRAYSIZE(szSpectatorList));

			Q_strncat(szSpectatorList, g_PlayerInfoList[playerIndex].name, ARRAYSIZE(szSpectatorList));
			nSpectators++;
		}
	}

	wchar_t wzSpectators[512] = L"";

	if (nSpectators > 0)
	{
		const char *pchFormat = (1 == nSpectators ? "#CKF3_ScoreBoard_Spectator" : "#CKF3_ScoreBoard_Spectators");

		wchar_t wzSpectatorCount[16];
		wchar_t wzSpectatorList[1024];
		_snwprintf(wzSpectatorCount, ARRAYSIZE(wzSpectatorCount), L"%i", nSpectators);
		g_pVGuiLocalize->ConvertANSIToUnicode(szSpectatorList, wzSpectatorList, sizeof(wzSpectatorList));
		g_pVGuiLocalize->ConstructString(wzSpectators, sizeof(wzSpectators), g_pVGuiLocalize->Find(pchFormat), 2, wzSpectatorCount, wzSpectatorList);

		SetDialogVariable("spectators", wzSpectators);
	}
	else
	{
		SetDialogVariable("spectators", g_pVGuiLocalize->Find("#CKF3_Scoreboard_NoSpectators"));
	}
}

bool CTFScoreBoardDialog::ShouldShowAsSpectator(int iPlayerIndex)
{
	int iTeam = (gCKFVars.g_PlayerInfo[iPlayerIndex].iTeam);//g_PlayerExtraInfo[iPlayerIndex].teamnumber;

	if (IsConnected(iPlayerIndex))
	{
		if (iTeam == TEAM_SPECTATOR || iTeam == TEAM_UNASSIGNED)
			return true;
	}

	return false;
}

bool CTFScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	hud_player_info_t info;
	engine->pfnGetPlayerInfo(playerIndex, &info);

	if (info.name == NULL || info.name[0] == '\0')
		return false;

	/*const char *oldName = info.name;
	int bufsize = strlen(oldName) * 2 + 1;
	char *newName = (char *)_alloca(bufsize);
	gViewPortInterface->MakeSafeName(oldName, newName, bufsize);*/

	kv->SetInt("status", 0);
	kv->SetString("name", info.name);
	kv->SetInt("nemesis", 0);
	kv->SetInt("class", 0);
	kv->SetInt("dominate", 0);
	kv->SetInt("frags", gCKFVars.g_PlayerInfo[playerIndex].iFrags);
	kv->SetInt("deaths", gCKFVars.g_PlayerInfo[playerIndex].iDeaths);
	if (info.ping <= 0)
	{
		const char *bot = engine->PlayerInfo_ValueForKey(playerIndex, "*bot");

		if (bot && atoi(bot) > 0)
			kv->SetString("ping", "BOT");
		else
			kv->SetString("ping", "HOST");
	}
	else
	{
		kv->SetInt("ping", info.ping);
	}

	UpdatePlayerAvatar(playerIndex, kv);
	UpdatePlayerClass(playerIndex, kv);
	return true;
}

void CTFScoreBoardDialog::UpdatePlayerClass(int playerIndex, KeyValues *kv)
{
	/*const char *hltv = engine->PlayerInfo_ValueForKey(playerIndex, "*hltv");

	if (hltv && atoi(hltv) > 0)
	{
		char numspecs[32];
		Q_snprintf(numspecs, sizeof(numspecs), "%i Spectators", m_HLTVSpectators);
		kv->SetString("class", numspecs);
	}*/
	if (gCKFVars.g_PlayerInfo[playerIndex].iTeam != TEAM_SPECTATOR && gCKFVars.g_PlayerInfo[playerIndex].iTeam != TEAM_UNASSIGNED)
	{
		//we are in ally team
		if ((*gCKFVars.g_iTeam) == gCKFVars.g_PlayerInfo[playerIndex].iTeam || (*gCKFVars.g_iTeam) == TEAM_SPECTATOR)
		{
			int iClassIndex = gCKFVars.g_PlayerInfo[playerIndex].iClass;
			if(iClassIndex >= CLASS_SCOUT && iClassIndex <= CLASS_SPY)
			{
				kv->SetInt("class", m_iImageClassIcon[iClassIndex - 1]);
			}
		}
		//show nemesis / dominated
		else
		{
			int iNemesis = gCKFVars.g_PlayerInfo[playerIndex].iDominateList[gHUD.m_iPlayerNum];
			int iDominate = gCKFVars.g_PlayerInfo[gHUD.m_iPlayerNum].iDominateList[playerIndex];
			if(iNemesis)
			{
				kv->SetInt("nemesis", m_iImageNemesis);
			}
			else if(iDominate)
			{
				kv->SetInt("nemesis", m_iImageDominated);
			}
		}

		if(gCKFVars.g_PlayerInfo[playerIndex].bIsDead)
		{
			kv->SetInt("status", m_iImageDead);
		}
		if(gCKFVars.g_PlayerInfo[playerIndex].iDominates)
		{
			int iDomIndex = min(gCKFVars.g_PlayerInfo[playerIndex].iDominates-1, 15);
			kv->SetInt("dominate", m_iImageDominates[iDomIndex]);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates details about a player
//-----------------------------------------------------------------------------
void CTFScoreBoardDialog::UpdatePlayerDetails(void)
{
	ClearPlayerDetails();
	
	int playerIndex = gHUD.m_iPlayerNum;

	hud_player_info_t info;
	engine->pfnGetPlayerInfo(playerIndex, &info);

	if (info.name == NULL || info.name[0] == '\0')
		return;

	const char *hltv = engine->PlayerInfo_ValueForKey(playerIndex, "*hltv");

	if (hltv && atoi(hltv) > 0)
	{
		SetDialogVariable( "playername", info.name );
		return;
	}

	m_pLocalPlayerStatsPanel->SetDialogVariable( "kills", gCKFVars.g_PlayerStats->iKill );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "deaths", gCKFVars.g_PlayerStats->iDeath );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "assists", gCKFVars.g_PlayerStats->iAssist );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "destruction", gCKFVars.g_PlayerStats->iDemolish );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "captures", gCKFVars.g_PlayerStats->iCapture );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "defenses", gCKFVars.g_PlayerStats->iDefence );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "dominations", gCKFVars.g_PlayerStats->iDominate );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "revenge", gCKFVars.g_PlayerStats->iRevenge );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "healing", gCKFVars.g_PlayerStats->iRevenge );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "invulns", gCKFVars.g_PlayerStats->iUbercharge );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "teleports", gCKFVars.g_PlayerStats->iTeleport );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "headshots", gCKFVars.g_PlayerStats->iHeadshot );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "backstabs", gCKFVars.g_PlayerStats->iBackstab );
	m_pLocalPlayerStatsPanel->SetDialogVariable( "bonus", gCKFVars.g_PlayerStats->iBonus );
	
	SetDialogVariable( "playername", info.name );
	SetDialogVariable( "playerscore", gCKFVars.g_PlayerInfo[playerIndex].iFrags );

	Color clr = g_pViewPort->GetTeamColor( (*gCKFVars.g_iTeam) );
	m_pPlayerNameLabel->SetFgColor( clr );
	m_pHorizontalLine->SetFillColor( clr );

	int iClass = (*gCKFVars.g_iDesiredClass);
	int iTeam = (*gCKFVars.g_iTeam);
	if ( ( iTeam >= TEAM_RED && iTeam <= TEAM_BLUE ) && ( iClass >= CLASS_SCOUT ) && ( iClass <= CLASS_SPY ) )
	{
		m_pClassImage->SetImage(m_pImageClassImage[iTeam-1][iClass-1]);
		m_pClassImage->SetVisible( true );
	}
	else
	{
		m_pClassImage->SetVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Clears score details
//-----------------------------------------------------------------------------
void CTFScoreBoardDialog::ClearPlayerDetails(void)
{
	m_pClassImage->SetVisible( false );

	int playerIndex = gHUD.m_iPlayerNum;

	const char *hltv = engine->PlayerInfo_ValueForKey(playerIndex, "*hltv");

	// HLTV has no game stats
	bool bVisible = true;
	if (hltv && atoi(hltv) > 0)
	{
		bVisible = false;
	}

	m_pLocalPlayerStatsPanel->SetDialogVariable( "kills", "" ); 
	m_pLocalPlayerStatsPanel->SetControlVisible( "KillsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "deaths", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "DeathsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "captures", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "CapturesLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "defenses", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "DefensesLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "dominations", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "DominationLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "revenge", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "RevengeLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "assists", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "AssistsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "destruction", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "DestructionLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "healing", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "HealingLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "invulns", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "InvulnLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "teleports", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "TeleportsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "headshots", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "HeadshotsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "backstabs", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "BackstabsLabel", bVisible );

	m_pLocalPlayerStatsPanel->SetDialogVariable( "bonus", "" );
	m_pLocalPlayerStatsPanel->SetControlVisible( "BonusLabel", bVisible );

	SetDialogVariable( "playername", "" );

	SetDialogVariable( "playerscore", "" );	
}