#ifndef BUYPRESET_WEAPONSETLABEL_H
#define BUYPRESET_WEAPONSETLABEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IImage.h>

namespace vgui
{
	class TextImage;
	class TextEntry;
};

struct ImageInfo
{
	vgui::IImage *image;
	int w;
	int h;
	int x;
	int y;
	int fullW;
	int fullH;

	void FitInBounds(int baseX, int baseY, int width, int height, bool center, int scaleAt1024, bool halfHeight = false);
	void Paint(void);
};

class WeaponImageInfo
{
public:
	WeaponImageInfo(void);
	~WeaponImageInfo(void);

public:
	void SetBounds(int left, int top, int wide, int tall);
	void SetCentered(bool isCentered);
	void SetScaleAt1024(int weaponScale, int ammoScale);
	void SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool useCurrentAmmoType);
	void ApplyTextSettings(vgui::IScheme *pScheme, bool isProportional);
	void Paint(void);
	void PaintBackground(void);

private:
	void PerformLayout(void);

private:
	int m_left;
	int m_top;
	int m_wide;
	int m_tall;

	bool m_isPrimary;

	int m_weaponScale;
	int m_ammoScale;

	bool m_needLayout;
	bool m_isCentered;
	ImageInfo m_weapon;
	ImageInfo m_ammo;

	vgui::TextImage *m_pAmmoText;
};

class ItemImageInfo
{
public:
	ItemImageInfo(void);
	~ItemImageInfo(void);

public:
	void SetBounds(int left, int top, int wide, int tall);
	void SetItem(const char *imageFname, int count);
	void ApplyTextSettings(vgui::IScheme *pScheme, bool isProportional);
	void Paint(void);
	void PaintBackground(void);
	void SetTextColor(Color color);
	void SetTextFont(vgui::HFont font);

private:
	void PerformLayout(void);

public:
	int m_left;
	int m_top;
	int m_wide;
	int m_tall;

	int m_count;

	bool m_needLayout;
	ImageInfo m_image;

	vgui::TextImage *m_pText;
};

class WeaponLabel : public vgui::ImagePanel
{
	typedef vgui::ImagePanel BaseClass;

public:
	WeaponLabel(vgui::Panel *parent, const char *panelName);
	~WeaponLabel(void);

public:
	void SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout(void);
	virtual void Paint(void);
	virtual void PaintBackground(void);

protected:
	WeaponImageInfo m_weapon;
};

class EquipmentLabel : public vgui::ImagePanel
{
	typedef vgui::ImagePanel BaseClass;

public:
	EquipmentLabel(vgui::Panel *parent, const char *panelName, int maxSlot = 1);
	~EquipmentLabel(void);

public:
	void Reset(void);
	void SetItem(int index, const char *imageFname, int count);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout(void);
	virtual void Paint(void);
	virtual void PaintBackground(void);

protected:
	CUtlVector<ItemImageInfo *> m_items;
	Color m_textColor;
	vgui::HFont m_textFont;
};

class BuyPresetEditPanel : public vgui::EditablePanel
{
	typedef vgui::EditablePanel BaseClass;

public:
	BuyPresetEditPanel(vgui::Panel *parent, const char *panelName, const char *resourceFilename, int fallbackIndex, bool editableName);
	virtual ~BuyPresetEditPanel(void);

public:
	virtual void SetText(const wchar_t *text);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PaintBackground(void);
	virtual void OnCommand(const char *command);
	virtual void OnSizeChanged(int wide, int tall);

public:
	void SetWeaponSet(const WeaponSet *pWeaponSet, bool current);

	void SetPanelBgColor(Color color)
	{
		if (m_pBgPanel)
			m_pBgPanel->SetBgColor(color);
	}

protected:
	void Reset(void);

protected:
	vgui::Panel *m_pBgPanel;

	vgui::TextEntry *m_pTitleEntry;
	vgui::Label *m_pTitleLabel;
	vgui::Label *m_pCostLabel;

	WeaponLabel *m_pPrimaryWeapon;
	WeaponLabel *m_pSecondaryWeapon;
	WeaponLabel *m_pKnifeWeapon;

	EquipmentLabel *m_pHEGrenade;
	EquipmentLabel *m_pSmokeGrenade;
	EquipmentLabel *m_pFlashbangs;

	EquipmentLabel *m_pDefuser;
	EquipmentLabel *m_pNightvision;
	EquipmentLabel *m_pArmor;

	EquipmentLabel *m_pGrenades;
	EquipmentLabel *m_pEquipments;

	int m_baseWide;
	int m_baseTall;

	int m_fallbackIndex;
};

#endif