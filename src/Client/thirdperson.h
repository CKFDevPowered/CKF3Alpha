class CHudThirdPerson : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void CalcRefdef(struct ref_params_s *pparams);

public:
	int MsgFunc_ThirdPerson(const char *pszName, int iSize, void *pbuf);

public:
	bool m_bToggle;
	bool m_bLater;
	float m_flForwardOfs;
	float m_flRightOfs;
	float m_flUpOfs;
};