#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include <demo_api.h>
#include "demo.h"
#include "configs.h"
#include "msghook.h"
#include "modules.h"
#include "pm_shared.h"
#include "eventscripts.h"
#include "game_shared/voice_status.h"
#include "GameUI_Interface.h"
#include "CounterStrikeViewport.h"
#include "tri.h"

hud_player_info_t g_PlayerInfoList[MAX_PLAYERS + 1];
int g_PlayerScoreAttrib[MAX_PLAYERS + 1];
TEMPENTITY *g_DeadPlayerModels[MAX_PLAYERS + 1];

extern cvar_t *sensitivity;
extern vgui::HFont font;

cvar_t *cl_lw = NULL;
cvar_t *cl_righthand = NULL;
cvar_t *cl_radartype = NULL;
cvar_t *cl_dynamiccrosshair = NULL;
cvar_t *cl_crosshair_color = NULL;
cvar_t *cl_crosshair_size = NULL;
cvar_t *cl_crosshair_translucent = NULL;
cvar_t *cl_crosshair_type = NULL;
cvar_t *cl_killeffect = NULL;
cvar_t *cl_killmessage = NULL;
cvar_t *cl_headname = NULL;
cvar_t *cl_newmenu = NULL;
cvar_t *cl_newmenu_drawbox = NULL;
cvar_t *cl_newradar = NULL;
cvar_t *cl_newradar_size = NULL;
cvar_t *cl_newradar_dark = NULL;
cvar_t *cl_newchat = NULL;
cvar_t *cl_shadows = NULL;
cvar_t *cl_scoreboard = NULL;
cvar_t *cl_fog_skybox = NULL;
cvar_t *cl_fog_density = NULL;
cvar_t *cl_fog_r = NULL;
cvar_t *cl_fog_g = NULL;
cvar_t *cl_fog_b = NULL;
cvar_t *cl_minmodel = NULL;
cvar_t *cl_min_t = NULL;
cvar_t *cl_min_ct = NULL;
cvar_t *cl_corpsestay = NULL;
cvar_t *cl_corpsefade = NULL;
cvar_t *cl_3dhud = NULL;
cvar_t *cl_autoreload = NULL;
cvar_t *hud_saytext_time;

pfnUserMsgHook g_pfnMSG_SetFOV;
pfnUserMsgHook g_pfnMSG_MOTD;
pfnUserMsgHook g_pfnMSG_ServerName;
pfnUserMsgHook g_pfnMSG_SpecHealth;
pfnUserMsgHook g_pfnMSG_SpecHealth2;
pfnUserMsgHook g_pfnMSG_ShadowIdx;
pfnUserMsgHook g_pfnMSG_GameMode;
pfnUserMsgHook g_pfnMSG_TeamInfo;
pfnUserMsgHook g_pfnMSG_HLTV;
pfnUserMsgHook g_pfnMSG_Brass;
pfnUserMsgHook g_pfnMSG_BombDrop;
pfnUserMsgHook g_pfnMSG_ResetHUD;
pfnUserMsgHook g_pfnMSG_InitHUD;
pfnUserMsgHook g_pfnMSG_ViewMode;
pfnUserMsgHook g_pfnMSG_Location;
pfnUserMsgHook g_pfnMSG_Fog;
pfnUserMsgHook g_pfnMSG_ScoreAttrib;

xcommand_t g_pfnShowCommandMenu;
xcommand_t g_pfnHideCommandMenu;
xcommand_t g_pfnLeftDown;
xcommand_t g_pfnRightDown;
xcommand_t g_pfnClientBuy;

const char *GetStringTeamColor(int i);

class CCStrikeVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if (entindex >= 0 && entindex < MAX_PLAYERS)
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;
			vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(gViewPortInterface->GetViewPortScheme());
			Color clr = pScheme->GetColor(GetStringTeamColor(iTeam), Color(255, 255, 255, 255));

			color[0] = clr.r();
			color[1] = clr.g();
			color[2] = clr.b();
		}
	}

	virtual int GetAckIconHeight(void)
	{
		return gHUD.m_iFontHeight * 3 + 6;
	}

	virtual bool CanShowSpeakerLabels(void)
	{
		return !gViewPortInterface->IsScoreBoardVisible();
	}

	virtual const char *GetPlayerLocation(int entindex)
	{
		if (entindex >= 0 && entindex < MAX_PLAYERS)
			return g_PlayerExtraInfo[entindex].location;

		return IVoiceStatusHelper::GetPlayerLocation(entindex);
	}
};

