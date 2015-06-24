#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "buildable.h"
#include "monsters.h"
#include "weapons.h"
#include "shake.h"
#include "gamerules.h"

extern int gmsgHUDBuild;
extern int g_fIsTraceLine;

LINK_ENTITY_TO_CLASS( buildable_sentry, CBuildSentry );
LINK_ENTITY_TO_CLASS( buildable_dispenser, CBuildDispenser );
LINK_ENTITY_TO_CLASS( buildable_teleporter, CBuildTeleporter );

int CBaseBuildable::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (pev->takedamage < DAMAGE_YES)
		return 0;

	if(!m_pPlayer)
		return 0;

	CBaseEntity *pAttacker = Instance(pevAttacker);

	if(pAttacker->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pAttacker;
		if(pPlayer->m_iTeam == m_iTeam)
			return 0;
	}

	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		g_pGameRules->BuildKilled(this, pevAttacker, pevInflictor);//death msg or sth..

		Killed(pevAttacker, GIB_NORMAL);
		return 0;
	}

	UpdateHUD();

	return 1;
}

void CBaseBuildable::Killed(entvars_t *pevAttacker, int iGib)
{
	if(!m_pPlayer)
		return;

	m_iLevel = 0;

	if(iGib != -1)
	{
		switch(RANDOM_LONG(0,2))
		{
		case 0:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode1.wav", 1.0, 0.80);break;
		case 1:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode2.wav", 1.0, 0.80);break;
		case 2:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode3.wav", 1.0, 0.80);break;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_EXPLOSION_MIDAIR);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		MESSAGE_END();

		UpdateHUD();

		if(m_pPlayer)
		{
			m_pPlayer->m_pBuildable[GetBuildClass()-1] = NULL;
		}
	}

	m_pPlayer = NULL;

	if(m_pSapper)
	{
		m_pSapper->Killed(pev, 0);
		m_pSapper = NULL;
	}

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBaseBuildable::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit)
{
	if (!pev->takedamage)
		return;

	if(!m_pPlayer)
		return;

	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType, iCrit);

	if(bitsDamageType & DMG_BULLET)
		UTIL_Ricochet( vecOrigin, 0.20 );
}

BOOL CBaseBuildable::Repair(CBasePlayer *pPlayer)
{
	BOOL bUpdate = FALSE;
	BOOL bSuccess = FALSE;
	if(m_iFlags & BUILD_BUILDING)
	{
		UpgradeProgress(5.0);
		bUpdate = TRUE;
		bSuccess = TRUE;
	}
	if(m_pSapper)
	{
		if(pPlayer->m_pActiveItem)
		{
			m_pSapper->TakeDamage(pPlayer->m_pActiveItem->pev, pPlayer->pev, 65, DMG_NEVERGIB | DMG_SLASH);
		}
		bUpdate = TRUE;
		bSuccess = TRUE;
	}
	if(pev->health < pev->max_health)
	{
		float flDamage = min(pev->max_health - pev->health, 105);
		int iMetalNeeded = int(1.0 + flDamage / 5.0);
		if(pPlayer->m_iMetal && iMetalNeeded)
		{
			if(iMetalNeeded > pPlayer->m_iMetal)
			{
				flDamage = flDamage * (float)pPlayer->m_iMetal / (float)iMetalNeeded;
				iMetalNeeded = pPlayer->m_iMetal;
			}
			pev->health = min(pev->health + flDamage, pev->max_health);
			pPlayer->AddPlayerMetal(-iMetalNeeded);

			bUpdate = TRUE;
			bSuccess = TRUE;
		}
	}
	if(bUpdate)
		UpdateHUD();
	if(bSuccess)
		return TRUE;
	return FALSE;
}

//Sentry

void CBuildSentry::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv1.mdl");
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv2.mdl");
	PRECACHE_MODEL("models/CKF_III/w_sentry_lv3.mdl");
	PRECACHE_SOUND("CKF_III/build_deploy.wav");
	PRECACHE_SOUND("CKF_III/sentry_shoot.wav");
}

CBaseBuildable *CBuildSentry::CreateBuildable(Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner)
{
	CBuildSentry *pSentry = GetClassPtr((CBuildSentry *)NULL);

	pSentry->pev->origin = vecOrigin;

	vecAngles.x *= -1;
	vecAngles.z = 0;
	pSentry->pev->angles = vecAngles;

	pSentry->m_pPlayer = pOwner;
	pSentry->pev->team = pSentry->m_iTeam = pOwner->m_iTeam;		
	pSentry->Spawn();

	pOwner->m_pBuildable[BUILDABLE_SENTRY-1] = (CBaseBuildable *)pSentry;

	return pSentry;
}

void CBuildSentry::Spawn(void)
{
	//These follow the steps of turrent
	Precache();
	pev->classname = MAKE_STRING("buildable_sentry");
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_SLIDEBOX;
	pev->effects = 0;
	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;
	pev->view_ofs.z = 18;

	pev->deadflag = DEAD_NO;
	pev->gravity = 1;

	//Model
	SET_MODEL(ENT(pev), "models/CKF_III/w_sentry_lv1.mdl");
	pev->modelindex = MODEL_INDEX("models/CKF_III/w_sentry_lv1.mdl");
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,48));
	pev->mins = Vector(-16,-16,0);
	pev->maxs = Vector(16,16,48);

	//Anim
	pev->sequence = 2;
	pev->framerate = 1;
	pev->frame = 0;
	ResetSequenceInfo();

	UTIL_SetOrigin(pev, pev->origin);
	DROP_TO_FLOOR(ENT(pev));

	pev->max_health = 1;
	pev->health = 1;

	pev->colormap = (m_iTeam == TEAM_RED) ? (1 | (1<<8)) : (140 | (140<<8));

	m_iLevel = 1;
	m_flProgress = 0;
	m_iUpgrade = 0;
	m_iFlags |= BUILD_BUILDING;
	m_iAmmo = m_iMaxAmmo = 100;
	m_iRocket = m_iMaxRocket = 20;
	m_flROF = 0.2;
	m_vecCurAngles.y = m_vecGoalAngles.y = UTIL_AngleMod(pev->angles.y);
	m_vecCurAngles.x = m_vecGoalAngles.x = 0;
	SetBoneController( 0, 0 );
	SetBoneController( 1, 0 );
	m_flStartYaw = UTIL_AngleMod(pev->angles.y);
	pev->frags = 0;
	m_iPredictRocket = 0;

	SetThink(&CBuildSentry::UpgradeThink);//WTF!!!!!
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/build_deploy.wav", VOL_NORM, ATTN_NORM);

	UpdateHUD();
}

