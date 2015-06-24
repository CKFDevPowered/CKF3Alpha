#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

vgui::HFont Newfont;
vgui::HFont font;

DECLARE_MESSAGE(m_Message, HudText)
DECLARE_MESSAGE(m_Message, HudTextPro)
DECLARE_MESSAGE(m_Message, HudTextArgs)
DECLARE_MESSAGE(m_Message, GameTitle)

int CHudMessage::Init(void)
{
	HOOK_MESSAGE(HudText);
	HOOK_MESSAGE(HudTextPro);
	HOOK_MESSAGE(HudTextArgs);
	HOOK_MESSAGE(GameTitle);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

int CHudMessage::VidInit(void)
{
	m_HUD_title_half = gHUD.GetSpriteIndex("title_half");
	m_HUD_title_life = gHUD.GetSpriteIndex("title_life");

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetDefaultScheme());

	if (pScheme)
	{
		if (!font)
			font = pScheme->GetFont("Legacy_CreditsFont", true);

		if (!Newfont)
			Newfont = pScheme->GetFont("CreditsFont", true);
	}

	return 1;
}

void CHudMessage::Reset(void)
{
	memset(m_pMessages, 0, sizeof(m_pMessages[0]) * maxHUDMessages);
	memset(m_startTime, 0, sizeof(m_startTime[0]) * maxHUDMessages);

	m_gameTitleTime = 0;
	m_pGameTitle = NULL;
}

float CHudMessage::FadeBlend(float fadein, float fadeout, float hold, float localTime)
{
	float fadeTime = fadein + hold;
	float fadeBlend;

	if (localTime < 0)
		return 0;

	if (localTime < fadein)
	{
		fadeBlend = 1 - ((fadein - localTime) / fadein);
	}
	else if (localTime > fadeTime)
	{
		if (fadeout > 0)
			fadeBlend = 1 - ((localTime - fadeTime) / fadeout);
		else
			fadeBlend = 0;
	}
	else
		fadeBlend = 1;

	return fadeBlend;
}

int CHudMessage::XPosition(float x, int width, int totalWidth)
{
	int xPos;

	if (x == -1)
	{
		xPos = (ScreenWidth - width) / 2;
	}
	else
	{
		if (x < 0)
			xPos = (1.0 + x) * ScreenWidth - totalWidth;
		else
			xPos = x * ScreenWidth;
	}

	if (xPos + width > ScreenWidth)
		xPos = ScreenWidth - width;
	else if (xPos < 0)
		xPos = 0;

	return xPos;
}

int CHudMessage::YPosition(float y, int height)
{
	int yPos;

	if (y == -1)
	{
		yPos = (ScreenHeight - height) * 0.5;
	}
	else
	{
		if (y < 0)
			yPos = (1.0 + y) * ScreenHeight - height;
		else
			yPos = y * ScreenHeight;
	}

	if (yPos + height > ScreenHeight)
		yPos = ScreenHeight - height;
	else if (yPos < 0)
		yPos = 0;

	return yPos;
}

void CHudMessage::MessageScanNextChar(unsigned int font)
{
	int srcRed, srcGreen, srcBlue, destRed, destGreen, destBlue;
	int blend;

	srcRed = m_parms.pMessage->r1;
	srcGreen = m_parms.pMessage->g1;
	srcBlue = m_parms.pMessage->b1;
	blend = 0;

	switch (m_parms.pMessage->effect)
	{
		case 0:
		case 1:
		{
			destRed = destGreen = destBlue = 0;
			blend = m_parms.fadeBlend;
			break;
		}

		case 2:
		{
			m_parms.charTime += m_parms.pMessage->fadein;

			if (m_parms.charTime > m_parms.time)
			{
				srcRed = srcGreen = srcBlue = 0;
				destRed = destGreen = destBlue = 0;
				blend = 0;
			}
			else
			{
				float deltaTime = m_parms.time - m_parms.charTime;

				destRed = destGreen = destBlue = 0;

				if (m_parms.time > m_parms.fadeTime)
				{
					blend = m_parms.fadeBlend;
				}
				else if (deltaTime > m_parms.pMessage->fxtime)
				{
					blend = 0;
				}
				else
				{
					destRed = m_parms.pMessage->r2;
					destGreen = m_parms.pMessage->g2;
					destBlue = m_parms.pMessage->b2;
					blend = 255 - (deltaTime * (1.0 / m_parms.pMessage->fxtime) * 255.0 + 0.5);
				}
			}

			break;
		}
	}

	if (blend > 255)
		blend = 255;
	else if (blend < 0)
		blend = 0;

	m_parms.r = ((srcRed * (255 - blend)) + (destRed * blend)) >> 8;
	m_parms.g = ((srcGreen * (255 - blend)) + (destGreen * blend)) >> 8;
	m_parms.b = ((srcBlue * (255 - blend)) + (destBlue * blend)) >> 8;

	if (m_parms.pMessage->effect == 1 && m_parms.charTime != 0)
	{
		if (m_parms.x >= 0 && m_parms.y >= 0)
			gEngfuncs.pfnVGUI2DrawCharacterAdd(m_parms.x, m_parms.y, m_parms.text, m_parms.pMessage->r2, m_parms.pMessage->g2, m_parms.pMessage->b2, font);
	}
}

