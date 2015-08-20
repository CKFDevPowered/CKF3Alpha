#include "hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "in_defs.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"
#include "configs.h"

extern "C" 
{
	float vecNewViewAngles[3];
	int iHasNewViewAngles;
	float vecNewViewOrigin[3];
	int iHasNewViewOrigin;
	int iIsSpectator;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int CL_IsThirdPerson(void);
void CL_CameraOffset(float *ofs);
void V_CalcRefdef(struct ref_params_s *pparams);

extern "C"
{
	void PM_ParticleLine(float *start, float *end, int pcolor, float life, float vert);
	int PM_GetVisEntInfo(int ent);
	int PM_GetPhysEntInfo(int ent);
	void InterpolateAngles(float *start, float *end, float *output, float frac);
	void NormalizeAngles(float *angles);
	float Distance(const float *v1, const float *v2);
	float AngleBetweenVectors(const float *v1, const float *v2);

	float vJumpOrigin[3];
	float vJumpAngles[3];
}

void V_DropPunchAngle(float frametime, float *ev_punchangle);
void VectorAngles(const float *forward, float *angles);

#include "r_studioint.h"
#include "com_model.h"
#include "kbutton.h"

extern engine_studio_api_t IEngineStudio;
extern kbutton_t *in_mlook;

extern cvar_t *cl_forwardspeed;
extern cvar_t *chase_active;
extern cvar_t *scr_ofsx, *scr_ofsy, *scr_ofsz;
extern cvar_t *cl_vsmoothing;
extern cvar_t *cl_vgunlag;

#define	CAM_MODE_RELAX 1
#define CAM_MODE_FOCUS 2

vec3_t v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles, v_lastFacing;
float v_frametime, v_lastDistance;
float v_cameraRelaxAngle = 5.0f;
float v_cameraFocusAngle = 35.0f;
int v_cameraMode = CAM_MODE_FOCUS;
qboolean v_resetCamera = 1;

vec3_t ev_punchangle;

cvar_t *scr_ofsx;
cvar_t *scr_ofsy;
cvar_t *scr_ofsz;

cvar_t *v_centermove;
cvar_t *v_centerspeed;

cvar_t *cl_bobcycle;
cvar_t *cl_bob;
cvar_t *cl_bobup;
cvar_t *cl_waterdist;
cvar_t *cl_chasedist;
cvar_t *cl_viewlag;

cvar_t v_iyaw_cycle = { "v_iyaw_cycle", "2", 0, 2 };
cvar_t v_iroll_cycle = { "v_iroll_cycle", "0.5", 0, 0.5 };
cvar_t v_ipitch_cycle = { "v_ipitch_cycle", "1", 0, 1 };
cvar_t v_iyaw_level = { "v_iyaw_level", "0.3", 0, 0.3 };
cvar_t v_iroll_level = { "v_iroll_level", "0.1", 0, 0.1 };
cvar_t v_ipitch_level = { "v_ipitch_level", "0.3", 0, 0.3 };

float v_idlescale;

float V_CalcBob(struct ref_params_s *pparams)
{
	static double bobtime;
	static float bob;
	float cycle;
	static float lasttime;
	vec3_t vel;

	if (pparams->onground == -1 || pparams->time == lasttime)
		return bob;

	lasttime = pparams->time;
	bobtime += pparams->frametime;
	cycle = bobtime - (int)(bobtime / cl_bobcycle->value) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;

	if (cycle < cl_bobup->value)
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle = M_PI + M_PI * (cycle - cl_bobup->value) / (1.0 - cl_bobup->value);

	VectorCopy(pparams->simvel, vel);
	vel[2] = 0;

	bob = sqrt(vel[0] * vel[0] + vel[1] * vel[1]) * cl_bob->value;
	bob = bob * 0.3 + bob * 0.7 * sin(cycle);
	bob = min(bob, 4);
	bob = max(bob, -7);
	return bob;
}

float V_CalcRoll(vec3_t angles, vec3_t velocity, float rollangle, float rollspeed)
{
	float sign;
	float side;
	float value;
	vec3_t forward, right, up;

	AngleVectors(angles, forward, right, up);

	side = DotProduct(velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = rollangle;

	if (side < rollspeed)
		side = side * value / rollspeed;
	else
		side = value;

	return side * sign;
}

typedef struct pitchdrift_s
{
	float pitchvel;
	int nodrift;
	float driftmove;
	double laststop;
}
pitchdrift_t;

static pitchdrift_t pd;

void V_StartPitchDrift(void)
{
	if (pd.laststop == gEngfuncs.GetClientTime())
		return;

	if (pd.nodrift || !pd.pitchvel)
	{
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift(void)
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift = 1;
	pd.pitchvel = 0;
}

void V_DriftPitch(struct ref_params_s *pparams)
{
	float delta, move;

	if (gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator)
	{
		pd.driftmove = 0;
		pd.pitchvel = 0;
		return;
	}

	if (pd.nodrift)
	{
		if (fabs(pparams->cmd->forwardmove) < cl_forwardspeed->value)
			pd.driftmove = 0;
		else
			pd.driftmove += pparams->frametime;

		if (pd.driftmove > v_centermove->value)
			V_StartPitchDrift();

		return;
	}

	delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

	if (!delta)
	{
		pd.pitchvel = 0;
		return;
	}

	move = pparams->frametime * pd.pitchvel;
	pd.pitchvel += pparams->frametime * v_centerspeed->value;

	if (delta > 0)
	{
		if (move > delta)
		{
			pd.pitchvel = 0;
			move = delta;
		}

		pparams->cl_viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			pd.pitchvel = 0;
			move = -delta;
		}

		pparams->cl_viewangles[PITCH] -= move;
	}
}

void V_CalcGunAngle(struct ref_params_s *pparams)
{
	cl_entity_t *viewent = gEngfuncs.GetViewModel();

	if (!viewent)
		return;

	viewent->angles[YAW] = pparams->viewangles[YAW] + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
	viewent->angles[ROLL] -= v_idlescale * sin(pparams->time * v_iroll_cycle.value) * v_iroll_level.value;

	viewent->angles[PITCH] -= v_idlescale * sin(pparams->time * v_ipitch_cycle.value) * (v_ipitch_level.value * 0.5);
	viewent->angles[YAW] -= v_idlescale * sin(pparams->time * v_iyaw_cycle.value) * v_iyaw_level.value;

	VectorCopy(viewent->angles, viewent->curstate.angles);
	VectorCopy(viewent->angles, viewent->latched.prevangles);
}

void V_AddIdle(struct ref_params_s *pparams)
{
	pparams->viewangles[ROLL] += v_idlescale * sin(pparams->time * v_iroll_cycle.value) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin(pparams->time * v_ipitch_cycle.value) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin(pparams->time * v_iyaw_cycle.value) * v_iyaw_level.value;
}

void V_CalcViewRoll(struct ref_params_s *pparams)
{
	float side;
	cl_entity_t *viewentity;

	viewentity = gEngfuncs.GetEntityByIndex(pparams->viewentity);

	if (!viewentity)
		return;

	side = V_CalcRoll(viewentity->angles, pparams->simvel, pparams->movevars->rollangle, pparams->movevars->rollspeed);

	pparams->viewangles[ROLL] += side;

	if (pparams->health <= 0 && (pparams->viewheight[2] != 0))
	{
		pparams->viewangles[ROLL] = 80;
		return;
	}
}

void V_CalcViewLag(struct ref_params_s *pparams)
{
	float speed;
	float diff;
	vec3_t newOrigin;
	vec3_t vecDiff;
	vec3_t forward, right, up;
	float pitch;

	AngleVectors(pparams->viewangles, forward, right, up);
	VectorSubtract(forward, v_lastFacing, vecDiff);

	if (pparams->frametime != 0.0f)
	{
		speed = 5.0f;
		diff = Length(vecDiff);

		if ((diff > cl_viewlag->value) && (cl_viewlag->value > 0.0f))
			speed *= diff / (cl_viewlag->value * 0.5f);

		VectorMA(v_lastFacing, speed * pparams->frametime, vecDiff, v_lastFacing);
		VectorNormalize(v_lastFacing);
	}

	if (cl_viewlag->value > 0.0f)
	{
		pitch = pparams->viewangles[PITCH];

		if (pitch > 180.0f)
			pitch -= 360.0f;
		else if (pitch < -180.0f)
			pitch += 360.0f;

		VectorMA(pparams->vieworg, 1.75f, vecDiff, newOrigin);
		VectorMA(newOrigin, -pitch * 0.035f, forward, newOrigin);
		VectorMA(newOrigin, -pitch * 0.03f, right, newOrigin);
		VectorMA(newOrigin, -pitch * 0.02f, up, newOrigin);
		VectorCopy(newOrigin, pparams->vieworg);
	}
}

void V_CalcIntermissionRefdef(struct ref_params_s *pparams)
{
	cl_entity_t *ent, *view;
	float old;

	ent = gEngfuncs.GetLocalPlayer();
	view = gEngfuncs.GetViewModel();

	VectorCopy(pparams->simorg, pparams->vieworg);
	VectorCopy(pparams->cl_viewangles, pparams->viewangles);

	view->model = NULL;

	old = v_idlescale;
	v_idlescale = 1;

	V_AddIdle(pparams);

	if (gEngfuncs.IsSpectateOnly())
	{
		VectorCopy(gHUD.m_Spectator.m_cameraOrigin, pparams->vieworg);
		VectorCopy(gHUD.m_Spectator.m_cameraAngles, pparams->viewangles);
	}

	v_idlescale = old;

	v_cl_angles = pparams->cl_viewangles;
	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
}

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK (ORIGIN_BACKUP - 1)

typedef struct
{
	float Origins[ORIGIN_BACKUP][3];
	float OriginTime[ORIGIN_BACKUP];

	float Angles[ORIGIN_BACKUP][3];
	float AngleTime[ORIGIN_BACKUP];

	int CurrentOrigin;
	int CurrentAngle;
}
viewinterp_t;

void V_CalcNormalRefdef(struct ref_params_s *pparams)
{
	cl_entity_t *ent, *view;
	int i;
	vec3_t angles;
	float bob, waterOffset;
	static viewinterp_t ViewInterp;

	static float oldz = 0;
	static float lasttime;

	vec3_t camAngles, camForward, camRight, camUp;
	cl_entity_t *pwater;

	V_DriftPitch(pparams);

	if (gEngfuncs.IsSpectateOnly())
		ent = gEngfuncs.GetEntityByIndex(g_iUser2);
	else
		ent = gEngfuncs.GetLocalPlayer();

	view = gEngfuncs.GetViewModel();
	bob = V_CalcBob(pparams);

	VectorCopy(pparams->simorg, pparams->vieworg);
	pparams->vieworg[2] += (bob);
	VectorAdd(pparams->vieworg, pparams->viewheight, pparams->vieworg);

	VectorCopy(pparams->cl_viewangles, pparams->viewangles);

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake(pparams->vieworg, pparams->viewangles, 1.0);

	pparams->vieworg[0] += 1.0 / 32;
	pparams->vieworg[1] += 1.0 / 32;
	pparams->vieworg[2] += 1.0 / 32;

	waterOffset = 0;

	if (pparams->waterlevel >= 2)
	{
		int i, contents, waterDist, waterEntity;
		vec3_t point;
		waterDist = cl_waterdist->value;

		if (pparams->hardware)
		{
			waterEntity = gEngfuncs.PM_WaterEntity(pparams->simorg);

			if (waterEntity >= 0 && waterEntity < pparams->max_entities)
			{
				pwater = gEngfuncs.GetEntityByIndex(waterEntity);

				if (pwater && (pwater->model != NULL))
				{
					waterDist += (pwater->curstate.scale * 16);
				}
			}
		}
		else
		{
			waterEntity = 0;
		}

		VectorCopy(pparams->vieworg, point);

		if (pparams->waterlevel == 2)
		{
			point[2] -= waterDist;

			for (i = 0; i < waterDist; i++)
			{
				contents = gEngfuncs.PM_PointContents(point, NULL);

				if (contents > CONTENTS_WATER)
					break;

				point[2] += 1;
			}

			waterOffset = (point[2] + waterDist) - pparams->vieworg[2];
		}
		else
		{
			point[2] += waterDist;

			for (i = 0; i < waterDist; i++)
			{
				contents = gEngfuncs.PM_PointContents(point, NULL);

				if (contents <= CONTENTS_WATER)
					break;

				point[2] -= 1;
			}

			waterOffset = (point[2] - waterDist) - pparams->vieworg[2];
		}
	}

	pparams->vieworg[2] += waterOffset;

	V_CalcViewRoll(pparams);
	V_AddIdle(pparams);
	V_CalcViewLag(pparams);

	VectorCopy(pparams->cl_viewangles, angles);
	AngleVectors(angles, pparams->forward, pparams->right, pparams->up);

	if (pparams->maxclients <= 1)
	{
		for (i = 0; i < 3; i++)
			pparams->vieworg[i] += scr_ofsx->value * pparams->forward[i] + scr_ofsy->value * pparams->right[i] + scr_ofsz->value * pparams->up[i];
	}

	if (CL_IsThirdPerson())
	{
		vec3_t ofs;
		ofs[0] = ofs[1] = ofs[2] = 0.0;

		CL_CameraOffset((float *)&ofs);

		VectorCopy(ofs, camAngles);
		camAngles[ROLL]	= 0;

		AngleVectors(camAngles, camForward, camRight, camUp);

		for (i = 0; i < 3; i++)
		{
			pparams->vieworg[i] += -ofs[2] * camForward[i];
		}
	}

	VectorCopy(pparams->cl_viewangles, view->angles);

	V_CalcGunAngle(pparams);

	VectorCopy(pparams->simorg, view->origin);
	view->origin[2] += (waterOffset);
	VectorAdd(view->origin, pparams->viewheight, view->origin);

	gEngfuncs.V_ApplyShake(view->origin, view->angles, 0.9);

	for (i = 0; i < 3; i++)
	{
		view->origin[i] += bob * 0.4 * pparams->forward[i];
	}

	view->origin[2] += bob;

	view->angles[YAW] -= bob * 0.5;
	view->angles[ROLL] -= bob * 1;
	view->angles[PITCH] -= bob * 0.3;

	view->origin[2] -= 1;

	if (pparams->viewsize == 110)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 100)
	{
		view->origin[2] += 2;
	}
	else if (pparams->viewsize == 90)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 80)
	{
		view->origin[2] += 0.5;
	}

	VectorAdd(pparams->viewangles, pparams->punchangle, pparams->viewangles);
	VectorAdd(pparams->viewangles, ev_punchangle, pparams->viewangles);
	V_DropPunchAngle ( pparams->frametime, (float *)&ev_punchangle );

#if 1
	if (!pparams->smoothing && pparams->onground && pparams->simorg[2] - oldz > 0)
	{
		float steptime = pparams->time - lasttime;

		if (steptime < 0)
			steptime = 0;

		oldz += steptime * 150;

		if (oldz > pparams->simorg[2])
			oldz = pparams->simorg[2];

		if (pparams->simorg[2] - oldz > 18)
			oldz = pparams->simorg[2] - 18;

		pparams->vieworg[2] += oldz - pparams->simorg[2];
		view->origin[2] += oldz - pparams->simorg[2];
	}
	else
	{
		oldz = pparams->simorg[2];
	}
#endif

	{
		static float lastorg[3];
		vec3_t delta;

		VectorSubtract(pparams->simorg, lastorg, delta);

		if (Length(delta) != 0.0)
		{
			VectorCopy(pparams->simorg, ViewInterp.Origins[ViewInterp.CurrentOrigin & ORIGIN_MASK]);
			ViewInterp.OriginTime[ViewInterp.CurrentOrigin & ORIGIN_MASK] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy(pparams->simorg, lastorg);
		}
	}

	if (cl_vsmoothing && cl_vsmoothing->value && (pparams->smoothing && (pparams->maxclients > 1)))
	{
		int foundidx;
		int i;
		float t;

		if (cl_vsmoothing->value < 0.0)
			gEngfuncs.Cvar_SetValue("cl_vsmoothing", 0.0);

		t = pparams->time - cl_vsmoothing->value;

		for (i = 1; i < ORIGIN_MASK; i++)
		{
			foundidx = ViewInterp.CurrentOrigin - 1 - i;

			if (ViewInterp.OriginTime[foundidx & ORIGIN_MASK] <= t)
				break;
		}

		if (i < ORIGIN_MASK && ViewInterp.OriginTime[foundidx & ORIGIN_MASK] != 0.0)
		{
			vec3_t delta;
			double frac;
			double dt;
			vec3_t neworg;

			dt = ViewInterp.OriginTime[(foundidx + 1) & ORIGIN_MASK] - ViewInterp.OriginTime[foundidx & ORIGIN_MASK];

			if (dt > 0.0)
			{
				frac = (t - ViewInterp.OriginTime[foundidx & ORIGIN_MASK]) / dt;
				frac = min(1.0, frac);
				VectorSubtract(ViewInterp.Origins[(foundidx + 1) & ORIGIN_MASK], ViewInterp.Origins[foundidx & ORIGIN_MASK], delta);
				VectorMA(ViewInterp.Origins[foundidx & ORIGIN_MASK], frac, delta, neworg);

				if (Length(delta) < 64)
				{
					VectorSubtract(neworg, pparams->simorg, delta);

					VectorAdd(pparams->simorg, delta, pparams->simorg);
					VectorAdd(pparams->vieworg, delta, pparams->vieworg);
					VectorAdd(view->origin, delta, view->origin);

				}
			}
		}
	}

	v_angles = pparams->viewangles;
	v_lastAngles = pparams->viewangles;

	if (CL_IsThirdPerson())
		VectorCopy(camAngles, pparams->viewangles);

	if (pparams->viewentity > pparams->maxclients)
	{
		cl_entity_t *viewentity = gEngfuncs.GetEntityByIndex(pparams->viewentity);

		if (viewentity)
		{
			VectorCopy(viewentity->origin, pparams->vieworg);
			VectorCopy(viewentity->angles, pparams->viewangles);

			v_angles = pparams->viewangles;
		}
	}

	lasttime = pparams->time;
	v_origin = pparams->vieworg;
}