static CCStrikeVoiceStatusHelper g_VoiceStatusHelper;

CHud::~CHud(void)
{
	Shutdown();

	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV(pszName, iSize, pbuf);
}

int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf);
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_InitHUD(pszName, iSize, pbuf);
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ViewMode(pszName, iSize, pbuf);
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_MOTD(pszName, iSize, pbuf);
}

int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ServerName(pszName, iSize, pbuf);
}

int __MsgFunc_TimeLimit(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_TimeLimit(pszName, iSize, pbuf);
}

int __MsgFunc_SpecHealth(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SpecHealth(pszName, iSize, pbuf);
}

int __MsgFunc_SpecHealth2(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SpecHealth2(pszName, iSize, pbuf);
}

int __MsgFunc_ShadowIdx(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ShadowIdx(pszName, iSize, pbuf);
}

int __MsgFunc_Brass(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Brass(pszName, iSize, pbuf);
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_GameMode(pszName, iSize, pbuf);
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_TeamInfo(pszName, iSize, pbuf);
}

int __MsgFunc_HLTV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_HLTV(pszName, iSize, pbuf);
}

int __MsgFunc_BombDrop(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_BombDrop(pszName, iSize, pbuf);
}

int __MsgFunc_Location(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Location(pszName, iSize, pbuf);
}

int __MsgFunc_Fog(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Fog(pszName, iSize, pbuf);
}

int __MsgFunc_ScoreAttrib(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ScoreAttrib(pszName, iSize, pbuf);
}

void __CmdFunc_ShowCommandMenu(void)
{
	g_pfnShowCommandMenu();
}

void __CmdFunc_HideCommandMenu(void)
{
	if (gConfigs.bEnableClientUI)
		return;

	g_pfnHideCommandMenu();
}

void __CmdFunc_LeftDown(void)
{
	if (gHUD.m_iIntermission)
		return;

	g_pfnLeftDown();
}

void __CmdFunc_RightDown(void)
{
	if (gHUD.m_iIntermission)
		return;

	g_pfnRightDown();
}

void __CmdFunc_ClientBuy(void)
{
	if (gHUD.m_iIntermission)
		return;

	if (!gViewPortInterface)
		return;

	if (!gHUD.m_StatusIcons.InBuyZone() || g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].dead)
		return;

	const char *vgui_menus = gEngfuncs.LocalPlayerInfo_ValueForKey("_vgui_menus");

	if (vgui_menus && strlen(vgui_menus) && atoi(vgui_menus) == 0)
	{
		gEngfuncs.pfnServerCmd("buy\n");
	}
	else
	{
		gViewPortInterface->ShowVGUIMenu(MENU_BUY);
		gEngfuncs.pfnServerCmd("client_buy_open\n");
	}
}

void InstallCounterStrikeViewportHook(void);

