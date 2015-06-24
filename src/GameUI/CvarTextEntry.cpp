#include "EngineInterface.h"
#include "CvarTextEntry.h"
#include <vgui/IVGui.h>
#include "IGameUIFuncs.h"
#include "tier1/KeyValues.h"

#include <cvardef.h>

using namespace vgui;

static const int MAX_CVAR_TEXT = 64;

CCvarTextEntry::CCvarTextEntry(Panel *parent, const char *panelName, char const *cvarname) : TextEntry(parent, panelName)
{
	m_pszCvarName = cvarname ? strdup(cvarname) : NULL;
	m_pszStartValue[0] = 0;

	if (m_pszCvarName)
		Reset();

	AddActionSignalTarget(this);
}

CCvarTextEntry::~CCvarTextEntry(void)
{
	if (m_pszCvarName)
		free(m_pszCvarName);
}

void CCvarTextEntry::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (GetMaximumCharCount() < 0 || GetMaximumCharCount() > MAX_CVAR_TEXT)
		SetMaximumCharCount(MAX_CVAR_TEXT - 1);
}

void CCvarTextEntry::ApplyChanges(void)
{
	if (!m_pszCvarName)
		return;

	char szText[MAX_CVAR_TEXT];
	GetText(szText, MAX_CVAR_TEXT);

	if (!szText[0])
		return;

	char szCommand[256];
	sprintf(szCommand, "%s \"%s\"\n", m_pszCvarName, szText);
	engine->pfnClientCmd(szCommand);
	strcpy(m_pszStartValue, szText);
}

void CCvarTextEntry::Reset(void)
{
	char *value = engine->pfnGetCvarString(m_pszCvarName);

	if (value && value[0])
	{
		SetText(value);
		strcpy(m_pszStartValue, value);
	}
}

bool CCvarTextEntry::HasBeenModified(void)
{
	char szText[MAX_CVAR_TEXT];
	GetText(szText, MAX_CVAR_TEXT);
	return stricmp(szText, m_pszStartValue);
}

void CCvarTextEntry::OnTextChanged(void)
{
	if (!m_pszCvarName)
		return;

	if (HasBeenModified())
		PostActionSignal(new KeyValues("ControlModified"));
}