#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "spectator.h"

void CBaseSpectator::SpectatorConnect(void)
{
	pev->flags = FL_SPECTATOR;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	m_pGoalEnt = NULL;
}

void CBaseSpectator::SpectatorDisconnect(void)
{
}

void CBaseSpectator::SpectatorImpulseCommand(void)
{
	switch (pev->impulse)
	{
		case 1:
		{
			static edict_t *pGoal = NULL;
			edict_t *pPreviousGoal = pGoal;
			edict_t *pCurrentGoal = pGoal;
			BOOL bFound = FALSE;

			while (1)
			{
				pCurrentGoal = FIND_ENTITY_BY_CLASSNAME(pCurrentGoal, "info_player_deathmatch");

				if (pCurrentGoal == pPreviousGoal)
				{
					ALERT(at_console, "Could not find a spawn spot.\n");
					break;
				}

				if (!FNullEnt(pCurrentGoal))
				{
					bFound = TRUE;
					break;
				}
			}

			if (!bFound)
				break;

			pGoal = pCurrentGoal;
			UTIL_SetOrigin(pev, pGoal->v.origin);
			pev->angles = pGoal->v.angles;
			pev->fixangle = FALSE;
			break;
		}

		default: ALERT(at_console, "Unknown spectator impulse\n"); break;
	}

	pev->impulse = 0;
}

void CBaseSpectator::SpectatorThink(void)
{
	if (!(pev->flags & FL_SPECTATOR))
		pev->flags = FL_SPECTATOR;

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	if (pev->impulse)
		SpectatorImpulseCommand();
}

void CBaseSpectator::Spawn(void)
{
	pev->flags = FL_SPECTATOR;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	m_pGoalEnt = NULL;
}