#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "keydefs.h"
#include "view.h"
#include "windows.h"
#include "configs.h"
#include "video.h"
#include "plugins.h"

#pragma comment(lib, "winmm.lib")

#define MOUSE_BUTTON_COUNT 5

int g_iVisibleMouse = 0;

extern cl_enginefunc_t gEngfuncs;

extern int iMouseInUse;

extern kbutton_t in_strafe;
extern kbutton_t in_mlook;
extern kbutton_t in_speed;
extern kbutton_t in_jlook;

extern cvar_t *m_pitch;
extern cvar_t *m_yaw;
extern cvar_t *m_forward;
extern cvar_t *m_side;

extern cvar_t *lookstrafe;
extern cvar_t *lookspring;
extern cvar_t *cl_pitchdown;
extern cvar_t *cl_pitchup;
extern cvar_t *cl_yawspeed;
extern cvar_t *cl_sidespeed;
extern cvar_t *cl_forwardspeed;
extern cvar_t *cl_pitchspeed;
extern cvar_t *cl_movespeedkey;

static bool m_bRawInput = false;
static bool m_bMouseThread = false;

cvar_t *m_rawinput;
cvar_t *m_filter;

cvar_t *sensitivity;

static cvar_t *m_customaccel;
static cvar_t *m_customaccel_scale;
static cvar_t *m_customaccel_max;
static cvar_t *m_customaccel_exponent;
static cvar_t *m_mousethread_sleep;

int mouse_buttons;
int mouse_oldbuttonstate;
POINT current_pos;
int old_mouse_x, old_mouse_y, mx_accum, my_accum;
float mouse_x, mouse_y;

static int restore_spi;
static int originalmouseparms[3], newmouseparms[3] = { 0, 0, 1 };
static int mouseactive;
int mouseinitialized;
static int mouseparmsvalid;
static int mouseshowtoggle = 1;
static qboolean dinput_acquired;

#define JOY_ABSOLUTE_AXIS 0x00000000
#define JOY_RELATIVE_AXIS 0x00000010
#define JOY_MAX_AXES 6
#define JOY_AXIS_X 0
#define JOY_AXIS_Y 1
#define JOY_AXIS_Z 2
#define JOY_AXIS_R 3
#define JOY_AXIS_U 4
#define JOY_AXIS_V 5

enum _ControlList
{
	AxisNada = 0,
	AxisForward,
	AxisLook,
	AxisSide,
	AxisTurn
};

DWORD dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX,
	JOY_RETURNY,
	JOY_RETURNZ,
	JOY_RETURNR,
	JOY_RETURNU,
	JOY_RETURNV
};

DWORD dwAxisMap[JOY_MAX_AXES];
DWORD dwControlMap[JOY_MAX_AXES];
PDWORD pdwRawValue[JOY_MAX_AXES];

cvar_t *in_joystick;
cvar_t *joy_name;
cvar_t *joy_advanced;
cvar_t *joy_advaxisx;
cvar_t *joy_advaxisy;
cvar_t *joy_advaxisz;
cvar_t *joy_advaxisr;
cvar_t *joy_advaxisu;
cvar_t *joy_advaxisv;
cvar_t *joy_forwardthreshold;
cvar_t *joy_sidethreshold;
cvar_t *joy_pitchthreshold;
cvar_t *joy_yawthreshold;
cvar_t *joy_forwardsensitivity;
cvar_t *joy_sidesensitivity;
cvar_t *joy_pitchsensitivity;
cvar_t *joy_yawsensitivity;
cvar_t *joy_wwhack1;
cvar_t *joy_wwhack2;

int joy_avail, joy_advancedinit, joy_haspov;
DWORD joy_oldbuttonstate, joy_oldpovstate;

int joy_id;
DWORD joy_flags;
DWORD joy_numbuttons;

static JOYINFOEX ji;

DWORD s_hMouseThreadId = 0;
HANDLE s_hMouseThread = 0;
HANDLE s_hMouseQuitEvent = 0;
HANDLE s_hMouseDoneQuitEvent = 0;