void V_SmoothInterpolateAngles(float *startAngle, float *endAngle, float *finalAngle, float degreesPerSec)
{
	float absd, frac, d, threshhold;

	NormalizeAngles(startAngle);
	NormalizeAngles(endAngle);

	for (int i = 0; i < 3; i++)
	{
		d = endAngle[i] - startAngle[i];

		if (d > 180.0f)
		{
			d -= 360.0f;
		}
		else if (d < -180.0f)
		{
			d += 360.0f;
		}

		absd = fabs(d);

		if (absd > 0.01f)
		{
			frac = degreesPerSec * v_frametime;
			threshhold = degreesPerSec / 4;

			if (absd < threshhold)
			{
				float h = absd / threshhold;
				h *= h;
				frac*= h;
			}

			if (frac > absd)
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if (d > 0)
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}
	}

	NormalizeAngles(finalAngle);
}

void V_GetChaseOrigin(float *angles, float *origin, float distance, float *returnvec)
{
	vec3_t vecEnd;
	vec3_t forward;
	vec3_t vecStart;
	pmtrace_t *trace;
	int maxLoops = 8;

	int ignoreent = -1;

	cl_entity_t *ent = NULL;

	AngleVectors(angles, forward, NULL, NULL);
	VectorScale(forward, -1, forward);
	VectorCopy(origin, vecStart);
	VectorMA(vecStart, distance, forward, vecEnd);

	while (maxLoops > 0)
	{
		trace = gEngfuncs.PM_TraceLine(vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent);

		if (trace->ent <= 0)
			break;

		ent = gEngfuncs.GetEntityByIndex(PM_GetPhysEntInfo(trace->ent));

		if (ent == NULL)
			break;

		if (ent->curstate.solid == SOLID_BSP && !ent->player)
			break;

		if (Distance(trace->endpos, vecEnd) < 1.0f)
		{
			break;
		}
		else
		{
			ignoreent = trace->ent;
			VectorCopy(trace->endpos, vecStart);
		}

		maxLoops--;
	}

	VectorMA(trace->endpos, 4, trace->plane.normal, returnvec);

	v_lastDistance = Distance(trace->endpos, origin);
}

