#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "monsters.h"
#include "pm_shared.h"
#include "enghack.h"
#include <string>

extern DLL_GLOBAL int g_fTraceLineIgnore;
extern DLL_GLOBAL int g_fIsTraceLine;
//extern DLL_GLOBAL ULONG g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL g_fGameOver;
extern DLL_GLOBAL int g_iSkillLevel;
extern DLL_GLOBAL ULONG g_ulFrameCount;

extern void CopyToBodyQue(entvars_t *pev);
extern int giPrecacheGrunt;
extern int gmsgSayText;
extern std::string g_MapInfoText;

void LinkUserMessages(void);

void set_suicide_frame(entvars_t *pev)
{
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return;

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;
	pev->deadflag = DEAD_DEAD;
	pev->nextthink = -1;
}

BOOL ClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128])
{
	return g_pGameRules->ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void ClientDisconnect(edict_t *pEntity)
{
	if (g_fGameOver)
		return;

	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_disconnected", STRING(pEntity->v.netname));
	CSound *pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(pEntity));

	if (pSound)
		pSound->Reset();

	pEntity->v.takedamage = DAMAGE_NO;
	pEntity->v.solid = SOLID_NOT;
	pEntity->v.flags = FL_DORMANT;
	CBasePlayer *pl = (CBasePlayer *)CBaseEntity::Instance(pEntity);

	if (pl)
	{
		pl->SetThink(NULL);
		pl->PlayerDisconnect();
	}

	UTIL_SetOrigin(&pEntity->v, pEntity->v.origin);
	g_pGameRules->ClientDisconnected(pEntity);
}

void respawn(entvars_t *pev, BOOL fCopyCorpse)
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if (fCopyCorpse)
			CopyToBodyQue(pev);

		GetClassPtr((CBasePlayer *)pev)->Spawn();
	}
	else if (pev->deadflag > DEAD_NO)
		SERVER_COMMAND("reload\n");
}

void ClientKill(edict_t *pEntity)
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance(pev);

	if (pl->IsObserver())
		return;

	if (pl->m_iJoiningState != JOINED)
		return;

	if (gpGlobals->time >= pl->m_fNextSuicideTime)
	{
		pl->m_LastHitGroup = 0;
		pl->m_fNextSuicideTime = gpGlobals->time + 1;
		pEntity->v.health = 0;
		pl->pev->health = 0;
		pl->Killed(pev, GIB_NEVER);
	}
}

void ShowMenu(CBasePlayer *pPlayer, int bitsKey, int time, int part, char *string)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pPlayer->pev);
	WRITE_SHORT(bitsKey);
	WRITE_CHAR(time);
	WRITE_BYTE(part);
	WRITE_STRING(string);
	MESSAGE_END();
}

void ShowVGUIMenu(CBasePlayer *pPlayer, int type, int bitsKey, char *string)
{
	if (pPlayer->m_bVGUIMenus == TRUE)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgVGUIMenu, NULL, pPlayer->pev);
		WRITE_BYTE(type);
		WRITE_SHORT(bitsKey);
		WRITE_CHAR(-1);
		WRITE_BYTE(0);
		WRITE_STRING(" ");
		MESSAGE_END();
	}
	else
		ShowMenu(pPlayer, bitsKey, -1, 0, string);

	pPlayer->m_iMenu = type;
}

void ShowHudMenu(CBasePlayer *pPlayer, int type, int keys, int skiplocal)
{
#ifdef CLIENT_WEAPONS
	if (skiplocal && ENGINE_CANSKIP(ENT(pPlayer->pev)))
		return;
#endif

	MESSAGE_BEGIN(MSG_ONE, gmsgHudMenu, NULL, pPlayer->pev);
	WRITE_BYTE(type);
	WRITE_SHORT(keys);
	MESSAGE_END();

	pPlayer->m_iHudMenu = type;
}

void ShowMGUIMenu(CBasePlayer *pPlayer, int type, int keys)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgMGUIMenu, NULL, pPlayer->pev);
	WRITE_BYTE(type);
	WRITE_SHORT(keys);
	MESSAGE_END();

	pPlayer->m_iMenu = type;
}

void SendMGUIPrint(CBasePlayer *pPlayer, int buftype, const char *text)
{
	char *p = (char *)text;
	int start = 0;

	#define MENUBUF_LEN 51
	char buf[MENUBUF_LEN + 1];

	while (strlen(p) >= MENUBUF_LEN - 1)
	{
		strncpy(buf, p, MENUBUF_LEN - 1);
		buf[MENUBUF_LEN - 1] = '\0';
		p += MENUBUF_LEN - 1;

		MESSAGE_BEGIN(MSG_ONE, gmsgMGUIPrint, NULL, pPlayer->pev);
		WRITE_BYTE(buftype);
		WRITE_BYTE(MESSAGEPART_START);		
		WRITE_STRING(buf);
		MESSAGE_END();
		start = MESSAGEPART_MID;
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgMGUIPrint, NULL, pPlayer->pev);
	WRITE_BYTE(buftype);
	WRITE_BYTE(MESSAGEPART_END);
	WRITE_STRING(p);
	MESSAGE_END();
}

void SendRespawnTime(CBasePlayer *pPlayer, int time)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgRespawnTime, NULL, pPlayer->pev);
	WRITE_BYTE(time);
	MESSAGE_END();
}

void ListPlayers(CBasePlayer *pPlayer)
{
	char buffer[120], userid[12];
	strcpy(buffer, "");
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (FBitSet(pEntity->pev->flags, FL_DORMANT))
			continue;

		CBasePlayer *pOther = GetClassPtr((CBasePlayer *)pEntity->pev);
		itoa(GETPLAYERUSERID(ENT(pOther->pev)), userid, 10);
		strcpy(buffer, "\n");
		strcat(buffer, userid);
		strcat(buffer, " : ");
		strcat(buffer, STRING(pOther->pev->netname));
		ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, buffer);
	}

	ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "\n");
}

int CountTeamPlayers(int team)
{
	int count = 0;
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (pEntity->pev->flags & FL_DORMANT)
			continue;

		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pPlayer->m_iTeam == team)
			count++;
	}

	return count;
}

void ProcessKickVote(CBasePlayer *pPlayer, CBasePlayer *pTarget)
{
	if (!pPlayer || !pTarget)
		return;

	int teamcount = CountTeamPlayers(pPlayer->m_iTeam);

	if (teamcount < 3)
		return;

	int votes = 0;
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pOther = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (!pOther || pOther->m_iTeam == TEAM_UNASSIGNED)
			continue;

		if (pOther->m_iTeam == pPlayer->m_iTeam && pOther->m_iVotingKikedPlayerId == pPlayer->m_iVotingKikedPlayerId)
			votes++;
	}

	if (kick_percent.value < 0)
		CVAR_SET_STRING("mp_kickpercent", "0.0");
	else if (kick_percent.value > 1)
		CVAR_SET_STRING("mp_kickpercent", "1.0");

	if (votes < (int)(teamcount * kick_percent.value + 0.5))
		return;

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Game_kicked", STRING(pTarget->pev->netname));
	SERVER_COMMAND(UTIL_VarArgs("kick # %d\n", pPlayer->m_iVotingKikedPlayerId));
	pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pOther = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (!pOther || pOther->m_iTeam == TEAM_UNASSIGNED)
			continue;

		if (pOther->m_iTeam == pPlayer->m_iTeam && pOther->m_iVotingKikedPlayerId == pPlayer->m_iVotingKikedPlayerId)
			pOther->m_iVotingKikedPlayerId = 0;
	}
}

int SelectDefaultTeam(void)
{
	int team = TEAM_UNASSIGNED;

	if (g_pGameRules->m_iNumTerrorist < g_pGameRules->m_iNumCT)
		team = TEAM_TERRORIST;
	else if (g_pGameRules->m_iNumTerrorist > g_pGameRules->m_iNumCT)
		team = TEAM_CT;
	else if (g_pGameRules->m_iNumCTWins > g_pGameRules->m_iNumTerroristWins)
		team = TEAM_TERRORIST;
	else if (g_pGameRules->m_iNumCTWins < g_pGameRules->m_iNumTerroristWins)
		team = TEAM_CT;
	else
		team = RANDOM_LONG(0, 1) ? TEAM_TERRORIST : TEAM_CT;

	if (g_pGameRules->TeamFull(team))
	{
		if (team == TEAM_TERRORIST)
			team = TEAM_CT;
		else
			team = TEAM_TERRORIST;

		if (g_pGameRules->TeamFull(team))
			return TEAM_UNASSIGNED;
	}

	return team;
}

void CheckStartMoney(void)
{
	if ((int)startmoney.value > 16000)
		CVAR_SET_FLOAT("mp_startmoney", 16000);
	else if ((int)startmoney.value < 800)
		CVAR_SET_FLOAT("mp_startmoney", 800);
}

void ClientPutInServer(edict_t *pEntity)
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);
	pPlayer->SetCustomDecalFrames(-1);
	pPlayer->SetPrefsFromUserinfo(g_engfuncs.pfnGetInfoKeyBuffer(pEntity));

	if (!g_pGameRules->IsMultiplayer())
	{
		pPlayer->Spawn();
		return;
	}

	pPlayer->m_bNotKilled = TRUE;
	pPlayer->m_iIgnoreMessage = IGNOREMSG_NONE;
	pPlayer->m_iTeamKills = 0;
	pPlayer->m_bJustConnected = TRUE;
	pPlayer->Spawn();
	pPlayer->m_bTeamChanged = FALSE;
	pPlayer->m_iNumSpawns = 0;

	CheckStartMoney();

	pPlayer->m_iAccount = (int)startmoney.value;
	pPlayer->m_fGameHUDInitialized = FALSE;
	pPlayer->m_flDisplayHistory &= ~DHF_ROUND_STARTED;
	pPlayer->pev->flags |= FL_SPECTATOR;
	pPlayer->pev->solid = SOLID_NOT;
	pPlayer->pev->movetype = MOVETYPE_NOCLIP;
	pPlayer->pev->effects = EF_NODRAW;
	pPlayer->pev->effects |= EF_NOINTERP;
	pPlayer->pev->takedamage = DAMAGE_NO;
	pPlayer->pev->deadflag = DEAD_DEAD;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->m_iJoiningState = READINGLTEXT;
	pPlayer->m_iTeam = TEAM_UNASSIGNED;
	pPlayer->pev->fixangle = TRUE;
	pPlayer->m_bHasNightVision = FALSE;
	pPlayer->m_iHostagesKilled = 0;
	pPlayer->m_iVotingMapId = 0;
	pPlayer->m_iVotingKikedPlayerId = 0;
	pPlayer->m_fDeadTime = 0;
	pPlayer->m_bDisconnect = FALSE;
	pPlayer->m_iMenu = 0;

	pPlayer->m_iNewClass = pPlayer->m_iClass = 0;
	pPlayer->m_Stats.iAssist = 0;
	pPlayer->m_Stats.iBackstab = 0;
	pPlayer->m_Stats.iBonus = 0;
	pPlayer->m_Stats.iCapture = 0;
	pPlayer->m_Stats.iDefence = 0;
	pPlayer->m_Stats.iDemolish = 0;
	pPlayer->m_Stats.iHeadshot = 0;
	pPlayer->m_Stats.iHealing = 0;
	pPlayer->m_Stats.iRevenge = 0;
	pPlayer->m_Stats.iTeleport = 0;
	pPlayer->m_Stats.iUbercharge = 0;
	pPlayer->m_Stats.iKill = 0;
	pPlayer->m_Stats.iDeath = 0;
	pPlayer->m_Stats.iAssist = 0;
	pPlayer->pev->frags = 0;
	pPlayer->m_iDominates = 0;
	memset(&pPlayer->m_Dominate, 0, sizeof(pPlayer->m_Dominate));

	g_engfuncs.pfnSetClientMaxspeed(ENT(pPlayer->pev), 1);
	SET_MODEL(ENT(pPlayer->pev), "models/player.mdl");
	pPlayer->SetThink(NULL);

	CBaseEntity *pTarget = UTIL_FindEntityByClassname(NULL, "trigger_camera");
	pPlayer->m_pIntroCamera = pTarget;

	if (g_pGameRules && g_pGameRules->m_iMapHasIntroCamera == 2)
		g_pGameRules->m_iMapHasIntroCamera = pPlayer->m_pIntroCamera != NULL;

	if (pPlayer->m_pIntroCamera)
		pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pPlayer->m_pIntroCamera->pev->target));

	if (pPlayer->m_pIntroCamera && pTarget)
	{
		Vector vecDir = (pTarget->pev->origin - pPlayer->m_pIntroCamera->pev->origin).Normalize();
		Vector vecGoal = UTIL_VecToAngles(vecDir);
		vecGoal.x = -vecGoal.x;

		UTIL_SetOrigin(pPlayer->pev, pPlayer->m_pIntroCamera->pev->origin);

		pPlayer->pev->angles = vecGoal;
		pPlayer->pev->v_angle = pev->angles;
		pPlayer->m_fIntroCamTime = gpGlobals->time + 6;
		pPlayer->pev->view_ofs = g_vecZero;
	}
	else
	{
		pPlayer->m_iTeam = TEAM_CT;

		if (g_pGameRules)
			g_pGameRules->GetPlayerSpawnSpot(pPlayer);

		pPlayer->m_iTeam = TEAM_UNASSIGNED;
		pPlayer->pev->v_angle = g_vecZero;
		pPlayer->pev->angles = gpGlobals->v_forward;
	}

	pPlayer->m_iJoiningState = SHOWLTEXT;

	static char sName[128];
	strcpy(sName, STRING(pPlayer->pev->netname));

	for (char *pApersand = sName; pApersand && *pApersand; pApersand++)
	{
		if (*pApersand == '%')
			*pApersand = ' ';
	}

	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_connected", sName[0] ? sName : "<unconnected>");
}

