#include <metahook.h>
#include <usercmd.h>
#include <entity_state.h>
#include "player.h"
#include "cdll_dll.h"
#include "mathlib.h"
#include "weapons.h"
#include <cvardef.h>
#include "com_weapons.h"

int g_rseq;
int g_gaitseq;
vec3_t g_clorg;
vec3_t g_clang;

void CounterStrike_GetSequence(int *seq, int *gaitseq)
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

void CounterStrike_SetSequence(int seq, int gaitseq)
{
	g_rseq = seq;
	g_gaitseq = gaitseq;
}

void CounterStrike_SetOrientation(float *o, float *a)
{
	VectorCopy(o, g_clorg);
	VectorCopy(a, g_clang);
}

void CounterStrike_GetOrientation(float *o, float *a)
{
	VectorCopy(g_clorg, o);
	VectorCopy(g_clang, a);
}

void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_runfuncs = runfuncs;

	if (cl_lw && cl_lw->value)
	{
		HUD_WeaponsPostThink(from, to, cmd, time, random_seed);
	}
	else
	{
		to->client.fov = g_lastFOV;
	}

	if (runfuncs)
	{
		CounterStrike_SetSequence(to->playerstate.sequence, to->playerstate.gaitsequence);
		CounterStrike_SetOrientation(to->playerstate.origin, cmd->viewangles);
	}

	if(gCkfClientFuncs.HUD_PostRunCmd)
		gCkfClientFuncs.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
}