BOOL CBuildSentry::Repair(CBasePlayer *pPlayer)
{
	if(CBaseBuildable::Repair(pPlayer))
		return TRUE;
	BOOL bUpdate = FALSE;
	BOOL bSuccess = FALSE;
	if(!(m_iFlags & BUILD_BUILDING) && !(m_iFlags & BUILD_UPGRADING))
	{
		int iMetalTotalCost = 0;
		if(m_iAmmo < m_iMaxAmmo)
		{
			int iAmmo = min(m_iMaxAmmo - m_iAmmo, 50);
			int iMetalNeeded = iAmmo;
			if(pPlayer->m_iMetal && iMetalNeeded)
			{
				if(iMetalNeeded > pPlayer->m_iMetal)
				{
					iAmmo = iAmmo * pPlayer->m_iMetal / iMetalNeeded;
					iMetalNeeded = pPlayer->m_iMetal;
				}
				m_iAmmo = min(m_iAmmo + iAmmo, m_iMaxAmmo);
				//pPlayer->m_iMetal -= iMetalNeeded;
				//pPlayer->pev->armorvalue = pPlayer->m_iMetal;
				iMetalTotalCost += iMetalNeeded;
				bSuccess = TRUE;
				bUpdate = TRUE;
			}
		}
		if(m_iRocket < m_iMaxRocket)
		{
			int iRocket = min(m_iMaxRocket - m_iRocket, 50);
			int iMetalNeeded = iRocket;
			if(pPlayer->m_iMetal && iMetalNeeded)
			{
				if(iMetalNeeded > pPlayer->m_iMetal)
				{
					iRocket = iRocket * pPlayer->m_iMetal / iMetalNeeded;
					iMetalNeeded = pPlayer->m_iMetal;
				}
				m_iRocket = min(m_iRocket + iRocket, m_iMaxRocket);
				//pPlayer->m_iMetal -= iMetalNeeded;
				//pPlayer->pev->armorvalue = pPlayer->m_iMetal;
				iMetalTotalCost += iMetalNeeded;
				bSuccess = TRUE;
				bUpdate = TRUE;
			}
		}
		if(m_iUpgrade < 200 && m_iLevel < 3)
		{
			int iUpgrade = min(200 - m_iUpgrade, 25);
			int iMetalNeeded = iUpgrade;
			if(pPlayer->m_iMetal && iMetalNeeded)
			{
				if(iMetalNeeded > pPlayer->m_iMetal)
				{
					iUpgrade = iUpgrade * pPlayer->m_iMetal / iMetalNeeded;
					iMetalNeeded = pPlayer->m_iMetal;
				}
				m_iUpgrade = min(m_iUpgrade + iUpgrade, 200);
				//pPlayer->m_iMetal -= iMetalNeeded;
				//pPlayer->pev->armorvalue = pPlayer->m_iMetal;
				iMetalTotalCost += iMetalNeeded;
				if(m_iUpgrade >= 200)
				{
					UpgradeMe();
				}
				bSuccess = TRUE;
				bUpdate = TRUE;
			}
		}
		if(iMetalTotalCost)
			pPlayer->AddPlayerMetal(-iMetalTotalCost);
	}
	if(bUpdate)
		UpdateHUD();
	if(bSuccess)
		return TRUE;
	return FALSE;
}

void CBuildSentry::UpgradeMe(void)
{
	if(m_iUpgrade < 200)
		return;

	if((m_iFlags & BUILD_BUILDING) || (m_iFlags & BUILD_UPGRADING))
		return;

	if(m_iLevel == 3)
		return;

	m_iLevel ++;

	m_iFlags |= BUILD_UPGRADING;

	m_flProgress = 0;
	m_iUpgrade = 0;

	int iOldMaxAmmo = m_iMaxAmmo;

	m_iMaxAmmo = (m_iLevel == 2) ? 120 : 144;

	m_iAmmo = m_iAmmo * m_iMaxAmmo / iOldMaxAmmo;

	SET_MODEL(ENT(pev), (m_iLevel == 2) ? "models/CKF_III/w_sentry_lv2.mdl" : "models/CKF_III/w_sentry_lv3.mdl");
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,48));
	pev->sequence = 2;
	pev->frame = 0;
	pev->framerate = 1;
	ResetSequenceInfo();

	SetThink(&CBuildSentry::UpgradeThink);
	pev->nextthink = gpGlobals->time + 0.1;

	UpdateHUD();
}

void CBuildSentry::UpgradeThink(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if(!(m_iFlags & BUILD_BUILDING) && !(m_iFlags & BUILD_UPGRADING))
		return;

	if(m_iLevel == 1)
	{
		UpgradeProgress(1);
		pev->frame = m_flProgress*2.55;
	}
	else
	{
		UpgradeProgress(5);		
		pev->frame = min(m_flProgress*2.55, 235);
	}

	if(m_flProgress >= 100)
	{
		Upgraded();
		return;
	}
}

void CBuildSentry::UpgradeProgress(float flProgressIncrease)
{
	if(m_flProgress + flProgressIncrease > 100)
		flProgressIncrease = 100 - m_flProgress;

	float flNextMaxHealth = 0;
	float flCurMaxHealth = 0;

	if(m_iFlags & BUILD_BUILDING)
	{
		flNextMaxHealth = 150;
		flCurMaxHealth = 1;
	}
	else if(m_iLevel == 2)
	{
		flNextMaxHealth = 180;
		flCurMaxHealth = 150;
	}
	else if(m_iLevel == 3)
	{
		flNextMaxHealth = 216;
		flCurMaxHealth = 180;
	}
	pev->health = min(pev->health + flProgressIncrease * (flNextMaxHealth-flCurMaxHealth)/100.0, flNextMaxHealth);

	if(pev->health > pev->max_health)
		pev->max_health = pev->health;

	m_flProgress += flProgressIncrease;
}

void CBuildSentry::Upgraded(void)
{
	m_flProgress = 0;
	m_iUpgrade = 0;

	pev->sequence = 0;
	ResetSequenceInfo();

	m_bStartTurn = 1;

	SetThink(&CBuildSentry::ScanThink);		
	pev->nextthink = gpGlobals->time + 0.1;

	if(m_iFlags & BUILD_BUILDING)
	{
		m_iFlags &= ~BUILD_BUILDING;
		pev->max_health = 150;
	}
	else if(m_iFlags & BUILD_UPGRADING)
	{
		m_iFlags &= ~BUILD_UPGRADING;
		pev->max_health = (m_iLevel == 2) ? 180 : 216;
		pev->view_ofs.z = 30;
		m_flROF = 0.1;
	}

	UpdateHUD();
}

void CBuildSentry::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	//Prepare to Draw Tracer
	int iTracerColor = 0;
	
	Vector vecDst = CKFFireBullets(vecSrc, vecDirToEnemy, 0.02, 8192, BULLET_SENTRY_TF2, RANDOM_LONG(11, 13), 0, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);
	
	DrawCoordTracer(vecSrc, vecDst, iTracerColor);

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/sentry_shoot.wav", 1, ATTN_NORM);
	pev->effects |= EF_MUZZLEFLASH;
	m_iAmmo --;
	UpdateHUD();
}

void CBuildSentry::ShootRocketPre(entvars_t *pevVictim, Vector vecSrc, Vector vecEnemy)
{
	vecSrc = vecSrc + gpGlobals->v_up * 20 + gpGlobals->v_forward * 8;

	Vector vecEnd = vecEnemy + Vector(0,0,-28);//Shoot the Feet

	Vector vecVelocity = vecEnd - vecSrc;

	float flDistance = vecVelocity.Length();

	if(flDistance > 200 && m_iPredictRocket % 3 == 1)
	{
		vecEnd = vecEnd + pevVictim->velocity * (flDistance / 800);
		vecVelocity = vecEnd - vecSrc;//Recalc
	}
	m_iPredictRocket ++;

	Vector vecRocketAngles = UTIL_VecToAngles(vecVelocity);
	vecRocketAngles.x *= -1;

	ShootRocket(vecSrc, vecRocketAngles);
}

