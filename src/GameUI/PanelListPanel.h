#if !defined (PANELLISTPANEL_H)
#define PANELLISTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

class KeyValues;

class CPanelListPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CPanelListPanel, vgui::Panel);

public:
	typedef struct dataitem_s
	{
		vgui::Panel *panel;
	}
	DATAITEM;

public:
	CPanelListPanel(vgui::Panel *parent, char const *panelName, bool inverseButtons = false);
	~CPanelListPanel(void);

public:
	virtual int computeVPixelsNeeded(void);
	virtual int AddItem(vgui::Panel *panel);
	virtual int GetItemCount(void);
	virtual vgui::Panel *GetItem(int itemIndex);
	virtual void RemoveItem(int itemIndex);
	virtual void DeleteAllItems(void);
	virtual vgui::Panel *GetCellRenderer(int row);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	MESSAGE_FUNC_INT(OnSliderMoved, "ScrollBarSliderMoved", position);

public:
	void SetSliderYOffset(int pixels);

	vgui::Panel *GetEmbedded(void)
	{
		return _embedded;
	}

protected:
	DATAITEM *GetDataItem(int itemIndex);

protected:
	virtual void PerformLayout(void);
	virtual void PaintBackground(void);
	virtual void OnMouseWheeled(int delta);

private:
	vgui::Dar<DATAITEM *> _dataItems;
	vgui::ScrollBar *_vbar;
	vgui::Panel *_embedded;

	int _tableStartX;
	int _tableStartY;
	int _sliderYOffset;
};

#endif