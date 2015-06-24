#include <vgui_controls/Frame.h>

class CBackGroundPanel : public vgui::Frame
{
	typedef vgui::Frame BaseClass;

public:
	CBackGroundPanel(vgui::Panel *parent) : BaseClass(parent, "ViewPortBackGround")
	{
		SetScheme("ClientScheme");

		SetTitleBarVisible(false);
		SetMoveable(false);
		SetSizeable(false);
		SetProportional(true);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetBgColor(pScheme->GetColor("ViewportBG", Color(0, 0, 0, 0)));
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();
	}

	virtual void OnMousePressed(vgui::MouseCode code)
	{
	}

	virtual vgui::VPANEL IsWithinTraverse(int x, int y, bool traversePopups)
	{
		return NULL;
	}
};