#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "event_api.h"

DECLARE_MESSAGE(m_StatusIcons, StatusIcon);

int CHudStatusIcons::Init(void)
{
	HOOK_MESSAGE(StatusIcon);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudStatusIcons::VidInit(void)
{
	int HUD_cross = gHUD.GetSpriteIndex("cross");

	m_iCrossWidth = gHUD.GetSpriteRect(HUD_cross).right - gHUD.GetSpriteRect(HUD_cross).left;
	return 1;
}

void CHudStatusIcons::Reset(void)
{
	memset(m_IconList, 0, sizeof m_IconList);

	m_iFlags &= ~HUD_ACTIVE;
	m_bFlashOn = true;
	m_tmNextFlash = 0;
}

int CHudStatusIcons::Draw(float flTime)
{
	if (gEngfuncs.IsSpectateOnly())
		return 1;

	int x = m_iCrossWidth / 2;
	int y = ScreenHeight / 2;

	if (flTime > m_tmNextFlash)
	{
		m_tmNextFlash = flTime + 0.1;
		m_bFlashOn = !m_bFlashOn;
	}

	for (int i = 0; i < MAX_ICONSPRITES; i++)
	{
		if (m_IconList[i].spr)
		{
			y += (m_IconList[i].rc.top - m_IconList[i].rc.bottom) - 5;

			if (!m_bFlashOn && m_IconList[i].bFlash)
				SPR_Set(m_IconList[i].spr, 250, 0, 0);
			else
				SPR_Set(m_IconList[i].spr, m_IconList[i].r, m_IconList[i].g, m_IconList[i].b);

			SPR_DrawAdditive(0, x, y, &m_IconList[i].rc);
		}
	}

	return 1;
}

int CHudStatusIcons::MsgFunc_StatusIcon(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int ShouldEnable = READ_BYTE();
	char *pszIconName = READ_STRING();

	if (ShouldEnable)
	{
		int r = READ_BYTE();
		int g = READ_BYTE();
		int b = READ_BYTE();
		bool bFlash = (ShouldEnable == STATUSICON_FLASH);

		EnableIcon(pszIconName, r, g, b, bFlash);
		m_iFlags |= HUD_ACTIVE;
	}
	else
		DisableIcon(pszIconName);

	return 1;
}

void CHudStatusIcons::EnableIcon(char *pszIconName, unsigned char red, unsigned char green, unsigned char blue, bool bFlash)
{
	int i;

	for (i = 0; i < MAX_ICONSPRITES; i++)
	{
		if (!stricmp(m_IconList[i].szSpriteName, pszIconName))
			break;
	}

	if (i == MAX_ICONSPRITES)
	{
		for (i = 0; i < MAX_ICONSPRITES; i++)
		{
			if (!m_IconList[i].spr)
				break;
		}
	}

	if (i == MAX_ICONSPRITES)
		i = 0;

	int spr_index = gHUD.GetSpriteIndex(pszIconName);
	m_IconList[i].spr = gHUD.GetSprite(spr_index);
	m_IconList[i].rc = gHUD.GetSpriteRect(spr_index);
	m_IconList[i].r = red;
	m_IconList[i].g = green;
	m_IconList[i].b = blue;
	m_IconList[i].bFlash = bFlash;
	strcpy(m_IconList[i].szSpriteName, pszIconName);

	if (strstr(m_IconList[i].szSpriteName, "grenade"))
	{
		cl_entity_t *pthisplayer = gEngfuncs.GetLocalPlayer();
		gEngfuncs.pEventAPI->EV_PlaySound(pthisplayer->index, pthisplayer->origin, CHAN_STATIC, "weapons/timer.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}

void CHudStatusIcons::DisableIcon(char *pszIconName)
{
	for (int i = 0; i < MAX_ICONSPRITES; i++)
	{
		if (!stricmp(m_IconList[i].szSpriteName, pszIconName))
		{
			memset(&m_IconList[i], 0, sizeof icon_sprite_t);
			return;
		}
	}
}

bool CHudStatusIcons::InBuyZone(void)
{
	for (int i = 0; i < MAX_ICONSPRITES; i++)
	{
		if (!stricmp(m_IconList[i].szSpriteName, "buyzone"))
			return true;
	}

	return false;
}