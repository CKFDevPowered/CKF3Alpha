#include "hud.h"
#include "cl_util.h"
#include "calcscreen.h"

#pragma warning(disable: 4244)

#ifndef NULL
#define NULL ((void *)0)
#endif NULL

#define BOUND_VALUE(var, min, max) if ((var) > (max)) { (var) = (max); }; if ((var) < (min)) { (var) = (min); }

inline float VectorAngle(Vector a, Vector b)
{
	float length_a = a.Length();
	float length_b = b.Length();
	float length_ab = length_a * length_b;

	if (length_ab == 0.0)
		return 0.0;

	return (double)(acos(DotProduct(a, b) / length_ab) * (180 / M_PI));
}

void MakeVector(Vector ain, Vector &vout)
{
	float pitch;
	float yaw;
	float tmp;

	pitch = (ain[0] * M_PI / 180);
	yaw = (ain[1] * M_PI / 180);
	tmp = cos(pitch);

	vout.x = (-tmp * -cos(yaw));
	vout.y = (sin(yaw) * tmp);
	vout.z = -sin(pitch);
}

void VectorRotateX(Vector in, float angle, Vector &out)
{
	float a, c, s;

	a = (angle * M_PI / 180);
	c = cos(a);
	s = sin(a);

	out.x = in.x;
	out.y = c * in.y - s * in.z;
	out.z = s * in.y + c * in.z;
}

void VectorRotateY(Vector in, float angle, Vector &out)
{
	float a, c, s;

	a = (angle * M_PI / 180);
	c = cos(a);
	s = sin(a);

	out.x = c * in.x + s * in.z;
	out.y = in.y;
	out.z = -s * in.x + c * in.z;
}

void VectorRotateZ(Vector in, float angle, Vector &out)
{
	float a, c, s;

	a = (angle * M_PI / 180);
	c = cos(a);
	s = sin(a);

	out.x = c * in.x - s * in.y;
	out.y = s * in.x + c * in.y;
	out.z = in.z;
}

extern vec3_t v_origin, v_angles;

int CalcScreen(float in[3], float out[2])
{
	vec3_t aim;
	vec3_t newaim;
	vec3_t view;
	vec3_t tmp;
	float num;

	VectorSubtract(in, v_origin, aim);
	MakeVector(v_angles, view);

	if (VectorAngle(view, aim) > (gHUD.m_iFOV / 1.8))
		return false;

	VectorRotateZ(aim, -v_angles[1], newaim);
	VectorRotateY(newaim, -v_angles[0], tmp);
	VectorRotateX(tmp, -v_angles[2], newaim);

	if (gHUD.m_iFOV == 0.0)
		return false;

	num = (((ScreenWidth / 2) / newaim[0]) * (120.0 / gHUD.m_iFOV - 1.0 / 3.0));
	out[0] = (ScreenWidth / 2) - num * newaim[1];
	out[1] = (ScreenHeight / 2) - num * newaim[2];

	BOUND_VALUE(out[0], 0, (ScreenWidth / 2) * 2);
	BOUND_VALUE(out[1], 0, (ScreenHeight / 2) * 2);
	return true;
}
