#pragma comment(lib, "Imm32.lib")

#if !defined (_X360)
#include <windows.h>
#include <imm.h>
#endif
#include <string.h>

#include "vgui_internal.h"
#include "VPanel.h"
#include "UtlVector.h"
#include <KeyValues.h>
#include "tier0/vcrmode.h"

#include <vgui/VGUI.h>
#include <vgui/IClientPanel.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>
#include "vgui/Cursor.h"

#include "UtlLinkedList.h"
#include "tier0/icommandline.h"

#if defined (_X360)
#include "xbox/xbox_win32stubs.h"
#endif

#include "tier0/memdbgon.h"

SHORT System_GetKeyState(int virtualKeyCode);

bool IsDispatchingMessageQueue(void);

using namespace vgui;

class CInputWin32 : public IInputInternal
{
public:
	CInputWin32(void);
	~CInputWin32(void);

public:
	virtual void RunFrame(void);

	virtual void PanelDeleted(VPANEL panel);

	virtual void UpdateMouseFocus(int x, int y);
	virtual void SetMouseFocus(VPANEL newMouseFocus);

	virtual void SetCursorPos(int x, int y);
	virtual void UpdateCursorPosInternal(int x, int y);
	virtual void GetCursorPos(int &x, int &y);
	virtual void SetCursorOveride(HCursor cursor);
	virtual HCursor GetCursorOveride(void);

	virtual void SetMouseCapture(VPANEL panel);

	virtual VPANEL GetFocus(void);
	virtual VPANEL GetMouseOver(void);

	virtual bool WasMousePressed(MouseCode code);
	virtual bool WasMouseDoublePressed(MouseCode code);
	virtual bool IsMouseDown(MouseCode code);
	virtual bool WasMouseReleased(MouseCode code);
	virtual bool WasKeyPressed(KeyCode code);
	virtual bool IsKeyDown(KeyCode code);
	virtual bool WasKeyTyped(KeyCode code);
	virtual bool WasKeyReleased(KeyCode code);

	virtual void GetKeyCodeText(KeyCode code, char *buf, int buflen);

	virtual bool InternalCursorMoved(int x,int y);
	virtual bool InternalMousePressed(MouseCode code);
	virtual bool InternalMouseDoublePressed(MouseCode code);
	virtual bool InternalMouseReleased(MouseCode code);
	virtual bool InternalMouseWheeled(int delta);
	virtual bool InternalKeyCodePressed(KeyCode code);
	virtual void InternalKeyCodeTyped(KeyCode code);
	virtual void InternalKeyTyped(wchar_t unichar);
	virtual bool InternalKeyCodeReleased(KeyCode code);

	virtual VPANEL GetAppModalSurface(void);
	virtual void SetAppModalSurface(VPANEL panel);
	virtual void ReleaseAppModalSurface(void);

	virtual bool IsChildOfModalPanel(VPANEL panel);

	virtual HInputContext CreateInputContext(void);
	virtual void DestroyInputContext(HInputContext context); 

	virtual void AssociatePanelWithInputContext(HInputContext context, VPANEL pRoot);

	virtual void ActivateInputContext(HInputContext context);
	virtual void PostCursorMessage(void);
	virtual void HandleExplicitSetCursor(void);

	virtual void ResetInputContext(HInputContext context);

	virtual void GetCursorPosition(int &x, int &y);

	virtual void SetIMEWindow(void *hwnd);
	virtual void *GetIMEWindow(void);

	virtual void OnChangeIME(bool forward);
	virtual int GetCurrentIMEHandle(void);
	virtual int GetEnglishIMEHandle(void);

	virtual void GetIMELanguageName(wchar_t *buf, int unicodeBufferSizeInBytes);
	virtual void GetIMELanguageShortCode(wchar_t *buf, int unicodeBufferSizeInBytes);

	virtual int GetIMELanguageList(LanguageItem *dest, int destcount);
	virtual int GetIMEConversionModes(ConversionModeItem *dest, int destcount);
	virtual int GetIMESentenceModes(SentenceModeItem *dest, int destcount);

	virtual void OnChangeIMEByHandle(int handleValue);
	virtual void OnChangeIMEConversionModeByHandle(int handleValue);
	virtual void OnChangeIMESentenceModeByHandle(int handleValue);

	virtual void OnInputLanguageChanged(void);
	virtual void OnIMEStartComposition(void);
	virtual void OnIMEComposition(int flags);
	virtual void OnIMEEndComposition(void);

	virtual void OnIMEShowCandidates(void);
	virtual void OnIMEChangeCandidates(void);
	virtual void OnIMECloseCandidates(void);

	virtual void OnIMERecomputeModes(void);
	virtual void OnIMESelectCandidate(int num);

	virtual int GetCandidateListCount(void);
	virtual void GetCandidate(int num, wchar_t *dest, int destSizeBytes);
	virtual int GetCandidateListSelectedItem(void);
	virtual int GetCandidateListPageSize(void);
	virtual int GetCandidateListPageStart(void);

	virtual void SetCandidateWindowPos(int x, int y);
	virtual bool GetShouldInvertCompositionString(void);
	virtual bool CandidateListStartsAtOne(void);

	virtual void SetCandidateListPageStart(int start);
	virtual void GetCompositionString(wchar_t *dest, int destSizeBytes);
	virtual void CancelCompositionString(void);

	virtual void SetMouseCaptureEx(VPANEL panel, MouseCode captureStartMouseCode);

	virtual void RegisterKeyCodeUnhandledListener(VPANEL panel);
	virtual void UnregisterKeyCodeUnhandledListener(VPANEL panel);

	virtual void OnKeyCodeUnhandled(int keyCode);

	virtual void SetModalSubTree(VPANEL subTree, VPANEL unhandledMouseClickListener, bool restrictMessagesToSubTree = true);
	virtual void ReleaseModalSubTree(void);
	virtual VPANEL GetModalSubTree(void);

	virtual void SetModalSubTreeReceiveMessages(bool state);
	virtual bool ShouldModalSubTreeReceiveMessages(void) const;

	virtual VPANEL GetMouseCapture(void);

private:
	VPanel *GetMouseFocusIgnoringModalSubtree(void);

	void InternalSetCompositionString(const wchar_t *compstr);
	void InternalShowCandidateWindow(void);
	void InternalHideCandidateWindow(void);
	void InternalUpdateCandidateWindow(void);

	bool PostKeyMessage(KeyValues *message);

	void DestroyCandidateList(void);
	void CreateNewCandidateList(void);

	VPanel *CalculateNewKeyFocus(void);

	void PostModalSubTreeMessage(VPanel *subTree, bool state);
	bool IsChildOfModalPanel(VPANEL panel, bool checkModalSubTree);
	bool IsChildOfModalSubTree(VPANEL panel);

	void SurfaceSetCursorPos(int x, int y);
	void SurfaceGetCursorPos(int &x, int &y);

	struct InputContext_t
	{
		VPANEL _rootPanel;

		bool _mousePressed[MOUSE_LAST];
		bool _mouseDoublePressed[MOUSE_LAST];
		bool _mouseDown[MOUSE_LAST];
		bool _mouseReleased[MOUSE_LAST];
		bool _keyPressed[KEY_LAST];
		bool _keyTyped[KEY_LAST];
		bool _keyDown[KEY_LAST];
		bool _keyReleased[KEY_LAST];

		VPanel *_keyFocus;
		VPanel *_oldMouseFocus;
		VPanel *_mouseFocus;
		VPanel *_mouseOver;

		VPanel *_mouseCapture;
		MouseCode m_MouseCaptureStartCode;
		VPanel *_appModalPanel;

		int m_nCursorX;
		int m_nCursorY;

		int m_nLastPostedCursorX;
		int m_nLastPostedCursorY;

		int m_nExternallySetCursorX;
		int m_nExternallySetCursorY;
		bool m_bSetCursorExplicitly;

		CUtlVector<VPanel *> m_KeyCodeUnhandledListeners;

		VPanel *m_pModalSubTree;
		VPanel *m_pUnhandledMouseClickListener;
		bool m_bRestrictMessagesToModalSubTree;
	};

	void InitInputContext(InputContext_t *pContext);
	InputContext_t *GetInputContext(HInputContext context);
	void PanelDeleted(VPANEL focus, InputContext_t &context);

	HCursor _cursorOverride;
	char *_keyTrans[KEY_LAST];

	InputContext_t m_DefaultInputContext;
	HInputContext m_hContext;

	CUtlLinkedList<InputContext_t, HInputContext> m_Contexts;

#ifndef _X360
	void *_imeWnd;
	CANDIDATELIST *_imeCandidates;
#endif

	int m_nDebugMessages;
};

CInputWin32 g_Input;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInput, VGUI_INPUT_INTERFACE_VERSION, g_Input);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInputInternal, VGUI_INPUTINTERNAL_INTERFACE_VERSION, g_Input);

namespace vgui
{
vgui::IInputInternal *g_pInput = &g_Input;
}

