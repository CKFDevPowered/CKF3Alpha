class CHudNightVision : public CHudBase
{
public:
	void Reset(void);
	int VidInit(void);
	int Init(void);
	int Draw(float flTime);

public:
	int MsgFunc_NVGToggle(const char *pszName, int iSize, void *pbuf);

	void _cdecl UserCmd_NVGAdjustUp(void);
	void _cdecl UserCmd_NVGAdjustDown(void);

public:
	BOOL m_fOn;
	int m_iAlpha;
};