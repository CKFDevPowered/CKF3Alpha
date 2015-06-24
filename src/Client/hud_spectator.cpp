#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "CounterStrikeViewport.h"
#include "hltv.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "entity_types.h"

#include "com_model.h"
#include "demo_api.h"
#include "event_api.h"
#include "studio_util.h"
#include "screenfade.h"
#include "shared_util.h"

#pragma warning(disable: 4244)

extern "C" int iJumpSpectator;
extern "C" float vJumpOrigin[3];
extern "C" float vJumpAngles[3];

extern void V_GetInEyePos(int entity, float *origin, float *angles);
extern void V_ResetChaseCam(void);
extern void V_GetChasePos(int target, float *cl_angles, float *origin, float *angles);
extern float *GetClientColor(int clientIndex);

extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_cl_angles;
extern vec3_t v_sim_org;

void DuckMessage(const char *str)
{
	static char tempString[128];
	static client_textmessage_t tst;

	strncpy(tempString, str, sizeof(tempString));

	tst.y = 0.7;
	tst.fadeout = 0.7;
	tst.pName = "Spec_Duck";
	tst.effect = 2;
	tst.r1 = 40;
	tst.g1 = 255;
	tst.b1 = 40;
	tst.a1 = 200;
	tst.r2 = 0;
	tst.g2 = 255;
	tst.b2 = 0;
	tst.a2 = 200;

	tst.x = -1.0;
	tst.fadein = 0.01;
	tst.fxtime = 0.07;
	tst.holdtime = 6.0;
	tst.pMessage = tempString;

	gHUD.m_Message.MessageAdd(&tst);
}

const char *GetSpectatorLabel(int iMode)
{
	switch (iMode)
	{
		case OBS_CHASE_LOCKED:
		{
			return "#OBS_CHASE_LOCKED";
		}

		case OBS_CHASE_FREE:
		{
			return "#OBS_CHASE_FREE";
		}
		case OBS_ROAMING:
		{
			return "#OBS_ROAMING";
		}

		case OBS_IN_EYE:
		{
			return "#OBS_IN_EYE";
		}

		case OBS_MAP_FREE:
		{
			return "#OBS_MAP_FREE";
		}

		case OBS_MAP_CHASE:
		{
			return "#OBS_MAP_CHASE";
		}

		case OBS_NONE:
		default:
		{
			return "#OBS_NONE";
		}
	}
}

void SpectatorMode(void)
{
	if (gEngfuncs.Cmd_Argc() <= 1)
	{
		gEngfuncs.Con_Printf("usage:  spec_mode <Main Mode> [<Inset Mode>]\n");
		return;
	}

	if (gEngfuncs.Cmd_Argc() == 2)
	{
		gHUD.m_Spectator.SetModes(atoi(gEngfuncs.Cmd_Argv(1)), -1);
	}
	else if (gEngfuncs.Cmd_Argc() == 3 && atoi(gEngfuncs.Cmd_Argv(2)) != -1)
	{
		gHUD.m_Spectator.SetModes(atoi(gEngfuncs.Cmd_Argv(1)), atoi(gEngfuncs.Cmd_Argv(2)));
	}
	else if (gEngfuncs.Cmd_Argc() == 3 && atoi(gEngfuncs.Cmd_Argv(2)) == -1)
	{
		if (gHUD.m_Spectator.m_pip->value == INSET_OFF)
			gHUD.m_Spectator.SetModes(-1, INSET_CHASE_FREE);
		else
			gHUD.m_Spectator.SetModes(-1, INSET_OFF);
	}
}

void SpectatorToggleInset(void)
{
	gHUD.m_Spectator.SetModes(-1, gHUD.m_Spectator.ToggleInset(false));
}

void SpectatorSpray(void)
{
	vec3_t forward;
	char string[128];

	if (!gEngfuncs.IsSpectateOnly())
		return;

	AngleVectors(v_angles,forward,NULL,NULL);
	VectorScale(forward, 128, forward);
	VectorAdd(forward, v_origin, forward);

	pmtrace_t *trace = gEngfuncs.PM_TraceLine(v_origin, forward, PM_TRACELINE_PHYSENTSONLY, 2, -1);

	if (trace->fraction != 1.0)
	{
		sprintf(string, "drc_spray %.2f %.2f %.2f %i", trace->endpos[0], trace->endpos[1], trace->endpos[2], trace->ent);
		gEngfuncs.pfnServerCmd(string);
	}
}

void SpectatorHelp(void)
{
	if (gViewPortInterface)
		gViewPortInterface->ShowVGUIMenu(MENU_SPECHELP);
}

void SpectatorMenu(void)
{
	if (gEngfuncs.Cmd_Argc() <= 1)
	{
		gEngfuncs.Con_Printf("usage:  spec_menu <0|1>\n");
		return;
	}

	if (atoi(gEngfuncs.Cmd_Argv(1)) != 0)
		gViewPortInterface->ShowSpectatorGUI();
	else
		gViewPortInterface->DeactivateSpectatorGUI();
}

void ToggleScores(void)
{
	if (gViewPortInterface)
	{
		if (gViewPortInterface->IsScoreBoardVisible())
		{
			gViewPortInterface->HideScoreBoard();
		}
		else
		{
			gViewPortInterface->ShowScoreBoard();
		}
	}
}

void SpectatorToggleDrawNames(void)
{
	if (gHUD.m_Spectator.m_drawnames->value)
		gEngfuncs.Cvar_SetValue("spec_drawnames_internal", 0);
	else
		gEngfuncs.Cvar_SetValue("spec_drawnames_internal", 1);
}

void SpectatorToggleDrawCone(void)
{
	if (gHUD.m_Spectator.m_drawcone->value)
		gEngfuncs.Cvar_SetValue("spec_drawcone_internal", 0);
	else
		gEngfuncs.Cvar_SetValue("spec_drawcone_internal", 1);
}

void SpectatorToggleDrawStatus(void)
{
	if (gHUD.m_Spectator.m_drawstatus->value)
		gEngfuncs.Cvar_SetValue("spec_drawstatus_internal", 0);
	else
		gEngfuncs.Cvar_SetValue("spec_drawstatus_internal", 1);
}

void SpectatorToggleAutoDirector(void)
{
	if (gHUD.m_Spectator.m_autoDirector->value)
		gEngfuncs.Cvar_SetValue("spec_autodirector_internal", 0);
	else
		gEngfuncs.Cvar_SetValue("spec_autodirector_internal", 1);
}

