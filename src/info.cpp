#include <metahook.h>
#include "console.h"

#define MAX_KV_LEN 127

char *Info_ValueForKey(char *s, char *key)
{
	char pkey[512];
	static char value[4][512];
	static int valueindex;
	char *o;

	valueindex = (valueindex + 1) % 4;

	if (*s = '\\')
		s++;

	while (1)
	{
		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return "";

			*o++ = *s++;
		}

		*o = 0;
		s++;
		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";

			*o++ = *s++;
		}

		*o = 0;

		if (!strcmp(key, pkey))
			return value[valueindex];

		if (!*s)
			return "";

		s++;
	}
}

void Info_RemoveKey(char *s, char *key)
{
	char *start;
	char pkey[512];
	char value[512];
	char *o;

	if (strstr(key, "\\"))
	{
		Con_Printf("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;

		if (*s == '\\')
			*s++;

		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;
		s++;
		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;

		if (!strcmp(key, pkey))
		{
			strcpy(start, s);
			return;
		}

		if (!*s)
			return;
	}
}

void Info_RemovePrefixedKeys(char *start, char prefix)
{
	char *s;
	char pkey[512];
	char value[512];
	char *o;

	s = start;

	while (1)
	{
		if (*s == '\\')
			*s++;

		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;
		s++;
		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;

		if (pkey[0] == prefix)
		{
			Info_RemoveKey(s, pkey);
			return;
		}

		if (!*s)
			return;
	}
}

qboolean Info_IsKeyImportant(char *key)
{
	if (key[0] == '*')
		return true;

	if (!strcmp(key, "name"))
		return true;

	if (!strcmp(key, "model"))
		return true;

	if (!strcmp(key, "rate"))
		return true;

	if (!strcmp(key, "topcolor"))
		return true;

	if (!strcmp(key, "bottomcolor"))
		return true;

	if (!strcmp(key, "cl_updaterate"))
		return true;

	if (!strcmp(key, "cl_lw"))
		return true;

	if (!strcmp(key, "cl_lc"))
		return true;

	if (!strcmp(key, "*tracker"))
		return true;

	if (!strcmp(key, "*hltv"))
		return true;

	return false;
}

char *Info_FindLargestKey(char *s)
{
	char key[256];
	char value[512];
	char *o;
	static char largest_key[256];
	size_t size, largest_size;

	size = 0;
	largest_key[0] = 0;
	largest_size = 0;

	if (*s == '\\')
		s++;

	while (*s)
	{
		o = key;

		while (*s && *s != '\\')
			*o++ = *s++;

		*o = 0;
		size = strlen(key);

		if (!*s)
			return largest_key;

		o = value;
		s++;

		while (*s && *s != '\\')
			*o++ = *s++;

		*o = 0;

		if (*s)
			s++;

		size += strlen(value);

		if (size > largest_size && !Info_IsKeyImportant(key))
		{
			largest_size = size;
			strncpy(largest_key, key, sizeof(largest_key) - 1);
			largest_key[sizeof(largest_key) - 1] = 0;
		}
	}

	return largest_key;
}

void Info_SetValueForStarKey(char *s, char *key, char *value, int maxsize)
{
	char _new[1024], *v;
	int c;

	if (strstr(key, "\\") || strstr(value, "\\"))
	{
		Con_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr(key, "..") || strstr(value, ".."))
		return;

	if (strstr(key, "\"") || strstr(value, "\""))
	{
		Con_Printf("Can't use keys or values with a \"\n");
		return;
	}

	if (strlen(key) > MAX_KV_LEN || strlen(value) > MAX_KV_LEN)
	{
		Con_Printf("Keys and values must be < %i characters.\n", MAX_KV_LEN + 1);
		return;
	}

	Info_RemoveKey(s, key);

	if (!value || !strlen(value))
		return;

	_snprintf(_new, sizeof(_new), "\\%s\\%s", key, value);

	if (strlen(_new) + strlen(s) >= (size_t)maxsize)
	{
		if (Info_IsKeyImportant(key))
		{
			char *largekey;

			do
			{
				largekey = Info_FindLargestKey(s);
				Info_RemoveKey(s, largekey);
			}
			while (strlen(_new) + strlen(s) >= (size_t)maxsize && *largekey);

			if (!largekey[0])
			{
				Con_Printf("Info string length exceeded\n");
				return;
			}
		}
		else
		{
			Con_Printf("Info string length exceeded\n");
			return;
		}
	}

	s += strlen(s);
	v = _new;

	while (*v)
	{
		c = (unsigned char)*v++;

		if (c > 13)
			*s++ = c;
	}

	*s = 0;
}

void Info_Print(char *s)
{
	char key[512], value[512];
	char *o;
	size_t l;

	if (*s == '\\')
		s++;

	while (*s)
	{
		o = key;

		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;

		if (l < 20)
		{
			memset(o, ' ', 20 - l);
			key[20] = 0;
		}
		else
			*o = 0;

		Con_Printf("%s", key);

		if (!*s)
		{
			Con_Printf("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;

		while (*s && *s != '\\')
			*o++ = *s++;

		*o = 0;

		if (*s)
			s++;

		Con_Printf("%s\n", value);
	}
}

void Info_SetValueForKey(char *s, char *key, char *value, int maxsize)
{
	if (key[0] == '*')
	{
		Con_Printf("Can't set * keys\n");
		return;
	}

	Info_SetValueForStarKey(s, key, value, maxsize);
}