CInputWin32::CInputWin32(void)
{
	m_nDebugMessages = -1;
#ifndef _X360
	_imeWnd = null;
	_imeCandidates = null;
#endif
	InitInputContext(&m_DefaultInputContext);
	m_hContext = DEFAULT_INPUT_CONTEXT;

	_keyTrans[KEY_0] = "0)KEY_0";
	_keyTrans[KEY_1] = "1!KEY_1";
	_keyTrans[KEY_2] = "2@KEY_2";
	_keyTrans[KEY_3] = "3#KEY_3";
	_keyTrans[KEY_4] = "4$KEY_4";
	_keyTrans[KEY_5] = "5%KEY_5";
	_keyTrans[KEY_6] = "6^KEY_6";
	_keyTrans[KEY_7] = "7&KEY_7";
	_keyTrans[KEY_8] = "8*KEY_8";
	_keyTrans[KEY_9] = "9(KEY_9";
	_keyTrans[KEY_A] = "aAKEY_A";
	_keyTrans[KEY_B] = "bBKEY_B";
	_keyTrans[KEY_C] = "cCKEY_C";
	_keyTrans[KEY_D] = "dDKEY_D";
	_keyTrans[KEY_E] = "eEKEY_E";
	_keyTrans[KEY_F] = "fFKEY_F";
	_keyTrans[KEY_G] = "gGKEY_G";
	_keyTrans[KEY_H] = "hHKEY_H";
	_keyTrans[KEY_I] = "iIKEY_I";
	_keyTrans[KEY_J] = "jJKEY_J";
	_keyTrans[KEY_K] = "kKKEY_K";
	_keyTrans[KEY_L] = "lLKEY_L"", L";
	_keyTrans[KEY_M] = "mMKEY_M";
	_keyTrans[KEY_N] = "nNKEY_N";
	_keyTrans[KEY_O] = "oOKEY_O";
	_keyTrans[KEY_P] = "pPKEY_P";
	_keyTrans[KEY_Q] = "qQKEY_Q";
	_keyTrans[KEY_R] = "rRKEY_R";
	_keyTrans[KEY_S] = "sSKEY_S";
	_keyTrans[KEY_T] = "tTKEY_T";
	_keyTrans[KEY_U] = "uUKEY_U";
	_keyTrans[KEY_V] = "vVKEY_V";
	_keyTrans[KEY_W] = "wWKEY_W";
	_keyTrans[KEY_X] = "xXKEY_X";
	_keyTrans[KEY_Y] = "yYKEY_Y";
	_keyTrans[KEY_Z] = "zZKEY_Z";
	_keyTrans[KEY_PAD_0] = "0\0KEY_PAD_0";
	_keyTrans[KEY_PAD_1] = "1\0KEY_PAD_1";
	_keyTrans[KEY_PAD_2] = "2\0KEY_PAD_2";
	_keyTrans[KEY_PAD_3] = "3\0KEY_PAD_3";
	_keyTrans[KEY_PAD_4] = "4\0KEY_PAD_4";
	_keyTrans[KEY_PAD_5] = "5\0KEY_PAD_5";
	_keyTrans[KEY_PAD_6] = "6\0KEY_PAD_6";
	_keyTrans[KEY_PAD_7] = "7\0KEY_PAD_7";
	_keyTrans[KEY_PAD_8] = "8\0KEY_PAD_8";
	_keyTrans[KEY_PAD_9] = "9\0KEY_PAD_9";
	_keyTrans[KEY_PAD_DIVIDE] = "//KEY_PAD_DIVIDE";
	_keyTrans[KEY_PAD_MULTIPLY] = "**KEY_PAD_MULTIPLY";
	_keyTrans[KEY_PAD_MINUS] = "--KEY_PAD_MINUS";
	_keyTrans[KEY_PAD_PLUS] = "++KEY_PAD_PLUS";
	_keyTrans[KEY_PAD_ENTER] = "\0\0KEY_PAD_ENTER";
	_keyTrans[KEY_PAD_DECIMAL] = ".\0KEY_PAD_DECIMAL"", L";
	_keyTrans[KEY_LBRACKET] = "[{KEY_LBRACKET";
	_keyTrans[KEY_RBRACKET] = "]}KEY_RBRACKET";
	_keyTrans[KEY_SEMICOLON] = ";:KEY_SEMICOLON";
	_keyTrans[KEY_APOSTROPHE] = "'\"KEY_APOSTROPHE";
	_keyTrans[KEY_BACKQUOTE] = "`~KEY_BACKQUOTE";
	_keyTrans[KEY_COMMA] = ",<KEY_COMMA";
	_keyTrans[KEY_PERIOD] = ".>KEY_PERIOD";
	_keyTrans[KEY_SLASH] = "/?KEY_SLASH";
	_keyTrans[KEY_BACKSLASH] = "\\|KEY_BACKSLASH";
	_keyTrans[KEY_MINUS] = "-_KEY_MINUS";
	_keyTrans[KEY_EQUAL] = "=+KEY_EQUAL"", L";
	_keyTrans[KEY_ENTER] = "\0\0KEY_ENTER";
	_keyTrans[KEY_SPACE] = "  KEY_SPACE";
	_keyTrans[KEY_BACKSPACE] = "\0\0KEY_BACKSPACE";
	_keyTrans[KEY_TAB] = "\0\0KEY_TAB";
	_keyTrans[KEY_CAPSLOCK] = "\0\0KEY_CAPSLOCK";
	_keyTrans[KEY_NUMLOCK] = "\0\0KEY_NUMLOCK";
	_keyTrans[KEY_ESCAPE] = "\0\0KEY_ESCAPE";
	_keyTrans[KEY_SCROLLLOCK] = "\0\0KEY_SCROLLLOCK";
	_keyTrans[KEY_INSERT] = "\0\0KEY_INSERT";
	_keyTrans[KEY_DELETE] = "\0\0KEY_DELETE";
	_keyTrans[KEY_HOME] = "\0\0KEY_HOME";
	_keyTrans[KEY_END] = "\0\0KEY_END";
	_keyTrans[KEY_PAGEUP] = "\0\0KEY_PAGEUP";
	_keyTrans[KEY_PAGEDOWN] = "\0\0KEY_PAGEDOWN";
	_keyTrans[KEY_BREAK] = "\0\0KEY_BREAK";
	_keyTrans[KEY_LSHIFT] = "\0\0KEY_LSHIFT";
	_keyTrans[KEY_RSHIFT] = "\0\0KEY_RSHIFT";
	_keyTrans[KEY_LALT] = "\0\0KEY_LALT";
	_keyTrans[KEY_RALT] = "\0\0KEY_RALT";
	_keyTrans[KEY_LCONTROL] = "\0\0KEY_LCONTROL"", L";
	_keyTrans[KEY_RCONTROL] = "\0\0KEY_RCONTROL"", L";
	_keyTrans[KEY_LWIN] = "\0\0KEY_LWIN";
	_keyTrans[KEY_RWIN] = "\0\0KEY_RWIN";
	_keyTrans[KEY_APP] = "\0\0KEY_APP";
	_keyTrans[KEY_UP] = "\0\0KEY_UP";
	_keyTrans[KEY_LEFT] = "\0\0KEY_LEFT";
	_keyTrans[KEY_DOWN] = "\0\0KEY_DOWN";
	_keyTrans[KEY_RIGHT] = "\0\0KEY_RIGHT";
	_keyTrans[KEY_F1] = "\0\0KEY_F1";
	_keyTrans[KEY_F2] = "\0\0KEY_F2";
	_keyTrans[KEY_F3] = "\0\0KEY_F3";
	_keyTrans[KEY_F4] = "\0\0KEY_F4";
	_keyTrans[KEY_F5] = "\0\0KEY_F5";
	_keyTrans[KEY_F6] = "\0\0KEY_F6";
	_keyTrans[KEY_F7] = "\0\0KEY_F7";
	_keyTrans[KEY_F8] = "\0\0KEY_F8";
	_keyTrans[KEY_F9] = "\0\0KEY_F9";
	_keyTrans[KEY_F10] = "\0\0KEY_F10";
	_keyTrans[KEY_F11] = "\0\0KEY_F11";
	_keyTrans[KEY_F12] = "\0\0KEY_F12";
}

CInputWin32::~CInputWin32(void)
{
	DestroyCandidateList();
}

void CInputWin32::InitInputContext(InputContext_t *pContext)
{
	pContext->_rootPanel = NULL;
	pContext->_keyFocus = NULL;
	pContext->_oldMouseFocus = NULL;
	pContext->_mouseFocus = NULL;
	pContext->_mouseOver = NULL;
	pContext->_mouseCapture = NULL;
	pContext->_appModalPanel = NULL;

	pContext->m_nCursorX = pContext->m_nCursorY = 0;
	pContext->m_nLastPostedCursorX = pContext->m_nLastPostedCursorY = -9999;
	pContext->m_nExternallySetCursorX = pContext->m_nExternallySetCursorY = 0;
	pContext->m_bSetCursorExplicitly = false;

	memset(pContext->_mousePressed, 0, sizeof(pContext->_mousePressed));
	memset(pContext->_mouseDoublePressed, 0, sizeof(pContext->_mouseDoublePressed));
	memset(pContext->_mouseDown, 0, sizeof(pContext->_mouseDown));
	memset(pContext->_mouseReleased, 0, sizeof(pContext->_mouseReleased));
	memset(pContext->_keyPressed, 0, sizeof(pContext->_keyPressed));
	memset(pContext->_keyTyped, 0, sizeof(pContext->_keyTyped));
	memset(pContext->_keyDown, 0, sizeof(pContext->_keyDown));
	memset(pContext->_keyReleased, 0, sizeof(pContext->_keyReleased));

	pContext->m_MouseCaptureStartCode = (MouseCode)-1;

	pContext->m_KeyCodeUnhandledListeners.RemoveAll();

	pContext->m_pModalSubTree = NULL;
	pContext->m_pUnhandledMouseClickListener = NULL;
	pContext->m_bRestrictMessagesToModalSubTree = false;
}

void CInputWin32::ResetInputContext(HInputContext context)
{
	InitInputContext(GetInputContext(context));
}

HInputContext CInputWin32::CreateInputContext(void)
{
	HInputContext i = m_Contexts.AddToTail();
	InitInputContext(&m_Contexts[i]);
	return i;
}

void CInputWin32::DestroyInputContext(HInputContext context)
{
	Assert(context != DEFAULT_INPUT_CONTEXT);

	if (m_hContext == context)
	{
		ActivateInputContext(DEFAULT_INPUT_CONTEXT);
	}

	m_Contexts.Remove(context);
}

CInputWin32::InputContext_t *CInputWin32::GetInputContext(HInputContext context)
{
	if (context == DEFAULT_INPUT_CONTEXT)
		return &m_DefaultInputContext;

	return &m_Contexts[context];
}

void CInputWin32::AssociatePanelWithInputContext(HInputContext context, VPANEL pRoot)
{
	if (GetInputContext(context)->_rootPanel != pRoot)
	{
		ResetInputContext(context);
		GetInputContext(context)->_rootPanel = pRoot;
	}
}

void CInputWin32::ActivateInputContext(HInputContext context)
{
	Assert((context == DEFAULT_INPUT_CONTEXT) || m_Contexts.IsValidIndex(context));
	m_hContext = context;
}

void CInputWin32::RunFrame(void)
{
	if (m_nDebugMessages == -1)
	{
		m_nDebugMessages = CommandLine()->CheckParm("-vguifocus") ? 1 : 0;
	}

	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->_keyFocus)
	{
		if (IsChildOfModalPanel((VPANEL)pContext->_keyFocus))
		{
			g_pIVgui->PostMessage((VPANEL)pContext->_keyFocus, new KeyValues("KeyFocusTicked"), NULL);
		}
	}

	if (pContext->_mouseFocus)
	{
		if (IsChildOfModalPanel((VPANEL)pContext->_mouseFocus))
		{
			g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("MouseFocusTicked"), NULL);
		}
	}
	else if (pContext->_appModalPanel)
	{
		g_pSurface->SetCursor(vgui::dc_arrow);
	}

	for (int i = 0; i < MOUSE_LAST; i++)
	{
		pContext->_mousePressed[i] = 0;
		pContext->_mouseDoublePressed[i] = 0;
		pContext->_mouseReleased[i] = 0;
	}

	for (int i = 0; i < KEY_LAST; i++)
	{
		pContext->_keyPressed[i] = 0;
		pContext->_keyTyped[i] = 0;
		pContext->_keyReleased[i] = 0;
	}

	VPanel *wantedKeyFocus = CalculateNewKeyFocus();

	if (pContext->_keyFocus != wantedKeyFocus)
	{
		if (pContext->_keyFocus != NULL)
		{
			pContext->_keyFocus->Client()->InternalFocusChanged(true);

			g_pIVgui->PostMessage((VPANEL)pContext->_keyFocus, new KeyValues("KillFocus"), NULL);

			pContext->_keyFocus->Client()->Repaint();

			VPanel *dlg = pContext->_keyFocus;

			while (dlg && !dlg->IsPopup())
			{
				dlg = dlg->GetParent();
			}

			if (dlg)
			{
				dlg->Client()->Repaint();
			}
		}

		if (wantedKeyFocus != NULL)
		{
			wantedKeyFocus->Client()->InternalFocusChanged(false);

			g_pIVgui->PostMessage((VPANEL)wantedKeyFocus, new KeyValues("SetFocus"), NULL);

			wantedKeyFocus->Client()->Repaint();

			VPanel *dlg = wantedKeyFocus;

			while (dlg && !dlg->IsPopup())
			{
				dlg = dlg->GetParent();
			}

			if (dlg)
			{
				dlg->Client()->Repaint();
			}
		}

		if (m_nDebugMessages > 0)
		{
			g_pIVgui->DPrintf2("changing kb focus from %s to %s\n", pContext->_keyFocus ? pContext->_keyFocus->GetName() : "(no name)", wantedKeyFocus ? wantedKeyFocus->GetName() : "(no name)");
		}

		pContext->_keyFocus = wantedKeyFocus;

		if (pContext->_keyFocus)
		{
			pContext->_keyFocus->MoveToFront();
		}
	}
}