void CHudMessage::MessageScanStart(void)
{
	switch (m_parms.pMessage->effect)
	{
		case 1:
		case 0:
		{
			m_parms.fadeTime = m_parms.pMessage->fadein + m_parms.pMessage->holdtime;

			if (m_parms.time < m_parms.pMessage->fadein)
			{
				m_parms.fadeBlend = ((m_parms.pMessage->fadein - m_parms.time) * (1.0 / m_parms.pMessage->fadein) * 255);
			}
			else if (m_parms.time > m_parms.fadeTime)
			{
				if (m_parms.pMessage->fadeout > 0)
					m_parms.fadeBlend = (((m_parms.time - m_parms.fadeTime) / m_parms.pMessage->fadeout) * 255);
				else
					m_parms.fadeBlend = 255;
			}
			else
				m_parms.fadeBlend = 0;

			m_parms.charTime = 0;

			if (m_parms.pMessage->effect == 1 && (rand() % 100) < 10)
				m_parms.charTime = 1;

			break;
		}

		case 2:
		{
			m_parms.fadeTime = (m_parms.pMessage->fadein * m_parms.length) + m_parms.pMessage->holdtime;

			if (m_parms.time > m_parms.fadeTime && m_parms.pMessage->fadeout > 0)
				m_parms.fadeBlend = (((m_parms.time - m_parms.fadeTime) / m_parms.pMessage->fadeout) * 255);
			else
				m_parms.fadeBlend = 0;

			break;
		}
	}
}

void CHudMessage::MessageDrawScan(client_message_t *pClientMessage, float time, unsigned int font)
{
	int i, j, length, width;
	wchar_t *pText;
	wchar_t line[256];
	wchar_t textBuf[2048];
	int a, b, c;
	char szTempMessage[256];
	client_textmessage_t *pMessage;
	int iMessageLength;
	wchar_t *pFullText;

	pMessage = pClientMessage->pMessage;

	if (!pMessage->pMessage || !*pMessage->pMessage)
		return;

	iMessageLength = strlen(pMessage->pMessage);

	m_parms.lines = 1;
	m_parms.time = time;
	m_parms.pMessage = pMessage;
	length = 0;
	width = 0;
	m_parms.totalWidth = 0;

	if (*pMessage->pMessage == '#')
	{
		strncpy(szTempMessage, pMessage->pMessage, sizeof(szTempMessage) - 1);
		szTempMessage[sizeof(szTempMessage) - 1] = 0;

		if (pMessage->pMessage[iMessageLength - 1] == '\r' || pMessage->pMessage[iMessageLength - 1] == '\n')
		{
			if (iMessageLength - 1 < sizeof(szTempMessage) - 1)
				szTempMessage[iMessageLength - 1] = 0;
		}

		pFullText = vgui::localize()->Find(szTempMessage);

		if (pClientMessage->numArgs)
		{
			vgui::localize()->ConstructString(textBuf, sizeof(textBuf), pFullText, pClientMessage->numArgs, pClientMessage->args[0], pClientMessage->args[1], pClientMessage->args[2], pClientMessage->args[3]);
			pFullText = textBuf;
		}

		if (!pFullText)
		{
			gEngfuncs.Con_DPrintf("ERROR: Missing %s from the cstrike_english.txt file!\n", pMessage->pMessage);
			return;
		}
	}
	else
	{
		vgui::localize()->ConvertANSIToUnicode(pMessage->pMessage, textBuf, sizeof(textBuf));
		pFullText = textBuf;
	}

	pText = pFullText;

	while (*pText)
	{
		if (*pText == '\n')
		{
			m_parms.lines++;

			if (width > m_parms.totalWidth)
				m_parms.totalWidth = width;

			width = 0;
		}
		else
		{
			vgui::surface()->GetCharABCwide(font, *pFullText, a, b, c);
			width += a + b + c;
		}

		pText++;
		length++;
	}

	m_parms.length = length;
	m_parms.totalHeight = vgui::surface()->GetFontTall(font) * m_parms.lines;

	m_parms.y = YPosition(pMessage->y, m_parms.totalHeight);
	pText = pFullText;

	m_parms.charTime = 0;

	MessageScanStart();

	for (i = 0; i < m_parms.lines; i++)
	{
		m_parms.lineLength = 0;
		m_parms.width = 0;

		while (*pText && *pText != '\n')
		{
			line[m_parms.lineLength] = *pText;

			vgui::surface()->GetCharABCwide(font, *pText, a, b, c);
			m_parms.width += a + b + c;
			m_parms.lineLength++;
			pText++;

			if (m_parms.lineLength >= sizeof(line) / sizeof(wchar_t))
				break;
		}

		pText++;
		line[m_parms.lineLength] = 0;

		m_parms.x = XPosition(pMessage->x, m_parms.width, m_parms.totalWidth);

		for (j = 0; j < m_parms.lineLength; j++)
		{
			m_parms.text = line[j];

			vgui::surface()->GetCharABCwide(font, m_parms.text, a, b, c);

			int next = m_parms.x + a + b + c;

			MessageScanNextChar(font);

			if (m_parms.x >= 0 && m_parms.y >= 0 && next <= ScreenWidth)
				gEngfuncs.pfnVGUI2DrawCharacterAdd(m_parms.x, m_parms.y, m_parms.text, m_parms.r, m_parms.g, m_parms.b, font);

			m_parms.x = next;
		}

		m_parms.y += vgui::surface()->GetFontTall(font);
	}
}

