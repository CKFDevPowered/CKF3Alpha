#include "eventscripts.h"

void main(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	int silencer_on;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;
	vec3_t smoke_origin;
	float base_scale;

	ent = GetViewEntity();
	idx = args->entindex;
	empty = args->bparam1;
	silencer_on = args->bparam2;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (IsLocal(idx))
	{
		ShotsFired();

		if (silencer_on)
		{
			if (!empty)
				WeaponAnimation(1 + RandomLong(0, 2));
			else
				WeaponAnimation(4);
		}
		else
		{
			MuzzleFlash();

			if (!empty)
				WeaponAnimation(9 + RandomLong(0, 2));
			else
				WeaponAnimation(12);
		}

		VectorCopy(ent->attachment[0], smoke_origin);

		smoke_origin[0] -= forward[0] * 3;
		smoke_origin[1] -= forward[1] * 3;
		smoke_origin[2] -= forward[2] * 3;

		base_scale = RandomFloat(0.1, 0.25);

		CreateSmoke(ent->attachment[0], forward, 0, smoke_origin[2], 7, 7, 7, 3, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 20, base_scale + 0.1, 10, 10, 10, 2, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 40, base_scale, 13, 13, 13, 2, velocity, false, 35);
	}

	if (IsLocal(idx))
	{
		if (!UseLeftHand())
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, -14.0, false);
		else
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 36.0, -14.0, 14.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.5, ShellVelocity);
		ShellVelocity[2] += 45.0;
	}
	else
		GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EjectBrass(ShellOrigin, ShellVelocity, angles[1], FindModelIndex("models/pshell.mdl"), TE_BOUNCE_SHELL, idx, 5);

	if (!silencer_on)
		PlaySound(idx, origin, CHAN_WEAPON, "weapons/usp_unsil-1.wav", 1.0, 0.8, 0, 87 + RandomLong(0, 18));
	else
		PlaySound(idx, origin, CHAN_WEAPON, "weapons/usp1.wav", 1.0, 2.0, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 4096, BULLET_PLAYER_45ACP, 2);
}