void CHud::Init(void)
{
	Shutdown();

	if (gConfigs.bEnableClientUI)
		InstallCounterStrikeViewportHook();

	g_pfnShowCommandMenu = HOOK_COMMAND("+commandmenu", ShowCommandMenu);
	g_pfnHideCommandMenu = HOOK_COMMAND("-commandmenu", HideCommandMenu);
	g_pfnLeftDown = HOOK_COMMAND("+left", LeftDown);
	g_pfnRightDown = HOOK_COMMAND("+right", RightDown);
	g_pfnClientBuy = HOOK_COMMAND("buy", ClientBuy);

	g_pfnMSG_SetFOV = HOOK_MESSAGE(SetFOV);
	g_pfnMSG_ResetHUD = HOOK_MESSAGE(ResetHUD);
	g_pfnMSG_InitHUD = HOOK_MESSAGE(InitHUD);
	g_pfnMSG_ViewMode = HOOK_MESSAGE(ViewMode);
	g_pfnMSG_Location = HOOK_MESSAGE(Location);
	g_pfnMSG_MOTD = HOOK_MESSAGE(MOTD);
	g_pfnMSG_ServerName = HOOK_MESSAGE(ServerName);
	g_pfnMSG_SpecHealth = HOOK_MESSAGE(SpecHealth);
	g_pfnMSG_SpecHealth2 = HOOK_MESSAGE(SpecHealth2);
	g_pfnMSG_ShadowIdx = HOOK_MESSAGE(ShadowIdx);
	g_pfnMSG_GameMode = HOOK_MESSAGE(GameMode);
	g_pfnMSG_TeamInfo = HOOK_MESSAGE(TeamInfo);
	g_pfnMSG_HLTV = HOOK_MESSAGE(HLTV);
	g_pfnMSG_Brass = HOOK_MESSAGE(Brass);
	g_pfnMSG_BombDrop = HOOK_MESSAGE(BombDrop);
	g_pfnMSG_Fog = HOOK_MESSAGE(Fog);
	g_pfnMSG_ScoreAttrib = HOOK_MESSAGE(ScoreAttrib);

	HOOK_MESSAGE(TimeLimit);

	m_pSpriteList = NULL;
	m_iPlayerNum = 0;
	m_flTime = 1;
	m_iFOV = 0;

	m_Ammo.Init();
	m_Health.Init();
	m_DeathNotice.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_NightVision.Init();
	m_TextMessage.Init();
	m_Menu.Init();
	m_roundTimer.Init();
	m_accountBalance.Init();
	m_Battery.Init();
	m_itemStatus.Init();
	m_headName.Init();
	m_Radar.Init();
	m_ThirdPerson.Init();
	m_GraphicMenu.Init();
	m_ViewModel.Init();
	m_Message.Init();
	m_Scoreboard.Init();
	m_StatusIcons.Init();
	m_scenarioStatus.Init();
	m_progressBar.Init();
	m_Corpse.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Flash.Init();
	m_StatusBar.Init();
	m_VGUI2Print.Init();

	GetClientVoice()->Init(&g_VoiceStatusHelper);

	m_pCvarDraw = gEngfuncs.pfnGetCvarPointer("hud_draw");

	if (!m_pCvarDraw)
		m_pCvarDraw = gEngfuncs.pfnRegisterVariable("hud_draw", "1", FCVAR_ARCHIVE);

	cl_lw = gEngfuncs.pfnGetCvarPointer("cl_lw");
	cl_righthand = gEngfuncs.pfnGetCvarPointer("cl_righthand");
	cl_minmodel = gEngfuncs.pfnGetCvarPointer("cl_minmodel");
	cl_min_t = gEngfuncs.pfnGetCvarPointer("cl_min_t");
	cl_min_ct = gEngfuncs.pfnGetCvarPointer("cl_min_ct");
	cl_corpsestay = gEngfuncs.pfnGetCvarPointer("cl_corpsestay");
	cl_corpsefade = gEngfuncs.pfnRegisterVariable("cl_corpsefade", "0", FCVAR_ARCHIVE);
	cl_crosshair_type = gEngfuncs.pfnRegisterVariable("cl_crosshair_type", "0", FCVAR_ARCHIVE);
	cl_killeffect = gEngfuncs.pfnRegisterVariable("cl_killeffect", "1", FCVAR_ARCHIVE);
	cl_killmessage = gEngfuncs.pfnRegisterVariable("cl_killmessage", "1", FCVAR_ARCHIVE);
	cl_headname = gEngfuncs.pfnRegisterVariable("cl_headname", "1", FCVAR_ARCHIVE);
	cl_newmenu = gEngfuncs.pfnRegisterVariable("cl_newmenu", "1", FCVAR_ARCHIVE);
	cl_newmenu_drawbox = gEngfuncs.pfnRegisterVariable("cl_newmenu_drawbox", "1", FCVAR_ARCHIVE);
	cl_newradar = gEngfuncs.pfnRegisterVariable("cl_newradar", "1", FCVAR_ARCHIVE);
	cl_newradar_size = gEngfuncs.pfnRegisterVariable("cl_newradar_size", "0.175", FCVAR_ARCHIVE);
	cl_newradar_dark = gEngfuncs.pfnRegisterVariable("cl_newradar_dark", "0.8", FCVAR_ARCHIVE);
	cl_newchat = gEngfuncs.pfnRegisterVariable("cl_newchat", "1", FCVAR_ARCHIVE);
	cl_scoreboard = gEngfuncs.pfnRegisterVariable("cl_scoreboard", "1", FCVAR_ARCHIVE);
	cl_3dhud = gEngfuncs.pfnRegisterVariable("cl_3dhud", "1", FCVAR_ARCHIVE);
	cl_autoreload = gEngfuncs.pfnRegisterVariable("_cl_autoreload", "1", FCVAR_ARCHIVE | FCVAR_USERINFO);

	if (!cl_righthand)
		cl_righthand = gEngfuncs.pfnRegisterVariable("cl_righthand", "1", FCVAR_ARCHIVE);

	gEngfuncs.pfnGetCvarFloat("developer");

	if (gEngfuncs.pfnGetCvarFloat("developer") > 0.0)
	{
		cl_fog_density = gEngfuncs.pfnGetCvarPointer("cl_fog_density");
		cl_fog_r = gEngfuncs.pfnGetCvarPointer("cl_fog_r");
		cl_fog_g = gEngfuncs.pfnGetCvarPointer("cl_fog_g");
		cl_fog_b = gEngfuncs.pfnGetCvarPointer("cl_fog_b");

		if (!cl_fog_density)
			cl_fog_density = gEngfuncs.pfnRegisterVariable("cl_fog_density", "0", 0);

		if (!cl_fog_r)
			cl_fog_r = gEngfuncs.pfnRegisterVariable("cl_fog_r", "0", 0);

		if (!cl_fog_g)
			cl_fog_g = gEngfuncs.pfnRegisterVariable("cl_fog_g", "0", 0);

		if (!cl_fog_b)
			cl_fog_b = gEngfuncs.pfnRegisterVariable("cl_fog_b", "0", 0);

		cl_fog_skybox = gEngfuncs.pfnRegisterVariable("cl_fog_skybox", "0", 0);
	}

	default_fov = gEngfuncs.pfnGetCvarPointer("default_fov");

	if (gConfigs.bEnableClientUI)
		g_pViewPort->Init();
}

