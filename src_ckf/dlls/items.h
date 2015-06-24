#ifndef ITEMS_H
#define ITEMS_H

class CItem : public CBaseEntity
{
public:
	virtual void Spawn(void);
	virtual CBaseEntity *Respawn(void);
	virtual BOOL MyTouch(CBasePlayer *pPlayer) { return FALSE; }
	virtual void Restart(void);
public:
	void EXPORT ItemTouch(CBaseEntity *pOther);
	void EXPORT Materialize(void);
};

#endif