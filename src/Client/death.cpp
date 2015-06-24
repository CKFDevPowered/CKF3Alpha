#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include <triangleapi.h>

#include "CounterStrikeViewport.h"

extern int g_iUser1;

DECLARE_MESSAGE(m_DeathNotice, DeathMsg);

struct DeathNoticeItem
{
	char szKiller[MAX_PLAYER_NAME_LENGTH * 2];
	char szVictim[MAX_PLAYER_NAME_LENGTH * 2];
	int iId;
	int iHeadShotId;
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
	vgui::IImage **DrawBg;
};

#define MAX_DEATHNOTICES 8
#define MAX_DRAWDEATHNOTICES 4

static int DEATHNOTICE_DISPLAY_TIME = 6;
static int KILLEFFECT_DISPLAY_TIME = 4;
static int KILLICON_DISPLAY_TIME = 1;

#define DEATHNOTICE_TOP 32

DeathNoticeItem rgDeathNoticeList[MAX_DEATHNOTICES + 1];

float g_ColorBlue[3] = { 0.6, 0.8, 1.0 };
float g_ColorRed[3] = { 1.0, 0.25, 0.25 };
float g_ColorGreen[3] = { 0.6, 1.0, 0.6 };
float g_ColorYellow[3] = { 1.0, 0.7, 0.0 };
float g_ColorGrey[3] = { 0.8, 0.8, 0.8 };

float *GetClientColor(int clientIndex)
{
	return g_pfnGetClientColor(clientIndex);
}

int CHudDeathNotice::Init(void)
{
	gHUD.AddHudElem(this);

	//HOOK_MESSAGE(DeathMsg);

	return 1;
}

void CHudDeathNotice::Reset(void)
{
	m_killNums = 0;
	m_multiKills = 0;
	m_showIcon = false;
	m_showKill = false;
	m_iconIndex = 0;
	m_killEffectTime = 0;
	m_killIconTime = 0;
	m_lastKillTime = 0;
	m_lastMultiKills = 0;
}

void CHudDeathNotice::InitHUDData(void)
{
	memset(rgDeathNoticeList, 0, sizeof(rgDeathNoticeList));
}

int CHudDeathNotice::VidInit(void)
{
	m_HUD_d_skull = gHUD.GetSpriteIndex("d_skull");
	m_headSprite = gHUD.GetSpriteIndex("d_headshot");
	m_headWidth = gHUD.GetSpriteRect(m_headSprite).right - gHUD.GetSpriteRect(m_headSprite).left;

	m_killBg[0] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_left", true);
	m_killBg[1] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_center", true);
	m_killBg[2] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/KillBg_right", true);
	m_deathBg[0] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_left", true);
	m_deathBg[1] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_center", true);
	m_deathBg[2] = vgui::scheme()->GetImage("resource/Hud/DeathNotice/DeathBg_right", true);

	int w, t;
	m_killBg[0]->GetContentSize(w, t);
	m_iDrawBgWidth = w;

	gEngfuncs.pfnDrawConsoleStringLen("N", &w, &t);

	m_iFontHeight = t;
	m_iDrawBgHeight = m_iFontHeight * 1.15;

	if (m_iDrawBgHeight < 24)
		m_iDrawBgHeight = 24;

	m_killBg[0]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_killBg[2]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_deathBg[0]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);
	m_deathBg[2]->SetSize(m_iDrawBgWidth, m_iDrawBgHeight);

	m_KM_Number0 = gHUD.GetSpriteIndex("KM_Number0");
	m_KM_Number1 = gHUD.GetSpriteIndex("KM_Number1");
	m_KM_Number2 = gHUD.GetSpriteIndex("KM_Number2");
	m_KM_Number3 = gHUD.GetSpriteIndex("KM_Number3");
	m_KM_KillText = gHUD.GetSpriteIndex("KM_KillText");
	m_KM_Icon_Head = gHUD.GetSpriteIndex("KM_Icon_Head");
	m_KM_Icon_Knife = gHUD.GetSpriteIndex("KM_Icon_knife");
	m_KM_Icon_Frag = gHUD.GetSpriteIndex("KM_Icon_Frag");

	return 1;
}

int CHudDeathNotice::GetDeathNoticeY(void)
{
	if (g_iUser1 || gEngfuncs.IsSpectateOnly())
		return gViewPortInterface->GetSpectatorTopBarHeight() + (YRES(DEATHNOTICE_TOP) / 4);

	return YRES(DEATHNOTICE_TOP);
}