void CBuildSentry::ShootRocket(Vector &vecSrc, Vector &vecAngles)
{
	UTIL_MakeVectors(vecAngles);
	CRocket *pRocket = CRocket::CreatePjRocket(vecSrc, vecAngles, m_pPlayer);

	pRocket->pev->classname = MAKE_STRING("pj_senrocket");
	pRocket->m_iCrit = 0;
	pRocket->m_iTeam = m_iTeam;
	pRocket->pev->frame = 0;
	pRocket->pev->skin = m_iTeam-1;
	pRocket->m_iPjFlags = PJ_AIRBLAST_DEFLECTABLE | PJ_AIRBLAST_OWNER;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_ROCKETTRAIL);
	WRITE_SHORT(ENTINDEX( pRocket->edict() ));
	MESSAGE_END();

	m_iRocket --;
}

void CBuildSentry::ScanThink(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.02;

	if(m_pSapper)
		return;

	// If we have a target and we're still healthy
	if (m_hEnemy)
	{
		if (!m_hEnemy->IsAlive() )
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}

	// Acquire Target
	if (m_hEnemy == NULL && m_iAmmo)
		LookForTarget();

	if(m_hEnemy)
	{
		pev->sequence = 1;
		ResetSequenceInfo();
		pev->framerate = 0;
		m_flLastSight = 0;
		SetThink(&CBuildSentry::ActiveThink);
	}
	else
	{
		float y = UTIL_AngleDiff(m_vecCurAngles.y, m_flStartYaw);
		BOOL bShouldTurn = FALSE;
		if(m_bStartTurn)
		{
			if(y > 0)
				y = -45;
			else
				y = 45;
			m_bStartTurn = 0;
			bShouldTurn = 1;
		}
		else
		{
			if(y > 40 && y < 50)
			{
				y = -45;
				bShouldTurn = 1;
			}
			else if(y < -40 && y > -50)
			{
				y = 45;
				bShouldTurn = 1;
			}
		}
		if(bShouldTurn)
		{
			m_vecGoalAngles.x = 0;
			m_vecGoalAngles.y = y+m_flStartYaw;
			m_flTurnRate = SENTRY_TURNRATE_SLOW;
			if (m_vecGoalAngles.y > 360)
				m_vecGoalAngles.y -= 360;
			if (m_vecGoalAngles.y < 0)
				m_vecGoalAngles.y += 360;
		}
		RotateMe();
	}
}

void CBuildSentry::ActiveThink(void)
{
	int fAttack = 0;
	BOOL bStopImmediately = FALSE;
	BOOL bStopActive = FALSE;
	BOOL bEnemyVisible = FALSE;
	Vector vecDirToEnemy;
	Vector vecGoalAngles;
	Vector vecSrc;
	Vector vecAng;

	pev->nextthink = gpGlobals->time + 0.02;
	StudioFrameAdvance();

	if(m_pSapper)
		return;

	if(g_pGameRules->m_iRoundStatus == ROUND_END && g_pGameRules->m_iRoundWinStatus != m_pPlayer->m_iTeam)
		return;

	if (!m_hEnemy || !m_iAmmo)
	{
		bStopActive = TRUE;
		bStopImmediately = TRUE;
	}

	if(m_hEnemy)
	{
		if(m_hEnemy->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)Instance(m_hEnemy->pev);
			if(pPlayer->m_iCloak == CLOAK_YES)
				bStopActive = TRUE;
		}
		if ( !m_hEnemy->IsAlive())
		{
			bStopActive = TRUE;
		}

		Vector vecMid = pev->origin + pev->view_ofs;
		Vector vecMidEnemy = m_hEnemy->Center();

		// Look for our current enemy
		bEnemyVisible = FALSE;

		TraceResult tr;

		UTIL_TraceLine(vecMid, vecMidEnemy, ignore_monsters, edict(), &tr);

		if(tr.flFraction == 1)
			bEnemyVisible = TRUE;

		vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
		float flDistToEnemy = vecDirToEnemy.Length();

		vecGoalAngles = UTIL_VecToAngles(vecDirToEnemy);
		if(vecGoalAngles.x > 270.0f) vecGoalAngles.x = 360.0f-vecGoalAngles.x;
		else if(vecGoalAngles.x < 90.0f) vecGoalAngles.x = -vecGoalAngles.x;
		vecGoalAngles.x *= -1;

		// Current enmey is not visible.
		if (!bEnemyVisible || flDistToEnemy > SENTRY_RANGE)
		{
			bStopActive = TRUE;
		}

		UTIL_MakeAimVectors(m_vecCurAngles);

		Vector vecLOS = vecDirToEnemy.Normalize();

		// Is the Gun looking at the target
		if (DotProduct(vecLOS, gpGlobals->v_forward) <= 0.887) // 30 degree slop cos(30)
			fAttack = FALSE;
		else if(!m_iAmmo)
			fAttack = FALSE;
		else
			fAttack = TRUE;

		// fire the gun
		if (!bStopActive && fAttack && m_flNextAttack <= gpGlobals->time)
		{
			if(m_iLevel == 1)
			{
				vecSrc = vecMid + gpGlobals->v_up * 8 + gpGlobals->v_forward * 6;
			}
			else
			{
				float flGun = (m_iAmmo % 2) ? 8 : -8;
				vecSrc = vecMid + gpGlobals->v_up * 8 + gpGlobals->v_forward * 8 + gpGlobals->v_right * flGun;
			}
			Shoot(vecSrc, gpGlobals->v_forward);
			m_flNextAttack = gpGlobals->time + m_flROF;
			if(m_iLevel == 3 && m_iRocket && m_flNextRocket <= gpGlobals->time)
			{
				ShootRocketPre(m_hEnemy->pev, vecMid, vecMidEnemy);
				m_flNextRocket = gpGlobals->time + 4;
			}
		}
	}

	if(bStopActive)
	{
		if (!m_flLastSight && !bStopImmediately)
		{
			m_flLastSight = gpGlobals->time + 0.5; // continue-shooting timeout
		}
		else
		{
			if (gpGlobals->time > m_flLastSight)
			{	
				m_hEnemy = NULL;
				m_flLastSight = gpGlobals->time + 1;
				m_bStartTurn = 1;
				m_flTurnRate = SENTRY_TURNRATE_SLOW;
				pev->sequence = 0;
				ResetSequenceInfo();
				SetThink(&CBuildSentry::ScanThink);
				return;
			}
		}
	}
	else
	{
		if(pev->framerate < 1.0)
			pev->framerate = pev->framerate + 0.05;
	}

	if (bEnemyVisible)
	{
		if (vecGoalAngles.y > 360)
			vecGoalAngles.y -= 360;

		if (vecGoalAngles.y < 0)
			vecGoalAngles.y += 360;

		if (vecGoalAngles.x > SENTRY_MAXPITCH)
			vecGoalAngles.x = SENTRY_MAXPITCH;
		else if (vecGoalAngles.x < SENTRY_MINPITCH)
			vecGoalAngles.x = SENTRY_MINPITCH;

		m_vecGoalAngles.y = vecGoalAngles.y;
		m_vecGoalAngles.x = vecGoalAngles.x;
		m_flTurnRate = SENTRY_TURNRATE;
	}
	RotateMe();
}