void CHud::Shutdown(void)
{
	if (m_pHudList)
	{
		HUDLIST *pList;

		while (m_pHudList)
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free(pList);
		}

		m_pHudList = NULL;
	}
}

void CHud::VidInit(void)
{
	hud_saytext_time = gEngfuncs.pfnGetCvarPointer("hud_saytext_time");

	cl_dynamiccrosshair = gEngfuncs.pfnGetCvarPointer("cl_dynamiccrosshair");
	cl_radartype = gEngfuncs.pfnGetCvarPointer("cl_radartype");
	cl_crosshair_color = gEngfuncs.pfnGetCvarPointer("cl_crosshair_color");
	cl_crosshair_size = gEngfuncs.pfnGetCvarPointer("cl_crosshair_size");
	cl_crosshair_translucent = gEngfuncs.pfnGetCvarPointer("cl_crosshair_translucent");
	cl_shadows = gEngfuncs.pfnGetCvarPointer("cl_shadows");

	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	if (!m_pSpriteList)
	{
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;

			for (int j = 0; j < m_iSpriteCountAllRes; j++)
			{
				if (p->iRes == m_iRes)
					m_iSpriteCount++;

				p++;
			}

			m_rghSprites = new HSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];
			p = m_pSpriteList;

			int index = 0;

			for (int j = 0; j < m_iSpriteCountAllRes; j++)
			{
				if (p->iRes == m_iRes)
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rghSprites[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy(&m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH);
					index++;
				}

				p++;
			}
		}
	}
	else
	{
		client_sprite_t *p = m_pSpriteList;
		int index = 0;

		for (int j = 0; j < m_iSpriteCountAllRes; j++)
		{
			if (p->iRes == m_iRes)
			{
				char sz[256];
				sprintf(sz, "sprites/%s.spr", p->szSprite);
				m_rghSprites[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	m_iPlayerNum = 0;
	m_szGameMode[0] = '\0';
	m_HUD_number_0 = GetSpriteIndex("number_0");
	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;
	m_iIntermission = 0;
	m_szMOTD[0] = 0;
	m_flTimeLeft = 0;

	m_Ammo.VidInit();
	m_Health.VidInit();
	m_DeathNotice.VidInit();
	m_Spectator.VidInit();
	m_NightVision.VidInit();
	m_SayText.VidInit();
	m_TextMessage.VidInit();
	m_Menu.VidInit();
	m_roundTimer.VidInit();
	m_accountBalance.VidInit();
	m_Battery.VidInit();
	m_itemStatus.VidInit();
	m_headName.VidInit();
	m_Radar.VidInit();
	m_ThirdPerson.VidInit();
	m_GraphicMenu.VidInit();
	m_ViewModel.VidInit();
	m_Message.VidInit();
	m_Scoreboard.VidInit();
	m_StatusIcons.VidInit();
	m_scenarioStatus.VidInit();
	m_progressBar.VidInit();
	m_Corpse.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Flash.VidInit();
	m_StatusBar.VidInit();
	m_VGUI2Print.VidInit();
	GetClientVoiceHud()->VidInit();

	m_iFontEngineHeight = vgui::surface()->GetFontTall(font);

	if (gConfigs.bEnableClientUI)
		g_pViewPort->VidInit();
}

int CHud::Redraw(float flTime, int intermission)
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	for (int i = 1; i <= MAX_CLIENTS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);

		if (g_PlayerInfoList[i].thisplayer)
			m_iPlayerNum = i;
	}

	m_fOldTime = m_flTime;
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;

	if (m_flTimeDelta < 0)
		m_flTimeDelta = 0;

	if (gConfigs.bEnableClientUI)
	{
		if (m_iIntermission && !intermission)
		{
			m_iIntermission = intermission;

			g_pViewPort->HideAllVGUIMenu();
			g_pViewPort->UpdateSpectatorPanel();
		}
		else if (!m_iIntermission && intermission)
		{
			m_iIntermission = intermission;

			g_pViewPort->HideAllVGUIMenu();
			g_pViewPort->ShowScoreBoard();
			g_pViewPort->UpdateSpectatorPanel();
		}
	}

	m_iIntermission = intermission;

	if (m_pCvarDraw->value)
	{
		HUDLIST *pList = m_pHudList;

		while (pList)
		{
			if (!intermission)
			{
				if ((pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL))
					pList->p->Draw(flTime);
			}
			else
			{
				if (pList->p->m_iFlags & HUD_INTERMISSION)
					pList->p->Draw(flTime);
			}

			pList = pList->pNext;
		}
	}

	if (gConfigs.bEnableClientUI)
		g_pViewPort->SetPaintEnabled(m_pCvarDraw->value);

	return 1;
}

float g_lastFOV = 0.0;

float HUD_GetFOV(void)
{
	if (gEngfuncs.pDemoAPI->IsPlayingback())
		g_lastFOV = g_demozoom;

	return g_lastFOV;
}

void CHud::Think(void)
{
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if (pList->p->m_iFlags & HUD_ACTIVE)
			pList->p->Think();

		pList = pList->pNext;
	}

	int newfov = HUD_GetFOV();

	if (newfov == 0)
		m_iFOV = default_fov->value;
	else
		m_iFOV = newfov;

	if (m_iFOV == default_fov->value)
		m_flMouseSensitivity = 0;
	else
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)default_fov->value) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");

	if (m_iFOV == 0)
		m_iFOV = max(default_fov->value, 90);

	if (gEngfuncs.IsSpectateOnly())
	{
		if (g_iUser1 == OBS_IN_EYE && g_iUser2 != 0)
		{
			if (g_iUser2 <= gEngfuncs.GetMaxClients())
				m_iFOV = m_PlayerFOV[g_iUser2];
			else
				m_iFOV = max(default_fov->value, 90);
		}
		else
			m_iFOV = m_Spectator.GetFOV();
	}
}