int CHudMessage::Draw(float fTime)
{
	int i, drawn;
	client_textmessage_t *pMessage;
	float endTime;

	drawn = 0;

	if (m_gameTitleTime > 0)
	{
		float localTime = gHUD.m_flTime - m_gameTitleTime;
		float brightness;

		if (m_gameTitleTime > gHUD.m_flTime)
			m_gameTitleTime = gHUD.m_flTime;

		if (localTime > (m_pGameTitle->fadein + m_pGameTitle->holdtime + m_pGameTitle->fadeout))
		{
			m_gameTitleTime = 0;
		}
		else
		{
			brightness = FadeBlend(m_pGameTitle->fadein, m_pGameTitle->fadeout, m_pGameTitle->holdtime, localTime);

			int halfWidth = gHUD.GetSpriteRect(m_HUD_title_half).right - gHUD.GetSpriteRect(m_HUD_title_half).left;
			int fullWidth = halfWidth + gHUD.GetSpriteRect(m_HUD_title_life).right - gHUD.GetSpriteRect(m_HUD_title_life).left;
			int fullHeight = gHUD.GetSpriteRect(m_HUD_title_half).bottom - gHUD.GetSpriteRect(m_HUD_title_half).top;

			int x = XPosition(m_pGameTitle->x, fullWidth, fullWidth);
			int y = YPosition(m_pGameTitle->y, fullHeight);

			SPR_Set(gHUD.GetSprite(m_HUD_title_half), brightness * m_pGameTitle->r1, brightness * m_pGameTitle->g1, brightness * m_pGameTitle->b1);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_title_half));

			SPR_Set(gHUD.GetSprite(m_HUD_title_life), brightness * m_pGameTitle->r1, brightness * m_pGameTitle->g1, brightness * m_pGameTitle->b1);
			SPR_DrawAdditive(0, x + halfWidth, y, &gHUD.GetSpriteRect(m_HUD_title_life));

			drawn = 1;
		}
	}

	for (i = 0; i < maxHUDMessages; i++)
	{
		if (m_pMessages[i].pMessage)
		{
			pMessage = m_pMessages[i].pMessage;

			if (m_startTime[i] > gHUD.m_flTime || m_startTime[i] == 1.0)
				m_startTime[i] = gHUD.m_flTime + m_parms.time - m_startTime[i] + 0.2;
		}
	}

	for (i = 0; i < maxHUDMessages; i++)
	{
		if (m_pMessages[i].pMessage)
		{
			pMessage = m_pMessages[i].pMessage;

			switch (pMessage->effect)
			{
				case 0:
				case 1:
				{
					endTime = m_startTime[i] + pMessage->fadein + pMessage->fadeout + pMessage->holdtime;
					break;
				}

				case 2:
				{
					endTime = m_startTime[i] + (pMessage->fadein * strlen(pMessage->pMessage)) + pMessage->fadeout + pMessage->holdtime;
					break;
				}
			}

			if (fTime <= endTime)
			{
				float messageTime = fTime - m_startTime[i];

				MessageDrawScan(&m_pMessages[i], messageTime, m_pMessages[i].font);
				drawn++;
			}
			else
			{
				m_pMessages[i].pMessage = NULL;
				m_pMessages[i].font = 0;
			}
		}
	}

	m_parms.time = gHUD.m_flTime;

	if (!drawn)
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}

