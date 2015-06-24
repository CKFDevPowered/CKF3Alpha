#include "EngineInterface.h"
#include "OptionsSubKeyboard.h"
#include "VControlsListPanel.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/QueryBox.h>

#include <vgui/Cursor.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "tier1/KeyValues.h"
#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>
#include <vgui/ISystem.h>
#include <vgui/IInput.h>

#include "FileSystem.h"
#include "tier1/UtlBuffer.h"
#include "igameuifuncs.h"
#include <vstdlib/IKeyValuesSystem.h>
#include "tier2/tier2.h"

#include <keydefs.h>

using namespace vgui;

int ConvertVGUIToEngine(KeyCode code)
{
	switch (code)
	{
		case KEY_0: return '0';
		case KEY_1: return '1';
		case KEY_2: return '2';
		case KEY_3: return '3';
		case KEY_4: return '4';
		case KEY_5: return '5';
		case KEY_6: return '6';
		case KEY_7: return '7';
		case KEY_8: return '8';
		case KEY_9: return '9';
		case KEY_A: return 'a';
		case KEY_B: return 'b';
		case KEY_C: return 'c';
		case KEY_D: return 'd';
		case KEY_E: return 'e';
		case KEY_F: return 'f';
		case KEY_G: return 'g';
		case KEY_H: return 'h';
		case KEY_I: return 'i';
		case KEY_J: return 'j';
		case KEY_K: return 'k';
		case KEY_L: return 'l';
		case KEY_M: return 'm';
		case KEY_N: return 'n';
		case KEY_O: return 'o';
		case KEY_P: return 'p';
		case KEY_Q: return 'q';
		case KEY_R: return 'r';
		case KEY_S: return 's';
		case KEY_T: return 't';
		case KEY_U: return 'u';
		case KEY_V: return 'v';
		case KEY_W: return 'w';
		case KEY_X: return 'x';
		case KEY_Y: return 'y';
		case KEY_Z: return 'z';
		case KEY_PAD_0: return K_KP_INS;
		case KEY_PAD_1: return K_KP_END;
		case KEY_PAD_2: return K_KP_DOWNARROW;
		case KEY_PAD_3: return K_KP_PGDN;
		case KEY_PAD_4: return K_KP_LEFTARROW;
		case KEY_PAD_5: return K_KP_5;
		case KEY_PAD_6: return K_KP_RIGHTARROW;
		case KEY_PAD_7: return K_KP_HOME;
		case KEY_PAD_8: return K_KP_UPARROW;
		case KEY_PAD_9: return K_KP_PGUP;
		case KEY_PAD_DIVIDE: return K_KP_SLASH;
		case KEY_PAD_MINUS: return K_KP_MINUS;
		case KEY_PAD_PLUS: return K_KP_PLUS;
		case KEY_PAD_ENTER: return K_KP_ENTER;
		case KEY_PAD_DECIMAL: return K_KP_DEL;
		case KEY_PAD_MULTIPLY: return '*';
		case KEY_LBRACKET: return '[';
		case KEY_RBRACKET: return ']';
		case KEY_SEMICOLON: return ';';
		case KEY_APOSTROPHE: return '\'';
		case KEY_BACKQUOTE: return '`';
		case KEY_COMMA: return ',';
		case KEY_PERIOD: return '.';
		case KEY_SLASH: return '/';
		case KEY_BACKSLASH: return '\\';
		case KEY_MINUS: return '-';
		case KEY_EQUAL: return '=';
		case KEY_ENTER: return K_ENTER;
		case KEY_SPACE: return K_SPACE;
		case KEY_BACKSPACE: return K_BACKSPACE;
		case KEY_TAB: return K_TAB;
		case KEY_CAPSLOCK: return K_CAPSLOCK;
		case KEY_ESCAPE: return K_ESCAPE;
		case KEY_INSERT: return K_INS;
		case KEY_DELETE: return K_DEL;
		case KEY_HOME: return K_HOME;
		case KEY_END: return K_END;
		case KEY_PAGEUP: return K_PGUP;
		case KEY_PAGEDOWN: return K_PGDN;
		case KEY_BREAK: return K_PAUSE;
		case KEY_LSHIFT: return K_SHIFT;
		case KEY_RSHIFT: return K_SHIFT;
		case KEY_LALT: return K_ALT;
		case KEY_RALT: return K_ALT;
		case KEY_LCONTROL: return K_CTRL;
		case KEY_RCONTROL: return K_CTRL;
		case KEY_UP: return K_UPARROW;
		case KEY_LEFT: return K_LEFTARROW;
		case KEY_DOWN: return K_DOWNARROW;
		case KEY_RIGHT: return K_RIGHTARROW;
		case KEY_F1: return K_F1;
		case KEY_F2: return K_F2;
		case KEY_F3: return K_F3;
		case KEY_F4: return K_F4;
		case KEY_F5: return K_F5;
		case KEY_F6: return K_F6;
		case KEY_F7: return K_F7;
		case KEY_F8: return K_F8;
		case KEY_F9: return K_F9;
		case KEY_F10: return K_F10;
		case KEY_F11: return K_F11;
		case KEY_F12: return K_F12;

		case KEY_NUMLOCK:
		case KEY_LWIN:
		case KEY_RWIN:
		case KEY_APP:
		case KEY_SCROLLLOCK:
		case KEY_CAPSLOCKTOGGLE:
		case KEY_NUMLOCKTOGGLE:
		case KEY_SCROLLLOCKTOGGLE: return 0;
	}

	return 0;
}