int CHud::MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if (gEngfuncs.pDemoAPI->IsPlayingback())
		return 1;

	BEGIN_READ(pbuf, iSize);

	bool iGotAllMOTD = READ_BYTE();
	strcat(m_szMOTD, READ_STRING());

	if (gConfigs.bEnableClientUI)
	{
		if (iGotAllMOTD)
		{
			g_pViewPort->ShowMOTD(g_pViewPort->GetServerName(), m_szMOTD);
			m_szMOTD[0] = 0;
			return 1;
		}
	}
	else
	{
		if (iGotAllMOTD)
		{
			m_szMOTD[0] = 0;
		}
	}

	return g_pfnMSG_MOTD(pszName, iSize, pbuf);
}

int CHud::MsgFunc_Fog(const char *pszName, int iSize, void *pbuf)
{
	int a, b, c, d;

	g_FogParameters.density = 0;
	g_FogParameters.affectsSkyBox = false;
	g_FogParameters.color[0] = 0;
	g_FogParameters.color[1] = 0;
	g_FogParameters.color[2] = 0;

	BEGIN_READ(pbuf, iSize);

	g_FogParameters.color[0] = READ_BYTE();
	g_FogParameters.color[1] = READ_BYTE();
	g_FogParameters.color[2] = READ_BYTE();

	a = READ_BYTE();
	b = READ_BYTE();
	c = READ_BYTE();
	d = READ_BYTE();

	union
	{
		unsigned char a, b, c, d;
		float v;
	}
	dat;

	dat.a = a;
	dat.b = b;
	dat.c = c;
	dat.d = d;

	if (READ_OK())
	{
		g_FogParameters.density = dat.v;
	}
	else
	{
		g_FogParameters.color[0] = 0;
		g_FogParameters.color[1] = 0;
		g_FogParameters.color[2] = 0;
		g_FogParameters.density = 0;
		g_FogParameters.affectsSkyBox = false;
	}

	if (iSize > 7)
		g_FogParameters.affectsSkyBox = READ_BYTE();

	if (cl_fog_skybox)
		gEngfuncs.Cvar_SetValue(cl_fog_skybox->name, g_FogParameters.affectsSkyBox);

	if (cl_fog_density)
		gEngfuncs.Cvar_SetValue(cl_fog_density->name, g_FogParameters.density);

	if (cl_fog_r)
		gEngfuncs.Cvar_SetValue(cl_fog_r->name, g_FogParameters.color[0]);

	if (cl_fog_g)
		gEngfuncs.Cvar_SetValue(cl_fog_g->name, g_FogParameters.color[1]);

	if (cl_fog_b)
		gEngfuncs.Cvar_SetValue(cl_fog_b->name, g_FogParameters.color[2]);

	return 1;
}

