#include "hud.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "ammohistory.h"

#include "weapon_csbase.h"

#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>
#include "vgui_controls/buildgroup.h"
#include "vgui_controls/BitmapImagePanel.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/TextImage.h"
#include "vgui_controls/RichText.h"
#include "vgui_controls/QueryBox.h"
#include "buy_presets/buy_presets.h"
#include "buypreset_weaponsetlabel.h"

#include "shared_util.h"

using namespace vgui;

#define DEBUG_DRAW_IMAGE_RECT 0

WeaponImageInfo::WeaponImageInfo(void)
{
	m_needLayout = false;
	m_isCentered = true;
	m_left = m_top = m_wide = m_tall = 0;
	m_isPrimary = false;
	memset(&m_weapon, 0, sizeof(ImageInfo));
	memset(&m_ammo, 0, sizeof(ImageInfo));
	m_weaponScale = m_ammoScale = 0;
	m_pAmmoText = new TextImage("");
}

WeaponImageInfo::~WeaponImageInfo(void)
{
	delete m_pAmmoText;
}

void WeaponImageInfo::ApplyTextSettings(vgui::IScheme *pScheme, bool isProportional)
{
	Color color = pScheme->GetColor("FgColor", Color(0, 0, 0, 0));

	m_pAmmoText->SetColor(color);
	m_pAmmoText->SetFont(pScheme->GetFont("Default", isProportional));
	m_pAmmoText->SetWrap(false);
}

void WeaponImageInfo::SetBounds(int left, int top, int wide, int tall)
{
	m_left = left;
	m_top = top;
	m_wide = wide;
	m_tall = tall;
	m_needLayout = true;
}

void WeaponImageInfo::SetCentered(bool isCentered)
{
	m_isCentered = isCentered;
	m_needLayout = true;
}

void WeaponImageInfo::SetScaleAt1024(int weaponScale, int ammoScale)
{
	m_weaponScale = weaponScale;
	m_ammoScale = ammoScale;
	m_needLayout = true;
}

void WeaponImageInfo::SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool useCurrentAmmoType)
{
	WeaponIdType weaponID = pWeapon->GetWeaponID();

	if (weaponID != WEAPON_NONE)
	{
		m_weapon.image = scheme()->GetImage(ImageFnameFromWeaponID(weaponID, m_isPrimary), true);
		m_isPrimary = isPrimary;
	}
	else
	{
		m_weapon.image = NULL;
		m_ammo.image = NULL;
	}

	m_needLayout = true;
	m_pAmmoText->SetText(L"");
}

void WeaponImageInfo::PaintBackground(void)
{
	if (m_needLayout)
		PerformLayout();

	m_weapon.Paint();
	m_ammo.Paint();
}

void WeaponImageInfo::Paint(void)
{
	if (m_needLayout)
		PerformLayout();

	m_pAmmoText->Paint();
}

void WeaponImageInfo::PerformLayout(void)
{
	m_needLayout = false;
	m_weapon.FitInBounds(m_left, m_top, m_wide, m_tall, m_isCentered, m_weaponScale);
#if 0
	int ammoX = min(m_wide * 5 / 6, m_weapon.w);
	int ammoSize = m_tall * 9 / 16;

	if (!m_isPrimary)
	{
		ammoSize = ammoSize * 25 / 40;
		ammoX = min(m_wide * 5 / 6, m_weapon.w * 3 / 4);
	}

	if (ammoX + ammoSize > m_wide)
		ammoX = m_wide - ammoSize;

	m_ammo.FitInBounds(m_left + ammoX, m_top + m_tall - ammoSize, ammoSize, ammoSize, false, m_ammoScale);

	int w, h;
	m_pAmmoText->ResizeImageToContent();
	m_pAmmoText->GetSize(w, h);

	if (m_isPrimary)
	{
		if (m_ammoScale < 75)
		{
			m_pAmmoText->SetPos(m_left + ammoX + ammoSize * 1.25f - w, m_top + m_tall - h);
		}
		else
		{
			m_pAmmoText->SetPos(m_left + ammoX + ammoSize - w, m_top + m_tall - h);
		}
	}
	else
		m_pAmmoText->SetPos(m_left + ammoX + ammoSize, m_top + m_tall - h);
#endif
}

