#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "util.h"

extern vec3_t g_vecZero;
extern float *ev_punchangle;

extern int g_rseq;
extern int g_gaitseq;
extern vec3_t g_clorg;
extern vec3_t g_clang;

int	g_runfuncs = 0;
local_state_t *g_finalstate = NULL;
int g_lasthealth = 0;

int g_WeaponSelect = 0;

CClientPlayer g_Player;

CClientScattergun g_Scattergun;
CClientPistol g_Pistol;
CClientBat g_Bat;

CClientRocketLauncher g_RocketLauncher;
CClientShotgun g_Shotgun;
CClientShovel g_Shovel;

CClientMinigun g_Minigun;
CClientFist g_Fist;

CClientFlamethrower g_Flamethrower;
CClientFireaxe g_Fireaxe;

CClientGrenadeLauncher g_GrenadeLauncher;
CClientStickyLauncher g_StickyLauncher;
CClientBottle g_Bottle;

CClientSniperifle g_Sniperifle;
CClientSMG g_SMG;
CClientKukri g_Kukri;

CClientSyringegun g_Syringegun;
CClientMedigun g_Medigun;
CClientBonesaw g_Bonesaw;

CClientWrench g_Wrench;
CClientBuildPDA g_BuildPDA;
CClientDestroyPDA g_DestroyPDA;

CClientRevolver g_Revolver;
CClientSapper g_Sapper;
CClientButterfly g_Butterfly;
CClientDisguiseKit g_DisguiseKit;

CClientWeapon *g_pClientWeapon[MAX_WEAPON_SLOTS];
CClientWeapon *g_pWeapons[MAX_WEAPONS];

int HUD_SwitchWeapon(int slot)
{
	if(!g_iMenu && slot >= 0 && slot < MAX_WEAPON_SLOTS)
	{
		if(g_pClientWeapon[slot] && g_pClientWeapon[slot]->CanDeploy())
		{
			if(g_Player.m_pActiveItem && !g_Player.m_pActiveItem->CanHolster())
				return 0;
			char cmd[32];
			sprintf(cmd, "switch %d\n", slot+1);
			gEngfuncs.pfnClientCmd(cmd);
			g_WeaponSelect = slot+1;
		}
	}
	return 0;
}

void HUD_InitWeapons(void)
{
	memset(g_pWeapons, 0, sizeof(g_pWeapons));
	memset(g_pClientWeapon, 0, sizeof(g_pClientWeapon));

	REGISTER_WEAPON(Scattergun, WEAPON_SCATTERGUN);
	REGISTER_WEAPON(Pistol, WEAPON_PISTOL);
	REGISTER_WEAPON(Bat, WEAPON_BAT);

	REGISTER_WEAPON(RocketLauncher, WEAPON_ROCKETLAUNCHER);
	REGISTER_WEAPON(Shotgun, WEAPON_SHOTGUN);
	REGISTER_WEAPON(Shovel, WEAPON_SHOVEL);

	REGISTER_WEAPON(Minigun, WEAPON_MINIGUN);
	REGISTER_WEAPON(Fist, WEAPON_FIST);

	REGISTER_WEAPON(Flamethrower, WEAPON_FLAMETHROWER);
	REGISTER_WEAPON(Fireaxe, WEAPON_FIREAXE);

	REGISTER_WEAPON(GrenadeLauncher, WEAPON_GRENADELAUNCHER);
	REGISTER_WEAPON(StickyLauncher, WEAPON_STICKYLAUNCHER);
	REGISTER_WEAPON(Bottle, WEAPON_BOTTLE);

	REGISTER_WEAPON(Sniperifle, WEAPON_SNIPERIFLE);
	REGISTER_WEAPON(SMG, WEAPON_SMG);
	REGISTER_WEAPON(Kukri, WEAPON_KUKRI);

	REGISTER_WEAPON(Syringegun, WEAPON_SYRINGEGUN);
	REGISTER_WEAPON(Medigun, WEAPON_MEDIGUN);
	REGISTER_WEAPON(Bonesaw, WEAPON_BONESAW);

	REGISTER_WEAPON(Wrench, WEAPON_WRENCH);
	REGISTER_WEAPON(BuildPDA, WEAPON_BUILDPDA);
	REGISTER_WEAPON(DestroyPDA, WEAPON_DESTROYPDA);

	REGISTER_WEAPON(Revolver, WEAPON_REVOLVER);
	REGISTER_WEAPON(Sapper, WEAPON_SAPPER);
	REGISTER_WEAPON(Butterfly, WEAPON_BUTTERFLY);
	REGISTER_WEAPON(DisguiseKit, WEAPON_DISGUISEKIT);

	g_Player.m_pLastItem = NULL;
	g_Player.m_pActiveItem = NULL;
	g_Player.m_iHealth = 0;
	g_Player.m_iClass = 0;
	g_Player.m_szAnimExtention[0] = 0;

	//default
	g_Player.m_bAutoReload = true;

	const char *cl_autoreload = gEngfuncs.LocalPlayerInfo_ValueForKey("_cl_autoreload");
	if (cl_autoreload && strlen(cl_autoreload) && atoi(cl_autoreload) == 0)
		g_Player.m_bAutoReload = false;

	g_Player.m_bHitDamage = true;

	const char *cl_hitdamage = gEngfuncs.LocalPlayerInfo_ValueForKey("_cl_hitdamage");
	if (cl_hitdamage && strlen(cl_hitdamage) && atoi(cl_hitdamage) == 0)
		g_Player.m_bHitDamage = false;
}