int CHud::MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *servername = READ_STRING();

	if (gConfigs.bEnableClientUI)
	{
		g_pViewPort->SetServerName(servername);
	}

	return g_pfnMSG_ServerName(pszName, iSize, pbuf);
}

int CHud::MsgFunc_TimeLimit(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_flTimeLeft = gHUD.m_flTime + READ_SHORT();
	return 1;
}

int CHud::MsgFunc_SpecHealth(const char *pszName, int iSize, void *pbuf)
{
	if (gConfigs.bEnableClientUI)
	{
		BEGIN_READ(pbuf, iSize);

		gHUD.m_Radar.m_iPlayerLastPointedAt = g_iUser2;
		int health = READ_BYTE();
		g_PlayerExtraInfo[g_iUser2].health = health;
		gCKFVars.g_PlayerInfo[g_iUser2].iHealth = health;
		g_pViewPort->UpdateSpectatorGUI();
		return 1;
	}

	return g_pfnMSG_SpecHealth(pszName, iSize, pbuf);
}

int CHud::MsgFunc_SpecHealth2(const char *pszName, int iSize, void *pbuf)
{
	if (gConfigs.bEnableClientUI)
	{
		BEGIN_READ(pbuf, iSize);

		int target = READ_BYTE();
		int health = READ_BYTE();		

		gHUD.m_Radar.m_iPlayerLastPointedAt = g_iUser2;
		g_PlayerExtraInfo[target].health = health;
		gCKFVars.g_PlayerInfo[target].iHealth = health;
		g_pViewPort->UpdateSpectatorGUI();
		return 1;
	}

	return g_pfnMSG_SpecHealth2(pszName, iSize, pbuf);
}

