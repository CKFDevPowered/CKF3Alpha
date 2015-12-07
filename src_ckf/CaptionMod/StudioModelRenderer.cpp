#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "studio_util.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"
#include "cvar.h"
#include "qgl.h"
#include "particle.h"
#include "weapon.h"

extern CGameStudioModelRenderer g_StudioRenderer;

typedef struct pmtrace_s pmtrace_t;

extern model_t *g_mdlPlayer[10];
extern model_t *g_mdlSapper;
extern model_t *g_mdlSpyMask;
extern char *g_szPlayerModel[];

void CStudioModelRenderer::StudioRenderAttach(int flags, model_t *mod)
{
	m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(mod);
	gpEngineStudio->StudioSetHeader( m_pStudioHeader );

	StudioMergeBones(mod);

	if(flags & STUDIO_RENDER)
	{
		StudioRenderModel(NULL);
	}
}

void CStudioModelRenderer::Init(void)
{
	m_pCvarHiModels = gpEngineStudio->GetCvar("cl_himodels");
	m_pCvarDeveloper = gpEngineStudio->GetCvar("developer");
	m_pCvarDrawEntities = gpEngineStudio->GetCvar("r_drawentities");

	m_pChromeSprite = gpEngineStudio->GetChromeSprite();
	m_texFireLayer = gRefExports.R_LoadTextureEx("resource/tga/firelayeredslowtiled512.tga", "firelayeredslowtiled512", NULL, NULL, GLT_SYSTEM, true, false);
	m_texCritLayer[0] = gRefExports.R_LoadTextureEx("resource/tga/invulnerability_red.tga", "invulnerability_red", NULL, NULL, GLT_SYSTEM, true, false);
	m_texCritLayer[1] = gRefExports.R_LoadTextureEx("resource/tga/invulnerability_blue.tga", "invulnerability_blue", NULL, NULL, GLT_SYSTEM, true, false);
	m_texInvulnLayer[0] = gRefExports.R_LoadTextureEx("resource/tga/invuln_red.dds", "invuln_red", NULL, NULL, GLT_SYSTEM, true, false);
	m_texInvulnLayer[1] = gRefExports.R_LoadTextureEx("resource/tga/invuln_blue.dds", "invuln_blue", NULL, NULL, GLT_SYSTEM, true, false);

	gpEngineStudio->GetModelCounters(&m_pStudioModelCount, &m_pModelsDrawn);

	m_pbonetransform = (float (*)[MAXSTUDIOBONES][3][4])gpEngineStudio->StudioGetBoneTransform();
	m_plighttransform = (float (*)[MAXSTUDIOBONES][3][4])gpEngineStudio->StudioGetLightTransform();
	m_paliastransform = (float (*)[3][4])gpEngineStudio->StudioGetAliasTransform();
	m_protationmatrix = (float (*)[3][4])gpEngineStudio->StudioGetRotationMatrix();
}

CStudioModelRenderer::CStudioModelRenderer(void)
{
	m_fDoInterp = 1;
	m_fGaitEstimation = 1;
	m_pCurrentEntity = NULL;
	m_pCvarHiModels = NULL;
	m_pCvarDeveloper = NULL;
	m_pCvarDrawEntities = NULL;
	m_pChromeSprite = NULL;
	m_pStudioModelCount = NULL;
	m_pModelsDrawn = NULL;
	m_protationmatrix = NULL;
	m_paliastransform = NULL;
	m_pbonetransform = NULL;
	m_plighttransform = NULL;
	m_pStudioHeader = NULL;
	m_pBodyPart = NULL;
	m_pSubModel = NULL;
	m_pPlayerInfo = NULL;
	m_pRenderModel = NULL;
}

CStudioModelRenderer::~CStudioModelRenderer(void)
{
}

void CStudioModelRenderer::StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen)
{
	int i, j;
	float value;
	mstudiobonecontroller_t *pbonecontroller;

	pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pStudioHeader + m_pStudioHeader->bonecontrollerindex);

	for (j = 0; j < m_pStudioHeader->numbonecontrollers; j++)
	{
		i = pbonecontroller[j].index;

		if (i <= 3)
		{
			if (pbonecontroller[j].type & STUDIO_RLOOP)
			{
				if (abs(pcontroller1[i] - pcontroller2[i]) > 128)
				{
					int a, b;
					a = (pcontroller1[j] + 128) % 256;
					b = (pcontroller2[j] + 128) % 256;
					value = ((a * dadt) + (b * (1 - dadt)) - 128) * (360.0 / 256.0) + pbonecontroller[j].start;
				}
				else
				{
					value = ((pcontroller1[i] * dadt + (pcontroller2[i]) * (1.0 - dadt))) * (360.0 / 256.0) + pbonecontroller[j].start;
				}
			}
			else
			{
				value = (pcontroller1[i] * dadt + pcontroller2[i] * (1.0 - dadt)) / 255.0;

				if (value < 0)
					value = 0;

				if (value > 1.0)
					value = 1.0;

				value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}
		}
		else
		{
			value = mouthopen / 64.0;

			if (value > 1.0)
				value = 1.0;

			value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
		}

		switch (pbonecontroller[j].type & STUDIO_TYPES)
		{
			case STUDIO_XR:
			case STUDIO_YR:
			case STUDIO_ZR:
			{
				adj[j] = value * (M_PI / 180.0);
				break;
			}
			case STUDIO_X:
			case STUDIO_Y:
			case STUDIO_Z:
			{
				adj[j] = value;
				break;
			}
		}
	}
}

