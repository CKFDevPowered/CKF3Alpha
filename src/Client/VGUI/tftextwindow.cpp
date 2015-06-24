#include "hud.h"
#include "cl_util.h"
#include "tftextwindow.h"
#include "tfmapinfomenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <tier1/byteswap.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ScalableImagePanel.h>

#include "configs.h"

using namespace vgui;

CTFTextWindow::CTFTextWindow(void) : BaseClass()
{
	SetProportional(true);
	SetMoveable(false);
	SetSizeable(false);

	SetCloseButtonVisible(false);
	SetTitleBarVisible(false);

	SetPaintBackgroundEnabled(false);

	SetScheme("ClientScheme");

	m_pTitleLabel = new Label(this, "Title", "");
	m_pMessageLabel = new TFRichText(this, "Message");

	LoadControlSettings("Resource/UI/TFTextWindow.res");
}

CTFTextWindow::~CTFTextWindow(void)
{
}

void CTFTextWindow::Update(void)
{
	m_pMessageLabel->SetVisible(false);

	BaseClass::Update();

	m_pOK->RequestFocus();
}

void CTFTextWindow::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	Update();
}

void CTFTextWindow::OnCommand( const char *command )
{
	if ( !Q_strcmp( command, "okay" ) )
	{
		g_pViewPort->ShowPanel( this, false );
		g_pViewPort->ShowPanel( PANEL_MAPINFO, true );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CTFTextWindow::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	InvalidateLayout(false, true);

	if (bShow)
	{
		SetVisible(true);
		MoveToFront();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

void CTFTextWindow::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_SPACE || code == KEY_ENTER)
	{
		OnCommand("okay");
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

void CTFTextWindow::ShowMOTD(char *title, char *string)
{
	Q_strncpy(m_szMessage, string, sizeof(m_szMessage));
	SetData(TYPE_INDEX, g_pViewPort->GetServerName(), m_szMessage, NULL);
	ShowPanel(true);
}

void CTFTextWindow::ShowText(const char *text)
{
	m_pMessageLabel->SetVisible(true);
	m_pMessageLabel->SetText(text);
	m_pMessageLabel->GotoTextStart();
}

void CTFTextWindow::Init(void)
{
	m_szMessage[0] = '\0';
}