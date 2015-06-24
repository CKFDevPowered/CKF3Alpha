#include "EngineInterface.h"
#include "GameConsole.h"
#include "GameConsoleDialog.h"
#include "vgui/ISurface.h"

#include "KeyValues.h"
#include "vgui/VGUI.h"
#include "vgui/IInput.h"
#include "vgui/IVGUI.h"

#include "BasePanel.h"
#include "common.h"

CGameConsole g_GameConsole;
CGameConsole *g_pGameConsole = NULL;

CGameConsole &GameConsole(void)
{
	return g_GameConsole;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameConsole, IGameConsole, GAMECONSOLE_INTERFACE_VERSION, g_GameConsole);

CGameConsole::CGameConsole(void)
{
	g_pGameConsole = this;
	m_bInitialized = false;
}

CGameConsole::~CGameConsole(void)
{
	m_bInitialized = false;
}

void CGameConsole::OnCmdCondump(void)
{
	g_GameConsole.m_pConsole->DumpConsoleTextToFile();
}

void CGameConsole::Initialize(void)
{
	m_pConsole = vgui::SETUP_PANEL(new CGameConsoleDialog());

	int swide, stall;
	vgui::surface()->GetScreenSize(swide, stall);
	int offset = vgui::scheme()->GetProportionalScaledValue(16);

	m_pConsole->SetBounds(swide / 2 - (offset * 4), offset, (swide / 2) + (offset * 3), stall - (offset * 8));
	m_bInitialized = true;

	engine->pfnAddCommand("condump", OnCmdCondump);
}

void CGameConsole::Activate(void)
{
	if (!m_bInitialized)
		return;

	if (vgui::surface()->GetModalPanel() || vgui::input()->GetAppModalSurface())
		return;

	m_pConsole->Activate();
	m_pConsole->MoveToFront();
}

void CGameConsole::Hide(void)
{
	if (!m_bInitialized)
		return;

	m_pConsole->Hide();
	m_pConsole->SetVisible(false);

	vgui::surface()->CalculateMouseVisible();
}

void CGameConsole::Clear(void)
{
	if (!m_bInitialized)
		return;

	m_pConsole->Clear();
}

bool CGameConsole::IsConsoleVisible(void)
{
	if (!m_bInitialized)
		return false;

	if (vgui::surface()->GetModalPanel() || vgui::input()->GetAppModalSurface())
		return false;

	return m_pConsole->IsVisible();
}

void CGameConsole::Printf(const char *format, ...)
{
	if (!m_bInitialized)
		return;

	va_list argptr;
	static char text[4096];

	va_start(argptr, format);
	_vsnprintf(text, sizeof(text), format, argptr);
	va_end(argptr);

	m_pConsole->Print(text);
}

void CGameConsole::DPrintf(const char *format, ...)
{
	if (!m_bInitialized)
		return;

	va_list argptr;
	static char text[4096];

	va_start(argptr, format);
	_vsnprintf(text, sizeof(text), format, argptr);
	va_end(argptr);

	m_pConsole->DPrint(text);
}

void CGameConsole::SetParent(int parent)
{
	if (!m_bInitialized)
		return;

	m_pConsole->SetParent(static_cast<vgui::VPANEL>(parent));
}