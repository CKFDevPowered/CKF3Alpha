#include "extdll.h"
#include "plane.h"

CPlane::CPlane(void)
{
	m_fInitialized = FALSE;
}

void CPlane::InitializePlane(const Vector &vecNormal, const Vector &vecPoint)
{
	m_vecNormal = vecNormal;
	m_flDist = DotProduct(m_vecNormal, vecPoint);
	m_fInitialized = TRUE;
}

BOOL CPlane::PointInFront(const Vector &vecPoint)
{
	if (!m_fInitialized)
		return FALSE;

	float flFace = DotProduct(m_vecNormal, vecPoint) - m_flDist;

	if (flFace >= 0)
		return TRUE;

	return FALSE;
}