void V_GetDeathCam(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3]; 
	float distance = 168.0f;

	v_lastDistance += v_frametime * 96.0f;

	if (v_resetCamera)
		v_lastDistance = 64.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
		newOrigin[2] += 17;

	if (ent2)
	{
		VectorSubtract(ent2->origin, ent1->origin, newAngle);
		VectorAngles(newAngle, newAngle);
		newAngle[0] = -newAngle[0];
	}
	else
	{
		newAngle[0] = 90.0f;
		newAngle[1] = 0.0f;
		newAngle[2] = 0;
	}

	V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);
	V_GetChaseOrigin(angle, newOrigin, distance, origin);

	VectorCopy(angle, v_lastAngles);
}

void V_GetSingleTargetCam(cl_entity_t *ent1, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;
	qboolean deadPlayer = ent1->player && (ent1->curstate.solid == SOLID_NOT);

	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
	float distance = 112.0f + (16.0f * dfactor);

	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;
	else if (deadPlayer)
		distance *= 1.5f;

	v_lastDistance += v_frametime * 32.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
	{
		if (deadPlayer)
			newOrigin[2] += 2;
		else
			newOrigin[2] += 17;
	}
	else
		newOrigin[2] += 8;

	VectorCopy(ent1->angles, newAngle);

	if (flags & DRC_FLAG_FACEPLAYER)
		newAngle[1] += 180.0f;

	newAngle[0] += 12.5f * dfactor;

	if (flags & DRC_FLAG_FINAL)
		newAngle[0] = 22.5f;

	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);
	V_GetChaseOrigin(angle, newOrigin, distance, origin);
}