VPanel *CInputWin32::CalculateNewKeyFocus(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	VPanel *wantedKeyFocus = NULL;
	VPanel *pRoot = (VPanel *)pContext->_rootPanel;
	VPanel *top = pRoot;

	if (g_pSurface->GetPopupCount() > 0)
	{
		int nIndex = g_pSurface->GetPopupCount();

		while (nIndex)
		{
			top = (VPanel *)g_pSurface->GetPopup(--nIndex);

			if (top && top->IsPopup() && top->IsVisible() && top->IsKeyBoardInputEnabled() && !g_pSurface->IsMinimized((VPANEL)top) && IsChildOfModalSubTree((VPANEL)top) && (!pRoot || top->HasParent(pRoot)))
			{
				bool bIsVisible = top->IsVisible();
				VPanel *p = top->GetParent();

				while (p && bIsVisible)
				{
					if (p->IsVisible() == false)
					{
						bIsVisible = false;
						break;
					}

					p = p->GetParent();
				}

				if (bIsVisible && !g_pSurface->IsMinimized((VPANEL)top))
					break;
			}

			top = pRoot;
		}
	}

	if (top)
	{
		wantedKeyFocus = (VPanel *)top->Client()->GetCurrentKeyFocus();

		if (!wantedKeyFocus)
		{
			wantedKeyFocus = top;
		}
	}

	if (!g_pSurface->HasFocus())
	{
		wantedKeyFocus = NULL;
	}

	if (!IsChildOfModalPanel((VPANEL)wantedKeyFocus))
	{
		wantedKeyFocus = NULL;
	}

	return wantedKeyFocus;
}

void CInputWin32::PanelDeleted(VPANEL vfocus, InputContext_t &context)
{
	VPanel *focus = (VPanel *)vfocus;

	if (context._keyFocus == focus)
	{
		if (m_nDebugMessages > 0)
		{
			g_pIVgui->DPrintf2("removing kb focus %s\n", context._keyFocus ? context._keyFocus->GetName() : "(no name)");
		}

		context._keyFocus = NULL;
	}

	if (context._mouseOver == focus)
	{
		context._mouseOver = NULL;
	}

	if (context._oldMouseFocus == focus)
	{
		context._oldMouseFocus = NULL;
	}

	if (context._mouseFocus == focus)
	{
		context._mouseFocus = NULL;
	}

	if (context._mouseCapture == focus)
	{
		SetMouseCapture(NULL);
		context._mouseCapture = NULL;
	}

	if (context._appModalPanel == focus)
	{
		ReleaseAppModalSurface();
	}

	if (context.m_pUnhandledMouseClickListener == focus)
	{
		context.m_pUnhandledMouseClickListener = NULL;
	}

	if (context.m_pModalSubTree == focus)
	{
		context.m_pModalSubTree = NULL;
		context.m_bRestrictMessagesToModalSubTree = false;
	}

	context.m_KeyCodeUnhandledListeners.FindAndRemove(focus);
}

void CInputWin32::PanelDeleted(VPANEL focus)
{
	HInputContext i;

	for (i = m_Contexts.Head(); i != m_Contexts.InvalidIndex(); i = m_Contexts.Next(i))
	{
		PanelDeleted(focus, m_Contexts[i]);
	}

	PanelDeleted(focus, m_DefaultInputContext);
}

void CInputWin32::SetMouseFocus(VPANEL newMouseFocus)
{
	if (!IsChildOfModalPanel(newMouseFocus))
	{
		return;
	}

	bool wantsMouse, isPopup;
	VPanel *panel = (VPanel *)newMouseFocus;

	InputContext_t *pContext = GetInputContext(m_hContext);

	wantsMouse = false;

	if (newMouseFocus)
	{
		do
		{
			wantsMouse = panel->IsMouseInputEnabled();
			isPopup = panel->IsPopup();
			panel = panel->GetParent();
		}
		while (wantsMouse && !isPopup && panel && panel->GetParent());
	}

	if (newMouseFocus && !wantsMouse)
	{
		return;
	}

	if ((VPANEL)pContext->_mouseOver != newMouseFocus || (!pContext->_mouseCapture && (VPANEL)pContext->_mouseFocus != newMouseFocus))
	{
		pContext->_oldMouseFocus = pContext->_mouseOver;
		pContext->_mouseOver = (VPanel *)newMouseFocus;

		if (pContext->_oldMouseFocus != NULL)
		{
			if (!pContext->_mouseCapture || pContext->_oldMouseFocus == pContext->_mouseCapture)
			{
				g_pIVgui->PostMessage((VPANEL)pContext->_oldMouseFocus, new KeyValues("CursorExited"), NULL);
			}
		}

		if (pContext->_mouseOver != NULL)
		{
			if (!pContext->_mouseCapture || pContext->_mouseOver == pContext->_mouseCapture)
			{
				g_pIVgui->PostMessage((VPANEL)pContext->_mouseOver, new KeyValues("CursorEntered"), NULL);
			}
		}

		VPanel *newFocus = pContext->_mouseCapture ? pContext->_mouseCapture : pContext->_mouseOver;

		if (m_nDebugMessages > 0)
		{
			g_pIVgui->DPrintf2("changing mouse focus from %s to %s\n", pContext->_mouseFocus ? pContext->_mouseFocus->GetName() : "(no name)", newFocus ? newFocus->GetName() : "(no name)");
		}

		pContext->_mouseFocus = newFocus;
	}
}

VPanel *CInputWin32::GetMouseFocusIgnoringModalSubtree(void)
{
	VPanel *focus = NULL; 
	InputContext_t *pContext = GetInputContext(m_hContext);

	int x = pContext->m_nCursorX;
	int y = pContext->m_nCursorY;

	if (!pContext->_rootPanel)
	{
		if (g_pSurface->IsCursorVisible() && g_pSurface->IsWithin(x, y))
		{
			for (int i = g_pSurface->GetPopupCount() - 1; i >= 0; i--)
			{
				VPanel *popup = (VPanel *)g_pSurface->GetPopup(i);
				VPanel *panel = popup;
				bool wantsMouse = panel->IsMouseInputEnabled();
				bool isVisible = !g_pSurface->IsMinimized((VPANEL)panel);

				while (isVisible && panel && panel->GetParent())
				{
					isVisible = panel->IsVisible();
					panel = panel->GetParent();
				}

				if (wantsMouse && isVisible)
				{
					focus = (VPanel *)popup->Client()->IsWithinTraverse(x, y, false);

					if (focus)
						break;
				}
			}
			if (!focus)
			{
				focus = (VPanel *)((VPanel *)g_pSurface->GetEmbeddedPanel())->Client()->IsWithinTraverse(x, y, false);
			}
		}
	}
	else
	{
		focus = (VPanel *)((VPanel *)(pContext->_rootPanel))->Client()->IsWithinTraverse(x, y, false);
	}

	if (!IsChildOfModalPanel((VPANEL)focus, false))
	{
		focus = NULL;
	}

	return focus;
}

void CInputWin32::UpdateMouseFocus(int x, int y)
{
	VPanel *focus = NULL; 
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext->_rootPanel)
	{
		if (g_pSurface->IsCursorVisible() && g_pSurface->IsWithin(x, y))
		{
			for (int i = g_pSurface->GetPopupCount() - 1; i >= 0; i--)
			{
				VPanel *popup = (VPanel *)g_pSurface->GetPopup(i);
				VPanel *panel = popup;
				bool wantsMouse = panel->IsMouseInputEnabled() && IsChildOfModalSubTree((VPANEL)panel);
				bool isVisible = !g_pSurface->IsMinimized((VPANEL)panel);

				while (isVisible && panel && panel->GetParent())
				{
					isVisible = panel->IsVisible();
					panel = panel->GetParent();
				}

				if (wantsMouse && isVisible)
				{
					focus = (VPanel *)popup->Client()->IsWithinTraverse(x, y, false);

					if (focus)
						break;
				}
			}

			if (!focus)
			{
				focus = (VPanel *)((VPanel *)g_pSurface->GetEmbeddedPanel())->Client()->IsWithinTraverse(x, y, false);
			}
		}
	}
	else
	{
		focus = (VPanel *)((VPanel *)(pContext->_rootPanel))->Client()->IsWithinTraverse(x, y, true);
	}

	if (!IsChildOfModalPanel((VPANEL)focus))
	{
		focus = NULL;
	}

	SetMouseFocus((VPANEL)focus);
}

void CInputWin32::SetMouseCaptureEx(VPANEL panel, MouseCode captureStartMouseCode)
{
	SetMouseCapture(panel);

	if (!IsChildOfModalPanel(panel))
	{
		return;
	}

	InputContext_t *pContext = GetInputContext(m_hContext);
	Assert(pContext);
	pContext->m_MouseCaptureStartCode = captureStartMouseCode;
}

VPANEL CInputWin32::GetMouseCapture(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);
	return (VPANEL)pContext->_mouseCapture;
}

void CInputWin32::SetMouseCapture(VPANEL panel)
{
	if (!IsChildOfModalPanel(panel))
	{
		return;
	}

	InputContext_t *pContext = GetInputContext(m_hContext);
	Assert(pContext);

	pContext->m_MouseCaptureStartCode = (MouseCode)-1;

	if (pContext->_mouseCapture && panel != (VPANEL)pContext->_mouseCapture)
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseCapture, new KeyValues("MouseCaptureLost"), NULL);
	}

	if (panel == NULL)
	{
		if (pContext->_mouseCapture != NULL)
		{
			g_pSurface->EnableMouseCapture((VPANEL)pContext->_mouseCapture, false);
		}
	}
	else
	{
		g_pSurface->EnableMouseCapture(panel, true);
	}

	pContext->_mouseCapture = (VPanel *)panel;
}

bool CInputWin32::IsChildOfModalSubTree(VPANEL panel)
{
	if (!panel)
		return true;

	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->m_pModalSubTree)
	{
		bool isChildOfModal = ((VPanel *)panel)->HasParent(pContext->m_pModalSubTree);

		if (isChildOfModal)
		{
			return pContext->m_bRestrictMessagesToModalSubTree;
		}
		else
		{
			return !pContext->m_bRestrictMessagesToModalSubTree;
		}
	}

	return true;
}

bool CInputWin32::IsChildOfModalPanel(VPANEL panel)
{
	return IsChildOfModalPanel(panel, true);
}

bool CInputWin32::IsChildOfModalPanel(VPANEL panel, bool checkModalSubTree)
{
	if (!panel)
		return true;

	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->_appModalPanel)
	{
		if (!((VPanel *)panel)->HasParent(pContext->_appModalPanel))
		{
			return false;
		}
	}

	if (!checkModalSubTree)
		return true;

	return IsChildOfModalSubTree(panel);
}

VPANEL CInputWin32::GetFocus(void)
{
	return (VPANEL)(GetInputContext(m_hContext)->_keyFocus);
}

VPANEL CInputWin32::GetMouseOver(void)
{
	return (VPANEL)(GetInputContext(m_hContext)->_mouseOver);
}

bool CInputWin32::WasMousePressed(MouseCode code)
{
	return GetInputContext(m_hContext)->_mousePressed[code];
}

bool CInputWin32::WasMouseDoublePressed(MouseCode code)
{
	return GetInputContext(m_hContext)->_mouseDoublePressed[code];
}

bool CInputWin32::IsMouseDown(MouseCode code)
{
	return GetInputContext(m_hContext)->_mouseDown[code];
}

bool CInputWin32::WasMouseReleased(MouseCode code)
{
	return GetInputContext(m_hContext)->_mouseReleased[code];
}

bool CInputWin32::WasKeyPressed(KeyCode code)
{
	return GetInputContext(m_hContext)->_keyPressed[code];
}

bool CInputWin32::IsKeyDown(KeyCode code)
{
	return GetInputContext(m_hContext)->_keyDown[code];
}

bool CInputWin32::WasKeyTyped(KeyCode code)
{
	return GetInputContext(m_hContext)->_keyTyped[code];
}

bool CInputWin32::WasKeyReleased(KeyCode code)
{
	return GetInputContext(m_hContext)->_keyReleased[code];
}

void CInputWin32::UpdateCursorPosInternal(int x, int y)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->m_nCursorX == x && pContext->m_nCursorY == y)
		return;

	pContext->m_nCursorX = x;
	pContext->m_nCursorY = y;

	UpdateMouseFocus(x, y);
}

