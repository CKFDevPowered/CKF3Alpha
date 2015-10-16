#include "qgl.h"
#include "hud.h"

#include "cl_util.h"

#include "parsemsg.h"
#include "player.h"
#include "client.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <event_api.h>
#include <cdll_dll.h>

#include <ref_params.h>

#include <VGUI/VGUI.h>
#include <VGUI/ISurface.h>
#include <VGUI/Vector.h>
#include <KeyValues.h>

#include "hud_screenoverlay.h"

#include "CounterStrikeViewport.h"

qboolean EV_IsLocal(int idx);

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: AmmoStatus Panel
//-----------------------------------------------------------------------------

CTFHudScreenOverlay::CTFHudScreenOverlay() : CHudElement(), BaseClass( NULL, "HudScreenOverlay" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");

	SetHiddenBits( HIDEHUD_HEALTH );

	m_iInvulnMaterial[0] = surface()->CreateNewTextureID();
	m_iInvulnMaterial[1] = surface()->CreateNewTextureID();
	m_iSniperScopeMaterial = surface()->CreateNewTextureID();

	m_bInvuln = false;
	m_iInvulnTeam = 0;
	m_nTeam = 0;
}

void CTFHudScreenOverlay::Reset(void)
{
	m_bInvuln = false;
	m_iInvulnTeam = 0;
	m_nTeam = 0;
}

void CTFHudScreenOverlay::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	surface()->DrawSetTextureFile(m_iInvulnMaterial[0], "resource/tga/overlay_invuln_red", true, false);
	surface()->DrawSetTextureFile(m_iInvulnMaterial[1], "resource/tga/overlay_invuln_blue", true, false);
	surface()->DrawSetTextureFile(m_iSniperScopeMaterial, "resource/tga/scope_1440", true, false);
}

//-----------------------------------------------------------------------------
// Purpose: Get ammo info from the weapon and update the displays.
//-----------------------------------------------------------------------------
void CTFHudScreenOverlay::OnThink(void)
{
	if(m_nTeam != (*gCKFVars.g_iTeam))
	{
		m_nTeam = (*gCKFVars.g_iTeam);
	}
}

void CTFHudScreenOverlay::Init(void)
{
	SetVisible(true);
}

void CTFHudScreenOverlay::VidInit(void)
{
}

int CTFHudScreenOverlay::FireMessage(const char *pszName, int iSize, void *pbuf)
{ 
	if(!strcmp(pszName, "DrawFX"))
	{
		BEGIN_READ(pbuf, iSize);
		int type = READ_BYTE();
		if(type == FX_INVULNPLAYER)
		{
			int entindex = READ_SHORT();
			if(EV_IsLocal(entindex))
			{
				int iTeam = READ_BYTE();
				float flDuration = READ_COORD();
				m_iInvulnTeam = iTeam;
				m_bInvuln = (flDuration > 0) ? true : false;
			}
		}
	}
	return 0;
}

void CTFHudScreenOverlay::Paint(void)
{
	int w, h;
	GetSize(w, h);

	if( m_bInvuln )
	{
		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawSetTexture(m_iInvulnMaterial[(m_iInvulnTeam-1)%2]);

		surface()->DrawTexturedRect(0, 0, w, h);
	}

	//Scoping
	if(gHUD.m_iFOV <= 60)
	{
		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawSetTexture(m_iSniperScopeMaterial);

		surface()->DrawTexturedRect(w/2-h*8/9, 0, w/2, h/2);

		qglEnable(GL_TEXTURE_2D);
		qglEnable(GL_BLEND);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4f(1, 1, 1, 1);

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0);
		qglVertex3f(w/2+h*8/9,0,0);
		qglTexCoord2f(1,0);
		qglVertex3f(w/2,0,0);
		qglTexCoord2f(1,1);
		qglVertex3f(w/2,h/2,0);
		qglTexCoord2f(0,1);
		qglVertex3f(w/2+h*8/9,h/2,0);
		qglEnd();

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0);
		qglVertex3f(w/2+h*8/9,h,0);
		qglTexCoord2f(1,0);
		qglVertex3f(w/2,h,0);
		qglTexCoord2f(1,1);
		qglVertex3f(w/2,h/2,0);
		qglTexCoord2f(0,1);
		qglVertex3f(w/2+h*8/9,h/2,0);
		qglEnd();

		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0);
		qglVertex3f(w/2-h*8/9,h,0);
		qglTexCoord2f(1,0);
		qglVertex3f(w/2,h,0);
		qglTexCoord2f(1,1);
		qglVertex3f(w/2,h/2,0);
		qglTexCoord2f(0,1);
		qglVertex3f(w/2-h*8/9,h/2,0);
		qglEnd();
	}
}

bool CTFHudScreenOverlay::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;

	if(*gCKFVars.g_iHealth <= 0)
		return false;

	return true;
}

}