void CBuildSentry::LookForTarget(void)
{
	Vector vecSrc = pev->origin + pev->view_ofs;
	CBaseEntity *pEntity = NULL;
	CBaseEntity *pCloest = NULL;
	float flClosestDist = 0;
	float flDistance = 0;	
	float flRadius = SENTRY_RANGE;
	BOOL bIsPlayer = FALSE;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage < DAMAGE_YES)
			continue;

		bIsPlayer = (pEntity->IsPlayer() && pEntity->IsAlive()) ? TRUE : FALSE;

		if(bIsPlayer)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
			if ( pPlayer->m_iTeam == m_iTeam )//Team Check
				continue;
			if( pPlayer->m_iCloak == CLOAK_YES )
				continue;
			if( pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam)
				continue;
		}
		else if(pEntity->Classify() == CLASS_BUILDABLE)
		{
			if ( ((CBaseBuildable *)pEntity)->m_iTeam == m_iTeam)
				continue;
		}
		else
			continue;

		TraceResult tr;
		Vector vecSpot = pEntity->BodyTarget(vecSrc);

		UTIL_TraceLine(vecSrc, vecSpot, ignore_monsters, edict(), &tr);

		float flDistance = ( vecSpot - vecSrc ).Length();

		if (tr.flFraction == 1)
		{
			if(!pCloest || flClosestDist > flDistance)
			{
				pCloest = pEntity;
				flClosestDist = flDistance;
			}
		}
	}
	if(pCloest)
		m_hEnemy = pCloest;
}

void CBuildSentry::RotateMe(void)
{
	if (m_vecCurAngles.x != m_vecGoalAngles.x)//Pitch
	{
		float flDir = (m_vecGoalAngles.x > m_vecCurAngles.x) ? 1 : -1 ;

		m_vecCurAngles.x += 5.0 * flDir;

		if (flDir == 1)
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		SetBoneController(1, m_vecCurAngles.x);
	}

	if (m_vecCurAngles.y != m_vecGoalAngles.y)//Yaw
	{
		float flDir;
		float flDist = m_vecGoalAngles.y - m_vecCurAngles.y;
		
		if(flDist > 0)
		{
			if(flDist > 180)
			{
				flDist = 360 - flDist;
				flDir = -1.0;
			}else{
				flDir = 1.0;			
			}
		}
		else if(flDist < 0)
		{
			flDist *= -1;
			if(flDist > 180)
			{
				flDir = 360 - flDist;
				flDir = 1.0;
			}else{
				flDir = -1.0;			
			}			
		}
		float flTurn = 0.02 * m_flTurnRate;

		m_vecCurAngles.y += flTurn * flDir;

		m_vecCurAngles.y = UTIL_AngleMod(m_vecCurAngles.y);

		if (fabs(UTIL_AngleDiff(m_vecGoalAngles.y, m_vecCurAngles.y)) <= flTurn)
			m_vecCurAngles.y = m_vecGoalAngles.y;

		SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
		//g_engfuncs.pfnClientPrintf(m_pPlayer->edict(), print_console, UTIL_VarArgs("view %f\n", UTIL_AngleMod(m_pPlayer->pev->v_angle.y)));
		//g_engfuncs.pfnClientPrintf(m_pPlayer->edict(), print_center, UTIL_VarArgs("cur %f goal %f\n", m_vecCurAngles.y, m_vecGoalAngles.y));
	}
}

void CBuildSentry::UpdateHUD(void)
{
	if(!m_pPlayer)
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgHUDBuild, NULL, m_pPlayer->pev);
	WRITE_BYTE(BUILDABLE_SENTRY);
	WRITE_BYTE(m_iLevel);
	if(m_iLevel != 0)
	{
		WRITE_BYTE(m_iFlags);//maxium (1<<7)
		WRITE_BYTE(pev->health);
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE(150);
		else
			WRITE_BYTE(pev->max_health);
		WRITE_BYTE(m_iAmmo);
		WRITE_BYTE(m_iMaxAmmo);	
		WRITE_BYTE(min(pev->frags, 255));
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE((int)(m_flProgress * 255.0 / 100));
		if(m_iLevel < 3)
			WRITE_BYTE(m_iUpgrade);
		else
		{
			WRITE_BYTE(m_iRocket);
			WRITE_BYTE(m_iMaxRocket);
		}
		MESSAGE_END();
	}
	else
		MESSAGE_END();
}

//Dispenser

void CBuildDispenser::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/w_dispenser.mdl");
	PRECACHE_SOUND("CKF_III/build_deploy.wav");
	PRECACHE_SOUND("CKF_III/dispenser_heal.wav");
}

CBaseBuildable *CBuildDispenser::CreateBuildable(Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner)
{
	CBuildDispenser *pDispenser = GetClassPtr((CBuildDispenser *)NULL);

	pDispenser->pev->origin = vecOrigin;

	vecAngles.x *= -1;
	vecAngles.z = 0;
	pDispenser->pev->angles = vecAngles;

	pDispenser->m_pPlayer = pOwner;
	pDispenser->pev->team = pDispenser->m_iTeam = pOwner->m_iTeam;
	pDispenser->Spawn();

	pOwner->m_pBuildable[BUILDABLE_DISPENSER-1] = (CBaseBuildable *)pDispenser;

	return pDispenser;
}

void CBuildDispenser::Spawn(void)
{
	//These follow the steps of turrent
	Precache();
	pev->classname = MAKE_STRING("buildable_dispenser");
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_SLIDEBOX;
	pev->effects = 0;
	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;

	pev->deadflag = DEAD_NO;
	pev->gravity = 1;

	//Model
	SET_MODEL(ENT(pev), "models/CKF_III/w_dispenser.mdl");
	pev->modelindex = MODEL_INDEX("models/CKF_III/w_dispenser.mdl");
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,52));
	pev->mins = Vector(-16,-16,0);
	pev->maxs = Vector(16,16,52);

	//Anim
	pev->sequence = 1;
	pev->frame = 0;
	ResetSequenceInfo();
	pev->framerate = 0.5;//better do it in mdl

	UTIL_SetOrigin(pev, pev->origin);
	DROP_TO_FLOOR(ENT(pev));

	pev->max_health = 1;
	pev->health = 1;

	pev->colormap = (m_iTeam == TEAM_RED) ? (1 | (1<<8)) : (140 | (140<<8));

	m_iLevel = 1;
	m_flProgress = 0;
	m_iUpgrade = 0;
	m_iFlags |= BUILD_BUILDING;
	m_iMetal = 25;
	m_iMaxMetal = 400;
	m_flRadius = 120;
	m_bShouldUpdateHUD = false;

	SetThink(&CBuildDispenser::UpgradeThink);//WTF!!!!!
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/build_deploy.wav", VOL_NORM, ATTN_NORM);

	UpdateHUD();
}

