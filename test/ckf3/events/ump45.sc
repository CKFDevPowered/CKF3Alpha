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
		WeaponAnimation(3 + RandomLong(0, 2));
		CreateSmoke(ent->attachment[0], forward, 3, 0.25, 10, 10, 10, 3, velocity, false, 35);
	}

	if (IsLocal(idx))
	{
		if (!UseLeftHand())
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, -11.0, false);
		else
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 50;
	}
	else
		GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EjectBrass(ShellOrigin, ShellVelocity, angles[1], FindModelIndex("models/pshell.mdl"), TE_BOUNCE_SHELL, idx, 13);

	PlaySound(idx, origin, CHAN_WEAPON, "weapons/ump45-1.wav", 1.0, 0.64, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_45ACP, 2);
}