__declspec(naked) int Q_UTF8ToUChar32(const char *pUTF8, wchar_t &uValueOut, bool &bErrorOut)
{
	_asm
	{
		push ebp
		mov ebp, [esp + 4 + 0x4]
		xor ecx, ecx
		push esi
		mov cl, [ebp + 0]
		push edi
		cmp ecx, 0x80
		mov edi, 1
		jb loc_1006530C
		lea eax, [ecx - 0xC0]
		cmp eax, 0x37
		ja loc_1006531F
		mov dl, [ebp + 1]
		mov al, dl
		and al, 0xC0
		cmp al, 0x80
		jnz loc_1006531F
		add ecx, 0xFFFFFF3E
		and edx, 0xFF
		shl ecx, 6
		add ecx, edx
		mov edi, 2
		test ch, 8
		mov esi, 0x80
		jz loc_100652D9
		mov dl, [ebp + 2]
		mov al, dl
		and al, 0xC0
		cmp al, 0x80
		jnz loc_1006531F
		add ecx, 0xFFFFF7FE
		and edx, 0xFF
		shl ecx, 6
		add ecx, edx
		mov edi, 3
		test ecx, 0x10000
		mov esi, 0x800
		jnz short loc_100652B3
		lea edx, [ecx - 0xD800]
		cmp edx, 0x400
		jnb short loc_100652D9
		cmp byte ptr [ebp + 3], 0xED
		jnz short loc_100652D9
		mov al, [ebp + 4]
		add al, 0x50
		cmp al, 0x10
		jnb short loc_100652D9
		mov dl, [ebp + 5]
		push ebx
		mov bl, dl
		and bl, 0xC0
		cmp bl, 0x80
		pop ebx
		jnz short loc_100652D9
		shl ecx, 4
		and eax, 0xFF
		mov edi, 6
		lea ecx, [ecx + eax - 0xD7C02]
		jmp short loc_100652C9

loc_100652B3:
		mov dl, [ebp + 3]
		mov al, dl
		and al, 0xC0
		cmp al, 0x80
		jnz short loc_1006531F
		add ecx, 0xFFFEFFFE
		mov edi, 4

loc_100652C9:
		shl ecx, 6
		and edx, 0xFF
		mov esi, 0x10000
		add ecx, edx

loc_100652D9:
		cmp ecx, esi
		jb short loc_1006531F
		cmp ecx, 0x110000
		jnb short loc_1006531F
		lea edx, [ecx - 0xD800]
		cmp edx, 0x7FF
		jbe short loc_1006531F
		mov eax, ecx
		and eax, 0xFFFF
		cmp eax, 0xFFFE
		jnb short loc_1006531F
		lea edx, [ecx - 0xFDD0]
		cmp edx, 0x1F
		jbe short loc_1006531F

loc_1006530C:
		mov eax, [esp + 0xC + 0x8]
		mov [eax], ecx
		mov ecx, [esp + 0xC + 0xC]
		mov eax, edi
		pop edi
		pop esi
		mov byte ptr [ecx], 0
		pop ebp
		retn

loc_1006531F:
		mov edx, [esp + 0xC + 0x8]
		mov eax, [esp + 0xC + 0xC]
		mov dword ptr [edx], 0x3F
		mov byte ptr [eax], 1
		mov eax, edi
		pop edi
		pop esi
		pop ebp
		retn
	}
}

bool Q_UnicodeValidate(const char *pUTF8)
{
	bool bError = false;
	wchar_t uVal = 0;
	int nCharSize;

	while (*pUTF8)
	{
		nCharSize = Q_UTF8ToUChar32(pUTF8, uVal, bError);

		if (bError)
			return false;

		if (nCharSize == 6)
			return false;

		pUTF8 += nCharSize;
	}

	return true;
}

void Host_Say(CBasePlayer *player, int teamonly)
{
	CBasePlayer *client;
	int j;
	char *p;
	char text[128];
	char szTemp[256];
	const char *cpSay = "say";
	const char *cpSayTeam = "say_team";
	const char *pcmd = CMD_ARGV(0);
	bool bSenderDead = false;
	char *pszFormat, *pszConsoleFormat;

	entvars_t *pev = player->pev;

	if (player->m_flLastTalk != 0 && gpGlobals->time - player->m_flLastTalk < 0.66)
		return;

	player->m_flLastTalk = gpGlobals->time;

	if (player->pev->deadflag != DEAD_NO)
		bSenderDead = true;

	if (CMD_ARGC() == 0)
		return;

	if (!stricmp(pcmd, cpSay) || !strcmp(pcmd, cpSayTeam))
	{
		if (CMD_ARGC() < 2)
			return;

		p = (char *)CMD_ARGS();
	}
	else
	{
		if (CMD_ARGC() >= 2)
			sprintf(szTemp, "%s %s", pcmd, CMD_ARGS());
		else
			sprintf(szTemp, "%s", CMD_ARGS());

		p = szTemp;
	}

	if (*p == '"')
	{
		p++;
		p[strlen(p) - 1] = '\0';
	}

	if (!p || !*p)
		return;

	char *pc = p;

	if (!Q_UnicodeValidate(pc))
		return;

	if (teamonly == TRUE)
	{
		if (player->m_iTeam == TEAM_CT && !bSenderDead)
		{
			pszFormat = "#CKF3_Chat_B";
			pszConsoleFormat = "(BLU) %s : %s";
		}
		else if (player->m_iTeam == TEAM_TERRORIST && !bSenderDead)
		{
			pszFormat = "#CKF3_Chat_R";
			pszConsoleFormat = "(RED) %s : %s";
		}
		else if (player->m_iTeam == TEAM_CT && bSenderDead)
		{
			pszFormat = "#CKF3_Chat_B_Dead";
			pszConsoleFormat = "*DEAD*(BLU) %s : %s";
		}
		else if (player->m_iTeam == TEAM_TERRORIST || bSenderDead)
		{
			pszFormat = "#CKF3_Chat_R_Dead";
			pszConsoleFormat = "*DEAD*(RED) %s : %s";
		}
		else
		{
			pszFormat = "#CKF3_Chat_Spec";
			pszConsoleFormat = "(Spectator) %s : %s";
		}
	}
	else
	{
		if (bSenderDead)
		{
			if (player->m_iTeam == TEAM_SPECTATOR)
			{
				pszFormat = "#CKF3_Chat_AllSpec";
				pszConsoleFormat = "*SPEC* %s : %s";
			}
			else
			{
				pszFormat = "#CKF3_Chat_AllDead";
				pszConsoleFormat = "*DEAD* %s : %s";
			}
		}
		else
		{
			pszFormat = "#CKF3_Chat_All";
			pszConsoleFormat = "%s : %s";
		}
	}

	text[0] = 0;
	j = sizeof(text) - 3 - strlen(pszFormat);

	if ((int)strlen(p) > j)
		p[j] = '\0';

	for (char *pApersand = p; pApersand && *pApersand; pApersand++)
	{
		if (pApersand[0] == '%' && pApersand[1] == 'l')
		{
			if (pApersand[1] != ' ' && pApersand[1] != '\0')
				pApersand[0] = ' ';
		}
	}

	strcat(text, p);
	strcat(text, "\n");
	client = NULL;

	while ((client = (CBasePlayer *)UTIL_FindEntityByClassname(client, "player")) != NULL)
	{
		if (FNullEnt(client->edict()))
			break;

		if (!client->pev)
			continue;

		if (client->edict() == player->edict())
			continue;

		if (!client->IsNetClient())
			continue;

		if (gpGlobals->deathmatch && g_pGameRules->m_VoiceGameMgr.PlayerHasBlockedPlayer(client, player))
			continue;

		if (teamonly && client->m_iTeam != player->m_iTeam)
			continue;

		if ((client->pev->deadflag != DEAD_NO && !bSenderDead) || (client->pev->deadflag == DEAD_NO && bSenderDead))
		{
			if (!FBitSet(player->pev->flags, FL_PROXY))
				continue;
		}

		if (client->m_iIgnoreMessage == IGNOREMSG_ENEMY)
		{
			if (client->m_iTeam == player->m_iTeam)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->pev);
				WRITE_BYTE(player->entindex());
				WRITE_STRING(pszFormat);
				WRITE_STRING("");
				WRITE_STRING(text);
				MESSAGE_END();
			}
		}
		else if (client->m_iIgnoreMessage == IGNOREMSG_NONE)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->pev);
			WRITE_BYTE(player->entindex());
			WRITE_STRING(pszFormat);
			WRITE_STRING("");
			WRITE_STRING(text);
			MESSAGE_END();
		}
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, player->pev);
	WRITE_BYTE(player->entindex());
	WRITE_STRING(pszFormat);
	WRITE_STRING("");
	WRITE_STRING(text);
	MESSAGE_END();

	if (pszConsoleFormat)
		g_engfuncs.pfnServerPrint(UTIL_VarArgs(pszConsoleFormat, STRING(player->pev->netname), text));

	if (CVAR_GET_FLOAT("mp_logmessages") != 0)
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" %s \"%s\"%s\n", STRING(player->pev->netname), GETPLAYERUSERID(player->edict()), GETPLAYERAUTHID(player->edict()), GetTeam(player->m_iTeam), (teamonly != TRUE) ? "say" : "say_team", pc, (player->m_iTeam != TEAM_SPECTATOR && bSenderDead) ? " (dead)" : "");
}

void BlinkAccount(CBasePlayer *pPlayer, int time);

extern int gmsgStatusIcon;
extern int gmsgPlayerVars;

