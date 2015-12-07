#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"

skycamera_t g_SkyCamera;

//sky camera

class CSkyCamera : public CPointEntity
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(sky_camera, CSkyCamera);

void CSkyCamera::Spawn(void)
{
	g_SkyCamera.origin = pev->origin;
	g_SkyCamera.enable = 1;
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

//shadow manager

class CShadowManager : public CPointEntity
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);

public:
	float m_flRadius;
	float m_flFarDist;
	float m_flScale;
	int m_iTexSize;
	string_t m_iAffectModel;
};

LINK_ENTITY_TO_CLASS(shadow_manager, CShadowManager);

void CShadowManager::Spawn(void)
{
	shadow_manager_t &mgr = g_pGameRules->m_ShadowManager[g_pGameRules->m_ShadowManager.AddToTail()];

	strncpy(mgr.affectmodel, STRING(m_iAffectModel), sizeof(mgr.affectmodel)-1);
	mgr.affectmodel[sizeof(mgr.affectmodel) - 1] = '\0';

	mgr.angles = pev->angles;
	mgr.radius = m_flRadius;
	mgr.scale = m_flScale;
	mgr.fard = m_flFarDist;
	mgr.texsize = m_iTexSize;
	
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CShadowManager::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "fard"))
	{
		m_flFarDist = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "scale"))
	{
		m_flScale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "texsize"))
	{
		m_iTexSize = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "affectmodel"))
	{
		m_iAffectModel = MAKE_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}