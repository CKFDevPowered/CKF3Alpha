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
		MuzzleFlash();
		WeaponAnimation(1 + RandomLong(0, 1));

		CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
		CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
	}

	if (IsLocal(idx))
	{
		ShotsFired();

		if (!UseLeftHand())
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, -11.0, false);
		else
			GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, 11.0, false);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EjectBrass(ShellOrigin, ShellVelocity, angles[1], FindModelIndex("models/shotgunshell.mdl"), TE_BOUNCE_SHOTSHELL, idx, 3);

	PlaySound(idx, origin, CHAN_WEAPON, "weapons/xm1014-1.wav", 1.0, 0.52, 0, 94 + RandomLong(0, 15));

	GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = 0.0725;
	vSpread[1] = 0.0725;
	vSpread[2] = 0;

	FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_BUCKSHOT, 1);
}