int CHudSpectator::Init(void)
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;
	m_flNextObserverInput = 0.0f;
	m_zoomDelta = 0.0f;
	m_moveDelta = 0.0f;
	m_FOV = 90.0f;
	m_chatEnabled = (gHUD.m_SayText.m_HUD_saytext->value != 0);
	iJumpSpectator = 0;
	m_lastAutoDirector = -1.0f;

	memset(&m_OverviewData, 0, sizeof(m_OverviewData));
	memset(&m_OverviewEntities, 0, sizeof(m_OverviewEntities));
	m_lastPrimaryObject = m_lastSecondaryObject = 0;

	Cmd_HookCmd("spec_mode", SpectatorMode);
	Cmd_HookCmd("spec_toggleinset", SpectatorToggleInset);
	Cmd_HookCmd("spec_decal", SpectatorSpray);
	Cmd_HookCmd("spec_help", SpectatorHelp);
	Cmd_HookCmd("spec_menu", SpectatorMenu);
	Cmd_HookCmd("togglescores", ToggleScores);
	Cmd_HookCmd("spec_drawnames", SpectatorToggleDrawNames);
	Cmd_HookCmd("spec_drawcone", SpectatorToggleDrawCone);
	Cmd_HookCmd("spec_drawstatus", SpectatorToggleDrawStatus);
	Cmd_HookCmd("spec_autodirector", SpectatorToggleAutoDirector);

	m_drawnames = gEngfuncs.pfnGetCvarPointer("spec_drawnames_internal");
	m_drawcone = gEngfuncs.pfnGetCvarPointer("spec_drawcone_internal");
	m_drawstatus = gEngfuncs.pfnGetCvarPointer("spec_drawstatus_internal");
	m_autoDirector = gEngfuncs.pfnGetCvarPointer("spec_autodirector_internal");
	m_mode = gEngfuncs.pfnGetCvarPointer("spec_mode_internal");
	m_pip = gEngfuncs.pfnGetCvarPointer("spec_pip");
	m_scoreboard = gEngfuncs.pfnGetCvarPointer("spec_scoreboard");

	if (!m_drawnames || !m_drawcone || !m_drawstatus || !m_autoDirector || !m_pip)
	{
		gEngfuncs.Con_Printf("ERROR! Couldn't register all spectator variables.\n");
		return 0;
	}

	return 1;
}

void UTIL_StringToVector(float *pVector, const char *pString)
{
	char *pstr, *pfront, tempString[128];
	int j;

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < 3; j++)
	{
		pVector[j] = atof(pfront);

		while (*pstr && *pstr != ' ')
			pstr++;

		if (!*pstr)
			break;

		pstr++;
		pfront = pstr;
	}

	if (j < 2)
	{
		for (j = j + 1; j < 3; j++)
			pVector[j] = 0;
	}
}

int UTIL_FindEntityInMap(char *name, float *origin, float *angle)
{
	int n,found = 0;
	char keyname[256];
	char token[1024];

	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex(0);

	if (!pEnt)
		return 0;

	if (!pEnt->model)
		return 0;

	char *data = pEnt->model->entities;

	while (data)
	{
		data = gEngfuncs.COM_ParseFile(data, token);

		if ((token[0] == '}') || (token[0] == 0))
			break;

		if (!data)
		{
			gEngfuncs.Con_DPrintf("UTIL_FindEntityInMap: EOF without closing brace\n");
			return 0;
		}

		if (token[0] != '{')
		{
			gEngfuncs.Con_DPrintf("UTIL_FindEntityInMap: expected {\n");
			return 0;
		}

		while (1)
		{
			data = gEngfuncs.COM_ParseFile(data, token);

			if (token[0] == '}')
				break;

			if (!data)
			{
				gEngfuncs.Con_DPrintf("UTIL_FindEntityInMap: EOF without closing brace\n");
				return 0;
			}

			strcpy (keyname, token);

			n = strlen(keyname);

			while (n && keyname[n - 1] == ' ')
			{
				keyname[n - 1] = 0;
				n--;
			}

			data = gEngfuncs.COM_ParseFile(data, token);

			if (!data)
			{
				gEngfuncs.Con_DPrintf("UTIL_FindEntityInMap: EOF without closing brace\n");
				return 0;
			}

			if (token[0] == '}')
			{
				gEngfuncs.Con_DPrintf("UTIL_FindEntityInMap: closing brace without data");
				return 0;
			}

			if (!strcmp(keyname,"classname"))
			{
				if (!strcmp(token, name))
					found = 1;
			}

			if (!strcmp(keyname, "angle"))
			{
				float y = atof(token);

				if (y >= 0)
				{
					angle[0] = 0.0f;
					angle[1] = y;
				}
				else if ((int)y == -1)
				{
					angle[0] = -90.0f;
					angle[1] = 0.0f;;
				}
				else
				{
					angle[0] = 90.0f;
					angle[1] = 0.0f;
				}

				angle[2] = 0.0f;
			}

			if (!strcmp(keyname, "angles"))
			{
				UTIL_StringToVector(angle, token);
			}

			if (!strcmp(keyname,"origin"))
			{
				UTIL_StringToVector(origin, token);
			}
		}

		if (found)
			return 1;
	}

	return 0;
}

void CHudSpectator::SetSpectatorStartPosition(void)
{
	if (UTIL_FindEntityInMap("trigger_camera", m_cameraOrigin, m_cameraAngles))
	{
		iJumpSpectator = 1;
	}
	else if (UTIL_FindEntityInMap("info_player_start", m_cameraOrigin, m_cameraAngles))
	{
		iJumpSpectator = 1;
	}
	else if (UTIL_FindEntityInMap("info_player_deathmatch", m_cameraOrigin, m_cameraAngles))
	{
		iJumpSpectator = 1;
	}
	else if (UTIL_FindEntityInMap("info_player_coop", m_cameraOrigin, m_cameraAngles))
	{
		iJumpSpectator = 1;
	}
	else
	{
		VectorCopy(vec3_origin, m_cameraOrigin);
		VectorCopy(vec3_origin, m_cameraAngles);
	}

	VectorCopy(m_cameraOrigin, vJumpOrigin);
	VectorCopy(m_cameraAngles, vJumpAngles);

	iJumpSpectator = 1;
}

void CHudSpectator::SetCameraView(vec3_t pos, vec3_t angle, float fov)
{
	m_FOV = fov;
	VectorCopy(pos, vJumpOrigin);
	VectorCopy(angle, vJumpAngles);
	gEngfuncs.SetViewAngles(vJumpAngles);
	iJumpSpectator = 1;
}

void CHudSpectator::AddWaypoint(float time, vec3_t pos, vec3_t angle, float fov, int flags)
{
	if (!flags == 0 && time == 0.0f)
	{
		SetCameraView(pos, angle, fov);
		return;
	}

	if (m_NumWayPoints >= MAX_CAM_WAYPOINTS)
	{
		gEngfuncs.Con_Printf("Too many camera waypoints!\n");
		return;
	}

	VectorCopy(angle, m_CamPath[m_NumWayPoints].angle);
	VectorCopy(pos, m_CamPath[m_NumWayPoints].position);
	m_CamPath[m_NumWayPoints].flags = flags;
	m_CamPath[m_NumWayPoints].fov = fov;
	m_CamPath[m_NumWayPoints].time = time;

	gEngfuncs.Con_DPrintf("Added waypoint %i\n", m_NumWayPoints);
	m_NumWayPoints++;
}