void Force_CenterView_f(void)
{
	vec3_t viewangles;

	if (!iMouseInUse)
	{
		gEngfuncs.GetViewAngles((float *)viewangles);
		viewangles[PITCH] = 0;
		gEngfuncs.SetViewAngles((float *)viewangles);
	}
}

long s_mouseDeltaX = 0;
long s_mouseDeltaY = 0;
POINT old_mouse_pos;

long ThreadInterlockedExchange(long *pDest, long value)
{
	return InterlockedExchange(pDest, value);
}

DWORD WINAPI MousePos_ThreadFunction(LPVOID p)
{
	s_hMouseDoneQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	while (1)
	{
		if (WaitForSingleObject(s_hMouseQuitEvent, (int)m_mousethread_sleep->value) == WAIT_OBJECT_0)
			return 0;

		if (mouseactive)
		{
			POINT mouse_pos;
			GetCursorPos(&mouse_pos);

			volatile int mx = mouse_pos.x - old_mouse_pos.x + s_mouseDeltaX;
			volatile int my = mouse_pos.y - old_mouse_pos.y + s_mouseDeltaY;
 
			ThreadInterlockedExchange(&old_mouse_pos.x, mouse_pos.x);
			ThreadInterlockedExchange(&old_mouse_pos.y, mouse_pos.y);

			ThreadInterlockedExchange(&s_mouseDeltaX, mx);
			ThreadInterlockedExchange(&s_mouseDeltaY, my);
		}
	}

	SetEvent(s_hMouseDoneQuitEvent);
	return 0;
}

void IN_UnlockCursor(void)
{
	VID_UnlockCursor();
}

void IN_LockCursor(void)
{
	VID_LockCursor();
}

void IN_HideCursor(void)
{
	VID_HideCursor();
}

void IN_ShowCursor(void)
{
	VID_ShowCursor();
}

void IN_ResetMouse(void);

void IN_ActivateMouse(void)
{
	if (mouseinitialized)
	{
		if (mouseparmsvalid)
			restore_spi = SystemParametersInfo(SPI_SETMOUSE, 0, newmouseparms, 0);

		mouseactive = 1;

		if (!vgui::surface()->IsCursorVisible() && strlen(gEngfuncs.pfnGetLevelName()) > 0)
			IN_LockCursor();

		IN_ResetMouse();
	}
}

void IN_DeactivateMouse(void)
{
	if (mouseinitialized)
	{
		if (restore_spi)
			SystemParametersInfo(SPI_SETMOUSE, 0, originalmouseparms, 0);

		mouseactive = 0;

		IN_UnlockCursor();
	}
}

void IN_StartupMouse(void)
{
	if (gEngfuncs.CheckParm("-nomouse", NULL))
		return; 

	mouseinitialized = 1;
	mouseparmsvalid = SystemParametersInfo(SPI_GETMOUSE, 0, originalmouseparms, 0);

	if (mouseparmsvalid)
	{
		if (gEngfuncs.CheckParm("-noforcemspd", NULL ))
			newmouseparms[2] = originalmouseparms[2];

		if (gEngfuncs.CheckParm("-noforcemaccel", NULL))
		{
			newmouseparms[0] = originalmouseparms[0];
			newmouseparms[1] = originalmouseparms[1];
		}

		if (gEngfuncs.CheckParm("-noforcemparms", NULL))
		{
			newmouseparms[0] = originalmouseparms[0];
			newmouseparms[1] = originalmouseparms[1];
			newmouseparms[2] = originalmouseparms[2];
		}
	}

	mouse_buttons = MOUSE_BUTTON_COUNT;
}

void IN_Shutdown(void)
{
	IN_DeactivateMouse();

	if (s_hMouseQuitEvent)
	{
		SetEvent(s_hMouseQuitEvent);
		WaitForSingleObject(s_hMouseDoneQuitEvent, 100);
	}

	if (s_hMouseThread)
	{
		TerminateThread(s_hMouseThread, 0);
		CloseHandle(s_hMouseThread);
		s_hMouseThread = NULL;
	}

	if (s_hMouseQuitEvent)
	{
		CloseHandle(s_hMouseQuitEvent);
		s_hMouseQuitEvent = NULL;
	}
}