void StudioSetShadowSprite(int iSprite);

int CHud::MsgFunc_ShadowIdx(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iSprite = READ_LONG();

	StudioSetShadowSprite(iSprite);
	return 1;
}

int CHud::MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	strcpy(m_szGameMode, READ_STRING());
	return 1;
}

char *GetTeam(int teamNo)
{
	switch (teamNo)
	{
		case TEAM_TERRORIST: return "TERRORIST";
		case TEAM_CT: return "CT";
		case TEAM_SPECTATOR: return "SPECTATOR";
	}

	return "";
}

int GetTeamIndex(const char *teamName)
{
	if (!strcmp(teamName, "TERRORIST"))
		return TEAM_TERRORIST;

	if (!strcmp(teamName, "CT"))
		return TEAM_CT;

	if (!strcmp(teamName, "SPECTATOR"))
		return TEAM_SPECTATOR;

	return TEAM_UNASSIGNED;
}

int CHud::MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client = READ_BYTE();

	if (iSize == 2)
	{
		int teamId = READ_BYTE();

		MSG_BeginWrite();
		MSG_WriteByte(client);
		MSG_WriteString(GetTeam(teamId));
		MSG_EndWrite(pszName, g_pfnMSG_TeamInfo);
		return 1;
	}

	return g_pfnMSG_TeamInfo(pszName, iSize, pbuf);
}

int CHud::MsgFunc_HLTV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int param1 = READ_BYTE();
	int param2 = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
	{
		if (!(param2 & 0x80))
		{
			int client = param1;
			int fov = param2;

			if (fov == 0 && client != 0)
			{
				fov = max(default_fov->value, 90);
			}

			if (client == 0)
			{
				for (int i = 1; i < MAX_PLAYERS; i++)
					m_PlayerFOV[i] = fov;
			}
			else
			{
				m_PlayerFOV[client] = fov;
			}
		}
	}

	return g_pfnMSG_HLTV(pszName, iSize, pbuf);
}

int CHud::MsgFunc_BombDrop(const char *pszName, int iSize, void *pbuf)
{
	gHUD.m_bShowTimer = false;

	return g_pfnMSG_BombDrop(pszName, iSize, pbuf);
}

int CHud::MsgFunc_Brass(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	vec3_t origin, velocity, left;
	float rotation;
	int model;
	int soundType, entityIndex;
	float cosRotation, sinRotation;
	int righthand;
	int isLocal;

	READ_BYTE();

	origin.x = READ_COORD();
	origin.y = READ_COORD();
	origin.z = READ_COORD();

	left.x = READ_COORD();
	left.y = READ_COORD();
	left.z = READ_COORD();

	velocity.x = READ_COORD();
	velocity.y = READ_COORD();
	velocity.z = READ_COORD();

	rotation = READ_ANGLE();
	model = READ_SHORT();
	soundType = READ_BYTE();
	entityIndex = READ_BYTE();

	rotation = (M_PI * rotation) / 180.0;
	cosRotation = cos(rotation);
	sinRotation = sin(rotation);

	righthand = gEngfuncs.pfnGetCvarFloat("cl_righthand");
	isLocal = EV_IsLocal(entityIndex);

	if (!righthand && isLocal)
	{
		origin = origin + Vector((sinRotation * -9.0), (cosRotation * 9.0), 0.0);
	}
	else
	{
		velocity.x += sinRotation * -120.0;
		velocity.y += cosRotation * 120.0;

		origin = origin + Vector(-(sinRotation * -9.0), -(cosRotation * 9.0), 0.0);
	}

	EV_EjectBrass(origin, velocity, rotation, model, soundType, 0, 1);
	return 1;
}