BOOL CBuildDispenser::Repair(CBasePlayer *pPlayer)
{
	if(CBaseBuildable::Repair(pPlayer))
		return TRUE;
	BOOL bUpdate = FALSE;
	BOOL bSuccess = FALSE;
	if(!(m_iFlags & BUILD_BUILDING) && !(m_iFlags & BUILD_UPGRADING))
	{
		if(m_iUpgrade < 200 && m_iLevel < 3)
		{
			int iUpgrade = min(200 - m_iUpgrade, 25);
			int iMetalNeeded = iUpgrade;
			if(pPlayer->m_iMetal && iMetalNeeded)
			{
				if(iMetalNeeded > pPlayer->m_iMetal)
				{
					iUpgrade = iUpgrade * pPlayer->m_iMetal / iMetalNeeded;
					iMetalNeeded = pPlayer->m_iMetal;
				}
				m_iUpgrade = min(m_iUpgrade + iUpgrade, 200);
				//pPlayer->m_iMetal -= iMetalNeeded;
				//pPlayer->pev->armorvalue = pPlayer->m_iMetal;
				pPlayer->AddPlayerMetal(-iMetalNeeded);
				if(m_iUpgrade >= 200)
				{
					UpgradeMe();
				}
				bSuccess = TRUE;
				bUpdate = TRUE;
			}
		}
	}
	if(bUpdate)
		UpdateHUD();
	if(bSuccess)
		return TRUE;
	return FALSE;
}

void CBuildDispenser::UpgradeMe(void)
{
	if(m_iUpgrade < 200)
		return;

	if((m_iFlags & BUILD_BUILDING) || (m_iFlags & BUILD_UPGRADING))
		return;

	if(m_iLevel == 3)
		return;

	m_iLevel ++;

	m_iFlags |= BUILD_UPGRADING;

	m_flProgress = 0;
	m_iUpgrade = 0;

	pev->sequence = (m_iLevel == 2) ? 3 : 5;//Wait to set
	ResetSequenceInfo();

	SetThink(&CBuildDispenser::UpgradeThink);
	pev->nextthink = gpGlobals->time + 0.1;

	UpdateHUD();

	ClearQueue();
}

void CBuildDispenser::UpgradeThink(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if(!(m_iFlags & BUILD_BUILDING) && !(m_iFlags & BUILD_UPGRADING))
		return;

	if(m_iLevel == 1)
	{
		UpgradeProgress(0.5);
		pev->frame = m_flProgress*2.55;
	}
	else
	{
		UpgradeProgress(5);		
		pev->frame = min(m_flProgress*2.55, 235);
	}

	if(m_flProgress >= 100)
	{
		Upgraded();
		return;
	}
}

void CBuildDispenser::UpgradeProgress(float flProgressIncrease)
{
	if(m_flProgress + flProgressIncrease > 100)
		flProgressIncrease = 100 - m_flProgress;

	float flNextMaxHealth = 0;
	float flCurMaxHealth = 0;

	if(m_iFlags & BUILD_BUILDING)
	{
		flNextMaxHealth = 150;
		flCurMaxHealth = 1;
	}
	else if(m_iLevel == 2)
	{
		flNextMaxHealth = 180;
		flCurMaxHealth = 150;
	}
	else if(m_iLevel == 3)
	{
		flNextMaxHealth = 216;
		flCurMaxHealth = 180;
	}
	pev->health = min(pev->health + flProgressIncrease * (flNextMaxHealth-flCurMaxHealth)/100.0, flNextMaxHealth);

	if(pev->health > pev->max_health)
		pev->max_health = pev->health;

	m_flProgress += flProgressIncrease;
}

void CBuildDispenser::Upgraded(void)
{
	m_flProgress = 0;
	m_iUpgrade = 0;
	pev->sequence = 0;
	ResetSequenceInfo();

	SetThink(&CBuildDispenser::ResupplyThink);
	pev->nextthink = gpGlobals->time + 0.1;

	if(m_iFlags & BUILD_BUILDING)
	{
		m_iFlags &= ~BUILD_BUILDING;
		m_flNextRegenerate = gpGlobals->time + 5;
		pev->max_health = 150;
	}
	else if(m_iFlags & BUILD_UPGRADING)
	{
		m_iFlags &= ~BUILD_UPGRADING;
		pev->max_health = (m_iLevel == 2) ? 180 : 216;
		pev->sequence = (m_iLevel == 2) ? 2 : 4;
	}

	UpdateHUD();	
}

void CBuildDispenser::UpdateHUD(void)
{
	if(!m_pPlayer)
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgHUDBuild, NULL, m_pPlayer->pev);
	WRITE_BYTE(BUILDABLE_DISPENSER);
	WRITE_BYTE(m_iLevel);
	if(m_iLevel != 0)
	{
		WRITE_BYTE(m_iFlags);//maxium (1<<7)
		WRITE_BYTE(pev->health);
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE(150);
		else
			WRITE_BYTE(pev->max_health);
		WRITE_BYTE(m_iMetal/2);//prevent to be larger than 255
		WRITE_BYTE(m_iMaxMetal/2);
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE((int)(m_flProgress * 255.0 / 100));
		if(m_iLevel < 3)
			WRITE_BYTE(m_iUpgrade);
		MESSAGE_END();
	}
	else
		MESSAGE_END();
}

void CBuildDispenser::ResupplyThink(void)
{
	if(!m_pPlayer)
		return;

	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance();

	if(m_pSapper)
		return;

	int heal, ammo, metal, cloak;
	switch(m_iLevel)
	{
		case 1:
			heal = 10;
			ammo = 20;
			metal = 40;
			cloak = 5;
			break;
		case 2:
			heal = 15;
			ammo = 30;
			metal = 50;
			cloak = 10;
			break;
		default:
			heal = 20;
			ammo = 40;
			metal = 60;
			cloak = 15;
			break;
	}
	m_bShouldUpdateHUD = false;
	if(m_flNextRegenerate < gpGlobals->time)
	{
		m_iMetal = min(m_iMetal+metal, m_iMaxMetal);
		m_flNextRegenerate = gpGlobals->time + 5;
		m_bShouldUpdateHUD = true;
	}
	if(m_flNextScan < gpGlobals->time)
	{
		CheckQueue();
		ScanPlayer();
		m_flNextScan = gpGlobals->time + 0.5;
	}
	if(m_flNextResupply < gpGlobals->time)
	{
		ResupplyPlayer(ammo, metal);
		m_flNextResupply = gpGlobals->time + 1.0f;
	}
	if(m_flNextBeam < gpGlobals->time)
	{
		DispenserBeam();
		m_flNextBeam = gpGlobals->time + 1.0f;
	}
	HealPlayer(heal, cloak);

	if(m_bShouldUpdateHUD)
		UpdateHUD();
}

void CBuildDispenser::ResupplyPlayer(int iAmmo, int iMetal)
{
	CBasePlayer *pPlayer;
	bool bPlaySnd;

	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		if(!m_Queue[i])
			continue;
		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer || !pPlayer->IsPlayer())
			continue;
		if(!pPlayer->IsAlive())
			continue;

		bPlaySnd = false;
		if(m_iMetal > 0 && pPlayer->m_iClass == CLASS_ENGINEER && pPlayer->m_iMetal < 200)
		{
			int iMetalGive = iMetal;
			if(iMetalGive > m_iMetal) iMetalGive = m_iMetal;
			if(pPlayer->EatAmmoBox(0, iMetalGive/2, 0))
			{
				bPlaySnd = true;
				m_iMetal -= iMetalGive;
				m_bShouldUpdateHUD = true;
			}
		}
		if(pPlayer->EatAmmoBox(iAmmo, 0, 0))
		{
			bPlaySnd = true;
		}

		if(bPlaySnd)
		{
			EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 0.4, ATTN_NORM);
		}
	}
}

