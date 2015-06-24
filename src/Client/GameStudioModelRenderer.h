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
	void CachePlayerBoneIndices(void);
	int GetPlayerBoneIndex(BoneIndex whichBone);
	bool GetPlayerBoneWorldPosition(BoneIndex whichBone, Vector *pos);

private:
	int m_nPlayerGaitSequences[MAX_CLIENTS];

private:
	bool m_bLocal;
	int m_boneIndexCache[BONE_MAX];
	bool m_isBoneCacheValid;
};

#endif