void CStudioModelRenderer::StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q)
{
	int j, k;
	vec4_t q1, q2;
	vec3_t angle1, angle2;
	mstudioanimvalue_t *panimvalue;

	for (j = 0; j < 3; j++)
	{
		if (panim->offset[j + 3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j + 3];
		}
		else
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j + 3]);
			k = frame;

			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;

			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;

				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}

			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k + 1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k + 2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;

				if (panimvalue->num.total > k + 1)
					angle2[j] = angle1[j];
				else
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
			}

			angle1[j] = pbone->value[j + 3] + angle1[j] * pbone->scale[j + 3];
			angle2[j] = pbone->value[j + 3] + angle2[j] * pbone->scale[j + 3];
		}

		if (pbone->bonecontroller[j + 3] != -1)
		{
			angle1[j] += adj[pbone->bonecontroller[j + 3]];
			angle2[j] += adj[pbone->bonecontroller[j + 3]];
		}
	}

	if (!VectorCompare(angle1, angle2))
	{
		AngleQuaternion(angle1, q1);
		AngleQuaternion(angle2, q2);
		QuaternionSlerp(q1, q2, s, q);
	}
	else
	{
		AngleQuaternion(angle1, q);
	}
}

void CStudioModelRenderer::StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos)
{
	int j, k;
	mstudioanimvalue_t *panimvalue;

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j];

		if (panim->offset[j] != 0)
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j]);
			k = frame;

			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;

			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;

				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}

			if (panimvalue->num.valid > k)
			{
				if (panimvalue->num.valid > k + 1)
					pos[j] += (panimvalue[k + 1].value * (1.0 - s) + s * panimvalue[k + 2].value) * pbone->scale[j];
				else
					pos[j] += panimvalue[k + 1].value * pbone->scale[j];
			}
			else
			{
				if (panimvalue->num.total <= k + 1)
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				else
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
			}
		}

		if (pbone->bonecontroller[j] != -1 && adj)
			pos[j] += adj[pbone->bonecontroller[j]];
	}
}

void CStudioModelRenderer::StudioSlerpBones(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s)
{
	int i;
	vec4_t q3;
	float s1;

	if (s < 0)
		s = 0;
	else if (s > 1.0)
		s = 1.0;

	s1 = 1.0 - s;

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionSlerp(q1[i], q2[i], s, q3);

		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];
		pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s;
	}
}

mstudioanim_t *CStudioModelRenderer::StudioGetAnim(model_t *mod, mstudioseqdesc_t *pseqdesc)
{
	mstudioseqgroup_t *pseqgroup;
	cache_user_t *paSequences;

	pseqgroup = (mstudioseqgroup_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqgroupindex) + pseqdesc->seqgroup;

	if (pseqdesc->seqgroup == 0)
		return (mstudioanim_t *)((byte *)m_pStudioHeader + pseqgroup->data + pseqdesc->animindex);

	paSequences = (cache_user_t *)mod->submodels;

	if (paSequences == NULL)
	{
		paSequences = (cache_user_t *)gpEngineStudio->Mem_Calloc(16, sizeof(cache_user_t));
		mod->submodels = (dmodel_t *)paSequences;
	}

	if (!gpEngineStudio->Cache_Check((struct cache_user_s *)&(paSequences[pseqdesc->seqgroup])))
	{
		gEngfuncs.Con_DPrintf("loading %s\n", pseqgroup->name);
		gpEngineStudio->LoadCacheFile(pseqgroup->name, (struct cache_user_s *)&paSequences[pseqdesc->seqgroup]);
	}

	return (mstudioanim_t *)((byte *)paSequences[pseqdesc->seqgroup].data + pseqdesc->animindex);
}

void CStudioModelRenderer::StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch)
{
	*pBlend = (*pPitch * 3);

	if (*pBlend < pseqdesc->blendstart[0])
	{
		*pPitch -= pseqdesc->blendstart[0] / 3.0;
		*pBlend = 0;
	}
	else if (*pBlend > pseqdesc->blendend[0])
	{
		*pPitch -= pseqdesc->blendend[0] / 3.0;
		*pBlend = 255;
	}
	else
	{
		if (pseqdesc->blendend[0] - pseqdesc->blendstart[0] < 0.1)
			*pBlend = 127;
		else
			*pBlend = 255 * (*pBlend - pseqdesc->blendstart[0]) / (pseqdesc->blendend[0] - pseqdesc->blendstart[0]);

		*pPitch = 0;
	}
}

void CStudioModelRenderer::StudioSetUpTransform(int trivial_accept)
{
	int i;
	vec3_t angles;
	vec3_t modelpos;

	VectorCopy(m_pCurrentEntity->origin, modelpos);

	angles[ROLL] = m_pCurrentEntity->curstate.angles[ROLL];
	angles[PITCH] = m_pCurrentEntity->curstate.angles[PITCH];
	angles[YAW] = m_pCurrentEntity->curstate.angles[YAW];

	if (m_pCurrentEntity->curstate.movetype == MOVETYPE_STEP)
	{
		float f = 0;
		float d;

		if ((m_clTime < m_pCurrentEntity->curstate.animtime + 1.0f) && (m_pCurrentEntity->curstate.animtime != m_pCurrentEntity->latched.prevanimtime))
			f = (m_clTime - m_pCurrentEntity->curstate.animtime) / (m_pCurrentEntity->curstate.animtime - m_pCurrentEntity->latched.prevanimtime);

		if (m_fDoInterp)
			f = f - 1.0;
		else
			f = 0;

		for (i = 0; i < 3; i++)
			modelpos[i] += (m_pCurrentEntity->origin[i] - m_pCurrentEntity->latched.prevorigin[i]) * f;

		for (i = 0; i < 3; i++)
		{
			float ang1, ang2;

			ang1 = m_pCurrentEntity->angles[i];
			ang2 = m_pCurrentEntity->latched.prevangles[i];

			d = ang1 - ang2;

			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;

			angles[i] += d * f;
		}
	}
	else if (m_pCurrentEntity->curstate.movetype != MOVETYPE_NONE)
	{
		VectorCopy(m_pCurrentEntity->angles, angles);
	}

	angles[PITCH] = -angles[PITCH];
	AngleMatrix(angles, (*m_protationmatrix));

	if (!gpEngineStudio->IsHardware())
	{
		static float viewmatrix[3][4];

		VectorCopy(m_vRight, viewmatrix[0]);
		VectorCopy(m_vUp, viewmatrix[1]);
		VectorInverse(viewmatrix[1]);
		VectorCopy(m_vNormal, viewmatrix[2]);

		(*m_protationmatrix)[0][3] = modelpos[0] - m_vRenderOrigin[0];
		(*m_protationmatrix)[1][3] = modelpos[1] - m_vRenderOrigin[1];
		(*m_protationmatrix)[2][3] = modelpos[2] - m_vRenderOrigin[2];

		ConcatTransforms(viewmatrix, (*m_protationmatrix), (*m_paliastransform));

		if (trivial_accept)
		{
			for (i = 0; i < 4; i++)
			{
				(*m_paliastransform)[0][i] *= m_fSoftwareXScale * (1.0 / (ZISCALE * 0x10000));
				(*m_paliastransform)[1][i] *= m_fSoftwareYScale * (1.0 / (ZISCALE * 0x10000));
				(*m_paliastransform)[2][i] *= 1.0 / (ZISCALE * 0x10000);
			}
		}
	}

	(*m_protationmatrix)[0][3] = modelpos[0];
	(*m_protationmatrix)[1][3] = modelpos[1];
	(*m_protationmatrix)[2][3] = modelpos[2];
}

