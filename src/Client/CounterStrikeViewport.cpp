#include "hud.h"
#include "cl_util.h"
#include "CounterStrikeViewport.h"

extern void *gpViewPortInterface;

static VGuiLibraryInterface_t *(__fastcall *m_pfnCounterStrikeViewpot_GetClientDllInterface)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_SetClientDllInterface)(void *pthis, int, VGuiLibraryInterface_t *clientInterface) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateScoreBoard)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_AllowedToPrintText)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_GetAllPlayersInfo)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_DeathMsg)(void *pthis, int, int killer, int victim) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowScoreBoard)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_CanShowScoreBoard)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideAllVGUIMenu)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateSpectatorPanel)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsScoreBoardVisible)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideScoreBoard)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_KeyInput)(void *pthis, int, int down, int keynum, const char *pszCurrentBinding) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowVGUIMenu)(void *pthis, int, int iMenu) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideVGUIMenu)(void *pthis, int, int iMenu) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowTutorTextWindow)(void *pthis, int, const wchar_t *szString, int id, int msgClass, int isSpectator) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowTutorLine)(void *pthis, int, int entindex, int id) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowTutorState)(void *pthis, int, const wchar_t *szString) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_CloseTutorTextWindow)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsTutorTextWindowOpen)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowSpectatorGUI)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowSpectatorGUIBar)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideSpectatorGUI)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_DeactivateSpectatorGUI)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsSpectatorGUIVisible)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsSpectatorBarVisible)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_ResetFade)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_SetSpectatorBanner)(void *pthis, int, const char *image) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_SpectatorGUIEnableInsetView)(void *pthis, int, int value) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ShowCommandMenu)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateCommandMenu)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideCommandMenu)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_IsCommandMenuVisible)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetValidClasses)(void *pthis, int, int iTeam) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetNumberOfTeams)(void *pthis, int, int iTeam) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_GetIsFeigning)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetIsSettingDetpack)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetBuildState)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_IsRandom)(void *pthis, int) = NULL;
static char *(__fastcall *m_pfnCounterStrikeViewpot_GetTeamName)(void *pthis, int, int iTeam) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetCurrentMenu)(void *pthis, int) = NULL;
static const char *(__fastcall *m_pfnCounterStrikeViewpot_GetMapName)(void *pthis, int) = NULL;
static const char *(__fastcall *m_pfnCounterStrikeViewpot_GetServerName)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_InputPlayerSpecial)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_OnTick)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetViewPortScheme)(void *pthis, int) = NULL;
static vgui::VPANEL (__fastcall *m_pfnCounterStrikeViewpot_GetViewPortPanel)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetAllowSpectators)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_OnLevelChange)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideBackGround)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ChatInputPosition)(void *pthis, int, int *x, int *y) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetSpectatorBottomBarHeight)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetSpectatorTopBarHeight)(void *pthis, int) = NULL;
static bool(__fastcall *m_pfnCounterStrikeViewpot_SlotInput)(void *pthis, int, int iSlot) = NULL;
static VGuiLibraryTeamInfo_t (__fastcall *m_pfnCounterStrikeViewpot_GetPlayerTeamInfo)(void *pthis, int, int playerIndex) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_MakeSafeName)(void *pthis, int, const char *oldName, char *newName, int newNameBufSize) = NULL;

static void (__fastcall *m_pfnCounterStrikeViewpot_Initialize)(void *pthis, int, CreateInterfaceFn *factories, int count) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_Start)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_Shutdown)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_SetParent)(void *pthis, int, vgui::VPANEL parent) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_UseVGUI1)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_ActivateClientUI)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_HideClientUI)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_SetNumberOfTeams)(void *pthis, int, int num) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_ValClass)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TeamNames)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_Feign)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_Detpack)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_VGUIMenu)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TutorText)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TutorLine)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TutorState)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TutorClose)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_MOTD)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_BuildSt)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_RandomPC)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_ServerName)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_ScoreInfo)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TeamScore)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_TeamInfo)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_Spectator)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_AllowSpec)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_SpecFade)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsVGUIMenuActive)(void *pthis, int, int iMenu) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsAnyVGUIMenuActive)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_DisplayVGUIMenu)(void *pthis, int, int iMenu) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_GetForceCamera)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_ForceCam)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static int (__fastcall *m_pfnCounterStrikeViewpot_MsgFunc_Location)(void *pthis, int, const char *pszName, int iSize, void *pbuf) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateBuyPresets)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateScheme)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnCounterStrikeViewpot_IsProgressBarVisible)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_StartProgressBar)(void *pthis, int, const char *title, int numTicks, int startTicks, bool isTimeBased) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_UpdateProgressBar)(void *pthis, int, const char *statusText, int tick) = NULL;
static void (__fastcall *m_pfnCounterStrikeViewpot_StopProgressBar)(void *pthis, int) = NULL;

