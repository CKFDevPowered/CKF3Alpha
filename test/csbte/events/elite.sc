#include "eventscripts.h"

void main(event_args_t *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int lefthand;
	int bullets_left;
	float time_diff;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;

	ent = GetViewEntity();
	idx = args->entindex;
	lefthand = args->bparam1;
	bullets_left = args->iparam2;
	time_diff = args->fparam1;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	if (IsLocal(idx))
	{
		ShotsFired();
		MuzzleFlash();

		if (lefthand)
		{
			if (bullets_left <= 1)
				WeaponAnimation(7);
			else if (time_diff >= 0.5)
				WeaponAnimation(6);
			else if (time_diff >= 0.4)
				WeaponAnimation(5);
			else if (time_diff >= 0.3)
				WeaponAnimation(4);
			else if (time_diff >= 0.2)
				WeaponAnimation(3);
			else if (time_diff >= 0.0)
				WeaponAnimation(2);

			CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			CreateSmoke(ent->attachment[0], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
		}
		else
		{
			if (bullets_left <= 1)
				WeaponAnimation(13);
			else if (time_diff >= 0.5)
				WeaponAnimation(12);
			else if (time_diff >= 0.4)
				WeaponAnimation(11);
			else if (time_diff >= 0.3)
				WeaponAnimation(10);
			else if (time_diff >= 0.2)
				WeaponAnimation(9);
			else if (time_diff >= 0.0)
				WeaponAnimation(8);

			CreateSmoke(ent->attachment[1], forward, 0, 0.25, 10, 10, 10, 3, velocity, false, 35);
			CreateSmoke(ent->attachment[1], forward, 25, 0.3, 15, 15, 15, 2, velocity, false, 35);
			CreateSmoke(ent->attachment[1], forward, 50, 0.2, 25, 25, 25, 2, velocity, false, 35);
		}
	}

	if (IsLocal(idx))
	{
		if (lefthand)
		{
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, false);
			VectorCopy(ent->attachment[2], ShellOrigin);
		}
		else
		{
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, false);
			VectorCopy(ent->attachment[3], ShellOrigin);
		}

		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] -= 25;
	}
	else
		GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EjectBrass(ShellOrigin, ShellVelocity, angles[1], FindModelIndex("models/pshell.mdl"), TE_BOUNCE_SHELL, idx, 5);

	PlaySound(idx, origin, CHAN_WEAPON, "weapons/elite_fire.wav", 1.0, 0.8, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam2;
	vSpread[1] = args->fparam1 / 100.0;
	vSpread[2] = 0;

	if (lefthand)
	{
		vecSrc[0] -= right[0] * 5;
		vecSrc[1] -= right[1] * 5;
		vecSrc[2] -= right[2] * 5;
	}
	else
	{
		vecSrc[0] += right[0] * 5;
		vecSrc[1] += right[1] * 5;
		vecSrc[2] += right[2] * 5;
	}

	FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 2);
}