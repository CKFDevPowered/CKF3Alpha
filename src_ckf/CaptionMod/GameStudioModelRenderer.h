#if !defined (GAMESTUDIOMODELRENDERER_H)
#define GAMESTUDIOMODELRENDERER_H
#if defined (_WIN32)
#pragma once
#endif

enum BoneIndex
{
	BONE_HEAD,
	BONE_PELVIS,
	BONE_SPINE1,
	BONE_SPINE2,
	BONE_SPINE3,
	BONE_MAX,
};

typedef struct
{
	vec3_t origin;
	vec3_t angles;

	vec3_t realangles;

	float animtime;
	float frame;
	int sequence;
	int gaitsequence;
	float framerate;

	int m_fSequenceLoops;
	int m_fSequenceFinished;

	byte controller[4];
	byte blending[2];

	latchedvars_t lv;
}client_anim_state_t;

class CGameStudioModelRenderer : public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer(void);

public:
	virtual void StudioSetupBones(void);
	virtual void StudioEstimateGait(entity_state_t *pplayer);
	virtual void StudioProcessGait(entity_state_t *pplayer);
	virtual int StudioDrawPlayer(int flags, entity_state_t *pplayer);
	virtual int _StudioDrawPlayer(int flags, entity_state_t *pplayer);
	virtual void StudioFxTransform(cl_entity_t *ent, float transform[3][4]);
	virtual void StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch);
	virtual void CalculateYawBlend(entity_state_t *pplayer);
	virtual void CalculatePitchBlend(entity_state_t *pplayer);

private:
	void SavePlayerState(entity_state_t *pplayer);
	void SetupClientAnimation(entity_state_t *pplayer);
	void RestorePlayerState(entity_state_t *pplayer);
	mstudioanim_t* LookupAnimation(mstudioseqdesc_t *pseqdesc, int index);

private:
	int m_nPlayerGaitSequences[MAX_CLIENTS];

private:
	bool m_bLocal;

public:
	//ckf3 added
	void StudioDrawPlayer_3DHUD(void);
	void PM_StudioSetupBones(int playerindex);
};

extern CGameStudioModelRenderer g_StudioRenderer;

void R_StudioInit(void);

#endif