void CHudSpectator::SetWayInterpolation(cameraWayPoint_t *prev, cameraWayPoint_t *start, cameraWayPoint_t *end, cameraWayPoint_t *next)
{
	m_WayInterpolation.SetViewAngles(start->angle, end->angle);
	m_WayInterpolation.SetFOVs(start->fov, end->fov);
	m_WayInterpolation.SetSmoothing((start->flags & DRC_FLAG_SLOWSTART) != 0, (start->flags & DRC_FLAG_SLOWEND) != 0);

	if (prev && next)
	{
		m_WayInterpolation.SetWaypoints(&prev->position, start->position, end->position, &next->position);
	}
	else if (prev)
	{
		m_WayInterpolation.SetWaypoints(&prev->position, start->position, end->position, NULL);
	}
	else if (next)
	{
		m_WayInterpolation.SetWaypoints(NULL, start->position, end->position, &next->position);
	}
	else
	{
		m_WayInterpolation.SetWaypoints(NULL, start->position, end->position, NULL);
	}
}

bool CHudSpectator::GetDirectorCamera(vec3_t &position, vec3_t &angle)
{
	float now = gHUD.m_flTime;
	float fov = 90.0f;

	if (m_ChaseEntity)
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex(m_ChaseEntity);

		if (ent)
		{
			vec3_t vt = ent->curstate.origin;

			if (m_ChaseEntity <= gEngfuncs.GetMaxClients())
			{
				if (ent->curstate.solid == SOLID_NOT)
				{
					vt[2] += -8;
				}
				else if (ent->curstate.usehull == 1)
				{
					vt[2] += 12;
				}
				else
				{
					vt[2] += 17;
				}
			}

			vt = vt - position;
			VectorAngles(vt, angle);
			angle[0] = -angle[0];
			return true;
		}
		else
		{
			return false;
		}
	}

	if (!m_IsInterpolating)
		return false;

	if (m_WayPoint < 0 || m_WayPoint >= (m_NumWayPoints - 1))
		return false;

	cameraWayPoint_t *wp1 = &m_CamPath[m_WayPoint];
	cameraWayPoint_t *wp2 = &m_CamPath[m_WayPoint + 1];

	if (now < wp1->time)
		return false;

	while (now > wp2->time)
	{
		m_WayPoint++;

		if (m_WayPoint >= (m_NumWayPoints - 1))
		{
			m_IsInterpolating = false;
			return false;
		}

		wp1 = wp2;
		wp2 = &m_CamPath[m_WayPoint + 1];

		if (m_WayPoint > 0)
		{
			if (m_WayPoint < (m_NumWayPoints - 1))
			{
				SetWayInterpolation(&m_CamPath[m_WayPoint - 1], wp1, wp2, &m_CamPath[m_WayPoint + 2]);
			}
			else
			{
				SetWayInterpolation(&m_CamPath[m_WayPoint - 1], wp1, wp2, NULL);
			}
		}
		else if (m_WayPoint < (m_NumWayPoints - 1))
		{
			SetWayInterpolation(NULL, wp1, wp2, &m_CamPath[m_WayPoint + 2]);
		}
		else
		{
			SetWayInterpolation(NULL, wp1, wp2, NULL);
		}
	}

	if (wp2->time <= wp1->time)
		return false;

	float fraction = (now - wp1->time) / (wp2->time - wp1->time);

	if (fraction < 0.0f)
		fraction = 0.0f;
	else if (fraction > 1.0f)
		fraction = 1.0f;

	m_WayInterpolation.Interpolate(fraction, position, angle, &fov);
	gEngfuncs.Con_Printf("Interpolate time: %.2f, fraction %.2f, point : %.2f,%.2f,%.2f\n", now, fraction, position[0], position[1], position[2]);

	SetCameraView(position, angle, fov);
	return true;
}

int CHudSpectator::VidInit(void)
{
	m_hsprPlayer = SPR_Load("sprites/iplayer.spr");
	m_hsprPlayerBlue = SPR_Load("sprites/iplayerblue.spr");
	m_hsprPlayerRed = SPR_Load("sprites/iplayerred.spr");
	m_hsprPlayerDead = SPR_Load("sprites/iplayerdead.spr");
	m_hsprPlayerVIP = SPR_Load("sprites/iplayervip.spr");
	m_hsprPlayerC4 = SPR_Load("sprites/iplayerc4.spr");
	m_hsprUnkownMap = SPR_Load("sprites/tile.spr");
	m_hsprBomb = SPR_Load("sprites/ic4.spr");
	m_hsprBackpack = SPR_Load("sprites/ibackpack.spr");
	m_hsprBeam = SPR_Load("sprites/laserbeam.spr");
	m_hsprCamera = SPR_Load("sprites/camera.spr");
	m_hsprHostage = SPR_Load("sprites/ihostage.spr");

	return 1;
}

float CHudSpectator::GetFOV(void)
{
	return m_FOV;
}

int CHudSpectator::Draw(float flTime)
{
	int lx;

	char string[256];
	float *color;

	if (!g_iUser1)
	{
		if (gViewPortInterface->IsSpectatorGUIVisible())
			gViewPortInterface->HideSpectatorGUI();

		return 0;
	}

	if (m_lastAutoDirector != m_autoDirector->value)
	{
		m_lastAutoDirector = m_autoDirector->value;

		gEngfuncs.pfnClientCmd(SharedVarArgs("spec_set_ad %f", m_autoDirector->value));

		if ((m_lastAutoDirector != 0.0f) && (g_iUser1 == OBS_CHASE_FREE))
		{
			SetModes(OBS_CHASE_LOCKED, -1);
		}
		else if ((m_lastAutoDirector == 0.0f) && (g_iUser1 == OBS_CHASE_LOCKED))
		{
			SetModes(OBS_CHASE_FREE, -1);
		}
	}

	if ((m_zoomDelta != 0.0f) && (g_iUser1 == OBS_MAP_FREE))
	{
		m_mapZoom += m_zoomDelta;

		if (m_mapZoom > 3.0f)
			m_mapZoom = 3.0f;

		if (m_mapZoom < 0.5f)
			m_mapZoom = 0.5;
	}

	if ((m_moveDelta != 0.0f) && (g_iUser1 != OBS_ROAMING))
	{
		vec3_t right;
		AngleVectors(v_angles, NULL, right, NULL);
		VectorNormalize(right);
		VectorScale(right, m_moveDelta, right);
		VectorAdd(m_mapOrigin, right, m_mapOrigin);
	}

	if (g_iUser1 != m_mode->value)
	{
		gEngfuncs.Cvar_SetValue("spec_mode_internal", g_iUser1);
	}

	if (g_iUser1 < OBS_MAP_FREE)
		return 1;

	if (!m_drawnames->value)
		return 1;

	gViewPortInterface->GetAllPlayersInfo();

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_vPlayerPos[i][2] < 0)
			continue;

		if (m_pip->value != INSET_OFF)
		{
			if (m_vPlayerPos[i][0] > m_OverviewData.insetWindowX && m_vPlayerPos[i][1] > m_OverviewData.insetWindowY && m_vPlayerPos[i][0] < m_OverviewData.insetWindowX + m_OverviewData.insetWindowWidth && m_vPlayerPos[i][1] < m_OverviewData.insetWindowY + m_OverviewData.insetWindowHeight)
				continue;
		}

		color = GetClientColor(i + 1);
		sprintf(string, "%s", g_PlayerInfoList[i + 1].name);
		lx = strlen(string) * 3;

		if (m_pip->value != INSET_OFF)
		{
			if (m_vPlayerPos[i][0] - lx > m_OverviewData.insetWindowX && m_vPlayerPos[i][1] > m_OverviewData.insetWindowY && m_vPlayerPos[i][0] - lx < m_OverviewData.insetWindowX + m_OverviewData.insetWindowWidth && m_vPlayerPos[i][1] < m_OverviewData.insetWindowY + m_OverviewData.insetWindowHeight)
				continue;
		}

		gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
		DrawConsoleString(m_vPlayerPos[i][0] - lx, m_vPlayerPos[i][1], string);
	}

	return 1;
}