float MaxAngleBetweenAngles(float *a1, float *a2)
{
	float d, maxd = 0.0f;

	NormalizeAngles(a1);
	NormalizeAngles(a2);

	for (int i = 0; i < 3; i++)
	{
		d = a2[i] - a1[i];

		if (d > 180)
		{
			d -= 360;
		}
		else if (d < -180)
		{
			d += 360;
		}

		d = fabs(d);

		if (d > maxd)
			maxd=d;
	}

	return maxd;
}

void V_GetDoubleTargetsCam(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	float newAngle[3]; float newOrigin[3]; float tempVec[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;
	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
	float distance = 112.0f + (16.0f * dfactor);

	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;

	v_lastDistance += v_frametime * 32.0f;

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if (ent1->player)
		newOrigin[2] += 17;
	else
		newOrigin[2] += 8;

	VectorSubtract(ent2->origin, ent1->origin, newAngle);
	VectorAngles(newAngle, newAngle);

	newAngle[0] = -newAngle[0];
	newAngle[0] += 12.5f * dfactor;

	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	float d = MaxAngleBetweenAngles(v_lastAngles, newAngle);

	if ((d < v_cameraFocusAngle) && (v_cameraMode == CAM_MODE_RELAX))
	{
		VectorCopy(v_lastAngles, newAngle);
	}
	else if ((d < v_cameraRelaxAngle) && (v_cameraMode == CAM_MODE_FOCUS))
	{
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		v_cameraMode = CAM_MODE_FOCUS;
	}

	if (v_resetCamera || (v_cameraMode == CAM_MODE_RELAX))
	{
		VectorCopy(newAngle, angle);
	}
	else
	{
		V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 180.0f);
	}

	V_GetChaseOrigin(newAngle, newOrigin, distance, origin);

	if (v_lastDistance < 64.0f)
		origin[2] += 16.0f * (1.0f - (v_lastDistance / 64.0f));

	VectorSubtract(ent2->origin, origin, tempVec);
	VectorAngles(tempVec, tempVec);
}