void CInputWin32::SetCursorPos(int x, int y)
{
	if (IsDispatchingMessageQueue())
	{
		InputContext_t *pContext = GetInputContext(m_hContext);
		pContext->m_nExternallySetCursorX = x;
		pContext->m_nExternallySetCursorY = y;
		pContext->m_bSetCursorExplicitly = true;
	}
	else
	{
		SurfaceSetCursorPos(x, y);
	}
}

void CInputWin32::GetCursorPos(int &x, int &y)
{
	if (IsDispatchingMessageQueue())
	{
		GetCursorPosition(x, y);
	}
	else
	{
		SurfaceGetCursorPos(x, y);
	}
}

void CInputWin32::GetCursorPosition(int &x, int &y)
{
	InputContext_t *pContext = GetInputContext(m_hContext);
	x = pContext->m_nCursorX;
	y = pContext->m_nCursorY;
}

void CInputWin32::GetKeyCodeText(KeyCode code, char *buf, int buflen)
{
	if (!buf)
		return;

	for (int i = 0; i < buflen; i++)
	{
		char ch = _keyTrans[code][i + 2];
		buf[i] = ch;

		if (ch == 0)
			break;
	}
}

void CInputWin32::SurfaceSetCursorPos(int x, int y)
{
	if (g_pSurface->HasCursorPosFunctions())
	{
		g_pSurface->SurfaceSetCursorPos(x, y);
	}
	else
	{
		int px, py, pw, pt;
		g_pSurface->GetAbsoluteWindowBounds(px, py, pw, pt);
		x += px;
		y += py;

		::SetCursorPos(x, y);
	}
}

void CInputWin32::SurfaceGetCursorPos(int &x, int &y)
{
#ifndef _X360
	if (g_pSurface->HasCursorPosFunctions())
	{
		g_pSurface->SurfaceGetCursorPos(x,y);
	}
	else
	{
		POINT pnt;
		::GetCursorPos(&pnt);
		x = pnt.x;
		y = pnt.y;

		int px, py, pw, pt;
		g_pSurface->GetAbsoluteWindowBounds(px, py, pw, pt);
		x -= px;
		y -= py;
	}
#else
	x = 0;
	y = 0;
#endif
}

void CInputWin32::SetCursorOveride(HCursor cursor)
{
	_cursorOverride = cursor;
}

HCursor CInputWin32::GetCursorOveride(void)
{
	return _cursorOverride;
}

bool CInputWin32::InternalCursorMoved(int x, int y)
{
	g_pIVgui->PostMessage((VPANEL)-1, new KeyValues("SetCursorPosInternal", "xpos", x, "ypos", y), NULL);
	return true;
}

void CInputWin32::HandleExplicitSetCursor(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->m_bSetCursorExplicitly)
	{
		pContext->m_nCursorX = pContext->m_nExternallySetCursorX;
		pContext->m_nCursorY = pContext->m_nExternallySetCursorY;
		pContext->m_bSetCursorExplicitly = false;

		pContext->m_nLastPostedCursorX = pContext->m_nLastPostedCursorY = -9999;

		SurfaceSetCursorPos(pContext->m_nCursorX, pContext->m_nCursorY);
		UpdateMouseFocus(pContext->m_nCursorX, pContext->m_nCursorY); 
	}
}
void CInputWin32::PostCursorMessage(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->m_bSetCursorExplicitly)
	{
		pContext->m_nCursorX = pContext->m_nExternallySetCursorX;
		pContext->m_nCursorY = pContext->m_nExternallySetCursorY;
	}

	if (pContext->m_nLastPostedCursorX == pContext->m_nCursorX && pContext->m_nLastPostedCursorY == pContext->m_nCursorY)
		return;

	pContext->m_nLastPostedCursorX = pContext->m_nCursorX;
	pContext->m_nLastPostedCursorY = pContext->m_nCursorY;

	if (pContext->_mouseCapture)
	{
		if (!IsChildOfModalPanel((VPANEL)pContext->_mouseCapture))
			return;

		g_pIVgui->PostMessage((VPANEL)pContext->_mouseCapture, new KeyValues("CursorMoved", "xpos", pContext->m_nCursorX, "ypos", pContext->m_nCursorY), NULL);
	}
	else if (pContext->_mouseFocus != NULL)
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("CursorMoved", "xpos", pContext->m_nCursorX, "ypos", pContext->m_nCursorY), NULL);
	}
}

bool CInputWin32::InternalMousePressed(MouseCode code)
{
	bool bFilter = false;

	InputContext_t *pContext = GetInputContext(m_hContext);
	VPanel *pTargetPanel = pContext->_mouseOver;

	pContext->_mousePressed[code] = 1;
	pContext->_mouseDown[code] = 1;

	if (pContext->_mouseCapture && IsChildOfModalPanel((VPANEL)pContext->_mouseCapture))
	{
		bFilter = true;

		g_pIVgui->PostMessage((VPANEL)pContext->_mouseCapture, new KeyValues("MousePressed", "code", code), NULL);
		pTargetPanel = pContext->_mouseCapture;

		bool captureLost = code == pContext->m_MouseCaptureStartCode || pContext->m_MouseCaptureStartCode == (MouseCode)-1;

		if (captureLost)
		{
			SetMouseCapture(NULL);
		}
	}
	else if ((pContext->_mouseFocus != NULL) && IsChildOfModalPanel((VPANEL)pContext->_mouseFocus))
	{
		bFilter = true;

		g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("MousePressed", "code", code), NULL);
		pTargetPanel = pContext->_mouseFocus;
	}
	else if (pContext->m_pModalSubTree && pContext->m_pUnhandledMouseClickListener)
	{
		VPanel *p = GetMouseFocusIgnoringModalSubtree();

		if (p)
		{
			bool isChildOfModal = IsChildOfModalSubTree((VPANEL)p);
			bool isUnRestricted = !pContext->m_bRestrictMessagesToModalSubTree;

			if (isUnRestricted != isChildOfModal)
			{
				g_pIVgui->PostMessage((VPANEL)pContext->m_pUnhandledMouseClickListener, new KeyValues("UnhandledMouseClick", "code", code), NULL);
				pTargetPanel = pContext->m_pUnhandledMouseClickListener;
				bFilter = true;
			}
		}
	}

	if (IsChildOfModalPanel((VPANEL)pTargetPanel))
	{
		 g_pSurface->SetTopLevelFocus((VPANEL)pTargetPanel);
	}

	return bFilter;
}

bool CInputWin32::InternalMouseDoublePressed(MouseCode code)
{
	bool bFilter = false;

	InputContext_t *pContext = GetInputContext(m_hContext);
	VPanel *pTargetPanel = pContext->_mouseOver;

	pContext->_mouseDoublePressed[code] = 1;

	if (pContext->_mouseCapture && IsChildOfModalPanel((VPANEL)pContext->_mouseCapture))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseCapture, new KeyValues("MouseDoublePressed", "code", code), NULL);
		pTargetPanel = pContext->_mouseCapture;
		bFilter = true;
	}
	else if ((pContext->_mouseFocus != NULL) && IsChildOfModalPanel((VPANEL)pContext->_mouseFocus))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("MouseDoublePressed", "code", code), NULL);
		pTargetPanel = pContext->_mouseFocus;
		bFilter = true;
	}

	if (IsChildOfModalPanel((VPANEL)pTargetPanel))
	{
		g_pSurface->SetTopLevelFocus((VPANEL)pTargetPanel);
	}

	return bFilter;
}

bool CInputWin32::InternalMouseReleased(MouseCode code)
{
	bool bFilter = false;

	InputContext_t *pContext = GetInputContext(m_hContext);

	pContext->_mouseReleased[code] = 1;
	pContext->_mouseDown[code] = 0;

	if (pContext->_mouseCapture && IsChildOfModalPanel((VPANEL)pContext->_mouseCapture))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseCapture, new KeyValues("MouseReleased", "code", code), NULL);
		bFilter = true;
	}
	else if ((pContext->_mouseFocus != NULL) && IsChildOfModalPanel((VPANEL)pContext->_mouseFocus))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("MouseReleased", "code", code), NULL);
		bFilter = true;
	}

	return bFilter;
}

bool CInputWin32::InternalMouseWheeled(int delta)
{
	bool bFilter = false;

	InputContext_t *pContext = GetInputContext(m_hContext);

	if ((pContext->_mouseFocus != NULL) && IsChildOfModalPanel((VPANEL)pContext->_mouseFocus))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_mouseFocus, new KeyValues("MouseWheeled", "delta", delta), NULL);
		bFilter = true;
	}

	return bFilter;
}

bool CInputWin32::InternalKeyCodePressed(KeyCode code)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (code < 0 || code >= KEY_LAST)
		return false;

	pContext->_keyPressed[code] = 1;
	pContext->_keyDown[code] = 1;

	bool bFilter = PostKeyMessage(new KeyValues("KeyCodePressed", "code", code));
	return bFilter;
}

void CInputWin32::InternalKeyCodeTyped(KeyCode code)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (code < 0 || code >= KEY_LAST)
		return;

	pContext->_keyTyped[code] = 1;

	PostKeyMessage(new KeyValues("KeyCodeTyped", "code", code));
}

void CInputWin32::InternalKeyTyped(wchar_t unichar)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (unichar <= KEY_LAST)
	{
		pContext->_keyTyped[unichar] = 1;
	}

	PostKeyMessage(new KeyValues("KeyTyped", "unichar", unichar));
}

bool CInputWin32::InternalKeyCodeReleased(KeyCode code)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (code < 0 || code >= KEY_LAST)
		return false;

	pContext->_keyReleased[code] = 1;
	pContext->_keyDown[code] = 0;

	return PostKeyMessage(new KeyValues("KeyCodeReleased", "code", code));
}

bool CInputWin32::PostKeyMessage(KeyValues *message)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if ((pContext->_keyFocus != NULL) && IsChildOfModalPanel((VPANEL)pContext->_keyFocus))
	{
		g_pIVgui->PostMessage((VPANEL)pContext->_keyFocus, message, NULL);
		return true;
	}

	message->deleteThis();
	return false;
}

VPANEL CInputWin32::GetAppModalSurface(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);
	return (VPANEL)pContext->_appModalPanel;
}

void CInputWin32::SetAppModalSurface(VPANEL panel)
{
	InputContext_t *pContext = GetInputContext(m_hContext);
	pContext->_appModalPanel = (VPanel *)panel;
}

void CInputWin32::ReleaseAppModalSurface(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);
	pContext->_appModalPanel = NULL;
}

enum LANGFLAG
{
	ENGLISH,
	TRADITIONAL_CHINESE,
	JAPANESE,
	KOREAN,
	SIMPLIFIED_CHINESE,
	UNKNOWN,
	NUM_IMES_SUPPORTED
}
LangFlag;

struct LanguageIds
{
	unsigned short id;
	int languageflag;
	wchar_t	const *shortcode;
	wchar_t const *displayname;
	bool invertcomposition;
};

