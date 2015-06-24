#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

pfnUserMsgHook g_pfnMSG_StatusText;
pfnUserMsgHook g_pfnMSG_StatusValue;

DECLARE_MESSAGE(m_StatusBar, StatusText);
DECLARE_MESSAGE(m_StatusBar, StatusValue);

float *GetClientColor(int clientIndex);
extern float g_ColorYellow[3];

int CHudStatusBar::Init(void)
{
	gHUD.AddHudElem(this);

	g_pfnMSG_StatusText = HOOK_MESSAGE(StatusText);
	g_pfnMSG_StatusValue = HOOK_MESSAGE(StatusValue);

	Reset();
	return 1;
}

int CHudStatusBar::VidInit(void)
{
	return 1;
}

void CHudStatusBar::Reset(void)
{
	int i = 0;

	m_iFlags &= ~HUD_ACTIVE;

	for (i = 0; i < MAX_STATUSBAR_LINES; i++)
		m_szStatusText[i][0] = 0;

	memset(m_iStatusValues, 0, sizeof m_iStatusValues);

	m_iStatusValues[0] = 1;

	for (i = 0; i < MAX_PLAYERS; i++)
		g_PlayerExtraInfo[i].showhealth = 0;

	for (i = 0; i < MAX_STATUSBAR_LINES; i++)
		m_pflNameColors[i] = g_ColorYellow;
}

void CHudStatusBar::ParseStatusString(int line_num)
{
	char szBuffer[MAX_STATUSTEXT_LENGTH];
	memset(szBuffer, 0, sizeof szBuffer);
	gHUD.m_TextMessage.LocaliseTextString(m_szStatusText[line_num], szBuffer, MAX_STATUSTEXT_LENGTH);

	memset(m_szStatusBar[line_num], 0, MAX_STATUSTEXT_LENGTH);

	char *src = szBuffer;
	char *dst = m_szStatusBar[line_num];

	client_textmessage_t *tempMessage;
	char *src_start = src, *dst_start = dst;

	while (*src != 0)
	{
		while (*src == '\n')
			src++;

		if (((src - src_start) >= MAX_STATUSTEXT_LENGTH) || ((dst - dst_start) >= MAX_STATUSTEXT_LENGTH))
			break;

		int index = atoi(src);

		if ((index >= 0 && index < MAX_STATUSBAR_VALUES) && (m_iStatusValues[index] != 0))
		{
			while (*src >= '0' && *src <= '9')
				src++;

			if (*src == '\n' || *src == 0)
				continue;

			while (*src != '\n' && *src != 0)
			{
				if (*src != '%')
				{
					*dst = *src;
					dst++, src++;
				}
				else
				{
					char valtype = *(++src);

					if (valtype == '%')
					{
						*dst = valtype;
						dst++, src++;
						continue;
					}

					index = atoi(++src); 

					while (*src >= '0' && *src <= '9')
						src++;

					if (index >= 0 && index < MAX_STATUSBAR_VALUES)
					{
						int indexval = m_iStatusValues[index];
						char szRepString[MAX_PLAYER_NAME_LENGTH];

						switch (valtype)
						{
							case 'p':
							{
								gEngfuncs.pfnGetPlayerInfo(indexval, &g_PlayerInfoList[indexval]);

								if (g_PlayerInfoList[indexval].name != NULL)
								{
									strncpy(szRepString, g_PlayerInfoList[indexval].name, MAX_PLAYER_NAME_LENGTH);
									gHUD.m_Radar.m_iPlayerLastPointedAt = indexval;
									m_pflNameColors[line_num] = GetClientColor(indexval);
								}
								else
								{
									strcpy(szRepString, "******");
								}

								break;
							}

							case 'i':
							{
								g_PlayerExtraInfo[gHUD.m_Radar.m_iPlayerLastPointedAt].showhealth = gHUD.m_flTime + 5;
								g_PlayerExtraInfo[gHUD.m_Radar.m_iPlayerLastPointedAt].health = indexval;
								sprintf(szRepString, "%d", indexval);
								break;
							}

							case 'h':
							{
								tempMessage = TextMessageGet("Health");

								if (tempMessage != NULL)
								{
									strncpy(szRepString, tempMessage->pMessage, MAX_PLAYER_NAME_LENGTH);
								}
								else
								{
									strcpy(szRepString, "Health");
								}

								break;
							}

							case 'c':
							{
								switch (indexval)
								{
									case 1:
									{
										tempMessage = TextMessageGet("Friend");

										if (tempMessage != NULL)
										{
											strncpy(szRepString, tempMessage->pMessage, MAX_PLAYER_NAME_LENGTH);
										}
										else
										{
											strcpy(szRepString, "Friend");
										}

										break;
									}

									case 2:
									{
										tempMessage = TextMessageGet("Enemy");

										if (tempMessage != NULL)
										{
											strncpy(szRepString, tempMessage->pMessage, MAX_PLAYER_NAME_LENGTH);
										}
										else
										{
											strcpy(szRepString, "Enemy");
										}

										break;
									}

									case 3:
									{
										tempMessage = TextMessageGet("Hostage");

										if (tempMessage != NULL)
										{
											strncpy(szRepString, tempMessage->pMessage, MAX_PLAYER_NAME_LENGTH);
										}
										else
										{
											strcpy(szRepString, "Hostage");
										}

										break;
									}
								}

								break;
							}

							default:
							{
								szRepString[0] = 0;
							}
						}

						for (char *cp = szRepString; *cp != 0 && ((dst - dst_start) < MAX_STATUSTEXT_LENGTH); cp++, dst++)
							*dst = *cp;
					}
				}
			}
		}
		else
		{
			while (*src != 0 && *src != '\n')
				src++;
		}
	}
}