void IN_GetMousePos(int *mx, int *my)
{
	gEngfuncs.GetMousePosition(mx, my);
}

void IN_ResetMouse(void)
{
	bool bCursorVisible = vgui::surface()->IsCursorVisible();

	if (!m_bRawInput && mouseactive && !bCursorVisible)
	{
		SetCursorPos(gEngfuncs.GetWindowCenterX(), gEngfuncs.GetWindowCenterY());

		if (m_bMouseThread)
		{
			ThreadInterlockedExchange(&old_mouse_pos.x, gEngfuncs.GetWindowCenterX());
			ThreadInterlockedExchange(&old_mouse_pos.y, gEngfuncs.GetWindowCenterY());
		}
		else
		{
			old_mouse_pos.x = gEngfuncs.GetWindowCenterX();
			old_mouse_pos.y = gEngfuncs.GetWindowCenterY();
		}
	}

	if (VID_IsSupportsItem(VIDEOITEM_RELATIVEMOUSE))
	{
		bool bRawInput = m_rawinput->value;

		if (m_bRawInput != bRawInput)
		{
			VID_SetRelativeMouseMode(bRawInput);

			if (!bCursorVisible)
				IN_LockCursor();

			m_bRawInput = bRawInput;
		}
	}
	else
	{
		m_bRawInput = false;
	}
}

void IN_MouseEvent(int mstate)
{
	int i;

	if (iMouseInUse || g_iVisibleMouse)
		return;

	if (vgui::surface()->IsCursorVisible())
		return;

	if (mouseactive)
	{
		for (i = 0; i < mouse_buttons; i++)
		{
			if ((mstate & (1 << i)) && !(mouse_oldbuttonstate & (1 << i)))
				gEngfuncs.Key_Event(K_MOUSE1 + i, 1);

			if (!(mstate & (1 << i)) && (mouse_oldbuttonstate & (1 << i)))
				gEngfuncs.Key_Event(K_MOUSE1 + i, 0);
		}

		mouse_oldbuttonstate = mstate;
	}
}

void IN_ScaleMouse(float *x, float *y)
{
	float mx = *x;
	float my = *y;

	float mouse_senstivity = (gHUD.GetSensitivity() != 0) ? gHUD.GetSensitivity() : sensitivity->value;

	if (m_customaccel->value != 0)
	{
		float raw_mouse_movement_distance = sqrt(mx * mx + my * my);
		float acceleration_scale = m_customaccel_scale->value;
		float accelerated_sensitivity_max = m_customaccel_max->value;
		float accelerated_sensitivity_exponent = m_customaccel_exponent->value;
		float accelerated_sensitivity = ((float)pow(raw_mouse_movement_distance, accelerated_sensitivity_exponent) * acceleration_scale + mouse_senstivity);

		if (accelerated_sensitivity_max > 0.0001f && accelerated_sensitivity > accelerated_sensitivity_max)
			accelerated_sensitivity = accelerated_sensitivity_max;

		*x *= accelerated_sensitivity;
		*y *= accelerated_sensitivity;

		if (m_customaccel->value == 2)
		{
			*x *= m_yaw->value; 
			*y *= m_pitch->value; 
		}
	}
	else
	{
		*x *= mouse_senstivity;
		*y *= mouse_senstivity;
	}
}

void IN_ClearStates(void);