WeaponLabel::WeaponLabel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	SetSize(10, 10);
	SetMouseInputEnabled(false);
}

WeaponLabel::~WeaponLabel(void)
{
}

void WeaponLabel::SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary)
{
	BuyPresetWeapon weapon(WEAPON_NONE);

	if (pWeapon)
		weapon = *pWeapon;

	m_weapon.SetWeapon(&weapon, isPrimary, false);
}

void WeaponLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_weapon.ApplyTextSettings(pScheme, IsProportional());
}

void WeaponLabel::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);
	m_weapon.SetBounds(0, 0, wide, tall);
}

void WeaponLabel::Paint(void)
{
	BaseClass::Paint();

	m_weapon.Paint();
}

void WeaponLabel::PaintBackground(void)
{
	BaseClass::PaintBackground();

	m_weapon.PaintBackground();
}

ItemImageInfo::ItemImageInfo(void)
{
	m_needLayout = false;
	m_left = m_top = m_wide = m_tall = 0;
	m_count = 0;
	memset(&m_image, 0, sizeof(ImageInfo));

	m_pText = new TextImage("");
	m_pText->SetWrap(false);

	SetBounds(0, 0, 100, 100);
}

ItemImageInfo::~ItemImageInfo(void)
{
	delete m_pText;
}

void ItemImageInfo::ApplyTextSettings(vgui::IScheme *pScheme, bool isProportional)
{
	Color color = pScheme->GetColor("Label.TextColor", Color(0, 0, 0, 0));

	m_pText->SetColor(color);
	m_pText->SetFont(pScheme->GetFont("Default", isProportional));
	m_pText->SetWrap(false);
}

void ItemImageInfo::SetBounds(int left, int top, int wide, int tall)
{
	m_left = left;
	m_top = top;
	m_wide = wide;
	m_tall = tall;
	m_needLayout = true;
}

void ItemImageInfo::SetItem(const char *imageFname, int count)
{
	m_count = count;

	if (imageFname)
		m_image.image = scheme()->GetImage(imageFname, true);
	else
		m_image.image = NULL;

	if (count > 1)
	{
		wchar_t *multiplierString = g_pVGuiLocalize->Find("#Cstrike_BuyMenuPresetMultiplier");

		if (!multiplierString)
			multiplierString = L"";

		const int BufLen = 32;
		wchar_t buf[BufLen];

		g_pVGuiLocalize->ConstructString(buf, sizeof(buf), multiplierString, 1, NumAsWString(count));
		m_pText->SetText(buf);
	}
	else
		m_pText->SetText(L"");

	m_needLayout = true;
}

void ItemImageInfo::PaintBackground(void)
{
	if (m_needLayout)
		PerformLayout();

	m_image.Paint();
}

void ItemImageInfo::Paint(void)
{
	if (m_needLayout)
		PerformLayout();

	m_pText->Paint();

#if DEBUG_DRAW_IMAGE_RECT == 1
	int x, y, w, h;
	m_pText->GetPos(x, y);
	m_pText->GetSize(w, h);

	if (w > 0 && h > 0)
	{
		surface()->DrawSetColor(Color(0, 255, 0, 128));
		surface()->DrawOutlinedRect(x, y, x + w, y + h);
	}

	surface()->DrawSetColor(Color(255, 255, 255, 128));
	surface()->DrawOutlinedRect(m_left, m_top, m_left + m_wide, m_top + m_tall);
#endif
}

void ItemImageInfo::SetTextColor(Color color)
{
	m_pText->SetColor(color);
}

void ItemImageInfo::SetTextFont(vgui::HFont font)
{
	m_pText->SetFont(font);
}

