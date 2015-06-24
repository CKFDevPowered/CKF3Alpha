#include "hud.h"
#include "player.h"
#include "com_model.h"
#include "cl_util.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <IEngineSurface.h>
#include <VGUI/VGUI.h>
#include <VGUI/ISurface.h>
#include <VGUI/IScheme.h>
#include "plugins.h"
#include "calcscreen.h"
#include "client.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_headName, HeadName)

int CHudHeadName::Init(void)
{
	HOOK_MESSAGE(HeadName);

	gHUD.AddHudElem(this);
	return 1;
}

int CHudHeadName::VidInit(void)
{
	m_iFlags |= HUD_ACTIVE;
	m_iDrawType = HEADNAME_TEAMMATE;

	if (!m_hHeadFont)
	{
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme("ClientScheme"));

		if (pScheme)
		{
			m_hHeadFont = pScheme->GetFont("CreditsText");

			if (!m_hHeadFont)
			{
				pScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetDefaultScheme());

				if (pScheme)
					m_hHeadFont = pScheme->GetFont("CreditsFont");
			}
		}
	}

	return 1;
}

void CHudHeadName::Reset(void)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_sUnicodes[i][0] = 0;
}

void CHudHeadName::BuildUnicodeList(void)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0])
		{
			m_sUnicodes[i][0] = 0;
			continue;
		}

		vgui::localize()->ConvertANSIToUnicode(g_PlayerInfoList[i].name, m_sUnicodes[i], sizeof(m_sUnicodes[i]));
	}
}

bool CHudHeadName::IsValidEntity(cl_entity_s *pEntity)
{
	bool bNotInPVS = (abs(gEngfuncs.GetLocalPlayer()->curstate.messagenum - pEntity->curstate.messagenum) > 15);

	if (pEntity && pEntity->model && pEntity->model->name && !bNotInPVS)
		return true;

	return false;
}

int CHudHeadName::Draw(float flTime)
{
	if (m_iDrawType == HEADNAME_HIDE)
		return 1;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL) || g_iUser1)
		return 1;

	if (gHUD.m_flTime > m_flNextBuild)
	{
		BuildUnicodeList();
		m_flNextBuild = gHUD.m_flTime + 1.0;
	}

	bool isZombie = (g_PlayerScoreAttrib[gHUD.m_iPlayerNum] & SCOREATTRIB_ZOMBIE);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_sUnicodes[i][0])
		{
			if (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0])
				continue;

			vgui::localize()->ConvertANSIToUnicode(g_PlayerInfoList[i].name, m_sUnicodes[i], sizeof(m_sUnicodes[i]));
		}

		if (g_PlayerExtraInfo[i].dead)
			continue;

		if (m_iDrawType == HEADNAME_TEAMMATE)
		{
			if (g_PlayerExtraInfo[i].teamnumber != g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamnumber)
				continue;
		}
		else if (m_iDrawType == HEADNAME_ZOMBIE)
		{
			if (isZombie != ((bool)(g_PlayerScoreAttrib[i] & SCOREATTRIB_ZOMBIE)))
				continue;
		}

		if (i != gHUD.m_iPlayerNum)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);

			if (!IsValidEntity(ent))
				continue;

			model_t *model = ent->model;
			vec3_t origin = ent->origin;

			if (model)
				origin.z += max((model->maxs.z - model->mins.z), 35.0);
			else
				origin.z += 35.0;

			if (cl_headname->value)
			{
				float screenPos[2];

				if (!CalcScreen(origin, screenPos))
					continue;

				int textWide, textTall;
				vgui::surface()->GetTextSize(m_hHeadFont, m_sUnicodes[i], textWide, textTall);
				vgui::surface()->DrawSetTextPos(screenPos[0] - ((textWide) / 2), screenPos[1] - ((textTall) / 2));
				vgui::surface()->DrawSetTextColor(255, 255, 255, 128);

				for (size_t j = 0; j < wcslen(m_sUnicodes[i]); j++)
					vgui::surface()->DrawUnicodeCharAdd(m_sUnicodes[i][j]);
			}
		}
	}

	return 1;
}

int CHudHeadName::MsgFunc_HeadName(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iDrawType = READ_BYTE();
	return 1;
}