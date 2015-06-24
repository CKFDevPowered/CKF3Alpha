#include "hud.h"
#include "cdll_dll.h"
#include "cl_util.h"

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
#include "buy_presets\buy_presets.h"
#include "buypreset_weaponsetlabel.h"

#include "shared_util.h"
#include <mathlib/Vector4D.h>

using namespace vgui;

const float horizTitleRatio = 18.0f / 68.0f;

static int GetAlternateProportionalValueFromNormal(int normalizedValue)
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);

	int proH, proW;
	surface()->GetProportionalBase(proW, proH);

	double scaleH = (double)tall / (double)proH;
	double scaleW = (double)wide / (double)proW;
	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	return (int)(normalizedValue * scale);
}

static int GetAlternateProportionalValueFromScaled(HScheme hScheme, int scaledValue)
{
	return GetAlternateProportionalValueFromNormal(scheme()->GetProportionalNormalizedValueEx(hScheme, scaledValue));
}

int GetScaledValue(HScheme hScheme, int unscaled)
{
	int val = scheme()->GetProportionalScaledValueEx(hScheme, unscaled);
	return GetAlternateProportionalValueFromScaled(hScheme, val);
}

class PresetBackgroundPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;

public:
	PresetBackgroundPanel(vgui::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
	}

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		SetBorder(pScheme->GetBorder("ButtonBorder"));
		m_lineColor = pScheme->GetColor("Border.Bright", Color(0, 0, 0, 0));
	}

	virtual void ApplySettings(KeyValues *inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);

		m_lines.RemoveAll();

		KeyValues *lines = inResourceData->FindKey("lines", false);

		if (lines)
		{
			KeyValues *line = lines->GetFirstValue();

			while (line)
			{
				const char *str = line->GetString(NULL, "");

				Vector4D p;
				int numPoints = sscanf(str, "%f %f %f %f", &p[0], &p[1], &p[2], &p[3]);

				if (numPoints == 4)
					m_lines.AddToTail(p);

				line = line->GetNextValue();
			}
		}
	}

	virtual void PaintBackground(void)
	{
		BaseClass::PaintBackground();

		vgui::surface()->DrawSetColor(m_lineColor);
		vgui::surface()->DrawSetTextColor(m_lineColor);

		for (int i = 0; i < m_scaledLines.Count(); ++i)
		{
			int x1, x2, y1, y2;

			x1 = m_scaledLines[i][0];
			y1 = m_scaledLines[i][1];
			x2 = m_scaledLines[i][2];
			y2 = m_scaledLines[i][3];

			vgui::surface()->DrawFilledRect(x1, y1, x2, y2);
		}
	}

	virtual void PerformLayout(void)
	{
		m_scaledLines.RemoveAll();

		for (int i = 0; i < m_lines.Count(); ++i)
		{
			int x1, x2, y1, y2;

			x1 = GetScaledValue(GetScheme(), m_lines[i][0]);
			y1 = GetScaledValue(GetScheme(), m_lines[i][1]);
			x2 = GetScaledValue(GetScheme(), m_lines[i][2]);
			y2 = GetScaledValue(GetScheme(), m_lines[i][3]);

			if (x1 == x2)
				++x2;

			if (y1 == y2)
				++y2;

			m_scaledLines.AddToTail(Vector4D(x1, y1, x2, y2));
		}
	}

private:
	Color m_lineColor;
	CUtlVector<Vector4D> m_lines;
	CUtlVector<Vector4D> m_scaledLines;
};