int HandleMenu_ChooseClass(CBasePlayer *pPlayer, int keys)
{
	char *model;
	char *classname;
	if(keys <= 0 || keys >= 10)
		keys = RANDOM_LONG(1,9);

	if(pPlayer->m_iNewClass == keys)
	{
		//ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Already_In_This_Class");
		return FALSE;
	}

	int iNewClass = pPlayer->m_iNewClass;
	switch (keys)
	{
		default:
		case CLASS_SCOUT:
		{
			pPlayer->m_iNewClass = CLASS_SCOUT;
			model = "ckf_scout";
			classname = "#CKF3_Scout";
			break;
		}
		case CLASS_HEAVY:
		{
			pPlayer->m_iNewClass = CLASS_HEAVY;
			model = "ckf_heavy";
			classname = "#CKF3_Heavy";
			break;
		}
		case CLASS_SOLDIER:
		{
			pPlayer->m_iNewClass = CLASS_SOLDIER;
			model = "ckf_soldier";
			classname = "#CKF3_Soldier";
			break;
		}
		case CLASS_PYRO:
		{
			pPlayer->m_iNewClass = CLASS_PYRO;
			model = "ckf_pyro";
			classname = "#CKF3_Pyro";
			break;
		}
		case CLASS_SNIPER:
		{
			pPlayer->m_iNewClass = CLASS_SNIPER;
			model = "ckf_sniper";
			classname = "#CKF3_Sniper";
			break;
		}
		case CLASS_MEDIC:
		{
			pPlayer->m_iNewClass = CLASS_MEDIC;
			model = "ckf_medic";
			classname = "#CKF3_Medic";
			break;
		}
		case CLASS_ENGINEER:
		{
			pPlayer->m_iNewClass = CLASS_ENGINEER;
			model = "ckf_engineer";
			classname = "#CKF3_Engineer";
			break;
		}
		case CLASS_DEMOMAN:
		{
			pPlayer->m_iNewClass = CLASS_DEMOMAN;
			model = "ckf_demoman";
			classname = "#CKF3_Demoman";
			break;
		}
		case CLASS_SPY:
		{
			pPlayer->m_iNewClass = CLASS_SPY;
			model = "ckf_spy";
			classname = "#CKF3_Spy";
			break;
		}
	}

	BOOL bInstantSpawn = FALSE;
	if (pPlayer->m_iJoiningState != JOINED)
	{
		if (pPlayer->m_iJoiningState == PICKINGTEAM)
			pPlayer->m_iJoiningState = GETINTOGAME;
	}
	else
	{
		if(pPlayer->IsAlive())
		{
			if(!(pPlayer->m_iMapZone & MAPZONE_RESUPPLYROOM))
			{
				ClientKill(pPlayer->edict());

				pPlayer->Respawn_Start();
			}
			else
			{
				pPlayer->RoundRespawn();
				bInstantSpawn = TRUE;
			}
		}
		else
			pPlayer->Respawn_Start();
	}
	if(!bInstantSpawn)
	{
		if(pPlayer->m_iClass == 0)
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#game_spawn_as", classname);
		else
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#game_respawn_as", classname);
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgPlayerVars, NULL, pPlayer->pev);
	WRITE_BYTE(PV_iDesiredClass);
	WRITE_BYTE(pPlayer->m_iNewClass);
	MESSAGE_END();

	pPlayer->pev->body = 0;
	g_engfuncs.pfnSetClientKeyValue(pPlayer->entindex(), g_engfuncs.pfnGetInfoKeyBuffer(ENT(pPlayer->pev)), "model", model);

	return TRUE;
}

extern int gmsgMoney;
extern int gmsgScoreInfo;
extern int gmsgTeamInfo;
extern int gmsgSpectator;
extern int gmsgSpawnInit;

BOOL HandleMenu_ChooseTeam(CBasePlayer *pPlayer, int keys)
{
	int team;
	if(keys >= 4)
		keys = 0;
	switch (keys)
	{
		case 0:
		{
			team = SelectDefaultTeam();

			if (team == TEAM_UNASSIGNED)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#All_Teams_Full");
				return FALSE;
			}
			break;
		}
		case 1:
		{
			team = TEAM_TERRORIST;
			break;
		}
		case 2:
		{
			team = TEAM_CT;
			break;
		}
		case 3:
		{
			if (pPlayer->m_iTeam == TEAM_SPECTATOR)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Already_In_This_Team");
				return FALSE;
			}

			if (!allow_spectators.value)
			{
				if (!FBitSet(pPlayer->pev->flags, FL_PROXY))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cannot_Be_Spectator");
					return FALSE;
				}
			}

			//if (g_pGameRules->m_iRoundStatus == ROUND_NORMAL)
			//{
			//	if (pPlayer->pev->deadflag == DEAD_NO)
			//	{
			//		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cannot_Be_Spectator");
			//		return FALSE;
			//	}
			//}

			if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
			{
				if (pPlayer->pev->deadflag == DEAD_NO)
					ClientKill(pPlayer->edict());
			}

			pPlayer->RemoveAllItems(TRUE);

			if (pPlayer->m_iTeam != TEAM_SPECTATOR)
				UTIL_LogPrintf("\"%s<%i><%s><%s>\" joined team \"SPECTATOR\"\n", STRING(pPlayer->pev->netname), GETPLAYERUSERID(pPlayer->edict()), GETPLAYERAUTHID(pPlayer->edict()), GetTeam(pPlayer->m_iTeam));

			pPlayer->m_iNewClass = pPlayer->m_iClass = 0;

			MESSAGE_BEGIN(MSG_ONE, gmsgSpawnInit, NULL, pPlayer->pev);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			MESSAGE_END();

			pPlayer->m_iTeam = TEAM_SPECTATOR;
			pPlayer->m_iJoiningState = JOINED;
			pPlayer->pev->frags = 0;
			pPlayer->m_iDominates = 0;
			memset(&pPlayer->m_Dominate, 0, sizeof(pPlayer->m_Dominate));

			//MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, pPlayer->pev);
			//WRITE_LONG(pPlayer->m_iAccount);
			//WRITE_BYTE(0);
			//MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
			WRITE_BYTE(ENTINDEX(pPlayer->edict()));
			WRITE_SHORT((int)pPlayer->pev->frags);
			WRITE_SHORT((int)pPlayer->m_Stats.iDeath);
			WRITE_BYTE(pPlayer->m_iDominates);
			MESSAGE_END();

			pPlayer->m_pIntroCamera = NULL;

			MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
			WRITE_BYTE(ENTINDEX(pPlayer->edict()));
			WRITE_STRING(GetTeam(pPlayer->m_iTeam));
			MESSAGE_END();

			if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
				pPlayer->SetScoreboardAttributes();

			edict_t *pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(pPlayer);
			pPlayer->StartObserver(VARS(pentSpawnSpot)->origin, VARS(pentSpawnSpot)->angles);
			pPlayer->Observer_SetMode(OBS_ROAMING);

			MESSAGE_BEGIN(MSG_ALL, gmsgSpectator);
			WRITE_BYTE(ENTINDEX(pPlayer->edict()));
			WRITE_BYTE(1);
			MESSAGE_END();
			return TRUE;
		}

		default: return FALSE;
	}

	if (pPlayer->m_iTeam == team)
	{
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Already_In_This_Team");
		return FALSE;
	}

	if (g_pGameRules->TeamFull(team))
	{
		if (team == TEAM_RED)
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#CKF3_Red_Team_Full");
		else
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#CKF3_Blue_Team_Full");

		return FALSE;
	}

	if (g_pGameRules->TeamStacked(team, pPlayer->m_iTeam))
	{
		if (team == TEAM_RED)
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#CKF3_Too_Many_Red");
		else
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#CKF3_Too_Many_Blue");

		return FALSE;
	}

	if (pPlayer->m_iTeam == TEAM_SPECTATOR && team != TEAM_SPECTATOR)
	{
		pPlayer->m_bNotKilled = TRUE;
		pPlayer->m_iIgnoreMessage = IGNOREMSG_NONE;
		pPlayer->m_iTeamKills = 0;

		pPlayer->pev->solid = SOLID_NOT;
		pPlayer->pev->movetype = MOVETYPE_NOCLIP;
		pPlayer->pev->effects = EF_NODRAW;
		pPlayer->pev->effects |= EF_NOINTERP;
		pPlayer->pev->takedamage = DAMAGE_NO;
		pPlayer->pev->deadflag = DEAD_DEAD;
		pPlayer->pev->velocity = g_vecZero;
		pPlayer->pev->punchangle = g_vecZero;
		pPlayer->m_bHasNightVision = FALSE;
		pPlayer->m_iHostagesKilled = 0;
		pPlayer->m_fDeadTime = 0;
		pPlayer->m_bDisconnect = FALSE;
		pPlayer->m_iJoiningState = GETINTOGAME;
		pPlayer->m_iNewClass = pPlayer->m_iClass = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgSpawnInit, NULL, pPlayer->pev);
		WRITE_SHORT(0);
		WRITE_BYTE(0);
		MESSAGE_END();

		g_engfuncs.pfnSetClientMaxspeed(ENT(pPlayer->pev), 1);
		SET_MODEL(ENT(pPlayer->pev), "models/player.mdl");
	}

	if(pPlayer->m_iTeam != team)
	{
		pPlayer->Build_DestroyAll();
	}

	if (pPlayer->pev->deadflag == DEAD_NO)
		ClientKill(pPlayer->edict());

	int oldteam = pPlayer->m_iTeam;
	pPlayer->m_iTeam = team;

	MESSAGE_BEGIN(MSG_ONE, gmsgResetHUD, NULL, pPlayer->pev);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(ENTINDEX(pPlayer->edict()));
	WRITE_STRING(GetTeam(team));
	MESSAGE_END();

	if(pPlayer->m_iNewClass == 0)
	{
		ShowVGUIMenu(pPlayer, (pPlayer->m_iTeam == TEAM_RED) ? MENU_CLASS_RED : MENU_CLASS_BLU, (pPlayer->m_iJoiningState != JOINED) ? KEY_1 : KEY_1 | KEY_0, "");
	}

	if (team != TEAM_UNASSIGNED)
		pPlayer->SetScoreboardAttributes();

	const char *name = pPlayer->pev->netname ? STRING(pPlayer->pev->netname) : "<unconnected>";

	if (!name[0])
		name = "<unconnected>";

	if (team == TEAM_RED)
		UTIL_ClientPrintAll(HUD_PRINTTALK, "#Game_join_red", name);
	else
		UTIL_ClientPrintAll(HUD_PRINTTALK, "#Game_join_blue", name);

	UTIL_LogPrintf("\"%s<%i><%s><%s>\" joined team \"%s\"\n", STRING(pPlayer->pev->netname), GETPLAYERUSERID(pPlayer->edict()), GETPLAYERAUTHID(pPlayer->edict()), GetTeam(oldteam), GetTeam(team));
	return TRUE;
}

void Radio1(CBasePlayer *pPlayer, int keys)
{
	if (gpGlobals->time <= pPlayer->m_flNextRadioTime)
		return;

	if (pPlayer->m_iRadioLeft <= 0)
		return;

	pPlayer->m_iRadioLeft--;
	pPlayer->m_flNextRadioTime = gpGlobals->time + 1.5;

	switch (keys)
	{
		case 1: pPlayer->Radio("%!MRAD_COVERME", "#Cover_me"); break;
		case 2: pPlayer->Radio("%!MRAD_TAKEPOINT", "#You_take_the_point"); break;
		case 3: pPlayer->Radio("%!MRAD_POSITION", "#Hold_this_position"); break;
		case 4: pPlayer->Radio("%!MRAD_REGROUP", "#Regroup_team"); break;
		case 5: pPlayer->Radio("%!MRAD_FOLLOWME", "#Follow_me"); break;
		case 6: pPlayer->Radio("%!MRAD_HITASSIST", "#Taking_fire"); break;
	}
}

void Radio2(CBasePlayer *pPlayer, int keys)
{
	if (gpGlobals->time <= pPlayer->m_flNextRadioTime)
		return;

	if (pPlayer->m_iRadioLeft <= 0)
		return;

	pPlayer->m_iRadioLeft--;
	pPlayer->m_flNextRadioTime = gpGlobals->time + 1.5;

	switch (keys)
	{
		case 1: pPlayer->Radio("%!MRAD_GO", "#Go_go_go"); break;
		case 2: pPlayer->Radio("%!MRAD_FALLBACK", "#Team_fall_back"); break;
		case 3: pPlayer->Radio("%!MRAD_STICKTOG", "#Stick_together_team"); break;
		case 4: pPlayer->Radio("%!MRAD_GETINPOS", "#Get_in_position_and_wait"); break;
		case 5: pPlayer->Radio("%!MRAD_STORMFRONT", "#Storm_the_front"); break;
		case 6: pPlayer->Radio("%!MRAD_REPORTIN", "#Report_in_team"); break;
	}
}

