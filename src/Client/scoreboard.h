class CHudScoreboard : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float fTime);
	void Reset(void);

public:
	int MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf);

public:
	void CacheTeamAliveNumber(void);
	void BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset);
	int DrawHudNumber(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber);

public:
	float m_flNextCache;
	int m_iTeamScore_T, m_iTeamScore_CT;
	int m_iTeamAlive_T, m_iTeamAlive_CT;

public:
	int m_iBGIndex;
	int m_iTextIndex;
	int m_iTTextIndex, m_iCTTextIndex;

	wrect_t m_rcNumber_Large[10];
	wrect_t m_rcNumber_Small[10];

public:
	int m_iOriginalBG;
	int m_iTeamDeathBG;
	int m_iUnitehBG;
	int m_iNum_L;
	int m_iNum_S;
	int m_iText_CT;
	int m_iText_T;
	int m_iText_TR;
	int m_iText_HM;
	int m_iText_ZB;
	int m_iText_1st;
	int m_iText_Kill;
	int m_iText_Round;
};