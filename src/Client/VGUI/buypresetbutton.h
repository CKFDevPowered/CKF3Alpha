#ifndef BUYPRESETPANELBUTTON_H
#define BUYPRESETPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <filesystem.h>
#include "shared_util.h"

class BuyPresetImagePanel;

class BuyPresetButton : public vgui::Button
{
	typedef vgui::Button BaseClass;

public:
	BuyPresetButton(Panel *parent, const char *panelName);
	~BuyPresetButton(void);

public:
	void ClearWeapons(void);
	void SetPrimaryWeapon(vgui::IImage *image);
	void SetSecondaryWeapon(vgui::IImage *image);
	void SetKnifeWeapon(vgui::IImage *image);

protected:
	BuyPresetImagePanel *m_pImagePanel;
};

#endif