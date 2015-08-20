#include "hud.h"
#include "cl_util.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "view.h"
#include <string.h>
#include <ctype.h>
#include "configs.h"
#include "links.h"
#include "CounterStrikeViewport.h"
#include "game_shared/voice_status.h"
#include <VPanel.h>

extern int g_iAlive;
extern int g_weaponselect;
extern cl_enginefunc_t gEngfuncs;

extern "C" float anglemod(float a);

void IN_Init(void);
void IN_Move(float frametime, usercmd_t *cmd);
void IN_Shutdown(void);
void V_Init(void);
void VectorAngles(const float *forward, float *angles);
int CL_ButtonBits(int);

extern cvar_t *in_joystick;

int in_impulse = 0;
int in_cancel = 0;

cvar_t *m_pitch;
cvar_t *m_yaw;
cvar_t *m_forward;
cvar_t *m_side;

cvar_t *lookstrafe;
cvar_t *lookspring;
cvar_t *cl_pitchup;
cvar_t *cl_pitchdown;
cvar_t *cl_upspeed;
cvar_t *cl_forwardspeed;
cvar_t *cl_backspeed;
cvar_t *cl_sidespeed;
cvar_t *cl_movespeedkey;
cvar_t *cl_yawspeed;
cvar_t *cl_pitchspeed;
cvar_t *cl_anglespeedkey;
cvar_t *cl_vsmoothing;

kbutton_t in_mlook;
kbutton_t in_klook;
kbutton_t in_jlook;
kbutton_t in_left;
kbutton_t in_right;
kbutton_t in_forward;
kbutton_t in_back;
kbutton_t in_lookup;
kbutton_t in_lookdown;
kbutton_t in_moveleft;
kbutton_t in_moveright;
kbutton_t in_strafe;
kbutton_t in_speed;
kbutton_t in_use;
kbutton_t in_jump;
kbutton_t in_attack;
kbutton_t in_attack2;
kbutton_t in_up;
kbutton_t in_down;
kbutton_t in_duck;
kbutton_t in_reload;
kbutton_t in_alt1;
kbutton_t in_score;
kbutton_t in_break;
kbutton_t in_graph;

typedef struct kblist_s
{
	struct kblist_s *next;
	kbutton_t *pkey;
	char name[32];
}
kblist_t;

kblist_t *g_kbkeys = NULL;

int KB_ConvertString(char *in, char **ppout)
{
	char sz[4096];
	char binding[64];
	char *p;
	char *pOut;
	char *pEnd;
	const char *pBinding;

	if (!ppout)
		return 0;

	*ppout = NULL;
	p = in;
	pOut = sz;

	while (*p)
	{
		if (*p == '+')
		{
			pEnd = binding;

			while (*p && (isalnum(*p) || (pEnd == binding)) && ((pEnd - binding) < 63))
				*pEnd++ = *p++;

			*pEnd = '\0';
			pBinding = NULL;

			if (strlen(binding + 1) > 0)
				pBinding = gEngfuncs.Key_LookupBinding(binding + 1);

			if (pBinding)
			{
				*pOut++ = '[';
				pEnd = (char *)pBinding;
			}
			else
				pEnd = binding;

			while (*pEnd)
				*pOut++ = *pEnd++;

			if (pBinding)
				*pOut++ = ']';
		}
		else
			*pOut++ = *p++;
	}

	*pOut = '\0';
	pOut = (char *)malloc(strlen(sz) + 1);
	strcpy(pOut, sz);
	*ppout = pOut;

	return 1;
}

struct kbutton_s *KB_Find(const char *name)
{
	kblist_t *p;
	p = g_kbkeys;

	while (p)
	{
		if (!stricmp(name, p->name))
			return p->pkey;

		p = p->next;
	}

	return NULL;
}

void KB_Add(const char *name, kbutton_t *pkb)
{
	kblist_t *p;
	kbutton_t *kb;

	kb = KB_Find(name);

	if (kb)
		return;

	p = (kblist_t *)malloc(sizeof(kblist_t));

	strcpy(p->name, name);
	p->pkey = pkb;

	p->next = g_kbkeys;
	g_kbkeys = p;
}