float CStudioModelRenderer::StudioEstimateInterpolant(void)
{
	float dadt = 1.0;

	if (m_fDoInterp && (m_pCurrentEntity->curstate.animtime >= m_pCurrentEntity->latched.prevanimtime + 0.01))
	{
		dadt = (m_clTime - m_pCurrentEntity->curstate.animtime) / 0.1;

		if (dadt > 2.0)
			dadt = 2.0;
	}

	return dadt;
}

void CStudioModelRenderer::StudioCalcRotations(float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f)
{
	int i;
	int frame;
	mstudiobone_t *pbone;

	float s;
	float adj[MAXSTUDIOCONTROLLERS];
	float dadt;

	if (f > pseqdesc->numframes - 1)
		f = 0;
	else if (f < -0.01)
		f = -0.01;

	frame = (int)f;
	dadt = StudioEstimateInterpolant();
	s = (f - frame);

	pbone = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	StudioCalcBoneAdj(dadt, adj, m_pCurrentEntity->curstate.controller, m_pCurrentEntity->latched.prevcontroller, m_pCurrentEntity->mouth.mouthopen);

	for (i = 0; i < m_pStudioHeader->numbones; i++, pbone++, panim++)
	{
		StudioCalcBoneQuaterion(frame, s, pbone, panim, adj, q[i]);
		StudioCalcBonePosition(frame, s, pbone, panim, adj, pos[i]);
	}

	if (pseqdesc->motiontype & STUDIO_X)
		pos[pseqdesc->motionbone][0] = 0.0;

	if (pseqdesc->motiontype & STUDIO_Y)
		pos[pseqdesc->motionbone][1] = 0.0;

	if (pseqdesc->motiontype & STUDIO_Z)
		pos[pseqdesc->motionbone][2] = 0.0;

	s = 0 * ((1.0 - (f - (int)(f))) / (pseqdesc->numframes)) * m_pCurrentEntity->curstate.framerate;

	if (pseqdesc->motiontype & STUDIO_LX)
		pos[pseqdesc->motionbone][0] += s * pseqdesc->linearmovement[0];

	if (pseqdesc->motiontype & STUDIO_LY)
		pos[pseqdesc->motionbone][1] += s * pseqdesc->linearmovement[1];

	if (pseqdesc->motiontype & STUDIO_LZ)
		pos[pseqdesc->motionbone][2] += s * pseqdesc->linearmovement[2];
}

void CStudioModelRenderer::StudioFxTransform(cl_entity_t *ent, float transform[3][4])
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
			float scale;

			scale = 1.0 + (m_clTime - ent->curstate.animtime) * 10.0;

			if (scale > 2)
				scale = 2;

			transform[0][1] *= scale;
			transform[1][1] *= scale;
			transform[2][1] *= scale;
			break;
		}
	}
}

float CStudioModelRenderer::StudioEstimateFrame(mstudioseqdesc_t *pseqdesc)
{
	double dfdt, f;

	if (m_fDoInterp)
	{
		if (m_clTime < m_pCurrentEntity->curstate.animtime)
			dfdt = 0;
		else
			dfdt = (m_clTime - m_pCurrentEntity->curstate.animtime) * m_pCurrentEntity->curstate.framerate * pseqdesc->fps;
	}
	else
		dfdt = 0;

	if (pseqdesc->numframes <= 1)
		f = 0;
	else
		f = (m_pCurrentEntity->curstate.frame * (pseqdesc->numframes - 1)) / 256.0;

	f += dfdt;

	if (pseqdesc->flags & STUDIO_LOOPING)
	{
		if (pseqdesc->numframes > 1)
			f -= (int)(f / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);

		if (f < 0)
			f += (pseqdesc->numframes - 1);
	}
	else
	{
		if (f >= pseqdesc->numframes - 1.001)
			f = pseqdesc->numframes - 1.001;

		if (f < 0.0)
			f = 0.0;
	}

	return f;
}