static const char *GetButtonName(int buttons)
{
	if (buttons & 1)
		return "MOUSE1";
	else if (buttons & 2)
		return "MOUSE2";
	else if (buttons & 4)
		return "MOUSE3";
	else if (buttons & 8)
		return "MOUSE4";
	else if (buttons & 16)
		return "MOUSE5";

	return "MOUSE1";
}

COptionsSubKeyboard::COptionsSubKeyboard(vgui::Panel *parent) : PropertyPage(parent, NULL)
{
	memset(m_Bindings, 0, sizeof(m_Bindings));

	CreateKeyBindingList();
	SaveCurrentBindings();
	ParseActionDescriptions();

	m_pSetBindingButton = new Button(this, "ChangeKeyButton", "");
	m_pClearBindingButton = new Button(this, "ClearKeyButton", "");

	LoadControlSettings("Resource/OptionsSubKeyboard.res");

	m_pSetBindingButton->SetEnabled(false);
	m_pClearBindingButton->SetEnabled(false);
}

COptionsSubKeyboard::~COptionsSubKeyboard(void)
{
	DeleteSavedBindings();
}

void COptionsSubKeyboard::OnResetData(void)
{
	FillInCurrentBindings();

	if (IsVisible())
		m_pKeyBindList->SetSelectedItem(0);
}

void COptionsSubKeyboard::OnApplyChanges(void)
{
	ApplyAllBindings();
}

void COptionsSubKeyboard::CreateKeyBindingList(void)
{
	m_pKeyBindList = new VControlsListPanel(this, "listpanel_keybindlist");
}

void COptionsSubKeyboard::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (m_pKeyBindList->IsCapturing())
	{
		if (code != KEY_ENTER)
		{
			int key = ConvertVGUIToEngine(code);
			Finish(key, 0);
		}

		return;
	}
	else
	{
		if (code == KEY_ENTER)
		{
			PostMessage(this, new KeyValues("Command", "command", "ChangeKey"));
			return;
		}
	}

	BaseClass::OnKeyCodeTyped(code);
}

void COptionsSubKeyboard::OnCommand(const char *command)
{
	if (!stricmp(command, "Defaults"))
	{
		QueryBox *box = new QueryBox("#GameUI_KeyboardSettings", "#GameUI_KeyboardSettingsText");
		box->AddActionSignalTarget(this);
		box->SetOKCommand(new KeyValues("Command", "command", "DefaultsOK"));
		box->DoModal();
	}
	else if (!stricmp(command, "DefaultsOK"))
	{
		FillInDefaultBindings();
		m_pKeyBindList->RequestFocus();
	}
	else if (!m_pKeyBindList->IsCapturing() && !stricmp(command, "ChangeKey"))
	{
		m_pKeyBindList->StartCaptureMode(dc_blank);
	}
	else if (!m_pKeyBindList->IsCapturing() && !stricmp(command, "ClearKey"))
	{
		OnKeyCodePressed(KEY_DELETE);
		m_pKeyBindList->RequestFocus();
	}
	else
		BaseClass::OnCommand(command);
}

#include "common.h"

char *UTIL_Parse(char *data, char *token, int sizeofToken)
{
	data = COM_ParseFile(data, token, sizeofToken);
	return data;
}

char *UTIL_CopyString(char const *in)
{
	char *out = new char[strlen(in) + 1];
	strcpy(out, in);
	return out;
}