class CCounterStrikeViewport : public CounterStrikeViewport
{
public:
	virtual VGuiLibraryInterface_t *GetClientDllInterface(void);
	virtual void SetClientDllInterface(VGuiLibraryInterface_t *clientInterface);
	virtual void UpdateScoreBoard(void);
	virtual bool AllowedToPrintText(void);
	virtual void GetAllPlayersInfo(void);
	virtual void DeathMsg(int killer, int victim);
	virtual void ShowScoreBoard(void);
	virtual bool CanShowScoreBoard(void);
	virtual void HideAllVGUIMenu(void);
	virtual void UpdateSpectatorPanel(void);
	virtual bool IsScoreBoardVisible(void);
	virtual void HideScoreBoard(void);
	virtual int KeyInput(int down, int keynum, const char *pszCurrentBinding);
	virtual void ShowVGUIMenu(int iMenu);
	virtual void HideVGUIMenu(int iMenu);
	virtual void ShowTutorTextWindow(const wchar_t *szString, int id, int msgClass, int isSpectator);
	virtual void ShowTutorLine(int entindex, int id);
	virtual void ShowTutorState(const wchar_t *szString);
	virtual void CloseTutorTextWindow(void);
	virtual bool IsTutorTextWindowOpen(void);
	virtual void ShowSpectatorGUI(void);
	virtual void ShowSpectatorGUIBar(void);
	virtual void HideSpectatorGUI(void);
	virtual void DeactivateSpectatorGUI(void);
	virtual bool IsSpectatorGUIVisible(void);
	virtual bool IsSpectatorBarVisible(void);
	virtual int MsgFunc_ResetFade(const char *pszName, int iSize, void *pbuf);
	virtual void SetSpectatorBanner(const char *image);
	virtual void SpectatorGUIEnableInsetView(int value);
	virtual void ShowCommandMenu(void);
	virtual void UpdateCommandMenu(void);
	virtual void HideCommandMenu(void);
	virtual int IsCommandMenuVisible(void);
	virtual int GetValidClasses(int iTeam);
	virtual int GetNumberOfTeams(int iTeam);
	virtual bool GetIsFeigning(void);
	virtual int GetIsSettingDetpack(void);
	virtual int GetBuildState(void);
	virtual int IsRandom(void);
	virtual char *GetTeamName(int iTeam);
	virtual int GetCurrentMenu(void);
	virtual const char *GetMapName(void);
	virtual const char *GetServerName(void);
	virtual void InputPlayerSpecial(void);
	virtual void OnTick(void);
	virtual int GetViewPortScheme(void);
	virtual vgui::VPANEL GetViewPortPanel(void);
	virtual int GetAllowSpectators(void);
	virtual void OnLevelChange(void);
	virtual void HideBackGround(void);
	virtual void ChatInputPosition(int *x, int *y);
	virtual int GetSpectatorBottomBarHeight(void);
	virtual int GetSpectatorTopBarHeight(void);
	virtual bool SlotInput(int iSlot);
	virtual VGuiLibraryTeamInfo_t GetPlayerTeamInfo(int playerIndex);
	virtual void MakeSafeName(const char *oldName, char *newName, int newNameBufSize);
};

CCounterStrikeViewport g_CounterStrikeViewport;
CounterStrikeViewport *g_pCounterStrikeViewpot;

VGuiLibraryInterface_t *CCounterStrikeViewport::GetClientDllInterface(void)
{
	return m_pfnCounterStrikeViewpot_GetClientDllInterface(this, 0);
}

void CCounterStrikeViewport::SetClientDllInterface(VGuiLibraryInterface_t *clientInterface)
{
	m_pfnCounterStrikeViewpot_SetClientDllInterface(this, 0, clientInterface);
}

void CCounterStrikeViewport::UpdateScoreBoard(void)
{
	m_pfnCounterStrikeViewpot_UpdateScoreBoard(this, 0);
}

bool CCounterStrikeViewport::AllowedToPrintText(void)
{
	return m_pfnCounterStrikeViewpot_AllowedToPrintText(this, 0);
}

void CCounterStrikeViewport::GetAllPlayersInfo(void)
{
	for (int j = 1; j < MAX_PLAYERS; j++)
		GetPlayerInfo(j, &g_PlayerInfoList[j]);
}

