#include "EngineInterface.h"
#include "VControlsListPanel.h"
#include "GameUI_Interface.h"

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/Cursor.h>
#include <KeyValues.h>

using namespace vgui;

class CInlineEditPanel : public vgui::Panel
{
public:
	CInlineEditPanel(void) : vgui::Panel(NULL, "InlineEditPanel")
	{
	}

	virtual void Paint(void)
	{
		int x = 0, y = 0, wide, tall;
		GetSize(wide, tall);

		vgui::surface()->DrawSetColor(255, 165, 0, 255);
		vgui::surface()->DrawFilledRect(x, y, x + wide, y + tall);
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		if (GetParent())
			GetParent()->OnKeyCodeTyped(code);
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Panel::ApplySchemeSettings(pScheme);
		SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
	}

	void OnMousePressed(vgui::MouseCode code)
	{
		if (GetParent())
			GetParent()->OnMousePressed(code);
	}
};

VControlsListPanel::VControlsListPanel(vgui::Panel *parent, const char *listName) : vgui::SectionedListPanel(parent, listName)
{
	m_bCaptureMode = false;
	m_nClickRow = 0;
	m_pInlineEditPanel = new CInlineEditPanel();
	m_hFont = INVALID_FONT;
}

VControlsListPanel::~VControlsListPanel(void)
{
	m_pInlineEditPanel->MarkForDeletion();
}

void VControlsListPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = pScheme->GetFont("Default", IsProportional());
}

void VControlsListPanel::StartCaptureMode(HCursor hCursor)
{
	m_bCaptureMode = true;
	EnterEditMode(m_nClickRow, 1, m_pInlineEditPanel);
	input()->SetMouseFocus(m_pInlineEditPanel->GetVPanel());
	input()->SetMouseCapture(m_pInlineEditPanel->GetVPanel());

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		vgui::input()->GetCursorPos(m_iMouseX, m_iMouseY);
	}
}

void VControlsListPanel::EndCaptureMode(HCursor hCursor)
{
	m_bCaptureMode = false;
	input()->SetMouseCapture(NULL);
	LeaveEditMode();
	RequestFocus();
	input()->SetMouseFocus(GetVPanel());

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		surface()->SetCursor(hCursor);

		if (hCursor != dc_none)
			vgui::input()->SetCursorPos(m_iMouseX, m_iMouseY);
	}
}

void VControlsListPanel::SetItemOfInterest(int itemID)
{
	m_nClickRow = itemID;
}

int VControlsListPanel::GetItemOfInterest(void)
{
	return m_nClickRow;
}

bool VControlsListPanel::IsCapturing(void)
{
	return m_bCaptureMode;
}

void VControlsListPanel::OnMousePressed(vgui::MouseCode code)
{
	if (IsCapturing())
	{
		if (GetParent())
			GetParent()->OnMousePressed(code);
	}
	else
		BaseClass::OnMousePressed(code);
}

void VControlsListPanel::OnMouseDoublePressed(vgui::MouseCode code)
{
	if (IsItemIDValid(GetSelectedItem()))
		OnKeyCodePressed(KEY_ENTER);
	else
		BaseClass::OnMouseDoublePressed(code);
}