class CHudCorpse : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);

public:
	int MsgFunc_ClCorpse(const char *pszName, int iSize, void *pbuf);
};