void CBuildDispenser::DispenserBeam(void)
{
	CBasePlayer *pPlayer;

	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		if(!m_Queue[i])
			continue;
		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer || !pPlayer->IsPlayer())
			continue;
		if(!pPlayer->IsAlive())
			continue;

		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_MEDIBEAM);
		WRITE_SHORT(ENTINDEX( ENT(pev) ) );
		WRITE_SHORT(ENTINDEX( pPlayer->edict() ));
		WRITE_BYTE(m_iTeam-1);
		MESSAGE_END();
	}
}

void CBuildDispenser::HealPlayer(int iHeal, int iCloak)
{
	CBasePlayer *pPlayer;

	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		if(!m_Queue[i])
			continue;
		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer || !pPlayer->IsPlayer())
			continue;
		if(!pPlayer->IsAlive())
			continue;

		float flHeal = (float)iHeal/10;

		if(pPlayer->m_iDisguise == DISGUISE_YES)
		{
			if(pPlayer->m_iDisguiseHealth < pPlayer->m_iDisguiseMaxHealth)
			{
				pPlayer->m_iDisguiseHealth = min(pPlayer->m_iDisguiseHealth + (float)iHeal/10, pPlayer->m_iDisguiseMaxHealth);
			}
		}

		if(pPlayer->pev->health < pPlayer->pev->max_health)
		{
			if(pPlayer->pev->health + flHeal > pPlayer->pev->max_health)
				flHeal = pPlayer->pev->max_health - pPlayer->pev->health;

			pPlayer->pev->health += flHeal;
			m_pPlayer->m_flHealAmount += flHeal;
		}

		if(pPlayer->m_iClass == CLASS_SPY)
		{
			pPlayer->m_flCloakEnergy = min(pPlayer->m_flCloakEnergy + (float)iCloak/10, 100);
			pPlayer->pev->armorvalue = pPlayer->m_flCloakEnergy;
		}
	}
}

int CBuildDispenser::FShouldCollide(CBaseEntity *pHit)
{
	if(g_fIsTraceLine)
	{
		if(pHit->IsBSPModel() && pHit->pev->team != 0)
		{
			if(m_pPlayer && pHit->pev->team == m_pPlayer->m_iTeam)
				return 0;
			if(pHit->pev->team == 3)
				return 0;
			if(g_pGameRules->m_iRoundStatus == ROUND_END)
				return 0;
			return 1;
		}
		if(pHit->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pHit;
			if(m_pPlayer && pPlayer->m_iTeam == m_pPlayer->m_iTeam)
				return 0;
		}
	}
	return 1;
}

void CBuildDispenser::ScanPlayer(void)
{
	CBaseEntity *pEntity = NULL;
	CBasePlayer *pPlayer = NULL;

	TraceResult tr;
	Vector vecSrc = Center();
	Vector vecSpot = vecSrc;
	
	while((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, m_flRadius)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;

		vecSpot = pEntity->Center();

		UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pev), &tr);

		if(tr.flFraction != 1)
			continue;

		if((vecSpot - vecSrc).Length() > m_flRadius)
			continue;

		pPlayer = (CBasePlayer *)pEntity;
		JoinQueue(pPlayer);
		m_flNextBeam = gpGlobals->time;
	}
}

void CBuildDispenser::JoinQueue(CBasePlayer *pPlayer)
{
	if(!pPlayer->IsAlive())
		return;

	if(pPlayer->m_iTeam != m_iTeam && pPlayer->m_iDisguise != DISGUISE_YES)
		return;

	if(m_Queue[pPlayer->entindex()])
		return;

	m_Queue[pPlayer->entindex()] = 1;
	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "CKF_III/dispenser_heal.wav", VOL_NORM, ATTN_NORM);//Check before Play
}

void CBuildDispenser::CheckQueue(void)
{
	CBasePlayer *pPlayer;
	bool bShouldQuitQueue;

	Vector vecSrc = Center();
	
	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		if(!m_Queue[i]) continue;

		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if(!pPlayer) continue;
		if(!pPlayer->IsPlayer()) continue;

		bShouldQuitQueue = false;

		if((pPlayer->Center() - vecSrc).Length() > m_flRadius)
			bShouldQuitQueue = true;

		if(pPlayer->m_iTeam != m_iTeam && !(pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam))
			bShouldQuitQueue = true;

		if(!pPlayer->IsAlive())
			bShouldQuitQueue = true;

		if(bShouldQuitQueue)
		{
			m_Queue[i] = 0;

			MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
			WRITE_BYTE(FX_KILLMEDIBEAM);
			WRITE_SHORT(ENTINDEX( ENT(pev) ) );
			WRITE_SHORT(ENTINDEX( pPlayer->edict() ));
			MESSAGE_END();

			EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "CKF_III/null.wav", VOL_NORM, ATTN_NORM);//Check before Play
		}
	}
}

void CBuildDispenser::ClearQueue(void)
{
	CBasePlayer *pPlayer;
	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		if(!m_Queue[i]) continue;

		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if(!pPlayer) continue;
		if(!pPlayer->IsPlayer()) continue;
	
		m_Queue[i] = 0;
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_KILLMEDIBEAM);
		WRITE_SHORT(ENTINDEX( ENT(pev) ) );
		WRITE_SHORT(ENTINDEX( pPlayer->edict() ));
		MESSAGE_END();
	}
}

void CBuildDispenser::Killed(entvars_t *pevAttacker, int iGib)
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "CKF_III/null.wav", VOL_NORM, ATTN_NORM);//Check before Play

	ClearQueue();

	return CBaseBuildable::Killed(pevAttacker, iGib);
}

//Teleporter

void CBuildTeleporter::Precache( void )
{
	PRECACHE_MODEL("models/CKF_III/w_teleporter.mdl");
	PRECACHE_SOUND("CKF_III/build_deploy.wav");
	PRECACHE_SOUND("CKF_III/teleporter_send.wav");
	PRECACHE_SOUND("CKF_III/teleporter_receive.wav");
	PRECACHE_SOUND("CKF_III/teleporter_ready.wav");
	PRECACHE_SOUND("CKF_III/teleporter_spin.wav");
}

CBaseBuildable *CBuildTeleporter::CreateBuildable(BOOL bIsEntrance, Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner)
{
	CBuildTeleporter *pTeleporter = GetClassPtr((CBuildTeleporter *)NULL);

	pTeleporter->pev->origin = vecOrigin;

	vecAngles.x *= -1;
	vecAngles.z = 0;
	pTeleporter->pev->angles = vecAngles;

	pTeleporter->m_pPlayer = pOwner;
	pTeleporter->pev->team = pTeleporter->m_iTeam = pOwner->m_iTeam;
	pTeleporter->m_bIsEntrance = bIsEntrance;
	pTeleporter->Spawn();

	int idx = (pTeleporter->m_bIsEntrance) ? (BUILDABLE_ENTRANCE-1) : (BUILDABLE_EXIT-1);
	pOwner->m_pBuildable[idx] = (CBaseBuildable *)pTeleporter;

	idx = (pTeleporter->m_bIsEntrance) ? (BUILDABLE_EXIT-1) : (BUILDABLE_ENTRANCE-1);
	CBuildTeleporter *pLinkTele = (CBuildTeleporter *)pOwner->m_pBuildable[idx];

	if(pLinkTele)
	{
		pTeleporter->m_pLinkTele = pLinkTele;
		pLinkTele->m_pLinkTele = pTeleporter;
		pTeleporter->m_iUpgrade = pLinkTele->m_iUpgrade;
		pTeleporter->m_iLevel = pLinkTele->m_iLevel;
	}

	return pTeleporter;
}