void KB_Init(void)
{
	g_kbkeys = NULL;

	KB_Add("in_graph", &in_graph);
	KB_Add("in_mlook", &in_mlook);
	KB_Add("in_jlook", &in_jlook);
}

void KB_Shutdown(void)
{
	kblist_t *p, *n;
	p = g_kbkeys;

	while (p)
	{
		n = p->next;
		free(p);
		p = n;
	}

	g_kbkeys = NULL;
}

void KeyDown(kbutton_t *b)
{
	int k;
	char *c;

	c = gEngfuncs.Cmd_Argv(1);

	if (c[0])
		k = atoi(c);
	else
		k = -1;

	if (k == b->down[0] || k == b->down[1])
		return;

	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
	{
		gEngfuncs.Con_DPrintf("Three keys down for a button '%c' '%c' '%c'!\n", b->down[0], b->down[1], c);
		return;
	}

	if (b->state & 1)
		return;

	b->state |= 1 + 2;
}

void KeyUp(kbutton_t *b)
{
	int k;
	char *c;

	c = gEngfuncs.Cmd_Argv(1);

	if (c[0])
		k = atoi(c);
	else
	{
		b->down[0] = b->down[1] = 0;
		b->state = 4;
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;

	if (b->down[0] || b->down[1])
		return;

	if (!(b->state & 1))
		return;

	b->state &= ~1;
	b->state |= 4;
}

int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding)
{
	if(gCkfClientFuncs.HUD_Key_Event && !gCkfClientFuncs.HUD_Key_Event(down, keynum, pszCurrentBinding))
		return 0;

	if (gViewPortInterface && !gViewPortInterface->KeyInput(down, keynum, pszCurrentBinding))
		return 0;

	return 1;
}

void IN_BreakDown(void) { KeyDown(&in_break); }
void IN_BreakUp(void) { KeyUp(&in_break); }
void IN_KLookDown(void) { KeyDown(&in_klook); }
void IN_KLookUp(void) { KeyUp(&in_klook); }
void IN_JLookDown(void) { KeyDown(&in_jlook); }
void IN_JLookUp(void) { KeyUp(&in_jlook); }
void IN_MLookDown(void) { KeyDown(&in_mlook); }
void IN_UpDown(void) { KeyDown(&in_up); }
void IN_UpUp(void) { KeyUp(&in_up); }
void IN_DownDown(void) { KeyDown(&in_down); }
void IN_DownUp(void) { KeyUp(&in_down); }
void IN_LeftDown(void) { KeyDown(&in_left); }
void IN_LeftUp(void) { KeyUp(&in_left); }
void IN_RightDown(void) { KeyDown(&in_right); }
void IN_RightUp(void) { KeyUp(&in_right); }

void IN_ForwardDown(void)
{
	KeyDown(&in_forward);
	gHUD.m_Spectator.HandleButtonsDown(IN_FORWARD);
}

void IN_ForwardUp(void)
{
	KeyUp(&in_forward);
	gHUD.m_Spectator.HandleButtonsUp(IN_FORWARD);
}

void IN_BackDown(void)
{
	KeyDown(&in_back);
	gHUD.m_Spectator.HandleButtonsDown(IN_BACK);
}

void IN_BackUp(void)
{
	KeyUp(&in_back);
	gHUD.m_Spectator.HandleButtonsUp(IN_BACK);
}
void IN_LookupDown(void) { KeyDown(&in_lookup); }
void IN_LookupUp(void) { KeyUp(&in_lookup); }
void IN_LookdownDown(void) { KeyDown(&in_lookdown); }
void IN_LookdownUp(void) { KeyUp(&in_lookdown); }

void IN_MoveleftDown(void)
{
	KeyDown(&in_moveleft);
	gHUD.m_Spectator.HandleButtonsDown(IN_MOVELEFT);
}

