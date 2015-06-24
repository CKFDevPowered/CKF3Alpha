class CViewPortPanel
{
public:
	virtual	~CViewPortPanel(void) {}

public:
	virtual void Init(void) = 0;
	virtual void VidInit(void) = 0;
	virtual void Reset(void) = 0;
	virtual void Update(void) = 0;

public:
	virtual const char *GetName(void) = 0;
	virtual void SetData(KeyValues *data) = 0;
	virtual bool NeedsUpdate(void) = 0;
	virtual bool HasInputElements(void) = 0;
	virtual void ShowPanel(bool state) = 0;
	virtual bool IsDynamic(void) { return false; }

public:
	virtual vgui::VPANEL GetVPanel(void) = 0;
	virtual bool IsVisible(void) = 0;
	virtual void SetParent(vgui::VPANEL parent) = 0;
	virtual void SetParent(vgui::Panel *newParent) = 0;
	virtual bool IsMouseInputEnabled(void) = 0;
	virtual void SetMouseInputEnabled(bool state) = 0;
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) = 0;
};

#define DECLARE_VIEWPORT_PANEL_SIMPLE() \
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); } \
	virtual bool IsVisible(void) { return BaseClass::IsVisible(); } \
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); } \
	virtual void SetParent(vgui::Panel *newParent) { BaseClass::SetParent(newParent); } \
	virtual bool IsMouseInputEnabled(void) { return BaseClass::IsMouseInputEnabled(); } \
	virtual void SetMouseInputEnabled(bool state) { BaseClass::SetMouseInputEnabled(state); } \
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) { BaseClass::InvalidateLayout(layoutNow, reloadScheme); }