void CStudioModelRenderer::StudioSetupBones(void)
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

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	f = StudioEstimateFrame(pseqdesc);
	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	StudioCalcRotations(pos, q, pseqdesc, panim, f);

	if (pseqdesc->numblends > 1)
	{
		float s;
		float dadt;

		panim += m_pStudioHeader->numbones;
		StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

		dadt = StudioEstimateInterpolant();
		s = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.prevblending[0] * (1.0 - dadt)) / 255.0;

		StudioSlerpBones(q, pos, q2, pos2, s);

		if (pseqdesc->numblends == 4)
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos4, q4, pseqdesc, panim, f);

			s = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.prevblending[0] * (1.0 - dadt)) / 255.0;
			StudioSlerpBones(q3, pos3, q4, pos4, s);

			s = (m_pCurrentEntity->curstate.blending[1] * dadt + m_pCurrentEntity->latched.prevblending[1] * (1.0 - dadt)) / 255.0;
			StudioSlerpBones(q, pos, q3, pos3, s);
		}
	}

	if (m_fDoInterp && m_pCurrentEntity->latched.sequencetime && (m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime) && (m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq))
	{
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s;

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim(m_pRenderModel, pseqdesc);

		StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

		if (pseqdesc->numblends > 1)
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

			s = (m_pCurrentEntity->latched.prevseqblending[0]) / 255.0;
			StudioSlerpBones(q1b, pos1b, q2, pos2, s);

			if (pseqdesc->numblends == 4)
			{
				panim += m_pStudioHeader->numbones;
				StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

				panim += m_pStudioHeader->numbones;
				StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

				s = (m_pCurrentEntity->latched.prevseqblending[0]) / 255.0;
				StudioSlerpBones(q3, pos3, q4, pos4, s);

				s = (m_pCurrentEntity->latched.prevseqblending[1]) / 255.0;
				StudioSlerpBones(q1b, pos1b, q3, pos3, s);
			}
		}

		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones(q, pos, q1b, pos1b, s);
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	if (m_pPlayerInfo && m_pPlayerInfo->gaitsequence != 0)
	{
		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim(m_pRenderModel, pseqdesc);
		StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe);

		for (i = 0; i < m_pStudioHeader->numbones; i++)
		{
			if (strcmp(pbones[i].name, "Bip01 Spine") == 0)
				break;

			memcpy(pos[i], pos2[i], sizeof( pos[i]));
			memcpy(q[i], q2[i], sizeof( q[i]));
		}
	}

	cl_entity_t *viewmodel = gEngfuncs.GetViewModel();
	qboolean bIsViewModel = false;

	if (viewmodel)
		bIsViewModel = viewmodel == m_pCurrentEntity;

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			if (cl_righthand && cl_righthand->value > 0 && bIsViewModel && gpEngineStudio->IsHardware())
			{
				bonematrix[1][0] = -bonematrix[1][0];
				bonematrix[1][1] = -bonematrix[1][1];
				bonematrix[1][2] = -bonematrix[1][2];
				bonematrix[1][3] = -bonematrix[1][3];
			}

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

void CStudioModelRenderer::StudioSaveBones(void)
{
	int i;

	mstudiobone_t *pbones;
	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	m_nCachedBones = m_pStudioHeader->numbones;

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		strcpy(m_nCachedBoneNames[i], pbones[i].name);
		MatrixCopy((*m_pbonetransform)[i], m_rgCachedBoneTransform[i]);
		MatrixCopy((*m_plighttransform)[i], m_rgCachedLightTransform[i]);
	}

	if(gRefExports.R_GetDrawPass() != r_draw_normal)
		return;

	SaveEntityBones();
}

void CStudioModelRenderer::StudioMergeBones(model_t *mod)
{
	int i, j;
	double f;
	int do_hunt = true;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	float bonematrix[3][4];
	static vec4_t q[MAXSTUDIOBONES];

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	f = StudioEstimateFrame(pseqdesc);

	if (m_pCurrentEntity->latched.prevframe > f)
	{
	}

	panim = StudioGetAnim(mod, pseqdesc);
	StudioCalcRotations(pos, q, pseqdesc, panim, f);

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		for (j = 0; j < m_nCachedBones; j++)
		{
			if (stricmp(pbones[i].name, m_nCachedBoneNames[j]) == 0)
			{
				MatrixCopy(m_rgCachedBoneTransform[j], (*m_pbonetransform)[i]);
				MatrixCopy(m_rgCachedLightTransform[j], (*m_plighttransform)[i]);
				break;
			}
		}

		if (j >= m_nCachedBones)
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
}

int CStudioModelRenderer::StudioDrawModel(int flags)
{
	g_bRenderPlayerWeapon = 0;

	m_pCurrentEntity = gpEngineStudio->GetCurrentEntity();

	gpEngineStudio->GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	gpEngineStudio->GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	gpEngineStudio->GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	if (m_pCurrentEntity->curstate.renderfx == kRenderFxDeadPlayer)
	{
		entity_state_t deadplayer;

		int result;
		int save_interp;

		if (m_pCurrentEntity->curstate.renderamt <= 0 || m_pCurrentEntity->curstate.renderamt > gEngfuncs.GetMaxClients())
			return 0;

		deadplayer = *(gpEngineStudio->GetPlayerState(m_pCurrentEntity->curstate.renderamt - 1));

		deadplayer.number = m_pCurrentEntity->curstate.renderamt;
		deadplayer.weaponmodel = 0;
		deadplayer.gaitsequence = 0;

		deadplayer.movetype = MOVETYPE_NONE;
		VectorCopy(m_pCurrentEntity->curstate.angles, deadplayer.angles);
		VectorCopy(m_pCurrentEntity->curstate.origin, deadplayer.origin);

		save_interp = m_fDoInterp;
		m_fDoInterp = 0;

		result = StudioDrawPlayer(flags, &deadplayer);

		m_fDoInterp = save_interp;
		return result;
	}

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(m_pRenderModel);

	gpEngineStudio->StudioSetHeader(m_pStudioHeader);
	gpEngineStudio->SetRenderModel(m_pRenderModel);

	StudioSetUpTransform(0);

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

	if (m_pCurrentEntity->curstate.movetype == MOVETYPE_FOLLOW)
		StudioMergeBones(m_pRenderModel);
	else
		StudioSetupBones();

	StudioSaveBones();

	if (flags & STUDIO_EVENTS)
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

		m_nTopColor = m_pCurrentEntity->curstate.colormap & 0xFF;
		m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;

		gpEngineStudio->StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		StudioRenderModel(dir);
	}

	return 1;
}