bool CHudSpectator::DirectorMessage(int iSize, void *pbuf)
{
	float f1, f2;
	char *string;
	vec3_t v1, v2;
	int i1, i2, i3;

	BEGIN_READ(pbuf, iSize);

	int cmd = READ_BYTE();

	switch (cmd)
	{
		case DRC_CMD_START:
		{
			g_iPlayerClass = 0;
			g_iTeamNumber = 0;

			gHUD.MsgFunc_InitHUD(NULL, 0, NULL);
			gHUD.MsgFunc_ResetHUD(NULL, 0, NULL);

			DuckMessage("#Spec_Duck");
			break;
		}

		case DRC_CMD_EVENT:
		{
			m_lastPrimaryObject = READ_WORD();
			m_lastSecondaryObject = READ_WORD();
			m_iObserverFlags = READ_LONG();

			if (m_autoDirector->value)
			{
				if ((g_iUser2 != m_lastPrimaryObject) || (g_iUser3 != m_lastSecondaryObject))
					V_ResetChaseCam();

				g_iUser2 = m_lastPrimaryObject;
				g_iUser3 = m_lastSecondaryObject;
				m_IsInterpolating = false;
				m_ChaseEntity = 0;
			}

			return false;
		}

		case DRC_CMD_MODE:
		{
			if (m_autoDirector->value)
				SetModes(READ_BYTE(), -1);

			break;
		}

		case DRC_CMD_CAMERA:
		{
			v1[0] = READ_COORD();
			v1[1] = READ_COORD();
			v1[2] = READ_COORD();

			v2[0] = READ_COORD();
			v2[1] = READ_COORD();
			v2[2] = READ_COORD();
			f1 = READ_BYTE();
			i1 = READ_WORD();

			if (m_autoDirector->value)
			{
				SetModes(OBS_ROAMING, -1);
				SetCameraView(v1, v2, f1);
				m_ChaseEntity = i1;
			}

			break;
		}

		case DRC_CMD_MESSAGE:
		{
			client_textmessage_t *msg = &m_HUDMessages[m_lastHudMessage];

			msg->effect = READ_BYTE();

			UnpackRGB((int&)msg->r1, (int&)msg->g1, (int&)msg->b1, READ_LONG());

			msg->r2 = msg->r1;
			msg->g2 = msg->g1;
			msg->b2 = msg->b1;
			msg->a2 = msg->a1 = 0xFF;

			msg->x = READ_FLOAT();
			msg->y = READ_FLOAT();
							
			msg->fadein = READ_FLOAT();
			msg->fadeout = READ_FLOAT();
			msg->holdtime = READ_FLOAT();
			msg->fxtime = READ_FLOAT();

			strncpy(m_HUDMessageText[m_lastHudMessage], READ_STRING(), 128);
			m_HUDMessageText[m_lastHudMessage][127] = 0;

			msg->pMessage = m_HUDMessageText[m_lastHudMessage];
			msg->pName = "HUD_MESSAGE";

			gHUD.m_Message.MessageAdd(msg);

			m_lastHudMessage++;
			m_lastHudMessage %= MAX_SPEC_HUD_MESSAGES;
			break;
		}

		case DRC_CMD_SOUND:
		{
			string = READ_STRING();
			f1 = READ_FLOAT();

			gEngfuncs.pEventAPI->EV_PlaySound(0, v_origin, CHAN_BODY, string, f1, ATTN_NORM, 0, PITCH_NORM);
			break;
		}

		case DRC_CMD_TIMESCALE:
		{
			f1 = READ_FLOAT();
			break;
		}

		case DRC_CMD_STATUS:
		{
			READ_LONG();
			m_iSpectatorNumber = READ_LONG();
			READ_WORD();

			gViewPortInterface->UpdateSpectatorPanel();
			break;
		}

		case DRC_CMD_BANNER:
		{
			gViewPortInterface->SetSpectatorBanner(READ_STRING());
			gViewPortInterface->UpdateSpectatorPanel();
			break;
		}

		case DRC_CMD_STUFFTEXT:
		{
			ClientCmd(READ_STRING());
			break;
		}

		case DRC_CMD_CAMPATH:
		{
			v1[0] = READ_COORD();
			v1[1] = READ_COORD();
			v1[2] = READ_COORD();

			v2[0] = READ_COORD();
			v2[1] = READ_COORD();
			v2[2] = READ_COORD();
			f1 = READ_BYTE();
			i1 = READ_BYTE();

			if (m_autoDirector->value)
			{
				SetModes(OBS_ROAMING, -1);
				SetCameraView(v1, v2, f1);
			}

			break;
		}

		case DRC_CMD_WAYPOINTS:
		{
			i1 = READ_BYTE();

			m_NumWayPoints = 0;
			m_WayPoint = 0;

			for (i2 = 0; i2 < i1; i2++)
			{
				f1 = gHUD.m_flTime + (float)(READ_SHORT()) / 100.0f;

				v1[0] = READ_COORD();
				v1[1] = READ_COORD();
				v1[2] = READ_COORD();

				v2[0] = READ_COORD();
				v2[1] = READ_COORD();
				v2[2] = READ_COORD();
				f2 = READ_BYTE();
				i3 = READ_BYTE();

				AddWaypoint(f1, v1, v2, f2, i3);
			}

			if (!m_autoDirector->value)
			{
				m_NumWayPoints = 0;
				break;
			}

			SetModes(OBS_ROAMING, -1);

			m_IsInterpolating = true;

			if (m_NumWayPoints > 2)
			{
				SetWayInterpolation(NULL, &m_CamPath[0], &m_CamPath[1], &m_CamPath[2]);
			}
			else
			{
				SetWayInterpolation(NULL, &m_CamPath[0], &m_CamPath[1], NULL);
			}

			break;
		}

		default: gEngfuncs.Con_DPrintf("CHudSpectator::DirectorMessage: unknown command %i.\n", cmd);
	}

	return true;
}

