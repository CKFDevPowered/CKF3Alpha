#include <metahook.h>
#include <r_studioint.h>
#include <entity_state.h>
#include <com_model.h>
#include <studio.h>
#include <cl_entity.h>
#include <cvardef.h>
#include <triangleapi.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "studio_util.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"
#include "client.h"

#define ANIM_WALK_SEQUENCE 3
#define ANIM_JUMP_SEQUENCE 6
#define ANIM_SWIM_1 8
#define ANIM_SWIM_2 9
#define ANIM_FIRST_DEATH_SEQUENCE 101
#define ANIM_LAST_DEATH_SEQUENCE 159
#define ANIM_FIRST_EMOTION_SEQUENCE 198
#define ANIM_LAST_EMOTION_SEQUENCE 207

CGameStudioModelRenderer g_StudioRenderer;

int g_rseq;
int g_gaitseq;
vec3_t g_clorg;
vec3_t g_clang;

void CounterStrike_GetSequence(int *seq, int *gaitseq)
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

void CounterStrike_GetOrientation(float *o, float *a)
{
	VectorCopy(g_clorg, o);
	VectorCopy(g_clang, a);
}

float g_flStartScaleTime;
int iPrevRenderState;
int iRenderStateChanged;

extern engine_studio_api_t IEngineStudio;

#define ANIM_WALK_SEQUENCE 3
#define ANIM_JUMP_SEQUENCE 6
#define ANIM_SWIM_1 8
#define ANIM_SWIM_2 9
#define ANIM_FIRST_DEATH_SEQUENCE 101
#define ANIM_LAST_DEATH_SEQUENCE 159
#define ANIM_FIRST_EMOTION_SEQUENCE 198
#define ANIM_LAST_EMOTION_SEQUENCE 207

static client_anim_state_t g_state;
static client_anim_state_t g_clientstate;

CGameStudioModelRenderer::CGameStudioModelRenderer(void)
{
	m_bLocal = false;
}

mstudioanim_t *CGameStudioModelRenderer::LookupAnimation(mstudioseqdesc_t *pseqdesc, int index)
{
	mstudioanim_t *panim = NULL;

	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	if (index < 0)
		return panim;

	if (index > (pseqdesc->numblends - 1))
		return panim;

	panim += index * m_pStudioHeader->numbones;
	return panim;
}