void ItemImageInfo::PerformLayout(void)
{
	m_needLayout = false;
	m_image.FitInBounds(m_left, m_top, m_wide, m_tall, true, 0);

	int w, h;
	m_pText->ResizeImageToContent();
	m_pText->GetSize(w, h);

	if (w > 0 && h > 0)
	{
		int x = m_image.x + (m_image.w - (w * 1.25));
		int y = m_image.y + (m_image.h - (h * 1.25));

		m_pText->SetPos(x, y);
	}
}

EquipmentLabel::EquipmentLabel(Panel *parent, const char *panelName, int maxSlot) : BaseClass(parent, panelName)
{
	for (int i = 0; i < maxSlot; i++)
		m_items[m_items.AddToTail()] = new ItemImageInfo;

	SetSize(10, 10);
	SetMouseInputEnabled(false);
}

EquipmentLabel::~EquipmentLabel(void)
{
	for (int i = 0; i < m_items.Size(); i++)
		delete m_items[i];

	m_items.RemoveAll();
}

void EquipmentLabel::Reset(void)
{
	for (int i = 0; i < m_items.Size(); i++)
		m_items[i]->SetItem(NULL, 0);
}

void EquipmentLabel::SetItem(int index, const char *imageFname, int count)
{
	index--;
	Assert(m_items.IsValidIndex(index));

	m_items[index]->SetItem(imageFname, count);
	m_items[index]->SetTextColor(m_textColor);
	m_items[index]->SetTextFont(m_textFont);

	InvalidateLayout();
}

void EquipmentLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for (int i = 0; i < m_items.Size(); i++)
		m_items[i]->ApplyTextSettings(pScheme, IsProportional());

	m_textColor = pScheme->GetColor("Label.TextColor", Color(0, 0, 0, 0));
	m_textFont = pScheme->GetFont("Default", IsProportional());
}

void EquipmentLabel::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);

	float offset = wide * 0.025;
	float x = 0 + offset;
	int count = m_items.Size();
	float w = (wide - (offset * 2)) / count;

	for (int i = 0; i < count; i++)
	{
		m_items[i]->SetBounds(x, 0, w, tall);
		x += w;
	}
}

void EquipmentLabel::Paint(void)
{
	BaseClass::PaintBackground();

	for (int i = 0; i < m_items.Size(); i++)
		m_items[i]->Paint();
}

void EquipmentLabel::PaintBackground(void)
{
	BaseClass::Paint();

	for (int i = 0; i < m_items.Size(); i++)
		m_items[i]->PaintBackground();
}

void ImageInfo::Paint(void)
{
	if (!image)
		return;

	int imageWide, imageTall;
	image->GetSize(imageWide, imageTall);

	image->SetSize(w, h);
	image->SetPos(x, y);
	image->Paint();

	image->SetSize(imageWide, imageTall);

#if DEBUG_DRAW_IMAGE_RECT == 1
	surface()->DrawSetColor(Color(255, 255, 0, 128));
	surface()->DrawOutlinedRect(x, y, x + w, y + h);
#endif
}

void ImageInfo::FitInBounds(int baseX, int baseY, int width, int height, bool center, int scaleAt1024, bool halfHeight)
{
	if (!image)
	{
		x = y = w = h = 0;
		return;
	}

	image->GetContentSize(fullW, fullH);

	if (scaleAt1024)
	{
		int screenW, screenH;
		surface()->GetScreenSize(screenW, screenH);

		w = fullW * screenW / 1024 * scaleAt1024 / 100;
		h = fullH * screenW / 1024 * scaleAt1024 / 100;

		if (fullH > 64 && scaleAt1024 == 100)
		{
			w = w * 64 / fullH;
			h = h * 64 / fullH;
		}

		if (h > height * 1.2)
			scaleAt1024 = 0;
	}
	else
	{
		w = fullW;
		h = fullH;

		if (w > width)
		{
			h = (int)h * 1.0f * width / w;
			w = width;
		}

		if (h > height)
		{
			w = (int)w * 1.0f * height / h;
			h = height;
		}
	}

	if (center)
	{
		x = baseX + (width - w) / 2;
	}
	else
	{
		x = baseX;
	}

	y = baseY + (height - h) / 2;
}