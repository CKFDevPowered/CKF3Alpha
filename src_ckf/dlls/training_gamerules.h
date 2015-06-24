class CHalfLifeTraining : public CHalfLifeMultiplay
{
public:
	CHalfLifeTraining(void);

public:
	virtual BOOL IsMultiplayer(void);
	virtual BOOL IsDeathmatch(void);
	virtual void InitHUD(CBasePlayer *pl);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual int ItemShouldRespawn(CItem *pItem);
	virtual void CheckMapConditions(void);
	virtual void CheckWinConditions(void);

public:
	static void HostageDied(void);
	static BOOL PlayerCanBuy(CBasePlayer *pPlayer);

private:
	float m_flNextGiveMoney;
	BOOL m_fShowCanBuy;
	BOOL m_fShowBuyTip;
	BOOL m_bVGUIMenus;
};