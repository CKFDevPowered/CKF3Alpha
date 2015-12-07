#pragma once

int R_StudioDrawModel(int flags);
int R_StudioDrawPlayer(int flags, entity_state_t *pplayer);

#include <map>

typedef struct
{
	float m_lighttransform[MAXSTUDIOBONES][3][4];
	float m_bonetransform[MAXSTUDIOBONES][3][4];
	char m_bonename[MAXSTUDIOBONES][32];
	int m_cachedbones;
	qboolean m_bonesaved;
}entity_bones_t;

typedef std::map<DWORD, entity_bones_t> entity_bones_map;

class CStudioModelRenderer
{
public:
	CStudioModelRenderer(void);
	virtual ~CStudioModelRenderer(void);

public:
	virtual void Init(void);
	virtual int StudioDrawModel(int flags);
	virtual int StudioDrawPlayer(int flags, struct entity_state_s *pplayer);

public:
	virtual mstudioanim_t *StudioGetAnim(model_t *mod, mstudioseqdesc_t *pseqdesc);
	virtual void StudioSetUpTransform(int trivial_accept);
	virtual void StudioSetupBones(void);
	virtual void StudioCalcAttachments(void);
	virtual void StudioSaveBones(void);
	virtual void StudioMergeBones(model_t *mod);
	virtual float StudioEstimateInterpolant(void);
	virtual float StudioEstimateFrame(mstudioseqdesc_t *pseqdesc);
	virtual void StudioFxTransform(cl_entity_t *ent, float transform[3][4]);
	virtual void StudioSlerpBones(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s);
	virtual void StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen);
	virtual void StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q);
	virtual void StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos);
	virtual void StudioCalcRotations(float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f);
	virtual void StudioRenderModel(float *lightdir);
	virtual void StudioRenderFinal(void);
	virtual void StudioRenderFinal_Software(void);
	virtual void StudioRenderFinal_Hardware(void);
	virtual void StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch);
	virtual void StudioEstimateGait(entity_state_t *pplayer);
	virtual void StudioProcessGait(entity_state_t *pplayer);

public:
	//ckf2 added
	void StudioRenderAttach(int flags, model_t *mod);
	void SaveEntityBones(void);
	void ResetEntityBones(void);
	entity_bones_t *GetEntityBones(void);
	void RestoreEntityBones(entity_bones_t *bones);
	void StudioSpecialLight(alight_t *plight);
	int StudioDrawBody(void);

public:
	double m_clTime;
	double m_clOldTime;
	int m_fDoInterp;
	int m_fGaitEstimation;
	int m_nFrameCount;
	cvar_t *m_pCvarHiModels;
	cvar_t *m_pCvarDeveloper;
	cvar_t *m_pCvarDrawEntities;
	cl_entity_t *m_pCurrentEntity;
	model_t *m_pRenderModel;
	player_info_t *m_pPlayerInfo;
	int m_nPlayerIndex;
	float m_flGaitMovement;
	studiohdr_t *m_pStudioHeader;
	mstudiobodyparts_t **m_pBodyPart;
	mstudiomodel_t **m_pSubModel;
	int m_nTopColor;
	int m_nBottomColor;
	model_t *m_pChromeSprite;
	int m_nCachedBones;
	char m_nCachedBoneNames[MAXSTUDIOBONES][32];
	float m_rgCachedBoneTransform[MAXSTUDIOBONES][3][4];
	float m_rgCachedLightTransform[MAXSTUDIOBONES][3][4];
	float m_fSoftwareXScale, m_fSoftwareYScale;
	float m_vUp[3];
	float m_vRight[3];
	float m_vNormal[3];
	float m_vRenderOrigin[3];
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;
	float (*m_protationmatrix)[3][4];
	float (*m_paliastransform)[3][4];
	float (*m_pbonetransform)[MAXSTUDIOBONES][3][4];
	float (*m_plighttransform)[MAXSTUDIOBONES][3][4];

	//CKF3 added
	int m_texFireLayer;
	int m_texCritLayer[2];
	int m_texInvulnLayer[2];
	entity_bones_map m_EntityBones;
};