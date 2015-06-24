#ifndef EXPLODE_H
#define EXPLODE_H

#define SF_ENVEXPLOSION_NODAMAGE (1<<0)
#define SF_ENVEXPLOSION_REPEATABLE (1<< 1)
#define SF_ENVEXPLOSION_NOFIREBALL (1<< 2)
#define SF_ENVEXPLOSION_NOSMOKE (1<<3)
#define SF_ENVEXPLOSION_NODECAL (1<<4)
#define SF_ENVEXPLOSION_NOSPARKS (1<<5)

extern DLL_GLOBAL short g_sModelIndexFireball;
extern DLL_GLOBAL short g_sModelIndexSmoke;

extern void ExplosionCreate(const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage);
#endif