LanguageIds g_LanguageIds[] =
{
	{ 0x0000, UNKNOWN, L"", L"Neutral" },
	{ 0x007f, UNKNOWN, L"", L"Invariant" },
	{ 0x0400, UNKNOWN, L"", L"User Default Language" },
	{ 0x0800, UNKNOWN, L"", L"System Default Language" },
	{ 0x0436, UNKNOWN, L"AF", L"Afrikaans" },
	{ 0x041c, UNKNOWN, L"SQ", L"Albanian" },
	{ 0x0401, UNKNOWN, L"AR", L"Arabic (Saudi Arabia)" },
	{ 0x0801, UNKNOWN, L"AR", L"Arabic (Iraq)" },
	{ 0x0c01, UNKNOWN, L"AR", L"Arabic (Egypt)" },
	{ 0x1001, UNKNOWN, L"AR", L"Arabic (Libya)" },
	{ 0x1401, UNKNOWN, L"AR", L"Arabic (Algeria)" },
	{ 0x1801, UNKNOWN, L"AR", L"Arabic (Morocco)" },
	{ 0x1c01, UNKNOWN, L"AR", L"Arabic (Tunisia)" },
	{ 0x2001, UNKNOWN, L"AR", L"Arabic (Oman)" },
	{ 0x2401, UNKNOWN, L"AR", L"Arabic (Yemen)" },
	{ 0x2801, UNKNOWN, L"AR", L"Arabic (Syria)" },
	{ 0x2c01, UNKNOWN, L"AR", L"Arabic (Jordan)" },
	{ 0x3001, UNKNOWN, L"AR", L"Arabic (Lebanon)" },
	{ 0x3401, UNKNOWN, L"AR", L"Arabic (Kuwait)" },
	{ 0x3801, UNKNOWN, L"AR", L"Arabic (U.A.E.)" },
	{ 0x3c01, UNKNOWN, L"AR", L"Arabic (Bahrain)" },
	{ 0x4001, UNKNOWN, L"AR", L"Arabic (Qatar)" },
	{ 0x042b, UNKNOWN, L"HY", L"Armenian" },
	{ 0x042c, UNKNOWN, L"AZ", L"Azeri (Latin)" },
	{ 0x082c, UNKNOWN, L"AZ", L"Azeri (Cyrillic)" },
	{ 0x042d, UNKNOWN, L"ES", L"Basque" },
	{ 0x0423, UNKNOWN, L"BE", L"Belarusian" },
	{ 0x0445, UNKNOWN, L"", L"Bengali (India)" },
	{ 0x141a, UNKNOWN, L"", L"Bosnian (Bosnia and Herzegovina)" },
	{ 0x0402, UNKNOWN, L"BG", L"Bulgarian" },
	{ 0x0455, UNKNOWN, L"", L"Burmese" },
	{ 0x0403, UNKNOWN, L"CA", L"Catalan" },
	{ 0x0404, TRADITIONAL_CHINESE, L"CHT", L"#IME_0404", true },
	{ 0x0804, SIMPLIFIED_CHINESE, L"CHS", L"#IME_0804", true },
	{ 0x0c04, UNKNOWN, L"CH", L"Chinese (Hong Kong SAR, PRC)" },
	{ 0x1004, UNKNOWN, L"CH", L"Chinese (Singapore)" },
	{ 0x1404, UNKNOWN, L"CH", L"Chinese (Macao SAR)" },
	{ 0x041a, UNKNOWN, L"HR", L"Croatian" },
	{ 0x101a, UNKNOWN, L"HR", L"Croatian (Bosnia and Herzegovina)" },
	{ 0x0405, UNKNOWN, L"CZ", L"Czech" },
	{ 0x0406, UNKNOWN, L"DK", L"Danish" },
	{ 0x0465, UNKNOWN, L"MV", L"Divehi" },
	{ 0x0413, UNKNOWN, L"NL", L"Dutch (Netherlands)" },
	{ 0x0813, UNKNOWN, L"BE", L"Dutch (Belgium)" },
	{ 0x0409, ENGLISH, L"EN", L"#IME_0409" },
	{ 0x0809, ENGLISH, L"EN", L"English (United Kingdom)" },
	{ 0x0c09, ENGLISH, L"EN", L"English (Australian)" },
	{ 0x1009, ENGLISH, L"EN", L"English (Canadian)" },
	{ 0x1409, ENGLISH, L"EN", L"English (New Zealand)" },
	{ 0x1809, ENGLISH, L"EN", L"English (Ireland)" },
	{ 0x1c09, ENGLISH, L"EN", L"English (South Africa)" },
	{ 0x2009, ENGLISH, L"EN", L"English (Jamaica)" },
	{ 0x2409, ENGLISH, L"EN", L"English (Caribbean)" },
	{ 0x2809, ENGLISH, L"EN", L"English (Belize)" },
	{ 0x2c09, ENGLISH, L"EN", L"English (Trinidad)" },
	{ 0x3009, ENGLISH, L"EN", L"English (Zimbabwe)" },
	{ 0x3409, ENGLISH, L"EN", L"English (Philippines)" },
	{ 0x0425, UNKNOWN, L"ET", L"Estonian" },
	{ 0x0438, UNKNOWN, L"FO", L"Faeroese" },
	{ 0x0429, UNKNOWN, L"FA", L"Farsi" },
	{ 0x040b, UNKNOWN, L"FI", L"Finnish" },
	{ 0x040c, UNKNOWN, L"FR", L"#IME_040c" },
	{ 0x080c, UNKNOWN, L"FR", L"French (Belgian)" },
	{ 0x0c0c, UNKNOWN, L"FR", L"French (Canadian)" },
	{ 0x100c, UNKNOWN, L"FR", L"French (Switzerland)" },
	{ 0x140c, UNKNOWN, L"FR", L"French (Luxembourg)" },
	{ 0x180c, UNKNOWN, L"FR", L"French (Monaco)" },
	{ 0x0456, UNKNOWN, L"GL", L"Galician" },
	{ 0x0437, UNKNOWN, L"KA", L"Georgian" },
	{ 0x0407, UNKNOWN, L"DE", L"#IME_0407" },
	{ 0x0807, UNKNOWN, L"DE", L"German (Switzerland)" },
	{ 0x0c07, UNKNOWN, L"DE", L"German (Austria)" },
	{ 0x1007, UNKNOWN, L"DE", L"German (Luxembourg)" },
	{ 0x1407, UNKNOWN, L"DE", L"German (Liechtenstein)" },
	{ 0x0408, UNKNOWN, L"GR", L"Greek" },
	{ 0x0447, UNKNOWN, L"IN", L"Gujarati" },
	{ 0x040d, UNKNOWN, L"HE", L"Hebrew" },
	{ 0x0439, UNKNOWN, L"HI", L"Hindi" },
	{ 0x040e, UNKNOWN, L"HU", L"Hungarian" },
	{ 0x040f, UNKNOWN, L"IS", L"Icelandic" },
	{ 0x0421, UNKNOWN, L"ID", L"Indonesian" },
	{ 0x0434, UNKNOWN, L"", L"isiXhosa/Xhosa (South Africa)" },
	{ 0x0435, UNKNOWN, L"", L"isiZulu/Zulu (South Africa)" },
	{ 0x0410, UNKNOWN, L"IT", L"#IME_0410" },
	{ 0x0810, UNKNOWN, L"IT", L"Italian (Switzerland)" },
	{ 0x0411, JAPANESE, L"JP", L"#IME_0411" },
	{ 0x044b, UNKNOWN, L"IN", L"Kannada" },
	{ 0x0457, UNKNOWN, L"IN", L"Konkani" },
	{ 0x0412, KOREAN, L"KR", L"#IME_0412" },
	{ 0x0812, UNKNOWN, L"KR", L"Korean (Johab)" },
	{ 0x0440, UNKNOWN, L"KZ", L"Kyrgyz." },
	{ 0x0426, UNKNOWN, L"LV", L"Latvian" },
	{ 0x0427, UNKNOWN, L"LT", L"Lithuanian" },
	{ 0x0827, UNKNOWN, L"LT", L"Lithuanian (Classic)" },
	{ 0x042f, UNKNOWN, L"MK", L"FYRO Macedonian" },
	{ 0x043e, UNKNOWN, L"MY", L"Malay (Malaysian)" },
	{ 0x083e, UNKNOWN, L"MY", L"Malay (Brunei Darussalam)" },
	{ 0x044c, UNKNOWN, L"IN", L"Malayalam (India)" },
	{ 0x0481, UNKNOWN, L"", L"Maori (New Zealand)" },
	{ 0x043a, UNKNOWN, L"", L"Maltese (Malta)" },
	{ 0x044e, UNKNOWN, L"IN", L"Marathi" },
	{ 0x0450, UNKNOWN, L"MN", L"Mongolian" },
	{ 0x0414, UNKNOWN, L"NO", L"Norwegian (Bokmal)" },
	{ 0x0814, UNKNOWN, L"NO", L"Norwegian (Nynorsk)" },
	{ 0x0415, UNKNOWN, L"PL", L"Polish" },
	{ 0x0416, UNKNOWN, L"PT", L"Portuguese (Brazil)" },
	{ 0x0816, UNKNOWN, L"PT", L"Portuguese (Portugal)" },
	{ 0x0446, UNKNOWN, L"IN", L"Punjabi" },
	{ 0x046b, UNKNOWN, L"", L"Quechua (Bolivia)" },
	{ 0x086b, UNKNOWN, L"", L"Quechua (Ecuador)" },
	{ 0x0c6b, UNKNOWN, L"", L"Quechua (Peru)" },
	{ 0x0418, UNKNOWN, L"RO", L"Romanian" },
	{ 0x0419, UNKNOWN, L"RU", L"#IME_0419" },
	{ 0x044f, UNKNOWN, L"IN", L"Sanskrit" },
	{ 0x043b, UNKNOWN, L"", L"Sami, Northern (Norway)" },
	{ 0x083b, UNKNOWN, L"", L"Sami, Northern (Sweden)" },
	{ 0x0c3b, UNKNOWN, L"", L"Sami, Northern (Finland)" },
	{ 0x103b, UNKNOWN, L"", L"Sami, Lule (Norway)" },
	{ 0x143b, UNKNOWN, L"", L"Sami, Lule (Sweden)" },
	{ 0x183b, UNKNOWN, L"", L"Sami, Southern (Norway)" },
	{ 0x1c3b, UNKNOWN, L"", L"Sami, Southern (Sweden)" },
	{ 0x203b, UNKNOWN, L"", L"Sami, Skolt (Finland)" },
	{ 0x243b, UNKNOWN, L"", L"Sami, Inari (Finland)" },
	{ 0x0c1a, UNKNOWN, L"SR", L"Serbian (Cyrillic)" },
	{ 0x1c1a, UNKNOWN, L"SR", L"Serbian (Cyrillic, Bosnia, and Herzegovina)" },
	{ 0x081a, UNKNOWN, L"SR", L"Serbian (Latin)" },
	{ 0x181a, UNKNOWN, L"SR", L"Serbian (Latin, Bosnia, and Herzegovina)" },
	{ 0x046c, UNKNOWN, L"", L"Sesotho sa Leboa/Northern Sotho (South Africa)" },
	{ 0x0432, UNKNOWN, L"", L"Setswana/Tswana (South Africa)" },
	{ 0x041b, UNKNOWN, L"SK", L"Slovak" },
	{ 0x0424, UNKNOWN, L"SI", L"Slovenian" },
	{ 0x040a, UNKNOWN, L"ES", L"#IME_040a" },
	{ 0x080a, UNKNOWN, L"ES", L"Spanish (Mexican)" },
	{ 0x0c0a, UNKNOWN, L"ES", L"Spanish (Spain, Modern Sort)" },
	{ 0x100a, UNKNOWN, L"ES", L"Spanish (Guatemala)" },
	{ 0x140a, UNKNOWN, L"ES", L"Spanish (Costa Rica)" },
	{ 0x180a, UNKNOWN, L"ES", L"Spanish (Panama)" },
	{ 0x1c0a, UNKNOWN, L"ES", L"Spanish (Dominican Republic)" },
	{ 0x200a, UNKNOWN, L"ES", L"Spanish (Venezuela)" },
	{ 0x240a, UNKNOWN, L"ES", L"Spanish (Colombia)" },
	{ 0x280a, UNKNOWN, L"ES", L"Spanish (Peru)" },
	{ 0x2c0a, UNKNOWN, L"ES", L"Spanish (Argentina)" },
	{ 0x300a, UNKNOWN, L"ES", L"Spanish (Ecuador)" },
	{ 0x340a, UNKNOWN, L"ES", L"Spanish (Chile)" },
	{ 0x380a, UNKNOWN, L"ES", L"Spanish (Uruguay)" },
	{ 0x3c0a, UNKNOWN, L"ES", L"Spanish (Paraguay)" },
	{ 0x400a, UNKNOWN, L"ES", L"Spanish (Bolivia)" },
	{ 0x440a, UNKNOWN, L"ES", L"Spanish (El Salvador)" },
	{ 0x480a, UNKNOWN, L"ES", L"Spanish (Honduras)" },
	{ 0x4c0a, UNKNOWN, L"ES", L"Spanish (Nicaragua)" },
	{ 0x500a, UNKNOWN, L"ES", L"Spanish (Puerto Rico)" },
	{ 0x0430, UNKNOWN, L"", L"Sutu" },
	{ 0x0441, UNKNOWN, L"KE", L"Swahili (Kenya)" },
	{ 0x041d, UNKNOWN, L"SV", L"Swedish" },
	{ 0x081d, UNKNOWN, L"SV", L"Swedish (Finland)" },
	{ 0x045a, UNKNOWN, L"SY", L"Syriac" },
	{ 0x0449, UNKNOWN, L"IN", L"Tamil" },
	{ 0x0444, UNKNOWN, L"RU", L"Tatar (Tatarstan)" },
	{ 0x044a, UNKNOWN, L"IN", L"Telugu" },
	{ 0x041e, UNKNOWN, L"TH", L"#IME_041e" },
	{ 0x041f, UNKNOWN, L"TR", L"Turkish" },
	{ 0x0422, UNKNOWN, L"UA", L"Ukrainian" },
	{ 0x0420, UNKNOWN, L"PK", L"Urdu (Pakistan)" },
	{ 0x0820, UNKNOWN, L"IN", L"Urdu (India)" },
	{ 0x0443, UNKNOWN, L"UZ", L"Uzbek (Latin)" },
	{ 0x0843, UNKNOWN, L"UZ", L"Uzbek (Cyrillic)" },
	{ 0x042a, UNKNOWN, L"VN", L"Vietnamese" },
	{ 0x0452, UNKNOWN, L"", L"Welsh (United Kingdom)" },
};