float UTIL_WeaponTimeBase(void)
{
	return 0;
}

void HUD_PlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 )
{
	if ( !g_runfuncs || !g_finalstate )
	     return;

	vec3_t new_origin, new_angles, view_ofs;
	if(CL_IsThirdPerson())
	{
		VectorCopy(g_finalstate->playerstate.origin, new_origin);
		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		VectorAdd(new_origin, view_ofs, new_origin);

		VectorCopy(g_finalstate->playerstate.angles, new_angles);
		new_angles[0] = -new_angles[0];
	}
	else
	{
		VectorCopy(g_finalstate->playerstate.origin, new_origin);
		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		VectorAdd(new_origin, view_ofs, new_origin);

		VectorCopy(refparams.viewangles, new_angles);
		VectorSubtract(new_angles, ev_punchangle, new_angles);//ignore client side punchangle
	}
	gEngfuncs.pfnPlaybackEvent( flags, pInvoker, eventindex, delay, (float *)&new_origin, (float *)&new_angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2 );
}

void HUD_SendWeaponAnim(int iAnim)
{
	// Don't actually change it.
	if ( !g_runfuncs )
		return;

	g_Player.pev.weaponanim = iAnim;

	HudWeaponAnim(iAnim);
}

void HUD_PlaySound( char *sound, float volume )
{
	if ( !g_runfuncs || !g_finalstate )
		return;

	gEngfuncs.pfnPlaySoundByNameAtLocation( sound, volume, (float *)&g_finalstate->playerstate.origin );
}

