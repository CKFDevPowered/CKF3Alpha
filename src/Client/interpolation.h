#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#ifdef _WIN32
#pragma once
#endif

class CInterpolation
{
public:
	CInterpolation(void);
	virtual ~CInterpolation(void);

public:
	void SetWaypoints(vec3_t *prev, vec3_t start, vec3_t end, vec3_t *next);
	void SetViewAngles(vec3_t start, vec3_t end);
	void SetFOVs(float start, float end);
	void SetSmoothing(bool start, bool end);
	void Interpolate(float t, vec3_t &point, vec3_t &angle, float *fov);

protected:
	void BezierInterpolatePoint(float t, vec3_t &point);
	void InterpolateAngle(float t, vec3_t &angle);

	vec3_t m_StartPoint;
	vec3_t m_EndPoint;
	vec3_t m_StartAngle;
	vec3_t m_EndAngle;
	vec3_t m_Center;
	float m_StartFov;
	float m_EndFov;

	bool m_SmoothStart;
	bool m_SmoothEnd;
};

#endif