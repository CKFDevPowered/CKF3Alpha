#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include <pmtrace.h>
#include <event_api.h>

char *sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl",
	"models/player/gign/gign.mdl",
	"models/player/vip/vip.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/sas/sas.mdl",
	"models/player/terror/terror.mdl",
	"models/player/urban/urban.mdl",
	"models/player/spetsnaz/spetsnaz.mdl",
	"models/player/militia/militia.mdl"
};

DECLARE_MESSAGE(m_Corpse, ClCorpse)

int CHudCorpse::Init(void)
{
	HOOK_MESSAGE(ClCorpse);
	return 1;
}

int CHudCorpse::VidInit(void)
{
	return 1;
}

bool BIsValidCTModelIndex(int i)
{
	if (i == 7 || i == 2 || i == 10 || i == 4 || i == 9)
		return true;

	return false;
}

bool BIsValidTModelIndex(int i)
{
	if (i == 1 || i == 5 || i == 8 || i == 6 || i == 11)
		return true;

	return false;
}

void RemoveBody(TEMPENTITY *te, float frametime, float current_time)
{
	if (current_time >= te->entity.curstate.fuser2)
		te->entity.origin[2] -= frametime * 5;
}

void HitBody(TEMPENTITY *ent, pmtrace_s *ptr)
{
	if (ptr->plane.normal[2] > 0)
		ent->flags |= 0x200000;
}

void CreateCorpse(Vector vOrigin, Vector vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody)
{
	int iModelIndex;
	TEMPENTITY *pBody;

	iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex(pModel);
	pBody = gEngfuncs.pEfxAPI->R_TempModel(vOrigin, Vector(0, 0, 0), vAngles, 100.0, iModelIndex, 0);

	if (pBody)
	{
		pBody->flags |= FTENT_COLLIDEWORLD | FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_CLIENTCUSTOM | 0x100000;
		pBody->frameMax = 255;
		pBody->entity.curstate.framerate = 1.0;
		pBody->entity.curstate.animtime = flAnimTime;
		pBody->entity.curstate.frame = 0;
		pBody->entity.curstate.sequence = iSequence;
		pBody->entity.curstate.body = iBody;
		pBody->entity.baseline.renderamt = 255;
		pBody->entity.curstate.renderamt = 255;

		pBody->entity.curstate.fuser1 = gHUD.m_flTime + 1;
		pBody->entity.curstate.fuser2 = gHUD.m_flTime + cl_corpsestay->value;

		pBody->hitcallback = HitBody;
		pBody->bounceFactor = 0;

		if (cl_corpsefade->value)
		{
			pBody->die = gEngfuncs.GetClientTime() + cl_corpsestay->value;
		}
		else
		{
			pBody->callback = RemoveBody;
			pBody->die = gEngfuncs.GetClientTime() + cl_corpsestay->value + 9.0;
		}

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (!g_DeadPlayerModels[i])
			{
				g_DeadPlayerModels[i] = pBody;
				break;
			}
		}
	}
}

int CHudCorpse::MsgFunc_ClCorpse(const char *pszName, int iSize, void *pbuf)
{
	char szModel[64];
	Vector vOrigin;
	Vector vAngles;
	float flAnimTime;
	int iSequence;
	int iBody;
	int iTeam;
	int iIndex;
	char *pModel;
	cl_entity_t *pEntity;

	BEGIN_READ(pbuf, iSize);

	pModel = READ_STRING();
	vOrigin.x = READ_LONG() / 128.0;
	vOrigin.y = READ_LONG() / 128.0;
	vOrigin.z = READ_LONG() / 128.0;
	vAngles.x = READ_COORD();
	vAngles.y = READ_COORD();
	vAngles.z = READ_COORD();
	flAnimTime = gEngfuncs.GetClientTime() + READ_LONG() / 100.0;
	iSequence = READ_BYTE();
	iBody = READ_BYTE();
	iTeam = READ_BYTE();
	iIndex = READ_BYTE();

	if (cl_minmodel && cl_minmodel->value)
	{
		if (iTeam == TEAM_TERRORIST)
		{
			if (cl_min_t && BIsValidTModelIndex(cl_min_t->value))
				pModel = sPlayerModelFiles[(int)cl_min_t->value];
			else
				pModel = "models/player/leet/leet.mdl";
		}
		else if (iTeam == TEAM_CT)
		{
			if (g_PlayerExtraInfo[iIndex].vip)
				pModel = "models/player/vip/vip.mdl";
			else if (cl_min_ct && BIsValidCTModelIndex(cl_min_ct->value))
				pModel = sPlayerModelFiles[(int)cl_min_ct->value];
			else
				pModel = "models/player/gign/gign.mdl";
		}
		else
		{
			pModel = "models/player/player.mdl";
		}

		_snprintf(szModel, sizeof(szModel), "%s", pModel);
		szModel[sizeof(szModel) - 1] = 0;
	}
	else
	{
		_snprintf(szModel, sizeof(szModel), "models/player/%s/%s.mdl", pModel, pModel);
		szModel[sizeof(szModel) - 1] = 0;
	}

	pEntity = gEngfuncs.GetEntityByIndex(iIndex);

	if (pEntity)
	{
		vOrigin = pEntity->curstate.origin;
	}

	CreateCorpse(vOrigin, vAngles, szModel, flAnimTime, iSequence, iBody);
	return 1;
}