void CStudioModelRenderer::StudioEstimateGait(entity_state_t *pplayer)
{
	float dt;
	vec3_t est_velocity;

	dt = (m_clTime - m_clOldTime);

	if (dt < 0)
		dt = 0;
	else if (dt > 1.0)
		dt = 1;

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

void CStudioModelRenderer::StudioProcessGait(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;
	float dt;
	int iBlend;
	float flYaw;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend(pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH]);

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[0] = iBlend;
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	dt = (m_clTime - m_clOldTime);

	if (dt < 0)
		dt = 0;
	else if (dt > 1.0)
		dt = 1;

	StudioEstimateGait(pplayer);

	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	flYaw = flYaw - (int)(flYaw / 360) * 360;

	if (flYaw < -180)
		flYaw = flYaw + 360;

	if (flYaw > 180)
		flYaw = flYaw - 360;

	if (flYaw > 120)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if (flYaw < -120)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	m_pCurrentEntity->curstate.controller[0] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[1] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[2] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[3] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
	m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
	m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
	m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

	if (m_pCurrentEntity->angles[YAW] < -0)
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;

	if (pseqdesc->linearmovement[0] > 0)
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt;

	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_pPlayerInfo->gaitframe < 0)
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

int CStudioModelRenderer::StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
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
		m_pPlayerInfo->gaitsequence = 0;

		StudioSetUpTransform(0);
	}

	if (flags & STUDIO_RENDER)
	{
		if (!gpEngineStudio->StudioCheckBBox())
			return 0;

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

	if (flags & STUDIO_EVENTS)
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
		if (m_pCvarHiModels->value && m_pRenderModel != m_pCurrentEntity->model)
			m_pCurrentEntity->curstate.body = 255;

		if (!(m_pCvarDeveloper->value == 0 && gEngfuncs.GetMaxClients() == 1) && (m_pRenderModel == m_pCurrentEntity->model))
			m_pCurrentEntity->curstate.body = 1;

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
			cl_entity_t saveent = *m_pCurrentEntity;
			model_t *pweaponmodel = gpEngineStudio->GetModelByIndex(pplayer->weaponmodel);

			m_pStudioHeader = (studiohdr_t *)gpEngineStudio->Mod_Extradata(pweaponmodel);
			gpEngineStudio->StudioSetHeader(m_pStudioHeader);

			StudioMergeBones(pweaponmodel);

			gpEngineStudio->StudioSetupLighting(&lighting);

			StudioRenderModel(dir);

			StudioCalcAttachments();

			*m_pCurrentEntity = saveent;
		}
	}

	return 1;
}

void CStudioModelRenderer::StudioCalcAttachments(void)
{
	int i;
	mstudioattachment_t *pattachment;

	if (m_pStudioHeader->numattachments > 4)
		gEngfuncs.Con_DPrintf("Too many attachments on %s\n", m_pCurrentEntity->model->name);

	pattachment = (mstudioattachment_t *)((byte *)m_pStudioHeader + m_pStudioHeader->attachmentindex);

	for (i = 0; i < m_pStudioHeader->numattachments; i++)
		VectorTransform(pattachment[i].org, (*m_plighttransform)[pattachment[i].bone], m_pCurrentEntity->attachment[i]);

	if(gRefExports.R_GetDrawPass() != r_draw_normal)
		return;

	CParticleSystem *t;
	CPartSystemAttachment *tb;
	int size;

	size = g_partsystems.size();
	for(i = 0; i < size; ++i)
	{
		t = g_partsystems[i];
		if(t->IsBindAttachment())
		{
			tb = (CPartSystemAttachment *)t;
			if(tb->m_entity == *CurrentEntity && tb->GetDead() < 2)
			{
				tb->m_attachmentsaved = true;
				memcpy(tb->m_attachment, (*CurrentEntity)->attachment, sizeof(vec3_t)*4);
			}
		}
	}
}