void CCounterStrikeViewport::DeathMsg(int killer, int victim)
{
	m_pfnCounterStrikeViewpot_DeathMsg(this, 0, killer, victim);
}

extern void CKF_ShowScoreBoard(bool state);

void CCounterStrikeViewport::ShowScoreBoard(void)
{
	//CKF_ShowScoreBoard(true);
	g_pViewPort->ShowScoreBoard();
}

bool CCounterStrikeViewport::CanShowScoreBoard(void)
{
	return m_pfnCounterStrikeViewpot_CanShowScoreBoard(this, 0);
}

void CCounterStrikeViewport::HideAllVGUIMenu(void)
{
}

void CCounterStrikeViewport::UpdateSpectatorPanel(void)
{
	g_pViewPort->UpdateSpectatorPanel();
}

bool CCounterStrikeViewport::IsScoreBoardVisible(void)
{
	return g_pViewPort->IsScoreBoardVisible();
}

void CCounterStrikeViewport::HideScoreBoard(void)
{
	//CKF_ShowScoreBoard(false);
	g_pViewPort->HideScoreBoard();
}

int CCounterStrikeViewport::KeyInput(int down, int keynum, const char *pszCurrentBinding)
{
	return m_pfnCounterStrikeViewpot_KeyInput(this, 0, down, keynum, pszCurrentBinding);
}

void CCounterStrikeViewport::ShowVGUIMenu(int iMenu)
{
	g_pViewPort->ShowVGUIMenu(iMenu);
}

void CCounterStrikeViewport::HideVGUIMenu(int iMenu)
{
	g_pViewPort->HideVGUIMenu(iMenu);
}

void CCounterStrikeViewport::ShowTutorTextWindow(const wchar_t *szString, int id, int msgClass, int isSpectator)
{
	m_pfnCounterStrikeViewpot_ShowTutorTextWindow(this, 0, szString, id, msgClass, isSpectator);
}

void CCounterStrikeViewport::ShowTutorLine(int entindex, int id)
{
	m_pfnCounterStrikeViewpot_ShowTutorLine(this, 0, entindex, id);
}

void CCounterStrikeViewport::ShowTutorState(const wchar_t *szString)
{
	m_pfnCounterStrikeViewpot_ShowTutorState(this, 0, szString);
}

void CCounterStrikeViewport::CloseTutorTextWindow(void)
{
	m_pfnCounterStrikeViewpot_CloseTutorTextWindow(this, 0);
}

bool CCounterStrikeViewport::IsTutorTextWindowOpen(void)
{
	return m_pfnCounterStrikeViewpot_IsTutorTextWindowOpen(this, 0);
}

void CCounterStrikeViewport::ShowSpectatorGUI(void)
{
	g_pViewPort->ShowSpectatorGUI();
}

void CCounterStrikeViewport::ShowSpectatorGUIBar(void)
{
	g_pViewPort->ShowSpectatorGUIBar();
}

void CCounterStrikeViewport::HideSpectatorGUI(void)
{
	g_pViewPort->HideSpectatorGUI();
}

void CCounterStrikeViewport::DeactivateSpectatorGUI(void)
{
	g_pViewPort->DeactivateSpectatorGUI();
}

bool CCounterStrikeViewport::IsSpectatorGUIVisible(void)
{
	return g_pViewPort->IsSpectatorGUIVisible();
}

bool CCounterStrikeViewport::IsSpectatorBarVisible(void)
{
	return g_pViewPort->IsSpectatorBarVisible();
}

int CCounterStrikeViewport::MsgFunc_ResetFade(const char *pszName, int iSize, void *pbuf)
{
	return m_pfnCounterStrikeViewpot_MsgFunc_ResetFade(this, 0, pszName, iSize, pbuf);
}

void CCounterStrikeViewport::SetSpectatorBanner(const char *image)
{
	g_pViewPort->SetSpectatorBanner(image);
}

void CCounterStrikeViewport::SpectatorGUIEnableInsetView(int value)
{
	m_pfnCounterStrikeViewpot_SpectatorGUIEnableInsetView(this, 0, value);
}

void CCounterStrikeViewport::ShowCommandMenu(void)
{
	g_pViewPort->ShowCommandMenu();
}

void CCounterStrikeViewport::UpdateCommandMenu(void)
{
	g_pViewPort->UpdateCommandMenu();
}

void CCounterStrikeViewport::HideCommandMenu(void)
{
	g_pViewPort->HideCommandMenu();
}