void CGameStudioModelRenderer::StudioSetupBones(void)
{
	int i;
	double f;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	static vec4_t q[MAXSTUDIOBONES];
	float bonematrix[3][4];

	static float pos2[MAXSTUDIOBONES][3];
	static vec4_t q2[MAXSTUDIOBONES];
	static float pos3[MAXSTUDIOBONES][3];
	static vec4_t q3[MAXSTUDIOBONES];
	static float pos4[MAXSTUDIOBONES][3];
	static vec4_t q4[MAXSTUDIOBONES];

	if (!m_pCurrentEntity->player)
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;
	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	//if (m_pPlayerInfo && !(m_pCurrentEntity->curstate.effects & EF_3DMENU))
	if (m_pPlayerInfo && m_pCurrentEntity->curstate.entityType != ET_HUDENTITY)
	{
		int playerNum = m_pCurrentEntity->index - 1;

		if (m_nPlayerGaitSequences[playerNum] != ANIM_JUMP_SEQUENCE && m_pPlayerInfo->gaitsequence == ANIM_JUMP_SEQUENCE)
			m_pPlayerInfo->gaitframe = 0.0;

		m_nPlayerGaitSequences[playerNum] = m_pPlayerInfo->gaitsequence;
	}

	f = StudioEstimateFrame(pseqdesc);

	if (m_pPlayerInfo->gaitsequence == ANIM_WALK_SEQUENCE)
	{
		if (m_pCurrentEntity->curstate.blending[0] <= 26)
		{
			m_pCurrentEntity->curstate.blending[0] = 0;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
		else
		{
			m_pCurrentEntity->curstate.blending[0] -= 26;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
	}

	if (pseqdesc->numblends == 9)
	{
		float s = m_pCurrentEntity->curstate.blending[0];
		float t = m_pCurrentEntity->curstate.blending[1];

		if (s <= 127.0)
		{
			s = (s * 2.0);

			if (t <= 127.0)
			{
				t = (t * 2.0);

				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0 * (t - 127.0);

				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 6);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}
		else
		{
			s = 2.0 * (s - 127.0);

			if (t <= 127.0)
			{
				t = (t * 2.0);

				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 2);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0 * (t - 127.0);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 8);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}

		s /= 255.0;
		t /= 255.0;

		StudioSlerpBones(q, pos, q2, pos2, s);
		StudioSlerpBones(q3, pos3, q4, pos4, s);
		StudioSlerpBones(q, pos, q3, pos3, t);
	}
	else
	{
		StudioCalcRotations(pos, q, pseqdesc, panim, f);
	}

	if (m_fDoInterp && m_pCurrentEntity->latched.sequencetime && (m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime) && (m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq))
	{
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s = m_pCurrentEntity->latched.prevseqblending[0];
		float t = m_pCurrentEntity->latched.prevseqblending[1];

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim(m_pRenderModel, pseqdesc);

		if (pseqdesc->numblends == 9)
		{
			if (s <= 127.0)
			{
				s = (s * 2.0);

				if (t <= 127.0)
				{
					t = (t * 2.0);

					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0 * (t - 127.0);

					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 6);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}
			else
			{
				s = 2.0 * (s - 127.0);

				if (t <= 127.0)
				{
					t = (t * 2.0);

					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 2);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0 * (t - 127.0);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 8);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}

			s /= 255.0;
			t /= 255.0;

			StudioSlerpBones(q1b, pos1b, q2, pos2, s);
			StudioSlerpBones(q3, pos3, q4, pos4, s);
			StudioSlerpBones(q1b, pos1b, q3, pos3, t);
		}
		else
		{
			StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
		}

		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones(q, pos, q1b, pos1b, s);
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	if (m_pPlayerInfo && (m_pCurrentEntity->curstate.sequence < ANIM_FIRST_DEATH_SEQUENCE || m_pCurrentEntity->curstate.sequence > ANIM_LAST_DEATH_SEQUENCE) && (m_pCurrentEntity->curstate.sequence < ANIM_FIRST_EMOTION_SEQUENCE || m_pCurrentEntity->curstate.sequence > ANIM_LAST_EMOTION_SEQUENCE) && m_pCurrentEntity->curstate.sequence != ANIM_SWIM_1 && m_pCurrentEntity->curstate.sequence != ANIM_SWIM_2)
	{
		int copy = 1;

		if (m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq)
			m_pPlayerInfo->gaitsequence = 0;

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim(m_pRenderModel, pseqdesc);
		StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe);

		for (i = 0; i < m_pStudioHeader->numbones; i++)
		{
			if (!strcmp(pbones[i].name, "Bip01 Spine"))
				copy = 0;
			else if (!strcmp(pbones[pbones[i].parent].name, "Bip01 Pelvis"))
				copy = 1;

			if (copy)
			{
				memcpy(pos[i], pos2[i], sizeof(pos[i]));
				memcpy(q[i], q2[i], sizeof(q[i]));
			}
		}
	}

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			if (gpEngineStudio->IsHardware())
			{
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_pbonetransform)[i]);
				MatrixCopy((*m_pbonetransform)[i], (*m_plighttransform)[i]);
			}
			else
			{
				ConcatTransforms((*m_paliastransform), bonematrix, (*m_pbonetransform)[i]);
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
			}

			StudioFxTransform(m_pCurrentEntity, (*m_pbonetransform)[i]);
		}
		else
		{
			ConcatTransforms((*m_pbonetransform)[pbones[i].parent], bonematrix, (*m_pbonetransform)[i]);
			ConcatTransforms((*m_plighttransform)[pbones[i].parent], bonematrix, (*m_plighttransform)[i]);
		}
	}
}

