#include <stdio.h>
#include <time.h>

#include "EngineInterface.h"
#include "CreateMultiplayerGameGameplayPage.h"

using namespace vgui;

#include <KeyValues.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>

#include "FileSystem.h"
#include "PanelListPanel.h"
#include "ScriptObject.h"
#include <tier0/vcrmode.h>

#define OPTIONS_FILE "settings.scr"

class CServerDescription : public CDescription
{
public:
	CServerDescription(CPanelListPanel *panel);

public:
	void WriteScriptHeader(FileHandle_t fp);
	void WriteFileHeader(FileHandle_t fp);
};

CCreateMultiplayerGameGameplayPage::CCreateMultiplayerGameGameplayPage(vgui::Panel *parent, const char *name) : PropertyPage(parent, name)
{
	m_pOptionsList = new CPanelListPanel(this, "GameOptions");

	m_pDescription = new CServerDescription(m_pOptionsList);
	m_pDescription->InitFromFile(OPTIONS_FILE);
	m_pList = NULL;

	LoadControlSettings("Resource/CreateMultiplayerGameGameplayPage.res");

	LoadGameOptionsList();
}

CCreateMultiplayerGameGameplayPage::~CCreateMultiplayerGameGameplayPage(void)
{
	delete m_pDescription;
}

int CCreateMultiplayerGameGameplayPage::GetMaxPlayers(void)
{
	return atoi(GetValue("maxplayers", "32"));
}

const char *CCreateMultiplayerGameGameplayPage::GetPassword(void)
{
	return GetValue("sv_password", "");
}

const char *CCreateMultiplayerGameGameplayPage::GetHostName(void)
{
	return GetValue("hostname", "Half-Life");
}

const char *CCreateMultiplayerGameGameplayPage::GetValue(const char *cvarName, const char *defaultValue)
{
	for (mpcontrol_t *mp = m_pList; mp != NULL; mp = mp->next)
	{
		Panel *control = mp->pControl;

		if (control && !stricmp(mp->GetName(), cvarName))
		{
			KeyValues *data = new KeyValues("GetText");
			static char buf[128];

			if (control && control->RequestInfo(data))
				strncpy(buf, data->GetString("text", defaultValue), sizeof(buf) - 1);
			else
				strncpy(buf, defaultValue, sizeof(buf) - 1);

			buf[sizeof(buf) - 1] = 0;
			data->deleteThis();
			return buf;
		}
	}

	return defaultValue;
}

void CCreateMultiplayerGameGameplayPage::SaveValues(void)
{
	GatherCurrentValues();

	if (m_pDescription)
	{
		FileHandle_t fp = g_pFullFileSystem->Open(OPTIONS_FILE, "wb");

		if (fp)
		{
			m_pDescription->WriteToScriptFile(fp);
			g_pFullFileSystem->Close(fp);
		}

		m_pDescription->WriteToConfig();
	}
}