void CStudioModelRenderer::StudioRenderModel(float *lightdir)
{
	if(CL_IsViewModel(m_pCurrentEntity) && lightdir)
	{
		gRefExports.R_PushRefDef();
		VectorMA(refdef->vieworg, 200, lightdir, refdef->vieworg);
		gRefExports.R_UpdateRefDef();
		gpEngineStudio->SetChromeOrigin();
		gRefExports.R_PopRefDef();
	}
	else
	{
		gpEngineStudio->SetChromeOrigin();
	}

	int iSaveRenderMode =  m_pCurrentEntity->curstate.rendermode;
	int iSaveRenderFx = m_pCurrentEntity->curstate.renderfx;
	int iSaveRenderAmt = m_pCurrentEntity->curstate.renderamt;

	if (iSaveRenderFx == kRenderFxCloak && iSaveRenderAmt == 0)
	{
		return;
	}

	if (iSaveRenderFx == kRenderFxGlowShell || iSaveRenderFx == kRenderFxCloak)
	{
		m_pCurrentEntity->curstate.renderfx = kRenderFxNone;
	}

	if(iSaveRenderFx == kRenderFxCloak)
	{
		gRefExports.R_RenderCloakTexture();
	}

	gpEngineStudio->SetForceFaceFlags(0);
	StudioRenderFinal();

	if (iSaveRenderFx == kRenderFxGlowShell)
	{
		m_pCurrentEntity->curstate.renderfx = kRenderFxGlowShell;

		gEngfuncs.pTriAPI->SpriteTexture(m_pChromeSprite, 0);

		gpEngineStudio->SetForceFaceFlags(STUDIO_NF_CHROME);	
		StudioRenderFinal();
	}
	
	if ( (m_pCurrentEntity->curstate.effects & EF_AFTERBURN) && !g_bRenderPlayerWeapon )
	{
		m_pCurrentEntity->curstate.rendermode = kRenderTransAdd;
		m_pCurrentEntity->curstate.renderfx = kRenderFxFireLayer;		
		m_pCurrentEntity->curstate.renderamt = 192;

		gRefExports.RefAPI.GL_Bind(m_texFireLayer);

		gpEngineStudio->SetForceFaceFlags(0);
		StudioRenderFinal();
	}

	//player's weapon model is critboosted
	if ( (m_pCurrentEntity->curstate.effects & EF_CRITBOOST) && g_bRenderPlayerWeapon )
	{
		m_pCurrentEntity->curstate.rendermode = kRenderTransAdd;
		m_pCurrentEntity->curstate.renderfx = kRenderFxFireLayer;		
		m_pCurrentEntity->curstate.renderamt = 255;

		int iTeam = m_pCurrentEntity->curstate.skin % 2;

		gRefExports.RefAPI.GL_Bind(m_texCritLayer[iTeam]);

		gpEngineStudio->SetForceFaceFlags( STUDIO_NF_FULLBRIGHT );
		StudioRenderFinal();
	}

	//my viewmodel is critboosted
	if ( (m_pCurrentEntity->curstate.effects & EF_CRITBOOST) && CL_IsViewModel(m_pCurrentEntity) )
	{
		m_pCurrentEntity->curstate.rendermode = kRenderTransAdd;
		m_pCurrentEntity->curstate.renderfx = kRenderFxFireLayer;		
		m_pCurrentEntity->curstate.renderamt = 255;

		int iTeam = m_pCurrentEntity->curstate.skin % 2;

		gRefExports.RefAPI.GL_Bind(m_texCritLayer[iTeam]);

		gpEngineStudio->SetForceFaceFlags( STUDIO_NF_FULLBRIGHT );
		StudioRenderFinal();
	}

	if ( (m_pCurrentEntity->curstate.effects & EF_INVULNERABLE) && (CL_IsViewModel(m_pCurrentEntity) || g_bRenderPlayerWeapon) )
	{
		m_pCurrentEntity->curstate.rendermode = kRenderNormal;
		m_pCurrentEntity->curstate.renderfx = kRenderFxInvulnLayer;		
		m_pCurrentEntity->curstate.renderamt = 255;

		int iTeam = m_pCurrentEntity->curstate.skin % 2;

		gRefExports.RefAPI.GL_SelectTexture(GL_TEXTURE0);
		gRefExports.RefAPI.GL_Bind(m_texInvulnLayer[iTeam]);

		gpEngineStudio->SetForceFaceFlags(0);
		StudioRenderFinal();
	}

	if (iSaveRenderFx == kRenderFxCloak)
	{
		m_pCurrentEntity->curstate.rendermode = kRenderNormal;
		m_pCurrentEntity->curstate.renderfx = kRenderFxCloak;
		m_pCurrentEntity->curstate.renderamt = iSaveRenderAmt;

		gpEngineStudio->SetForceFaceFlags(0);
		StudioRenderFinal();
	}
	m_pCurrentEntity->curstate.rendermode = iSaveRenderMode;
	m_pCurrentEntity->curstate.renderfx = iSaveRenderFx;
	m_pCurrentEntity->curstate.renderamt = iSaveRenderAmt;
}

void CStudioModelRenderer::StudioRenderFinal_Software(void)
{
	int i;

	gpEngineStudio->SetupRenderer(0);

	if (m_pCvarDrawEntities->value == 2)
	{
		gpEngineStudio->StudioDrawBones();
	}
	else if (m_pCvarDrawEntities->value == 3)
	{
		gpEngineStudio->StudioDrawHulls();
	}
	else
	{
		for (i = 0; i < m_pStudioHeader->numbodyparts; i++)
		{
			gpEngineStudio->StudioSetupModel(i, (void **)&m_pBodyPart, (void **)&m_pSubModel);
			if(StudioDrawBody())
				gpEngineStudio->StudioDrawPoints();
		}
	}

	if (m_pCvarDrawEntities->value == 4)
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gpEngineStudio->StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	}

	if (m_pCvarDrawEntities->value == 5)
		gpEngineStudio->StudioDrawAbsBBox();

	gpEngineStudio->RestoreRenderer();
}

void CStudioModelRenderer::StudioRenderFinal_Hardware(void)
{
	int i;
	int rendermode;

	rendermode = (gpEngineStudio->GetForceFaceFlags() & (STUDIO_NF_CHROME | STUDIO_NF_ALPHA | STUDIO_NF_ADDITIVE | STUDIO_NF_MASKED)) ? kRenderTransAdd : m_pCurrentEntity->curstate.rendermode;
	gpEngineStudio->SetupRenderer(rendermode);

	if (m_pCvarDrawEntities->value == 2)
	{
		gpEngineStudio->StudioDrawBones();
	}
	else if (m_pCvarDrawEntities->value == 3)
	{
		gpEngineStudio->StudioDrawHulls();
	}
	else
	{
		for (i = 0; i < m_pStudioHeader->numbodyparts; i++)
		{
			gpEngineStudio->StudioSetupModel(i, (void **)&m_pBodyPart, (void **)&m_pSubModel);
			
			if (m_fDoInterp)
				m_pCurrentEntity->trivial_accept = 0;

			if(StudioDrawBody())
			{
				gpEngineStudio->GL_SetRenderMode(rendermode);
				gpEngineStudio->StudioSetRenderamt(m_pCurrentEntity->curstate.renderamt);
				gpEngineStudio->StudioDrawPoints();
			}
		}
	}

	if (m_pCvarDrawEntities->value == 4)
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gpEngineStudio->StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	}

	gpEngineStudio->RestoreRenderer();
}