void Radio3(CBasePlayer *pPlayer, int keys)
{
	if (gpGlobals->time <= pPlayer->m_flNextRadioTime)
		return;

	if (pPlayer->m_iRadioLeft <= 0)
		return;

	pPlayer->m_iRadioLeft--;
	pPlayer->m_flNextRadioTime = gpGlobals->time + 1.5;

	switch (keys)
	{
		case 1:
		{
			if (RANDOM_LONG(0, 1))
				pPlayer->Radio("%!MRAD_AFFIRM", "#Affirmative");
			else
				pPlayer->Radio("%!MRAD_ROGER", "#Roger_that");

			break;
		}

		case 2: pPlayer->Radio("%!MRAD_ENEMYSPOT", "#Enemy_spotted"); break;
		case 3: pPlayer->Radio("%!MRAD_BACKUP", "#Need_backup"); break;
		case 4: pPlayer->Radio("%!MRAD_CLEAR", "#Sector_clear"); break;
		case 5: pPlayer->Radio("%!MRAD_INPOS", "#In_position"); break;
		case 6: pPlayer->Radio("%!MRAD_REPRTINGIN", "#Reporting_in"); break;
		case 7: pPlayer->Radio("%!MRAD_BLOW", "#Get_out_of_there"); break;
		case 8: pPlayer->Radio("%!MRAD_NEGATIVE", "#Negative"); break;
		case 9: pPlayer->Radio("%!MRAD_ENEMYDOWN", "#Enemy_down"); break;
	}
}

float g_flTimeLimit = 0;
float g_flResetTime = 0;

extern int GetMapCount(void);
extern int gmsgNVGToggle;

void ClientCommand(edict_t *pEntity)
{
	const char *pcmd = CMD_ARGV(0);

	if (!pEntity->pvPrivateData)
		return;

	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

	if (FStrEq(pcmd, "say"))
	{
		Host_Say(pPlayer, FALSE);
		return;
	}

	if (FStrEq(pcmd, "say_team"))
	{
		Host_Say(pPlayer, TRUE);
		return;
	}

/*
if (FStrEq(pcmd, "fullupdate"))
	{
		pPlayer->ForceClientDllUpdate();
		return;
	}
*/

	if (FStrEq(pcmd, "vote"))
	{
		BOOL bShowUsage = FALSE;

		if (gpGlobals->time < pPlayer->m_flNextVoteTime)
		{
			ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Wait_3_Seconds");
			return;
		}

		pPlayer->m_flNextVoteTime = gpGlobals->time + 3;

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
		{
			int argc = CMD_ARGC();
			const char *idstr = CMD_ARGV(1);
			int len = strlen(idstr);

			if (argc != 2 || (len <= 0 || len > 7))
				bShowUsage = TRUE;

			int id = atoi(idstr);

			if (id <= 0)
				bShowUsage = TRUE;

			if (bShowUsage)
			{
				ListPlayers(pPlayer);
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_vote_usage");
				return;
			}

			CBaseEntity *pEntity = NULL;
			int count = 0;

			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
			{
				if (FNullEnt(pEntity->edict()))
					break;

				if (pEntity->pev->flags & FL_DORMANT)
					continue;

				if (GetClassPtr((CBasePlayer *)pEntity->pev)->m_iTeam == pPlayer->m_iTeam)
					count++;
			}

			if (count < 3)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Cannot_Vote_With_Less_Than_Three");
				return;
			}

			pEntity = NULL;

			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
			{
				if (FNullEnt(pEntity->edict()))
					break;

				if (GetClassPtr((CBasePlayer *)pEntity->pev)->m_iTeam == TEAM_UNASSIGNED)
					continue;

				CBasePlayer *pOther = GetClassPtr((CBasePlayer *)pEntity->pev);

				if (id != GETPLAYERUSERID(pOther->edict()))
					continue;

				if (pOther->m_iTeam != pPlayer->m_iTeam)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_vote_players_on_your_team");
					return;
				}

				if (pOther == pPlayer)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_vote_not_yourself");
					return;
				}

				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_vote_cast", UTIL_dtos1(id));
				pPlayer->m_iVotingKikedPlayerId = id;
				ProcessKickVote(pPlayer, pOther);
				return;
			}

			ListPlayers(pPlayer);
			ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_vote_player_not_found", UTIL_dtos1(id));
		}

		return;
	}

	if (FStrEq(pcmd, "listmaps"))
	{
		g_pGameRules->DisplayMaps(pPlayer, 0);
		return;
	}

	if (FStrEq(pcmd, "addcond"))
	{
		if(CVAR_GET_FLOAT("sv_cheats") != 0)
		{
			g_pGameRules->Cmd_AddCondition(pPlayer);
		}
		return;
	}

	if (FStrEq(pcmd, "votemap"))
	{
		BOOL bShowUsage = FALSE;

		if (gpGlobals->time < pPlayer->m_flNextVoteTime)
		{
			ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Wait_3_Seconds");
			return;
		}

		pPlayer->m_flNextVoteTime = gpGlobals->time + 3;

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
		{
			if (gpGlobals->time < 180)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Cannot_Vote_Map");
				return;
			}

			if (CMD_ARGC() != 2 || strlen(CMD_ARGV(1)) > 5)
				bShowUsage = TRUE;

			const char *idstr = CMD_ARGV(1);
			int id = atoi(idstr);

			if (id < 1 || id > MAX_MAPS)
				bShowUsage = TRUE;

			if (id < GetMapCount())
				bShowUsage = TRUE;

			if (bShowUsage)
			{
				g_pGameRules->DisplayMaps(pPlayer, 0);
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_votemap_usage");
				return;
			}

			CBaseEntity *pEntity = NULL;
			int count = 0;

			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
			{
				if (FNullEnt(pEntity->edict()))
					break;

				if (FBitSet(pEntity->pev->flags, FL_DORMANT))
					continue;

				if (GetClassPtr((CBasePlayer *)pEntity->pev)->m_iTeam == pPlayer->m_iTeam)
					count++;
			}

			if (count < 2)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Cannot_Vote_Need_More_People");
				return;
			}

			if (pPlayer->m_iVotingMapId)
			{
				g_pGameRules->m_rgiVotingMapCount[pPlayer->m_iVotingMapId]--;

				if (g_pGameRules->m_rgiVotingMapCount[pPlayer->m_iVotingMapId] < 0)
					g_pGameRules->m_rgiVotingMapCount[pPlayer->m_iVotingMapId] = 0;
			}

			ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_voted_for_map", UTIL_dtos1(id));
			pPlayer->m_iVotingMapId = id;
			g_pGameRules->ProcessMapVote(pPlayer, id);
		}

		return;
	}

	if (FStrEq(pcmd, "timeleft"))
	{
		if (!timelimit.value)
		{
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#Game_no_timelimit");
			return;
		}

		int timeleft = (int)(g_flTimeLimit - gpGlobals->time);

		if (timeleft < 0)
			timeleft = 0;

		int minutes = timeleft % 60;
		int hours = timeleft / 60;
		char sMinutes[3];
		char *pszMinutes = UTIL_dtos2(minutes);

		if (minutes >= 10)
		{
			sMinutes[0] = pszMinutes[0];
			sMinutes[1] = pszMinutes[1];
			sMinutes[2] = '\0';
		}
		else
		{
			sMinutes[0] = '0';
			sMinutes[1] = pszMinutes[0];
			sMinutes[2] = '\0';
		}

		ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#Game_timelimit", UTIL_dtos1(hours), sMinutes);
		return;
	}

	if (FStrEq(pcmd, "listplayers"))
	{
		ListPlayers(pPlayer);
		return;
	}

	if (FStrEq(pcmd, "disguise"))
	{
		if(!pPlayer->m_pActiveItem)
			return;
		if(pPlayer->m_pActiveItem->iItemSlot() != WEAPON_SLOT_PDA)
			return;
		const char *pkey = CMD_ARGV(1);
		int keyteam = atoi(pkey);
		pkey = CMD_ARGV(2);
		int keyclass = atoi(pkey);
		if(keyteam != 1 && keyteam != 2)
		{
			if(keyteam == -1)
				keyteam = 3-pPlayer->m_iTeam;
			else if(keyteam == -2)
				keyteam = pPlayer->m_iTeam;
			else
				return;
		}
		pPlayer->Disguise_Start(keyteam, keyclass);
		pPlayer->m_iHudMenu = 0;
		pPlayer->SelectLastItem();
		return;
	}

	if (FStrEq(pcmd, "disguise_last"))
	{
		if(pPlayer->m_iDisguise == DISGUISE_YES)
			pPlayer->Disguise_Weapon();
		else if (pPlayer->m_iDisguiseNewTeam && pPlayer->m_iDisguiseNewClass)
			pPlayer->Disguise_Start(pPlayer->m_iDisguiseNewTeam, pPlayer->m_iDisguiseNewClass);

		if(pPlayer->m_pActiveItem->iItemSlot() == WEAPON_SLOT_PDA)
		{
			pPlayer->SelectLastItem();
		}
		return;
	}

	if (FStrEq(pcmd, "build"))
	{
		if(!pPlayer->m_pActiveItem)
			return;
		if(pPlayer->m_pActiveItem->iItemSlot() != WEAPON_SLOT_PDA)
			return;
		const char *pkey = CMD_ARGV(1);
		int buildclass = atoi(pkey);
		if(buildclass >= BUILDABLE_SENTRY && buildclass <= BUILDABLE_EXIT)
		{
			pPlayer->Build_Start(buildclass);
			pPlayer->m_iHudMenu = 0;
		}
		return;
	}

	if (FStrEq(pcmd, "demolish"))
	{
		if(!pPlayer->m_pActiveItem)
			return;
		if(pPlayer->m_pActiveItem->iItemSlot() != WEAPON_SLOT_PDA2)
			return;
		const char *pkey = CMD_ARGV(1);
		int buildclass = atoi(pkey);
		if(buildclass >= BUILDABLE_SENTRY && buildclass <= BUILDABLE_EXIT)
		{
			pPlayer->Build_Demolish(buildclass);
			pPlayer->m_iHudMenu = 0;
		}
		return;
	}

	if (FStrEq(pcmd, "chooseclass") || FStrEq(pcmd, "changeclass") )
	{
		if(pPlayer->m_iMenu == MENU_TEAM || pPlayer->m_iMenu == MENU_INTRO)
			return;
		ShowVGUIMenu(pPlayer, (pPlayer->m_iTeam == TEAM_RED) ? MENU_CLASS_RED : MENU_CLASS_BLU, (pPlayer->m_iJoiningState != JOINED) ? KEY_1 : KEY_1 | KEY_0, "");
		//ShowVGUIMenu(pPlayer, MENU_TEAM, 0, "");
		return;
	}

	if (FStrEq(pcmd, "joinclass"))
	{
		if(pPlayer->m_iJoiningState == JOINED && pPlayer->m_iMenu != MENU_CLASS_RED && pPlayer->m_iMenu != MENU_CLASS_BLU)
			return;

		const char *pkey = CMD_ARGV(1);
		int keys = atoi(pkey);

		pPlayer->m_iMenu = 0;

		if (HandleMenu_ChooseClass(pPlayer, keys))
			return;

		ShowVGUIMenu(pPlayer, (pPlayer->m_iTeam == TEAM_RED) ? MENU_CLASS_RED : MENU_CLASS_BLU, (pPlayer->m_iJoiningState != JOINED) ? KEY_1 : KEY_1 | KEY_0, "");
	}

	if (FStrEq(pcmd, "chooseteam") || FStrEq(pcmd, "changeteam"))
	{
		if(pPlayer->m_iMenu == MENU_CLASS_RED || pPlayer->m_iMenu == MENU_CLASS_BLU)
			return;

		int menukeys = KEY_1 | KEY_2 | KEY_3 | KEY_4 | KEY_0;
		
		if (!allow_spectators.value)
			menukeys &= ~KEY_2;
		if(pPlayer->m_iJoiningState != JOINED)
			menukeys &= ~KEY_0;

		ShowVGUIMenu(pPlayer, MENU_TEAM, menukeys, "");
		return;
	}

	if (FStrEq(pcmd, "jointeam"))
	{
		if(pPlayer->m_iJoiningState == JOINED && pPlayer->m_iMenu != MENU_TEAM)
			return;

		const char *pkey = CMD_ARGV(1);
		int keys = atoi(pkey);

		pPlayer->m_iMenu = 0;

		if (HandleMenu_ChooseTeam(pPlayer, keys)) return;

		int menukeys = KEY_1 | KEY_2 | KEY_3 | KEY_4 | KEY_0;
		if (!allow_spectators.value)//these keys are only for text menus
			menukeys &= ~KEY_2;
		if(pPlayer->m_iJoiningState != JOINED)
			menukeys &= ~KEY_0;

		ShowVGUIMenu(pPlayer, MENU_TEAM, menukeys, "");
	}

	if (FStrEq(pcmd, "closemenu"))
	{
		pPlayer->m_iMenu = 0;
		return;
	}

	if (FStrEq(pcmd, "showmapinfo"))
	{
		//if (g_MapInfoText.empty())
		//	return;

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
			return;

		if (pPlayer->m_afPhysicsFlags & PFLAG_OBSERVER)
			return;

		ShowVGUIMenu(pPlayer, MENU_MAPINFO, 0, "");

		//SendMGUIPrint(pPlayer, MENUBUF_MAPINFO, g_MapInfoText.c_str());
		pPlayer->m_bMissionBriefing = TRUE;
		return;
	}

	if (FStrEq(pcmd, "ignoremsg"))
	{
		if (pPlayer->m_iIgnoreMessage == IGNOREMSG_NONE)
		{
			pPlayer->m_iIgnoreMessage = IGNOREMSG_ENEMY;
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Ignore_Broadcast_Messages");
		}
		else if (pPlayer->m_iIgnoreMessage == IGNOREMSG_ENEMY)
		{
			pPlayer->m_iIgnoreMessage = IGNOREMSG_TEAM;
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Ignore_Broadcast_Team_Messages");
		}
		else if (pPlayer->m_iIgnoreMessage == IGNOREMSG_TEAM)
		{
			pPlayer->m_iIgnoreMessage = IGNOREMSG_NONE;
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Accept_All_Messages");
		}

		return;
	}

	if (FStrEq(pcmd, "ignorerad"))
	{
		if (!pPlayer->m_bIgnoreRadio)
		{
			pPlayer->m_bIgnoreRadio = TRUE;
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Ignore_Radio");
		}
		else
		{
			pPlayer->m_bIgnoreRadio = FALSE;
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Accept_Radio");
		}

		return;
	}

	if (FStrEq(pcmd, "spectate") && FBitSet(pPlayer->pev->flags, FL_PROXY))
	{
		HandleMenu_ChooseTeam(pPlayer, 2);
		return;
	}

	if (FStrEq(pcmd, "specmode"))
	{
		if (pPlayer->IsObserver())
			pPlayer->Observer_SetMode(atoi(CMD_ARGV(1)));

		return;
	}

	if (FStrEq(pcmd, "follownext"))
	{
		if (pPlayer->IsObserver())
			pPlayer->Observer_FindNextPlayer(atoi(CMD_ARGV(1)) != FALSE);

		return;
	}

	if (g_pGameRules->ClientCommand_DeadOrAlive(GetClassPtr((CBasePlayer *)pev), pcmd))
		return;

	if (pPlayer->pev->deadflag != DEAD_NO)
		return;

	if (FStrEq(pcmd, "drop"))
	{
		//GetClassPtr((CBasePlayer *)pev)->DropPlayerItem(CMD_ARGV(1));
		return;
	}

	if (FStrEq(pcmd, "fov"))
	{
		return;
	}