void CHudSpectator::FindNextPlayer(bool bReverse)
{
	int iStart;
	cl_entity_t *pEnt = NULL;

	if (gEngfuncs.IsSpectateOnly())
	{
		char cmdstring[32];
		sprintf(cmdstring, "follownext %i", bReverse ? 1 : 0);
		gEngfuncs.pfnServerCmd(cmdstring);
		return;
	}

	if (g_iUser2)
		iStart = g_iUser2;
	else
		iStart = 1;

	int iCurrent = iStart;
	int iDir = bReverse ? -1 : 1;

	gViewPortInterface->GetAllPlayersInfo();

	do
	{
		iCurrent += iDir;

		if (iCurrent > MAX_PLAYERS)
			iCurrent = 1;

		if (iCurrent < 1)
			iCurrent = MAX_PLAYERS;

		pEnt = gEngfuncs.GetEntityByIndex(iCurrent);

		if (!IsActivePlayer(pEnt))
			continue;

		g_iUser2 = iCurrent;
		break;

	}
	while (iCurrent != iStart);

	if (!g_iUser2)
	{
		gEngfuncs.Con_DPrintf("No observer targets.\n");

		VectorCopy(m_cameraOrigin, vJumpOrigin);
		VectorCopy(m_cameraAngles, vJumpAngles);
	}
	else
	{
		VectorCopy(pEnt->origin, vJumpOrigin);
		VectorCopy(pEnt->angles, vJumpAngles);
	}

	gViewPortInterface->UpdateSpectatorPanel();
	iJumpSpectator = 1;
}

void CHudSpectator::FindPlayer(const char *name)
{
	if (!gEngfuncs.IsSpectateOnly())
	{
		char cmdstring[32];
		sprintf(cmdstring,"follow %s",name);
		gEngfuncs.pfnServerCmd(cmdstring);
		return;
	}

	g_iUser2 = 0;
	gViewPortInterface->GetAllPlayersInfo();

	cl_entity_t *pEnt = NULL;

	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		pEnt = gEngfuncs.GetEntityByIndex(i);

		if (!IsActivePlayer(pEnt))
			continue;

		if (!stricmp(g_PlayerInfoList[pEnt->index].name,name))
		{
			g_iUser2 = i;
			break;
		}
	}

	if (!g_iUser2)
	{
		gEngfuncs.Con_DPrintf("No observer targets.\n");

		VectorCopy(m_cameraOrigin, vJumpOrigin);
		VectorCopy(m_cameraAngles, vJumpAngles);
	}
	else
	{
		VectorCopy(pEnt->origin, vJumpOrigin);
		VectorCopy(pEnt->angles, vJumpAngles);
	}

	gViewPortInterface->UpdateSpectatorPanel();
	iJumpSpectator = 1;
}

void CHudSpectator::HandleButtonsDown(int ButtonPressed)
{
	double time = gEngfuncs.GetClientTime();

	int newMainMode = g_iUser1;
	int newInsetMode = m_pip->value;

	if (!gViewPortInterface)
		return;

	if (gHUD.m_iIntermission)
		return;

	if (!g_iUser1)
		return;

	if (gEngfuncs.pDemoAPI->IsPlayingback() && !gEngfuncs.IsSpectateOnly())
		return;

	if (m_flNextObserverInput > time)
		return;

	if (ButtonPressed & IN_DUCK)
	{
		if (!gViewPortInterface->IsSpectatorBarVisible())
			gViewPortInterface->ShowSpectatorGUIBar();
	}

	if (ButtonPressed & IN_USE)
	{
		newInsetMode = ToggleInset(true);
	}

	if (gEngfuncs.IsSpectateOnly())
	{
		if (ButtonPressed & IN_JUMP)
		{
			if (g_iUser1 == OBS_CHASE_LOCKED)
				newMainMode = OBS_CHASE_FREE;
			else if (g_iUser1 == OBS_CHASE_FREE)
				newMainMode = OBS_IN_EYE;
			else if (g_iUser1 == OBS_IN_EYE)
				newMainMode = OBS_ROAMING;
			else if (g_iUser1 == OBS_ROAMING)
				newMainMode = OBS_MAP_FREE;
			else if (g_iUser1 == OBS_MAP_FREE)
				newMainMode = OBS_MAP_CHASE;
			else
				newMainMode = OBS_CHASE_FREE;
		}

		if (ButtonPressed & (IN_ATTACK | IN_ATTACK2))
		{
			FindNextPlayer((ButtonPressed & IN_ATTACK2) ? true : false);

			if (g_iUser1 == OBS_ROAMING)
			{
				gEngfuncs.SetViewAngles(vJumpAngles);
				iJumpSpectator = 1;
				gHUD.m_Radar.m_iPlayerLastPointedAt = g_iUser2;
				g_PlayerExtraInfo[g_iUser2].showhealth = gHUD.m_flTime = 3;
			}

			m_autoDirector->value = 0.0f;
		}
	}

	SetModes(newMainMode, newInsetMode);

	if (g_iUser1 == OBS_MAP_FREE)
	{
		if (ButtonPressed & IN_FORWARD)
			m_zoomDelta = 0.01f;

		if (ButtonPressed & IN_BACK)
			m_zoomDelta = -0.01f;

		if (ButtonPressed & IN_MOVELEFT)
			m_moveDelta = -12.0f;

		if (ButtonPressed & IN_MOVERIGHT)
			m_moveDelta = 12.0f;
	}

	m_flNextObserverInput = time + 0.2;
}

void CHudSpectator::HandleButtonsUp(int ButtonPressed)
{
	if (!gViewPortInterface)
		return;

	if (!gViewPortInterface->IsSpectatorGUIVisible())
		return;

	if (ButtonPressed & (IN_FORWARD | IN_BACK))
		m_zoomDelta = 0.0f;

	if (ButtonPressed & (IN_MOVELEFT | IN_MOVERIGHT))
		m_moveDelta = 0.0f;
}