void IN_MoveleftUp(void)
{
	KeyUp(&in_moveleft);
	gHUD.m_Spectator.HandleButtonsUp(IN_MOVELEFT);
}

void IN_MoverightDown(void)
{
	KeyDown(&in_moveright);
	gHUD.m_Spectator.HandleButtonsDown(IN_MOVERIGHT);
}

void IN_MoverightUp(void)
{
	KeyUp(&in_moveright);
	gHUD.m_Spectator.HandleButtonsUp(IN_MOVERIGHT);
}

void IN_SpeedDown(void) { KeyDown(&in_speed); }
void IN_SpeedUp(void) { KeyUp(&in_speed); }
void IN_StrafeDown(void) { KeyDown(&in_strafe); }
void IN_StrafeUp(void) { KeyUp(&in_strafe); }

void IN_Attack2Down(void)
{
	KeyDown(&in_attack2);
	gHUD.m_Spectator.HandleButtonsDown(IN_ATTACK2);
}

void IN_Attack2Up(void) { KeyUp(&in_attack2); }

void IN_UseDown(void)
{
	KeyDown(&in_use);
	gHUD.m_Spectator.HandleButtonsDown(IN_USE);
}

void IN_UseUp(void) { KeyUp(&in_use); }

void IN_JumpDown(void)
{
	KeyDown(&in_jump);
	gHUD.m_Spectator.HandleButtonsDown(IN_JUMP);

}
void IN_JumpUp(void) { KeyUp(&in_jump); }

void IN_DuckDown(void)
{
	KeyDown(&in_duck);
	gHUD.m_Spectator.HandleButtonsDown(IN_DUCK);
}

void IN_DuckUp(void) { KeyUp(&in_duck); }
void IN_ReloadDown(void) { KeyDown(&in_reload); }
void IN_ReloadUp(void) { KeyUp(&in_reload); }
void IN_Alt1Down(void) { KeyDown(&in_alt1); }
void IN_Alt1Up(void) { KeyUp(&in_alt1); }
void IN_GraphDown(void) { KeyDown(&in_graph); }
void IN_GraphUp(void) { KeyUp(&in_graph); }

void IN_AttackDown(void)
{
	KeyDown(&in_attack);
	gHUD.m_Spectator.HandleButtonsDown(IN_ATTACK);
}

void IN_AttackUp(void)
{
	KeyUp(&in_attack);
	in_cancel = 0;
}

void IN_Cancel(void)
{
	in_cancel = 1;
}

void IN_Impulse(void)
{
	in_impulse = atoi(gEngfuncs.Cmd_Argv(1));
}

void IN_ScoreDown(void)
{
	KeyDown(&in_score);

	if (gViewPortInterface)
		gViewPortInterface->ShowScoreBoard();
}

void IN_ScoreUp(void)
{
	KeyUp(&in_score);

	if (gViewPortInterface)
	{
		gViewPortInterface->HideScoreBoard();
	}
}

void IN_MLookUp(void)
{
	KeyUp(&in_mlook);

	if (!(in_mlook.state & 1) && lookspring->value)
	{
		V_StartPitchDrift();
	}
}

float CL_KeyState(kbutton_t *key)
{
	float val = 0.0;
	int impulsedown, impulseup, down;

	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;

	if (impulsedown && !impulseup)
		val = down ? 0.5 : 0.0;

	if (impulseup && !impulsedown)
		val = down ? 0.0 : 0.0;

	if (!impulsedown && !impulseup)
		val = down ? 1.0 : 0.0;

	if (impulsedown && impulseup)
	{
		if (down)
			val = 0.75;
		else
			val = 0.25;
	}

	key->state &= 1;
	return val;
}