void IN_MouseMove(float frametime, usercmd_t *cmd)
{
	int mx, my;
	vec3_t viewangles;

	gEngfuncs.GetViewAngles((float *)viewangles);

	if (in_mlook.state & 1)
		V_StopPitchDrift();

	static bool lastCursorVisible = false;
	bool cursorVisible = vgui::surface()->IsCursorVisible();

	if (lastCursorVisible != cursorVisible)
	{
		lastCursorVisible = cursorVisible;

		if (m_bRawInput)
		{
			int deltaX, deltaY;
			VID_GetRelativeMouseState(&deltaX, &deltaY);

			if (cursorVisible)
				VID_SetRelativeMouseMode(false);
			else
				VID_SetRelativeMouseMode(true);
		}

		if (cursorVisible)
			IN_UnlockCursor();
		else
			IN_LockCursor();

		IN_ResetMouse();
		IN_ClearStates();
	}

	if (!iMouseInUse && !g_iVisibleMouse && !gHUD.m_iIntermission)
	{
		if (!cursorVisible)
		{
			if (m_bRawInput)
			{
				int deltaX, deltaY;
				VID_GetRelativeMouseState(&deltaX, &deltaY);

				current_pos.x = deltaX;
				current_pos.y = deltaY;

				mx = deltaX + mx_accum;
				my = deltaY + my_accum;
			}
			else
			{
				if (m_bMouseThread)
				{
					ThreadInterlockedExchange(&current_pos.x, s_mouseDeltaX);
					ThreadInterlockedExchange(&current_pos.y, s_mouseDeltaY);
					ThreadInterlockedExchange(&s_mouseDeltaX, 0);
					ThreadInterlockedExchange(&s_mouseDeltaY, 0);

					mx = current_pos.x;
					my = current_pos.y;
				}
				else
				{
					GetCursorPos(&current_pos);

					mx = current_pos.x - gEngfuncs.GetWindowCenterX() + mx_accum;
					my = current_pos.y - gEngfuncs.GetWindowCenterY() + my_accum;
				}
			}

			mx_accum = 0;
			my_accum = 0;

			if (m_filter->value)
			{
				mouse_x = (mx + old_mouse_x) * 0.5;
				mouse_y = (my + old_mouse_y) * 0.5;
			}
			else
			{
				mouse_x = mx;
				mouse_y = my;
			}

			old_mouse_x = mx;
			old_mouse_y = my;

			IN_ScaleMouse(&mouse_x, &mouse_y);

			if ((in_strafe.state & 1) || (lookstrafe->value && (in_mlook.state & 1)))
				cmd->sidemove += m_side->value * mouse_x;
			else
				viewangles[YAW] -= m_yaw->value * mouse_x;

			if ((in_mlook.state & 1) && !(in_strafe.state & 1))
			{
				viewangles[PITCH] += m_pitch->value * mouse_y;

				if (viewangles[PITCH] > cl_pitchdown->value)
					viewangles[PITCH] = cl_pitchdown->value;

				if (viewangles[PITCH] < -cl_pitchup->value)
					viewangles[PITCH] = -cl_pitchup->value;
			}
			else
			{
				if ((in_strafe.state & 1) && gEngfuncs.IsNoClipping())
					cmd->upmove -= m_forward->value * mouse_y;
				else
					cmd->forwardmove -= m_forward->value * mouse_y;
			}

			if (mx || my)
				IN_ResetMouse();
		}
	}

	gEngfuncs.SetViewAngles((float *)viewangles);
}

void IN_Accumulate(void)
{
	if (!iMouseInUse && !g_iVisibleMouse)
	{
		if (!vgui::surface()->IsCursorVisible() && mouseactive)
		{
			if (!m_bRawInput)
			{
				if (!m_bMouseThread)
				{
					GetCursorPos(&current_pos);

					mx_accum += current_pos.x - gEngfuncs.GetWindowCenterX();
					my_accum += current_pos.y - gEngfuncs.GetWindowCenterY();
				}
			}
			else
			{
				int deltaX, deltaY;
				VID_GetRelativeMouseState(&deltaX, &deltaY);

				mx_accum += deltaX;
				my_accum += deltaY;
			}

			IN_ResetMouse();
		}
	}
}

void IN_ClearStates(void)
{
	if (!mouseactive)
		return;

	mx_accum = 0;
	my_accum = 0;

	mouse_oldbuttonstate = 0;
}