int CHudMessage::MessageAdd(const char *pName, float time, int hintMessage, unsigned int font)
{
	int i;
	client_textmessage_t *tempMessage;
	int emptySlot = -1;

	for (i = 0; i < maxHUDMessages; i++)
	{
		if (m_pMessages[i].pMessage)
		{
			if (m_pMessages[i].hintMessage & hintMessage)
				return -1;
		}
		else
		{
			if (emptySlot == -1)
				emptySlot = i;
		}
	}

	if (emptySlot == -1)
		return -1;

	if (pName[0] == '#')
		tempMessage = gEngfuncs.pfnTextMessageGet(pName + 1);
	else
		tempMessage = gEngfuncs.pfnTextMessageGet(pName);

	if (hintMessage)
	{
		tempMessage->effect = 2;
		tempMessage->r1 = 40;
		tempMessage->g1 = 255;
		tempMessage->b1 = 40;
		tempMessage->a1 = 200;
		tempMessage->r2 = 0;
		tempMessage->g2 = 255;
		tempMessage->b2 = 0;
		tempMessage->a2 = 200;
		tempMessage->x = -1;
		tempMessage->y = 0.7;
		tempMessage->fadein = 0.01;
		tempMessage->fadeout = 0.7;
		tempMessage->fxtime = 0.07;
		tempMessage->holdtime = 5;

		if (!strcmp(pName, "#Spec_Duck"))
		{
			tempMessage->holdtime = 6;
		}
		else if (tempMessage->pMessage)
		{
			tempMessage->holdtime = strlen(tempMessage->pMessage) / 25;

			if (tempMessage->holdtime < 1)
				tempMessage->holdtime = 1;
		}
	}

	m_pMessages[emptySlot].pMessage = tempMessage;
	m_pMessages[emptySlot].font = font;
	m_pMessages[emptySlot].hintMessage = hintMessage;
	m_startTime[emptySlot] = time;
	return emptySlot;
}

int CHudMessage::MsgFunc_HudText(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *pString = READ_STRING();
	int hintMessage = READ_BYTE();

	if (!READ_OK())
		hintMessage = 0;

	MessageAdd(pString, gHUD.m_flTime, hintMessage, font);
	m_parms.time = gHUD.m_flTime;

	if (!(m_iFlags & HUD_ACTIVE))
		m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudMessage::MsgFunc_HudTextPro(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *pString = READ_STRING();
	int hintMessage = READ_BYTE();

	MessageAdd(pString, gHUD.m_flTime, hintMessage, Newfont);
	m_parms.time = gHUD.m_flTime;

	if (!(m_iFlags & HUD_ACTIVE))
		m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudMessage::MsgFunc_HudTextArgs(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *pString = READ_STRING();
	int hintMessage = READ_BYTE();

	int slotNum = MessageAdd(pString, gHUD.m_flTime, hintMessage, Newfont);

	if (slotNum > -1)
	{
		m_pMessages[slotNum].numArgs = max(min(0, READ_BYTE()), MAX_MESSAGE_ARGS);

		for (int i = 0; i < m_pMessages[slotNum].numArgs; i++)
		{
			char *tmp = READ_STRING();

			if (!tmp)
				tmp = "";

			vgui::localize()->ConvertANSIToUnicode(tmp, m_pMessages[slotNum].args[i], MESSAGE_ARG_LEN);
		}
	}

	m_parms.time = gHUD.m_flTime;

	if (!(m_iFlags & HUD_ACTIVE))
		m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudMessage::MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf)
{
	m_pGameTitle = TextMessageGet("GAMETITLE");

	if (m_pGameTitle != NULL)
	{
		m_gameTitleTime = gHUD.m_flTime;

		if (!(m_iFlags & HUD_ACTIVE))
			m_iFlags |= HUD_ACTIVE;
	}

	return 1;
}

void CHudMessage::MessageAdd(client_textmessage_t *newMessage)
{
	m_parms.time = gHUD.m_flTime;

	if (!(m_iFlags & HUD_ACTIVE))
		m_iFlags |= HUD_ACTIVE;

	if (!strcmp(newMessage->pName, "Spec_Duck"))
	{
		MessageAdd(newMessage->pName, gHUD.m_flTime, 1, Newfont);
		return;
	}

	for (int i = 0; i < maxHUDMessages; i++)
	{
		if (!m_pMessages[i].pMessage)
		{
			m_pMessages[i].pMessage = newMessage;
			m_pMessages[i].font = Newfont;
			m_startTime[i] = gHUD.m_flTime;
			return;
		}
	}
}