#ifndef PLANE_H
#define PLANE_H

class CPlane 
{
public:
	CPlane(void);

public:
	void InitializePlane(const Vector &vecNormal, const Vector &vecPoint);
	BOOL PointInFront(const Vector &vecPoint);

public:
	Vector m_vecNormal;
	float m_flDist;
	BOOL m_fInitialized;
};

#endif