#include "hud.h"
#include "teammenu.h"
#include "ClientScoreBoardDialog.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <tier1/byteswap.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>

#include "configs.h"
#include "game_controls/mouseoverpanelbutton.h"

using namespace vgui;

const char *GetStringTeamColor(int i)
{
	switch (i)
	{
		case 0: return "team0";
		case 1: return "team1";
		case 2: return "team2";
		case 3: return "team3";
		case 4:
		default: return "team4";
	}
}

CTeamMenu::CTeamMenu(void) : Frame(NULL, PANEL_TEAM)
{
	m_iJumpKey = KEY_NONE;
	m_iScoreBoardKey = KEY_NONE;

	SetTitle("", true);
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ClassInfo");
	m_pMapInfo = new RichText(this, "MapInfo");
#if defined (ENABLE_HTML_WINDOW)
	m_pMapInfoHTML = new HTML(this, "MapInfoHTML");
#endif
	LoadControlSettings("Resource/UI/TeamMenu.res", "GAME");
	InvalidateLayout();

	m_szMapName[0] = 0;
}

CTeamMenu::~CTeamMenu(void)
{
}

MouseOverPanelButton *CTeamMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

Panel *CTeamMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		m_mouseoverButtons.AddToTail(newButton);
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName(controlName);
	}
}

void CTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pMapInfo->SetFgColor(pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)));

	if (*m_szMapName)
		LoadMapPage(m_szMapName);
}

void CTeamMenu::AutoAssign(void)
{
	engine->pfnClientCmd("jointeam 5");

	OnClose();
}

void CTeamMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();

		if(!IsMouseInputEnabled())
			SetMouseInputEnabled( true );

		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			if (i == 0)
				m_mouseoverButtons[i]->ShowPage();
			else
				m_mouseoverButtons[i]->HidePage();
		}

		if (m_iJumpKey == KEY_NONE)
			m_iJumpKey = gameuifuncs->GetVGUI2KeyCodeForBind("jump");

		if (m_iScoreBoardKey == KEY_NONE)
			m_iScoreBoardKey = gameuifuncs->GetVGUI2KeyCodeForBind("showscores");
	}
	else
	{
		SetVisible(false);
		//SetMouseInputEnabled(false);
	}

	g_pViewPort->ShowBackGround(bShow);
}

void CTeamMenu::Update(void)
{
	char mapname[32];
	Q_FileBase(engine->pfnGetLevelName(), mapname, sizeof(mapname));

	SetLabelText("mapname", mapname);
	LoadMapPage(mapname);
}

void CTeamMenu::LoadMapPage(const char *mapName)
{
	Q_strncpy(m_szMapName, mapName, strlen(mapName) + 1);

	char mapRES[MAX_PATH];
	Q_snprintf(mapRES, sizeof(mapRES), "maps/%s.txt", mapName);

	if (!g_pFullFileSystem->FileExists(mapRES))
	{
		if (g_pFullFileSystem->FileExists("maps/default.txt"))
		{
			Q_snprintf(mapRES, sizeof(mapRES), "maps/default.txt");
		}
		else
		{
			m_pMapInfo->SetText("");
			return;
		}
	}

	FileHandle_t f = g_pFullFileSystem->Open(mapRES, "r");

	int fileSize = g_pFullFileSystem->Size(f);
	int dataSize = fileSize + sizeof(wchar_t);

	if (dataSize % 2)
		++dataSize;

	wchar_t *memBlock = (wchar_t *)malloc(dataSize);
	int bytesRead = g_pFullFileSystem->Read(memBlock, fileSize, f);

	if (bytesRead < fileSize)
	{
		char *data = reinterpret_cast<char *>(memBlock);
		data[bytesRead] = 0;
		data[bytesRead + 1] = 0;
	}

	memBlock[dataSize / sizeof(wchar_t) - 1] = 0x0000;

	CByteswap byteSwap;
	byteSwap.SetTargetBigEndian(false);
	byteSwap.SwapBufferToTargetEndian(memBlock, memBlock, dataSize / sizeof(wchar_t));

	if (memBlock[0] != 0xFEFF)
		m_pMapInfo->SetText(reinterpret_cast<char *>(memBlock));
	else
		m_pMapInfo->SetText(memBlock + 1);

	m_pMapInfo->GotoTextStart();

	g_pFullFileSystem->Close(f);
	free(memBlock);

	InvalidateLayout();
	Repaint();
}

void CTeamMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CTeamMenu::OnKeyCodePressed(KeyCode code)
{
	if (m_iJumpKey != KEY_NONE && m_iJumpKey == code)
	{
		AutoAssign();
	}
	else if (m_iScoreBoardKey != KEY_NONE && m_iScoreBoardKey == code)
	{
		g_pViewPort->ShowPanel(PANEL_SCOREBOARD, true);
		g_pViewPort->PostMessageToPanel(PANEL_SCOREBOARD, new KeyValues("PollHideCode", "code", code));
	}
	else
		BaseClass::OnKeyCodePressed(code);
}

void CTeamMenu::Reset(void)
{
	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *pPanel = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (pPanel)
			pPanel->HidePage();
	}

	for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		m_mouseoverButtons[i]->HidePage();
}