void CL_AdjustAngles(float frametime, float *viewangles)
{
	float speed;
	float up, down;

	if (in_speed.state & 1)
	{
		speed = frametime * cl_anglespeedkey->value;
	}
	else
	{
		speed = frametime;
	}

	if (!(in_strafe.state & 1))
	{
		viewangles[YAW] -= speed * cl_yawspeed->value * CL_KeyState(&in_right);
		viewangles[YAW] += speed * cl_yawspeed->value * CL_KeyState(&in_left);
		viewangles[YAW] = anglemod(viewangles[YAW]);
	}

	if (in_klook.state & 1)
	{
		V_StopPitchDrift();
		viewangles[PITCH] -= speed * cl_pitchspeed->value * CL_KeyState(&in_forward);
		viewangles[PITCH] += speed * cl_pitchspeed->value * CL_KeyState(&in_back);
	}

	up = CL_KeyState(&in_lookup);
	down = CL_KeyState(&in_lookdown);

	viewangles[PITCH] -= speed * cl_pitchspeed->value * up;
	viewangles[PITCH] += speed * cl_pitchspeed->value * down;

	if (up || down)
		V_StopPitchDrift();

	if (viewangles[PITCH] > cl_pitchdown->value)
		viewangles[PITCH] = cl_pitchdown->value;
	if (viewangles[PITCH] < -cl_pitchup->value)
		viewangles[PITCH] = -cl_pitchup->value;

	if (viewangles[ROLL] > 50)
		viewangles[ROLL] = 50;
	if (viewangles[ROLL] < -50)
		viewangles[ROLL] = -50;
}

void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	float spd;
	vec3_t viewangles;
	static vec3_t oldangles;

	if (active)
	{
		gEngfuncs.GetViewAngles((float *)viewangles);

		CL_AdjustAngles(frametime, viewangles);

		memset(cmd, 0, sizeof(*cmd));

		gEngfuncs.SetViewAngles((float *)viewangles);

		if (in_strafe.state & 1)
		{
			cmd->sidemove += cl_sidespeed->value * CL_KeyState(&in_right);
			cmd->sidemove -= cl_sidespeed->value * CL_KeyState(&in_left);
		}

		cmd->sidemove += cl_sidespeed->value * CL_KeyState(&in_moveright);
		cmd->sidemove -= cl_sidespeed->value * CL_KeyState(&in_moveleft);

		cmd->upmove += cl_upspeed->value * CL_KeyState(&in_up);
		cmd->upmove -= cl_upspeed->value * CL_KeyState(&in_down);

		if (!(in_klook.state & 1))
		{
			cmd->forwardmove += cl_forwardspeed->value * CL_KeyState(&in_forward);
			cmd->forwardmove -= cl_backspeed->value * CL_KeyState(&in_back);
		}

		IN_Move(frametime, cmd);

		spd = gEngfuncs.GetClientMaxspeed();

		if (spd != 0.0)
		{
			float fmov = sqrt((cmd->forwardmove * cmd->forwardmove) + (cmd->sidemove * cmd->sidemove) + (cmd->upmove * cmd->upmove));

			if (fmov > spd)
			{
				float fratio = spd / fmov;
				cmd->forwardmove *= fratio;
				cmd->sidemove *= fratio;
				cmd->upmove *= fratio;
			}
		}

		if (in_speed.state & 1)
		{
			cmd->forwardmove *= cl_movespeedkey->value;
			cmd->sidemove *= cl_movespeedkey->value;
			cmd->upmove *= cl_movespeedkey->value;
		}
	}

	cmd->impulse = in_impulse;
	in_impulse = 0;

	cmd->weaponselect = g_weaponselect;
	g_weaponselect = 0;

	cmd->buttons = CL_ButtonBits(1);

	if (GetClientVoice()->IsInSquelchMode())
		cmd->buttons &= ~IN_ATTACK;

	if (in_joystick->value)
	{
		if (cmd->forwardmove > 0)
		{
			cmd->buttons |= IN_FORWARD;
		}
		else if (cmd->forwardmove < 0)
		{
			cmd->buttons |= IN_BACK;
		}
	}

	gEngfuncs.GetViewAngles((float *)viewangles);

	if ((*gCKFVars.g_iHealth) > 0)
	{
		VectorCopy(viewangles, cmd->viewangles);
		VectorCopy(viewangles, oldangles);
	}
	else
	{
		VectorCopy(oldangles, cmd->viewangles);
	}

	if(gCkfClientFuncs.CL_CreateMove)
		gCkfClientFuncs.CL_CreateMove(frametime, cmd, active);
}

