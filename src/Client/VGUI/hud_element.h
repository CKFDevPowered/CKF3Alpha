#ifndef VGUI_HUD_ELEMENT_H
#define VGUI_HUD_ELEMENT_H

class CHudElement
{
public:
	virtual	~CHudElement(void) {}

public:
	virtual void Init(void) = 0;
	virtual void VidInit(void) = 0;
	virtual void Reset(void) = 0;
	virtual void Think(void) = 0;
	virtual int FireMessage(const char *pszName, int iSize, void *pbuf) = 0;
	virtual bool ShouldDraw( void ) = 0;
public:
	virtual vgui::VPANEL GetVPanel(void) = 0;
	virtual bool IsVisible(void) = 0;
	virtual void SetParent(vgui::VPANEL parent) = 0;
	virtual void SetParent(vgui::Panel *newParent) = 0;
	virtual bool IsMouseInputEnabled(void) = 0;
	virtual void SetMouseInputEnabled(bool state) = 0;
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) = 0;
	virtual void SetHiddenBits( int iBits ) = 0;
	int m_iHiddenBits;
};

#define DECLARE_HUD_ELEMENT_SIMPLE() \
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); } \
	virtual bool IsVisible(void) { return BaseClass::IsVisible(); } \
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); } \
	virtual void SetParent(vgui::Panel *newParent) { BaseClass::SetParent(newParent); } \
	virtual bool IsMouseInputEnabled(void) { return BaseClass::IsMouseInputEnabled(); } \
	virtual void SetMouseInputEnabled(bool state) { BaseClass::SetMouseInputEnabled(state); } \
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) { BaseClass::InvalidateLayout(layoutNow, reloadScheme); }\
	virtual void SetHiddenBits( int iBits ){m_iHiddenBits = iBits;}\
	int m_iHiddenBits;

#endif