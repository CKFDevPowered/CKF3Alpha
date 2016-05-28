#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "CounterStrikeViewport.h"

#include "Configs.h"

float *GetClientColor(int clientIndex);
extern float g_LocationColor[3];

float *GetTextColor(int colorNum, int clientIndex)
{
	switch (colorNum)
	{
		case TEXTCOLOR_PLAYERNAME:
		{
			return GetClientColor(clientIndex);
		}

		case TEXTCOLOR_LOCATION:
		{
			return g_LocationColor;
		}
	}

	return NULL;
}

#define MAX_LINES 5
#define MAX_CHARS_PER_LINE 256

#define MAX_LINE_WIDTH (ScreenWidth - 40)
#define LINE_START 10

static float SCROLL_SPEED = 5;

struct TextRange
{
	int start;
	int end;
	float *color;
};

class SayTextLine
{
public:
	SayTextLine(void);

public:
	void Clear(void);
	void Draw(int x, int y, float r, float g, float b);
	void Colorize(void);
	void SetText(wchar_t *buf, int clientIndex);

public:
	wchar_t m_line[MAX_CHARS_PER_LINE];
	CUtlVector<TextRange> m_textRanges;
	int m_clientIndex;
	float *m_teamColor;

public:
	SayTextLine &operator = (SayTextLine &other);
};

SayTextLine::SayTextLine(void)
{
	Clear();
}

void SayTextLine::Clear(void)
{
	m_textRanges.RemoveAll();
	m_line[0] = 0;
}

void SayTextLine::Draw(int x, int y, float r, float g, float b)
{
	return ;

	int rangeIndex;
	TextRange *range;
	wchar_t ch;

	if (m_textRanges.Size() != 0)
	{
		for (rangeIndex = 0; rangeIndex < m_textRanges.Size(); rangeIndex++)
		{
			range = &m_textRanges[rangeIndex];

			ch = m_line[range->end];
			m_line[range->end] = 0;

			if (range->color)
				x = gHUD.m_VGUI2Print.DrawVGUI2String(m_line + range->start, x, y, range->color[0], range->color[1], range->color[2]);
			else
				x = gHUD.m_VGUI2Print.DrawVGUI2String(m_line + range->start, x, y, r, g, b);

			m_line[range->end] = ch;
		}
	}
	else
	{
		gHUD.m_VGUI2Print.DrawVGUI2String(m_line, x, y, r, g, b);
		return;
	}
}

SayTextLine &SayTextLine::operator = (SayTextLine &other)
{
	m_textRanges.RemoveAll();
	m_textRanges.SetCount(other.m_textRanges.Count());

	for (int i = 0; i < other.m_textRanges.Size(); i++)
		m_textRanges[i] = other.m_textRanges[i];

	m_clientIndex = other.m_clientIndex;
	m_teamColor[0] = other.m_teamColor[0];
	m_teamColor[1] = other.m_teamColor[1];
	m_teamColor[2] = other.m_teamColor[2];

	memcpy(m_line, other.m_line, sizeof(m_line));
	return *this;
}

void SayTextLine::Colorize(void)
{
	wchar_t *txt = m_line;
	int lineLen = wcslen(m_line);

	if (m_line[0] == TEXTCOLOR_PLAYERNAME || m_line[0] == TEXTCOLOR_LOCATION || m_line[0] == TEXTCOLOR_NORMAL)
	{
		while (txt && *txt)
		{
			TextRange range;
			int count;

			switch (*txt)
			{
				case TEXTCOLOR_PLAYERNAME:
				case TEXTCOLOR_LOCATION:
				case TEXTCOLOR_NORMAL:
				{
					range.start = (txt - m_line) + 1;

					switch (*txt)
					{
						case TEXTCOLOR_PLAYERNAME:
						{
							range.color = m_teamColor;
							break;
						}

						case TEXTCOLOR_LOCATION:
						{
							range.color = g_LocationColor;
							break;
						}

						default:
						{
							range.color = NULL;
						}
					}

					range.end = lineLen;
					count = m_textRanges.Count();

					if (count)
						m_textRanges[count - 1].end = range.start - 1;

					m_textRanges.AddToTail(range);
					break;
				}
			}

			txt++;
		}
	}
	else if (m_line[0] == TEXTCOLOR_USEOLDCOLORS)
	{
		wchar_t wName[128];
		char *pName = g_PlayerInfoList[m_clientIndex].name;

		vgui::localize()->ConvertANSIToUnicode(pName, wName, sizeof(wName));

		if (pName)
		{
			wchar_t *nameInString = wcsstr(m_line, wName);

			if (nameInString)
			{
				TextRange range;
				range.start = 1;
				range.end = wcslen(wName) + (nameInString - m_line);
				range.color = GetClientColor(m_clientIndex);
				m_textRanges.AddToTail(range);

				range.start = range.end;
				range.end = wcslen(m_line);
				range.color = NULL;
				m_textRanges.AddToTail(range);
			}
		}
	}

	if (!m_textRanges.Size())
	{
		TextRange range;
		range.start = 0;
		range.end = wcslen(m_line);
		range.color = NULL;
		m_textRanges.AddToTail(range);
	}
}