static LanguageIds *GetLanguageInfo(unsigned short id)
{
	for (int j = 0; j < sizeof(g_LanguageIds) / sizeof(g_LanguageIds[0]); ++j)
	{
		if (g_LanguageIds[j].id == id)
		{
			return &g_LanguageIds[j];
			break;
		}
	}

	return NULL;
}

static bool IsIDInList(unsigned short id, int count, HKL *list)
{
	for (int i = 0; i < count; ++i)
	{
		if (LOWORD(list[i]) == id)
		{
			return true;
		}
	}

	return false;
}

static const wchar_t *GetLanguageName(unsigned short id)
{
	wchar_t const *name = L"???";

	for (int j = 0; j < sizeof(g_LanguageIds) / sizeof(g_LanguageIds[0]); ++j)
	{
		if (g_LanguageIds[j].id == id)
		{
			name = g_LanguageIds[j].displayname;
			break;
		}
	}

	return name;
}

void CInputWin32::SetIMEWindow(void *hwnd)
{
#ifndef _X360
	_imeWnd = hwnd;
#endif
}

void *CInputWin32::GetIMEWindow(void)
{
#ifndef _X360
	return _imeWnd;
#else
	return NULL;
#endif
}

static void SpewIMEInfo(int langid)
{
	LanguageIds *info = GetLanguageInfo(langid);

	if (info)
	{
		wchar_t const *name = info->shortcode ? info->shortcode : L"???";
		wchar_t outstr[512];
		_snwprintf(outstr, sizeof(outstr) / sizeof(wchar_t), L"IME language changed to:  %s", name);
		OutputDebugStringW(outstr);
		OutputDebugStringW(L"\n");
	}
}

void CInputWin32::OnChangeIME(bool forward)
{
#ifndef _X360
	HKL currentKb = GetKeyboardLayout(0);
	UINT numKBs = GetKeyboardLayoutList(0, NULL);

	if (numKBs > 0)
	{
		HKL *list = new HKL [numKBs];
		GetKeyboardLayoutList(numKBs, list);

		int oldKb = 0;
		CUtlVector<HKL> selections;

		for (unsigned int i = 0; i < numKBs; ++i)
		{
			BOOL first = !IsIDInList(LOWORD(list[i]), i, list);

			if (!first)
				continue;

			selections.AddToTail(list[i]);

			if (list[i] == currentKb)
				oldKb = selections.Count() - 1;
		}

		oldKb += forward ? 1 : -1;

		if (oldKb < 0)
		{
			oldKb = max(0, selections.Count() - 1);
		}
		else if (oldKb >= selections.Count())
		{
			oldKb = 0;
		}

		ActivateKeyboardLayout(selections[oldKb], 0);

		int langid = LOWORD(selections[oldKb]);
		SpewIMEInfo(langid);

		delete [] list;
	}
#endif
}

int CInputWin32::GetCurrentIMEHandle(void)
{
#ifndef _X360
	HKL hkl = (HKL)GetKeyboardLayout(0);
	return (int)hkl;
#else
	return 0;
#endif
}

int CInputWin32::GetEnglishIMEHandle(void)
{
#ifndef _X360
	HKL hkl = (HKL)0x04090409;
	return (int)hkl;
#else
	return 0;
#endif
}

void CInputWin32::OnChangeIMEByHandle(int handleValue)
{
#ifndef _X360
	HKL hkl = (HKL)handleValue;
	ActivateKeyboardLayout(hkl, 0);

	int langid = LOWORD(hkl);
	SpewIMEInfo(langid);
#endif
}

void CInputWin32::GetIMELanguageName(wchar_t *buf, int unicodeBufferSizeInBytes)
{
#ifndef _X360
	wchar_t const *name = GetLanguageName(LOWORD(GetKeyboardLayout(0)));
	wcsncpy(buf, name, unicodeBufferSizeInBytes / sizeof(wchar_t) - 1);
	buf[unicodeBufferSizeInBytes / sizeof(wchar_t) - 1] = L'\0';
#else
	buf[0] = L'\0';
#endif
}

void CInputWin32::GetIMELanguageShortCode(wchar_t *buf, int unicodeBufferSizeInBytes)
{
#ifndef _X360
	LanguageIds *info = GetLanguageInfo(LOWORD(GetKeyboardLayout(0)));

	if (!info)
	{
		buf[0] = L'\0';
	}
	else
	{
		wcsncpy(buf, info->shortcode, unicodeBufferSizeInBytes / sizeof(wchar_t) - 1);
		buf[unicodeBufferSizeInBytes / sizeof(wchar_t) - 1] = L'\0';
	}
#else
	buf[0] = L'\0';
#endif
}

int CInputWin32::GetIMELanguageList(LanguageItem *dest, int destcount)
{
#ifndef _X360
	int iret = 0;
	UINT numKBs = GetKeyboardLayoutList(0, NULL);

	if (numKBs > 0)
	{
		HKL *list = new HKL [numKBs];
		GetKeyboardLayoutList(numKBs, list);

		CUtlVector<HKL> selections;

		for (unsigned int i = 0; i < numKBs; ++i)
		{
			BOOL first = !IsIDInList(LOWORD(list[i]), i, list);

			if (!first)
				continue;

			selections.AddToTail(list[i]);
		}

		iret = selections.Count();

		if (dest)
		{
			int langid = LOWORD(GetKeyboardLayout(0));

			for (int i = 0; i < min(iret, destcount); ++i)
			{
				HKL hkl = selections[i];
				IInput::LanguageItem *p = &dest[i];

				LanguageIds *info = GetLanguageInfo(LOWORD(hkl));
				memset(p, 0, sizeof(IInput::LanguageItem));

				wcsncpy(p->shortname, info->shortcode, sizeof(p->shortname) / sizeof(wchar_t));
				p->shortname[sizeof(p->shortname) / sizeof(wchar_t) - 1] = L'\0';

				wcsncpy(p->menuname, info->displayname, sizeof(p->menuname) / sizeof(wchar_t));
				p->menuname[sizeof(p->menuname) / sizeof(wchar_t) - 1] = L'\0';

				p->handleValue = (int)hkl;
				p->active = (info->id == langid) ? true : false;
			}
		}

		delete [] list;
	}

	return iret;
#else
	return 0;
#endif
}

#ifndef _X360

struct IMESettingsTransform
{
	IMESettingsTransform(unsigned int cmr, unsigned int cma, unsigned int smr, unsigned int sma) : cmode_remove(cmr), cmode_add(cma), smode_remove(smr), smode_add(sma)
	{
	}

	void Apply(HWND hwnd)
	{
		HIMC hImc = ImmGetContext(hwnd);

		if (hImc)
		{
			DWORD dwConvMode, dwSentMode;
			ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);

			dwConvMode &= ~cmode_remove;
			dwSentMode &= ~smode_remove;

			dwConvMode |= cmode_add;
			dwSentMode |= smode_add;

			ImmSetConversionStatus(hImc, dwConvMode, dwSentMode);
			ImmReleaseContext(hwnd, hImc);
		}
	}

	bool ConvMatches(DWORD convFlags)
	{
		convFlags &= (cmode_remove | cmode_add);

		if (convFlags & cmode_remove)
			return false;

		if ((convFlags == cmode_add) || (convFlags & cmode_add))
			return true;

		return false;
	}

	bool SentMatches(DWORD sentFlags)
	{
		sentFlags &= (smode_remove | smode_add);

		if (sentFlags & smode_remove)
			return false;

		if ((sentFlags & smode_add) == smode_add)
			return true;

		return false;
	}

	unsigned int cmode_remove;
	unsigned int cmode_add;
	unsigned int smode_remove;
	unsigned int smode_add;
};

static IMESettingsTransform g_ConversionMode_CHT_ToChinese(IME_CMODE_ALPHANUMERIC, IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, 0, 0);
static IMESettingsTransform g_ConversionMode_CHT_ToEnglish(IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, IME_CMODE_ALPHANUMERIC, 0, 0);
static IMESettingsTransform g_ConversionMode_CHS_ToChinese(IME_CMODE_ALPHANUMERIC, IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, 0, 0);
static IMESettingsTransform g_ConversionMode_CHS_ToEnglish(IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, IME_CMODE_ALPHANUMERIC, 0, 0);
static IMESettingsTransform g_ConversionMode_KO_ToKorean(IME_CMODE_ALPHANUMERIC, IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, 0, 0);
static IMESettingsTransform g_ConversionMode_KO_ToEnglish(IME_CMODE_NATIVE | IME_CMODE_LANGUAGE, IME_CMODE_ALPHANUMERIC, 0, 0);
static IMESettingsTransform g_ConversionMode_JP_Hiragana(IME_CMODE_ALPHANUMERIC | IME_CMODE_KATAKANA, IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, 0, 0);
static IMESettingsTransform g_ConversionMode_JP_DirectInput(IME_CMODE_NATIVE | (IME_CMODE_KATAKANA | IME_CMODE_LANGUAGE) | IME_CMODE_FULLSHAPE | IME_CMODE_ROMAN, IME_CMODE_ALPHANUMERIC, 0, 0);
static IMESettingsTransform g_ConversionMode_JP_FullwidthKatakana(IME_CMODE_ALPHANUMERIC, IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_ROMAN | IME_CMODE_KATAKANA | IME_CMODE_LANGUAGE, 0, 0);
static IMESettingsTransform g_ConversionMode_JP_HalfwidthKatakana(IME_CMODE_ALPHANUMERIC | IME_CMODE_FULLSHAPE, IME_CMODE_NATIVE | IME_CMODE_ROMAN | (IME_CMODE_KATAKANA | IME_CMODE_LANGUAGE), 0, 0);
static IMESettingsTransform g_ConversionMode_JP_FullwidthAlphanumeric(IME_CMODE_NATIVE | (IME_CMODE_KATAKANA | IME_CMODE_LANGUAGE), IME_CMODE_ALPHANUMERIC | IME_CMODE_FULLSHAPE | IME_CMODE_ROMAN, 0, 0);
static IMESettingsTransform g_ConversionMode_JP_HalfwidthAlphanumeric(IME_CMODE_NATIVE | (IME_CMODE_KATAKANA | IME_CMODE_LANGUAGE) | IME_CMODE_FULLSHAPE, IME_CMODE_ALPHANUMERIC | IME_CMODE_ROMAN, 0, 0);

