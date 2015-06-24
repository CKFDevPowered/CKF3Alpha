#ifndef GAMECONSOLEDIALOG_H
#define GAMECONSOLEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <Color.h>
#include "tier1/UtlVector.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Frame.h"

class CHistoryItem
{
public:
	CHistoryItem(void);
	CHistoryItem(const char *text, const char *extra = NULL);
	CHistoryItem(const CHistoryItem &src);
	~CHistoryItem(void);

public:
	const char *GetText(void) const;
	const char *GetExtra(void) const;
	void SetText(const char *text, const char *extra);
	bool HasExtra(void) {return m_bHasExtra;}

private:
	char *m_text;
	char *m_extraText;
	bool m_bHasExtra;
};

class CGameConsoleDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CGameConsoleDialog, vgui::Frame);

public:
	CGameConsoleDialog(void);
	~CGameConsoleDialog(void);

public:
	void ColorPrint(const Color &clr, const char *pMessage);
	void Print(const char *pMessage);
	void DPrint(const char *pMessage);
	void Clear(void);
	void DumpConsoleTextToFile(void);
	void Hide(void);
	void Activate(void);
	void Close(void);

private:
	enum
	{
		MAX_HISTORY_ITEMS = 100,
	};

	class CompletionItem
	{
	public:
		CompletionItem(void);
		CompletionItem(const CompletionItem &src);
		CompletionItem &CompletionItem::operator = (const CompletionItem &src);
		~CompletionItem(void);

	public:
		const char *GetItemText(void);
		const char *GetCommand(void) const;
		const char *GetName(void) const;

		bool m_bIsCommand;
		unsigned m_pCommand;
		struct cvar_s *m_pCvars;
		CHistoryItem *m_pText;
	};

protected:
	void RebuildCompletionList(const char *partialText);
	void UpdateCompletionListPosition(void);
	void ClearCompletionList(void);
	void AddToHistory(const char *commandText, const char *extraText);
	void PerformLayout(void);
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void OnCommand(const char *command);
	void OnKeyCodeTyped(vgui::KeyCode code);
	void OnThink(void);
	void OnAutoComplete(bool reverse);

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	MESSAGE_FUNC(CloseCompletionList, "CloseCompletionList");
	MESSAGE_FUNC_CHARPTR(OnMenuItemSelected, "CompletionCommand", command);

protected:
	vgui::RichText *m_pHistory;
	vgui::TextEntry *m_pEntry;
	vgui::Button *m_pSubmit;
	vgui::Menu *m_pCompletionList;
	Color m_PrintColor;
	Color m_DPrintColor;

	int m_iNextCompletion;
	char m_szPartialText[256];
	char m_szPreviousPartialText[256];
	bool m_bAutoCompleteMode;
	bool m_bWasBackspacing;

	CUtlVector<CompletionItem *> m_CompletionList;
	CUtlVector<CHistoryItem> m_CommandHistory;
};

#endif