void IN_StartupJoystick(void)
{
	int numdevs;
	JOYCAPS jc;
	MMRESULT mmr;

	joy_avail = 0; 

	if (gEngfuncs.CheckParm("-nojoy", NULL))
		return; 

	if ((numdevs = joyGetNumDevs()) == 0)
	{
		gEngfuncs.Con_DPrintf("joystick not found -- driver not present\n\n");
		return;
	}

	for (joy_id = 0; joy_id < numdevs; joy_id++)
	{
		memset(&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx(joy_id, &ji)) == JOYERR_NOERROR)
			break;
	}

	if (mmr != JOYERR_NOERROR)
	{
		gEngfuncs.Con_DPrintf("joystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}

	memset(&jc, 0, sizeof(jc));

	if ((mmr = joyGetDevCaps(joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		gEngfuncs.Con_DPrintf("joystick not found -- invalid joystick capabilities (%x)\n\n", mmr);
		return;
	}

	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	joy_oldbuttonstate = joy_oldpovstate = 0;

	gEngfuncs.Con_Printf("joystick found\n\n", mmr);
	joy_avail = 1;
	joy_advancedinit = 0;
}

PDWORD RawValuePointer(int axis)
{
	switch (axis)
	{
		case JOY_AXIS_X: return &ji.dwXpos;
		case JOY_AXIS_Y: return &ji.dwYpos;
		case JOY_AXIS_Z: return &ji.dwZpos;
		case JOY_AXIS_R: return &ji.dwRpos;
		case JOY_AXIS_U: return &ji.dwUpos;
		case JOY_AXIS_V: return &ji.dwVpos;
	}

	return &ji.dwXpos;
}

void Joy_AdvancedUpdate_f(void)
{
	int i;
	DWORD dwTemp;

	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if (joy_advanced->value == 0.0)
	{
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
	}
	else
	{
		if (strcmp(joy_name->string, "joystick") != 0)
			gEngfuncs.Con_Printf("\n%s configured\n\n", joy_name->string);

		dwTemp = (DWORD) joy_advaxisx->value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy->value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz->value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr->value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu->value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv->value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;

	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (dwAxisMap[i] != AxisNada)
			joy_flags |= dwAxisFlags[i];
	}
}

void IN_Commands(void)
{
	int i, key_index;
	DWORD buttonstate, povstate;

	if (!joy_avail)
		return;

	buttonstate = ji.dwButtons;

	for (i = 0; i < (int)joy_numbuttons; i++)
	{
		if ((buttonstate & (1 << i)) && !(joy_oldbuttonstate & (1 << i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			gEngfuncs.Key_Event(key_index + i, 1);
		}

		if (!(buttonstate & (1 << i)) && (joy_oldbuttonstate & (1 << i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			gEngfuncs.Key_Event(key_index + i, 0);
		}
	}

	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		povstate = 0;

		if (ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;

			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;

			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;

			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}

		for (i = 0; i < 4; i++)
		{
			if ((povstate & (1 << i)) && !(joy_oldpovstate & (1 << i)))
				gEngfuncs.Key_Event(K_AUX29 + i, 1);

			if (!(povstate & (1 << i)) && (joy_oldpovstate & (1 << i)))
				gEngfuncs.Key_Event(K_AUX29 + i, 0);
		}

		joy_oldpovstate = povstate;
	}
}

int IN_ReadJoystick(void)
{
	memset(&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx(joy_id, &ji) == JOYERR_NOERROR)
	{
		if (joy_wwhack1->value != 0.0)
			ji.dwUpos += 100;

		return 1;
	}
	else
	{
		return 0;
	}
}

void IN_JoyMove(float frametime, usercmd_t *cmd)
{
	float speed, aspeed;
	float fAxisValue, fTemp;
	int i;
	vec3_t viewangles;

	gEngfuncs.GetViewAngles((float *)viewangles);

	if (joy_advancedinit != 1)
	{
		Joy_AdvancedUpdate_f();
		joy_advancedinit = 1;
	}

	if (!joy_avail || !in_joystick->value)
		return; 

	if (IN_ReadJoystick () != 1)
		return;

	if (in_speed.state & 1)
		speed = cl_movespeedkey->value;
	else
		speed = 1;

	aspeed = speed * frametime;

	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		fAxisValue = (float)*pdwRawValue[i];
		fAxisValue -= 32768.0;

		if (joy_wwhack2->value != 0.0)
		{
			if (dwAxisMap[i] == AxisTurn)
			{
				fTemp = 300.0 * pow(abs(fAxisValue) / 800.0, 1.3);

				if (fTemp > 14000.0)
					fTemp = 14000.0;

				fAxisValue = (fAxisValue > 0.0) ? fTemp : -fTemp;
			}
		}

		fAxisValue /= 32768.0;

		switch (dwAxisMap[i])
		{
			case AxisForward:
			{
				if ((joy_advanced->value == 0.0) && (in_jlook.state & 1))
				{
					if (fabs(fAxisValue) > joy_pitchthreshold->value)
					{
						if (m_pitch->value < 0.0)
							viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
						else
							viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;

						V_StopPitchDrift();
					}
					else
					{
						if (lookspring->value == 0.0)
						{
							V_StopPitchDrift();
						}
					}
				}
				else
				{
					if (fabs(fAxisValue) > joy_forwardthreshold->value)
					{
						cmd->forwardmove += (fAxisValue * joy_forwardsensitivity->value) * speed * cl_forwardspeed->value;
					}
				}

				break;
			}

			case AxisSide:
			{
				if (fabs(fAxisValue) > joy_sidethreshold->value)
				{
					cmd->sidemove += (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
				}

				break;
			}

			case AxisTurn:
			{
				if ((in_strafe.state & 1) || (lookstrafe->value && (in_jlook.state & 1)))
				{
					if (fabs(fAxisValue) > joy_sidethreshold->value)
					{
						cmd->sidemove -= (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
					}
				}
				else
				{
					if (fabs(fAxisValue) > joy_yawthreshold->value)
					{
						if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
						{
							viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * aspeed * cl_yawspeed->value;
						}
						else
						{
							viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * speed * 180.0;
						}
					}
				}

				break;
			}

			case AxisLook:
			{
				if (in_jlook.state & 1)
				{
					if (fabs(fAxisValue) > joy_pitchthreshold->value)
					{
						if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
						{
							viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
						}
						else
						{
							viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * speed * 180.0;
						}

						V_StopPitchDrift();
					}
					else
					{
						if (lookspring->value == 0.0)
						{
							V_StopPitchDrift();
						}
					}
				}

				break;
			}

			default: break;
		}
	}

	if (viewangles[PITCH] > cl_pitchdown->value)
		viewangles[PITCH] = cl_pitchdown->value;

	if (viewangles[PITCH] < -cl_pitchup->value)
		viewangles[PITCH] = -cl_pitchup->value;

	gEngfuncs.SetViewAngles((float *)viewangles);
}

void IN_Move(float frametime, usercmd_t *cmd)
{
	if (!iMouseInUse && mouseactive)
		IN_MouseMove(frametime, cmd);

	IN_JoyMove(frametime, cmd);
}

void IN_ActiveMouse_f(void)
{
	IN_ActivateMouse();
}

void IN_Init(void)
{
	m_filter = gEngfuncs.pfnGetCvarPointer("m_filter");
	sensitivity = gEngfuncs.pfnGetCvarPointer("sensitivity");

	in_joystick = gEngfuncs.pfnGetCvarPointer("joystick");
	joy_name = gEngfuncs.pfnGetCvarPointer("joyname");
	joy_advanced = gEngfuncs.pfnGetCvarPointer("joyadvanced");
	joy_advaxisx = gEngfuncs.pfnGetCvarPointer("joyadvaxisx");
	joy_advaxisy = gEngfuncs.pfnGetCvarPointer("joyadvaxisy");
	joy_advaxisz = gEngfuncs.pfnGetCvarPointer("joyadvaxisz");
	joy_advaxisr = gEngfuncs.pfnGetCvarPointer("joyadvaxisr");
	joy_advaxisu = gEngfuncs.pfnGetCvarPointer("joyadvaxisu");
	joy_advaxisv = gEngfuncs.pfnGetCvarPointer("joyadvaxisv");
	joy_forwardthreshold = gEngfuncs.pfnGetCvarPointer("joyforwardthreshold");
	joy_sidethreshold = gEngfuncs.pfnGetCvarPointer("joysidethreshold");
	joy_pitchthreshold = gEngfuncs.pfnGetCvarPointer("joypitchthreshold");
	joy_yawthreshold = gEngfuncs.pfnGetCvarPointer("joyyawthreshold");
	joy_forwardsensitivity = gEngfuncs.pfnGetCvarPointer("joyforwardsensitivity");
	joy_sidesensitivity = gEngfuncs.pfnGetCvarPointer("joysidesensitivity");
	joy_pitchsensitivity = gEngfuncs.pfnGetCvarPointer("joypitchsensitivity");
	joy_yawsensitivity = gEngfuncs.pfnGetCvarPointer("joyyawsensitivity");
	joy_wwhack1 = gEngfuncs.pfnGetCvarPointer("joywwhack1");
	joy_wwhack2 = gEngfuncs.pfnGetCvarPointer("joywwhack2");

	m_rawinput = gEngfuncs.pfnGetCvarPointer("m_rawinput");
	m_customaccel = gEngfuncs.pfnGetCvarPointer("m_customaccel");
	m_customaccel_scale = gEngfuncs.pfnGetCvarPointer("m_customaccel_scale");
	m_customaccel_max = gEngfuncs.pfnGetCvarPointer("m_customaccel_max");
	m_customaccel_exponent = gEngfuncs.pfnGetCvarPointer("m_customaccel_exponent");
	m_mousethread_sleep = gEngfuncs.pfnGetCvarPointer("m_mousethread_sleep");

	if (!m_rawinput)
		m_rawinput = gEngfuncs.pfnRegisterVariable("m_rawinput", "1", FCVAR_ARCHIVE);

	if (!m_customaccel)
		m_customaccel = gEngfuncs.pfnRegisterVariable("m_customaccel", "0", FCVAR_ARCHIVE);

	if (!m_customaccel_scale)
		m_customaccel_scale = gEngfuncs.pfnRegisterVariable("m_customaccel_scale", "0.04", FCVAR_ARCHIVE);

	if (!m_customaccel_max)
		m_customaccel_max = gEngfuncs.pfnRegisterVariable("m_customaccel_max", "0", FCVAR_ARCHIVE);

	if (!m_customaccel_exponent)
		m_customaccel_exponent = gEngfuncs.pfnRegisterVariable("m_customaccel_exponent", "1", FCVAR_ARCHIVE);

	if (!m_mousethread_sleep)
		m_mousethread_sleep = gEngfuncs.pfnRegisterVariable("m_mousethread_sleep", "10", FCVAR_ARCHIVE);

	m_bRawInput = false;
	m_bMouseThread = gEngfuncs.CheckParm("-mousethread", NULL) != NULL;

	if (!m_bRawInput && m_bMouseThread && m_mousethread_sleep)
	{
		s_mouseDeltaX = s_mouseDeltaY = 0;
		s_hMouseQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		if (s_hMouseQuitEvent)
			s_hMouseThread = CreateThread(NULL, 0, MousePos_ThreadFunction, NULL, 0, &s_hMouseThreadId);
	}

	Cmd_HookCmd("force_centerview", Force_CenterView_f);
	Cmd_HookCmd("joyadvancedupdate", Joy_AdvancedUpdate_f);

	IN_StartupMouse();
	IN_StartupJoystick();
}

void IN_InstallHook(void)
{
}