BuyPresetEditPanel::BuyPresetEditPanel(Panel *parent, const char *panelName, const char *resourceFilename, int fallbackIndex, bool editableName) : BaseClass(parent, panelName)
{
	SetProportional(parent->IsProportional());

	m_fallbackIndex = fallbackIndex;
	m_pBgPanel = new PresetBackgroundPanel(this, "mainBackground");
	m_pBgPanel->SetVisible(false);

	m_pTitleEntry = NULL;
	m_pTitleLabel = NULL;
	m_pCostLabel = NULL;

	m_pPrimaryWeapon = new WeaponLabel(this, "primary");
	m_pSecondaryWeapon = new WeaponLabel(this, "secondary");
	m_pKnifeWeapon = new WeaponLabel(this, "knife");

	m_pHEGrenade = new EquipmentLabel(this, "hegrenade");
	m_pSmokeGrenade = new EquipmentLabel(this, "smokegrenade");
	m_pFlashbangs = new EquipmentLabel(this, "flashbang");

	m_pDefuser = new EquipmentLabel(this, "defuser");
	m_pNightvision = new EquipmentLabel(this, "nightvision");
	m_pArmor = new EquipmentLabel(this, "armor");

	m_pGrenades = new EquipmentLabel(this, "grenades", 3);
	m_pEquipments = new EquipmentLabel(this, "equipments", 3);

	m_pPrimaryWeapon->SetVisible(false);
	m_pSecondaryWeapon->SetVisible(false);
	m_pKnifeWeapon->SetVisible(false);

	m_pHEGrenade->SetVisible(false);
	m_pSmokeGrenade->SetVisible(false);
	m_pFlashbangs->SetVisible(false);

	m_pDefuser->SetVisible(false);
	m_pNightvision->SetVisible(false);
	m_pArmor->SetVisible(false);

	m_pGrenades->SetVisible(false);
	m_pEquipments->SetVisible(false);

	if (resourceFilename)
		LoadControlSettings(resourceFilename);
}

BuyPresetEditPanel::~BuyPresetEditPanel(void)
{
}

void BuyPresetEditPanel::SetWeaponSet(const WeaponSet *pWeaponSet, bool current)
{
	Reset();

	if (pWeaponSet)
	{
		if (m_pTitleLabel)
			m_pTitleLabel->SetText(SharedVarArgs("#Cstrike_BuyPresetChoice%d", m_fallbackIndex));

		if (m_pTitleEntry)
			m_pTitleEntry->SetText(SharedVarArgs("#Cstrike_BuyPresetChoice%d", m_fallbackIndex));

		if (m_pCostLabel)
		{
			const int BufLen = 256;
			wchar_t wbuf[BufLen];
			g_pVGuiLocalize->ConstructString(wbuf, sizeof(wbuf), g_pVGuiLocalize->Find("#Cstrike_BuyPresetPlainCost"), 1, NumAsWString(pWeaponSet->FullCost()));
			m_pCostLabel->SetText(wbuf);
		}

		m_pPrimaryWeapon->SetWeapon(&pWeaponSet->m_primaryWeapon, true);
		m_pSecondaryWeapon->SetWeapon(&pWeaponSet->m_secondaryWeapon, false);
		m_pKnifeWeapon->SetWeapon(&pWeaponSet->m_knifeWeapon, false);

		if (pWeaponSet->m_HEGrenade)
			m_pHEGrenade->SetItem(1, "gfx/vgui/hegrenade", pWeaponSet->m_HEGrenade);

		if (pWeaponSet->m_smokeGrenade)
			m_pSmokeGrenade->SetItem(1, "gfx/vgui/smokegrenade", pWeaponSet->m_smokeGrenade);

		if (pWeaponSet->m_flashbangs)
			m_pFlashbangs->SetItem(1, "gfx/vgui/flashbang", pWeaponSet->m_flashbangs);

		if (pWeaponSet->m_defuser)
			m_pDefuser->SetItem(1, "gfx/vgui/defuser", 1);

		if (pWeaponSet->m_nightvision)
			m_pNightvision->SetItem(1, "gfx/vgui/nightvision", 1);

		if (pWeaponSet->m_armor)
		{
			if (pWeaponSet->m_helmet)
				m_pArmor->SetItem(1, "gfx/vgui/kevlar_helmet", 1);
			else
				m_pArmor->SetItem(1, "gfx/vgui/kevlar", 1);
		}

		if (pWeaponSet->m_HEGrenade)
			m_pGrenades->SetItem(1, "gfx/vgui/hegrenade", pWeaponSet->m_HEGrenade);

		if (pWeaponSet->m_flashbangs)
			m_pGrenades->SetItem(2, "gfx/vgui/flashbang", pWeaponSet->m_flashbangs);

		if (pWeaponSet->m_smokeGrenade)
			m_pGrenades->SetItem(3, "gfx/vgui/smokegrenade", pWeaponSet->m_smokeGrenade);

		if (pWeaponSet->m_defuser)
			m_pEquipments->SetItem(1, "gfx/vgui/defuser", 1);

		if (pWeaponSet->m_nightvision)
			m_pEquipments->SetItem(2, "gfx/vgui/nightvision", 1);

		if (pWeaponSet->m_armor)
		{
			if (pWeaponSet->m_helmet)
				m_pEquipments->SetItem(3, "gfx/vgui/kevlar_helmet", 1);
			else
				m_pEquipments->SetItem(3, "gfx/vgui/kevlar", 1);
		}
	}
}