int CHudDeathNotice::Draw(float flTime)
{
	int x, y, r, g, b;
	int noticeY = GetDeathNoticeY();

	int c = 0;

	for (; c < MAX_DEATHNOTICES; c++)
	{
		if (rgDeathNoticeList[c].iId == 0)
			break;
	}

	c = max(c, MAX_DRAWDEATHNOTICES);

	for (int i = c - 4; i < c; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;

		if (rgDeathNoticeList[i].flDisplayTime < flTime)
		{
			memmove(&rgDeathNoticeList[i], &rgDeathNoticeList[i + 1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i));
			i--;
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min(rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME);

		if (gViewPortInterface && gViewPortInterface->AllowedToPrintText() != FALSE)
		{
			int xMin, id;
			int iconWidth;
			int killerWidth, victimWidth;
			int xOffset;
			int yOffset;

			y = noticeY + ((m_iDrawBgHeight * 1.15) * (i - (c - 4)));
			id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			iconWidth = gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left;
			killerWidth = ConsoleStringLen(rgDeathNoticeList[i].szKiller);
			victimWidth = ConsoleStringLen(rgDeathNoticeList[i].szVictim);
			xOffset = 3;
			yOffset = (m_iDrawBgHeight - m_iFontHeight);
			y = y - yOffset;

			if (cl_killeffect->value && rgDeathNoticeList[i].DrawBg)
			{
				x = ScreenWidth - victimWidth - ((iconWidth + YRES(5)) + ((rgDeathNoticeList[i].iHeadShotId != -1) ? (m_headWidth + YRES(5)) : 0)) - (YRES(5) * 3);
				xMin = x;

				if (!rgDeathNoticeList[i].iSuicide)
				{
					x -= YRES(5) + killerWidth;
					xMin = x;
					x += YRES(5) + killerWidth;
				}

				x += iconWidth + YRES(5);

				if (rgDeathNoticeList[i].iHeadShotId != -1)
					x += m_headWidth + YRES(5);

				if (rgDeathNoticeList[i].iNonPlayerKill == FALSE)
					x += victimWidth;

				gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);

				rgDeathNoticeList[i].DrawBg[0]->SetPos(xMin - 3 - xOffset, y);
				rgDeathNoticeList[i].DrawBg[0]->Paint();

				rgDeathNoticeList[i].DrawBg[1]->SetPos(xMin - 3 - xOffset + m_iDrawBgWidth, y);
				rgDeathNoticeList[i].DrawBg[1]->SetSize(x + xOffset - (xMin - 3 - xOffset + m_iDrawBgWidth), m_iDrawBgHeight);
				rgDeathNoticeList[i].DrawBg[1]->Paint();

				rgDeathNoticeList[i].DrawBg[2]->SetPos(x + xOffset, y);
				rgDeathNoticeList[i].DrawBg[2]->Paint();
			}

			x = ScreenWidth - victimWidth - ((iconWidth + YRES(5)) + ((rgDeathNoticeList[i].iHeadShotId != -1) ? (m_headWidth + YRES(5)) : 0)) - (YRES(5) * 3);

			if (!rgDeathNoticeList[i].iSuicide)
			{
				x -= (YRES(5) + killerWidth);

				if (rgDeathNoticeList[i].KillerColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2]);

				x = YRES(5) + DrawConsoleString(x, y + ((m_iDrawBgHeight - m_iFontHeight) / 2), rgDeathNoticeList[i].szKiller);
			}

			if (rgDeathNoticeList[i].iTeamKill)
			{
				r = 10;
				g = 240;
				b = 10;
			}
			else
			{
				r = 255;
				g = 80;
				b = 0;
			}

			SPR_Set(gHUD.GetSprite(id), r, g, b);
			SPR_DrawAdditive(0, x, y + (abs(m_iDrawBgHeight - (gHUD.GetSpriteRect(id).bottom - gHUD.GetSpriteRect(id).top)) / 2), &gHUD.GetSpriteRect(id));

			x += iconWidth + YRES(5);

			if (rgDeathNoticeList[i].iHeadShotId != -1)
			{
				SPR_Set(gHUD.GetSprite(rgDeathNoticeList[i].iHeadShotId), r, g, b);
				SPR_DrawAdditive(0, x, y + (abs(m_iDrawBgHeight - (gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId).bottom - gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId).top)) / 2), &gHUD.GetSpriteRect(rgDeathNoticeList[i].iHeadShotId));

				x += m_headWidth + YRES(5);
			}

			if (rgDeathNoticeList[i].iNonPlayerKill == FALSE)
			{
				if (rgDeathNoticeList[i].VictimColor)
					gEngfuncs.pfnDrawSetTextColor(rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2]);

				x = DrawConsoleString(x, y + ((m_iDrawBgHeight - m_iFontHeight) / 2), rgDeathNoticeList[i].szVictim);
			}
		}
	}

	if (m_showKill)
	{
		m_killEffectTime = min(m_killEffectTime, gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME);

		if (gHUD.m_flTime < m_killEffectTime)
		{
			int r = 255, g = 255, b = 255;
			float alpha = (m_killEffectTime - gHUD.m_flTime) / KILLEFFECT_DISPLAY_TIME;
			int numIndex = -1;

			if (alpha > 0)
			{
				ScaleColors(r, g, b, alpha * 255);

				switch (m_multiKills)
				{
					case 1:
					{
						numIndex = m_KM_Number0;
						break;
					}

					case 2:
					{
						numIndex = m_KM_Number1;
						break;
					}

					case 3:
					{
						numIndex = m_KM_Number2;
						break;
					}

					case 4:
					{
						numIndex = m_KM_Number3;
						break;
					}
				}

				if (numIndex != -1)
				{
					int numWidth, numHeight;
					int textWidth, textHeight;
					int iconWidth, iconHeight;

					numWidth = gHUD.GetSpriteRect(numIndex).right - gHUD.GetSpriteRect(numIndex).left;
					numHeight = gHUD.GetSpriteRect(numIndex).bottom - gHUD.GetSpriteRect(numIndex).top;
					textWidth = gHUD.GetSpriteRect(m_KM_KillText).right - gHUD.GetSpriteRect(m_KM_KillText).left;
					textHeight = gHUD.GetSpriteRect(m_KM_KillText).bottom - gHUD.GetSpriteRect(m_KM_KillText).top;
					iconWidth = gHUD.GetSpriteRect(m_KM_Icon_Head).right - gHUD.GetSpriteRect(m_KM_Icon_Head).left;
					iconHeight = gHUD.GetSpriteRect(m_KM_Icon_Head).bottom - gHUD.GetSpriteRect(m_KM_Icon_Head).top;

					if (m_multiKills == 1)
						numWidth += 10;

					y = (25.0 * 0.01 * ScreenHeight) - (iconHeight + textHeight) * 0.5;
					x = (50.0 * 0.01 * ScreenWidth) - (numHeight + textWidth) * 0.5;

					SPR_Set(gHUD.GetSprite(numIndex), r, g, b);
					SPR_DrawAdditive(0, x, y - (gHUD.GetSpriteRect(numIndex).bottom + gHUD.GetSpriteRect(m_KM_KillText).top - gHUD.GetSpriteRect(m_KM_KillText).bottom - gHUD.GetSpriteRect(numIndex).top) * 0.6, &gHUD.GetSpriteRect(numIndex));

					SPR_Set(gHUD.GetSprite(m_KM_KillText), r, g, b);
					SPR_DrawAdditive(0, x + numWidth, y, &gHUD.GetSpriteRect(m_KM_KillText));

					x = (50.0 * 0.01 * ScreenWidth) - (iconWidth) * 0.5;
					y = y + textHeight;

					m_killIconTime = min(m_killIconTime, gHUD.m_flTime + KILLICON_DISPLAY_TIME);

					if (m_showIcon)
					{
						alpha = (m_killIconTime - gHUD.m_flTime) / KILLICON_DISPLAY_TIME;

						if (alpha > 0)
						{
							ScaleColors(r, g, b, alpha * 255);

							switch (m_iconIndex)
							{
								case 1:
								{
									SPR_Set(gHUD.GetSprite(m_KM_Icon_Head), r, g, b);
									SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Head));
									break;
								}

								case 2:
								{
									SPR_Set(gHUD.GetSprite(m_KM_Icon_Knife), r, g, b);
									SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Knife));
									break;
								}

								case 3:
								{
									SPR_Set(gHUD.GetSprite(m_KM_Icon_Frag), r, g, b);
									SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Frag));
									break;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			m_showKill = false;
			m_showIcon = false;
		}
	}

	return 1;
}

