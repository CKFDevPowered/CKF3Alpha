#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "event_api.h"

DECLARE_MESSAGE(m_ThirdPerson, ThirdPerson)

int CHudThirdPerson::Init(void)
{
	m_iFlags = 0;

	HOOK_MESSAGE(ThirdPerson);

	return 1;
}

int CHudThirdPerson::VidInit(void)
{
	m_bToggle = false;

	return 1;
}

void CHudThirdPerson::CalcRefdef(struct ref_params_s *pparams)
{
	if (!m_bToggle)
		return;

	vec3_t start, end;
	vec3_t forward, right, up;
	VectorCopy(pparams->vieworg, start);
	VectorCopy(start, end);
	AngleVectors(pparams->viewangles, forward, right, up);
	VectorScale(forward, m_flForwardOfs, forward);
	VectorScale(right, m_flRightOfs, right);
	VectorScale(up, m_flUpOfs, up);
	VectorAdd(end, forward, end);
	VectorAdd(end, right, end);
	VectorAdd(end, up, end);

	if (m_bLater)
	{
		pmtrace_t tr;
		gEngfuncs.pEventAPI->EV_PlayerTrace(start, end, PM_NORMAL, -1, &tr);

		if (tr.fraction != 1.0)
			VectorCopy(tr.endpos, end);
	}

	VectorCopy(end, pparams->vieworg);
}

int CHudThirdPerson::MsgFunc_ThirdPerson(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_bToggle = READ_BYTE();

	if (m_bToggle)
	{
		m_bLater = READ_BYTE();
		m_flForwardOfs = READ_COORD();
		m_flRightOfs = READ_COORD();
		m_flUpOfs = READ_COORD();
	}

	return 1;
}