void CHudSpectator::SetModes(int iNewMainMode, int iNewInsetMode)
{
	if (iNewMainMode == -1)
		iNewMainMode = g_iUser1;

	if (iNewInsetMode == -1)
		iNewInsetMode = m_pip->value;

	m_pip->value = iNewInsetMode;

	if (gViewPortInterface->GetClientDllInterface()->InIntermission())
		m_pip->value = INSET_OFF;

	if (iNewMainMode < OBS_CHASE_LOCKED || iNewMainMode > OBS_MAP_CHASE)
	{
		gEngfuncs.Con_Printf("Invalid spectator mode.\n");
		return;
	}

	CheckSettings();

	m_IsInterpolating = false;
	m_ChaseEntity = 0;

	if (iNewMainMode != g_iUser1)
	{
		if (!gEngfuncs.IsSpectateOnly())
		{
			if (iNewMainMode == OBS_CHASE_FREE && gViewPortInterface->GetClientDllInterface()->InIntermission() != 2)
				m_autoDirector->value = 0;

			char cmdstring[32];
			sprintf(cmdstring,"specmode %i",iNewMainMode );
			gEngfuncs.pfnServerCmd(cmdstring);
			return;
		}

		if (!g_iUser2 && (iNewMainMode != OBS_ROAMING))
		{
			if (IsActivePlayer(gEngfuncs.GetEntityByIndex(m_lastPrimaryObject)))
			{
				g_iUser2 = m_lastPrimaryObject;
				g_iUser3 = m_lastSecondaryObject;
			}
			else
			{
				FindNextPlayer(false);
			}
		}

		switch (iNewMainMode)
		{
			case OBS_CHASE_LOCKED:
			{
				g_iUser1 = OBS_CHASE_LOCKED;
				break;
			}

			case OBS_CHASE_FREE:
			{
				g_iUser1 = OBS_CHASE_FREE;
				m_autoDirector->value = 0;
				break;
			}

			case OBS_ROAMING:
			{
				g_iUser1 = OBS_ROAMING;

				if (g_iUser2)
				{
					V_GetChasePos(g_iUser2, v_cl_angles, vJumpOrigin, vJumpAngles);
					gEngfuncs.SetViewAngles(vJumpAngles);
					iJumpSpectator = 1;
				}

				break;
			}

			case OBS_IN_EYE:
			{
				g_iUser1 = OBS_IN_EYE;
				break;
			}

			case OBS_MAP_FREE:
			{
				g_iUser1 = OBS_MAP_FREE;

				m_mapZoom = m_OverviewData.zoom;
				m_mapOrigin = m_OverviewData.origin;
				break;
			}

			case OBS_MAP_CHASE:
			{
				g_iUser1 = OBS_MAP_CHASE;

				m_mapZoom = m_OverviewData.zoom;
				m_mapOrigin = m_OverviewData.origin;
				break;
			}
		}

		if (g_iUser1 == OBS_ROAMING)
		{
			if (gEngfuncs.pfnGetCvarFloat("cl_observercrosshair") != 0.0)
			{
				SetCrosshair(gHUD.m_Ammo.m_hObserverCrosshair, gHUD.m_Ammo.m_rcObserverCrosshair, 255, 255, 255);
			}
			else
			{
				static wrect_t nullrc;
				SetCrosshair(0, nullrc, 0, 0, 0);
			}
		}

		gViewPortInterface->UpdateSpectatorPanel();
	}
}

bool CHudSpectator::IsActivePlayer(cl_entity_t *ent)
{
	return (ent && ent->player && ent->curstate.solid != SOLID_NOT && ent != gEngfuncs.GetLocalPlayer() && g_PlayerInfoList[ent->index].name != NULL);
}