void SayTextLine::SetText(wchar_t *buf, int clientIndex)
{
	Clear();

	if (buf)
	{
		wcsncpy(m_line, buf, MAX_CHARS_PER_LINE);
		m_line[MAX_CHARS_PER_LINE - 1] = 0;
		m_clientIndex = clientIndex;
		m_teamColor = NULL;

		if (clientIndex > 0)
		{
			GetPlayerInfo(clientIndex, &g_PlayerInfoList[clientIndex]);
			m_teamColor = GetClientColor(clientIndex);
			Colorize();
		}
		else
		{
			TextRange range;
			range.start = 0;
			range.end = wcslen(m_line);
			range.color = NULL;
			m_textRanges.AddToTail(range);
		}
	}
}

static SayTextLine g_sayTextLine[MAX_LINES + 1];
static float flScrollTime = 0;

static int Y_START = 0;
static int line_height = 0;

DECLARE_MESSAGE(m_SayText, SayText);
DECLARE_MESSAGE(m_SayText, SendAudio);
DECLARE_MESSAGE(m_SayText, SendRadio);
DECLARE_COMMAND(m_SayText, MessageMode);
DECLARE_COMMAND(m_SayText, MessageMode2);

pfnUserMsgHook g_pfnSayText;
pfnUserMsgHook g_pfnSendAudio;
pfnUserMsgHook g_pfnSendRadio;

xcommand_t g_pfnMessageMode;
xcommand_t g_pfnMessageMode2;

int CHudSayText::Init(void)
{
	g_pfnSendAudio = HOOK_MESSAGE(SendAudio);
	g_pfnSendRadio = HOOK_MESSAGE(SendRadio);
	g_pfnSayText = HOOK_MESSAGE(SayText);

	g_pfnMessageMode = HOOK_COMMAND("messagemode", MessageMode);
	g_pfnMessageMode2 = HOOK_COMMAND("messagemode2", MessageMode2);

	InitHUDData();

	m_HUD_saytext = gEngfuncs.pfnGetCvarPointer("hud_saytext_internal");
	m_HUD_saytext_time = gEngfuncs.pfnGetCvarPointer("hud_saytext_time");

	m_iFlags |= HUD_INTERMISSION;

	gHUD.AddHudElem(this);
	return 1;
}

void CHudSayText::InitHUDData(void)
{
	for (int i = 0; i < MAX_LINES; i++)
	{
		g_sayTextLine[i].m_line[0] = 0;
		g_sayTextLine[i].m_textRanges.RemoveAll();
	}
}

int CHudSayText::VidInit(void)
{
	return 1;
}

void CHudSayText::Reset(void)
{
}

void CHudSayText::UserCmd_MessageMode(void)
{
	if (!gConfigs.bEnableClientUI)
		return g_pfnMessageMode();

	if (!gHUD.m_pCvarDraw->value || g_pViewPort->IsScoreBoardVisible())
		return g_pfnMessageMode();

	if (!gHUD.m_iIntermission && gEngfuncs.Cmd_Argc() == 1 && cl_newchat->value)
		return g_pViewPort->StartMessageMode();

	return g_pfnMessageMode();
}

void CHudSayText::UserCmd_MessageMode2(void)
{
	if (!gConfigs.bEnableClientUI)
		return g_pfnMessageMode2();

	if (!gHUD.m_pCvarDraw->value || g_pViewPort->IsScoreBoardVisible())
		return g_pfnMessageMode2();

	if (!gHUD.m_iIntermission && gEngfuncs.Cmd_Argc() == 1 && cl_newchat->value)
		return g_pViewPort->StartMessageMode2();

	return g_pfnMessageMode2();
}