void CStudioModelRenderer::StudioRenderFinal(void)
{
	if (gpEngineStudio->IsHardware())
		StudioRenderFinal_Hardware();
	else
		StudioRenderFinal_Software();
}

void R_StudioInit(void)
{
	g_StudioRenderer.Init();
}

int R_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
#define DRAWPLAYER_FL_DISGUISEMASK (1<<0)
	int iResult;
	int iDrawFlags;
	float flDistance;
	vec3_t vecDistance;
	cl_entity_t *pEntity;
	float flSavePitch;
	char szSaveModel[32];
	player_info_t *pPlayerInfo;

	g_fLOD = 1;
	g_iLODLevel = 0;

	iDrawFlags = 0;

	pEntity = (*CurrentEntity);
	VectorSubtract(pEntity->origin, refdef->vieworg, vecDistance);
	flDistance = VectorLength(vecDistance);
	if(flDistance > g_lod2->value)
	{
		g_iLODLevel = 2;
	}
	else if(flDistance > g_lod1->value)
	{
		g_iLODLevel = 1;
	}
	else
	{
		g_iLODLevel = 0;
	}

	pPlayerInfo = NULL;

	if(pEntity == gEngfuncs.GetLocalPlayer())//fix angle
	{
		flSavePitch = pEntity->angles[0];
		pEntity->angles[0] *= -1;
	}
	if(pEntity->index >= 1 && pEntity->index <= gEngfuncs.GetMaxClients() )//player & rendering
	{
		int iShowTeam = pEntity->curstate.team;
		int iRealTeam = g_PlayerInfo[pEntity->index].iTeam;
		int iShowClass = pEntity->curstate.playerclass;
		int iDisgTarget = pEntity->curstate.endpos[0];
		if(iShowClass >= CLASS_SCOUT && iShowClass <= CLASS_SPY && (iShowTeam == 1 || iShowTeam == 2) && iDisgTarget)
		{
			if(iRealTeam != g_iTeam && iRealTeam != iShowTeam)//disguised enemy
			{
				pPlayerInfo = gpEngineStudio->PlayerInfo(pEntity->index-1);
				strcpy(szSaveModel, pPlayerInfo->model);
				strcpy(pPlayerInfo->model, g_szPlayerModel[iShowClass-1]);
				/*int color = 0;
				if (iShowTeam == 2)
				{
					color = 140;
				}
				else if (iShowTeam == 1)
				{
					color = 1;
				}
				iSaveColor = pPlayerInfo->topcolor;
				pPlayerInfo->topcolor = color;
				pPlayerInfo->bottomcolor = color;*/
			}
			else//disguised friend
			{
				iDrawFlags |= DRAWPLAYER_FL_DISGUISEMASK;
			}
		}
	}

	iResult = g_StudioRenderer.StudioDrawPlayer(flags, pplayer);
	
	if(iDrawFlags & DRAWPLAYER_FL_DISGUISEMASK)
	{
		int iSaveSkin = pEntity->curstate.skin;
		int iSaveSequence = pEntity->curstate.sequence;

		pEntity->curstate.sequence = 0;
		pEntity->curstate.skin = pEntity->curstate.playerclass-1;

		g_bRenderPlayerWeapon = 0;
		g_StudioRenderer.StudioRenderAttach(flags, g_mdlSpyMask);

		pEntity->curstate.sequence = iSaveSequence;
		pEntity->curstate.skin = iSaveSkin;
	}

	if(pEntity == gEngfuncs.GetLocalPlayer())
	{
		pEntity->angles[0] = flSavePitch;
	}
	if(pPlayerInfo)
	{
		strcpy(pPlayerInfo->model, szSaveModel);
		//pPlayerInfo->topcolor = iSaveColor;
		//pPlayerInfo->bottomcolor = iSaveColor;
	}

	g_fLOD = 0;
	return iResult;
}

void SpyWatch_Draw(int flags);
void R_UpdateViewModel(void);

