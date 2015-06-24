#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"

int CL_ButtonBits(int);
void CL_ResetButtonBits(int bits);

int CHud::UpdateClientData(client_data_t *cdata, float time)
{
	memcpy(m_vecOrigin, cdata->origin, sizeof(vec3_t));
	memcpy(m_vecAngles, cdata->viewangles, sizeof(vec3_t));

	m_iKeyBits = CL_ButtonBits(0);
	m_iWeaponBits = cdata->iWeaponBits;

	Think();

	cdata->fov = m_iFOV;

	CL_ResetButtonBits(m_iKeyBits);
	return 1;
}

void CHud::CalcRefdef(struct ref_params_s *pparams)
{
	m_ThirdPerson.CalcRefdef(pparams);
	m_ViewModel.CalcRefdef(pparams);
}