void CBuildTeleporter::Spawn(void)
{
	//These follow the steps of turrent
	Precache();
	pev->classname = MAKE_STRING("buildable_teleporter");
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_SLIDEBOX;
	pev->effects = 0;
	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;

	pev->deadflag = DEAD_NO;
	pev->gravity = 1;

	//Model
	SET_MODEL(ENT(pev), "models/CKF_III/w_teleporter.mdl");
	pev->modelindex = MODEL_INDEX("models/CKF_III/w_teleporter.mdl");
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,12));
	pev->mins = Vector(-16,-16,0);
	pev->maxs = Vector(16,16,12);

	//Anim //No build anim now
	pev->sequence = 0;
	pev->framerate = 1;
	pev->frame = 0;
	ResetSequenceInfo();

	UTIL_SetOrigin(pev, pev->origin);
	DROP_TO_FLOOR(ENT(pev));

	pev->max_health = 1;
	pev->health = 1;

	pev->colormap = (m_iTeam == TEAM_RED) ? (1 | (1<<8)) : (140 | (140<<8));

	m_iLevel = 1;
	m_flProgress = 0;
	m_iUpgrade = 0;
	m_iFlags |= BUILD_BUILDING;
	m_flChargeRate = 10;

	if(m_bIsEntrance)
	{
		m_bChargeReady = false;
		m_flCharge = 0;		
		m_pTelePlayer = NULL;
		pev->frags = 0;
	}

	SetThink(&CBuildTeleporter::BuildThink);
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/build_deploy.wav", VOL_NORM, ATTN_NORM);

	UpdateHUD();
}

BOOL CBuildTeleporter::Repair(CBasePlayer *pPlayer)
{
	BOOL bShouldReturn = FALSE;

	bShouldReturn += CBaseBuildable::Repair(pPlayer);//Fix me first

	if(m_pLinkTele)
	{
		if(!(m_pLinkTele->m_iFlags & BUILD_BUILDING))
			bShouldReturn += m_pLinkTele->CBaseBuildable::Repair(pPlayer);
	}
	
	if(bShouldReturn)
		return TRUE;

	BOOL bUpdate = FALSE;
	BOOL bSuccess = FALSE;
	if(!(m_iFlags & BUILD_BUILDING))
	{
		if(m_iUpgrade < 200 && m_iLevel < 3)
		{
			int iUpgrade = min(200 - m_iUpgrade, 25);
			int iMetalNeeded = iUpgrade;
			if(pPlayer->m_iMetal && iMetalNeeded)
			{
				if(iMetalNeeded > pPlayer->m_iMetal)
				{
					iUpgrade = iUpgrade * pPlayer->m_iMetal / iMetalNeeded;
					iMetalNeeded = pPlayer->m_iMetal;
				}
				m_iUpgrade = min(m_iUpgrade + iUpgrade, 200);
				pPlayer->AddPlayerMetal(-iMetalNeeded);
				if(m_iUpgrade >= 200)
				{
					UpgradeMe();
				}
				bSuccess = TRUE;
				bUpdate = TRUE;
			}
		}
	}
	SyncTeleporter();
	if(bUpdate)
		UpdateHUD();
	if(bSuccess)
		return TRUE;
	return FALSE;
}

void CBuildTeleporter::UpgradeMe(void)
{
	if(m_iLevel == 3)
		return;

	m_iLevel ++;

	m_flProgress = 0;
	m_iUpgrade = 0;

	int iMaxHealth = pev->max_health;

	m_flChargeRate = (m_iLevel == 2) ? 20 : 33;

	pev->max_health = (m_iLevel == 2) ? 180 : 216;
	int iOldHealth = pev->health;
	pev->health = pev->health * pev->max_health / iMaxHealth;

	UpdateHUD();

	if(m_pLinkTele)
	{
		m_pLinkTele->pev->max_health = pev->max_health;
		m_pLinkTele->pev->health += (pev->health - iOldHealth);
		m_pLinkTele->m_flChargeRate = m_flChargeRate;

		SyncTeleporter();
		m_pLinkTele->UpdateHUD();
	}
}

void CBuildTeleporter::BuildThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	UpgradeProgress(0.5);

	if(m_flProgress >= 100)
	{
		BuildFinish();
		return;
	}
}

void CBuildTeleporter::BuildFinish(void)
{
	if(!(m_iFlags & BUILD_BUILDING))
		return;

	m_flProgress = 0;
	if(!m_pLinkTele)
	{
		m_iUpgrade = 0;
	}
	else
	{
		m_iUpgrade = m_pLinkTele->m_iUpgrade;
	}

	if(m_bIsEntrance)
	{
		m_flCharge = 100;
		if(m_pLinkTele)
		{
			m_flChargeRate = m_pLinkTele->m_flChargeRate;
		}
		SetThink(&CBuildTeleporter::TeleportThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetThink(NULL);
	}

	m_iFlags &= ~BUILD_BUILDING;
	pev->max_health = 150;

	UpdateHUD();
}

void CBuildTeleporter::UpgradeProgress(float flProgressIncrease)
{
	if(m_flProgress + flProgressIncrease > 100)
		flProgressIncrease = 100 - m_flProgress;

	float flNextMaxHealth = 0;
	float flCurMaxHealth = 0;

	if(m_iFlags & BUILD_BUILDING)
	{
		flNextMaxHealth = 150;
		flCurMaxHealth = 1;
	}
	else if(m_iLevel == 2)
	{
		flNextMaxHealth = 180;
		flCurMaxHealth = 150;
	}
	else if(m_iLevel == 3)
	{
		flNextMaxHealth = 216;
		flCurMaxHealth = 180;
	}
	pev->health = min(pev->health + flProgressIncrease * (flNextMaxHealth-flCurMaxHealth)/100.0, flNextMaxHealth);

	if(pev->health > pev->max_health)
		pev->max_health = pev->health;

	m_flProgress += flProgressIncrease;
}

void CBuildTeleporter::UpdateHUD(void)
{
	if(!m_pPlayer)
		return;

	int iLevel = GetBuildLevel();
	int iUpgrade = GetBuildUpgrade();

	BOOL bAlive = (m_iLevel > 0) ? TRUE : FALSE; 

	if(!bAlive) iLevel = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgHUDBuild, NULL, m_pPlayer->pev);
	WRITE_BYTE(GetBuildClass());
	WRITE_BYTE(iLevel);
	if(iLevel != 0)
	{
		WRITE_BYTE(m_iFlags);//maxium (1<<7)
		WRITE_BYTE(pev->health);
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE(150);
		else
			WRITE_BYTE(pev->max_health);
		if(m_iFlags & BUILD_BUILDING)
			WRITE_BYTE((int)(m_flProgress * 255.0 / 100));
		if(m_bIsEntrance)
		{
			WRITE_COORD(m_flCharge);
			WRITE_COORD(GetChargeRate());
			WRITE_BYTE(pev->frags);
			if(m_iLevel < 3)
				WRITE_BYTE(iUpgrade);
		}
		else
		{
			if(m_iLevel < 3)
				WRITE_BYTE(iUpgrade);
		}
		MESSAGE_END();
	}
	else
		MESSAGE_END();
}

