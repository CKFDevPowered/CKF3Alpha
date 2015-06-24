#include <wtypes.h>
#include <winuser.h>
#include <tier0/dbg.h>
#include "vgui_key_translation.h"

static vgui::KeyCode s_pVirtualKeyToKeyCode[256];
static int s_pKeyCodeToVirtual[vgui::KEY_LAST];

void KeyCode_InitKeyTranslationTable(void)
{
	memset(s_pVirtualKeyToKeyCode, vgui::KEY_NONE, sizeof(s_pVirtualKeyToKeyCode));

	s_pVirtualKeyToKeyCode['0'] = vgui::KEY_0;
	s_pVirtualKeyToKeyCode['1'] = vgui::KEY_1;
	s_pVirtualKeyToKeyCode['2'] = vgui::KEY_2;
	s_pVirtualKeyToKeyCode['3'] = vgui::KEY_3;
	s_pVirtualKeyToKeyCode['4'] = vgui::KEY_4;
	s_pVirtualKeyToKeyCode['5'] = vgui::KEY_5;
	s_pVirtualKeyToKeyCode['6'] = vgui::KEY_6;
	s_pVirtualKeyToKeyCode['7'] = vgui::KEY_7;
	s_pVirtualKeyToKeyCode['8'] = vgui::KEY_8;
	s_pVirtualKeyToKeyCode['9'] = vgui::KEY_9;
	s_pVirtualKeyToKeyCode['A'] = vgui::KEY_A;
	s_pVirtualKeyToKeyCode['B'] = vgui::KEY_B;
	s_pVirtualKeyToKeyCode['C'] = vgui::KEY_C;
	s_pVirtualKeyToKeyCode['D'] = vgui::KEY_D;
	s_pVirtualKeyToKeyCode['E'] = vgui::KEY_E;
	s_pVirtualKeyToKeyCode['F'] = vgui::KEY_F;
	s_pVirtualKeyToKeyCode['G'] = vgui::KEY_G;
	s_pVirtualKeyToKeyCode['H'] = vgui::KEY_H;
	s_pVirtualKeyToKeyCode['I'] = vgui::KEY_I;
	s_pVirtualKeyToKeyCode['J'] = vgui::KEY_J;
	s_pVirtualKeyToKeyCode['K'] = vgui::KEY_K;
	s_pVirtualKeyToKeyCode['L'] = vgui::KEY_L;
	s_pVirtualKeyToKeyCode['M'] = vgui::KEY_M;
	s_pVirtualKeyToKeyCode['N'] = vgui::KEY_N;
	s_pVirtualKeyToKeyCode['O'] = vgui::KEY_O;
	s_pVirtualKeyToKeyCode['P'] = vgui::KEY_P;
	s_pVirtualKeyToKeyCode['Q'] = vgui::KEY_Q;
	s_pVirtualKeyToKeyCode['R'] = vgui::KEY_R;
	s_pVirtualKeyToKeyCode['S'] = vgui::KEY_S;
	s_pVirtualKeyToKeyCode['T'] = vgui::KEY_T;
	s_pVirtualKeyToKeyCode['U'] = vgui::KEY_U;
	s_pVirtualKeyToKeyCode['V'] = vgui::KEY_V;
	s_pVirtualKeyToKeyCode['W'] = vgui::KEY_W;
	s_pVirtualKeyToKeyCode['X'] = vgui::KEY_X;
	s_pVirtualKeyToKeyCode['Y'] = vgui::KEY_Y;
	s_pVirtualKeyToKeyCode['Z'] = vgui::KEY_Z;
	s_pVirtualKeyToKeyCode[VK_NUMPAD0] = vgui::KEY_PAD_0;
	s_pVirtualKeyToKeyCode[VK_NUMPAD1] = vgui::KEY_PAD_1;
	s_pVirtualKeyToKeyCode[VK_NUMPAD2] = vgui::KEY_PAD_2;
	s_pVirtualKeyToKeyCode[VK_NUMPAD3] = vgui::KEY_PAD_3;
	s_pVirtualKeyToKeyCode[VK_NUMPAD4] = vgui::KEY_PAD_4;
	s_pVirtualKeyToKeyCode[VK_NUMPAD5] = vgui::KEY_PAD_5;
	s_pVirtualKeyToKeyCode[VK_NUMPAD6] = vgui::KEY_PAD_6;
	s_pVirtualKeyToKeyCode[VK_NUMPAD7] = vgui::KEY_PAD_7;
	s_pVirtualKeyToKeyCode[VK_NUMPAD8] = vgui::KEY_PAD_8;
	s_pVirtualKeyToKeyCode[VK_NUMPAD9] = vgui::KEY_PAD_9;
	s_pVirtualKeyToKeyCode[VK_DIVIDE] = vgui::KEY_PAD_DIVIDE;
	s_pVirtualKeyToKeyCode[VK_MULTIPLY] = vgui::KEY_PAD_MULTIPLY;
	s_pVirtualKeyToKeyCode[VK_SUBTRACT] = vgui::KEY_PAD_MINUS;
	s_pVirtualKeyToKeyCode[VK_ADD] = vgui::KEY_PAD_PLUS;
	s_pVirtualKeyToKeyCode[0xd] = vgui::KEY_PAD_ENTER;
	s_pVirtualKeyToKeyCode[VK_DECIMAL] = vgui::KEY_PAD_DECIMAL;

	s_pVirtualKeyToKeyCode[0xdb] = vgui::KEY_LBRACKET;
	s_pVirtualKeyToKeyCode[0xdd] = vgui::KEY_RBRACKET;
	s_pVirtualKeyToKeyCode[0xba] = vgui::KEY_SEMICOLON;
	s_pVirtualKeyToKeyCode[0xde] = vgui::KEY_APOSTROPHE;
	s_pVirtualKeyToKeyCode[0xc0] = vgui::KEY_BACKQUOTE;
	s_pVirtualKeyToKeyCode[0xbc] = vgui::KEY_COMMA;
	s_pVirtualKeyToKeyCode[0xbe] = vgui::KEY_PERIOD;
	s_pVirtualKeyToKeyCode[0xbf] = vgui::KEY_SLASH;
	s_pVirtualKeyToKeyCode[0xdc] = vgui::KEY_BACKSLASH;
	s_pVirtualKeyToKeyCode[0xbd] = vgui::KEY_MINUS;
	s_pVirtualKeyToKeyCode[0xbb] = vgui::KEY_EQUAL;
	s_pVirtualKeyToKeyCode[VK_RETURN] = vgui::KEY_ENTER;
	s_pVirtualKeyToKeyCode[VK_SPACE] = vgui::KEY_SPACE;
	s_pVirtualKeyToKeyCode[VK_BACK] = vgui::KEY_BACKSPACE;
	s_pVirtualKeyToKeyCode[VK_TAB] = vgui::KEY_TAB;
	s_pVirtualKeyToKeyCode[0x14] = vgui::KEY_CAPSLOCK;
	s_pVirtualKeyToKeyCode[0x90] = vgui::KEY_NUMLOCK;
	s_pVirtualKeyToKeyCode[VK_ESCAPE] = vgui::KEY_ESCAPE;
	s_pVirtualKeyToKeyCode[VK_SCROLL] = vgui::KEY_SCROLLLOCK;
	s_pVirtualKeyToKeyCode[VK_INSERT] = vgui::KEY_INSERT;
	s_pVirtualKeyToKeyCode[VK_DELETE] = vgui::KEY_DELETE;
	s_pVirtualKeyToKeyCode[VK_HOME] = vgui::KEY_HOME;
	s_pVirtualKeyToKeyCode[VK_END] = vgui::KEY_END;
	s_pVirtualKeyToKeyCode[VK_PRIOR] = vgui::KEY_PAGEUP;
	s_pVirtualKeyToKeyCode[VK_NEXT] = vgui::KEY_PAGEDOWN;
	s_pVirtualKeyToKeyCode[0x13] = vgui::KEY_BREAK;
	s_pVirtualKeyToKeyCode[0x10] = vgui::KEY_LSHIFT;
	s_pVirtualKeyToKeyCode[0x10] = vgui::KEY_RSHIFT;
	s_pVirtualKeyToKeyCode[0x12] = vgui::KEY_LALT;
	s_pVirtualKeyToKeyCode[0x12] = vgui::KEY_RALT;
	s_pVirtualKeyToKeyCode[0x11] = vgui::KEY_LCONTROL;
	s_pVirtualKeyToKeyCode[0x11] = vgui::KEY_RCONTROL;
	s_pVirtualKeyToKeyCode[VK_LWIN] = vgui::KEY_LWIN;
	s_pVirtualKeyToKeyCode[VK_RWIN] = vgui::KEY_RWIN;
	s_pVirtualKeyToKeyCode[VK_APPS] = vgui::KEY_APP;
	s_pVirtualKeyToKeyCode[VK_UP] = vgui::KEY_UP;
	s_pVirtualKeyToKeyCode[VK_LEFT] = vgui::KEY_LEFT;
	s_pVirtualKeyToKeyCode[VK_DOWN] = vgui::KEY_DOWN;
	s_pVirtualKeyToKeyCode[VK_RIGHT] = vgui::KEY_RIGHT;

	s_pVirtualKeyToKeyCode[VK_F1] = vgui::KEY_F1;
	s_pVirtualKeyToKeyCode[VK_F2] = vgui::KEY_F2;
	s_pVirtualKeyToKeyCode[VK_F3] = vgui::KEY_F3;
	s_pVirtualKeyToKeyCode[VK_F4] = vgui::KEY_F4;
	s_pVirtualKeyToKeyCode[VK_F5] = vgui::KEY_F5;
	s_pVirtualKeyToKeyCode[VK_F6] = vgui::KEY_F6;
	s_pVirtualKeyToKeyCode[VK_F7] = vgui::KEY_F7;
	s_pVirtualKeyToKeyCode[VK_F8] = vgui::KEY_F8;
	s_pVirtualKeyToKeyCode[VK_F9] = vgui::KEY_F9;
	s_pVirtualKeyToKeyCode[VK_F10] = vgui::KEY_F10;
	s_pVirtualKeyToKeyCode[VK_F11] = vgui::KEY_F11;
	s_pVirtualKeyToKeyCode[VK_F12] = vgui::KEY_F12;

	for (int i = 0; i < vgui::KEY_LAST; i++)
		s_pKeyCodeToVirtual[s_pVirtualKeyToKeyCode[i]] = i;

	s_pKeyCodeToVirtual[0] = 0;
}

vgui::KeyCode KeyCode_VirtualKeyToVGUI(int key)
{
	if (key < 0 || key >= sizeof(s_pVirtualKeyToKeyCode) / sizeof(s_pVirtualKeyToKeyCode[0]))
	{
		Assert(false);
		return vgui::KEY_NONE;
	}

	return s_pVirtualKeyToKeyCode[key];
}

int KeyCode_VGUIToVirtualKey(vgui::KeyCode code)
{
	return s_pKeyCodeToVirtual[code];
}