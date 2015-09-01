#ifndef CDLL_DLL_H
#define CDLL_DLL_H

#define MAX_WEAPONS 32
#define MAX_WEAPON_SLOTS 5
#define MAX_ITEM_TYPES 6
#define MAX_ITEMS 4

#define HIDEHUD_WEAPONS (1<<0)
#define HIDEHUD_CHARGE (1<<1)
#define HIDEHUD_ALL (1<<2)
#define HIDEHUD_HEALTH (1<<3)
#define HIDEHUD_TIMER (1<<4)
#define HIDEHUD_MONEY (1<<5)
#define HIDEHUD_CROSSHAIR (1<<6)
#define	HIDEHUD_FLASHLIGHT (1<<7)//HL special

#define MAX_AMMO_TYPES 32//32
#define MAX_AMMO_SLOTS 32

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

#define SCOREATTRIB_DEAD (1<<0)
#define SCOREATTRIB_BOMB (1<<1)
#define SCOREATTRIB_VIP (1<<2)

#define STATUSICON_HIDE 0
#define STATUSICON_SHOW 1
#define STATUSICON_FLASH 2

#define TEAM_UNASSIGNED 0
#define TEAM_TERRORIST 1
#define TEAM_CT 2
#define TEAM_RED 1
#define TEAM_BLU 2
#define TEAM_BLUE 2
#define TEAM_SPECTATOR 3

#define CLASS_UNASSIGNED 0
#define CLASS_URBAN 1
#define CLASS_TERROR 2
#define CLASS_LEET 3
#define CLASS_ARCTIC 4
#define CLASS_GSG9 5
#define CLASS_GIGN 6
#define CLASS_SAS 7
#define CLASS_GUERILLA 8
#define CLASS_VIP 9

#define CLASS_SCOUT 1
#define CLASS_HEAVY 2
#define CLASS_SOLDIER 3
#define CLASS_PYRO 4
#define CLASS_SNIPER 5
#define CLASS_MEDIC 6
#define CLASS_ENGINEER 7
#define CLASS_DEMOMAN 8
#define CLASS_SPY 9

#define COLOR_RED "1"
#define COLOR_BLU "140"

#define KEY_1 (1<<0)
#define KEY_2 (1<<1)
#define KEY_3 (1<<2)
#define KEY_4 (1<<3)
#define KEY_5 (1<<4)
#define KEY_6 (1<<5)
#define KEY_7 (1<<6)
#define KEY_8 (1<<7)
#define KEY_9 (1<<8)
#define KEY_0 (1<<9)
#define KEY_10 (1<<10)

#define MENU_CLOSE 0
#define MENU_TEAM 10
#define MENU_CLASS_RED 11
#define MENU_INTRO 12
#define MENU_MAPINFO 13
#define MENU_CLASS_BLU 14

#define MENUBUF_MOTD 1
#define MENUBUF_MAPINFO 2
#define MENUBUF_CLASSINTRO 3

#define CDFLAG_LIMIT (1<<0)
#define CDFLAG_FREEZE (1<<1)
#define CDFLAG_PLANTING (1<<2)
#define CDFLAG_NOBUILD (1<<3)

#define WEAPON_SUIT 31
#endif
