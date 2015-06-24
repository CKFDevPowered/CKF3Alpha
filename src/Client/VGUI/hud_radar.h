#ifndef VGUI_HUD_RADAR_H
#define VGUI_HUD_RADAR_H

#ifdef _WIN32
#pragma once
#endif

#include "hud_element.h"

#ifndef MAX_HOSTAGES
#define MAX_HOSTAGES 24
#endif

namespace vgui
{
class CHudRadar : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudRadar, Panel);

public:
	CHudRadar(void);

public:
	void Init(void);
	void VidInit(void);
	void Reset(void);
	void Think(void);
	int FireMessage(const char *pszName, int iSize, void *pbuf);
	void UpdateMapSprite(void);
	void RenderMapSprite(void);
	void CalcRefdef(struct ref_params_s *pparams);
	void Paint(void);
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void BuildHostageList(void);
	bool IsValidEntity(cl_entity_s *pEntity);
	bool CalcPoint(float *origin, int &screenX, int &screenY, int &scale);
	void DrawSprite(int x, int y, HSPRITE hspr, float yaw, int scale, int r, int g, int b, int a);

public:
	DECLARE_HUD_ELEMENT_SIMPLE();

private:
	vgui::HFont m_hHudFont;
	float m_flNextBuild;
	struct cl_entity_s *m_pHostages[MAX_HOSTAGES + 1];
	bool m_bCanRenderMapSprite;
	int m_iLastWide;

private:
	HSPRITE m_hsprPlayer;
	HSPRITE m_hsprPlayerBlue;
	HSPRITE m_hsprPlayerRed;
	HSPRITE m_hsprPlayerDead;
	HSPRITE m_hsprPlayerVIP;
	HSPRITE m_hsprPlayerC4;
	HSPRITE m_hsprBackPack;
	HSPRITE m_hsprCamera;
	HSPRITE m_hsprRadio;
	HSPRITE m_hsprC4;
	HSPRITE m_hsprHostage;

private:
	int m_HUD_radar;
	int m_HUD_radaropaque;

private:
	wrect_t *m_hrad;
	wrect_t *m_hradopaque;
	HSPRITE m_hRadar;
	HSPRITE m_hRadaropaque;
};
}

#endif