int CL_IsDead(void)
{
	return ((*gCKFVars.g_iHealth) <= 0) ? 1 : 0;
}

int CL_ButtonBits(int bResetState)
{
	int bits = 0;

	if (in_attack.state & 3)
	{
		bits |= IN_ATTACK;
	}

	if (in_duck.state & 3)
	{
		bits |= IN_DUCK;
	}
 
	if (in_jump.state & 3)
	{
		bits |= IN_JUMP;
	}

	if (in_forward.state & 3)
	{
		bits |= IN_FORWARD;
	}

	if (in_back.state & 3)
	{
		bits |= IN_BACK;
	}

	if (in_use.state & 3)
	{
		bits |= IN_USE;
	}

	if (in_cancel)
	{
		bits |= IN_CANCEL;
	}

	if (in_left.state & 3)
	{
		bits |= IN_LEFT;
	}

	if (in_right.state & 3)
	{
		bits |= IN_RIGHT;
	}

	if (in_moveleft.state & 3)
	{
		bits |= IN_MOVELEFT;
	}

	if (in_moveright.state & 3)
	{
		bits |= IN_MOVERIGHT;
	}

	if (in_attack2.state & 3)
	{
		bits |= IN_ATTACK2;
	}

	if (in_reload.state & 3)
	{
		bits |= IN_RELOAD;
	}

	if (in_alt1.state & 3)
	{
		bits |= IN_ALT1;
	}

	if (in_score.state & 3)
	{
		bits |= IN_SCORE;
	}

	if (CL_IsDead() && gHUD.m_iIntermission)
	{
		bits |= IN_SCORE;
	}

	if (bResetState)
	{
		in_attack.state &= ~2;
		in_duck.state &= ~2;
		in_jump.state &= ~2;
		in_forward.state &= ~2;
		in_back.state &= ~2;
		in_use.state &= ~2;
		in_left.state &= ~2;
		in_right.state &= ~2;
		in_moveleft.state &= ~2;
		in_moveright.state &= ~2;
		in_attack2.state &= ~2;
		in_reload.state &= ~2;
		in_alt1.state &= ~2;
		in_score.state &= ~2;
	}

	return bits;
}

void CL_ResetButtonBits(int bits)
{
	int bitsNew = CL_ButtonBits(0) ^ bits;

	if (bitsNew & IN_ATTACK)
	{
		if (bits & IN_ATTACK)
		{
			KeyDown(&in_attack);
		}
		else
		{
			in_attack.state &= ~7;
		}
	}
}