void V_GetDirectedChasePosition(cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin)
{
	if (v_resetCamera)
	{
		v_lastDistance = 4096.0f;
	}

	if ((ent2 == (cl_entity_t *)0xFFFFFFFF) || (ent1->player && (ent1->curstate.solid == SOLID_NOT)))
	{
		V_GetSingleTargetCam(ent1, angle, origin);
	}
	else if (ent2)
	{
		V_GetDoubleTargetsCam(ent1, ent2, angle, origin);
	}
	else
	{
		float newOrigin[3];

		int flags = gHUD.m_Spectator.m_iObserverFlags;
		float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;
		float distance = 112.0f + (16.0f * dfactor);

		if (flags & DRC_FLAG_FINAL)
			distance *= 2.0f;

		v_lastDistance += v_frametime * 32.0f;

		if (distance > v_lastDistance)
			distance = v_lastDistance;

		VectorCopy(ent1->origin, newOrigin);

		if (ent1->player)
			newOrigin[2] += 17;
		else
			newOrigin[2] += 8;

		V_GetChaseOrigin(angle, newOrigin, distance, origin);
	}

	VectorCopy(angle, v_lastAngles);
}

void V_GetChasePos(int target, float *cl_angles, float *origin, float *angles)
{
	cl_entity_t *ent = NULL;

	if (target)
		ent = gEngfuncs.GetEntityByIndex(target);

	if (!ent)
	{
		VectorCopy(vJumpAngles, angles);
		VectorCopy(vJumpOrigin, origin);
		return;
	}

	if (ent->index == gEngfuncs.GetLocalPlayer()->index)
	{
		if (g_iUser3)
			V_GetDeathCam(ent, gEngfuncs.GetEntityByIndex(g_iUser3), angles, origin);
		else
			V_GetDeathCam(ent, NULL, angles, origin);
	}
	else if (gHUD.m_Spectator.m_autoDirector->value)
	{
		if (g_iUser3)
			V_GetDirectedChasePosition(ent, gEngfuncs.GetEntityByIndex(g_iUser3), angles, origin);
		else
			V_GetDirectedChasePosition(ent, (cl_entity_t *)0xFFFFFFFF, angles, origin);
	}
	else
	{
		if (cl_angles == NULL)
		{
			VectorCopy(ent->angles, angles);
			angles[0] *= -1;
		}
		else
			VectorCopy(cl_angles, angles);

		VectorCopy(ent->origin, origin);

		origin[2] += 17;

		V_GetChaseOrigin(angles, origin, cl_chasedist->value, origin);
	}

	v_resetCamera = false;
}