void CGameStudioModelRenderer::StudioEstimateGait(entity_state_t *pplayer)
{
	float dt;
	vec3_t est_velocity;

	dt = (m_clTime - m_clOldTime);
	dt = max(0.0, dt);
	dt = min(1.0, dt);

	if (dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount)
	{
		m_flGaitMovement = 0;
		return;
	}

	if (m_fGaitEstimation)
	{
		VectorSubtract(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity);
		VectorCopy(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin);
		m_flGaitMovement = VectorLength(est_velocity);

		if (dt <= 0 || m_flGaitMovement / dt < 5)
		{
			m_flGaitMovement = 0;
			est_velocity[0] = 0;
			est_velocity[1] = 0;
		}
	}
	else
	{
		VectorCopy(pplayer->velocity, est_velocity);
		m_flGaitMovement = VectorLength(est_velocity) * dt;
	}

	if (est_velocity[1] == 0 && est_velocity[0] == 0)
	{
		float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;

		if (flYawDiff > 180)
			flYawDiff -= 360;

		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)(m_pPlayerInfo->gaityaw / 360) * 360;

		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = (atan2(est_velocity[1], est_velocity[0]) * 180 / M_PI);

		if (m_pPlayerInfo->gaityaw > 180)
			m_pPlayerInfo->gaityaw = 180;

		if (m_pPlayerInfo->gaityaw < -180)
			m_pPlayerInfo->gaityaw = -180;
	}
}

void CGameStudioModelRenderer::StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch)
{
	float range = 45.0;

	*pBlend = (*pPitch * 3);

	if (*pBlend <= -range)
		*pBlend = 255;
	else if (*pBlend >= range)
		*pBlend = 0;
	else
		*pBlend = 255 * (range - *pBlend) / (2 * range);

	*pPitch = 0;
}

void CGameStudioModelRenderer::CalculatePitchBlend(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;
	int iBlend;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend(pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH]);

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = m_pCurrentEntity->curstate.blending[1];
	m_pCurrentEntity->latched.prevseqblending[1] = m_pCurrentEntity->curstate.blending[1];
}

void CGameStudioModelRenderer::CalculateYawBlend(entity_state_t *pplayer)
{
	float dt;
	float flYaw;

	dt = (m_clTime - m_clOldTime);
	dt = max(0.0, dt);
	dt = min(1.0, dt);

	StudioEstimateGait(pplayer);

	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	flYaw = fmod(flYaw, 360.0f);

	if (flYaw < -180)
		flYaw = flYaw + 360;
	else if (flYaw > 180)
		flYaw = flYaw - 360;

	float maxyaw = 120.0;

	if (flYaw > maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if (flYaw < -maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	float blend_yaw = (flYaw / 90.0) * 128.0 + 127.0;

	blend_yaw = min(255.0, blend_yaw);
	blend_yaw = max(0.0, blend_yaw);

	blend_yaw = 255.0 - blend_yaw;

	m_pCurrentEntity->curstate.blending[0] = (int)(blend_yaw);
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

	if (m_pCurrentEntity->angles[YAW] < -0)
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];
}

void CGameStudioModelRenderer::StudioProcessGait(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;
	float dt;

	CalculateYawBlend(pplayer);
	CalculatePitchBlend(pplayer);

	dt = (m_clTime - m_clOldTime);
	dt = max(0.0, dt);
	dt = min(1.0, dt);

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;

	if (pseqdesc->linearmovement[0] > 0)
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;

	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_pPlayerInfo->gaitframe < 0)
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

void CGameStudioModelRenderer::SavePlayerState(entity_state_t *pplayer)
{
	client_anim_state_t *st;
	cl_entity_t *ent = gpEngineStudio->GetCurrentEntity();

	if (!ent)
		return;

	st = &g_state;

	VectorCopy(ent->curstate.angles, st->angles);
	VectorCopy(ent->curstate.origin, st->origin);

	VectorCopy(ent->angles, st->realangles);

	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy(st->blending, ent->curstate.blending, 2);
	memcpy(st->controller, ent->curstate.controller, 4);

	st->lv = ent->latched;
}

void GetSequenceInfo(void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed)
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr)
		return;

	mstudioseqdesc_t *pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt(pseqdesc->linearmovement[0] * pseqdesc->linearmovement[0] + pseqdesc->linearmovement[1] * pseqdesc->linearmovement[1] + pseqdesc->linearmovement[2] * pseqdesc->linearmovement[2]);
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

