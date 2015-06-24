#include <windows.h>
#include <VGUI/KeyCode.h>

wchar_t *UTF8ToUnicodeEx(const char *str)
{
	wchar_t *result;
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t *)malloc(sizeof(wchar_t)*(len+1));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, result, len);
	result[len] = L'\0';
	return result;
}

wchar_t *UTF8ToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, result, len);
	result[len] = L'\0';
	return result;
}

wchar_t *ANSIToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, result, len);
	result[len] = '\0';
	return result;
}

char *UnicodeToUTF8(const wchar_t *str)
{
	static char result[1024];
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, len, NULL, NULL);
	result[len] = '\0';
	return result;
}

char *UnicodeToANSI(const wchar_t *str)
{
	static char result[1024];
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, len, NULL, NULL);
	result[len] = '\0';
	return result;
}

namespace vgui
{

wchar_t *ConvertVGUIKeyCodeToUnicodeString(vgui::KeyCode code)
{
	switch (code)
	{
		case KEY_NONE:return L"NOT BOUND";
		case KEY_0: return L"0";
		case KEY_1: return L"1";
		case KEY_2: return L"2";
		case KEY_3: return L"3";
		case KEY_4: return L"4";
		case KEY_5: return L"5";
		case KEY_6: return L"6";
		case KEY_7: return L"7";
		case KEY_8: return L"8";
		case KEY_9: return L"9";
		case KEY_A: return L"a";
		case KEY_B: return L"b";
		case KEY_C: return L"c";
		case KEY_D: return L"d";
		case KEY_E: return L"e";
		case KEY_F: return L"f";
		case KEY_G: return L"g";
		case KEY_H: return L"h";
		case KEY_I: return L"i";
		case KEY_J: return L"j";
		case KEY_K: return L"k";
		case KEY_L: return L"l";
		case KEY_M: return L"m";
		case KEY_N: return L"n";
		case KEY_O: return L"o";
		case KEY_P: return L"p";
		case KEY_Q: return L"q";
		case KEY_R: return L"r";
		case KEY_S: return L"s";
		case KEY_T: return L"t";
		case KEY_U: return L"u";
		case KEY_V: return L"v";
		case KEY_W: return L"w";
		case KEY_X: return L"x";
		case KEY_Y: return L"y";
		case KEY_Z: return L"z";
		case KEY_PAD_0: return L"PAD 0";
		case KEY_PAD_1: return L"PAD 1";
		case KEY_PAD_2: return L"PAD 2";
		case KEY_PAD_3: return L"PAD 3";
		case KEY_PAD_4: return L"PAD 4";
		case KEY_PAD_5: return L"PAD 5";
		case KEY_PAD_6: return L"PAD 6";
		case KEY_PAD_7: return L"PAD 7";
		case KEY_PAD_8: return L"PAD 8";
		case KEY_PAD_9: return L"PAD 9";
		case KEY_PAD_DIVIDE: return L"PAD /";
		case KEY_PAD_MINUS: return L"PAD -";
		case KEY_PAD_PLUS: return L"PAD +";
		case KEY_PAD_ENTER: return L"PAD Enter";
		case KEY_PAD_DECIMAL: return L"Pad DEL";
		case KEY_PAD_MULTIPLY: return L"PAD *";
		case KEY_LBRACKET: return L"[";
		case KEY_RBRACKET: return L"]";
		case KEY_SEMICOLON: return L";";
		case KEY_APOSTROPHE: return L"\'";
		case KEY_BACKQUOTE: return L"`";
		case KEY_COMMA: return L",";
		case KEY_PERIOD: return L".";
		case KEY_SLASH: return L"/";
		case KEY_BACKSLASH: return L"\\";
		case KEY_MINUS: return L"-";
		case KEY_EQUAL: return L"=";
		case KEY_ENTER: return L"Enter";
		case KEY_SPACE: return L"Space";
		case KEY_BACKSPACE: return L"Backspace";
		case KEY_TAB: return L"TAB";
		case KEY_CAPSLOCK: return L"CAPSLOCK";
		case KEY_ESCAPE: return L"ESC";
		case KEY_INSERT: return L"INS";
		case KEY_DELETE: return L"DEL";
		case KEY_HOME: return L"HOME";
		case KEY_END: return L"END";
		case KEY_PAGEUP: return L"PAGE UP";
		case KEY_PAGEDOWN: return L"PAGE DOWN";
		case KEY_BREAK: return L"PAUSE";
		case KEY_LSHIFT: return L"Left SHIFT";
		case KEY_RSHIFT: return L"Right SHIFT";
		case KEY_LALT: return L"Left ALT";
		case KEY_RALT: return L"Right ALT";
		case KEY_LCONTROL: return L"Left CTRL";
		case KEY_RCONTROL: return L"Right CTRL";
		case KEY_UP: return L"¡ü";
		case KEY_LEFT: return L"¡û";
		case KEY_DOWN: return L"¡ý";
		case KEY_RIGHT: return L"¡ú";
		case KEY_F1: return L"F1";
		case KEY_F2: return L"F2";
		case KEY_F3: return L"F3";
		case KEY_F4: return L"F4";
		case KEY_F5: return L"F5";
		case KEY_F6: return L"F6";
		case KEY_F7: return L"F7";
		case KEY_F8: return L"F8";
		case KEY_F9: return L"F9";
		case KEY_F10: return L"F10";
		case KEY_F11: return L"F11";
		case KEY_F12: return L"F12";

		case KEY_NUMLOCK:
		case KEY_LWIN:
		case KEY_RWIN:
		case KEY_APP:
		case KEY_SCROLLLOCK:
		case KEY_CAPSLOCKTOGGLE:
		case KEY_NUMLOCKTOGGLE:
		case KEY_SCROLLLOCKTOGGLE: return L"";
	}

	return L"";
}

};