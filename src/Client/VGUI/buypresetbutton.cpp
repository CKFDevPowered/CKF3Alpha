#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "buypresetbutton.h"
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>

using namespace vgui;

class BuyPresetImagePanel : public EditablePanel
{
	typedef EditablePanel BaseClass;

public:
	BuyPresetImagePanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);

		m_pPrimaryWeapon = new ImagePanel(this, "primary");
		m_pSecondaryWeapon = new ImagePanel(this, "secondary");
		m_pKnifeWeapon = new ImagePanel(this, "knife");
		m_pPrimaryWeapon->SetShouldScaleImage(true);
		m_pPrimaryWeapon->SetShouldCenterImage(true);
	}

	void ClearWeapons(void)
	{
		m_pPrimaryWeapon->SetImage((IImage *)NULL);
		m_pSecondaryWeapon->SetImage((IImage *)NULL);
		m_pKnifeWeapon->SetImage((IImage *)NULL);
	}

	void SetPrimaryWeapon(IImage *image)
	{
		m_pPrimaryWeapon->SetImage(image);
	}

	void SetSecondaryWeapon(IImage *image)
	{
		m_pSecondaryWeapon->SetImage(image);
	}

	void SetKnifeWeapon(IImage *image)
	{
		m_pKnifeWeapon->SetImage(image);
	}

private:
	ImagePanel *m_pPrimaryWeapon;
	ImagePanel *m_pSecondaryWeapon;
	ImagePanel *m_pKnifeWeapon;
};

BuyPresetButton::BuyPresetButton(Panel *parent, const char *panelName) : BaseClass(parent, panelName, "BuyPresetButton")
{
	m_pImagePanel = new BuyPresetImagePanel(this, "ImagePanel");
	m_pImagePanel->LoadControlSettings("UI/BuyPresetButton.res" );
}

BuyPresetButton::~BuyPresetButton(void)
{
	delete m_pImagePanel;
}

void BuyPresetButton::ClearWeapons(void)
{
	m_pImagePanel->ClearWeapons();
}

void BuyPresetButton::SetPrimaryWeapon(IImage *image)
{
	m_pImagePanel->SetPrimaryWeapon(image);
}

void BuyPresetButton::SetSecondaryWeapon(IImage *image)
{
	m_pImagePanel->SetSecondaryWeapon(image);
}

void BuyPresetButton::SetKnifeWeapon(IImage *image)
{
	m_pImagePanel->SetKnifeWeapon(image);
}