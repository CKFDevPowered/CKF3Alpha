#include <windows.h>
#include "tier1/strtools.h"
#include "common.h"
#include <stdio.h>
#include <time.h>
#include "sys.h"

static qboolean s_com_token_unget = false;

#define COM_TOKEN_MAX_LENGTH 1024

char com_token[COM_TOKEN_MAX_LENGTH];
bool com_ignorecolons = false;

void COM_UngetToken(void)
{
	s_com_token_unget = true;
}

char *COM_Parse(char *data)
{
	int c;
	int len;

	if (s_com_token_unget)
	{
		s_com_token_unget = false;
		return data;
	}

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;

		data++;
	}

	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;

		goto skipwhite;
	}

	if (c == '\"')
	{
		data++;

		while (1)
		{
			c = *data++;

			if (c == '\"' || !c || c > (COM_TOKEN_MAX_LENGTH - 1))
			{
				com_token[len] = 0;
				return data;
			}

			com_token[len] = c;
			len++;
		}
	}

	if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || (!com_ignorecolons && c == ':') || c == ',')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data + 1;
	}

	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;

		if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || (!com_ignorecolons && c == ':') || c == ',')
			break;
	}
	while (c > 32);

	com_token[len] = 0;
	return data;
}

char *COM_ParseFile(char *data, char *token, int maxtoken)
{
	char *return_data = COM_Parse(data);
	Q_strncpy(token, com_token, maxtoken);
	return return_data;
}

char *COM_GetToken(void)
{
	return com_token;
}

char *COM_TimeString(void)
{
	static char timedate[16];
	time_t ltime;

	time(&ltime);
	strftime(timedate, sizeof(timedate), "%y%m%d%H%M", localtime(&ltime));
	return timedate;
}

char *COM_SkipPath(char *pathname)
{
	char *last = pathname;

	while (*pathname)
	{
		if (*pathname == '/' || *pathname == '\\')
			last = pathname + 1;

		pathname++;
	}

	return last;
}

void COM_FileBase(const char *in, char *out)
{
	int len, start, end;

	len = Q_strlen(in);
	end = len - 1;

	while (end && in[end] != '.' && in[end] != '/' && in[end] != '\\')
		end--;

	if (in[end] != '.')
		end = len - 1;
	else
		end--;

	start = len - 1;

	while (start >= 0 && in[start] != '/' && in[start] != '\\')
		start--;

	if (start < 0 || ( in[start] != '/' && in[start] != '\\'))
		start = 0;
	else
		start++;

	len = end - start + 1;

	Q_strncpy(out, &in[start], len);
	out[len] = 0;
}

char *COM_VarArgs(char *format, ...)
{
	va_list va;
	static char string[1024];

	va_start(va, format);
	Q_vsnprintf(string, sizeof(string), format, va);
	va_end(va);

	return string;
}

unsigned char COM_Nibble(char c)
{
	if ((c >= '0') && (c <= '9'))
		return (unsigned char)(c - '0');

	if ((c >= 'A') && (c <= 'F'))
		return (unsigned char)(c - 'A' + 0x0A);

	if ((c >= 'a') && (c <= 'f'))
		return (unsigned char)(c - 'a' + 0x0A);

	return '0';
}

void COM_HexConvert(const char *pszInput, int nInputLength, unsigned char *pOutput)
{
	unsigned char *p;
	int i;
	const char *pIn;

	p = pOutput;

	for (i = 0; i < nInputLength; i += 2)
	{
		pIn = &pszInput[i];
		*p = COM_Nibble(pIn[0]) << 4 | COM_Nibble(pIn[1]);
		p++;
	}
}

char *COM_BinPrintf(unsigned char *buf, int nLen)
{
	static char szReturn[4096];
	unsigned char c;
	char szChunk[10];
	int i;

	Q_memset(szReturn, 0, sizeof(szReturn));

	for (i = 0; i < nLen; i++)
	{
		c = (unsigned char)buf[i];
		Q_snprintf(szChunk, sizeof(szChunk), "%02x", c);
		Q_strcat(szReturn, szChunk, sizeof(szReturn));
	}

	return szReturn;
}

void COM_Init(void)
{
}