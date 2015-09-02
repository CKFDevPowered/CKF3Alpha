#ifndef TFIMAGEBUTTON_H
#define TFIMAGEBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Button.h>
#include <vgui_controls/EditablePanel.h>
#include <game_controls/mouseoverpanelbutton.h>

namespace vgui
{

class EditablePanel;

class TFImageButton : public MouseOverPanelButton
{
	typedef MouseOverPanelButton BaseClass;

public:
	~TFImageButton( );
	TFImageButton( vgui::Panel *parent, const char *panelName, EditablePanel *panel );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void GetSettings(KeyValues *outResourceData);
	virtual void PaintBackground();

	void SetDefaultImage(IImage *image);
	void SetDefaultImage(const char *imageName);
	IImage *GetDefaultImage();
	void SetArmedImage(IImage *image);
	void SetArmedImage(const char *imageName);
	IImage *GetArmedImage();
	void SetSelectedImage(IImage *image);
	void SetSelectedImage(const char *imageName);
	IImage *GetSelectedImage();
	void SetDrawColor( Color drawColor );
	Color GetDrawColor( void );
	void SetArmedColor( Color armedColor );
	Color GetArmedColor( void );
	void SetSelectedColor( Color selectedColor );
	Color GetSelectedColor( void );

private:
	char *m_szDefaultImage;
	char *m_szArmedImage;
	char *m_szSelectedImage;
	char *m_pszDrawColorName;
	char *m_pszArmedColorName;
	char *m_pszSelectedColorName;

	IImage *m_pDefaultImage;
	IImage *m_pArmedImage;
	IImage *m_pSelectedImage;

	Color m_DrawColor;
	Color m_ArmedColor;
	Color m_SelectedColor;
};

}

#endif