int CHud::MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT("default_fov");

	g_lastFOV = newfov;

	if (newfov == 0)
		m_iFOV = def_fov;
	else
		m_iFOV = newfov;

	if (m_iFOV == def_fov)
		m_flMouseSensitivity = 0;
	else
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");

	return g_pfnMSG_SetFOV(pszName, iSize, pbuf);
}

int CHud::MsgFunc_Location(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int cl = READ_BYTE();

	if (cl >= 1 && cl <= MAX_CLIENTS)
	{
		strncpy(g_PlayerExtraInfo[cl].location, READ_STRING(), sizeof(g_PlayerExtraInfo[cl].location));
		GetClientVoiceHud()->UpdateLocation(cl, g_PlayerExtraInfo[cl].location);
	}

	return 1;
}

int CHud::MsgFunc_ScoreAttrib(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int cl = READ_BYTE();
	int attrib = READ_BYTE();

	if (cl >= 1 && cl <= MAX_CLIENTS)
		g_PlayerScoreAttrib[cl] = attrib;

	return g_pfnMSG_ScoreAttrib(pszName, iSize, pbuf);
}

int CHud::GetSpriteIndex(const char *SpriteName)
{
	for (int i = 0; i < m_iSpriteCount; i++)
	{
		if (strncmp(SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH) == 0)
			return i;
	}

	return -1;
}

int CHud::DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b)
{
	xpos += gEngfuncs.pfnDrawString(xpos, ypos, szIt, r, g, b);
	return xpos;
}

int CHud::DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b)
{
	char szString[32];
	sprintf(szString, "%d", iNumber);
	return DrawHudStringReverse(xpos, ypos, iMinX, szString, r, g, b);
}

int CHud::DrawHudStringReverse(int xpos, int ypos, int iMinX, char *szString, int r, int g, int b)
{
	xpos -= gEngfuncs.pfnDrawStringReverse(xpos, ypos, szString, r, g, b);
	return xpos;
}

int CHud::DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		if (iNumber >= 10000)
		{
			k = iNumber / 10000;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_5DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iNumber >= 1000)
		{
			k = (iNumber % 10000) / 1000;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iNumber >= 100)
		{
			k = (iNumber % 1000) / 100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	}
	else if (iFlags & DHN_DRAWZERO)
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b);

		if (iFlags & (DHN_5DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
		{
			if (iFlags & DHN_FILLZERO)
				SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));

			x += iWidth;
		}

		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}

int CHud::DrawHudNumber(int x, int y, int iNumber, int r, int g, int b)
{
	static char szBuffer[16];
	const char *pszPosint;
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	sprintf(szBuffer, "%5i", iNumber);
	pszPosint = szBuffer;

	if (iNumber < 0)
		pszPosint++;

	while (*pszPosint)
	{
		k = *pszPosint - '0';

		if (k >= 0 && k <= 9)
		{
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
		}

		x += iWidth;
		pszPosint++;
	}

	return x;
}

int CHud::GetNumWidth(int iNumber, int iFlags)
{
	if (iFlags & (DHN_5DIGITS))
		return 5;

	if (iFlags & (DHN_4DIGITS))
		return 4;

	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	if (iNumber < 1000)
		return 3;

	if (iNumber < 10000)
		return 4;

	return 5;
}

int CHud::GetNumBits(int iNumber)
{
	int k = iNumber;
	int bits = 0;

	while (k)
	{
		k /= 10;
		bits++;
	}

	return bits;
}

void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));

	if (!pdl)
		return;

	pdl->p = phudelem;
	pdl->pNext = NULL;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity(void)
{
	return m_flMouseSensitivity;
}

void CHud::VGUI2HudPrint(char *charMsg, int x, int y, float r, float g, float b)
{
	m_VGUI2Print.VGUI2HudPrint(charMsg, x, y, r, g, b);
}

bool CHud::IsHidden( int iHudFlags )
{
	// Not in game?
	if ( !GameUI().IsInLevel() )
		return true;

	// Get current hidden flags
	int iHideHud = m_iHideHUDDisplay;

	// Everything hidden?
	if ( iHideHud & HIDEHUD_ALL )
		return true;

	return ( ( iHudFlags & iHideHud ) != 0);
}

int BTE_GetHUDFov(void)
{
	return gHUD.m_iFOV;
}