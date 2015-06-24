#ifndef BACKGROUNDMENUBUTTON_H
#define BACKGROUNDMENUBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <VGUI/IImage.h>
#include <vgui_controls/Button.h>

class CBackgroundMenuButton : public vgui::Button
{
public:
	CBackgroundMenuButton(vgui::Panel *parent, const char *name);
	~CBackgroundMenuButton(void);

public:
	void SetImages(vgui::IImage *image, vgui::IImage *mouseoverImage)
	{
		m_pImage = image;
		m_pMouseOverImage = mouseoverImage;

		if (image)
		{
			int w, h;
			image->GetSize(w, h);

			SetSize(w, h);
		}
		else
		{
			SetSize(0, 0);
		}

		SetArmed(IsArmed());
	}

	void SetArmed(bool state)
	{
		if (state)
		{
			SetImageAtIndex(0, m_pMouseOverImage, 0);
		}
		else
		{
			SetImageAtIndex(0, m_pImage, 0);
		}

		BaseClass::SetArmed(state);
	}

public:
	virtual void SetVisible(bool state);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	virtual void OnKillFocus(void);
	virtual void OnCommand(const char *command);

protected:
	vgui::Menu *RecursiveLoadGameMenu(KeyValues *datafile);
	vgui::Menu *m_pMenu;

private:
	vgui::IImage *m_pImage, *m_pMouseOverImage;
	typedef vgui::Button BaseClass;
};

#endif