void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int i;
	CClientWeapon *pCurWeapon;

	pCurWeapon = NULL;

	if(from->client.m_iId)
	{
		pCurWeapon = g_pWeapons[from->client.m_iId];
	}
	g_iWeaponID = from->client.m_iId;

	g_finalstate = to;

	if ( g_runfuncs )
	{
		if(g_iHideHUD & HIDEHUD_ALL)
		{
			g_Player.m_iHealth = 0;
		}
		if(g_iClass != g_Player.m_iClass)
		{
			g_Player.m_iHealth = 0;
			g_Player.m_iClass = g_iClass;
		}
		if ( to->client.health <= 0 && g_Player.m_iHealth > 0 )
		{
			g_Player.Killed();
		}
		else if ( to->client.health > 0 && g_Player.m_iHealth <= 0 )
		{
			if(g_Player.m_pActiveItem)
			{
				g_Player.m_pActiveItem->Holster();
			}
			g_Player.m_pActiveItem = pCurWeapon;
			g_Player.Spawn();
		}
		g_Player.m_iHealth = to->client.health;
	}

	//server-side weapon switch
	if(!cmd->weaponselect && g_Player.m_pLastItem && g_Player.m_pLastItem != pCurWeapon)
	{
		g_Player.m_pLastItem->Holster();
		if(pCurWeapon)
		{
			int runfuncs = g_runfuncs;
			g_runfuncs = 1;
			pCurWeapon->Deploy();
			g_runfuncs = runfuncs;
		}
	}

	g_Player.m_pLastItem = g_Player.m_pActiveItem;
	g_Player.m_pActiveItem = pCurWeapon;

	//Even if we don't have a weapon, we still receive these data

	g_Player.random_seed = random_seed;
	g_Player.m_afButtonLast = from->playerstate.oldbuttons;
	int buttonsChanged = (g_Player.m_afButtonLast ^ cmd->buttons);
	g_Player.m_afButtonPressed =  buttonsChanged & cmd->buttons;	
	g_Player.m_afButtonReleased = buttonsChanged & (~cmd->buttons);
	g_Player.pev.button = cmd->buttons;
	g_Player.pev.oldbuttons = from->playerstate.oldbuttons;
	VectorCopy(from->client.velocity, g_Player.pev.velocity);
	g_Player.pev.flags = from->client.flags;
	g_Player.pev.deadflag = from->client.deadflag;
	g_Player.pev.waterlevel = from->client.waterlevel;
	g_Player.pev.maxspeed = from->client.maxspeed;
	g_Player.pev.fov = from->client.fov;
	g_Player.pev.weaponanim = from->client.weaponanim;
	g_Player.pev.viewmodel = from->client.viewmodel;
	g_Player.pev.bInDuck = from->client.bInDuck;
	g_Player.pev.fuser2 = from->client.fuser2;//for jump check
	g_Player.pev.iuser3 = from->client.iuser3;//for cdflag check
	g_Player.m_flNextAttack = from->client.m_flNextAttack;

	g_Player.m_bCritBuff = (from->client.ammo_cells & 1);
	g_Player.m_iCritBoost = (from->client.ammo_cells >> 1) & 1;
	g_Player.m_iDmgDone_Recent = from->client.ammo_nails;
	g_Player.m_fCritChance = from->client.fuser1;
	g_Player.m_iHealer = from->client.ammo_rockets;

	if(g_iClass == CLASS_SPY)
	{
		g_Player.m_iDisguise = ((int)from->client.vuser2[0]) & 1;
		g_Player.m_iCloak = ((int)from->client.vuser2[0]) >> 1;
		g_Player.m_flCloakEnergy = from->client.vuser3[1];
		if(g_Player.m_iDisguise)
		{
			g_Player.m_iDisguiseTeam = ((int)from->client.vuser2[1]) & 3;
			g_Player.m_iDisguiseClass = ( ((int)from->client.vuser2[1]) >> 2) & 15;
			g_Player.m_iDisguiseHealth = from->client.vuser3[0];
			g_Player.m_iDisguiseWeaponBody = from->client.vuser3[2];
			g_Player.m_iDisguiseWeapon = from->client.vuser4[0];
			g_Player.m_iDisguiseSequence = from->client.vuser2[2];
		}
		else
		{
			g_Player.m_iDisguiseTeam = 0;
			g_Player.m_iDisguiseClass = 0;
			g_Player.m_iDisguiseHealth = 0;
			g_Player.m_iDisguiseWeapon = 0;
			g_Player.m_iDisguiseWeaponBody = 0;
		}
	}
	else if(g_iClass == CLASS_MEDIC)
	{
		g_Player.m_iUbercharge = (int)(from->client.vuser2[0]);
		g_Player.m_fUbercharge = from->client.vuser3[1];
	}
	else if(g_iClass == CLASS_ENGINEER)
	{
		g_Player.m_iBluePrintYaw = (int)(from->client.vuser2[1]);
		g_Player.m_iCarryBluePrint = (int)(from->client.vuser2[0]);
		g_Player.m_iMetal = (int)from->client.vuser3[0];
	}

	if( !pCurWeapon )
		return;

	//store weapon states
	for (i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		weapon_data_t *pweapon = &from->weapondata[ i ];

		if(!pweapon->m_iId)
		{
			g_pClientWeapon[i] = NULL;
			continue;
		}

		g_pClientWeapon[i] = g_pWeapons[pweapon->m_iId];

		if(!g_pClientWeapon[i])
			continue;

		g_pClientWeapon[i]->m_iId = pweapon->m_iId;

		g_pClientWeapon[i]->m_iClip = pweapon->m_iClip;
		g_pClientWeapon[i]->m_iMaxClip = pweapon->iuser2;
		g_pClientWeapon[i]->m_iAmmo = pweapon->iuser3;		
		g_pClientWeapon[i]->m_iMaxAmmo = pweapon->iuser4;

		g_pClientWeapon[i]->m_fInReload = pweapon->m_fInReload;
		g_pClientWeapon[i]->m_fInSpecialReload = pweapon->m_fInSpecialReload;
		g_pClientWeapon[i]->m_iShotsFired = pweapon->m_fInZoom;		
		g_pClientWeapon[i]->m_iWeaponState = pweapon->m_iWeaponState;
		g_pClientWeapon[i]->m_bMeleeAttack = 0;
		g_pClientWeapon[i]->m_iMeleeCrit = 0;

		g_pClientWeapon[i]->m_flTimeWeaponIdle = pweapon->m_flTimeWeaponIdle;
		g_pClientWeapon[i]->m_flNextPrimaryAttack = pweapon->m_flNextPrimaryAttack;
		g_pClientWeapon[i]->m_flNextSecondaryAttack = pweapon->m_flNextSecondaryAttack;
		g_pClientWeapon[i]->m_flNextReload = pweapon->m_flNextReload;
		g_pClientWeapon[i]->m_flDecreaseShotsFired = pweapon->m_fAimedDamage;
		g_pClientWeapon[i]->m_flMeleeAttack = 0;

		if(i+1 == WEAPON_SLOT_MELEE)
		{
			if(g_pClientWeapon[i]->m_iId == WEAPON_BOTTLE)
			{
				g_Bottle.m_bBroken = (pweapon->iuser1 & 1) ? TRUE : FALSE;
				g_pClientWeapon[i]->m_bMeleeAttack = (pweapon->iuser1 & 2) ? TRUE : FALSE;
				g_pClientWeapon[i]->m_iMeleeCrit = (pweapon->iuser1 >> 2);
			}
			else
			{
				g_pClientWeapon[i]->m_bMeleeAttack = (pweapon->iuser1 & 1) ? TRUE : FALSE;
				g_pClientWeapon[i]->m_iMeleeCrit = (pweapon->iuser1 >> 1);
			}
			g_pClientWeapon[i]->m_flMeleeAttack = pweapon->m_fNextAimBonus;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_MINIGUN)
		{
			g_Minigun.m_fSpin = pweapon->m_fNextAimBonus;
			g_Minigun.m_iSpin = pweapon->iuser1;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_STICKYLAUNCHER)
		{
			g_StickyLauncher.m_fChargeTimer = pweapon->m_fNextAimBonus;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_SNIPERIFLE)
		{
			g_Sniperifle.m_fChargeTimer = pweapon->m_fNextAimBonus;
			g_Sniperifle.m_iResumeZoom = (pweapon->iuser1 >> 1);
			g_Sniperifle.m_fSpotActive = (pweapon->iuser1 & 1);
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_FLAMETHROWER)
		{
			g_Flamethrower.m_flState = pweapon->m_fNextAimBonus;
			g_Flamethrower.m_iState = (pweapon->iuser1 >> 1);
			g_Flamethrower.m_iAmmoConsumption = (pweapon->iuser1 & 1);
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_MEDIGUN)
		{
			g_Medigun.m_bDelayedFire = pweapon->iuser1;
		}
	}

	if(g_pClientWeapon[WEAPON_SLOT_SECONDARY-1])
	{
		if(g_pClientWeapon[WEAPON_SLOT_SECONDARY-1]->m_iId == WEAPON_STICKYLAUNCHER)
		{
			g_StickyLauncher.m_iStickyNum = from->client.ammo_shells;
		}
		if(g_pClientWeapon[WEAPON_SLOT_SECONDARY-1]->m_iId == WEAPON_MEDIGUN)
		{
			g_Medigun.m_iHealTarget = from->client.ammo_shells;
		}
	}

	if(g_iClass == CLASS_ENGINEER)
	{
		to->client.vuser2[0] = (float)g_Player.m_iCarryBluePrint;
		to->client.vuser2[1] = (float)g_Player.m_iBluePrintYaw;
	}

	// Don't go firing anything if we have died.
	// Or if we don't have a weapon model deployed
	if ( ( g_Player.pev.deadflag != ( DEAD_DISCARDBODY + 1 ) ) && CL_IsAlive() && g_Player.pev.viewmodel && !g_iUser1 )
	{
		if ( g_Player.m_flNextAttack <= 0 )
		{
			g_Player.m_pActiveItem->ItemPostFrame();
		}
	}

	to->client.m_iId = from->client.m_iId;

	//client-side weapon switch
	do
	{
		if ( cmd->weaponselect && ( g_Player.pev.deadflag != ( DEAD_DISCARDBODY + 1 ) ) )
		{
			// Switched to a different weapon?
			CClientWeapon *pNew = g_pClientWeapon[ cmd->weaponselect-1 ];
			if ( pNew && pNew != g_Player.m_pActiveItem )
			{
				if (g_Player.m_pActiveItem)
				{
					if(!g_Player.m_pActiveItem->CanHolster())
						break;
					g_Player.m_pActiveItem->Holster();
				}

				g_Player.m_pLastItem = g_Player.m_pActiveItem;
				g_Player.m_pActiveItem = pNew;

				// Deploy new weapon
				if (g_Player.m_pActiveItem)
				{
					if(!g_Player.m_pActiveItem->CanDeploy())
						break;
					g_Player.m_pActiveItem->Deploy( );
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = pNew->m_iId;
			}
		}
	}while(FALSE);

	// Copy in results of prediction code
	to->client.viewmodel				= g_Player.pev.viewmodel;
	to->client.fov						= g_Player.pev.fov;
	to->client.weaponanim				= g_Player.pev.weaponanim;
	to->client.m_flNextAttack			= g_Player.m_flNextAttack;
	to->client.maxspeed					= g_Player.pev.maxspeed;
	to->client.flags					= g_Player.pev.flags;

	if(g_pClientWeapon[WEAPON_SLOT_SECONDARY-1])
	{
		if(g_pClientWeapon[WEAPON_SLOT_SECONDARY-1]->m_iId == WEAPON_STICKYLAUNCHER)
		{
			to->client.ammo_shells = g_StickyLauncher.m_iStickyNum;
		}
	}

	//if ( g_runfuncs && ( (*cls_viewmodel_sequence) != to->client.weaponanim ) )
	//{
	//	HudWeaponAnim(to->client.weaponanim);
	//}

	//copy weapon data back
	for ( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		weapon_data_t *pto = &to->weapondata[ i ];
	
		if(!g_pClientWeapon[i])
		{
			memset( pto, 0, sizeof( weapon_data_t ) );
			continue;
		}
		
		pto->m_iId = g_pClientWeapon[i]->m_iId;

		pto->m_iClip = g_pClientWeapon[i]->m_iClip;
		pto->iuser3 = g_pClientWeapon[i]->m_iAmmo;

		pto->m_fInReload = g_pClientWeapon[i]->m_fInReload;
		pto->m_fInSpecialReload = g_pClientWeapon[i]->m_fInSpecialReload;
		pto->m_fInZoom = g_pClientWeapon[i]->m_iShotsFired;
		pto->m_iWeaponState = g_pClientWeapon[i]->m_iWeaponState;

		pto->m_flTimeWeaponIdle = g_pClientWeapon[i]->m_flTimeWeaponIdle;
		pto->m_flNextPrimaryAttack = g_pClientWeapon[i]->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = g_pClientWeapon[i]->m_flNextSecondaryAttack;
		pto->m_flNextReload = g_pClientWeapon[i]->m_flNextReload;
		pto->m_fAimedDamage = g_pClientWeapon[i]->m_flDecreaseShotsFired;
		if(i+1 == WEAPON_SLOT_MELEE)
		{
			pto->m_fNextAimBonus = g_pClientWeapon[i]->m_flMeleeAttack;
			if(g_pClientWeapon[i]->m_iId == WEAPON_BOTTLE)
			{
				pto->iuser1 = (g_Bottle.m_bBroken & 1) | (g_pClientWeapon[i]->m_bMeleeAttack & 2) | (g_pClientWeapon[i]->m_iMeleeCrit << 2);
			}
			else
			{
				pto->iuser1 = (g_pClientWeapon[i]->m_bMeleeAttack & 1) | (g_pClientWeapon[i]->m_iMeleeCrit << 1);
			}
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_MINIGUN)
		{
			pto->m_fNextAimBonus = g_Minigun.m_fSpin;
			pto->iuser1 = g_Minigun.m_iSpin;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_STICKYLAUNCHER)
		{
			pto->m_fNextAimBonus = g_StickyLauncher.m_fChargeTimer;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_SNIPERIFLE)
		{
			pto->m_fNextAimBonus = g_Sniperifle.m_fChargeTimer;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_FLAMETHROWER)
		{
			pto->m_fNextAimBonus = g_Flamethrower.m_flState;
			pto->iuser1 = (g_Flamethrower.m_iAmmoConsumption & 1) | (g_Flamethrower.m_iState << 1);
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_MEDIGUN)
		{
			pto->iuser1 = g_Medigun.m_bDelayedFire;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_BUILDPDA)
		{
			pto->iuser1 = g_BuildPDA.m_bDelayedFire;
		}
		else if(g_pClientWeapon[i]->m_iId == WEAPON_DESTROYPDA)
		{
			pto->iuser1 = g_DestroyPDA.m_bDelayedFire;
		}

		pto->m_flNextReload				-= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack		-= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack	-= cmd->msec / 1000.0;
		pto->m_flNextReload				-= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle			-= cmd->msec / 1000.0;
		pto->m_fAimedDamage				-= cmd->msec / 1000.0;
		pto->m_fNextAimBonus			-= cmd->msec / 1000.0;

		if ( pto->m_flTimeWeaponIdle < -0.001 )
			pto->m_flTimeWeaponIdle = -0.001;
		if ( pto->m_flNextPrimaryAttack < -1.000 )
			pto->m_flNextPrimaryAttack = -1.000;
		if ( pto->m_flNextSecondaryAttack < -0.001 )
			pto->m_flNextSecondaryAttack = -0.001;
		if ( pto->m_flNextReload < -0.001 )
			pto->m_flNextReload = -0.001;
		if ( pto->m_fAimedDamage < -0.001 )
			pto->m_fAimedDamage = -0.001;
		if ( pto->m_fNextAimBonus < -0.001 )
			pto->m_fNextAimBonus = -0.001;
	}
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if ( to->client.m_flNextAttack < -0.001 )
		to->client.m_flNextAttack = -0.001;

	g_finalstate = NULL;
}

void HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_runfuncs = runfuncs;

	HUD_WeaponsPostThink(from, to, cmd,  time, random_seed );

	if(runfuncs)
	{
		g_rseq = to->playerstate.sequence;
		g_gaitseq = to->playerstate.gaitsequence;
		VectorCopy(to->playerstate.origin, g_clorg);
		VectorCopy(to->playerstate.angles, g_clang);
	}
}

int LookupSequence(void *pmodel, const char *label)
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (!stricmp(pseqdesc[i].label, label))
			return i;
	}

	return 0;
}

BOOL CClientWeapon::GroupDeploy(char *szViewModel, char *szWeaponModel, int iViewAnim, int iViewBody, int iViewSkin, const char *szAnimExt)
{
	if (!CanDeploy())
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &g_Player.pev.viewmodel );
	gEngfuncs.CL_LoadModel( szWeaponModel, &g_Player.pev.weaponmodel );

	if ( g_runfuncs )
	{
		if(!iViewBody && !iViewBody)
			HudWeaponAnim(iViewAnim);
		else
			HudWeaponAnimEx(iViewAnim, iViewBody, iViewSkin, gEngfuncs.GetClientTime());
	}

	g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.75;
	g_Player.pev.weaponanim = iViewAnim;
	g_Player.pev.fov = g_iDefaultFOV;
	g_Player.ResetMaxSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	m_flDecreaseShotsFired = UTIL_WeaponTimeBase();
	m_bMeleeAttack = FALSE;

	strcpy(g_Player.m_szAnimExtention, szAnimExt);
	char szSequenceName[32];
	sprintf(szSequenceName, "ref_aim_%s", szAnimExt);
	model_t *mod = gEngfuncs.GetLocalPlayer()->model;
	if(mod)
	{
		studiohdr_t *hdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(mod);
		g_Player.pev.sequence = LookupSequence(hdr, szSequenceName);
	}
	else
	{
		g_Player.pev.sequence = 0;
	}
	return TRUE;
}