#endif

int CInputWin32::GetIMEConversionModes(ConversionModeItem *dest, int destcount)
{
#ifndef _X360
	if (dest)
		memset(dest, 0, destcount * sizeof(ConversionModeItem));

	DWORD dwConvMode = 0, dwSentMode = 0;
	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}

	LanguageIds *info = GetLanguageInfo(LOWORD(GetKeyboardLayout(0)));

	switch (info->languageflag)
	{
		default:
		{
			return 0;
		}

		case TRADITIONAL_CHINESE:
		{
			if (dest)
			{
				ConversionModeItem *item;
				int i = 0;
				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_Chinese", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_CHT_ToChinese;
				item->active = g_ConversionMode_CHT_ToChinese.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_English", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_CHT_ToEnglish;
				item->active = g_ConversionMode_CHT_ToEnglish.ConvMatches(dwConvMode);
			}

			return 2;
		}

		case JAPANESE:
		{
			if (dest)
			{
				ConversionModeItem *item;

				int i = 0;
				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_Hiragana", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_Hiragana;
				item->active = g_ConversionMode_JP_Hiragana.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_FullWidthKatakana", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_FullwidthKatakana;
				item->active = g_ConversionMode_JP_FullwidthKatakana.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_FullWidthAlphanumeric", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_FullwidthAlphanumeric;
				item->active = g_ConversionMode_JP_FullwidthAlphanumeric.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_HalfWidthKatakana", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_HalfwidthKatakana;
				item->active = g_ConversionMode_JP_HalfwidthKatakana.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_HalfWidthAlphanumeric", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_HalfwidthAlphanumeric;
				item->active = g_ConversionMode_JP_HalfwidthAlphanumeric.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_English", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_JP_DirectInput;
				item->active = g_ConversionMode_JP_DirectInput.ConvMatches(dwConvMode);
			}

			return 6;
		}

		case KOREAN:
		{
			if (dest)
			{
				ConversionModeItem *item;
				int i = 0;
				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_Korean", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_KO_ToKorean;
				item->active = g_ConversionMode_KO_ToKorean.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_English", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_KO_ToEnglish;
				item->active = g_ConversionMode_KO_ToEnglish.ConvMatches(dwConvMode);
			}

			return 2;
		}

		case SIMPLIFIED_CHINESE:
		{
			if (dest)
			{
				ConversionModeItem *item;
				int i = 0;
				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_Chinese", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_CHS_ToChinese;
				item->active = g_ConversionMode_CHS_ToChinese.ConvMatches(dwConvMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_English", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_ConversionMode_CHS_ToEnglish;
				item->active = g_ConversionMode_CHS_ToEnglish.ConvMatches(dwConvMode);
			}

			return 2;
		}
	}
#endif
	return 0;
}

#ifndef _X360

static IMESettingsTransform g_SentenceMode_JP_None(0, 0, IME_SMODE_PLAURALCLAUSE | IME_SMODE_SINGLECONVERT | IME_SMODE_AUTOMATIC | IME_SMODE_PHRASEPREDICT | IME_SMODE_CONVERSATION, IME_SMODE_NONE);
static IMESettingsTransform g_SentenceMode_JP_General(0, 0, IME_SMODE_NONE | IME_SMODE_PLAURALCLAUSE | IME_SMODE_SINGLECONVERT | IME_SMODE_AUTOMATIC | IME_SMODE_CONVERSATION, IME_SMODE_PHRASEPREDICT);
static IMESettingsTransform g_SentenceMode_JP_BiasNames(0, 0, IME_SMODE_NONE | IME_SMODE_PHRASEPREDICT | IME_SMODE_SINGLECONVERT | IME_SMODE_AUTOMATIC | IME_SMODE_CONVERSATION, IME_SMODE_PLAURALCLAUSE);
static IMESettingsTransform g_SentenceMode_JP_BiasSpeech(0, 0, IME_SMODE_NONE | IME_SMODE_PHRASEPREDICT | IME_SMODE_SINGLECONVERT | IME_SMODE_AUTOMATIC | IME_SMODE_PLAURALCLAUSE, IME_SMODE_CONVERSATION);

#endif

int CInputWin32::GetIMESentenceModes(SentenceModeItem *dest, int destcount)
{
#ifndef _X360
	if (dest)
		memset(dest, 0, destcount * sizeof(SentenceModeItem));

	DWORD dwConvMode = 0, dwSentMode = 0;
	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}

	LanguageIds *info = GetLanguageInfo(LOWORD(GetKeyboardLayout(0)));

	switch (info->languageflag)
	{
		default:
		{
			return 0;
		}

		case JAPANESE:
		{
			if (dest)
			{
				SentenceModeItem *item;

				int i = 0;
				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_General", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_SentenceMode_JP_General;
				item->active = g_SentenceMode_JP_General.SentMatches(dwSentMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_BiasNames", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_SentenceMode_JP_BiasNames;
				item->active = g_SentenceMode_JP_BiasNames.SentMatches(dwSentMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_BiasSpeech", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_SentenceMode_JP_BiasSpeech;
				item->active = g_SentenceMode_JP_BiasSpeech.SentMatches(dwSentMode);

				item = &dest[i++];
				wcsncpy(item->menuname, L"#IME_NoConversion", sizeof(item->menuname) / sizeof(wchar_t));
				item->handleValue = (int)&g_SentenceMode_JP_None;
				item->active = g_SentenceMode_JP_None.SentMatches(dwSentMode);
			}

			return 4;
		}
	}
#endif
	return 0;
}

void CInputWin32::OnChangeIMEConversionModeByHandle(int handleValue)
{
#ifndef _X360
	if (handleValue == 0)
		return;

	IMESettingsTransform *txform = (IMESettingsTransform *)handleValue;
	txform->Apply((HWND)GetIMEWindow());
#endif
}

void CInputWin32::OnChangeIMESentenceModeByHandle(int handleValue)
{
}

void CInputWin32::OnInputLanguageChanged(void)
{
}

void CInputWin32::OnIMEStartComposition(void)
{
}

void CInputWin32::OnIMEComposition(int flags)
{
#ifndef _X360
	HIMC hIMC = ImmGetContext((HWND)GetIMEWindow());

	if (hIMC)
	{
		if (flags & VGUI_GCS_RESULTSTR)
		{
			wchar_t tempstr[64];
			int len = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, (LPVOID)tempstr, sizeof(tempstr));

			if (len > 0)
			{
				if ((len % 2) != 0)
					len++;

				int numchars = len / sizeof(wchar_t);

				for (int i = 0; i < numchars; ++i)
				{
					PostKeyMessage(new KeyValues("KeyTyped", "unichar", tempstr[i]));
				}
			}

			len = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, (LPVOID)tempstr, sizeof(tempstr));

			if (len == 0)
			{
				InternalSetCompositionString(L"");
				InternalHideCandidateWindow();
				DestroyCandidateList();
			}
		}

		if (flags & VGUI_GCS_COMPSTR)
		{
			wchar_t tempstr[512];
			int len = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, (LPVOID)tempstr, sizeof(tempstr));

			if (len > 0)
			{
				if ((len % 2) != 0)
					len++;

				int numchars = len / sizeof(wchar_t);
				tempstr[numchars] = L'\0';

				InternalSetCompositionString(tempstr);

				DestroyCandidateList();
				CreateNewCandidateList();

				InternalShowCandidateWindow();
			}
		}

		ImmReleaseContext((HWND)GetIMEWindow(), hIMC);
	}
#endif
}

void CInputWin32::OnIMEEndComposition(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext)
	{
		PostKeyMessage(new KeyValues("DoCompositionString", "string", L""));
	}
}

void CInputWin32::DestroyCandidateList(void)
{
#ifndef _X360
	if (_imeCandidates)
	{
		delete [] (char *)_imeCandidates;
		_imeCandidates = null;
	}
#endif
}

void CInputWin32::OnIMEShowCandidates(void)
{
#ifndef _X360
	DestroyCandidateList();
	CreateNewCandidateList();

	InternalShowCandidateWindow();
#endif
}

void CInputWin32::OnIMECloseCandidates(void)
{
#ifndef _X360
	InternalHideCandidateWindow();
	DestroyCandidateList();
#endif
}

void CInputWin32::OnIMEChangeCandidates(void)
{
#ifndef _X360
	DestroyCandidateList();
	CreateNewCandidateList();

	InternalUpdateCandidateWindow();
#endif
}

void CInputWin32::CreateNewCandidateList(void)
{
#ifndef _X360
	Assert(!_imeCandidates);

	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		DWORD buflen = ImmGetCandidateListW(hImc, 0, NULL, 0);

		if (buflen > 0)
		{
			char *buf = new char [buflen];
			Q_memset(buf, 0, buflen);

			CANDIDATELIST *list = (CANDIDATELIST *)buf;
			DWORD copyBytes = ImmGetCandidateListW(hImc, 0, list, buflen);

			if (copyBytes > 0)
			{
				_imeCandidates = list;
			}
			else
			{
				delete [] buf;
			}
		}

		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}
#endif
}

int CInputWin32::GetCandidateListCount(void)
{
#ifndef _X360
	if (!_imeCandidates)
		return 0;

	return (int)_imeCandidates->dwCount;
#else
	return 0;
#endif
}

void CInputWin32::GetCandidate(int num, wchar_t *dest, int destSizeBytes)
{
	dest[0] = L'\0';
#ifndef _X360
	if (num < 0 || num >= (int)_imeCandidates->dwCount)
	{
		return;
	}

	DWORD offset = *(DWORD *)((char *)(_imeCandidates->dwOffset + num));
	wchar_t *s = (wchar_t *)((char *)_imeCandidates + offset);

	wcsncpy(dest, s, destSizeBytes / sizeof(wchar_t) - 1);
	dest[destSizeBytes / sizeof(wchar_t) - 1] = L'\0';
#endif
}

int CInputWin32::GetCandidateListSelectedItem(void)
{
#ifndef _X360
	if (!_imeCandidates)
		return 0;

	return (int)_imeCandidates->dwSelection;
#else
	return 0;
#endif
}

int CInputWin32::GetCandidateListPageSize(void)
{
#ifndef _X360
	if (!_imeCandidates)
		return 0;
	return (int)_imeCandidates->dwPageSize;
#else
	return 0;
#endif
}

int CInputWin32::GetCandidateListPageStart(void)
{
#ifndef _X360
	if (!_imeCandidates)
		return 0;
	return (int)_imeCandidates->dwPageStart;
#else
	return 0;
#endif
}

void CInputWin32::SetCandidateListPageStart(int start)
{
#ifndef _X360
	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, 0, start);
		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}
#endif
}

void CInputWin32::GetCompositionString(wchar_t *dest, int destSizeBytes)
{
	dest[0] = L'\0';
#ifndef _X360
	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		int len = ImmGetCompositionStringW(hImc, GCS_COMPSTR, (LPVOID)dest, destSizeBytes);

		if (len > 0)
		{
			if ((len % 2) != 0)
				len++;

			int numchars = len / sizeof(wchar_t);
			dest[numchars] = L'\0';
		}

		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}
#endif
}