int ScrollTextUp(void)
{
	g_sayTextLine[MAX_LINES].Clear();
	memmove(&g_sayTextLine[0], &g_sayTextLine[1], sizeof(g_sayTextLine) - sizeof(g_sayTextLine[0]));
	g_sayTextLine[MAX_LINES - 1].Clear();

	if (g_sayTextLine[0].m_line[0] == ' ')
	{
		g_sayTextLine[0].m_line[0] = 2;
		return 1 + ScrollTextUp();
	}

	return 1;
}

int CHudSayText::Draw(float flTime)
{
	int y = Y_START;
	char *colour;
	int r, g, b;
	float fR, fG, fB;

	if (gConfigs.bEnableClientUI)
	{
		if (g_pViewPort->AllowedToPrintText() == FALSE)
			return 1;
	}
	else
	{
		if (gViewPortInterface && gViewPortInterface->AllowedToPrintText() == FALSE)
			return 1;
	}

	if (!m_HUD_saytext->value)
		return 1;

	flScrollTime = min(flScrollTime, flTime + m_HUD_saytext_time->value);

	if (flScrollTime <= flTime)
	{
		if (g_sayTextLine[0].m_line[0])
		{
			flScrollTime = flTime + m_HUD_saytext_time->value;
			ScrollTextUp();
		}
		else
		{
			m_iFlags &= ~HUD_ACTIVE;
		}
	}

	gEngfuncs.pfnDrawConsoleString(10, y, "");

	colour = gEngfuncs.pfnGetCvarString("con_color");
	sscanf(colour, "%i %i %i", &r, &g, &b);

	fR = r / 255.0;
	fG = g / 255.0;
	fB = b / 255.0;

	for (int i = 0; i < MAX_LINES; i++)
	{
		if (g_sayTextLine[i].m_line[0])
			g_sayTextLine[i].Draw(ScreenWidth * 0.05, y, fR, fG, fB);

		y += line_height;
	}

	return 1;
}

int CHudSayText::MsgFunc_SayText(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client_index = READ_BYTE();

	char formatStr[256];
	safe_strcpy(formatStr, READ_STRING(), sizeof(formatStr));

	char sstr1[256], sstr2[256], sstr3[256], sstr4[256];
	safe_strcpy(sstr1, READ_STRING(), sizeof(sstr1));
	safe_strcpy(sstr2, READ_STRING(), sizeof(sstr2));
	safe_strcpy(sstr3, READ_STRING(), sizeof(sstr3));
	safe_strcpy(sstr4, READ_STRING(), sizeof(sstr4));

	if (!sstr1[0] && client_index > 0)
		strcpy(sstr1, g_PlayerInfoList[client_index].name);

	SayTextPrint(formatStr, iSize - 1, client_index, sstr1, sstr2, sstr3, sstr4);
	return 1;
}

int CHudSayText::MsgFunc_SendAudio(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client = READ_BYTE();
	char *code = READ_STRING();
	int pitch = READ_SHORT();

	char name[64];
	sprintf(name, "misc/talk.wav");

	char *pName = name;
	char *pCode = strstr(code, "%!");

	if (pCode)
	{
		{
			pCode++;

			while (*pCode)
			{
				if (*pCode <= 32 || *pCode > 122)
					break;

				*pName++ = *pCode++;
			}

			*pName = '\0';
		}

		gEngfuncs.pfnPlaySoundByNameAtPitch(name, 1.0, pitch);
	}
	else
	{
		gEngfuncs.pfnPlaySoundByNameAtPitch(code, 1.0, pitch);
	}

	g_PlayerExtraInfo[client].radarflashes = 22;
	g_PlayerExtraInfo[client].radarflash = gHUD.m_flTime;
	g_PlayerExtraInfo[client].radarflashon = 1;
	return 1;
}

int CHudSayText::MsgFunc_SendRadio(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client = READ_BYTE();
	char *file = READ_STRING();
	int pitch = READ_SHORT();

	char name[64];
	sprintf(name, "radio/%s.wav", file);
	gEngfuncs.pfnPlaySoundVoiceByName(name, 1.0, pitch);

	g_PlayerExtraInfo[client].radarflashes = 22;
	g_PlayerExtraInfo[client].radarflash = gHUD.m_flTime;
	g_PlayerExtraInfo[client].radarflashon = 1;
	return 1;
}

