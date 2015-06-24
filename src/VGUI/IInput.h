#ifndef VGUI_IINPUT_H
#define VGUI_IINPUT_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "tier1/interface.h"
#include "vgui/MouseCode.h"
#include "vgui/KeyCode.h"

namespace vgui
{

class Cursor;
typedef unsigned long HCursor;

#define VGUI_GCS_COMPREADSTR 0x0001
#define VGUI_GCS_COMPREADATTR 0x0002
#define VGUI_GCS_COMPREADCLAUSE 0x0004
#define VGUI_GCS_COMPSTR 0x0008
#define VGUI_GCS_COMPATTR 0x0010
#define VGUI_GCS_COMPCLAUSE 0x0020
#define VGUI_GCS_CURSORPOS 0x0080
#define VGUI_GCS_DELTASTART 0x0100
#define VGUI_GCS_RESULTREADSTR 0x0200
#define VGUI_GCS_RESULTREADCLAUSE 0x0400
#define VGUI_GCS_RESULTSTR 0x0800
#define VGUI_GCS_RESULTCLAUSE 0x1000

#define VGUI_CS_INSERTCHAR 0x2000
#define VGUI_CS_NOMOVECARET 0x4000

class IInput : public IBaseInterface
{
public:
	virtual void SetMouseFocus(VPANEL newMouseFocus) = 0;
	virtual void SetMouseCapture(VPANEL panel) = 0;
	virtual void GetKeyCodeText(KeyCode code, char *buf, int buflen) = 0;
	virtual VPANEL GetFocus(void) = 0;
	virtual VPANEL GetMouseOver(void) = 0;
	virtual void SetCursorPos(int x, int y) = 0;
	virtual void GetCursorPos(int &x, int &y) = 0;
	virtual bool WasMousePressed(MouseCode code) = 0;
	virtual bool WasMouseDoublePressed(MouseCode code) = 0;
	virtual bool IsMouseDown(MouseCode code) = 0;
	virtual void SetCursorOveride(HCursor cursor) = 0;
	virtual HCursor GetCursorOveride(void) = 0;
	virtual bool WasMouseReleased(MouseCode code) = 0;
	virtual bool WasKeyPressed(KeyCode code) = 0;
	virtual bool IsKeyDown(KeyCode code) = 0;
	virtual bool WasKeyTyped(KeyCode code) = 0;
	virtual bool WasKeyReleased(KeyCode code) = 0;
	virtual VPANEL GetAppModalSurface(void) = 0;
	virtual void SetAppModalSurface(VPANEL panel) = 0;
	virtual void ReleaseAppModalSurface(void) = 0;
	virtual void GetCursorPosition(int &x, int &y) = 0;

public:
	void SetIMEWindow(void *hwnd);
	void *GetIMEWindow(void);

	void OnChangeIME(bool forward);

	int GetCurrentIMEHandle(void);
	int GetEnglishIMEHandle(void);

	void GetIMELanguageName(wchar_t *buf, int unicodeBufferSizeInBytes);
	void GetIMELanguageShortCode(wchar_t *buf, int unicodeBufferSizeInBytes);

	struct LanguageItem
	{
		wchar_t shortname[4];
		wchar_t menuname[128];
		int handleValue;
		bool active;
	};

	struct ConversionModeItem
	{
		wchar_t menuname[128];
		int handleValue;
		bool active;
	};

	struct SentenceModeItem
	{
		wchar_t menuname[128];
		int handleValue;
		bool active;
	};

	int GetIMELanguageList(LanguageItem *dest, int destcount);
	int GetIMEConversionModes(ConversionModeItem *dest, int destcount);
	int GetIMESentenceModes(SentenceModeItem *dest, int destcount);

	void OnChangeIMEByHandle(int handleValue);
	void OnChangeIMEConversionModeByHandle(int handleValue);
	void OnChangeIMESentenceModeByHandle(int handleValue);

	void OnInputLanguageChanged(void);
	void OnIMEStartComposition(void);
	void OnIMEComposition(int flags);
	void OnIMEEndComposition(void);

	void OnIMEShowCandidates(void);
	void OnIMEChangeCandidates(void);
	void OnIMECloseCandidates(void);
	void OnIMERecomputeModes(void);
	void OnIMESelectCandidate(int num);

	int GetCandidateListCount(void);
	void GetCandidate(int num, wchar_t *dest, int destSizeBytes);
	int GetCandidateListSelectedItem(void);
	int GetCandidateListPageSize(void);
	int GetCandidateListPageStart(void);

	void SetCandidateWindowPos(int x, int y);

	bool GetShouldInvertCompositionString(void);
	bool CandidateListStartsAtOne(void);

	void SetCandidateListPageStart(int start);
	void GetCompositionString(wchar_t *dest, int destSizeBytes);
	void CancelCompositionString(void);

public:
	void SetMouseCaptureEx(VPANEL panel, MouseCode captureStartMouseCode);

	void RegisterKeyCodeUnhandledListener(VPANEL panel);
	void UnregisterKeyCodeUnhandledListener(VPANEL panel);

	void OnKeyCodeUnhandled(int keyCode);

	void SetModalSubTree(VPANEL subTree, VPANEL unhandledMouseClickListener, bool restrictMessagesToSubTree = true);
	void ReleaseModalSubTree(void);
	VPANEL GetModalSubTree(void);

	void SetModalSubTreeReceiveMessages(bool state);
	bool ShouldModalSubTreeReceiveMessages(void) const;

	VPANEL GetMouseCapture(void);
};
}

#define VGUI_INPUT_INTERFACE_VERSION "VGUI_Input004"

#endif