void CCreateMultiplayerGameGameplayPage::LoadGameOptionsList(void)
{
	if (m_pList)
	{
		mpcontrol_t *p, *n;

		p = m_pList;

		while (p)
		{
			n = p->next;

			delete p->pControl;
			delete p->pPrompt;
			delete p;

			p = n;
		}

		m_pList = NULL;
	}

	CScriptObject *pObj = m_pDescription->pObjList;
	mpcontrol_t *pCtrl;
	CheckButton *pBox;
	TextEntry *pEdit;
	ComboBox *pCombo;
	CScriptListItem *pListItem;

	Panel *objParent = m_pOptionsList;

	while (pObj)
	{
		if (pObj->type == O_OBSOLETE)
		{
			pObj = pObj->pNext;
			continue;
		}

		pCtrl = new mpcontrol_t(objParent, pObj->cvarname);
		pCtrl->type = pObj->type;

		switch (pCtrl->type)
		{
			case O_BOOL:
			{
				pBox = new CheckButton(pCtrl, "DescCheckButton", pObj->prompt);
				pBox->SetSelected(pObj->fdefValue != 0.0f ? true : false);
				pCtrl->pControl = (Panel *)pBox;
				break;
			}

			case O_STRING:
			case O_NUMBER:
			{
				pEdit = new TextEntry(pCtrl, "DescEdit");
				pEdit->InsertString(pObj->defValue);
				pCtrl->pControl = (Panel *)pEdit;
				break;
			}

			case O_LIST:
			{
				pCombo = new ComboBox(pCtrl, "DescEdit", 5, false);
				pListItem = pObj->pListItems;

				while (pListItem)
				{
					pCombo->AddItem(pListItem->szItemText, NULL);
					pListItem = pListItem->pNext;
				}

				pCombo->ActivateItemByRow((int)pObj->fdefValue);
				pCtrl->pControl = (Panel *)pCombo;
				break;
			}

			default: break;
		}

		if (pCtrl->type != O_BOOL)
		{
			pCtrl->pPrompt = new vgui::Label(pCtrl, "DescLabel", "");
			pCtrl->pPrompt->SetContentAlignment(vgui::Label::a_west);
			pCtrl->pPrompt->SetTextInset(5, 0);
			pCtrl->pPrompt->SetText(pObj->prompt);
		}

		pCtrl->pScrObj = pObj;
		pCtrl->SetSize(100, 28);
		m_pOptionsList->AddItem(pCtrl);

		if (!m_pList)
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			mpcontrol_t *p = m_pList;

			while (p)
			{
				if (!p->next)
				{
					p->next = pCtrl;
					pCtrl->next = NULL;
					break;
				}

				p = p->next;
			}
		}

		pObj = pObj->pNext;
	}
}

void CCreateMultiplayerGameGameplayPage::GatherCurrentValues(void)
{
	if (!m_pDescription)
		return;

	CheckButton *pBox;
	TextEntry *pEdit;
	ComboBox *pCombo;

	mpcontrol_t *pList;

	CScriptObject *pObj;
	CScriptListItem *pItem;

	char szValue[256];
	char strValue[256];
	wchar_t w_szStrValue[256];

	pList = m_pList;

	while (pList)
	{
		pObj = pList->pScrObj;

		if (!pList->pControl)
		{
			pObj->SetCurValue(pObj->defValue);
			pList = pList->next;
			continue;
		}

		switch (pObj->type)
		{
			case O_BOOL:
			{
				pBox = (CheckButton *)pList->pControl;
				Q_snprintf(szValue, sizeof(szValue), "%s", pBox->IsSelected() ? "1" : "0");
				break;
			}

			case O_NUMBER:
			{
				pEdit = (TextEntry *)pList->pControl;
				pEdit->GetText(strValue, sizeof(strValue));
				Q_snprintf(szValue, sizeof(szValue), "%s", strValue);
				break;
			}

			case O_STRING:
			{
				pEdit = (TextEntry *)pList->pControl;
				pEdit->GetText(strValue, sizeof(strValue));
				Q_snprintf(szValue, sizeof(szValue), "%s", strValue);
				break;
			}

			case O_LIST:
			{
				pCombo = (ComboBox *)pList->pControl;
				pCombo->GetText(w_szStrValue, sizeof(w_szStrValue) / sizeof(wchar_t));

				pItem = pObj->pListItems;

				while (pItem)
				{
					wchar_t *wLocalizedString = NULL;
					wchar_t w_szStrTemp[256];

					if (pItem->szItemText[0] == '#')
						wLocalizedString = g_pVGuiLocalize->Find(pItem->szItemText);

					if (wLocalizedString)
						wcsncpy(w_szStrTemp, wLocalizedString, sizeof(w_szStrTemp) / sizeof(wchar_t));
					else
						g_pVGuiLocalize->ConvertANSIToUnicode(pItem->szItemText, w_szStrTemp, sizeof(w_szStrTemp));

					if (_wcsicmp(w_szStrTemp, w_szStrValue) == 0)
						break;

					pItem = pItem->pNext;
				}

				if (pItem)
					Q_snprintf(szValue, sizeof(szValue), "%s", pItem->szValue);
				else
					Q_snprintf(szValue, sizeof(szValue), "%s", pObj->defValue);

				break;
			}
		}

		UTIL_StripInvalidCharacters(szValue, sizeof(szValue));

		Q_strncpy(strValue, szValue, sizeof(strValue));

		pObj->SetCurValue(strValue);

		pList = pList->next;
	}
}