#ifdef _DEBUG
	if (FStrEq(pcmd, "give"))
	{
		GetClassPtr((CBasePlayer *)pev)->GiveNamedItem(CMD_ARGV(1));
		return;
	}
#endif

	if (FStrEq(pcmd, "use"))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem(CMD_ARGV(1));
		return;
	}

	const char *pstr = strstr(pcmd, "weapon_");

	if (pstr && pstr == pcmd)
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem(pcmd);
		return;
	}

	if (FStrEq(pcmd, "lastinv"))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectLastItem();
		return;
	}

	if (FStrEq(pcmd, "switch"))
	{
		GetClassPtr((CBasePlayer *)pev)->SwitchSlotWeapon(atoi(CMD_ARGV(1)));
		return;
	}

	if (g_pGameRules->ClientCommand(GetClassPtr((CBasePlayer *)pev), pcmd))
		return;

	char command[128];
	strncpy(command, pcmd, 127);
	command[127] = '\0';
	ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, "#Game_unknown_command", command);
}

extern int gmsgBlinkAcct;

void BlinkAccount(CBasePlayer *pPlayer, int time)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, pPlayer->pev);
	WRITE_BYTE(2);
	MESSAGE_END();
}

void ClientUserInfoChanged(edict_t *pEntity, char *infobuffer)
{
	if (!pEntity->pvPrivateData)
		return;

	char *szBufferName = g_engfuncs.pfnInfoKeyValue(infobuffer, "name");
	int iClientIndex = ENTINDEX(pEntity);

	if (pEntity->v.netname && STRING(pEntity->v.netname)[0] != '\0' && !FStrEq(STRING(pEntity->v.netname), szBufferName))
	{
		char sName[32];
		_snprintf(sName, sizeof(sName), "%s", szBufferName);

		for (char *pApersand = sName; pApersand != NULL && *pApersand != '\0'; pApersand++)
		{
			if (*pApersand == '%' || *pApersand == '&')
				*pApersand = ' ';
		}

		if (sName[0] == '#')
			sName[0] = '*';

		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEntity);

		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			pPlayer->m_bNameChanged = TRUE;
			_snprintf(pPlayer->m_szNewName, sizeof(pPlayer->m_szNewName), "%s", sName);
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#Name_change_at_respawn");
			g_engfuncs.pfnSetClientKeyValue(iClientIndex, infobuffer, "name", (char *)STRING(pEntity->v.netname));
		}
		else
		{
			g_engfuncs.pfnSetClientKeyValue(iClientIndex, infobuffer, "name", sName);

			MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
			WRITE_BYTE(iClientIndex);
			WRITE_STRING("#CKF3_Name_Change");
			WRITE_STRING(STRING(pEntity->v.netname));
			WRITE_STRING(sName);
			MESSAGE_END();

			UTIL_LogPrintf("\"%s<%i><%s><%s>\" changed name to \"%s\"\n", STRING(pEntity->v.netname), GETPLAYERUSERID(pEntity), GETPLAYERAUTHID(pEntity), GetTeam(pPlayer->m_iTeam), sName);
		}
	}

	g_pGameRules->ClientUserInfoChanged(GetClassPtr((CBasePlayer *)&pEntity->v), infobuffer);
}

static int g_serveractive = 0;

void ServerDeactivate(void)
{
	if (g_serveractive != 1)
		return;

	g_serveractive = 0;
	//CLocalNav::Reset();//Hostage
}

void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	g_serveractive = 1;

	for (int i = 0; i < edictCount; i++)
	{
		if (pEdictList[i].free)
			continue;

		if (i < clientMax || !pEdictList[i].pvPrivateData)
			continue;

		CBaseEntity *pClass = CBaseEntity::Instance(&pEdictList[i]);

		if (pClass && !(pClass->pev->flags & FL_DORMANT))
			pClass->Activate();
		else
			ALERT(at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname));
	}

	LinkUserMessages();

	if (g_pGameRules)
		g_pGameRules->CheckMapConditions();
}

void PlayerPreThink(edict_t *pEntity)
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PreThink();
}

void PlayerPostThink(edict_t *pEntity)
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PostThink();
}

void ParmsNewLevel(void)
{
}

void ParmsChangeLevel(void)
{
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if (pSaveData)
		pSaveData->connectionCount = BuildChangeList(pSaveData->levelList, MAX_LEVEL_CONNECTIONS);
}

void StartFrame(void)
{
	if (g_pGameRules)
		g_pGameRules->Think();

	if (g_fGameOver)
		return;

	//CLocalNav::Think();//Hostage
	static cvar_t *skill = NULL;

	if (!skill)
		skill = CVAR_GET_POINTER("skill");

	gpGlobals->teamplay = 1;

	if (skill)
		g_iSkillLevel = skill->value;
	else
		g_iSkillLevel = 0;

	g_ulFrameCount++;
}

unsigned short m_usResetDecals;

char *sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/ckf_scout/ckf_scout.mdl",
	"models/player/ckf_heavy/ckf_heavy.mdl",
	"models/player/ckf_soldier/ckf_soldier.mdl",
	"models/player/ckf_pyro/ckf_pyro.mdl",
	"models/player/ckf_sniper/ckf_sniper.mdl",
	"models/player/ckf_medic/ckf_medic.mdl",
	"models/player/ckf_engineer/ckf_engineer.mdl",
	"models/player/ckf_demoman/ckf_demoman.mdl",
	"models/player/ckf_spy/ckf_spy.mdl",
};

#define PLAYERMODEL_COUNT (sizeof(sPlayerModelFiles) / sizeof(sPlayerModelFiles[0]))

void ClientPrecache(void)
{
	int i;

	PRECACHE_SOUND("weapons/dryfire_pistol.wav");
	PRECACHE_SOUND("weapons/dryfire_rifle.wav");
	PRECACHE_SOUND("player/pl_shot1.wav");
	PRECACHE_SOUND("player/pl_die1.wav");
	PRECACHE_SOUND("player/headshot1.wav");
	PRECACHE_SOUND("player/headshot2.wav");
	PRECACHE_SOUND("player/headshot3.wav");
	PRECACHE_SOUND("player/bhit_flesh-1.wav");
	PRECACHE_SOUND("player/bhit_flesh-2.wav");
	PRECACHE_SOUND("player/bhit_flesh-3.wav");
	PRECACHE_SOUND("player/bhit_kevlar-1.wav");
	PRECACHE_SOUND("player/bhit_helmet-1.wav");
	PRECACHE_SOUND("player/die1.wav");
	PRECACHE_SOUND("player/die2.wav");
	PRECACHE_SOUND("player/die3.wav");
	PRECACHE_SOUND("player/death6.wav");
	/*PRECACHE_SOUND("radio/locknload.wav");
	PRECACHE_SOUND("radio/letsgo.wav");
	PRECACHE_SOUND("radio/moveout.wav");
	PRECACHE_SOUND("radio/com_go.wav");
	PRECACHE_SOUND("radio/rescued.wav");
	PRECACHE_SOUND("radio/rounddraw.wav");*/
	PRECACHE_SOUND("items/kevlar.wav");
	PRECACHE_SOUND("items/ammopickup2.wav");
	/*PRECACHE_SOUND("items/nvg_on.wav");
	PRECACHE_SOUND("items/nvg_off.wav");
	PRECACHE_SOUND("items/equip_nvg.wav");
	PRECACHE_SOUND("weapons/c4_beep1.wav");
	PRECACHE_SOUND("weapons/c4_beep2.wav");
	PRECACHE_SOUND("weapons/c4_beep3.wav");
	PRECACHE_SOUND("weapons/c4_beep4.wav");
	PRECACHE_SOUND("weapons/c4_beep5.wav");
	PRECACHE_SOUND("weapons/c4_explode1.wav");
	PRECACHE_SOUND("weapons/c4_plant.wav");
	PRECACHE_SOUND("weapons/c4_disarm.wav");
	PRECACHE_SOUND("weapons/c4_disarmed.wav");*/
	PRECACHE_SOUND("weapons/explode3.wav");
	PRECACHE_SOUND("weapons/explode4.wav");
	PRECACHE_SOUND("weapons/explode5.wav");
	PRECACHE_SOUND("player/sprayer.wav");
	PRECACHE_SOUND("player/pl_fallpain2.wav");
	PRECACHE_SOUND("player/pl_fallpain3.wav");
	PRECACHE_SOUND("player/pl_snow1.wav");
	PRECACHE_SOUND("player/pl_snow2.wav");
	PRECACHE_SOUND("player/pl_snow3.wav");
	PRECACHE_SOUND("player/pl_snow4.wav");
	PRECACHE_SOUND("player/pl_snow5.wav");
	PRECACHE_SOUND("player/pl_snow6.wav");
	PRECACHE_SOUND("player/pl_step1.wav");
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");
	PRECACHE_SOUND("common/npc_step1.wav");
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");
	PRECACHE_SOUND("player/pl_metal1.wav");
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");
	PRECACHE_SOUND("player/pl_dirt1.wav");
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");
	PRECACHE_SOUND("player/pl_duct1.wav");
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");
	PRECACHE_SOUND("player/pl_grate1.wav");
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");
	PRECACHE_SOUND("player/pl_slosh1.wav");
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");
	PRECACHE_SOUND("player/pl_tile1.wav");
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");
	PRECACHE_SOUND("player/pl_swim1.wav");
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");
	PRECACHE_SOUND("player/pl_ladder1.wav");
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");
	PRECACHE_SOUND("player/pl_wade1.wav");
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");
	PRECACHE_SOUND("debris/wood1.wav");
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");
	PRECACHE_SOUND("plats/train_use1.wav");
	PRECACHE_SOUND("plats/vehicle_ignition.wav");
	PRECACHE_SOUND("buttons/spark5.wav");
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");
	PRECACHE_SOUND("common/bodysplat.wav");
	PRECACHE_SOUND("player/pl_pain2.wav");
	PRECACHE_SOUND("player/pl_pain4.wav");
	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");

	for (i = 0; i < PLAYERMODEL_COUNT; i++)
	{
		if(i >= CLASS_SCOUT && i <= CLASS_SPY)
			g_sModelIndexPlayerClass[i-1] = PRECACHE_MODEL(sPlayerModelFiles[i]);
		else
			PRECACHE_MODEL(sPlayerModelFiles[i]);
	}

	for (i = 0; i < 4; i++)
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-38, -24, -41), (float *)&Vector(38, 24, 41), sPlayerModelFiles[i]);

	for (i = 4; i < 10; i++)
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds_if_avail, (float *)&Vector(-38, -24, -41), (float *)&Vector(38, 24, 41), sPlayerModelFiles[i]);

