#ifndef TFCLASSMENU_H
#define TFCLASSMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <game_controls/TFImageButton.h>
#include <game_controls/classmenu.h>
#include <vgui_controls/KeyRepeat.h>

namespace vgui
{
	class Label;
	class Button;
	class ImagePanel;
	class ScalableImagePanel;
	class ModelPanel;
	class TFImageButton;
	class TFClassTipsListPanel;
	class CKeyRepeatHandler;

	class TFClassMenuButton : public TFImageButton
	{
	public:
		typedef TFImageButton BaseClass;

		TFClassMenuButton( vgui::Panel *parent, const char *panelName, EditablePanel *panel ) : BaseClass(parent, panelName, panel)
		{
		}

		virtual bool LoadClassPage(void);

		int GetClassIndex(void){return m_iClassIndex;}

	private:
		int m_iClassIndex;
	};
}

#define CLASS_COUNT_IMAGES 11

class CTFClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CTFClassMenu, CClassMenu );

public:
	CTFClassMenu();

	void SetTeam( int iTeam ){m_iTeam = iTeam;}

	virtual void Update( void );
	virtual void PaintBackground( void );
	virtual void SetVisible( bool state );
	virtual void PerformLayout();
	virtual Panel *CreateControlByName(const char *controlName);

	virtual void OnClose();
	virtual void ShowPanel( bool bShow );
	virtual void OnCommand(const char *command);
	virtual bool NeedsUpdate(void) { return true; }

	int GetSelectedClass( void );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnKeyCodePressed( vgui::KeyCode code );
	virtual void OnKeyCodeReleased( vgui::KeyCode code );
	virtual void OnThink();
	virtual void UpdateNumClassLabels( int iTeam );

	MESSAGE_FUNC(OnTick, "Tick");

protected:
	vgui::Label					*m_pClassCount[9];

	vgui::ScalableImagePanel	*m_pLocalPlayerBG;
	vgui::ImagePanel			*m_pLocalPlayerImage;
	vgui::ImagePanel			*m_pClassCountImages[CLASS_COUNT_IMAGES];
	vgui::Label					*m_pCountLabel;
	vgui::Label					*m_pClassMenuSelectLabel;

	vgui::EditablePanel			*m_pClassTipsPanel;

	vgui::ModelPanel			*m_pModelPanel;

	vgui::IImage				*m_pImageClassImage[2][10];

private:
	vgui::KeyCode				m_iClassMenuKey;
	vgui::CKeyRepeatHandler		m_KeyRepeat;
};

#endif