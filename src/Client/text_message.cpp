#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "configs.h"

#include "client.h"
#include "CounterStrikeViewport.h"

pfnUserMsgHook g_pfnTextMsg;

DECLARE_MESSAGE(m_TextMessage, TextMsg);

int CHudTextMessage::Init(void)
{
	g_pfnTextMsg = HOOK_MESSAGE(TextMsg);

	gHUD.AddHudElem(this);

	Reset();
	return 1;
}

char *CHudTextMessage::LocaliseTextString(const char *msg, char *dst_buffer, int buffer_size)
{
	char *dst = dst_buffer;

	for (char *src = (char *)msg; *src != 0 && buffer_size > 0; buffer_size--)
	{
		if (*src == '#')
		{
			static char word_buf[255];
			char *wdst = word_buf, *word_start = src;

			for (++src ; (*src >= 'A' && *src <= 'z') || (*src >= '0' && *src <= '9'); wdst++, src++)
				*wdst = *src;

			*wdst = 0;

			client_textmessage_t *clmsg = TextMessageGet(word_buf);

			if (!clmsg || !(clmsg->pMessage))
			{
				src = word_start;
				*dst = *src;
				dst++, src++;
				continue;
			}

			for (char *wsrc = (char *)clmsg->pMessage; *wsrc != 0; wsrc++, dst++)
				*dst = *wsrc;

			*dst = 0;
		}
		else
		{
			*dst = *src;
			dst++, src++;
			*dst = 0;
		}
	}

	dst_buffer[buffer_size - 1] = 0;
	return dst_buffer;
}

char *CHudTextMessage::BufferedLocaliseTextString(const char *msg)
{
	static char dst_buffer[1024];
	LocaliseTextString(msg, dst_buffer, 1024);
	return dst_buffer;
}

char *CHudTextMessage::LookupString(const char *msg, int *msg_dest)
{
	if (!msg)
		return "";

	if (msg[0] == '#')
	{
		client_textmessage_t *clmsg = TextMessageGet(msg + 1);

		if (!clmsg || !(clmsg->pMessage))
			return (char *)msg;

		if (msg_dest)
		{
			if (clmsg->effect < 0)
				*msg_dest = -clmsg->effect;
		}

		return (char *)clmsg->pMessage;
	}
	else
	{
		return (char *)msg;
	}
}

char *ConvertCRtoNL(char *str)
{
	for (char *ch = str; *ch != 0; ch++)
	{
		if (*ch == '\r')
			*ch = '\n';
	}

	return str;
}

wchar_t *ConvertCRtoNL(wchar_t *str)
{
	for (wchar_t *ch = str; *ch != 0; ch++)
	{
		if (*ch == L'\r')
			*ch = L'\n';
	}

	return str;
}

void StripEndNewlineFromString(char *str)
{
	int s = strlen(str) - 1;

	if (s >= 0)
	{
		if (str[s] == '\n' || str[s] == '\r')
			str[s] = 0;
	}
}

void StripEndNewlineFromString(wchar_t *str)
{
	int s = wcslen(str) - 1;

	if (s >= 0)
	{
		if (str[s] == L'\n' || str[s] == L'\r')
			str[s] = 0;
	}
}

char *LookupString(const char *msg, int *msg_dest)
{
	if (!msg)
		return "";

	if (msg[0] == '#')
	{
		client_textmessage_t *clmsg = TextMessageGet(msg + 1);

		if (!clmsg || !(clmsg->pMessage))
			return (char *)msg;

		if (msg_dest)
		{
			if (clmsg->effect < 0)
				*msg_dest = -clmsg->effect;
		}

		return (char *)clmsg->pMessage;
	}
	else
		return (char *)msg;
}

wchar_t *ReadLocalizedString(wchar_t *pOut, int outSize, bool bStripNewline, char *originalString, int originalSize)
{
	char szString[2048];
	safe_strcpy(szString, READ_STRING(), sizeof(szString));

	if (originalString)
		Q_strncpy(originalString, szString, originalSize);

	const wchar_t *pBuf = g_pVGuiLocalize->Find(szString);

	if (pBuf)
	{
		wcsncpy(pOut, pBuf, outSize / sizeof(wchar_t));
		pOut[outSize / sizeof(wchar_t) - 1] = 0;
	}
	else
	{
		char *str = LookupString(szString, NULL);

		if (!str[0])
			g_pVGuiLocalize->ConvertANSIToUnicode(szString, pOut, outSize);
		else
			g_pVGuiLocalize->ConvertANSIToUnicode(str, pOut, outSize);
	}

	if (bStripNewline)
		StripEndNewlineFromString(pOut);

	return pOut;
}

