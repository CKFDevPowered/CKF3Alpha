#ifndef EFFECTS_H
#define EFFECTS_H

#define SF_BEAM_STARTON 0x0001
#define SF_BEAM_TOGGLE 0x0002
#define SF_BEAM_RANDOM 0x0004
#define SF_BEAM_RING 0x0008
#define SF_BEAM_SPARKSTART 0x0010
#define SF_BEAM_SPARKEND 0x0020
#define SF_BEAM_DECALS 0x0040
#define SF_BEAM_SHADEIN 0x0080
#define SF_BEAM_SHADEOUT 0x0100
#define SF_BEAM_TEMPORARY 0x8000

#define SF_SPRITE_STARTON 0x0001
#define SF_SPRITE_ONCE 0x0002
#define SF_SPRITE_TEMPORARY 0x8000

class CSprite : public CPointEntity
{
public:
	void Spawn(void);
	void Restart(void);
	void Precache(void);

	int ObjectCaps(void)
	{
		int flags = 0;

		if (pev->spawnflags & SF_SPRITE_TEMPORARY)
			flags = FCAP_DONT_SAVE;

		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Animate(float frames);
	void Expand(float scaleSpeed, float fadeSpeed);
	void SpriteInit(const char *pSpriteName, const Vector &origin);

	inline void SetAttachment(edict_t *pEntity, int attachment)
	{
		if (pEntity)
		{
			pev->skin = ENTINDEX(pEntity);
			pev->body = attachment;
			pev->aiment = pEntity;
			pev->movetype = MOVETYPE_FOLLOW;
		}
	}
	void TurnOff(void);
	void TurnOn(void);

public:
	inline float Frames(void) { return m_maxFrame; }

	inline void SetTransparency(int rendermode, int r, int g, int b, int a, int fx)
	{
		pev->rendermode = rendermode;
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
		pev->renderamt = a;
		pev->renderfx = fx;
	}
	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetScale(float scale) { pev->scale = scale; }
	inline void SetColor(int r, int g, int b) { pev->rendercolor.x = r; pev->rendercolor.y = g; pev->rendercolor.z = b; }
	inline void SetBrightness(int brightness) { pev->renderamt = brightness; }

	inline void AnimateAndDie(float framerate)
	{
		SetThink(&CSprite::AnimateUntilDead);
		pev->framerate = framerate;
		pev->dmgtime = gpGlobals->time + (m_maxFrame / framerate);
		pev->nextthink = gpGlobals->time;
	}

public:
	void EXPORT AnimateThink(void);
	void EXPORT ExpandThink(void);
	void EXPORT AnimateUntilDead(void);

	static CSprite *SpriteCreate(const char *pSpriteName, const Vector &origin, BOOL animate);

private:
	float m_lastTime;
	float m_maxFrame;
};

class CBeam : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);

	int ObjectCaps(void)
	{
		int flags = 0;

		if (pev->spawnflags & SF_BEAM_TEMPORARY)
			flags = FCAP_DONT_SAVE;

		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

public:
	inline void SetType(int type) { pev->rendermode = (pev->rendermode & 0xF0) | (type & 0x0F); }
	inline void SetFlags(int flags) { pev->rendermode = (pev->rendermode & 0x0F) | (flags & 0xF0); }
	inline void SetStartPos(const Vector &pos) { pev->origin = pos; }
	inline void SetEndPos(const Vector &pos) { pev->angles = pos; }

	void SetStartEntity(int entityIndex);
	void SetEndEntity(int entityIndex);

	inline void SetStartAttachment(int attachment) { pev->sequence = (pev->sequence & 0x0FFF) | ((attachment & 0xF) << 12); }
	inline void SetEndAttachment(int attachment) { pev->skin = (pev->skin & 0x0FFF) | ((attachment & 0xF) << 12); }
	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetWidth(int width) { pev->scale = width; }
	inline void SetNoise(int amplitude) { pev->body = amplitude; }
	inline void SetColor(int r, int g, int b) { pev->rendercolor.x = r; pev->rendercolor.y = g; pev->rendercolor.z = b; }
	inline void SetBrightness(int brightness) { pev->renderamt = brightness; }
	inline void SetFrame(float frame) { pev->frame = frame; }
	inline void SetScrollRate(int speed) { pev->animtime = speed; }

public:
	inline int GetType(void) { return pev->rendermode & 0x0F; }
	inline int GetFlags(void) { return pev->rendermode & 0xF0; }
	inline int GetStartEntity(void) { return pev->sequence & 0xFFF; }
	inline int GetEndEntity(void) { return pev->skin & 0xFFF; }
	const Vector &GetStartPos(void);
	const Vector &GetEndPos(void);

public:
	Vector Center(void) { return (GetStartPos() + GetEndPos()) * 0.5; }

public:
	inline int GetTexture(void) { return pev->modelindex; }
	inline int GetWidth(void) { return (int)(pev->scale); }
	inline int GetNoise(void) { return pev->body; }
	inline int GetBrightness(void) { return (int)(pev->renderamt); }
	inline int GetFrame(void) { return (int)(pev->frame); }
	inline int GetScrollRate(void) { return (int)(pev->animtime); }

public:
	void EXPORT TriggerTouch(CBaseEntity *pOther);
	void RelinkBeam(void);
	void DoSparks(const Vector &start, const Vector &end);
	CBaseEntity *RandomTargetname(const char *szName);
	void BeamDamage(TraceResult *ptr);
	void BeamInit(const char *pSpriteName, int width);
	void PointsInit(const Vector &start, const Vector &end);
	void PointEntInit(const Vector &start, int endIndex);
	void EntsInit(int startIndex, int endIndex);
	void HoseInit(const Vector &start, const Vector &direction);

public:
	static CBeam *BeamCreate(const char *pSpriteName, int width);

public:
	inline void LiveForTime(float time) { SetThink(&CBaseEntity::SUB_Remove); pev->nextthink = gpGlobals->time + time; }

	inline void BeamDamageInstant(TraceResult *ptr, float damage)
	{
		pev->dmg = damage;
		pev->dmgtime = gpGlobals->time - 1;
		BeamDamage(ptr);
	}
};

#define SF_MESSAGE_ONCE 0x0001
#define SF_MESSAGE_ALL 0x0002

class CLaser : public CBeam
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	void TurnOn(void);
	void TurnOff(void);
	int IsOn(void);

public:
	void FireAtPoint(TraceResult &point);
	void EXPORT StrikeThink(void);

public:
	CSprite *m_pSprite;
	int m_iszSpriteName;
	Vector m_firePosition;
};

#endif