bool CHudSpectator::ParseOverviewFile(void)
{
	char filename[255];
	char levelname[255];
	char token[1024];
	float height;

	char *pfile = NULL;

	memset(&m_OverviewData, 0, sizeof(m_OverviewData));

	SCREENINFO screenInfo;
	screenInfo.iSize = sizeof(screenInfo);
	GetScreenInfo(&screenInfo);

	float screenRatio = (screenInfo.iWidth / 640.0) + 0.5;

	m_OverviewData.insetWindowX = 4;
	m_OverviewData.insetWindowY = 4;
	m_OverviewData.insetWindowHeight = 180 * screenRatio;
	m_OverviewData.insetWindowWidth = 240 * screenRatio;
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom	= 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0] = 0.0f;
	strncpy(m_OverviewData.map, gEngfuncs.pfnGetLevelName(), sizeof(m_OverviewData.map));

	if (strlen(m_OverviewData.map) == 0)
		return false;

	strncpy(levelname, m_OverviewData.map + 5, sizeof(levelname));
	levelname[strlen(levelname) - 4] = 0;

	sprintf(filename, "overviews/%s.txt", levelname);

	pfile = (char *)gEngfuncs.COM_LoadFile(filename, 5, NULL);

	if (!pfile)
	{
		gEngfuncs.Con_DPrintf("Couldn't open file %s. Using default values for overiew mode.\n", filename);
		return false;
	}

	while (true)
	{
		pfile = gEngfuncs.COM_ParseFile(pfile, token);

		if (!pfile)
			break;

		if (!stricmp(token, "global"))
		{
			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			if (stricmp(token, "{"))
			{
				gEngfuncs.Con_Printf("Error parsing overview file %s. (expected { )\n", filename);
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			while (stricmp(token, "}"))
			{
				if (!stricmp(token, "zoom"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.zoom = atof(token);
				}
				else if (!stricmp(token, "origin"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[0] = atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[1] = atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[2] = atof(token);
				}
				else if (!stricmp(token, "rotated"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.rotated = atoi(token);
				}
				else if (!stricmp(token, "inset"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.insetWindowX = atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.insetWindowY = atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.insetWindowWidth = atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.insetWindowHeight = atof(token);
				}
				else
				{
					gEngfuncs.Con_Printf("Error parsing overview file %s. (%s unkown)\n", filename, token);
					return false;
				}

				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
		}
		else if (!stricmp(token, "layer"))
		{
			if (m_OverviewData.layers == OVERVIEW_MAX_LAYERS)
			{
				gEngfuncs.Con_Printf("Error parsing overview file %s. ( too many layers )\n", filename);
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			if (stricmp(token, "{"))
			{
				gEngfuncs.Con_Printf("Error parsing overview file %s. (expected { )\n", filename);
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			while (stricmp(token, "}"))
			{
				if (!stricmp(token, "image"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					strcpy(m_OverviewData.layersImages[m_OverviewData.layers], token);
				}
				else if (!stricmp(token, "height"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					height = atof(token);
					m_OverviewData.layersHeights[m_OverviewData.layers] = height;
				}
				else
				{
					gEngfuncs.Con_Printf("Error parsing overview file %s. (%s unkown)\n", filename, token);
					return false;
				}

				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}

			m_OverviewData.layers++;
		}
	}

	gEngfuncs.COM_FreeFile(pfile);

	m_mapZoom = m_OverviewData.zoom;
	m_mapOrigin = m_OverviewData.origin;

	return true;
}

void CHudSpectator::LoadMapSprites(void)
{
	if (m_OverviewData.layers > 0)
		m_MapSprite = gEngfuncs.LoadMapSprite(m_OverviewData.layersImages[0]);
	else
		m_MapSprite = NULL;
}

void CHudSpectator::DrawOverviewLayer(void)
{
	float screenaspect, xs, ys, xStep, yStep, x, y, z;
	int ix, iy, i, xTiles, yTiles, frame;

	qboolean hasMapImage = m_MapSprite ? TRUE : FALSE;
	model_t *dummySprite = (struct model_s *)gEngfuncs.GetSpritePointer(m_hsprUnkownMap);

	if (hasMapImage)
	{
		i = m_MapSprite->numframes / (4 * 3);
		i = sqrt((float)i);
		xTiles = i * 4;
		yTiles = i * 3;
	}
	else
	{
		xTiles = 8;
		yTiles = 6;
	}

	screenaspect = 4.0f / 3.0f;

	xs = m_OverviewData.origin[0];
	ys = m_OverviewData.origin[1];
	z = (90.0f - v_angles[0]) / 90.0f;
	z *= m_OverviewData.layersHeights[0];

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, 1.0);

	frame = 0;

	if (m_OverviewData.rotated)
	{
		xStep = (2 * 4096.0f / m_OverviewData.zoom) / xTiles;
		yStep = -(2 * 4096.0f / (m_OverviewData.zoom * screenaspect)) / yTiles;

		y = ys + (4096.0f / (m_OverviewData.zoom * screenaspect));

		for (iy = 0; iy < yTiles; iy++)
		{
			x = xs - (4096.0f / (m_OverviewData.zoom));

			for (ix = 0; ix < xTiles; ix++)
			{
				if (hasMapImage)
					gEngfuncs.pTriAPI->SpriteTexture(m_MapSprite, frame);
				else
					gEngfuncs.pTriAPI->SpriteTexture(dummySprite, 0);

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y, z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y, z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y + yStep, z);

				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x, y + yStep, z);
				gEngfuncs.pTriAPI->End();

				frame++;
				x += xStep;
			}

			y += yStep;
		}
	}
	else
	{
		xStep = -(2 * 4096.0f / m_OverviewData.zoom) / xTiles;
		yStep = -(2 * 4096.0f / (m_OverviewData.zoom * screenaspect)) / yTiles;

		x = xs + (4096.0f / (m_OverviewData.zoom * screenaspect));

		for (ix = 0; ix < yTiles; ix++)
		{
			y = ys + (4096.0f / (m_OverviewData.zoom));

			for (iy = 0; iy < xTiles; iy++)
			{
				if (hasMapImage)
					gEngfuncs.pTriAPI->SpriteTexture(m_MapSprite, frame);
				else
					gEngfuncs.pTriAPI->SpriteTexture(dummySprite, 0);

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y, z);

				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y, z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y + yStep, z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y + yStep, z);
				gEngfuncs.pTriAPI->End();

				frame++;
				y += yStep;
			}

			x += xStep;
		}
	}
}

void CHudSpectator::DrawOverviewEntities(void)
{
	int i, ir, ig, ib;
	struct model_s *hSpriteModel;
	vec3_t origin, angles, point, forward, right, left, up, world, screen, offset;
	float x, y, z, r, g, b, sizeScale = 4.0f;
	cl_entity_t *ent;
	float rmatrix[3][4];

	float zScale = (90.0f - v_angles[0]) / 90.0f;

	z = m_OverviewData.layersHeights[0] * zScale;

	UnpackRGB(ir, ig, ib, RGB_YELLOWISH);
	r = (float)ir / 255.0f;
	g = (float)ig / 255.0f;
	b = (float)ib / 255.0f;

	gEngfuncs.pTriAPI->CullFace(TRI_NONE);

	for (i = 0; i < MAX_PLAYERS; i++)
		m_vPlayerPos[i][2] = -1;

	for (i = 0; i < MAX_OVERVIEW_ENTITIES; i++)
	{
		if (!m_OverviewEntities[i].hSprite)
			continue;

		if (m_OverviewEntities[i].entity == gEngfuncs.GetLocalPlayer() && m_OverviewEntities[i].hSprite == m_hsprPlayerDead)
			continue;

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer(m_OverviewEntities[i].hSprite);
		ent = m_OverviewEntities[i].entity;

		gEngfuncs.pTriAPI->SpriteTexture(hSpriteModel, 0);
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);

		AngleVectors(ent->angles, right, up, NULL);
		VectorCopy(ent->origin, origin);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, 1.0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		VectorMA(origin, 16.0f * sizeScale, up, point);
		VectorMA(point, 16.0f * sizeScale, right, point);
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);

		VectorMA(origin, 16.0f * sizeScale, up, point);
		VectorMA(point, -16.0f * sizeScale, right, point);
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		VectorMA(origin, -16.0f * sizeScale, up, point);
		VectorMA(point, -16.0f * sizeScale, right, point);
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		VectorMA(origin, -16.0f * sizeScale, up, point);
		VectorMA(point, 16.0f * sizeScale, right, point);
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->End();

		if (!ent->player)
			continue;

		origin[2] *= zScale;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer(m_hsprBeam);
		gEngfuncs.pTriAPI->SpriteTexture(hSpriteModel, 0);

		gEngfuncs.pTriAPI->Color4f(r, g, b, 0.3);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] + 4, origin[1] + 4, origin[2] - zScale);
		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] - 4, origin[1] - 4, origin[2] - zScale);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] - 4, origin[1] - 4, z);
		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] + 4, origin[1] + 4, z);
		gEngfuncs.pTriAPI->End();

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] - 4, origin[1] + 4, origin[2] - zScale);
		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] + 4, origin[1] - 4, origin[2] - zScale);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] + 4, origin[1] - 4, z);
		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f(origin[0] - 4, origin[1] + 4, z);
		gEngfuncs.pTriAPI->End();

		if (gEngfuncs.pTriAPI->WorldToScreen(origin, screen))
			continue;

		screen[0] = XPROJECT(screen[0]);
		screen[1] = YPROJECT(screen[1]);
		screen[2] = 0.0f;

		origin[0] += 32.0f;
		origin[1] += 32.0f;

		gEngfuncs.pTriAPI->WorldToScreen(origin, offset);

		offset[0] = XPROJECT(offset[0]);
		offset[1] = YPROJECT(offset[1]);
		offset[2] = 0.0f;

		VectorSubtract(offset, screen, offset);

		int playerNum = ent->index - 1;

		m_vPlayerPos[playerNum][0] = screen[0];
		m_vPlayerPos[playerNum][1] = screen[1] + Length(offset);
		m_vPlayerPos[playerNum][2] = 1;
	}

	if (!m_pip->value || !m_drawcone->value)
		return;

	if (m_pip->value == INSET_IN_EYE || g_iUser1 == OBS_IN_EYE)
	{
		V_GetInEyePos(g_iUser2, origin, angles);
	}
	else if (m_pip->value == INSET_CHASE_FREE || g_iUser1 == OBS_CHASE_FREE)
	{
		V_GetChasePos(g_iUser2, v_cl_angles, origin, angles);
	}
	else if (g_iUser1 == OBS_ROAMING)
	{
		VectorCopy(v_sim_org, origin);
		VectorCopy(v_cl_angles, angles);
	}
	else
		V_GetChasePos(g_iUser2, NULL, origin, angles);

	x = origin[0];
	y = origin[1];
	z = origin[2];

	angles[0] = 0;

	hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer(m_hsprCamera);
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->SpriteTexture(hSpriteModel, 0);

	gEngfuncs.pTriAPI->Color4f(r, g, b, 1.0);

	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, 512.0f, forward);

	offset[0] = 0.0f;
	offset[1] = 45.0f;
	offset[2] = 0.0f;

	AngleMatrix(offset, rmatrix);
	VectorTransform(forward, rmatrix, right);

	offset[1] = -45.0f;

	AngleMatrix(offset, rmatrix);
	VectorTransform(forward, rmatrix, left);

	gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(x + right[0], y + right[1], (z + right[2]) * zScale);

	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(x, y, z * zScale);

	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(x + left[0], y + left[1], (z + left[2]) * zScale);
	gEngfuncs.pTriAPI->End();
}