int CCounterStrikeViewport::IsCommandMenuVisible(void)
{
	return g_pViewPort->IsCommandMenuVisible();
}

int CCounterStrikeViewport::GetValidClasses(int iTeam)
{
	return m_pfnCounterStrikeViewpot_GetValidClasses(this, 0, iTeam);
}

int CCounterStrikeViewport::GetNumberOfTeams(int iTeam)
{
	return m_pfnCounterStrikeViewpot_GetNumberOfTeams(this, 0, iTeam);
}

bool CCounterStrikeViewport::GetIsFeigning(void)
{
	return m_pfnCounterStrikeViewpot_GetIsFeigning(this, 0);
}

int CCounterStrikeViewport::GetIsSettingDetpack(void)
{
	return m_pfnCounterStrikeViewpot_GetIsSettingDetpack(this, 0);
}

int CCounterStrikeViewport::GetBuildState(void)
{
	return m_pfnCounterStrikeViewpot_GetBuildState(this, 0);
}

int CCounterStrikeViewport::IsRandom(void)
{
	return m_pfnCounterStrikeViewpot_IsRandom(this, 0);
}

char *CCounterStrikeViewport::GetTeamName(int iTeam)
{
	return m_pfnCounterStrikeViewpot_GetTeamName(this, 0, iTeam);
}

int CCounterStrikeViewport::GetCurrentMenu(void)
{
	return m_pfnCounterStrikeViewpot_GetCurrentMenu(this, 0);
}

const char *CCounterStrikeViewport::GetMapName(void)
{
	return m_pfnCounterStrikeViewpot_GetMapName(this, 0);
}

const char *CCounterStrikeViewport::GetServerName(void)
{
	return m_pfnCounterStrikeViewpot_GetServerName(this, 0);
}

void CCounterStrikeViewport::InputPlayerSpecial(void)
{
	m_pfnCounterStrikeViewpot_InputPlayerSpecial(this, 0);
}

void CCounterStrikeViewport::OnTick(void)
{
	m_pfnCounterStrikeViewpot_OnTick(this, 0);
}

int CCounterStrikeViewport::GetViewPortScheme(void)
{
	return g_pViewPort->GetScheme();
}

vgui::VPANEL CCounterStrikeViewport::GetViewPortPanel(void)
{
	return g_pViewPort->GetVPanel();
}

int CCounterStrikeViewport::GetAllowSpectators(void)
{
	return m_pfnCounterStrikeViewpot_GetAllowSpectators(this, 0);
}

void CCounterStrikeViewport::OnLevelChange(void)
{
	m_pfnCounterStrikeViewpot_OnLevelChange(this, 0);
}

void CCounterStrikeViewport::HideBackGround(void)
{
	m_pfnCounterStrikeViewpot_HideBackGround(this, 0);
}

void CCounterStrikeViewport::ChatInputPosition(int *x, int *y)
{
	m_pfnCounterStrikeViewpot_ChatInputPosition(this, 0, x, y);
}

int CCounterStrikeViewport::GetSpectatorBottomBarHeight(void)
{
	return g_pViewPort->GetSpectatorBottomBarHeight();
}

int CCounterStrikeViewport::GetSpectatorTopBarHeight(void)
{
	return g_pViewPort->GetSpectatorTopBarHeight();
}

bool CCounterStrikeViewport::SlotInput(int iSlot)
{
	return m_pfnCounterStrikeViewpot_SlotInput(this, 0, iSlot);
}

VGuiLibraryTeamInfo_t CCounterStrikeViewport::GetPlayerTeamInfo(int playerIndex)
{
	return m_pfnCounterStrikeViewpot_GetPlayerTeamInfo(this, 0, playerIndex);
}

void CCounterStrikeViewport::MakeSafeName(const char *oldName, char *newName, int newNameBufSize)
{
	m_pfnCounterStrikeViewpot_MakeSafeName(this, 0, oldName, newName, newNameBufSize);
}

extern void (__fastcall *m_pfnCounterStrikeViewpot_StartProgressBar)(void *pthis, int, const char *title, int numTicks, int startTicks, bool isTimeBased);
extern void (__fastcall *m_pfnCounterStrikeViewpot_UpdateProgressBar)(void *pthis, int, const char *statusText, int tick);
extern void (__fastcall *m_pfnCounterStrikeViewpot_StopProgressBar)(void *pthis, int);

void __fastcall CCounterStrikeViewport_StartProgressBar(void *pthis, int, const char *title, int numTicks, int startTicks, bool isTimeBased)
{
	g_pViewPort->StartProgressBar(title, numTicks, startTicks, isTimeBased);
}