/*	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/black_smoke1.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/black_smoke2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/black_smoke3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/black_smoke4.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/fast_wallpuff1.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_left.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_left2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_left3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_right.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_right2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/bottom_right3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/left.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/left2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/left3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/right.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/right2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/right3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_left.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_left2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_left3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_right.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_right2.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/top_right3.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/vertical.spr");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, (float *)&g_vecZero, (float *)&g_vecZero, "sprites/horizontal.spr");*/

	/*
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-17, -6, -19), (float *)&Vector(17, 12, 29), "models/p_ak47.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-23, -6, -18), (float *)&Vector(23, 7, 18), "models/p_aug.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-30, -24, -19), (float *)&Vector(30, 17, 19), "models/p_awp.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-7, -7, -15), (float *)&Vector(7, 7, 15), "models/p_c4.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-4, -8, -3), (float *)&Vector(3, 7, 3), "models/w_c4.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-8, -4, -19), (float *)&Vector(8, 4, 19), "models/p_deagle.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-7, -2, -18), (float *)&Vector(7, 2, 18), "models/p_flashbang.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-5, -5, -5), (float *)&Vector(5, 5, 14), "models/w_flashbang.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-31, -7, -19), (float *)&Vector(31, 5, 27), "models/p_g3sg1.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_samebounds, (float *)&Vector(-33, -4, -20), (float *)&Vector(33, 7, 30), "models/p_sg550.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-8, -5, -18), (float *)&Vector(8, 4, 18), "models/p_glock18.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-7, -2, -18), (float *)&Vector(7, 2, 18), "models/p_hegrenade.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-5, -5, -5), (float *)&Vector(5, 5, 14), "models/w_hegrenade.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-7, -7, -18), (float *)&Vector(7, 5, 18), "models/p_knife.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-29, -7, -18), (float *)&Vector(29, 5, 18), "models/p_m249.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-21, -13, -18), (float *)&Vector(21, 23, 18), "models/p_m3.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-22, -8, -19), (float *)&Vector(22, 4, 25), "models/p_m4a1.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-10, -6, -18), (float *)&Vector(10, 4, 18), "models/p_mac10.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-23, -8, -18), (float *)&Vector(23, 3, 18), "models/p_mp5.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_samebounds, (float *)&Vector(-14, -2, -18), (float *)&Vector(14, 2, 18), "models/p_ump45.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-10, -4, -18), (float *)&Vector(10, 4, 18), "models/p_p228.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-15, -4, -18), (float *)&Vector(13, 6, 18), "models/p_p90.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-32, -4, -19), (float *)&Vector(32, 5, 19), "models/p_scout.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-23, -5, -18), (float *)&Vector(23, 7, 19), "models/p_sg552.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-7, -2, -18), (float *)&Vector(7, 2, 18), "models/p_smokegrenade.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-5, -5, -5), (float *)&Vector(5, 5, 14), "models/w_smokegrenade.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-15, -4, -18), (float *)&Vector(15, 6, 18), "models/p_tmp.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-13, -4, -18), (float *)&Vector(13, 3, 18), "models/p_usp.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-10, -4, -20), (float *)&Vector(10, 4, 20), "models/p_elite.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-8, -3, -18), (float *)&Vector(8, 3, 18), "models/p_fiveseven.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-25, -19, -18), (float *)&Vector(25, 16, 18), "models/p_xm1014.mdl");
	ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&Vector(-22, -8, -19), (float *)&Vector(22, 4, 25), "models/wp_rocketlauncher.mdl");
	*/
	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");
	PRECACHE_SOUND("player/geiger6.wav");
	PRECACHE_SOUND("player/geiger5.wav");
	PRECACHE_SOUND("player/geiger4.wav");
	PRECACHE_SOUND("player/geiger3.wav");
	PRECACHE_SOUND("player/geiger2.wav");
	PRECACHE_SOUND("player/geiger1.wav");

	if (giPrecacheGrunt)
		UTIL_PrecacheOther("enemy_terrorist");

	PRECACHE_MODEL("sprites/wall_puff1.spr");
	PRECACHE_MODEL("sprites/wall_puff2.spr");
	PRECACHE_MODEL("sprites/wall_puff3.spr");
	PRECACHE_MODEL("sprites/wall_puff4.spr");
	PRECACHE_MODEL("sprites/black_smoke1.spr");
	PRECACHE_MODEL("sprites/black_smoke2.spr");
	PRECACHE_MODEL("sprites/black_smoke3.spr");
	PRECACHE_MODEL("sprites/black_smoke4.spr");
	PRECACHE_MODEL("sprites/fast_wallpuff1.spr");
	PRECACHE_MODEL("sprites/pistol_smoke1.spr");
	PRECACHE_MODEL("sprites/pistol_smoke2.spr");
	PRECACHE_MODEL("sprites/rifle_smoke1.spr");
	PRECACHE_MODEL("sprites/rifle_smoke2.spr");
	PRECACHE_MODEL("sprites/rifle_smoke3.spr");
/*	PRECACHE_MODEL("sprites/bottom.spr");
	PRECACHE_MODEL("sprites/bottom2.spr");
	PRECACHE_MODEL("sprites/bottom3.spr");
	PRECACHE_MODEL("sprites/bottom_left.spr");
	PRECACHE_MODEL("sprites/bottom_left2.spr");
	PRECACHE_MODEL("sprites/bottom_left3.spr");
	PRECACHE_MODEL("sprites/bottom_right.spr");
	PRECACHE_MODEL("sprites/bottom_right2.spr");
	PRECACHE_MODEL("sprites/bottom_right3.spr");
	PRECACHE_MODEL("sprites/left.spr");
	PRECACHE_MODEL("sprites/left2.spr");
	PRECACHE_MODEL("sprites/left3.spr");
	PRECACHE_MODEL("sprites/right.spr");
	PRECACHE_MODEL("sprites/right2.spr");
	PRECACHE_MODEL("sprites/right3.spr");
	PRECACHE_MODEL("sprites/top.spr");
	PRECACHE_MODEL("sprites/top2.spr");
	PRECACHE_MODEL("sprites/top3.spr");
	PRECACHE_MODEL("sprites/top_left.spr");
	PRECACHE_MODEL("sprites/top_left2.spr");
	PRECACHE_MODEL("sprites/top_left3.spr");
	PRECACHE_MODEL("sprites/top_right.spr");
	PRECACHE_MODEL("sprites/top_right2.spr");
	PRECACHE_MODEL("sprites/top_right3.spr");
	PRECACHE_MODEL("sprites/vertical.spr");
	PRECACHE_MODEL("sprites/horizontal.spr");*/
	//ckf
	PRECACHE_MODEL("sprites/smoke.spr");

	m_usResetDecals = PRECACHE_EVENT(1, "events/decal_reset.sc");
}

const char *GetGameDescription(void)
{
	if (g_pGameRules)
		return g_pGameRules->GetGameDescription();
	else
		return "Chicken Fortress 3";
}

void Sys_Error(const char *error_string)
{
}

void PlayerCustomization(edict_t *pEntity, customization_t *pCust)
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
		case t_decal: pPlayer->SetCustomDecalFrames(pCust->nUserData2); break;
		case t_sound:
		case t_skin:
		case t_model: break;
		default: ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n"); break;
	}
}

void SpectatorConnect(edict_t *pEntity)
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect();
}

void SpectatorDisconnect(edict_t *pEntity)
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect();
}

void SpectatorThink(edict_t *pEntity)
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink();
}

void SetupVisibility(edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas)
{
	edict_t *pView = pClient;

	if (pViewEntity)
		pView = pViewEntity;

	if (pClient->v.flags & FL_PROXY)
	{
		*pvs = NULL;
		*pas = NULL;
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);

	if (pPlayer->pev->iuser2)
	{
		if (pPlayer->m_hObserverTarget)
			pView = pPlayer->m_hObserverTarget->edict();
	}

	Vector org = pView->v.origin + pView->v.view_ofs;

	if (pView->v.flags & FL_DUCKING)
		org = org + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

	*pvs = ENGINE_SET_PVS((float *)&org);
	if(g_pGameRules->m_SkyCamera.enable)
	{
		gSVFuncs.SV_AddToFatPVS( (float *)&(g_pGameRules->m_SkyCamera.origin.x), (*sv_worldmodel)->nodes );
	}
	*pas = ENGINE_SET_PAS((float *)&org);
	if(g_pGameRules->m_SkyCamera.enable)
	{
		gSVFuncs.SV_AddToFatPAS( (float *)&(g_pGameRules->m_SkyCamera.origin.x), (*sv_worldmodel)->nodes );
	}
}

struct
{
	float lastCheckTime[1380];
	int headnode;
	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
}
g_PVSStatus[32];

void MarkEntityInPVS(int index, int e, float time, BOOL unmark = FALSE)
{
	if (!unmark)
		g_PVSStatus[index].lastCheckTime[e] = time;
	else
		g_PVSStatus[index].lastCheckTime[e] = 0;
}

void ResetPlayerPVS(edict_t *player, int index)
{
	memset(&g_PVSStatus[index], 0, sizeof(g_PVSStatus[index]));
	g_PVSStatus->headnode = player->headnode;
	g_PVSStatus->num_leafs = player->num_leafs;
	memcpy(g_PVSStatus->leafnums, player->leafnums, sizeof(g_PVSStatus->leafnums));
}

BOOL CheckEntityRecentlyInPVS(int index, int e, float time)
{
	if (g_PVSStatus[index].lastCheckTime[e] && time > g_PVSStatus[index].lastCheckTime[e])
		return TRUE;

	return FALSE;
}

BOOL CheckPlayerPVSLeafChanged(edict_t *player, int index)
{
	if (g_PVSStatus[index].headnode != player->headnode || g_PVSStatus[index].num_leafs != player->num_leafs)
		return TRUE;

	if (g_PVSStatus[index].num_leafs > 0)
	{
		for (int i = 0; i < g_PVSStatus[index].num_leafs; i++)
		{
			if (player->leafnums[i] != g_PVSStatus[index].leafnums[i])
				return TRUE;
		}
	}

	return FALSE;
}

#include "entity_state.h"

int AddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
{
	if ((ent->v.effects == EF_NODRAW) && ent != host)
		return 0;

	if (!ent->v.modelindex || !STRING(ent->v.model))
		return 0;

	if ((ent->v.flags & FL_SPECTATOR) && ent != host)
		return 0;

	int i;
	int hostindex = ENTINDEX(host) - 1;

	if (CheckPlayerPVSLeafChanged(host, hostindex))
		ResetPlayerPVS(host, hostindex);

	if (ent != host)
	{
		if (!CheckEntityRecentlyInPVS(hostindex, e, gpGlobals->time))
		{
			if (!ENGINE_CHECK_VISIBILITY((const struct edict_s *)ent, pSet))
			{
				MarkEntityInPVS(hostindex, e, gpGlobals->time, true);
				return 0;
			}

			MarkEntityInPVS(hostindex, e, gpGlobals->time);
		}
	}

	if (ent->v.flags & FL_SKIPLOCALHOST)
	{
		if ((hostflags & 1) && (ent->v.owner == host))
			return 0;
	}

	if (host->v.groupinfo)
	{
		UTIL_SetGroupTrace(host->v.groupinfo, GROUP_OP_AND);

		if (ent->v.groupinfo)
		{
			if (g_groupop == GROUP_OP_AND)
			{
				if (!(ent->v.groupinfo & host->v.groupinfo))
					return 0;
			}
			else if (g_groupop == GROUP_OP_NAND)
			{
				if (ent->v.groupinfo & host->v.groupinfo)
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset(state, 0, sizeof(*state));

	state->number = e;
	state->entityType = ENTITY_NORMAL;

	if (ent->v.flags & FL_CUSTOMENTITY)
		state->entityType = ENTITY_BEAM;

	state->animtime = (int)(1000.0 * ent->v.animtime) / 1000.0;

	memcpy(state->origin, ent->v.origin, 3 * sizeof(float));
	memcpy(state->angles, ent->v.angles, 3 * sizeof(float));
	memcpy(state->mins, ent->v.mins, 3 * sizeof(float));
	memcpy(state->maxs, ent->v.maxs, 3 * sizeof(float));
	memcpy(state->startpos, ent->v.startpos, 3 * sizeof(float));
	memcpy(state->endpos, ent->v.endpos, 3 * sizeof(float));

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;
	state->modelindex = ent->v.modelindex;
	state->frame = ent->v.frame;
	state->skin = ent->v.skin;
	state->effects = ent->v.effects;

	if (!player && ent->v.animtime && !ent->v.velocity[0] && !ent->v.velocity[1] && !ent->v.velocity[2])
		state->eflags |= EFLAG_SLERP;

	state->scale = ent->v.scale;
	state->solid = ent->v.solid;
	state->colormap = ent->v.colormap;
	state->movetype = ent->v.movetype;
	state->sequence = ent->v.sequence;
	state->framerate = ent->v.framerate;
	state->body = ent->v.body;

	for (i = 0; i < 4; i++)
		state->controller[i] = ent->v.controller[i];

	for (i = 0; i < 2; i++)
		state->blending[i] = ent->v.blending[i];

	state->rendermode = ent->v.rendermode;
	state->renderamt = (int)ent->v.renderamt;
	state->renderfx = ent->v.renderfx;
	state->rendercolor.r = (byte)ent->v.rendercolor.x;
	state->rendercolor.g = (byte)ent->v.rendercolor.y;
	state->rendercolor.b = (byte)ent->v.rendercolor.z;
	state->aiment = 0;

	if (ent->v.aiment)
		state->aiment = ENTINDEX(ent->v.aiment);

	state->owner = 0;

	if (ent->v.owner)
	{
		int owner = ENTINDEX(ent->v.owner);

		if (owner >= 1 && owner <= gpGlobals->maxClients)
			state->owner = owner;
	}

	CBasePlayer *pHost = (CBasePlayer *)CBaseEntity::Instance(host);

	if (!player)
	{
		state->team = ent->v.team;
		state->playerclass = ent->v.playerclass;

		CBaseEntity *pEntity = CBaseEntity::Instance(ent);
		if(pEntity)
		{
			int classify = pEntity->Classify();
			if(classify == CLASS_BUILDABLE)
			{
				CBaseBuildable *pBuild = (CBaseBuildable *)pEntity;
				state->team = pBuild->m_iTeam;
				state->playerclass = CLASS_BUILDABLE;
				state->iuser4 = (pBuild->m_pSapper != NULL) ? 1 : 0;
				if(pBuild->m_pPlayer)
				{
					state->iuser1 = pBuild->m_pPlayer->entindex();
					state->iuser2 = pBuild->GetBuildLevel();
					state->iuser3 = pBuild->GetBuildClass();
				}
			}
			else if(classify == CLASS_PROJECTILE)
			{
				CGrenade *pGrenade = (CGrenade *)pEntity;
				state->playerclass = CLASS_PROJECTILE;
				state->team = pGrenade->m_iTeam;
				if(pGrenade->pev->owner)
				{
					state->iuser1 = ENTINDEX(pGrenade->pev->owner);
				}
			}
		}
	}
	else
	{
		memcpy(state->basevelocity, ent->v.basevelocity, 3 * sizeof(float));

		state->weaponmodel = MODEL_INDEX(STRING(ent->v.weaponmodel));
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction = ent->v.friction;
		state->gravity = ent->v.gravity;
		state->usehull = (ent->v.flags & FL_DUCKING) ? 1 : 0;
		state->health = (int)ent->v.health;

		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(ent);

		if(pHost && pPlayer)
		{
			if(pPlayer->m_iCloak != CLOAK_NO)
			{
				state->renderfx = kRenderFxCloak;
				if(pPlayer->m_iTeam != 3 - pHost->m_iTeam)
				{
					state->renderamt = int( 255.0 - (255 - ent->v.renderamt) * 0.85 ); 
				}
			}
			else
			{
				if(pHost != pPlayer)
					state->effects |= EF_SHADOW;
			}
			if(pPlayer->m_Cond.AfterBurn.m_iStatus)
			{
				state->effects |= EF_AFTERBURN;
			}
			if(pPlayer->m_iDisguise == DISGUISE_YES)
			{
				if(pPlayer->m_iTeam != pHost->m_iTeam)
				{
					state->weaponmodel = MODEL_INDEX(STRING(pPlayer->m_iDisguiseWeapon));
					state->sequence = pPlayer->m_iDisguiseSequence;
					state->scale = pPlayer->m_iDisguiseWeaponBody;
					state->startpos.x = pPlayer->m_iDisguiseHealth;
					state->startpos.y = pPlayer->m_iDisguiseMaxHealth;

					state->skin = (pPlayer->m_iDisguiseTeam == 1) ? 0 : 1;
					if(pPlayer->pev->effects & EF_INVULNERABLE)
						state->skin += 2;
				}
				else
				{
					state->startpos.x = pPlayer->pev->health;
					state->startpos.y = pPlayer->pev->max_health;
				}
				state->playerclass = pPlayer->m_iDisguiseClass;
				state->team = pPlayer->m_iDisguiseTeam;
				state->endpos.x = 1 | (pPlayer->m_iDisguiseTarget<<1);
			}
			else
			{
				state->playerclass = pPlayer->m_iClass;
				state->team = pPlayer->m_iTeam;
				state->endpos.x = 0;
				state->startpos.x = pPlayer->pev->health;
				state->startpos.y = pPlayer->pev->max_health;
			}
			if(pPlayer->m_iTeam == pHost->m_iTeam)
			{
				if(pPlayer->m_iClass == CLASS_MEDIC)
				{
					state->startpos.z = pPlayer->m_fUbercharge;
				}
			}
		}
	}

	state->iuser4 = ent->v.iuser4;
	return 1;
}

void CreateBaseline(int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs)
{
	baseline->origin = entity->v.origin;
	baseline->angles = entity->v.angles;
	baseline->frame = entity->v.frame;
	baseline->skin = (short)entity->v.skin;
	baseline->rendermode = (byte)entity->v.rendermode;
	baseline->renderamt = (byte)entity->v.renderamt;
	baseline->rendercolor.r = (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g = (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b = (byte)entity->v.rendercolor.z;
	baseline->renderfx = (byte)entity->v.renderfx;

	if (player)
	{
		baseline->mins = player_mins;
		baseline->maxs = player_maxs;
		baseline->colormap = eindex;
		baseline->modelindex = playermodelindex;
		baseline->friction = 1;
		baseline->movetype = MOVETYPE_WALK;
		baseline->scale = entity->v.scale;
		baseline->solid = SOLID_SLIDEBOX;
		baseline->framerate = 1;
		baseline->gravity = 1;
		baseline->team = entity->v.team;
		baseline->playerclass = entity->v.playerclass;
	}
	else
	{
		baseline->mins = entity->v.mins;
		baseline->maxs = entity->v.maxs;
		baseline->colormap = 0;
		baseline->modelindex = entity->v.modelindex;
		baseline->movetype = entity->v.movetype;
		baseline->scale = entity->v.scale;
		baseline->solid = entity->v.solid;
		baseline->framerate = entity->v.framerate;
		baseline->gravity = entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int field;
}
entity_field_alias_t;

#define FIELD_ORIGIN0 0
#define FIELD_ORIGIN1 1
#define FIELD_ORIGIN2 2
#define FIELD_ANGLES0 3
#define FIELD_ANGLES1 4
#define FIELD_ANGLES2 5

static entity_field_alias_t entity_field_alias[] =
{
	{ "origin[0]", 0 },
	{ "origin[1]", 0 },
	{ "origin[2]", 0 },
	{ "angles[0]", 0 },
	{ "angles[1]", 0 },
	{ "angles[2]", 0 },
};

void Entity_FieldInit(struct delta_s *pFields)
{
	entity_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN0].name);
	entity_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN1].name);
	entity_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN2].name);
	entity_field_alias[FIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES0].name);
	entity_field_alias[FIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES1].name);
	entity_field_alias[FIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES2].name);
}

void Entity_Encode(struct delta_s *pFields, const unsigned char *from, const unsigned char *to)
{
	static int initialized = 0;

	if (!initialized)
	{
		Entity_FieldInit(pFields);
		initialized = 1;
	}

	entity_state_t *f = (entity_state_t *)from;
	entity_state_t *t = (entity_state_t *)to;
	int localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();

	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}

	if (t->impacttime != 0 && t->starttime != 0)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES2].field);
	}

	if (t->movetype == MOVETYPE_FOLLOW && t->aiment != 0)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
}

static entity_field_alias_t player_field_alias[] =
{
	{ "origin[0]", 0 },
	{ "origin[1]", 0 },
	{ "origin[2]", 0 },
};

void Player_FieldInit(struct delta_s *pFields)
{
	player_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN0].name);
	player_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN1].name);
	player_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN2].name);
}

void Player_Encode(struct delta_s *pFields, const unsigned char *from, const unsigned char *to)
{
	static int initialized = 0;

	if (!initialized)
	{
		Player_FieldInit(pFields);
		initialized = 1;
	}

	entity_state_t *f = (entity_state_t *)from;
	entity_state_t *t = (entity_state_t *)to;
	int localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();

	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN2].field);
	}

	if (t->movetype == MOVETYPE_FOLLOW && t->aiment != 0)
	{
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, player_field_alias[FIELD_ORIGIN2].field);
	}
}

#define CUSTOMFIELD_ORIGIN0 0
#define CUSTOMFIELD_ORIGIN1 1
#define CUSTOMFIELD_ORIGIN2 2
#define CUSTOMFIELD_ANGLES0 3
#define CUSTOMFIELD_ANGLES1 4
#define CUSTOMFIELD_ANGLES2 5
#define CUSTOMFIELD_SKIN 6
#define CUSTOMFIELD_SEQUENCE 7
#define CUSTOMFIELD_ANIMTIME 8

entity_field_alias_t custom_entity_field_alias[] =
{
	{ "origin[0]", 0 },
	{ "origin[1]", 0 },
	{ "origin[2]", 0 },
	{ "angles[0]", 0 },
	{ "angles[1]", 0 },
	{ "angles[2]", 0 },
	{ "skin", 0 },
	{ "sequence", 0 },
	{ "animtime", 0 },
};

void Custom_Entity_FieldInit(struct delta_s *pFields)
{
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].name);
	custom_entity_field_alias[CUSTOMFIELD_SKIN].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].name);
	custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field= DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].name);
	custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field= DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].name);
}