void CInputWin32::CancelCompositionString(void)
{
#ifndef _X360
	HIMC hImc = ImmGetContext((HWND)GetIMEWindow());

	if (hImc)
	{
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		ImmSetCompositionStringW(hImc, SCS_SETSTR, L"", 2, L"", 2);
		ImmReleaseContext((HWND)GetIMEWindow(), hImc);
	}
#endif
}

void CInputWin32::OnIMERecomputeModes(void)
{
}

void CInputWin32::OnIMESelectCandidate(int num)
{
	if (num < 0 || num > 9)
		return;

	BYTE nVirtKey = '0' + num;

	keybd_event(nVirtKey, 0, 0, 0);
	keybd_event(nVirtKey, 0, KEYEVENTF_KEYUP, 0);
}

bool CInputWin32::CandidateListStartsAtOne(void)
{
#ifndef _X360
	DWORD prop = ImmGetProperty(GetKeyboardLayout(0), IGP_PROPERTY);

	if (prop & IME_PROP_CANDLIST_START_FROM_1)
	{
		return true;
	}
#endif
	return false;
}

void CInputWin32::SetCandidateWindowPos(int x, int y)
{
#ifndef _X360
	POINT point;
	CANDIDATEFORM Candidate;

	point.x = x;
	point.y = y;

	HIMC hIMC = ImmGetContext((HWND)GetIMEWindow());

	if (hIMC)
	{
		Candidate.dwIndex = 0;
		Candidate.dwStyle = CFS_FORCE_POSITION;
		Candidate.ptCurrentPos.x = point.x;
		Candidate.ptCurrentPos.y = point.y;
		ImmSetCandidateWindow(hIMC, &Candidate);

		ImmReleaseContext((HWND)GetIMEWindow(),hIMC);
	}
#endif
}

void CInputWin32::InternalSetCompositionString(const wchar_t *compstr)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext)
	{
		PostKeyMessage(new KeyValues("DoCompositionString", "string", compstr));
	}
}

void CInputWin32::InternalShowCandidateWindow(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext)
	{
		PostKeyMessage(new KeyValues("DoShowIMECandidates"));
	}
}

void CInputWin32::InternalHideCandidateWindow(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext)
	{
		PostKeyMessage(new KeyValues("DoHideIMECandidates"));
	}
}

void CInputWin32::InternalUpdateCandidateWindow(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext)
	{
		PostKeyMessage(new KeyValues("DoUpdateIMECandidates"));
	}
}

bool CInputWin32::GetShouldInvertCompositionString(void)
{
#ifndef _X360
	LanguageIds *info = GetLanguageInfo(LOWORD(GetKeyboardLayout(0)));

	if (!info)
		return false;

	return info->invertcomposition;
#else
	return false;
#endif
}

void CInputWin32::RegisterKeyCodeUnhandledListener(VPANEL panel)
{
	if (!panel)
		return;

	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	VPanel *listener = (VPanel *)panel;

	if (pContext->m_KeyCodeUnhandledListeners.Find(listener) == pContext->m_KeyCodeUnhandledListeners.InvalidIndex())
	{
		pContext->m_KeyCodeUnhandledListeners.AddToTail(listener);
	}
}

void CInputWin32::UnregisterKeyCodeUnhandledListener(VPANEL panel)
{
	if (!panel)
		return;

	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	VPanel *listener = (VPanel *)panel;

	pContext->m_KeyCodeUnhandledListeners.FindAndRemove(listener);
}

void CInputWin32::OnKeyCodeUnhandled(int keyCode)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	int c = pContext->m_KeyCodeUnhandledListeners.Count();

	for (int i = 0; i < c; ++i)
	{
		VPanel *listener = pContext->m_KeyCodeUnhandledListeners[i];
		g_pIVgui->PostMessage((VPANEL)listener, new KeyValues("KeyCodeUnhandled", "code", keyCode), NULL);
	}
}

void CInputWin32::PostModalSubTreeMessage(VPanel *subTree, bool state)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (pContext->m_pModalSubTree == NULL)
		return;

	KeyValues *kv = new KeyValues("ModalSubTree", "state", state ? 1 : 0);
	g_pIVgui->PostMessage((VPANEL)pContext->m_pModalSubTree, kv, NULL);
}

void CInputWin32::SetModalSubTree(VPANEL subTree, VPANEL unhandledMouseClickListener, bool restrictMessagesToSubTree)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	if (pContext->m_pModalSubTree && pContext->m_pModalSubTree != (VPanel *)subTree)
	{
		ReleaseModalSubTree();
	}

	if (!subTree)
		return;

	pContext->m_pModalSubTree = (VPanel *)subTree;
	pContext->m_pUnhandledMouseClickListener = (VPanel *)unhandledMouseClickListener;
	pContext->m_bRestrictMessagesToModalSubTree = restrictMessagesToSubTree;

	PostModalSubTreeMessage(pContext->m_pModalSubTree, true);
}

void CInputWin32::ReleaseModalSubTree(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	if (pContext->m_pModalSubTree)
	{
		PostModalSubTreeMessage(pContext->m_pModalSubTree, false);
	}

	pContext->m_pModalSubTree = NULL;
	pContext->m_pUnhandledMouseClickListener = NULL;
	pContext->m_bRestrictMessagesToModalSubTree = false;
}

VPANEL CInputWin32::GetModalSubTree(void)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return 0;

	return (VPANEL)pContext->m_pModalSubTree;
}

void CInputWin32::SetModalSubTreeReceiveMessages(bool state)
{
	InputContext_t *pContext = GetInputContext(m_hContext);

	if (!pContext)
		return;

	Assert(pContext->m_pModalSubTree);

	if (!pContext->m_pModalSubTree)
		return;

	pContext->m_bRestrictMessagesToModalSubTree = state;
}

bool CInputWin32::ShouldModalSubTreeReceiveMessages(void) const
{
	InputContext_t *pContext = const_cast<CInputWin32 *>(this)->GetInputContext(m_hContext);

	if (!pContext)
		return true;

	return pContext->m_bRestrictMessagesToModalSubTree;
}

void IInput::SetIMEWindow(void *hwnd)
{
	g_Input.SetIMEWindow(hwnd);
}

void *IInput::GetIMEWindow(void)
{
	return g_Input.GetIMEWindow();
}

void IInput::OnChangeIME(bool forward)
{
	g_Input.OnChangeIME(forward);
}

int IInput::GetCurrentIMEHandle(void)
{
	return g_Input.GetCurrentIMEHandle();
}

int IInput::GetEnglishIMEHandle(void)
{
	return g_Input.GetEnglishIMEHandle();
}

void IInput::GetIMELanguageName(wchar_t *buf, int unicodeBufferSizeInBytes)
{
	g_Input.GetIMELanguageName(buf, unicodeBufferSizeInBytes);
}

void IInput::GetIMELanguageShortCode(wchar_t *buf, int unicodeBufferSizeInBytes)
{
	g_Input.GetIMELanguageShortCode(buf, unicodeBufferSizeInBytes);
}

int IInput::GetIMELanguageList(LanguageItem *dest, int destcount)
{
	return g_Input.GetIMELanguageList(dest, destcount);
}

int IInput::GetIMEConversionModes(ConversionModeItem *dest, int destcount)
{
	return g_Input.GetIMEConversionModes(dest, destcount);
}

int IInput::GetIMESentenceModes(SentenceModeItem *dest, int destcount)
{
	return g_Input.GetIMESentenceModes(dest, destcount);
}

void IInput::OnChangeIMEByHandle(int handleValue)
{
	g_Input.OnChangeIMEByHandle(handleValue);
}

void IInput::OnChangeIMEConversionModeByHandle(int handleValue)
{
	g_Input.OnChangeIMEConversionModeByHandle(handleValue);
}

void IInput::OnChangeIMESentenceModeByHandle(int handleValue)
{
	g_Input.OnChangeIMESentenceModeByHandle(handleValue);
}

void IInput::OnInputLanguageChanged(void)
{
	g_Input.OnInputLanguageChanged();
}

void IInput::OnIMEStartComposition(void)
{
	g_Input.OnIMEStartComposition();
}

void IInput::OnIMEComposition(int flags)
{
	g_Input.OnIMEComposition(flags);
}

void IInput::OnIMEEndComposition(void)
{
	g_Input.OnIMEEndComposition();
}

void IInput::OnIMEShowCandidates(void)
{
	g_Input.OnIMEShowCandidates();
}

void IInput::OnIMEChangeCandidates(void)
{
	g_Input.OnIMEChangeCandidates();
}

void IInput::OnIMECloseCandidates(void)
{
	g_Input.OnIMECloseCandidates();
}

void IInput::OnIMERecomputeModes(void)
{
	g_Input.OnIMERecomputeModes();
}

void IInput::OnIMESelectCandidate(int num)
{
	g_Input.OnIMESelectCandidate(num);
}

int IInput::GetCandidateListCount(void)
{
	return g_Input.GetCandidateListCount();
}

void IInput::GetCandidate(int num, wchar_t *dest, int destSizeBytes)
{
	g_Input.GetCandidate(num, dest, destSizeBytes);
}

int IInput::GetCandidateListSelectedItem(void)
{
	return g_Input.GetCandidateListSelectedItem();
}

int IInput::GetCandidateListPageSize(void)
{
	return g_Input.GetCandidateListPageSize();
}

int IInput::GetCandidateListPageStart(void)
{
	return g_Input.GetCandidateListPageStart();
}

void IInput::SetCandidateWindowPos(int x, int y)
{
	g_Input.SetCandidateWindowPos(x, y);
}

bool IInput::GetShouldInvertCompositionString(void)
{
	return g_Input.GetShouldInvertCompositionString();
}

bool IInput::CandidateListStartsAtOne(void)
{
	return g_Input.CandidateListStartsAtOne();
}

void IInput::SetCandidateListPageStart(int start)
{
	g_Input.SetCandidateListPageStart(start);
}

void IInput::GetCompositionString(wchar_t *dest, int destSizeBytes)
{
	g_Input.GetCompositionString(dest, destSizeBytes);
}

void IInput::CancelCompositionString(void)
{
	g_Input.CancelCompositionString();
}

void IInput::SetMouseCaptureEx(VPANEL panel, MouseCode captureStartMouseCode)
{
	g_Input.SetMouseCaptureEx(panel, captureStartMouseCode);
}

void IInput::RegisterKeyCodeUnhandledListener(VPANEL panel)
{
	g_Input.RegisterKeyCodeUnhandledListener(panel);
}

void IInput::UnregisterKeyCodeUnhandledListener(VPANEL panel)
{
	g_Input.UnregisterKeyCodeUnhandledListener(panel);
}

void IInput::OnKeyCodeUnhandled(int keyCode)
{
	g_Input.OnKeyCodeUnhandled(keyCode);
}

void IInput::SetModalSubTree(VPANEL subTree, VPANEL unhandledMouseClickListener, bool restrictMessagesToSubTree)
{
	g_Input.SetModalSubTree(subTree, unhandledMouseClickListener, restrictMessagesToSubTree);
}

void IInput::ReleaseModalSubTree(void)
{
	g_Input.ReleaseModalSubTree();
}

VPANEL IInput::GetModalSubTree(void)
{
	return g_Input.GetModalSubTree();
}

void IInput::SetModalSubTreeReceiveMessages(bool state)
{
	g_Input.SetModalSubTreeReceiveMessages(state);
}

bool IInput::ShouldModalSubTreeReceiveMessages(void) const
{
	return g_Input.ShouldModalSubTreeReceiveMessages();
}

VPANEL IInput::GetMouseCapture(void)
{
	return g_Input.GetMouseCapture();
}

void IInputInternal::UpdateCursorPosInternal(int x, int y)
{
	g_Input.UpdateCursorPosInternal(x, y);
}

void IInputInternal::PostCursorMessage(void)
{
	g_Input.PostCursorMessage();
}

void IInputInternal::HandleExplicitSetCursor(void)
{
	g_Input.HandleExplicitSetCursor();
}