int R_StudioDrawModel(int flags)
{
#define DRAWMODEL_FL_SENTRYCLIP (1<<0)
#define DRAWMODEL_FL_SAPPERATTACH (1<<1)

	cl_entity_t *pEntity = (*CurrentEntity);
	float flDistance;
	vec3_t vecDistance;
	int iResult;
	int iDrawFlags;
	int iSaveSkin;

	g_fLOD = 0;
	iDrawFlags = 0;

	if(*CurrentEntity == cl_viewent)
	{
		R_UpdateViewModel();
	}

	if(pEntity->model && (flags & STUDIO_RENDER) && *CurrentEntity != cl_viewent)
	{
		studiohdr_t *header = (studiohdr_t *)gpEngineStudio->Mod_Extradata(pEntity->model);
		if(header)
		{
			mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((char *)header + header->bodypartindex);
			
			if(pbodypart->nummodels > 0 && pbodypart->nummodels % 3 == 0)
			{
				g_fLOD = 1;
				VectorSubtract(pEntity->origin, refdef->vieworg, vecDistance);
				flDistance = VectorLength(vecDistance);
				if(flDistance > g_lod2->value)
				{
					g_iLODLevel = 2;
				}
				else if(flDistance > g_lod1->value)
				{
					g_iLODLevel = 1;
				}
				else
				{
					g_iLODLevel = 0;
				}
			}
		}

		if(pEntity->curstate.playerclass == CLASS_BUILDABLE)
		{
			if(g_pTraceEntity == pEntity && g_Player.m_pActiveItem && g_Player.m_pActiveItem->m_iId == WEAPON_SAPPER && !pEntity->curstate.iuser4)//buildable with sapper can't be sapped again
			{
				iDrawFlags |= DRAWMODEL_FL_SAPPERATTACH;
			}
			if(!strncmp(&pEntity->model->name[15], "w_sentry_lv1", 12))
			{
				iDrawFlags |= DRAWMODEL_FL_SENTRYCLIP;
				float flClipZ = pEntity->curstate.origin[2] + pEntity->curstate.mins[2];
				double clipPlane[] = {0, 0, 1, -flClipZ};
				qglEnable(GL_CLIP_PLANE1);
				qglClipPlane(GL_CLIP_PLANE1, clipPlane);			
			}
		}
	}

	iResult = g_StudioRenderer.StudioDrawModel(flags);

	if(iDrawFlags & DRAWMODEL_FL_SAPPERATTACH)
	{
		int iSapperSequence = -1;
		switch(pEntity->curstate.iuser3)
		{
		case BUILDABLE_SENTRY:
			iSapperSequence = pEntity->curstate.iuser2-1;
			break;
		case BUILDABLE_DISPENSER:
			iSapperSequence = 3;
			break;
		case BUILDABLE_ENTRANCE:
		case BUILDABLE_EXIT:
			iSapperSequence = 4;
			break;
		default:
			break;
		}
		int iSaveSequence = pEntity->curstate.sequence;
		iSaveSkin = pEntity->curstate.skin;
		pEntity->curstate.sequence = iSapperSequence;
		if(iSapperSequence != -1)
		{
			pEntity->curstate.skin = 1;
			g_bRenderPlayerWeapon = 0;
			g_StudioRenderer.StudioRenderAttach(flags, g_mdlSapper);
		}
		pEntity->curstate.skin = iSaveSkin;
		pEntity->curstate.sequence = iSaveSequence;
	}

	if(iDrawFlags & DRAWMODEL_FL_SENTRYCLIP)
	{
		qglDisable(GL_CLIP_PLANE1);
	}

	if(*CurrentEntity == cl_viewent)
	{
		SpyWatch_Draw(flags);
	}	

	g_fLOD = 0;
	return iResult;
}

void CStudioModelRenderer::SaveEntityBones(void)
{
	DWORD key = (DWORD)m_pCurrentEntity;

	mstudiobone_t *pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	m_EntityBones[key].m_cachedbones = m_pStudioHeader->numbones;

	for (int i = 0; i < m_pStudioHeader->numbones; i++)
		strcpy(m_EntityBones[key].m_bonename[i], pbones[i].name);
	memcpy(m_EntityBones[key].m_bonetransform, m_pbonetransform, m_pStudioHeader->numbones*sizeof(matrix3x4));
	memcpy(m_EntityBones[key].m_lighttransform, m_plighttransform, m_pStudioHeader->numbones*sizeof(matrix3x4));

	m_EntityBones[key].m_bonesaved = true;
}

void CStudioModelRenderer::RestoreEntityBones(entity_bones_t *bones)
{
	memcpy(m_pbonetransform, bones->m_bonetransform, m_pStudioHeader->numbones*sizeof(matrix3x4));
	memcpy(m_plighttransform, bones->m_lighttransform, m_pStudioHeader->numbones*sizeof(matrix3x4));

	m_nCachedBones = bones->m_cachedbones;

	for (int i = 0; i < m_nCachedBones; i++)
		strcpy( m_nCachedBoneNames[i], bones->m_bonename[i] );
	memcpy(m_rgCachedBoneTransform, bones->m_bonetransform, m_nCachedBones * sizeof(matrix3x4));
	memcpy(m_rgCachedLightTransform, bones->m_lighttransform, m_nCachedBones * sizeof(matrix3x4));
}

entity_bones_t *CStudioModelRenderer::GetEntityBones(void)
{
	DWORD key = (DWORD)m_pCurrentEntity;
	entity_bones_map::iterator it = m_EntityBones.find(key);
	if(it != m_EntityBones.end())
	{
		return &it->second;
	}
	return NULL;
}

void CStudioModelRenderer::ResetEntityBones(void)
{
	entity_bones_map::iterator it = m_EntityBones.begin();
	while(it != m_EntityBones.end())
	{
		it->second.m_bonesaved = false;
		it ++;
	}
}

void CStudioModelRenderer::StudioSpecialLight(alight_t *plight)
{
	if(m_pCurrentEntity->player)
	{
		if((m_pCurrentEntity->curstate.effects & EF_INVULNERABLE) && !g_bRenderPlayerWeapon)
		{
			plight->ambientlight = 128;
			plight->shadelight = 128;
		}
		if((m_pCurrentEntity->curstate.effects & EF_CRITBOOST) && g_bRenderPlayerWeapon)
		{
			plight->ambientlight = 128;
			plight->shadelight = 128;
		}
	}
	//my viewmodel is critboosted, but don't light up here
	/*if((m_pCurrentEntity->curstate.effects & EF_CRITBOOST) && CL_IsViewModel(m_pCurrentEntity) )
	{
		plight->ambientlight = 128;
		plight->shadelight = 128;
	}*/
	if( m_pCurrentEntity->curstate.entityType == ET_HUDENTITY )
	{
		plight->ambientlight = m_pCurrentEntity->curstate.iuser1;
		plight->shadelight = m_pCurrentEntity->curstate.iuser2;
		plight->color[0] = 1.0f;
		plight->color[1] = 1.0f;
		plight->color[2] = 1.0f;

		plight->plightvec[0] = 0;
		plight->plightvec[1] = 1;
		plight->plightvec[2] = -1;
	}
}

int CStudioModelRenderer::StudioDrawBody(void)
{
	if(CL_IsViewModel(m_pCurrentEntity))
	{
		//that is a critboost
		if(m_pCurrentEntity->curstate.renderfx == kRenderFxFireLayer && m_pCurrentEntity->curstate.renderamt == 255)
		{			
			if(!stricmp((*m_pBodyPart)->name,"arms"))
			{
				return false;
			}
		}
	}
	return true;
}