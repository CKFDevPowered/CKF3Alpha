#ifndef VGUI_HUD_LAYER_H
#define VGUI_HUD_LAYER_H

#include "hud_element.h"

namespace vgui
{
	class CHudNewRadar;
	class CHudNewMenu;
	class CTFHudPlayerStatus;
};

class CHudLayer : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudLayer, vgui::Panel);

public:
	CHudLayer(vgui::Panel *parent);
	~CHudLayer(void);

public:
	void Start(void);
	void SetParent(vgui::VPANEL parent);
	void SetVisible(bool state);

public:
	void Init(void);
	void VidInit(void);
	void Reset(void);
	void Think(void);

public:
	CHudElement *AddNewPanel(CHudElement *pPanel, char const *pchDebugName = NULL);
	void RemoveAllPanels(void);

public:
	bool SelectMenuItem(int menu_item);
	void UpdateMapSprite(void);
	void RenderMapSprite(void);
	void CalcRefdef(struct ref_params_s *pparams);
	int FireMessage(const char *pszName, int iSize, void *pbuf);

private:
	CUtlVector<CHudElement *> m_Panels;

private:
	vgui::CHudNewMenu *m_pMenu;
	vgui::CHudNewRadar *m_pRadar;
	vgui::CTFHudPlayerStatus *m_pTFPlayerStatus;
};

#endif