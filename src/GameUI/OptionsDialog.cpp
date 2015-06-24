#include "EngineInterface.h"

#include "BasePanel.h"
#include "OptionsDialog.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"

#include "KeyValues.h"
#include "OptionsSubKeyboard.h"
#include "OptionsSubMouse.h"
#include "OptionsSubAudio.h"
#include "OptionsSubVideo.h"
#include "OptionsSubVoice.h"
#include "OptionsSubMultiplayer.h"
#include "ModInfo.h"

using namespace vgui;

COptionsDialog::COptionsDialog(vgui::Panel *parent) : PropertyDialog(parent, "OptionsDialog")
{
	SetBounds(0, 0, 512, 406);
	SetSizeable(false);

	SetTitle("#GameUI_Options", true);

	m_pOptionsSubMultiplayer = NULL;
	m_pOptionsSubKeyboard = NULL;
	m_pOptionsSubMouse = NULL;
	m_pOptionsSubAudio = NULL;
	m_pOptionsSubVideo = NULL;
	m_pOptionsSubVoice = NULL;

	if ((ModInfo().IsMultiplayerOnly() && !ModInfo().IsSinglePlayerOnly()) || (!ModInfo().IsMultiplayerOnly() && !ModInfo().IsSinglePlayerOnly()))
		m_pOptionsSubMultiplayer = new COptionsSubMultiplayer(this);

	m_pOptionsSubKeyboard = new COptionsSubKeyboard(this);
	m_pOptionsSubMouse = new COptionsSubMouse(this);
	m_pOptionsSubAudio = new COptionsSubAudio(this);
	m_pOptionsSubVideo = new COptionsSubVideo(this);
	m_pOptionsSubVoice = new COptionsSubVoice(this);

	m_pOptionsSubMultiplayer->MakeReadyForUse();
	m_pOptionsSubKeyboard->MakeReadyForUse();
	m_pOptionsSubMouse->MakeReadyForUse();
	m_pOptionsSubAudio->MakeReadyForUse();
	m_pOptionsSubVideo->MakeReadyForUse();
	m_pOptionsSubVoice->MakeReadyForUse();

	AddPage(m_pOptionsSubMultiplayer, "#GameUI_Multiplayer");
	AddPage(m_pOptionsSubKeyboard, "#GameUI_Keyboard");
	AddPage(m_pOptionsSubMouse, "#GameUI_Mouse");
	AddPage(m_pOptionsSubAudio, "#GameUI_Audio");
	AddPage(m_pOptionsSubVideo, "#GameUI_Video");

	if (!ModInfo().IsSinglePlayerOnly())
		AddPage(m_pOptionsSubVoice, "#GameUI_Voice");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(84);
}

COptionsDialog::~COptionsDialog(void)
{
}

void COptionsDialog::Activate(void)
{
	BaseClass::Activate();

	if (m_pOptionsSubMultiplayer)
	{
		if (GetActivePage() != m_pOptionsSubMultiplayer)
			GetPropertySheet()->SetActivePage(m_pOptionsSubMultiplayer);
	}
	else
	{
		if (GetActivePage() != m_pOptionsSubKeyboard)
			GetPropertySheet()->SetActivePage(m_pOptionsSubKeyboard);
	}

	ResetAllData();
	EnableApplyButton(false);
}

void COptionsDialog::Run(void)
{
	SetTitle("#GameUI_Options", true);
	Activate();
}

void COptionsDialog::OnClose(void)
{
	BaseClass::OnClose();
}

void COptionsDialog::OnGameUIHidden(void)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);

		if (pChild)
			PostMessage(pChild, new KeyValues("GameUIHidden"));
	}
}