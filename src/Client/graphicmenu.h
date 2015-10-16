class CHudGraphicMenu : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);

public:
	int MsgFunc_TeamMenu(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ClassMenu(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_BuyMenu(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GraphicMenu(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_DrawFX(const char *pszName, int iSize, void *pbuf);
public:
};