CServerDescription::CServerDescription(CPanelListPanel *panel) : CDescription(panel)
{
	setHint("// NOTE:  THIS FILE IS AUTOMATICALLY REGENERATED, \r\n//DO NOT EDIT THIS HEADER, YOUR COMMENTS WILL BE LOST IF YOU DO\r\n// Multiplayer options script\r\n//\r\n// Format:\r\n//  Version [float]\r\n//  Options description followed by \r\n//  Options defaults\r\n//\r\n// Option description syntax:\r\n//\r\n//  \"cvar\" { \"Prompt\" { type [ type info ] } { default } }\r\n//\r\n//  type = \r\n//   BOOL   (a yes/no toggle)\r\n//   STRING\r\n//   NUMBER\r\n//   LIST\r\n//\r\n// type info:\r\n// BOOL                 no type info\r\n// NUMBER       min max range, use -1 -1 for no limits\r\n// STRING       no type info\r\n// LIST          delimited list of options value pairs\r\n//\r\n//\r\n// default depends on type\r\n// BOOL is \"0\" or \"1\"\r\n// NUMBER is \"value\"\r\n// STRING is \"value\"\r\n// LIST is \"index\", where index \"0\" is the first element of the list\r\n\r\n\r\n");
	setDescription("SERVER_OPTIONS");
}

void CServerDescription::WriteScriptHeader(FileHandle_t fp)
{
	char am_pm[] = "AM";

	time_t timer = time(NULL);
	tm *tblock = localtime(&timer);
	tm newtime = *tblock;

	if (newtime.tm_hour > 12)
		Q_strncpy(am_pm, "PM", sizeof(am_pm));

	if (newtime.tm_hour > 12)
		newtime.tm_hour -= 12;

	if (newtime.tm_hour == 0)
		newtime.tm_hour = 12;

	g_pFullFileSystem->FPrintf(fp, (char *)getHint());

	g_pFullFileSystem->FPrintf(fp, "// Half-Life Server Configuration Layout Script (stores last settings chosen, too)\r\n");
	g_pFullFileSystem->FPrintf(fp, "// File generated:  %.19s %s\r\n", asctime(&newtime), am_pm);
	g_pFullFileSystem->FPrintf(fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n");
	g_pFullFileSystem->FPrintf(fp, "VERSION %.1f\r\n\r\n", SCRIPT_VERSION);
	g_pFullFileSystem->FPrintf(fp, "DESCRIPTION SERVER_OPTIONS\r\n{\r\n");
}

void CServerDescription::WriteFileHeader(FileHandle_t fp)
{
	char am_pm[] = "AM";

	time_t timer = time(NULL);
	tm *tblock = localtime(&timer);
	tm newtime = *tblock;

	if (newtime.tm_hour > 12)
		Q_strncpy(am_pm, "PM", sizeof(am_pm));

	if (newtime.tm_hour > 12)
		newtime.tm_hour -= 12;

	if (newtime.tm_hour == 0)
		newtime.tm_hour = 12;

	g_pFullFileSystem->FPrintf(fp, "// Half-Life Server Configuration Settings\r\n");
	g_pFullFileSystem->FPrintf(fp, "// DO NOT EDIT, GENERATED BY HALF-LIFE\r\n");
	g_pFullFileSystem->FPrintf(fp, "// File generated:  %.19s %s\r\n", asctime(&newtime), am_pm);
	g_pFullFileSystem->FPrintf(fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n");
}
