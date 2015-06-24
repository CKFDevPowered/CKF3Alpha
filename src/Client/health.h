#define DMG_IMAGE_LIFE 2

#define DMG_IMAGE_POISON 0
#define DMG_IMAGE_ACID 1
#define DMG_IMAGE_COLD 2
#define DMG_IMAGE_DROWN 3
#define DMG_IMAGE_BURN 4
#define DMG_IMAGE_NERVE 5
#define DMG_IMAGE_RAD 6
#define DMG_IMAGE_SHOCK 7
#define DMG_IMAGE_CALTROP 8
#define DMG_IMAGE_TRANQ 9
#define DMG_IMAGE_CONCUSS 10
#define DMG_IMAGE_HALLUC 11
#define NUM_DMG_TYPES 12

#define DMG_GENERIC 0
#define DMG_CRUSH (1<<0)
#define DMG_BULLET (1<<1)
#define DMG_SLASH (1<<2)
#define DMG_BURN (1<<3)
#define DMG_FREEZE (1<<4)
#define DMG_FALL (1<<5)
#define DMG_BLAST (1<<6)
#define DMG_CLUB (1<<7)
#define DMG_SHOCK (1<<8)
#define DMG_SONIC (1<<9)
#define DMG_ENERGYBEAM (1<<10)
#define DMG_NEVERGIB (1<<12)
#define DMG_ALWAYSGIB (1<<13)

#define DMG_TIMEBASED (~(0xff003fff))

#define DMG_DROWN (1<<14)
#define DMG_FIRSTTIMEBASED DMG_DROW

#define DMG_PARALYZE (1<<15)
#define DMG_NERVEGAS (1<<16)
#define DMG_POISON (1<<17)
#define DMG_RADIATION (1<<18)
#define DMG_DROWNRECOVER (1<<19)
#define DMG_ACID (1<<20)
#define DMG_SLOWBURN (1<<21)
#define DMG_SLOWFREEZE (1<<22)
#define DMG_MORTAR (1<<23)

#define DMG_IGNITE (1<<24)
#define DMG_RADIUS_MAX (1<<25)
#define DMG_RADIUS_QUAKE (1<<26)
#define DMG_IGNOREARMOR (1<<27)
#define DMG_AIMED (1<<28)
#define DMG_WALLPIERCING (1<<29)

#define DMG_CALTROP (1<<30)
#define DMG_HALLUC (1<<31)

#define DMG_IGNORE_MAXHEALTH DMG_IGNITE
#define DMG_NAIL DMG_SLASH
#define DMG_NOT_SELF DMG_FREEZE

#define DMG_TRANQ DMG_MORTAR
#define DMG_CONCUSS DMG_SONIC

typedef struct
{
	float fExpire;
	float fBaseline;
	int x, y;
}
DAMAGE_IMAGE;

#define MAX_POINTS 64

class CHudHealth : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float fTime);
	void Reset(void);

	int MsgFunc_Health(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Damage(const char *pszName, int iSize, void *pbuf);

	void GetPainColor(int &r, int &g, int &b);
	int DrawPain(float fTime);
	int DrawDamage(float fTime);
	void CalcDamageDirection(vec3_t vecFrom);
	void UpdateTiles(float fTime, long bits);

public:
	int m_iHealth;
	int m_HUD_dmg_bio;
	int m_HUD_cross;
	float m_fFade;
	float m_fAttackFront, m_fAttackRear, m_fAttackLeft, m_fAttackRight;

private:
	HSPRITE m_hSprite;
	HSPRITE m_hDamage;
	DAMAGE_IMAGE m_dmg[NUM_DMG_TYPES];
	int m_bitsDamage;
};