int GetSequenceFlags(void *pmodel, client_anim_state_t *pev)
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq)
		return 0;

	mstudioseqdesc_t *pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}

float StudioFrameAdvance(client_anim_state_t *st, float framerate, float flInterval)
{
	if (flInterval == 0.0)
	{
		flInterval = (gEngfuncs.GetClientTime() - st->animtime);

		if (flInterval <= 0.001)
		{
			st->animtime = gEngfuncs.GetClientTime();
			return 0.0;
		}
	}

	if (!st->animtime)
		flInterval = 0.0;

	st->frame += flInterval * framerate * st->framerate;
	st->animtime = gEngfuncs.GetClientTime();

	if (st->frame < 0.0 || st->frame >= 256.0)
	{
		if (st->m_fSequenceLoops)
			st->frame -= (int)(st->frame / 256.0) * 256.0;
		else
			st->frame = (st->frame < 0.0) ? 0 : 255;

		st->m_fSequenceFinished = TRUE;
	}

	return flInterval;
}

void CGameStudioModelRenderer::SetupClientAnimation(entity_state_t *pplayer)
{
	static double oldtime;
	double curtime, dt;

	client_anim_state_t *st;
	float fr, gs;

	cl_entity_t *ent = gpEngineStudio->GetCurrentEntity();

	if (!ent)
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = curtime - oldtime;
	dt = min(1.0, max(0.0, dt));

	oldtime = curtime;
	st = &g_clientstate;

	st->framerate = 1.0;

	int oldseq = st->sequence;
	CounterStrike_GetSequence(&st->sequence, &st->gaitsequence);
	CounterStrike_GetOrientation((float *)&st->origin, (float *)&st->angles);
	VectorCopy(st->angles, st->realangles);

	if (st->sequence != oldseq)
	{
		st->frame = 0.0;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		memcpy(st->lv.prevseqblending, st->blending, 2);
		memcpy(st->lv.prevcontroller, st->controller, 4);
	}

	void *pmodel = (studiohdr_t *)gpEngineStudio->Mod_Extradata(ent->model);

	GetSequenceInfo(pmodel, st, &fr, &gs);
	st->m_fSequenceLoops = ((GetSequenceFlags(pmodel, st) & STUDIO_LOOPING) != 0);
	StudioFrameAdvance(st, fr, dt);

	VectorCopy(st->realangles, ent->angles);
	VectorCopy(st->angles, ent->curstate.angles);
	VectorCopy(st->origin, ent->curstate.origin);

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy(ent->curstate.blending, st->blending, 2);
	memcpy(ent->curstate.controller, st->controller, 4);

	ent->latched = st->lv;
}

void CGameStudioModelRenderer::RestorePlayerState(entity_state_t *pplayer)
{
	client_anim_state_t *st;
	cl_entity_t *ent = gpEngineStudio->GetCurrentEntity();

	if (!ent)
		return;

	st = &g_clientstate;

	VectorCopy(ent->curstate.angles, st->angles);
	VectorCopy(ent->curstate.origin, st->origin);
	VectorCopy(ent->angles, st->realangles);

	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy(st->blending, ent->curstate.blending, 2);
	memcpy(st->controller, ent->curstate.controller, 4);

	st->lv = ent->latched;

	st = &g_state;

	VectorCopy(st->angles, ent->curstate.angles);
	VectorCopy(st->origin, ent->curstate.origin);
	VectorCopy(st->realangles, ent->angles);

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy(ent->curstate.blending, st->blending, 2);
	memcpy(ent->curstate.controller, st->controller, 4);

	ent->latched = st->lv;
}

int CGameStudioModelRenderer::StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	int iret = 0;
	bool isLocalPlayer = false;

	if (m_bLocal && gpEngineStudio->GetCurrentEntity() == gEngfuncs.GetLocalPlayer())
		isLocalPlayer = true;

	if (isLocalPlayer)
	{
		SavePlayerState(pplayer);
		SetupClientAnimation(pplayer);
	}

	iret = _StudioDrawPlayer(flags, pplayer);

	if (isLocalPlayer)
		RestorePlayerState(pplayer);

	return iret;
}

