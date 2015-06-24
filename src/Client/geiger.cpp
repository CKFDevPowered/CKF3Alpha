#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "parsemsg.h"

DECLARE_MESSAGE(m_Geiger, Geiger)

int CHudGeiger::Init(void)
{
	HOOK_MESSAGE(Geiger);

	m_iGeigerRange = 0;
	m_iFlags = 0;

	gHUD.AddHudElem(this);

	srand((unsigned)time(NULL));
	return 1;
}

int CHudGeiger::VidInit(void)
{
	return 1;
}

int CHudGeiger::MsgFunc_Geiger(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iGeigerRange = READ_BYTE();
	m_iGeigerRange = m_iGeigerRange << 2;

	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudGeiger::Draw(float flTime)
{
	int pct;
	float flvol;
	int rg[3];
	int i;

	if (m_iGeigerRange < 1000 && m_iGeigerRange > 0)
	{
		if (m_iGeigerRange > 800)
		{
			pct = 0;
		}
		else if (m_iGeigerRange > 600)
		{
			pct = 2;
			flvol = 0.4;
			rg[0] = 1;
			rg[1] = 1;
			i = 2;
		}
		else if (m_iGeigerRange > 500)
		{
			pct = 4;
			flvol = 0.5;
			rg[0] = 1;
			rg[1] = 2;
			i = 2;
		}
		else if (m_iGeigerRange > 400)
		{
			pct = 8;
			flvol = 0.6;
			rg[0] = 1;
			rg[1] = 2;
			rg[2] = 3;
			i = 3;
		}
		else if (m_iGeigerRange > 300)
		{
			pct = 8;
			flvol = 0.7;
			rg[0] = 2;
			rg[1] = 3;
			rg[2] = 4;
			i = 3;
		}
		else if (m_iGeigerRange > 200)
		{
			pct = 28;
			flvol = 0.78;
			rg[0] = 2;
			rg[1] = 3;
			rg[2] = 4;
			i = 3;
		}
		else if (m_iGeigerRange > 150)
		{
			pct = 40;
			flvol = 0.80;
			rg[0] = 3;
			rg[1] = 4;
			rg[2] = 5;
			i = 3;
		}
		else if (m_iGeigerRange > 100)
		{
			pct = 60;
			flvol = 0.85;
			rg[0] = 3;
			rg[1] = 4;
			rg[2] = 5;
			i = 3;
		}
		else if (m_iGeigerRange > 75)
		{
			pct = 80;
			flvol = 0.9;
			rg[0] = 4;
			rg[1] = 5;
			rg[2] = 6;
			i = 3;
		}
		else if (m_iGeigerRange > 50)
		{
			pct = 90;
			flvol = 0.95;
			rg[0] = 5;
			rg[1] = 6;
			i = 2;
		}
		else
		{
			pct = 95;
			flvol = 1.0;
			rg[0] = 5;
			rg[1] = 6;
			i = 2;
		}

		flvol = (flvol * ((rand() & 127)) / 255) + 0.25;

		if ((rand() & 127) < pct || (rand() & 127) < pct)
		{
			char sz[256];

			int j = rand() & 1;

			if (i > 2)
				j += rand() & 1;

			sprintf(sz, "player/geiger%d.wav", j + 1);
			PlaySound(sz, flvol);
		}
	}

	return 1;
}
