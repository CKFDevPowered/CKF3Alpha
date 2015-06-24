#define MAX_POINTS 64

class CHudRadar : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float fTime);
	void Reset(void);

	int MsgFunc_Radar(const char *pszName, int iSize, void *pbuf);

	void _cdecl UserCmd_TrackPlayer(void);
	void _cdecl UserCmd_ClearPlayers(void);
	void _cdecl UserCmd_DrawRadar(void);
	void _cdecl UserCmd_HideRadar(void);

	void DrawRadarDot(int x, int y, float z_diff, int iBaseDotSize, int flags, int r, int g, int b, int a);
	void DrawRadar(float flTime);
	void DrawPlayerLocation(void);
	int GetRadarSize(void);

public:
	bool m_bTrackArray[MAX_POINTS + 1];
	int m_iPlayerLastPointedAt;
	bool m_bDrawRadar;
	int m_HUD_radar;
	int m_HUD_radaropaque;

private:
	wrect_t *m_hrad;
	wrect_t *m_hradopaque;
	HSPRITE m_hRadar;
	HSPRITE m_hRadaropaque;
};