void V_ResetChaseCam(void)
{
	v_resetCamera = true;
}

void V_GetInEyePos(int target, float *origin, float *angles)
{
	if (!target)
	{
		VectorCopy(vJumpAngles, angles);
		VectorCopy(vJumpOrigin, origin);
		return;
	}

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex(target);

	if (!ent)
		return;

	VectorCopy(ent->origin, origin);
	VectorCopy(ent->angles, angles);

	angles[PITCH] *= -3.0f;

	if (ent->curstate.solid == SOLID_NOT)
	{
		angles[ROLL] = 80;
		origin[2] += -8;
	}
	else if (ent->curstate.usehull == 1)
	{
		origin[2] += 12; 
	}
	else
	{
		origin[2] += 17;
	}
}

void V_GetMapFreePosition(float *cl_angles, float *origin, float *angles)
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy(cl_angles, angles);

	angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * ((90.0f - angles[0]) / 90.0f);

	AngleVectors(angles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorMA(zScaledTarget, -(4096.0f / gHUD.m_Spectator.m_mapZoom), forward, origin);
}

void V_GetMapChasePosition(int target, float *cl_angles, float *origin, float *angles)
{
	vec3_t forward;

	if (target)
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex(target);

		if (gHUD.m_Spectator.m_autoDirector->value)
		{
			V_GetChasePos(target, cl_angles, origin, angles);
			VectorCopy(ent->origin, origin);

			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy(cl_angles, angles);
			VectorCopy(ent->origin, origin);

			angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);
		}
	}
	else
	{
		VectorCopy(cl_angles, angles);
		angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);
	}

	origin[2] *= ((90.0f - angles[0]) / 90.0f);
	angles[2] = 0.0f;

	AngleVectors(angles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorMA(origin, -1536, forward, origin);
}

