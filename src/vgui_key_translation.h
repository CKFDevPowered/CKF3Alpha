#ifndef VGUI_KEY_TRANSLATION_H
#define VGUI_KEY_TRANSLATION_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/keycode.h>

void KeyCode_InitKeyTranslationTable(void);
vgui::KeyCode KeyCode_VirtualKeyToVGUI(int key);
int KeyCode_VGUIToVirtualKey(vgui::KeyCode keycode);

#endif