char *UTIL_va(char *format, ...)
{
	va_list argptr;
	static char string[4][1024];
	static int curstring = 0;

	curstring = (curstring + 1) % 4;

	va_start(argptr, format);
	_vsnprintf(string[curstring], 1024, format, argptr);
	va_end(argptr);

	return string[curstring];
}

#define SCRIPTS_DIR "gfx/shell"

void COptionsSubKeyboard::ParseActionDescriptions(void)
{
	char szFileName[_MAX_PATH];
	char szBinding[256];
	char szDescription[256];

	sprintf(szFileName, "%s/kb_act.lst", SCRIPTS_DIR);

	FileHandle_t fh = g_pFullFileSystem->Open(szFileName, "rb");

	if (fh == FILESYSTEM_INVALID_HANDLE)
		return;

	int size = g_pFullFileSystem->Size(fh);
	CUtlBuffer buf(0, size, CUtlBuffer::TEXT_BUFFER);
	g_pFullFileSystem->Read(buf.Base(), size, fh);
	g_pFullFileSystem->Close(fh);

	char *data = (char *)buf.Base();

	int sectionIndex = 0;
	char token[512];

	while (1)
	{
		data = UTIL_Parse(data, token, sizeof(token));

		if (strlen(token) <= 0)
			break;

		strcpy(szBinding, token);
		data = UTIL_Parse(data, token, sizeof(token));

		if (strlen(token) <= 0)
			break;

		strcpy(szDescription, token);

		if (szDescription[0] != '=')
		{
			if (!stricmp(szBinding, "blank"))
			{
				m_pKeyBindList->AddSection(++sectionIndex, szDescription);
				m_pKeyBindList->AddColumnToSection(sectionIndex, "Action", szDescription, SectionedListPanel::COLUMN_BRIGHT, 226);
				m_pKeyBindList->AddColumnToSection(sectionIndex, "Key", "#GameUI_KeyButton", SectionedListPanel::COLUMN_BRIGHT, 128);
				m_pKeyBindList->AddColumnToSection(sectionIndex, "AltKey", "#GameUI_Alternate", SectionedListPanel::COLUMN_BRIGHT, 128);
			}
			else
			{
				KeyValues *item = new KeyValues("Item");

				item->SetString("Action", szDescription);
				item->SetString("Binding", szBinding);
				item->SetString("Key", "");
				item->SetString("AltKey", "");

				m_pKeyBindList->AddItem(sectionIndex, item);
				item->deleteThis();
			}
		}
	}
}

KeyValues *COptionsSubKeyboard::GetItemForBinding(const char *binding)
{
	static int bindingSymbol = KeyValuesSystem()->GetSymbolForString("Binding");

	for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
	{
		KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));

		if (!item)
			continue;

		KeyValues *bindingItem = item->FindKey(bindingSymbol);
		const char *bindString = bindingItem->GetString();

		if (!stricmp(bindString, binding))
			return item;
	}

	return NULL;
}

const char *COptionsSubKeyboard::GetKeyName(int keynum)
{
	return gameuifuncs->Key_NameForKey(keynum);
}

int COptionsSubKeyboard::FindKeyForName(char const *keyname)
{
	for (int i = 0; i < 256; i++)
	{
		char const *name = GetKeyName(i);

		if (!name || !name[0])
			continue;

		if (strlen(keyname) == 1)
		{
			if (!strcmp(keyname, name))
				return i;
		}
		else
		{
			if (!stricmp(keyname, name))
				return i;
		}
	}

	return -1;
}

void COptionsSubKeyboard::AddBinding(KeyValues *item, const char *keyname)
{
	if (!stricmp(item->GetString("Key", ""), keyname))
		return;

	RemoveKeyFromBindItems(keyname);

	item->SetString("AltKey", item->GetString("Key", ""));
	item->SetString("Key", keyname);
}

void COptionsSubKeyboard::ClearBindItems(void)
{
	m_KeysToUnbind.RemoveAll();

	for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
	{
		KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));

		if (!item)
			continue;

		item->SetString("Key", "");
		item->SetString("AltKey", "");
		m_pKeyBindList->InvalidateItem(i);
	}

	m_pKeyBindList->InvalidateLayout();
}