int V_FindViewModelByWeaponModel(int weaponindex)
{
	static char modelName[MAX_PATH];
	struct model_s *weaponModel = IEngineStudio.GetModelByIndex(weaponindex);

	if (weaponModel && weaponModel->name[0])
	{
		int len = strlen(weaponModel->name);

		strncpy(modelName, weaponModel->name, sizeof(modelName) - 1);
		modelName[sizeof(modelName) - 1] = 0;

		char *pos = strstr(modelName, "/p_");

		if (pos)
		{
			pos[1] = 'v';
			return gEngfuncs.pEventAPI->EV_FindModelIndex(modelName);
		}

		return 0;
	}
	else
		return 0;
}

void V_CalcSpectatorRefdef(struct ref_params_s *pparams)
{
	static vec3_t velocity(0.0f, 0.0f, 0.0f);

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex = 0;

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex(g_iUser2);

	pparams->onlyClientDraw = false;

	VectorCopy(pparams->simorg, v_sim_org);

	VectorCopy(pparams->cl_viewangles, v_cl_angles);
	VectorCopy(pparams->viewangles, v_angles);
	VectorCopy(pparams->vieworg, v_origin);

	v_lastFacing[0] = 0;
	v_lastFacing[1] = 0;
	v_lastFacing[2] = 0;

	if ((g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE) && ent)
	{
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if (timeDiff > 0)
		{
			vec3_t distance;
			VectorSubtract(ent->prevstate.origin, ent->curstate.origin, distance);
			VectorScale(distance, 1 / timeDiff, distance);

			velocity[0] = velocity[0] * 0.9f + distance[0] * 0.1f;
			velocity[1] = velocity[1] * 0.9f + distance[1] * 0.1f;
			velocity[2] = velocity[2] * 0.9f + distance[2] * 0.1f;

			VectorCopy(velocity, pparams->simvel);
		}

		if (gEngfuncs.IsSpectateOnly() || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE)
		{
			V_GetInEyePos(g_iUser2, pparams->simorg, pparams->cl_viewangles);

			pparams->health = 1;

			cl_entity_t *gunModel = gEngfuncs.GetViewModel();

			if (lastWeaponModelIndex != ent->curstate.weaponmodel)
			{
				lastWeaponModelIndex = ent->curstate.weaponmodel;
				lastViewModelIndex = V_FindViewModelByWeaponModel(lastWeaponModelIndex);

				if (lastViewModelIndex)
				{
					gEngfuncs.pfnWeaponAnim(0, 0);
				}
				else
				{
					gunModel->model = NULL;
					lastWeaponModelIndex = lastViewModelIndex = 0;
				}
			}

			if (lastViewModelIndex)
			{
				gunModel->model = IEngineStudio.GetModelByIndex(lastViewModelIndex);
				gunModel->curstate.modelindex = lastViewModelIndex;
				gunModel->curstate.frame = 0;
				gunModel->curstate.colormap = 0;
				gunModel->index = g_iUser2;
			}
			else
			{
				gunModel->model = NULL;
			}
		}
		else
		{
			VectorCopy(ent->angles, pparams->cl_viewangles);
			pparams->cl_viewangles[PITCH] *= -3.0f;
		}
	}

	v_frametime = pparams->frametime;

	if (pparams->nextView == 0)
	{
		switch (g_iUser1)
		{
			case OBS_CHASE_LOCKED:
			{
				V_GetChasePos(g_iUser2, NULL, v_origin, v_angles);
				break;
			}

			case OBS_CHASE_FREE:
			{
				V_GetChasePos(g_iUser2, v_cl_angles, v_origin, v_angles);
				break;
			}

			case OBS_ROAMING:
			{
				VectorCopy(v_cl_angles, v_angles);
				VectorCopy(v_sim_org, v_origin);

				gHUD.m_Spectator.GetDirectorCamera(v_origin, v_angles);
				break;
			}

			case OBS_IN_EYE:
			{
				V_CalcNormalRefdef(pparams);
				break;
			}

			case OBS_MAP_FREE:
			{
				pparams->onlyClientDraw = true;
				V_GetMapFreePosition(v_cl_angles, v_origin, v_angles);
				break;
			}

			case OBS_MAP_CHASE:
			{
				pparams->onlyClientDraw = true;
				V_GetMapChasePosition(g_iUser2, v_cl_angles, v_origin, v_angles);
				break;
			}
		}

		if (gHUD.m_Spectator.m_pip->value && gEngfuncs.GetLocalPlayer()->index != g_iUser2)
			pparams->nextView = 1;

		gHUD.m_Spectator.m_iDrawCycle = 0;
	}
	else
	{
		pparams->viewport[0] = gHUD.m_Spectator.m_OverviewData.insetWindowX;
		pparams->viewport[1] = gHUD.m_Spectator.m_OverviewData.insetWindowY;
		pparams->viewport[2] = gHUD.m_Spectator.m_OverviewData.insetWindowWidth;
		pparams->viewport[3] = gHUD.m_Spectator.m_OverviewData.insetWindowHeight;
		pparams->nextView = 0;

		switch ((int)gHUD.m_Spectator.m_pip->value)
		{
			case INSET_CHASE_FREE:
			{
				V_GetChasePos(g_iUser2, v_cl_angles, v_origin, v_angles);
				break;
			}

			case INSET_IN_EYE:
			{
				V_CalcNormalRefdef(pparams);
				break;
			}

			case INSET_MAP_FREE:
			{
				pparams->onlyClientDraw = true;
				V_GetMapFreePosition(v_cl_angles, v_origin, v_angles);
				break;
			}

			case INSET_MAP_CHASE:
			{
				pparams->onlyClientDraw = true;

				if (g_iUser1 == OBS_ROAMING)
					V_GetMapChasePosition(0, v_cl_angles, v_origin, v_angles);
				else
					V_GetMapChasePosition(g_iUser2, v_cl_angles, v_origin, v_angles);

				break;
			}
		}

		gHUD.m_Spectator.m_iDrawCycle = 1;
	}

	VectorCopy(v_cl_angles, pparams->cl_viewangles);
	VectorCopy(v_angles, pparams->viewangles);
	VectorCopy(v_origin, pparams->vieworg);
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	if (pparams->intermission)
	{
		V_CalcIntermissionRefdef(pparams);
	}
	else if (pparams->spectator || g_iUser1)
	{
		V_CalcSpectatorRefdef(pparams);
	}
	else if (!pparams->paused)
	{
		V_CalcNormalRefdef(pparams);
	}

	if (gConfigs.bEnableClientUI)
		g_pViewPort->CalcRefdef(pparams);

	gHUD.CalcRefdef(pparams);

	if(gRefClientFuncs.V_CalcRefdef)
		gRefClientFuncs.V_CalcRefdef(pparams);

	if(gCkfClientFuncs.V_CalcRefdef)
		gCkfClientFuncs.V_CalcRefdef(pparams);
}

