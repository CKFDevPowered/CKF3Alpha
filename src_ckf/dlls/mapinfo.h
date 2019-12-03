#define BUYING_EVERYONE 0
#define BUYING_ONLY_CT 1
#define BUYING_ONLY_T 2
#define BUYING_NO_ONE 3

class CMapInfo : public CPointEntity
{
public:
	CMapInfo(void)
	{
		m_iSetupCondition = 0;
		m_iWaitTime = 0;
		m_iFreezeTime = 0;
		m_iSetupTime = 0;
		m_iEndTime = 0;
		m_iRedDominatedAction = 0;
		m_iBluDominatedAction = 0;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);

public:
	int m_iSetupCondition;
	int m_iWaitTime;
	int m_iFreezeTime;
	int m_iSetupTime;
	int m_iEndTime;
	int m_iRedDominatedAction;
	int m_iBluDominatedAction;
};

typedef enum {
	SKIP = 0,
	UPDATE,
	TOGGLE,
	RESET,
	MODIFY
} CONTROL_OP;

class CCPControls : public CPointEntity
{
public:
	CCPControls(void)
	{
		m_opLockedOp = SKIP;
		m_bLockedArg = FALSE;
		m_opDisabledOp = SKIP;
		m_bDisabledArg = FALSE;
		m_opCanRedCapOp = SKIP;
		m_bCanRedCapArg = FALSE;
		m_opCanBluCapOp = SKIP;
		m_bCanBluCapArg = FALSE;
		m_opTeamOp = SKIP;
		m_iTeamArg = 0;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	CONTROL_OP	m_opLockedOp;
	BOOL		m_bLockedArg;
	CONTROL_OP	m_opDisabledOp;
	BOOL		m_bDisabledArg;
	CONTROL_OP	m_opCanRedCapOp;
	BOOL		m_bCanRedCapArg;
	CONTROL_OP	m_opCanBluCapOp;
	BOOL		m_bCanBluCapArg;
	CONTROL_OP	m_opTeamOp;
	int			m_iTeamArg;
};

class CRoundTerminator : public CPointEntity
{
public:
	CRoundTerminator(void)
	{
		m_iWinStatus = 0;
		m_iEndTime = 0;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	int m_iWinStatus;
	int m_iEndTime;
};

class CRTControls : public CPointEntity
{
public:
	CRTControls(void)
	{
		m_opLockedOp = SKIP;
		m_bLockedArg = FALSE;
		m_opDisabledOp = SKIP;
		m_bDisabledArg = FALSE;
		m_opTimeOp = SKIP;
		m_iTimeArg = 0;
	}
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	CONTROL_OP	m_opLockedOp;
	BOOL		m_bLockedArg;
	CONTROL_OP	m_opDisabledOp;
	BOOL		m_bDisabledArg;
	CONTROL_OP	m_opTimeOp;
	int			m_iTimeArg;
};
