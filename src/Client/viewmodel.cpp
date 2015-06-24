#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "event_api.h"
#include <cl_entity.h>

DECLARE_MESSAGE(m_ViewModel, ViewBody)
DECLARE_MESSAGE(m_ViewModel, ViewRender)

int CHudViewModel::Init(void)
{
	m_iFlags = 0;

	HOOK_MESSAGE(ViewBody);

	return 1;
}

int CHudViewModel::VidInit(void)
{
	m_body = 0;
	m_rendermode = kRenderNormal;
	m_renderamt = 0.0;
	m_rendercolor.r = 0;
	m_rendercolor.g = 0;
	m_rendercolor.b = 0;
	m_renderfx = kRenderFxNone;

	return 1;
}

void CHudViewModel::CalcRefdef(struct ref_params_s *pparams)
{
	cl_entity_t *viewmodel = gEngfuncs.GetViewModel();

	if (viewmodel)
	{
		viewmodel->curstate.body = m_body;
		viewmodel->curstate.rendermode = m_rendermode;
		viewmodel->curstate.renderamt = m_renderamt;
		viewmodel->curstate.rendercolor.r = m_rendercolor.r;
		viewmodel->curstate.rendercolor.g = m_rendercolor.g;
		viewmodel->curstate.rendercolor.b = m_rendercolor.b;
		viewmodel->curstate.renderfx = m_renderfx;
	}
}

int CHudViewModel::MsgFunc_ViewBody(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_body = READ_BYTE();

	return 1;
}

int CHudViewModel::MsgFunc_ViewRender(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_rendermode = READ_BYTE();
	m_renderamt = READ_BYTE();
	m_rendercolor.r = READ_BYTE();
	m_rendercolor.g = READ_BYTE();
	m_rendercolor.b = READ_BYTE();
	m_renderfx = READ_BYTE();

	return 1;
}