int CHudDeathNotice::MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);

	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int headshot = READ_BYTE();
	int multiKills = 0;

	char killedwith[32];
	strcpy(killedwith, "d_");
	strncat(killedwith, READ_STRING(), 32);

	if (gViewPortInterface)
		gViewPortInterface->DeathMsg(killer, victim);

	gHUD.m_Spectator.DeathMessage(victim);

	for (int j = 0; j < MAX_DEATHNOTICES; j++)
	{
		if (rgDeathNoticeList[j].iId == 0)
			break;

		if (rgDeathNoticeList[j].DrawBg == m_killBg)
			multiKills++;
	}

	if (m_lastMultiKills)
	{
		if (m_lastMultiKills >= multiKills && gHUD.m_flTime < m_lastKillTime + 3.0)
			multiKills = m_lastMultiKills + 1;
		else
			m_lastMultiKills = 0;
	}

	if (cl_killmessage->value)
	{
		if (killer == gHUD.m_iPlayerNum && victim != gHUD.m_iPlayerNum)
		{
			m_killNums++;
			m_showIcon = false;

			if (headshot)
			{
				gEngfuncs.pfnClientCmd("speak \"HeadShot\"\n");

				m_showIcon = true;
				m_iconIndex = 1;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}
		}

		if (!strcmp(killedwith, "d_knife"))
		{
			if (killer == gHUD.m_iPlayerNum)
			{
				gEngfuncs.pfnClientCmd("speak \"Humililation\"\n");

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}

			if (victim == gHUD.m_iPlayerNum)
			{
				gEngfuncs.pfnClientCmd("speak \"OhNo\"\n");

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}
		}

		if (killer == gHUD.m_iPlayerNum && victim != gHUD.m_iPlayerNum)
		{
			switch (multiKills)
			{
				case 0:
				{
					m_showKill = true;
					m_multiKills = 1;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				case 1:
				{
					gEngfuncs.pfnClientCmd("speak \"DoubleKill\"\n");

					m_showKill = true;
					m_multiKills = 2;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				case 2:
				{
					gEngfuncs.pfnClientCmd("speak \"TripleKill\"\n");

					m_showKill = true;
					m_multiKills = 3;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				default:
				{
					gEngfuncs.pfnClientCmd("speak \"MultiKill\"\n");

					m_showKill = true;
					m_multiKills = 4;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}
			}

			switch (m_killNums)
			{
				case 5:
				{
					gEngfuncs.pfnClientCmd("speak \"Excellent\"\n");
					break;
				}

				case 10:
				{
					gEngfuncs.pfnClientCmd("speak \"Incredible\"\n");
					break;
				}

				case 15:
				{
					gEngfuncs.pfnClientCmd("speak \"Crazy\"\n");
					break;
				}

				case 20:
				{
					gEngfuncs.pfnClientCmd("speak \"CantBelive\"\n");
					break;
				}

				case 25:
				{
					gEngfuncs.pfnClientCmd("speak \"OutofWorld\"\n");
					break;
				}
			}
		}
	}

	int i = 0;

	for (; i < MAX_DEATHNOTICES; i++)
	{
		if (rgDeathNoticeList[i].iId == 0)
			break;
	}

	if (i == MAX_DEATHNOTICES)
	{
		memmove(rgDeathNoticeList, rgDeathNoticeList + 1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES);
		i = MAX_DEATHNOTICES - 1;
	}

	if (gViewPortInterface)
		gViewPortInterface->GetAllPlayersInfo();

	char *killer_name = g_PlayerInfoList[killer].name;

	if (!killer_name)
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor(killer);
		strncpy(rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	char *victim_name = NULL;

	if (((char)victim) != -1)
		victim_name = g_PlayerInfoList[victim].name;

	if (!victim_name)
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor(victim);
		strncpy(rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH);
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH - 1] = 0;
	}

	if (((char)victim) == -1)
	{
		rgDeathNoticeList[i].iNonPlayerKill = TRUE;
		strcpy(rgDeathNoticeList[i].szVictim, killedwith + 2);
	}
	else
	{
		if (killer == victim || killer == 0)
			rgDeathNoticeList[i].iSuicide = TRUE;

		if (!strcmp(killedwith, "d_teammate"))
			rgDeathNoticeList[i].iTeamKill = TRUE;
	}

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT("hud_deathnotice_time");

	rgDeathNoticeList[i].iId = gHUD.GetSpriteIndex(killedwith);
	rgDeathNoticeList[i].iHeadShotId = headshot ? m_headSprite : -1;
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	if (victim == gHUD.m_iPlayerNum)
		rgDeathNoticeList[i].DrawBg = m_deathBg;
	else if (killer == gHUD.m_iPlayerNum)
		rgDeathNoticeList[i].DrawBg = m_killBg;
	else
		rgDeathNoticeList[i].DrawBg = NULL;

	if (rgDeathNoticeList[i].iNonPlayerKill)
	{
		ConsolePrint(rgDeathNoticeList[i].szKiller);
		ConsolePrint(" killed a ");
		ConsolePrint(rgDeathNoticeList[i].szVictim);
		ConsolePrint("\n");
	}
	else
	{
		if (rgDeathNoticeList[i].iSuicide)
		{
			ConsolePrint(rgDeathNoticeList[i].szVictim);

			if (!strcmp(killedwith, "d_world"))
				ConsolePrint(" died");
			else
				ConsolePrint(" killed self");
		}
		else if (rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed his teammate ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}
		else
		{
			ConsolePrint(rgDeathNoticeList[i].szKiller);
			ConsolePrint(" killed ");
			ConsolePrint(rgDeathNoticeList[i].szVictim);
		}

		if (killedwith && *killedwith && (*killedwith > 13) && strcmp(killedwith, "d_world") && !rgDeathNoticeList[i].iTeamKill)
		{
			ConsolePrint(" with ");
			ConsolePrint(killedwith + 2);
		}

		ConsolePrint("\n");
	}

	m_lastKillTime = gHUD.m_flTime;
	m_lastMultiKills = multiKills;
	return 1;
}