void CHudSpectator::DrawOverview(void)
{
	if (!g_iUser1)
		return;

	if (m_iDrawCycle == 0 && ((g_iUser1 != OBS_MAP_FREE) && (g_iUser1 != OBS_MAP_CHASE)))
		return;

	if (m_iDrawCycle == 1 && m_pip->value < INSET_MAP_FREE)
		return;

	if (gViewPortInterface->GetClientDllInterface()->InIntermission())
	{
		m_pip->value = 0;
		return;
	}

	DrawOverviewLayer();
	DrawOverviewEntities();
	CheckOverviewEntities();
}

void CHudSpectator::CheckOverviewEntities(void)
{
	double time = gEngfuncs.GetClientTime();

	for (int i = 0; i< MAX_OVERVIEW_ENTITIES; i++)
	{
		if (m_OverviewEntities[i].killTime < time)
			memset(&m_OverviewEntities[i], 0, sizeof(overviewEntity_t));
	}
}

bool CHudSpectator::AddOverviewEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	HSPRITE hSprite = 0;
	double duration = -1.0f;

	if (!ent)
		return false;

	if (type == ET_PLAYER)
	{
		if (ent->curstate.solid != SOLID_NOT)
		{
			switch (g_PlayerExtraInfo[ent->index].teamnumber)
			{
				case TEAM_TERRORIST: hSprite = m_hsprPlayerRed; break;
				case TEAM_CT: hSprite = m_hsprPlayerBlue; break;
				default: hSprite = m_hsprPlayer; break;
			}

			if (g_PlayerExtraInfo[ent->index].vip)
				hSprite = m_hsprPlayerVIP;
			else if (g_PlayerExtraInfo[ent->index].has_c4)
				hSprite = m_hsprPlayerC4;
		}
		else
			return false;
	}
	else if (type == ET_NORMAL)
	{
		if (!strcmp(modelname, "models/w_c4.mdl"))
			hSprite = m_hsprBomb;
		else if (!strcmp(modelname, "models/w_backpack.mdl"))
			hSprite = m_hsprBackpack;
		else if (!strcmp(modelname, "models/hostage") || !strcmp(modelname, "models/scientist"))
			hSprite = m_hsprHostage;
	}
	else
		return false;

	return AddOverviewEntityToList(hSprite, ent, gEngfuncs.GetClientTime() + duration);
}

void CHudSpectator::DeathMessage(int victim)
{
	cl_entity_t *pl = gEngfuncs.GetEntityByIndex(victim);

	if (pl && pl->player)
	{
		if (AddOverviewEntityToList(m_hsprPlayerDead, pl, gEngfuncs.GetClientTime() + 4.0f))
		{
			if (pl->index == gEngfuncs.GetLocalPlayer()->index)
			{
				m_iObserverFlags = DRC_FLAG_DRAMATIC | DRC_FLAG_FINAL;
				V_ResetChaseCam();
			}
		}
	}
}

bool CHudSpectator::AddOverviewEntityToList(HSPRITE sprite, cl_entity_t *ent, double killTime)
{
	for (int i = 0; i< MAX_OVERVIEW_ENTITIES; i++)
	{
		if ( m_OverviewEntities[i].entity == NULL)
		{
			m_OverviewEntities[i].entity = ent;
			m_OverviewEntities[i].hSprite = sprite;
			m_OverviewEntities[i].killTime = killTime;
			return true;
		}
	}

	return false;
}

void CHudSpectator::CheckSettings(void)
{
	m_pip->value = (int)m_pip->value;

	if ((g_iUser1 < OBS_MAP_FREE) && (m_pip->value == INSET_CHASE_FREE || m_pip->value == INSET_IN_EYE))
	{
		m_pip->value = INSET_MAP_FREE;
	}

	if ((g_iUser1 >= OBS_MAP_FREE) && (m_pip->value >= INSET_MAP_FREE))
	{
		m_pip->value = INSET_CHASE_FREE;
	} 

	if (gHUD.m_iIntermission)
		m_pip->value = INSET_OFF;

	if (m_chatEnabled != (gHUD.m_SayText.m_HUD_saytext->value != 0))
	{
		m_chatEnabled = (gHUD.m_SayText.m_HUD_saytext->value != 0);

		if (gEngfuncs.IsSpectateOnly())
		{
			char chatcmd[32];
			sprintf(chatcmd, "ignoremsg %i", m_chatEnabled ? 0 : 1);
			gEngfuncs.pfnServerCmd(chatcmd);
		}
	}

	if (((g_iTeamNumber == TEAM_TERRORIST) || (g_iTeamNumber == TEAM_CT)) && (g_iUser1 == OBS_IN_EYE))
	{
		if (m_pip->value != INSET_OFF)
		{
			gHUD.VGUI2HudPrint("#Spec_No_PIP", -1, ScreenHeight * 0.35, 1.0, 0.705, 0.118);
			m_pip->value = INSET_OFF;
		}
	}

	if (gEngfuncs.GetLocalPlayer()->index == g_iUser2)
	{
		gViewPortInterface->SpectatorGUIEnableInsetView(false);
	}
	else
	{
		gViewPortInterface->SpectatorGUIEnableInsetView(m_pip->value != INSET_OFF);
	}
}

int CHudSpectator::ToggleInset(bool allowOff)
{
	int newInsetMode = (int)m_pip->value + 1;

	if (g_iUser1 < OBS_MAP_FREE)
	{
		if (newInsetMode > INSET_MAP_CHASE)
		{
			if (allowOff)
				newInsetMode = INSET_OFF;
			else
				newInsetMode = INSET_MAP_FREE;
		}

		if (newInsetMode == INSET_CHASE_FREE)
			newInsetMode = INSET_MAP_FREE;
	}
	else
	{
		if (newInsetMode > INSET_IN_EYE)
		{
			if (allowOff)
				newInsetMode = INSET_OFF;
			else
				newInsetMode = INSET_CHASE_FREE;
		}
	}

	return newInsetMode;
}

void CHudSpectator::Reset(void)
{
	if (strcmp(m_OverviewData.map, gEngfuncs.pfnGetLevelName()))
	{
		ParseOverviewFile();
		LoadMapSprites();
	}

	memset(&m_OverviewEntities, 0, sizeof(m_OverviewEntities));

	m_FOV = 90.0f;

	m_IsInterpolating = false;

	m_ChaseEntity = 0;

	SetSpectatorStartPosition();
	SetModes(m_mode->value, m_pip->value);
}

void CHudSpectator::InitHUDData(void)
{
	m_lastPrimaryObject = m_lastSecondaryObject = 0;
	m_flNextObserverInput = 0.0f;
	m_lastHudMessage = 0;
	m_iSpectatorNumber = 0;
	iJumpSpectator	= 0;
	g_iUser1 = g_iUser2 = 0;

	memset(&m_OverviewData, 0, sizeof(m_OverviewData));
	memset(&m_OverviewEntities, 0, sizeof(m_OverviewEntities));

	Reset();

	g_iUser2 = 0;

	gHUD.m_iFOV = CVAR_GET_FLOAT("default_fov");
}