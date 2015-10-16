#include "eventscripts.h"

void main(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
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
		WeaponAnimation(1 + RandomLong(0, 2));

		CreateSmoke(ent->attachment[0], forward, 3, 0.5, 20, 20, 20, 3, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 40, 0.5, 15, 15, 15, 2, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 80, 0.5, 10, 10, 10, 2, velocity, false, 35);
	}

	PlaySound(idx, origin, CHAN_WEAPON, "weapons/awp1.wav", 1.0, 0.28, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	VectorCopy(forward, vecAiming);

	FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_338MAG, 3);
}