int CHudStatusBar::Draw(float fTime)
{
	if (m_bReparseString)
	{
		for (int i = 0; i < MAX_STATUSBAR_LINES; i++)
		{
			m_pflNameColors[i] = g_ColorYellow;
			ParseStatusString(i);
		}

		m_bReparseString = FALSE;
	}

	int Y_START = ScreenHeight - 52;

	for (int i = 0; i < MAX_STATUSBAR_LINES; i++)
	{
		int TextHeight, TextWidth;
		GetConsoleStringSize(m_szStatusBar[i], &TextWidth, &TextHeight);

		int x = 10;
		int y = Y_START - gHUD.m_iFontHeight;

		if (CVAR_GET_FLOAT("hud_centerid"))
		{
			x = max(0, max(2, (ScreenWidth - TextWidth)) / 2);
			y = (ScreenHeight / 2) + (TextHeight * CVAR_GET_FLOAT("hud_centerid"));
		}

		if (m_pflNameColors[i])
			gEngfuncs.pfnDrawSetTextColor(m_pflNameColors[i][0], m_pflNameColors[i][1], m_pflNameColors[i][2]);

		DrawConsoleString(x, y, m_szStatusBar[i]);
	}

	return 1;
}

int CHudStatusBar::MsgFunc_StatusText(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int line = READ_BYTE();

	if (line < 0 || line > MAX_STATUSBAR_LINES)
		return 1;

	strncpy(m_szStatusText[line], READ_STRING(), MAX_STATUSTEXT_LENGTH);
	m_szStatusText[line][MAX_STATUSTEXT_LENGTH - 1] = 0;

	m_iFlags |= HUD_ACTIVE;
	m_bReparseString = TRUE;

	return g_pfnMSG_StatusText(pszName, iSize, pbuf);
}

int CHudStatusBar::MsgFunc_StatusValue(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int index = READ_BYTE();

	if (index < 1 || index > MAX_STATUSBAR_VALUES)
		return 1;

	m_iStatusValues[index] = READ_SHORT();
	m_bReparseString = TRUE;

	return g_pfnMSG_StatusValue(pszName, iSize, pbuf);
}