void COptionsSubKeyboard::RemoveKeyFromBindItems(const char *key)
{
	Assert(key && key[0]);

	if (!key || !key[0])
		return;

	for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
	{
		KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));

		if (!item)
			continue;

		if (!stricmp(key, item->GetString("AltKey", "")))
		{
			item->SetString("AltKey", "");
			m_pKeyBindList->InvalidateItem(i);
		}

		if (!stricmp(key, item->GetString("Key", "")))
		{
			item->SetString("Key", "");
			m_pKeyBindList->InvalidateItem(i);

			const char *alt = item->GetString("AltKey", "");

			if (alt && alt[0])
			{
				item->SetString("Key", alt);
				item->SetString("AltKey", "");
			}
		}
	}

	m_pKeyBindList->InvalidateLayout();
}

void COptionsSubKeyboard::FillInCurrentBindings(void)
{
	ClearBindItems();

	for (int i = 0; i < 256; i++)
	{
		const char *binding = gameuifuncs->Key_BindingForKey(i);

		if (!binding)
			continue;

		KeyValues *item = GetItemForBinding(binding);

		if (item)
		{
			const char *keyName = GetKeyName(i);
			AddBinding(item, keyName);
			m_KeysToUnbind.AddToTail(keyName);
		}
	}
}

void COptionsSubKeyboard::DeleteSavedBindings(void)
{
	for (int i = 0; i < 256; i++)
	{
		if (m_Bindings[i].binding)
			delete [] m_Bindings[i].binding;

		m_Bindings[i].binding = NULL;
	}
}

void COptionsSubKeyboard::SaveCurrentBindings(void)
{
	DeleteSavedBindings();

	for (int i = 0; i < 256; i++)
	{
		const char *binding = gameuifuncs->Key_BindingForKey(i);

		if (!binding || !binding[0])
			continue;

		m_Bindings[i].binding = UTIL_CopyString(binding);
	}
}

void COptionsSubKeyboard::BindKey(const char *key, const char *binding)
{
	engine->pfnClientCmd(UTIL_va("bind \"%s\" \"%s\"\n", key, binding));
}

void COptionsSubKeyboard::UnbindKey(const char *key)
{
	engine->pfnClientCmd(UTIL_va("unbind \"%s\"\n", key));
}

void COptionsSubKeyboard::ApplyAllBindings(void)
{
	for (int i = 0; i < m_KeysToUnbind.Count(); i++)
		UnbindKey(m_KeysToUnbind[i].String());

	m_KeysToUnbind.RemoveAll();
	DeleteSavedBindings();

	for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
	{
		KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));

		if (!item)
			continue;

		const char *binding = item->GetString("Binding", "");

		if (!binding || !binding[0])
			continue;

		const char *keyname = item->GetString("Key", "");

		if (keyname && keyname[0])
		{
			BindKey(keyname, binding);
			int bindIndex = FindKeyForName(keyname);

			if (bindIndex != -1)
				m_Bindings[bindIndex].binding = UTIL_CopyString(binding);
		}

		keyname = item->GetString("AltKey", "");

		if (keyname && keyname[0])
		{
			BindKey(keyname, binding);
			int bindIndex = FindKeyForName(keyname);

			if (bindIndex != -1)
				m_Bindings[bindIndex].binding = UTIL_CopyString(binding);
		}
	}
}

void COptionsSubKeyboard::FillInDefaultBindings(void)
{
	char szFileName[_MAX_PATH];
	sprintf(szFileName, "%s/kb_def.lst", SCRIPTS_DIR);

	FileHandle_t fh = g_pFullFileSystem->Open(szFileName, "rb");

	if (fh == FILESYSTEM_INVALID_HANDLE)
		return;

	int size = g_pFullFileSystem->Size(fh);

	CUtlBuffer buf(0, size, CUtlBuffer::TEXT_BUFFER);
	g_pFullFileSystem->Read(buf.Base(), size, fh);
	g_pFullFileSystem->Close(fh);

	ClearBindItems();

	char *data = (char *)buf.Base();
	KeyValues *item;

	while (1)
	{
		char szKeyName[256];
		data = UTIL_Parse(data, szKeyName, sizeof(szKeyName));

		if (strlen(szKeyName) <= 0)
			break;

		char szBinding[256];
		data = UTIL_Parse(data, szBinding, sizeof(szBinding));

		if (strlen(szBinding) <= 0)
			break;

		item = GetItemForBinding(szBinding);

		if (item)
			AddBinding(item, szKeyName);
	}

	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	item = GetItemForBinding("toggleconsole");

	if (item)
		AddBinding(item, "`");

	item = GetItemForBinding("cancelselect");

	if (item)
		AddBinding(item, "ESCAPE");
}