void CBuildTeleporter::TeleportThink(void)
{
	if(!m_pPlayer)
		return;

	if(!m_bIsEntrance)
		return;

	pev->nextthink = gpGlobals->time + 0.1;

	BOOL bCanRun = ShouldRun();

	if(bCanRun)
	{
		pev->sequence = 1;
		pev->framerate = m_flCharge / 200.0;

		if(m_flCharge >= 100)
		{
			if(m_flNextCheck < gpGlobals->time)
			{
				ScanPlayer();
				m_flNextCheck = gpGlobals->time + 0.5;
			}
			if(m_pTelePlayer && m_flTeleport < gpGlobals->time)//Baby go come!
			{
				TelePlayer();
			}
		}
		m_flCharge = min(m_flCharge + m_flChargeRate * 0.1, 100);

		if(m_flNextSpinSnd < gpGlobals->time)
		{
			m_flNextSpinSnd = gpGlobals->time + 4.0f;
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "CKF_III/teleporter_spin.wav", VOL_NORM, ATTN_NORM);
		}

		if(!m_bChargeReady && m_flCharge >= 100)
		{
			EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/teleporter_ready.wav", VOL_NORM, ATTN_NORM);
			m_bChargeReady = true;
		}
	}
	else
	{
		pev->sequence = 0;
	}

	SyncTeleporter();
}

void CBuildTeleporter::SyncTeleporter(void)
{
	if(!m_pLinkTele)
		return;

	m_pLinkTele->m_iLevel = m_iLevel;
	m_pLinkTele->m_iUpgrade = m_iUpgrade;

	if(m_bIsEntrance)
	{
		if(!(m_pLinkTele->m_iFlags & BUILD_BUILDING))
		{
			m_pLinkTele->pev->sequence = pev->sequence;
			m_pLinkTele->pev->framerate = pev->framerate;
		}
	}
}

void CBuildTeleporter::TelePlayer(void)
{
	if(!m_pLinkTele)
		return;

	if(!m_pTelePlayer)
		return;

	if(!CheckPlayer(m_pTelePlayer))
		return;

	Vector vecSrc = m_pLinkTele->Center();
	vecSrc.z += 52;

	UTIL_SetOrigin(m_pTelePlayer->pev, vecSrc);

	m_pTelePlayer->pev->angles.y = m_pLinkTele->pev->angles.y;
	m_pTelePlayer->pev->fixangle = 1;

	FixPlayerCrouchStuck(m_pTelePlayer->edict());

	//Kill Stucker
	CBaseEntity *pEntity = NULL;
	while((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 32)) != NULL)
	{
		if (!pEntity->IsPlayer() || pEntity->pev == m_pTelePlayer->pev)
			continue;
		if(!pEntity->IsAlive())
			continue;
		CBasePlayer *pVictim = (CBasePlayer *)pEntity;
		if(pVictim->m_iTeam != m_pTelePlayer->m_iTeam)
		{
			pVictim->TakeDamage(pev, m_pTelePlayer->pev, 9999, DMG_GENERIC | DMG_NEVERGIB, 0);
		}
	}

	UTIL_ScreenFade(m_pTelePlayer, Vector(255, 255, 255), 0.5, 0.5, 255, FFADE_IN);

	m_pTelePlayer = NULL;

	m_flCharge = 0;
	m_bChargeReady = false;

	//Add frags

	m_pPlayer->AddPoints(0.5, TRUE);
	m_pPlayer->m_Stats.iTeleport ++;
	m_pPlayer->SendStatsInfo(STATS_TELEPORT);
	pev->frags ++;

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/teleporter_send.wav", VOL_NORM, ATTN_NORM);

	EMIT_SOUND(ENT(m_pLinkTele->pev), CHAN_STATIC, "CKF_III/teleporter_receive.wav", VOL_NORM, ATTN_NORM);

	UpdateHUD();
}

bool CBuildTeleporter::CheckPlayer(CBasePlayer *pPlayer)
{
	if(!pPlayer->IsAlive())
		return false;

	if(pPlayer->m_iCloak != CLOAK_NO)
		return false;

	if(pPlayer->m_iTeam != m_iTeam && !(pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam))
		return false;

	//if(pPlayer->pev->groundentity != edict())
	//	return false;

	if(!(pPlayer->pev->flags & FL_ONGROUND))
		return false;

	if(pPlayer->pev->bInDuck)
		return false;

	return true;
}

void CBuildTeleporter::ScanPlayer(void)
{
	if(m_pTelePlayer)
	{
		if(CheckPlayer(m_pTelePlayer))
			return;
	}
	CBaseEntity *pEntity = NULL;
	CBasePlayer *pPlayer = NULL;

	Vector vecSrc = Center();
	
	while((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 18)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;

		pPlayer = (CBasePlayer *)pEntity;

		if(!CheckPlayer(pPlayer))
			continue;

		m_pTelePlayer = pPlayer;
		m_flTeleport = gpGlobals->time + 0.6;
		break;
	}
}

void CBuildTeleporter::Killed(entvars_t *pevAttacker, int iGib)
{
	if(!m_pPlayer)
		return;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "CKF_III/null.wav", VOL_NORM, ATTN_NORM);//Check before Play

	if(m_pLinkTele)
	{
		if(iGib != -1)//Kill Link, drop to lv1
		{
			m_pLinkTele->m_pLinkTele = NULL;
			m_pLinkTele->m_iLevel = 1;
			m_pLinkTele->m_iUpgrade = 0;
			m_pLinkTele->m_flCharge = 0;
			m_pLinkTele->m_flChargeRate = 10;
			m_pLinkTele->pev->sequence = 0;//Stop animating
			m_pLinkTele->UpdateHUD();
		}
	}

	return CBaseBuildable::Killed(pevAttacker, iGib);
}

bool CBuildTeleporter::ShouldRun(void)
{
	if(!m_pLinkTele)
		return false;
	if(m_iFlags & BUILD_BUILDING)
		return false;
	if(m_pLinkTele->m_iFlags & BUILD_BUILDING)
		return false;
	if(m_pSapper)
		return false;
	if(m_pLinkTele->m_pSapper)
		return false;
	return true;
}

int CBuildTeleporter::GetBuildLevel(void)
{
	return m_iLevel;
}

int CBuildTeleporter::GetBuildUpgrade(void)
{
	return m_iUpgrade;
}

int CBuildTeleporter::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (bitsDamageType & DMG_SAPPER)
	{
		if(m_pLinkTele)
		{
			m_pLinkTele->CBaseBuildable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, iCrit);
		}
	}
	return CBaseBuildable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, iCrit);
}