int CHudSayText::GetTextPrintY(void)
{
	int iRetVal;

	if (g_iUser1 || gEngfuncs.IsSpectateOnly())
	{
		iRetVal = (ScreenHeight - gViewPortInterface->GetSpectatorBottomBarHeight() - 4) - (line_height * MAX_LINES) - (line_height * 0.5);
	}
	else
	{
		iRetVal = (ScreenHeight - gHUD.m_iFontHeight * 8) - (line_height * MAX_LINES) - (line_height * 0.5);
	}

	return iRetVal;
}

void CHudSayText::SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex, char *sstr1, char *sstr2, char *sstr3, char *sstr4)
{
	if (gViewPortInterface && gViewPortInterface->AllowedToPrintText() == FALSE)
	{
		ConsolePrint(pszBuf);
		return;
	}

	int lineNum;
	wchar_t finalBuffer[MAX_CHARS_PER_LINE];
	const char *localized;
	char buf[MAX_CHARS_PER_LINE];
	int len;
	wchar_t *msg;
	wchar_t temp[MAX_CHARS_PER_LINE];
	wchar_t out[MAX_CHARS_PER_LINE];

	if (clientIndex >= 0 && clientIndex <= MAX_PLAYERS)
		GetPlayerInfo(clientIndex, &g_PlayerInfoList[clientIndex]);

	for (lineNum = 0; lineNum < MAX_LINES; lineNum++)
	{
		if (!g_sayTextLine[lineNum].m_line[0])
			break;
	}

	if (lineNum == MAX_LINES)
	{
		ScrollTextUp();
		lineNum = MAX_LINES - 1;
	}

	len = strlen(pszBuf);

	if (pszBuf[len - 1] == '\n' || pszBuf[len - 1] == '\r')
	{
		strncpy(buf, pszBuf, sizeof(buf) - 1);
		buf[len - 1] = 0;
		localized = buf;
	}
	else
	{
		strncpy(buf, pszBuf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = 0;
		localized = buf;
	}

	msg = vgui::localize()->Find(localized);

	if (!msg)
	{
		wchar_t *wsz;
		wchar_t locTerm[128];
		bool inword;
		int charsLeft;
		wchar_t *wordPtr;
		int outPos;
		wchar_t ch;
		int wordLen;
		wchar_t *localizedTerm;
		int num;

		vgui::localize()->ConvertANSIToUnicode(localized, temp, sizeof(temp));

		if (temp[0])
		{
			wsz = temp;
			charsLeft = wcslen(temp);
			inword = false;
			wordPtr = NULL;
			outPos = 0;

			while (charsLeft)
			{
				ch = *wsz;

				if (inword)
				{
					if (iswspace(ch) || charsLeft == 1)
					{
						wordLen = wsz - wordPtr;
						wcsncpy(locTerm, wordPtr, wordLen + 1);

						if (charsLeft == 1)
							locTerm[wordLen + 1] = 0;
						else
							locTerm[wordLen] = 0;

						if (vgui::localize()->ConvertUnicodeToANSI(locTerm, buf, sizeof(buf)))
							localizedTerm = vgui::localize()->Find(buf);
						else
							localizedTerm = locTerm;

						num = min((int)wcslen(localizedTerm), MAX_CHARS_PER_LINE - outPos);
						wcsncpy(&out[outPos], localizedTerm, num);
						outPos += num;

						if (charsLeft > 1)
						{
							out[outPos] = ch;
							outPos++;
						}

						inword = false;
					}
				}
				else
				{
					if (ch == '#')
					{
						inword = true;
						wordPtr = wsz;
					}
					else
					{
						if (outPos < MAX_CHARS_PER_LINE)
						{
							out[outPos] = ch;
							outPos++;
						}
					}
				}

				wsz++;
				charsLeft--;
			}

			if (outPos < MAX_CHARS_PER_LINE)
				out[outPos] = 0;
			else
				out[MAX_CHARS_PER_LINE - 1] = 0;
		}

		msg = out;
	}

	static wchar_t wstr[4][256];
	wchar_t *w[4];
	char *sptrs[4];
	int j;
	bool useStdPrintf;
	const wchar_t *test;

	sptrs[0] = sstr1;
	sptrs[1] = sstr2;
	sptrs[2] = sstr3;
	sptrs[3] = sstr4;

	w[0] = NULL;
	w[1] = NULL;
	w[2] = NULL;
	w[3] = NULL;

	for (j = 0; j < 4; j++)
	{
		if (sptrs[j][0] == '#')
		{
			w[j] = vgui::localize()->Find(sptrs[j]);
		}

		if (!w[j])
		{
			vgui::localize()->ConvertANSIToUnicode(sptrs[j], wstr[j], sizeof(wstr[j]));
			w[j] = wstr[j];
		}
	}

	test = wcsstr(msg, L"%s");
	useStdPrintf = (test && !(test[2] > '0' && test[2] < '9'));

	if (useStdPrintf)
	{
		_snwprintf(finalBuffer, sizeof(finalBuffer), msg, w[0], w[1], w[2], w[3]);
	}
	else
	{
		vgui::localize()->ConstructString(finalBuffer, sizeof(finalBuffer), msg, 4, w[0], w[1], w[2], w[3]);
	}

	g_sayTextLine[lineNum].SetText(finalBuffer, clientIndex);

	EnsureTextFitsInOneLineAndWrapIfHaveTo(lineNum);

	if (lineNum == 0)
		flScrollTime = gHUD.m_flTime + m_HUD_saytext_time->value;

	m_iFlags |= HUD_ACTIVE;

	Y_START = GetTextPrintY();

	g_pViewPort->ChatPrintf(clientIndex, finalBuffer);
}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo(int line)
{
	int line_width = 0;
	gHUD.m_VGUI2Print.GetStringSize(g_sayTextLine[line].m_line, &line_width, &line_height);

	if ((line_width + LINE_START) > MAX_LINE_WIDTH)
	{
		int length = LINE_START;
		int tmp_len = 0;
		wchar_t *last_break = NULL;

		for (wchar_t *x = g_sayTextLine[line].m_line; *x != 0; x++)
		{
			if (x[0] == '/' && x[1] == '(')
			{
				x += 2;

				while (*x != 0 && *x != ')')
					x++;

				if (*x != 0)
					x++;

				if (*x == 0)
					break;
			}

			wchar_t buf[2];
			buf[1] = 0;

			if (*x == ' ' && x != g_sayTextLine[line].m_line)
				last_break = x;

			buf[0] = *x;
			gHUD.m_VGUI2Print.GetStringSize(buf, &tmp_len, &line_height);
			length += tmp_len;

			if (length > MAX_LINE_WIDTH)
			{
				if (!last_break)
					last_break = x - 1;

				x = last_break;

				int j;

				do
				{
					for (j = 0; j < MAX_LINES; j++)
					{
						if (!*g_sayTextLine[j].m_line)
							break;
					}

					if (j == MAX_LINES)
					{
						int linesmoved = ScrollTextUp();
						line -= linesmoved;
						last_break = last_break - (sizeof(g_sayTextLine[0].m_line) * linesmoved);
					}
				}
				while (j == MAX_LINES);

				if ((wchar_t)*last_break == (wchar_t)' ')
				{
					int linelen = wcslen(g_sayTextLine[j].m_line);
					int remaininglen = wcslen(last_break);

					if ((linelen - remaininglen) <= MAX_CHARS_PER_LINE)
					{
						wcsncat(g_sayTextLine[j].m_line, last_break, MAX_CHARS_PER_LINE);
						g_sayTextLine[j].m_line[MAX_CHARS_PER_LINE - 1] = 0;
						g_sayTextLine[j].Colorize();
					}
				}
				else
				{
					if ((wcslen(g_sayTextLine[j].m_line) - wcslen(last_break) - 2) < MAX_CHARS_PER_LINE)
					{
						wcsncat(g_sayTextLine[j].m_line, L" ", MAX_CHARS_PER_LINE);
						wcsncat(g_sayTextLine[j].m_line, last_break, MAX_CHARS_PER_LINE);
						g_sayTextLine[j].m_line[MAX_CHARS_PER_LINE - 1] = 0;
						g_sayTextLine[j].Colorize();
					}
				}

				*last_break = 0;

				EnsureTextFitsInOneLineAndWrapIfHaveTo(j);
				break;
			}
		}
	}
}