BOOL CClientWeapon::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, const char *szAnimExt)
{
	if (!CanDeploy())
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &g_Player.pev.viewmodel );
	gEngfuncs.CL_LoadModel( szWeaponModel, &g_Player.pev.weaponmodel );

	SendWeaponAnim(iAnim);

	g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.75;
	g_Player.pev.weaponanim = iAnim;
	g_Player.pev.fov = g_iDefaultFOV;
	g_Player.ResetMaxSpeed();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	m_flDecreaseShotsFired = UTIL_WeaponTimeBase();
	m_bMeleeAttack = FALSE;

	strcpy(g_Player.m_szAnimExtention, szAnimExt);

	char szSequenceName[32];
	sprintf(szSequenceName, "ref_aim_%s", szAnimExt);
	model_t *mod = gEngfuncs.GetLocalPlayer()->model;
	if(mod)
	{
		studiohdr_t *hdr = (studiohdr_t *)IEngineStudio.Mod_Extradata(mod);
		g_Player.pev.sequence = LookupSequence(hdr, szSequenceName);
	}
	else
	{
		g_Player.pev.sequence = 0;
	}
	return TRUE;
}

void CClientWeapon::SendWeaponAnim(int iAnim)
{
	HUD_SendWeaponAnim(iAnim);
}