bool WeaponHasAttachments(entity_state_t *pplayer)
{
	studiohdr_t *modelheader = NULL;
	model_t *pweaponmodel;

	if (!pplayer)
		return false;

	pweaponmodel = gpEngineStudio->GetModelByIndex(pplayer->weaponmodel);
	modelheader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(pweaponmodel);

	return (modelheader->numattachments != 0);
}

int CGameStudioModelRenderer::_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	g_bRenderPlayerWeapon = 0;

	m_pCurrentEntity = gpEngineStudio->GetCurrentEntity();

	gpEngineStudio->GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	gpEngineStudio->GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	gpEngineStudio->GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_nPlayerIndex = pplayer->number - 1;

	if (m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients())
		return 0;

	m_pRenderModel = gpEngineStudio->SetupPlayerModel(m_nPlayerIndex);

	if (m_pRenderModel == NULL)
		return 0;

	m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(m_pRenderModel);
	gpEngineStudio->StudioSetHeader(m_pStudioHeader);
	gpEngineStudio->SetRenderModel(m_pRenderModel);

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	if (pplayer->sequence >= m_pStudioHeader->numseq)
		pplayer->sequence = 0;

	if (m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.gaitsequence = 0;

	if (pplayer->gaitsequence >= m_pStudioHeader->numseq)
		pplayer->gaitsequence = 0;

	entity_bones_t *bones = GetEntityBones();

	if (pplayer->gaitsequence)
	{
		vec3_t orig_angles;
		m_pPlayerInfo = gpEngineStudio->PlayerInfo(m_nPlayerIndex);

		VectorCopy(m_pCurrentEntity->angles, orig_angles);

		StudioProcessGait(pplayer);

		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = NULL;

		StudioSetUpTransform(0);
		VectorCopy(orig_angles, m_pCurrentEntity->angles);
	}
	else
	{
		m_pCurrentEntity->curstate.controller[0] = 127;
		m_pCurrentEntity->curstate.controller[1] = 127;
		m_pCurrentEntity->curstate.controller[2] = 127;
		m_pCurrentEntity->curstate.controller[3] = 127;
		m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
		m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
		m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
		m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

		m_pPlayerInfo = gpEngineStudio->PlayerInfo(m_nPlayerIndex);

		CalculatePitchBlend(pplayer);
		CalculateYawBlend(pplayer);

		m_pPlayerInfo->gaitsequence = 0;
		StudioSetUpTransform(0);
	}

	if (flags & STUDIO_RENDER)
	{
		if(m_pCurrentEntity->curstate.entityType != ET_HUDENTITY)//if(!(m_pCurrentEntity->curstate.effects & EF_3DMENU))
		{
			if (!gpEngineStudio->StudioCheckBBox())
				return 0;
		}

		(*m_pModelsDrawn)++;
		(*m_pStudioModelCount)++;

		if (m_pStudioHeader->numbodyparts == 0)
			return 1;
	}

	m_pPlayerInfo = gpEngineStudio->PlayerInfo(m_nPlayerIndex);

	StudioSetupBones();
	StudioSaveBones();

	m_pPlayerInfo->renderframe = m_nFrameCount;
	m_pPlayerInfo = NULL;

	//we don't have a weapon so we just CalcAttachments here, if we have, we CalcAttachments when we render weapons
	if ((flags & STUDIO_EVENTS) && (!(flags & STUDIO_RENDER) || !pplayer->weaponmodel || !WeaponHasAttachments(pplayer)))
	{
		StudioCalcAttachments();
		gpEngineStudio->StudioClientEvents();

		if (m_pCurrentEntity->index > 0)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(m_pCurrentEntity->index);
			memcpy(ent->attachment, m_pCurrentEntity->attachment, sizeof(vec3_t) * 4);
		}
	}

	if (flags & STUDIO_RENDER)
	{
		alight_t lighting;
		vec3_t dir;

		lighting.plightvec = dir;

		gpEngineStudio->StudioDynamicLight(m_pCurrentEntity, &lighting);
		StudioSpecialLight(&lighting);
		gpEngineStudio->StudioEntityLight(&lighting);
		gpEngineStudio->StudioSetupLighting(&lighting);

		m_pPlayerInfo = gpEngineStudio->PlayerInfo(m_nPlayerIndex);
		m_nTopColor = m_pPlayerInfo->topcolor;

		if (m_nTopColor < 0)
			m_nTopColor = 0;

		if (m_nTopColor > 360)
			m_nTopColor = 360;

		m_nBottomColor = m_pPlayerInfo->bottomcolor;

		if (m_nBottomColor < 0)
			m_nBottomColor = 0;

		if (m_nBottomColor > 360)
			m_nBottomColor = 360;

		gpEngineStudio->StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		StudioRenderModel(dir);
		m_pPlayerInfo = NULL;

		if (pplayer->weaponmodel)
		{
			studiohdr_t *saveheader = m_pStudioHeader;
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = gpEngineStudio->GetModelByIndex(pplayer->weaponmodel);

			m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(pweaponmodel);
			gpEngineStudio->StudioSetHeader(m_pStudioHeader);

			g_bRenderPlayerWeapon = 1;

			StudioMergeBones(pweaponmodel);

			SaveEntityBones();

			//gpEngineStudio->StudioDynamicLight(m_pCurrentEntity, &lighting);
			//StudioSpecialLight(&lighting);
			//gpEngineStudio->StudioEntityLight(&lighting);
			gpEngineStudio->StudioSetupLighting(&lighting);

			StudioRenderModel(dir);

			StudioCalcAttachments();

			if (m_pCurrentEntity->index > 0)
				memcpy(saveent.attachment, m_pCurrentEntity->attachment, sizeof(vec3_t) * m_pStudioHeader->numattachments);

			*m_pCurrentEntity = saveent;
			m_pStudioHeader = saveheader;
			gpEngineStudio->StudioSetHeader(m_pStudioHeader);

			if (flags & STUDIO_EVENTS)
				gpEngineStudio->StudioClientEvents();
		}
	}
	return 1;
}