void __fastcall CCounterStrikeViewport_UpdateProgressBar(void *pthis, int, const char *statusText, int tick)
{
	g_pViewPort->UpdateProgressBar(statusText, tick);
}

void __fastcall CCounterStrikeViewport_StopProgressBar(void *pthis, int)
{
	g_pViewPort->StopProgressBar();
}

void InstallCounterStrikeViewportHook(void)
{
	DWORD *pVFTable = *(DWORD **)&g_CounterStrikeViewport;

	g_pCounterStrikeViewpot = gViewPortInterface;
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 2, (void *)pVFTable[2], (void *&)m_pfnCounterStrikeViewpot_UpdateScoreBoard);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 4, (void *)pVFTable[4], (void *&)m_pfnCounterStrikeViewpot_GetAllPlayersInfo);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 6, (void *)pVFTable[6], (void *&)m_pfnCounterStrikeViewpot_ShowScoreBoard);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 8, (void *)pVFTable[8], (void *&)m_pfnCounterStrikeViewpot_HideAllVGUIMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 9, (void *)pVFTable[9], (void *&)m_pfnCounterStrikeViewpot_UpdateSpectatorPanel);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 10, (void *)pVFTable[10], (void *&)m_pfnCounterStrikeViewpot_IsScoreBoardVisible);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 11, (void *)pVFTable[11], (void *&)m_pfnCounterStrikeViewpot_HideScoreBoard);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 13, (void *)pVFTable[13], (void *&)m_pfnCounterStrikeViewpot_ShowVGUIMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 14, (void *)pVFTable[14], (void *&)m_pfnCounterStrikeViewpot_HideVGUIMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 20, (void *)pVFTable[20], (void *&)m_pfnCounterStrikeViewpot_ShowSpectatorGUI);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 21, (void *)pVFTable[21], (void *&)m_pfnCounterStrikeViewpot_ShowSpectatorGUIBar);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 22, (void *)pVFTable[22], (void *&)m_pfnCounterStrikeViewpot_HideSpectatorGUI);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 23, (void *)pVFTable[23], (void *&)m_pfnCounterStrikeViewpot_DeactivateSpectatorGUI);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 24, (void *)pVFTable[24], (void *&)m_pfnCounterStrikeViewpot_IsSpectatorGUIVisible);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 25, (void *)pVFTable[25], (void *&)m_pfnCounterStrikeViewpot_IsSpectatorBarVisible);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 27, (void *)pVFTable[27], (void *&)m_pfnCounterStrikeViewpot_SetSpectatorBanner);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 28, (void *)pVFTable[28], (void *&)m_pfnCounterStrikeViewpot_SpectatorGUIEnableInsetView);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 29, (void *)pVFTable[29], (void *&)m_pfnCounterStrikeViewpot_ShowCommandMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 30, (void *)pVFTable[30], (void *&)m_pfnCounterStrikeViewpot_UpdateCommandMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 31, (void *)pVFTable[31], (void *&)m_pfnCounterStrikeViewpot_HideCommandMenu);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 32, (void *)pVFTable[32], (void *&)m_pfnCounterStrikeViewpot_IsCommandMenuVisible);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 45, (void *)pVFTable[45], (void *&)m_pfnCounterStrikeViewpot_GetViewPortScheme);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 46, (void *)pVFTable[46], (void *&)m_pfnCounterStrikeViewpot_GetViewPortPanel);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 51, (void *)pVFTable[51], (void *&)m_pfnCounterStrikeViewpot_GetSpectatorBottomBarHeight);
	g_pMetaHookAPI->VFTHook(g_pCounterStrikeViewpot, 0, 52, (void *)pVFTable[52], (void *&)m_pfnCounterStrikeViewpot_GetSpectatorTopBarHeight);

	g_pMetaHookAPI->InlineHook(m_pfnCounterStrikeViewpot_StartProgressBar, CCounterStrikeViewport_StartProgressBar, (void *&)m_pfnCounterStrikeViewpot_StartProgressBar);
	g_pMetaHookAPI->InlineHook(m_pfnCounterStrikeViewpot_UpdateProgressBar, CCounterStrikeViewport_UpdateProgressBar, (void *&)m_pfnCounterStrikeViewpot_UpdateProgressBar);
	g_pMetaHookAPI->InlineHook(m_pfnCounterStrikeViewpot_StopProgressBar, CCounterStrikeViewport_StopProgressBar, (void *&)m_pfnCounterStrikeViewpot_StopProgressBar);
}