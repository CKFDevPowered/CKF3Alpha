#include "eventscripts.h"

void main(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (IsLocal(idx))
	{
		ShotsFired();
		MuzzleFlash();
		WeaponAnimation(1 + RandomLong(0, 1));

		CreateSmoke(ent->attachment[0], forward, 3, 0.5, 25, 25, 25, 3, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 25, 0.5, 20, 20, 20, 2, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 50, 0.45, 15, 15, 15, 2, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 75, 0.35, 7, 7, 7, 2, velocity, false, 35);
	}

	PlaySound(idx, origin, CHAN_WEAPON, "weapons/m3-1.wav", 1.0, 0.48, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = 0.0675;
	vSpread[1] = 0.0675;
	vSpread[2] = 0;

	FireBullets(idx, forward, right, up, 9, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_BUCKSHOT, 1);
}