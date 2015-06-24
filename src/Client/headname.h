class CHudHeadName : public CHudBase
{
public:
	int Draw(float flTime);
	int Init(void);
	int VidInit(void);
	void Reset(void);

public:
	void BuildUnicodeList(void);
	bool IsValidEntity(cl_entity_s *pEntity);

public:
	int MsgFunc_HeadName(const char *pszName, int iSize, void *pbuf);

private:
	wchar_t m_sUnicodes[MAX_CLIENTS][32];
	float m_flNextBuild;
	vgui::HFont m_hHeadFont;
	int m_iDrawType;
};