void CGameStudioModelRenderer::StudioFxTransform(cl_entity_t *ent, float transform[3][4])
{
	switch (ent->curstate.renderfx)
	{
		case kRenderFxDistort:
		case kRenderFxHologram:
		{
			if (gEngfuncs.pfnRandomLong(0, 49) == 0)
			{
				int axis = gEngfuncs.pfnRandomLong(0, 1);

				if (axis == 1)
					axis = 2;

				VectorScale(transform[axis], gEngfuncs.pfnRandomFloat(1, 1.484));
			}
			else if (gEngfuncs.pfnRandomLong(0, 49) == 0)
			{
				float offset;
				int axis = gEngfuncs.pfnRandomLong(0, 1);

				if (axis == 1)
					axis = 2;

				offset = gEngfuncs.pfnRandomFloat(-10, 10);
				transform[gEngfuncs.pfnRandomLong(0, 2)][3] += offset;
			}

			break;
		}

		case kRenderFxExplode:
		{
			if (iRenderStateChanged)
			{
				g_flStartScaleTime = m_clTime;
				iRenderStateChanged = FALSE;
			}

			float flTimeDelta = m_clTime - g_flStartScaleTime;

			if (flTimeDelta > 0)
			{
				float flScale = 0.001;

				if (flTimeDelta <= 2.0)
					flScale = 1.0 - (flTimeDelta / 2.0);

				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
						transform[i][j] *= flScale;
				}
			}

			break;
		}
	}
}

