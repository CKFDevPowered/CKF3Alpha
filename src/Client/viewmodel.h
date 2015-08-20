class CHudViewModel : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void CalcRefdef(struct ref_params_s *pparams);

public:
	int MsgFunc_ViewBodySkin(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ViewRender(const char *pszName, int iSize, void *pbuf);

public:
	int m_body;
	int m_skin;
	int m_rendermode;
	int m_renderamt;
	color24 m_rendercolor;
	int m_renderfx;
};