void Custom_Encode(struct delta_s *pFields, const unsigned char *from, const unsigned char *to)
{
	static int initialized = 0;

	if (!initialized)
	{
		Custom_Entity_FieldInit(pFields);
		initialized = 1;
	}

	entity_state_t *f = (entity_state_t *)from;
	entity_state_t *t = (entity_state_t *)to;
	int beamType = t->rendermode & 0xF;

	if (beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field);
	}

	if (beamType != BEAM_POINTS)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field);
	}

	if (beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field);
	}

	if ((int)f->animtime == (int)t->animtime)
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field);
}

void RegisterEncoders(void)
{
	DELTA_ADDENCODER("Entity_Encode", Entity_Encode);
	DELTA_ADDENCODER("Custom_Encode", Custom_Encode);
	DELTA_ADDENCODER("Player_Encode", Player_Encode);
}

int GetWeaponData(struct edict_s *player, struct weapon_data_s *info)
{
	#ifdef CLIENT_WEAPONS
	entvars_t *pev = &player->v;
	CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance(pev);
	memset(info, 0, 32 * sizeof(weapon_data_t));

	if (!pl)
		return 1;

	for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		if (pl->m_rgpPlayerItems[i+1])
		{
			CBasePlayerItem *pPlayerItem = pl->m_rgpPlayerItems[i + 1];

			while (pPlayerItem)
			{
				CBasePlayerWeapon *gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();

				if (gun && gun->UseDecrement())
				{
					ItemInfo II;
					memset(&II, 0, sizeof(II));
					gun->GetItemInfo(&II);

					weapon_data_t *item = &info[i];//from 0~4
					item->m_iId = II.iId;

					item->m_iClip = gun->m_iClip;
					item->iuser2 = gun->iMaxClip();
					item->iuser3 = (gun->pszAmmo1()) ? pl->m_rgAmmo[gun->m_iPrimaryAmmoType] : 0;
					item->iuser4 = (gun->pszAmmo1()) ? gun->iMaxAmmo1() : 0;

					item->m_fInReload = gun->m_fInReload;
					item->m_fInSpecialReload = gun->m_fInSpecialReload;
					item->m_fInZoom = gun->m_iShotsFired;
					item->m_iWeaponState = gun->m_iWeaponState;
					item->iuser1 = 0;

					item->m_flTimeWeaponIdle = max(gun->m_flTimeWeaponIdle, -0.001);
					item->m_flNextPrimaryAttack = max(gun->m_flNextPrimaryAttack, -0.001);
					item->m_flNextSecondaryAttack = max(gun->m_flNextSecondaryAttack, -0.001);
					item->m_flNextReload = max(gun->m_flNextReload, -0.001);
					item->m_fAimedDamage = max(gun->m_flDecreaseShotsFired, -0.001);
					item->m_fNextAimBonus = -0.001;
					if(gun->iItemSlot() == WEAPON_SLOT_KNIFE)
					{
						if(gun->m_iId == WEAPON_BOTTLE)
						{
							item->iuser1 = (((CBottle *)gun)->m_bBroken & 1) | (gun->m_bMeleeAttack & 2) | (gun->m_iMeleeCrit << 2);
						}
						else
						{
							item->iuser1 = (gun->m_bMeleeAttack & 1) | (gun->m_iMeleeCrit << 1);
						}
						item->m_fNextAimBonus = max(gun->m_flMeleeAttack, -0.001);
						
					}
					else if(gun->m_iId == WEAPON_MINIGUN)
					{
						CMinigun *minigun = (CMinigun *)gun;
						item->m_fNextAimBonus = max(minigun->m_fSpin, -0.001);
						item->iuser1 = minigun->m_iSpin;
					}
					else if(gun->m_iId == WEAPON_STICKYLAUNCHER)
					{
						CStickyLauncher *stickylauncher = (CStickyLauncher *)gun;
						item->m_fNextAimBonus = max(stickylauncher->m_fChargeTimer, -0.001);
					}
					else if(gun->m_iId == WEAPON_SNIPERIFLE)
					{
						CSniperifle *sniperifle = (CSniperifle *)gun;
						item->m_fNextAimBonus = max(sniperifle->m_fChargeTimer, -0.001);
						item->iuser1 = (sniperifle->m_fSpotActive & 1) | (sniperifle->m_iResumeZoom << 1);
					}
					else if(gun->m_iId == WEAPON_FLAMETHROWER)
					{
						CFlamethrower *flamethrower = (CFlamethrower *)gun;
						item->m_fNextAimBonus = max(flamethrower->m_flState, -0.001);
						item->iuser1 = (flamethrower->m_iAmmoConsumption & 1) | (flamethrower->m_iState << 1);
					}
					else if(gun->m_iId == WEAPON_MEDIGUN)
					{
						CMedigun *medigun = (CMedigun *)gun;
						item->iuser1 = medigun->m_bDelayedFire;
					}
					else if(gun->m_iId == WEAPON_BUILDPDA)
					{
						CBuildPDA *buildpda = (CBuildPDA *)gun;
						item->iuser1 = buildpda->m_bDelayedFire;
					}
					else if(gun->m_iId == WEAPON_DESTROYPDA)
					{
						CDestroyPDA *destroypda = (CDestroyPDA *)gun;
						item->iuser1 = destroypda->m_bDelayedFire;
					}
					else if(gun->m_iId == WEAPON_DISGUISEKIT)
					{
						CDisguiseKit *disguisekit = (CDisguiseKit *)gun;
						item->iuser1 = disguisekit->m_bDelayedFire;
					}
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
	#endif
	return 1;
}

void UpdateClientData(const struct edict_s *ent, int sendweapons, struct clientdata_s *cd)
{
	entvars_t *pev = (entvars_t *)&ent->v;
	CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance(pev);
	entvars_t *opev = NULL;

	if (pl->pev->iuser1 == OBS_IN_EYE && pl->m_hObserverTarget != NULL)
	{
		opev = pev;
		pev = pl->m_hObserverTarget->pev;
		pl = (CBasePlayer *)CBasePlayer::Instance(pev);
	}

	cd->flags = pev->flags;
	cd->health = pev->health;
	cd->viewmodel = MODEL_INDEX(STRING(pev->viewmodel));
	cd->waterlevel = pev->waterlevel;
	cd->watertype = pev->watertype;
	cd->weapons = pev->weapons;
	cd->origin = pev->origin;
	cd->velocity = pev->velocity;
	cd->view_ofs = pev->view_ofs;
	cd->punchangle = pev->punchangle;
	cd->bInDuck = pev->bInDuck;
	cd->flTimeStepSound = pev->flTimeStepSound;
	cd->flDuckTime = pev->flDuckTime;
	cd->flSwimTime = pev->flSwimTime;
	cd->waterjumptime = (const int)pev->teleport_time;

	strcpy(cd->physinfo, ENGINE_GETPHYSINFO(ent));

	cd->maxspeed = pev->maxspeed;
	cd->fov = pev->fov;
	cd->weaponanim = pev->weaponanim;
	cd->pushmsec = pev->pushmsec;

	if (opev)
	{
		cd->iuser1 = opev->iuser1;
		cd->iuser2 = opev->iuser2;
		cd->iuser3 = opev->iuser3;
	}
	else
	{
		cd->iuser1 = pev->iuser1;
		cd->iuser2 = pev->iuser2;
		cd->iuser3 = pev->iuser3;
	}

	cd->fuser2 = pev->fuser2;//for jump check

	if (sendweapons && pl)
	{
		cd->m_flNextAttack = pl->m_flNextAttack;

		int iCritBuff = pl->m_bCritBuff ? 1 : 0;
		int iCritBoost = pl->m_Cond.CritBoost.m_iStatus ? 1 : 0;
		cd->ammo_cells = iCritBuff | (iCritBoost << 1);
		cd->ammo_nails = pl->m_iDmgDone_Recent;
		cd->fuser1 = pl->m_fCritChance;

		if(pl->m_iClass == CLASS_SPY)
		{
			cd->vuser2.x = (float)((pl->m_iCloak << 1) | (pl->m_iDisguise & 1));
			cd->vuser3.y = pl->m_flCloakEnergy;
			if(pl->m_iDisguise)
			{
				cd->vuser2.y = (int)pl->m_iDisguiseTeam;
				cd->vuser2.z = (int)pl->m_iDisguiseClass;
				cd->vuser3.x = (int)pl->m_iDisguiseHealth;
				cd->maxspeed = (int)pl->m_iDisguiseMaxSpeed;
			}
		}
		if(pl->m_iClass == CLASS_MEDIC)
		{
			cd->vuser2.x = (float)pl->m_iUbercharge;
			cd->vuser3.y = pl->m_fUbercharge;
		}
		if(pl->m_iClass == CLASS_ENGINEER)
		{
			cd->vuser2.y = (float)pl->m_iBluePrintYaw;
			cd->vuser2.x = (float)pl->m_iCarryBluePrint;
			cd->vuser3.x = (float)pl->m_iMetal;
		}
		if(pl->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY])
		{
			if(pl->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY]->m_iId == WEAPON_STICKYLAUNCHER)
			{
				CStickyLauncher *pStickyLauncher = (CStickyLauncher *)pl->m_rgpPlayerItems[WEAPON_SLOT_SECONDARY];
				cd->ammo_shells = pStickyLauncher->m_iStickyNum;
			}
		}

		int cdflag = 0;

		if (pl->m_bAllowAttack == false)
			cdflag |= CDFLAG_LIMIT;

		if (g_pGameRules->m_bFreezePeriod)
			cdflag |= CDFLAG_FREEZE;

		if (!pl->IsObserver() && !opev)
			cd->iuser3 = cdflag;

		if (pl->m_pActiveItem)
		{
			CBasePlayerWeapon *gun = (CBasePlayerWeapon *)pl->m_pActiveItem->GetWeaponPtr();

			if (gun && gun->UseDecrement())
			{
				cd->m_iId = gun->m_iId;
				//cd->vuser4.x = gun->m_iPrimaryAmmoType;
				//cd->vuser4.y = pl->m_rgAmmo[gun->m_iSecondaryAmmoType];
			}
		}
	}
}

void CmdStart(const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed)
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = (CBasePlayer *) CBasePlayer::Instance(pev);

	if (!pl)
		return;

	if (pl->pev->groupinfo != 0)
		UTIL_SetGroupTrace(pl->pev->groupinfo, GROUP_OP_AND);

	pl->random_seed = random_seed;
}

void CmdEnd(const edict_t *player)
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = (CBasePlayer *) CBasePlayer::Instance(pev);

	if (!pl)
		return;

	if (pl->pev->groupinfo != 0)
		UTIL_UnsetGroupTrace();

	if (pev->flags & FL_DUCKING)
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
}

int ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
	return 0;
}

int GetHullBounds(int hullnumber, float *mins, float *maxs)
{
	int iret = 0;

	switch (hullnumber)
	{
		case 0:
		{
			mins = VEC_HULL_MIN;
			maxs = VEC_HULL_MAX;
			iret = 1;
			break;
		}

		case 1:
		{
			mins = VEC_DUCK_HULL_MIN;
			maxs = VEC_DUCK_HULL_MAX;
			iret = 1;
			break;
		}

		case 2:
		{
			mins = Vector(0, 0, 0);
			maxs = Vector(0, 0, 0);
			iret = 1;
			break;
		}
	}

	return iret;
}

void CreateInstancedBaselines(void)
{
	entity_state_t state;
	memset(&state, 0, sizeof(state));
}

int InconsistentFile(const edict_t *player, const char *filename, char *disconnect_message)
{
	if (CVAR_GET_FLOAT("mp_consistency") != 2)
		return 0;

	sprintf(disconnect_message, "Server is enforcing file consistency for %s\n", filename);
	return 1;
}

int AllowLagCompensation(void)
{
	return 1;
}

int ShouldCollide(edict_t *pentHit, edict_t *pentPass)
{
	CBaseEntity *pPass = CBaseEntity::Instance(pentPass);
	CBaseEntity *pHit = CBaseEntity::Instance(pentHit);
	if(!pPass || !pHit)
		return 1;

	return pPass->FShouldCollide(pHit);
}

void ViewAnglesForPlayBack(Vector &v)
{
	v.x = -(v.x / 3.0);
}