#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

vec3_t vec3_origin(0, 0, 0);

double sqrt(double x);

float Length(const float *v)
{
	int i;
	float length;

	length = 0;

	for (i = 0; i < 3; i++)
		length += v[i] * v[i];

	length = sqrt(length);
	return length;
}

void VectorAngles(const float *forward, float *angles)
{
	float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(forward[2], tmp) * 180 / M_PI);

		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

float VectorNormalize(float *v)
{
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if (length)
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

void VectorInverse(float *v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void VectorScale(const float *in, float scale, float *out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

void VectorMA(const float *veca, float scale, const float *vecb, float *vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

HSPRITE LoadSprite(const char *pszName)
{
	int i;
	char sz[256];

	if (ScreenWidth < 640)
		i = 320;
	else
		i = 640;

	sprintf(sz, pszName, i);
	return SPR_Load(sz);
}

char *VarArgs(char *format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return string;
}

const wchar_t *NumAsWString(int val)
{
	const int BufLen = 16;
	static wchar_t buf[BufLen];
	int len = BufLen;
	BufWPrintf(buf, len, L"%d", val);
	return buf;
}

wchar_t *BufWPrintf(wchar_t *buf, int &len, const wchar_t *fmt, ...)
{
	if (len <= 0)
		return NULL;

	va_list argptr;

	va_start(argptr, fmt);
	_vsnwprintf(buf, len, fmt, argptr);
	va_end(argptr);

	len -= wcslen(buf);
	return buf + wcslen(buf);
}

char *BufPrintf(char *buf, int &len, const char *fmt, ...)
{
	if (len <= 0)
		return NULL;

	va_list argptr;

	va_start(argptr, fmt);
	_vsnprintf(buf, len, fmt, argptr);
	va_end(argptr);

	len -= strlen(buf);
	return buf + strlen(buf);
}