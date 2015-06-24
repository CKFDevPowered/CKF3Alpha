#include "cl_dll.h"
#include <usercmd.h>
#include <entity_state.h>
#include "player.h"
#include "cdll_dll.h"
#include <cvardef.h>
#include "com_weapons.h"
#include "links.h"

void HUD_WeaponsPostThink(local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed)
{
	g_pv_angles->x = v_angles.x;
	g_pv_angles->y = v_angles.y;
	g_pv_angles->z = v_angles.z;

	g_iCurrentWeapon = to->client.m_iId;
	g_iPlayerFlags = from->client.flags;
	g_flPlayerSpeed = from->client.velocity.Length();
	g_iWeaponFlags = (int)from->client.vuser4[0];
	g_iWaterLevel = from->client.waterlevel;
	g_iFreezeTimeOver = from->client.iuser3 & PLAYERSTATE_FREEZETIMEOVER;
	g_finalstate = to;
}