void V_DropPunchAngle(float frametime, float *ev_punch)
{
	float len;

	len = VectorNormalize(ev_punch);
	len -= (10.0 + len * 0.5) * frametime;
	len = max(len, 0.0);
	VectorScale(ev_punch, len, ev_punch);
}

float *EV_GetPunchAngles(void)
{
	return &ev_punchangle.x;
}

void V_PunchAxis(int axis, float punch)
{
	ev_punchangle[axis] = punch;
}

void V_Init(void)
{
	Cmd_HookCmd("centerview", V_StartPitchDrift);

	scr_ofsx = gEngfuncs.pfnGetCvarPointer("scr_ofsx");
	scr_ofsy = gEngfuncs.pfnGetCvarPointer("scr_ofsy");
	scr_ofsz = gEngfuncs.pfnGetCvarPointer("scr_ofsz");

	v_centermove = gEngfuncs.pfnGetCvarPointer("v_centermove");
	v_centerspeed = gEngfuncs.pfnGetCvarPointer("v_centerspeed");

	cl_bobcycle = gEngfuncs.pfnGetCvarPointer("cl_bobcycle");
	cl_bob = gEngfuncs.pfnGetCvarPointer("cl_bob");
	cl_bobup = gEngfuncs.pfnGetCvarPointer("cl_bobup");
	cl_waterdist = gEngfuncs.pfnGetCvarPointer("cl_waterdist");
	cl_chasedist = gEngfuncs.pfnGetCvarPointer("cl_chasedist");
	cl_viewlag = gEngfuncs.pfnRegisterVariable("cl_viewlag", "0", FCVAR_ARCHIVE);
}