wchar_t *ReadChatTextString(wchar_t *pOut, int outSize)
{
	char szString[2048];
	safe_strcpy(szString, READ_STRING(), sizeof(szString));

	g_pVGuiLocalize->ConvertANSIToUnicode(szString, pOut, outSize);

	StripEndNewlineFromString(pOut);
	return pOut;
}

#define MSG_BUF_SIZE 128

int CHudTextMessage::MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int msg_dest = READ_BYTE();
	int playerIndex = -1;

	if (msg_dest == HUD_PRINTRADIO)
	{
		char *tmp = READ_STRING();

		if (tmp)
			playerIndex = atoi(tmp);
	}

	static char szBuf[6][MSG_BUF_SIZE];
	char *msg_text = LookupString(READ_STRING(), &msg_dest);
	msg_text = safe_strcpy(szBuf[0], msg_text , MSG_BUF_SIZE);

	char *sstr1 = LookupString(READ_STRING());
	sstr1 = safe_strcpy(szBuf[1], sstr1, MSG_BUF_SIZE);
	StripEndNewlineFromString(sstr1);
	char *sstr2 = LookupString(READ_STRING());
	sstr2 = safe_strcpy(szBuf[2], sstr2, MSG_BUF_SIZE);
	StripEndNewlineFromString(sstr2);
	char *sstr3 = LookupString(READ_STRING());
	sstr3 = safe_strcpy(szBuf[3], sstr3, MSG_BUF_SIZE);
	StripEndNewlineFromString(sstr3);
	char *sstr4 = LookupString(READ_STRING());
	sstr4 = safe_strcpy(szBuf[4], sstr4, MSG_BUF_SIZE);
	StripEndNewlineFromString(sstr4);
	char *psz = szBuf[5];

	if (gViewPortInterface && gViewPortInterface->AllowedToPrintText() == FALSE)
		return 1;

	switch (msg_dest)
	{
		case HUD_PRINTRADIO:
		{
			static char szNewBuf[6][MSG_BUF_SIZE];

			if (strlen(sstr1) > 0)
			{
				safe_sprintf(szNewBuf[1], MSG_BUF_SIZE, "#%s", sstr1);

				if (vgui::localize()->Find(szNewBuf[1]))
					sstr1 = szNewBuf[1];
			}

			if (strlen(sstr2) > 0)
			{
				safe_sprintf(szNewBuf[2], MSG_BUF_SIZE, "#%s", sstr2);

				if (vgui::localize()->Find(szNewBuf[2]))
					sstr2 = szNewBuf[2];
			}

			if (strlen(sstr3) > 0)
			{
				safe_sprintf(szNewBuf[3], MSG_BUF_SIZE, "#%s", sstr3);

				if (vgui::localize()->Find(szNewBuf[3]))
					sstr3 = szNewBuf[3];
			}

			if (strlen(sstr4) > 0)
			{
				safe_sprintf(szNewBuf[4], MSG_BUF_SIZE, "#%s", sstr4);

				if (vgui::localize()->Find(szNewBuf[4]))
					sstr4 = szNewBuf[4];
			}

			gHUD.m_SayText.SayTextPrint(msg_text, MSG_BUF_SIZE, playerIndex, sstr1, sstr2, sstr3, sstr4);
			break;
		}

		case HUD_PRINTCENTER:
		{
			StripEndNewlineFromString(msg_text);
			gHUD.m_VGUI2Print.VGUI2HudPrintArgs(msg_text, sstr1, sstr2, sstr3, sstr4, -1, ScreenHeight * 0.35, 1.0, 0.705, 0.118);
			break;
		}

		case HUD_PRINTNOTIFY:
		{
			psz[0] = 1;
			safe_sprintf(psz + 1, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
			ConsolePrint(ConvertCRtoNL(psz));
			break;
		}

		case HUD_PRINTTALK:
		{
			gHUD.m_SayText.SayTextPrint(msg_text, MSG_BUF_SIZE, -1, sstr1, sstr2, sstr3, sstr4);
			break;
		}

		case HUD_PRINTCONSOLE:
		{
			safe_sprintf(psz, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
			ConsolePrint(ConvertCRtoNL(psz));
			break;
		}
	}

	return 1;
}