void BuyPresetEditPanel::SetText(const wchar_t *text)
{
	if (!text)
		text = L"";

	if (m_pTitleLabel)
		m_pTitleLabel->SetText(text);

	if (m_pTitleEntry)
		m_pTitleEntry->SetText(text);

	InvalidateLayout();
}

void BuyPresetEditPanel::OnCommand(const char *command)
{
	if (stricmp(command, "close"))
		PostActionSignal(new KeyValues("Command", "command", SharedVarArgs("%s %d", command, m_fallbackIndex)));

	BaseClass::OnCommand(command);
}

void BuyPresetEditPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *child = GetChild(i);

		if (!child)
			continue;

		if (!stricmp("button", child->GetClassName()))
		{
			Button *pButton = dynamic_cast<Button *>(child);

			if (pButton)
			{
				pButton->SetDefaultBorder(NULL);
				pButton->SetDepressedBorder(NULL);
				pButton->SetKeyFocusBorder(NULL);
			}
		}
	}

	IBorder *pBorder = pScheme->GetBorder("BuyPresetButtonBorder");

	const int NumButtons = 4;
	const char *buttonNames[4] = { "editPrimary", "editSecondary", "editGrenades", "editEquipment" };

	for (int i = 0; i < NumButtons; ++i)
	{
		Panel *pPanel = FindChildByName(buttonNames[i]);

		if (pPanel)
		{
			pPanel->SetBorder(pBorder);

			if (!stricmp("button", pPanel->GetClassName()))
			{
				Button *pButton = dynamic_cast<Button *>(pPanel);

				if (pButton)
				{
					pButton->SetDefaultBorder(pBorder);
					pButton->SetDepressedBorder(pBorder);
					pButton->SetKeyFocusBorder(pBorder);

					Color fgColor, bgColor;
					fgColor = GetSchemeColor("Label.TextDullColor", GetFgColor(), pScheme);
					bgColor = Color(0, 0, 0, 0);
					pButton->SetDefaultColor(fgColor, bgColor);
				}
			}
		}
	}

	SetBgColor(Color(0, 0, 0, 0));
}

void BuyPresetEditPanel::OnSizeChanged(int wide, int tall)
{
	Panel::OnSizeChanged(wide, tall);
}

void BuyPresetEditPanel::Reset(void)
{
	if (m_pTitleLabel)
		m_pTitleLabel->SetText("#Cstrike_BuyPresetNewChoice");

	if (m_pTitleEntry)
		m_pTitleEntry->SetText("#Cstrike_BuyPresetNewChoice");

	if (m_pCostLabel)
		m_pCostLabel->SetText("");

	static BuyPresetWeapon m_blankWeapon;

	m_pPrimaryWeapon->SetWeapon(&m_blankWeapon, true);
	m_pSecondaryWeapon->SetWeapon(&m_blankWeapon, false);
	m_pKnifeWeapon->SetWeapon(&m_blankWeapon, false);

	m_pHEGrenade->Reset();
	m_pSmokeGrenade->Reset();
	m_pFlashbangs->Reset();

	m_pDefuser->Reset();
	m_pNightvision->Reset();
	m_pArmor->Reset();

	m_pGrenades->Reset();
	m_pEquipments->Reset();
}

void BuyPresetEditPanel::PaintBackground(void)
{
	BaseClass::PaintBackground();
}