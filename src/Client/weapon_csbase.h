#ifndef WEAPON_CSBASE_H
#define WEAPON_CSBASE_H

#ifdef _WIN32
#pragma once
#endif

#include "weapons.h"

extern int ClassnameToWeaponID(const char *classname);
extern int AliasToWeaponID(const char *alias);
extern const char *WeaponIDToAlias(int id);
extern const char *GetTranslatedWeaponAlias(const char *alias);
extern bool IsPrimaryWeapon(int id);
extern bool IsSecondaryWeapon(int id);
extern int GetShellForAmmoType(const char *ammoname);

#endif