void CClientWeapon::Reloaded(void)
{
	int j = min( m_iMaxClip - m_iClip, m_iAmmo);	

	m_iClip += j;
	m_iAmmo -= j;

	m_fInReload = false;
}

void CClientWeapon::ItemPostFrame(void)
{
	//ResetEmptySound();

	if ((m_fInReload || m_fInSpecialReload) && m_flNextReload <= UTIL_WeaponTimeBase())
	{
		Reloaded();
	}

	if(m_bMeleeAttack && m_flMeleeAttack <= UTIL_WeaponTimeBase())
	{
		m_bMeleeAttack = FALSE;
		Swing();		
	}
	else if ((g_Player.pev.button & IN_ATTACK2) && (m_flNextSecondaryAttack <= UTIL_WeaponTimeBase()))
	{
		if ( iMaxAmmo() && !m_iAmmo )
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		//g_Player.pev.button &= ~IN_ATTACK2;
	}
	else if ((g_Player.pev.button & IN_ATTACK) && (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() ) && g_Player.PlayerCanAttack())
	{
		if ( (!m_iClip && iMaxAmmo()) || (iMaxClip() == WEAPON_NOCLIP && !m_iAmmo ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		PrimaryAttack();
	}
	else if ( (g_Player.pev.button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && !m_fInSpecialReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if ( !(g_Player.pev.button & (IN_ATTACK|IN_ATTACK2) ) )
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;
		m_bDelayedFire = FALSE;

		// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( IsUseable() && m_iAmmo > 0 && (!m_iClip || (m_iClip < m_iMaxClip && g_Player.m_bAutoReload)) && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
		{
			Reload();
		}

		WeaponIdle();
	}
}

void CClientWeapon::ResetEmptySound(void)
{
	m_iPlayEmptySound = 1;
}

BOOL CClientWeapon::PlayEmptySound(void)
{
	if (m_iPlayEmptySound)
	{
		HUD_PlaySound( "weapons/dryfire_rifle.wav", 0.8 );
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

BOOL CClientWeapon::IsUseable(void)
{
	//// If I don't use ammo of any kind, I can always fire
	//if ( m_iMaxClip <= 0 && m_iMaxAmmo <= 0 )
	//	return TRUE;

	//// Otherwise, I need ammo of either type
	//return ( (m_iClip > 0) || (m_iMaxAmmo > 0 && m_iAmmo > 0) );
	if (m_iClip <= 0)
		if (m_iAmmo <= 0 && m_iMaxAmmo > 0)
			return FALSE;

	return TRUE;
}

static unsigned int glSeed = 0; 

unsigned int seed_table[ 256 ] =
{
	28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
	27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
	26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
	10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
	10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
	18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
	18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
	28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
	31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
	21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
	617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
	18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
	12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
	9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
	29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
	25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};

unsigned int U_Random( void ) 
{ 
	glSeed *= 69069; 
	glSeed += seed_table[ glSeed & 0xff ];
 
	return ( ++glSeed & 0x0fffffff ); 
} 

void U_Srand( unsigned int seed )
{
	glSeed = seed_table[ seed & 0xff ];
}

int UTIL_SharedRandomLong( unsigned int seed, int low, int high )
{
	unsigned int range;

	U_Srand( (int)seed + low + high );

	range = high - low + 1;
	if ( !(range - 1) )
	{
		return low;
	}
	else
	{
		int offset;
		int rnum;

		rnum = U_Random();

		offset = rnum % range;

		return (low + offset);
	}
}

float UTIL_SharedRandomFloat( unsigned int seed, float low, float high )
{
	unsigned int range;

	U_Srand( (int)seed + *(int *)&low + *(int *)&high );

	U_Random();
	U_Random();

	range = high - low;
	if ( !range )
	{
		return low;
	}
	else
	{
		int tensixrand;
		float offset;

		tensixrand = U_Random() & 65535;

		offset = (float)tensixrand / 65536.0;

		return (low + offset * range );
	}
}

int CClientPlayer::GetCriticalFire(int iType, unsigned int iRandSeed)
{
	int iCrit = 0;

	if(m_iCritBoost)
		iCrit += 2;

	float fRand = 0;
	switch(iType)
	{
		case 0:
		{
			fRand = UTIL_SharedRandomFloat(iRandSeed, 0.0, 100.0);
			if(fRand <= m_fCritChance) iCrit += 2;
			break;
		}
		case 1:
		{
			if(m_bCritBuff)
			{
				iCrit += 2;
			}
			break;
		}
		case 2:
		{
			float fCritChance = 15.0f + 50.0f*m_iDmgDone_Recent/800.0f;
			if(fCritChance > 65.0f) fCritChance = 65.0f;
			fRand = UTIL_SharedRandomFloat(iRandSeed, 0.0, 100.0);
			if(fRand <= fCritChance) iCrit += 2;
			break;
		}
	}
	return iCrit;
}

BOOL CClientWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay )
{
	if (m_iAmmo <= 0)
		return FALSE;

	if(m_fInReload)
		return FALSE;

	int j = min(iClipSize - m_iClip, m_iAmmo);	

	if (j == 0)
		return FALSE;

	m_flNextReload = UTIL_WeaponTimeBase() + fDelay;
	m_flTimeWeaponIdle = m_flNextReload + 0.5;

	SendWeaponAnim(iAnim);

	m_fInReload = TRUE;
	return TRUE;
}

void CClientPlayer::Spawn(void)
{
	if ( m_pActiveItem )
		m_pActiveItem->Deploy( );
}

void CL_BluePrint(int bp);

void CClientPlayer::Killed(void)
{
	if ( m_pActiveItem )
		 m_pActiveItem->Holster( );

	//Clear these client-side effect here
	CL_BluePrint(0);
}

float GetClassMaxSpeed(void)
{
	switch(g_iClass)
	{
	case CLASS_SCOUT: return 325;
	case CLASS_HEAVY: return 192.5;
	case CLASS_SOLDIER: return 200;
	case CLASS_PYRO: return 250;
	case CLASS_SNIPER: return 250;
	case CLASS_MEDIC: return 267.5;
	case CLASS_ENGINEER: return 250;
	case CLASS_DEMOMAN: return 232.5;
	case CLASS_SPY: return 250;
	}
	return 250;
}

void CClientPlayer::ResetMaxSpeed(void)
{
	float speed = GetClassMaxSpeed();

	if (g_iUser1)
		speed = 1200;
	else if (g_iUser3 & CDFLAG_FREEZE)
		speed = 1;
	else if(m_iDisguise == DISGUISE_YES)
		speed = pev.maxspeed;
	else if (m_pActiveItem)
		speed *= m_pActiveItem->GetMaxSpeed();

	g_Player.pev.maxspeed = speed;
}

BOOL CClientPlayer::PlayerCanAttack(void)
{
	if(g_iUser3 & CDFLAG_LIMIT)
		return FALSE;
	if(m_iCloak != CLOAK_NO)
		return FALSE;
	if(g_iUser3 & CDFLAG_FREEZE)
		return FALSE;
	return TRUE;
}

int CClientPlayer::GetNumActivePipebombs(void)
{
	return g_StickyLauncher.m_iStickyNum;
}

int CClientPlayer::GetMedigunHealingTarget(void)
{
	return g_Medigun.m_iHealTarget;
}

bool CClientPlayer::CanPickupBuilding(cl_entity_t *pEntity)
{
	if(g_iClass != CLASS_ENGINEER || m_iCarryBluePrint)
		return false;

	int iOwner = pEntity->curstate.iuser1;

	if(iOwner != gEngfuncs.GetLocalPlayer()->index)
		return false;

	if(!(pev.flags & FL_ONGROUND))
		return false;

	vec3_t vecOrigin, vecLength;
	VectorAdd( pEntity->curstate.mins, pEntity->curstate.maxs, vecOrigin);
	VectorMultiply(vecOrigin, 0.5, vecOrigin);
	VectorAdd(vecOrigin, pEntity->curstate.origin, vecOrigin);
	VectorSubtract( vecOrigin, refparams.vieworg, vecLength );

	float flDistance = VectorLength(vecLength);

	if(flDistance > 64)
		return false;

	int bCanPickup = pEntity->curstate.startpos[2];
	if(bCanPickup)
		return false;

	return true;
}

bool CClientPlayer::PickupBuilding(void)
{
	cl_entity_t *pEnt = g_pTraceEntity;

	if(!pEnt)
		return false;

	if(!CanPickupBuilding(pEnt))
		return false;

	return true;
}

pmtrace_t *FindHullIntersection(float *vecSrc, float *vecEnd, pmtrace_t *tr, float *pflMins, float *pfkMaxs)
{
	pmtrace_t *trTemp, *trResult;
	float flDistance = 1000000;
	float *pflMinMaxs[2] = { pflMins, pfkMaxs };
	vec3_t vecHullEnd, vecTemp, vecSub, vecNewEnd;

	VectorCopy(vecEnd, vecHullEnd);;
	VectorSubtract(vecHullEnd, vecSrc, vecTemp);
	VectorMA(vecSrc, 2, vecTemp, vecHullEnd);

	trTemp = cl_pmove->PM_TraceLineEx(vecSrc, vecHullEnd, PM_NORMAL, 0, CL_TraceEntity_Ignore );

	if (trTemp->fraction < 1)
	{
		return trTemp;
	}

	trResult = tr;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				VectorAdd(vecHullEnd, pflMinMaxs[i], vecNewEnd);

				trTemp = cl_pmove->PM_TraceLineEx(vecSrc, vecNewEnd, PM_NORMAL, 0, CL_TraceEntity_Ignore );

				if (tr->fraction < 1)
				{
					VectorSubtract(vecEnd, vecSrc, vecSub);
					float flThisDistance = VectorLength(vecSub);

					if (flThisDistance < flDistance)
					{
						trResult = trTemp;
						flDistance = flThisDistance;
					}
				}
			}
		}
	}
	return trResult;
}

BOOL IsBackFace(vec3_t anglesAttacker, vec3_t anglesVictim)
{
	float flAngles = anglesAttacker[1] - anglesVictim[1];
	if(flAngles < -180.0) flAngles += 360.0;
	if(flAngles <= 90.0 && flAngles >= -90.0)
		return TRUE;
	return FALSE;
}