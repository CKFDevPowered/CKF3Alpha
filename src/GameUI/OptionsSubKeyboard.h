#ifndef OPTIONS_SUB_KEYBOARD_H
#define OPTIONS_SUB_KEYBOARD_H

#ifdef _WIN32
#pragma once
#endif

#include "UtlVector.h"
#include "UtlSymbol.h"

#include <vgui_controls/PropertyPage.h>

class VControlsListPanel;

class COptionsSubKeyboard : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubKeyboard, vgui::PropertyPage);

public:
	COptionsSubKeyboard(vgui::Panel *parent);
	~COptionsSubKeyboard(void);

public:
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseWheeled(int delta);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnCommand(const char *command);
	virtual void OnKeyCodeTyped(vgui::KeyCode code);

public:
	MESSAGE_FUNC_INT(ItemSelected, "ItemSelected", itemID);

private:
	void Finish(int key, int button, const char *pchKeyName = NULL);

private:
	struct KeyBinding
	{
		char *binding;
	};

private:
	void CreateKeyBindingList(void);
	void BindKey(const char *key, const char *binding);
	void UnbindKey(const char *key);

	const char *GetKeyName(int keynum);
	int FindKeyForName(char const *keyname);

	void SaveCurrentBindings(void);
	void DeleteSavedBindings(void);
	void ParseActionDescriptions(void);
	void FillInCurrentBindings(void);
	void ClearBindItems(void);
	void FillInDefaultBindings(void);
	void ApplyAllBindings(void);
	void AddBinding(KeyValues *item, const char *keyname);
	void RemoveKeyFromBindItems(const char *key);
	KeyValues *GetItemForBinding(const char *binding);

private:
	VControlsListPanel *m_pKeyBindList;

	vgui::Button *m_pSetBindingButton;
	vgui::Button *m_pClearBindingButton;

	KeyBinding m_Bindings[256];
	CUtlVector<CUtlSymbol> m_KeysToUnbind;
};

#endif