void COptionsSubKeyboard::ItemSelected(int itemID)
{
	m_pKeyBindList->SetItemOfInterest(itemID);

	if (m_pKeyBindList->IsItemIDValid(itemID))
	{
		m_pSetBindingButton->SetEnabled(true);
		KeyValues *kv = m_pKeyBindList->GetItemData(itemID);

		if (kv)
		{
			const char *key = kv->GetString("Key", NULL);

			if (key && *key)
				m_pClearBindingButton->SetEnabled(true);
			else
				m_pClearBindingButton->SetEnabled(false);

			if (kv->GetInt("Header"))
				m_pSetBindingButton->SetEnabled(false);
		}
	}
	else
	{
		m_pSetBindingButton->SetEnabled(false);
		m_pClearBindingButton->SetEnabled(false);
	}
}

void COptionsSubKeyboard::Finish(int key, int button, const char *pchKeyName)
{
	int r = m_pKeyBindList->GetItemOfInterest();
	m_pKeyBindList->EndCaptureMode(dc_arrow);
	KeyValues *item = m_pKeyBindList->GetItemData(r);

	if (item)
	{
		if (pchKeyName)
		{
			AddBinding(item, pchKeyName);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
		else if (button != 0)
		{
			AddBinding(item, GetButtonName(button));
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
		else if (key != 0 && key != 27)
		{
			AddBinding(item, GetKeyName(key));
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}

		m_pKeyBindList->InvalidateItem(r);
	}

	m_pSetBindingButton->SetEnabled(true);
	m_pClearBindingButton->SetEnabled(true);
}

void COptionsSubKeyboard::OnMousePressed(vgui::MouseCode code)
{
	if (m_pKeyBindList->IsCapturing())
	{
		switch (code)
		{
			case MOUSE_LEFT:
			{
				Finish(0, 1);
				break;
			}

			case MOUSE_RIGHT:
			{
				Finish(0, 2);
				break;
			}

			case MOUSE_MIDDLE:
			{
				Finish(0, 4);
				break;
			}

			case MOUSE_4:
			{
				Finish(0, 8);
				break;
			}

			case MOUSE_5:
			{
				Finish(0, 16);
				break;
			}
		}

		return;
	}

	BaseClass::OnMousePressed(code);
}

void COptionsSubKeyboard::OnMouseWheeled(int delta)
{
	if (m_pKeyBindList->IsCapturing())
	{
		if (delta > 0)
		{
			for (int i = 0; i < 256; i++)
			{
				const char *pchKeyName = gameuifuncs->Key_BindingForKey(i);

				if (pchKeyName)
				{
					if (pchKeyName[0] && !stricmp(pchKeyName, "MWHEELUP"))
					{
						Finish(i, 0, "MWHEELUP");
						break;
					}
				}
			}

			Finish(-1, 0, "MWHEELUP");
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				const char *pchKeyName = gameuifuncs->Key_BindingForKey(i);

				if (pchKeyName)
				{
					if (pchKeyName[0] && !stricmp(pchKeyName, "MWHEELDOWN"))
					{
						Finish(i, 0, "MWHEELDOWN");
						break;
					}
				}
			}

			Finish(-1, 0, "MWHEELDOWN");
		}

		return;
	}

	BaseClass::OnMouseWheeled(delta);
}

void COptionsSubKeyboard::OnKeyCodePressed(vgui::KeyCode code)
{
	if (!m_pKeyBindList->IsCapturing())
	{
		int r = m_pKeyBindList->GetItemOfInterest();
		int x, y, w, h;
		bool visible = m_pKeyBindList->GetCellBounds(r, 1, x, y, w, h);

		if (visible)
		{
			if (code == KEY_ENTER)
			{
				PostMessage(this, new KeyValues("Command", "command", "ChangeKey"));
				return;
			}
			else if (code == KEY_DELETE)
			{
				KeyValues *kv = m_pKeyBindList->GetItemData(r);
				const char *altkey = kv->GetString("AltKey", NULL);

				if (altkey && *altkey)
					RemoveKeyFromBindItems(altkey);

				const char *key = kv->GetString("Key", NULL);

				if (key && *key)
					RemoveKeyFromBindItems(key);

				m_pClearBindingButton->SetEnabled(false);
				m_pKeyBindList->InvalidateItem(r);
				return;
			}
		}
	}

	BaseClass::OnKeyCodePressed(code);
}