#define BUYING_EVERYONE 0
#define BUYING_ONLY_CT 1
#define BUYING_ONLY_T 2
#define BUYING_NO_ONE 3

class CMapInfo : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);

public:
	int m_iSetupCondition;
	int m_iWaitTime;
	int m_iFreezeTime;
	int m_iSetupTime;
	int m_iRoundTime;
	int m_iEndTime;
	int m_iEndAction;
	int m_iRedDominatedAction;
	int m_iBluDominatedAction;
};

class CCPLocker : public CPointEntity
{
public:
	CCPLocker(void)
	{
		m_iLockState = 0;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	int m_iLockState;
};

class CRoundTerminator : public CPointEntity
{
public:
	CRoundTerminator(void)
	{
		m_iWinStatus = 0;
		m_iEndTime = -1;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	int m_iWinStatus;
	int m_iEndTime;
};