void InitInput(void)
{
	Cmd_HookCmd("+moveup", IN_UpDown);
	Cmd_HookCmd("-moveup", IN_UpUp);
	Cmd_HookCmd("+movedown", IN_DownDown);
	Cmd_HookCmd("-movedown", IN_DownUp);
	Cmd_HookCmd("+left", IN_LeftDown);
	Cmd_HookCmd("-left", IN_LeftUp);
	Cmd_HookCmd("+right", IN_RightDown);
	Cmd_HookCmd("-right", IN_RightUp);
	Cmd_HookCmd("+forward", IN_ForwardDown);
	Cmd_HookCmd("-forward", IN_ForwardUp);
	Cmd_HookCmd("+back", IN_BackDown);
	Cmd_HookCmd("-back", IN_BackUp);
	Cmd_HookCmd("+lookup", IN_LookupDown);
	Cmd_HookCmd("-lookup", IN_LookupUp);
	Cmd_HookCmd("+lookdown", IN_LookdownDown);
	Cmd_HookCmd("-lookdown", IN_LookdownUp);
	Cmd_HookCmd("+strafe", IN_StrafeDown);
	Cmd_HookCmd("-strafe", IN_StrafeUp);
	Cmd_HookCmd("+moveleft", IN_MoveleftDown);
	Cmd_HookCmd("-moveleft", IN_MoveleftUp);
	Cmd_HookCmd("+moveright", IN_MoverightDown);
	Cmd_HookCmd("-moveright", IN_MoverightUp);
	Cmd_HookCmd("+speed", IN_SpeedDown);
	Cmd_HookCmd("-speed", IN_SpeedUp);
	Cmd_HookCmd("+attack", IN_AttackDown);
	Cmd_HookCmd("-attack", IN_AttackUp);
	Cmd_HookCmd("+attack2", IN_Attack2Down);
	Cmd_HookCmd("-attack2", IN_Attack2Up);
	Cmd_HookCmd("+use", IN_UseDown);
	Cmd_HookCmd("-use", IN_UseUp);
	Cmd_HookCmd("+jump", IN_JumpDown);
	Cmd_HookCmd("-jump", IN_JumpUp);
	Cmd_HookCmd("impulse", IN_Impulse);
	Cmd_HookCmd("+klook", IN_KLookDown);
	Cmd_HookCmd("-klook", IN_KLookUp);
	Cmd_HookCmd("+mlook", IN_MLookDown);
	Cmd_HookCmd("-mlook", IN_MLookUp);
	Cmd_HookCmd("+jlook", IN_JLookDown);
	Cmd_HookCmd("-jlook", IN_JLookUp);
	Cmd_HookCmd("+duck", IN_DuckDown);
	Cmd_HookCmd("-duck", IN_DuckUp);
	Cmd_HookCmd("+reload", IN_ReloadDown);
	Cmd_HookCmd("-reload", IN_ReloadUp);
	Cmd_HookCmd("+alt1", IN_Alt1Down);
	Cmd_HookCmd("-alt1", IN_Alt1Up);
	Cmd_HookCmd("+score", IN_ScoreDown);
	Cmd_HookCmd("-score", IN_ScoreUp);
	Cmd_HookCmd("+showscores", IN_ScoreDown);
	Cmd_HookCmd("-showscores", IN_ScoreUp);
	Cmd_HookCmd("+graph", IN_GraphDown);
	Cmd_HookCmd("-graph", IN_GraphUp);
	Cmd_HookCmd("+break", IN_BreakDown);
	Cmd_HookCmd("-break", IN_BreakUp);

	lookstrafe = gEngfuncs.pfnGetCvarPointer("lookstrafe");
	lookspring = gEngfuncs.pfnGetCvarPointer("lookspring");

	cl_anglespeedkey = gEngfuncs.pfnGetCvarPointer("cl_anglespeedkey");
	cl_yawspeed = gEngfuncs.pfnGetCvarPointer("cl_yawspeed");
	cl_pitchspeed = gEngfuncs.pfnGetCvarPointer("cl_pitchspeed");
	cl_upspeed = gEngfuncs.pfnGetCvarPointer("cl_upspeed");
	cl_forwardspeed = gEngfuncs.pfnGetCvarPointer("cl_forwardspeed");
	cl_backspeed = gEngfuncs.pfnGetCvarPointer("cl_backspeed");
	cl_sidespeed = gEngfuncs.pfnGetCvarPointer("cl_sidespeed");
	cl_movespeedkey = gEngfuncs.pfnGetCvarPointer("cl_movespeedkey");
	cl_pitchup = gEngfuncs.pfnGetCvarPointer("cl_pitchup");
	cl_pitchdown = gEngfuncs.pfnGetCvarPointer("cl_pitchdown");

	cl_vsmoothing = gEngfuncs.pfnGetCvarPointer("cl_vsmoothing");

	m_pitch = gEngfuncs.pfnGetCvarPointer("m_pitch");
	m_yaw = gEngfuncs.pfnGetCvarPointer("m_yaw");
	m_forward = gEngfuncs.pfnGetCvarPointer("m_forward");
	m_side = gEngfuncs.pfnGetCvarPointer("m_side");

	CAM_Init();
	IN_Init();
	KB_Init();
	V_Init();
}

void ShutdownInput(void)
{
	IN_Shutdown();
	KB_Shutdown();
}