void CGameStudioModelRenderer::StudioDrawPlayer_3DHUD(void)
{
	static entity_state_t player;
	static player_info_t playerinfo;
	alight_t lighting;
	vec3_t dir;	

	g_fLOD = 1;
	g_bRenderPlayerWeapon = 0;
	g_iLODLevel = 0;

	m_pCurrentEntity = gpEngineStudio->GetCurrentEntity();
	gpEngineStudio->GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	gpEngineStudio->GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	gpEngineStudio->GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	player.number = gEngfuncs.GetLocalPlayer()->index;
	player.weaponmodel = m_pCurrentEntity->curstate.weaponmodel;
	player.gaitsequence = m_pCurrentEntity->curstate.gaitsequence;
	player.sequence = m_pCurrentEntity->curstate.sequence;
	VectorClear(player.velocity);

	VectorCopy(m_pCurrentEntity->angles, player.angles);
	VectorCopy(m_pCurrentEntity->origin, player.origin);

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata (m_pRenderModel);
	gpEngineStudio->StudioSetHeader( m_pStudioHeader );
	gpEngineStudio->SetRenderModel( m_pRenderModel );

	vec3_t orig_angles;
	m_pPlayerInfo = &playerinfo;
	m_pPlayerInfo->gaityaw = m_pCurrentEntity->angles[1];
	m_pPlayerInfo->gaitsequence = player.gaitsequence;
	VectorCopy(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin);

	VectorCopy( m_pCurrentEntity->angles, orig_angles );
	m_pCurrentEntity->angles[0] = m_pCurrentEntity->curstate.vuser1[0];
	m_pCurrentEntity->angles[1] = orig_angles[1] + m_pCurrentEntity->curstate.vuser1[1];
	StudioProcessGait(&player);
	m_pPlayerInfo->gaitsequence = player.gaitsequence;
	m_pPlayerInfo = NULL;
	VectorCopy( orig_angles, m_pCurrentEntity->angles );

	//hack hack
	m_pCurrentEntity->angles[2] = -refdef->viewangles[2];

	StudioSetUpTransform(0);

	if (!gpEngineStudio->StudioCheckBBox())
		return;

	(*m_pModelsDrawn)++;
	(*m_pStudioModelCount)++;

	if (m_pStudioHeader->numbodyparts == 0)
		return;

	m_pPlayerInfo = &playerinfo;

	StudioSetupBones();
	StudioSaveBones();

	m_pPlayerInfo->renderframe = m_nFrameCount;
	m_pPlayerInfo = NULL;

	lighting.plightvec = dir;
	gpEngineStudio->StudioDynamicLight(m_pCurrentEntity, &lighting );
	StudioSpecialLight(&lighting);
	gpEngineStudio->StudioEntityLight(&lighting);
	gpEngineStudio->StudioSetupLighting(&lighting);

	m_pPlayerInfo = &playerinfo;

	m_nTopColor = m_pCurrentEntity->curstate.colormap & 0xFF;
	m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;
	m_pPlayerInfo->topcolor = m_nTopColor;
	m_pPlayerInfo->bottomcolor = m_nBottomColor;
	gpEngineStudio->StudioSetRemapColors( m_nTopColor, m_nBottomColor );

	StudioRenderModel(dir);
	m_pPlayerInfo = NULL;

	if (m_pCurrentEntity->curstate.weaponmodel > 0)
	{
		cl_entity_t saveent = *m_pCurrentEntity;

		model_t *pweaponmodel = gpEngineStudio->GetModelByIndex( m_pCurrentEntity->curstate.weaponmodel );

		m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(pweaponmodel);
		gpEngineStudio->StudioSetHeader( m_pStudioHeader );

		g_bRenderPlayerWeapon = 1;

		StudioMergeBones(pweaponmodel);

		gpEngineStudio->StudioSetupLighting(&lighting);

		StudioRenderModel(dir);

		StudioCalcAttachments();

		*m_pCurrentEntity = saveent;
	}

	g_fLOD = 0;
}

void CGameStudioModelRenderer::PM_StudioSetupBones(int playerindex)
{
	static cl_entity_t saveentity;
	cl_entity_t *pPlayer = gEngfuncs.GetEntityByIndex(playerindex);
	if(!pPlayer || !pPlayer->model || !pPlayer->player)
		return;
	memcpy(&saveentity, pPlayer, sizeof(cl_entity_t));
	m_pCurrentEntity = pPlayer;
	m_nPlayerIndex = playerindex - 1;
	m_pPlayerInfo = gpEngineStudio->PlayerInfo( m_nPlayerIndex );
	m_pRenderModel = pPlayer->model;
	m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(m_pRenderModel);
	gpEngineStudio->StudioSetHeader( m_pStudioHeader );
	gpEngineStudio->SetRenderModel( m_pRenderModel );
	StudioProcessGait(&m_pCurrentEntity->curstate);
	StudioSetUpTransform(0);
	StudioSetupBones();
	StudioSaveBones();
	m_pPlayerInfo = NULL;
	memcpy(pPlayer, &saveentity, sizeof(cl_entity_t));
}