#include <ctype.h>
#include "UtlBuffer.h"

wchar_t *AdvanceOverWhitespace(wchar_t *Start)
{
	while (*Start != 0 && iswspace(*Start))
		Start++;

	return Start;
}

wchar_t *ReadUnicodeToken(wchar_t *start, wchar_t *token, int tokenBufferSize, bool &quoted)
{
	start = AdvanceOverWhitespace(start);
	quoted = false;
	*token = 0;

	if (!*start)
		return start;

	if (*start == '\"')
	{
		quoted = true;
		start++;

		int count = 0;

		while (*start && *start != '\"' && count < tokenBufferSize-1)
		{
			if (*start == '\\' && *(start + 1) == 'n')
			{
				start++;
				*token = '\n';
			}
			else if (*start == '\\' && *(start + 1) == '\"')
			{
				start++;
				*token = '\"';
			}
			else
			{
				*token = *start;
			}

			start++;
			token++;
			count++;
		}

		if (*start == '\"')
			start++;
	}
	else
	{
		int count = 0;

		while (*start && !iswspace(*start) && count < tokenBufferSize - 1)
		{
			*token = *start;

			start++;
			token++;
			count++;
		}
	}

	*token = 0;
	return start;
}

wchar_t *ReadUnicodeTokenNoSpecial(wchar_t *start, wchar_t *token, int tokenBufferSize, bool &quoted)
{
	start = AdvanceOverWhitespace(start);
	quoted = false;
	*token = 0;

	if (!*start)
	{
		return start;
	}

	if (*start == '\"')
	{
		quoted = true;
		start++;

		int count = 0;

		while (*start && *start != '\"' && count < tokenBufferSize - 1)
		{
			if (*start == '\\' && *(start+1) == '\"')
			{
				start++;
				*token = '\"';
			}
			else
			{
				*token = *start;
			}

			start++;
			token++;
			count++;
		}

		if (*start == '\"')
		{
			start++;
		}
	}
	else
	{
		int count = 0;

		while (*start && !iswspace(*start) && count < tokenBufferSize - 1)
		{
			*token = *start;

			start++;
			token++;
			count++;
		}
	}

	*token = 0;
	return start;
}

wchar_t *ReadToEndOfLine(wchar_t *start)
{
	if (!*start)
		return start;

	while (*start)
	{
		if (*start == 0x0D || *start== 0x0A)
			break;
		start++;
	}

	while (*start == 0x0D || *start== 0x0A)
		start++;

	return start;
}

void WriteUnicodeString(CUtlBuffer &buf, const wchar_t *string, bool addQuotes)
{
	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}

	for (const wchar_t *ws = string; *ws != 0; ws++)
	{
		if (addQuotes && *ws == '\"')
		{
			buf.PutUnsignedShort('\\');
		}

		buf.PutUnsignedShort(*ws);
	}

	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}
}

void WriteAsciiStringAsUnicode(CUtlBuffer &buf, const char *string, bool addQuotes)
{
	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}

	for (const char *sz = string; *sz != 0; sz++)
	{
		if (addQuotes && *sz == '\"')
		{
			buf.PutUnsignedShort('\\');
		}

		buf.PutUnsignedShort(*sz);
	}

	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}
}