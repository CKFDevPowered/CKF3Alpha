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

#include "hud_playerstatus.h"

#include "CounterStrikeViewport.h"

static int g_iWeaponBody[] = {0, 9, 3, 6, 66, 57, 27, 3, 0, 48, 0};

static const char *g_sPlayerModel[] = {
	NULL,
	"models/player/ckf_scout/ckf_scout.mdl", 
	"models/player/ckf_heavy/ckf_heavy.mdl",
	"models/player/ckf_soldier/ckf_soldier.mdl", 
	"models/player/ckf_pyro/ckf_pyro.mdl",
	"models/player/ckf_sniper/ckf_sniper.mdl", 
	"models/player/ckf_medic/ckf_medic.mdl",
	"models/player/ckf_engineer/ckf_engineer.mdl",
	"models/player/ckf_demoman/ckf_demoman.mdl", 
	"models/player/ckf_spy/ckf_spy.mdl",
	NULL
};

static const char *g_sClassName[] = {
	"scout",
	"heavy",
	"soldier",
	"pyro",
	"sniper",
	"medic",
	"engineer",
	"demoman",	
	"spy",
	"random"
};

int GetBuffedMaxHealth( int iMaxHealth )
{
	float flBoostMax = iMaxHealth * 1.5f;

	int iRoundDown = floor( flBoostMax / 5 );
	iRoundDown = iRoundDown * 5;

	return iRoundDown;
}

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

TFPlayerModelPanel::TFPlayerModelPanel( vgui::Panel *parent, const char *name ) : ModelPanel( parent, name )
{
	memset(m_modPlayerModels, 0, sizeof(m_modPlayerModels));
	memset(m_CustomClassData, 0, sizeof(m_CustomClassData));
	LoadModel(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFPlayerModelPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	KeyValues *kvCustomClassData = inResourceData->FindKey("customclassdata");

	if(kvCustomClassData)
	{
		for(int i = 0; i < 9; ++i)
		{
			KeyValues *kvClassData = kvCustomClassData->FindKey(g_sClassName[i]);
			if(kvClassData)
			{
				sscanf(kvClassData->GetString("origin", "0 0 0"), "%f %f %f", &m_CustomClassData[i].origin[0], &m_CustomClassData[i].origin[1], &m_CustomClassData[i].origin[2]);
				sscanf(kvClassData->GetString("angles", "0 0 0"), "%f %f %f", &m_CustomClassData[i].angles[0], &m_CustomClassData[i].angles[1], &m_CustomClassData[i].angles[2]);
				m_CustomClassData[i].use = true;	
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFPlayerModelPanel::SetClass( int iTeam, int iClass )
{
	if ( iTeam >= TEAM_RED && iTeam <= TEAM_BLUE && iClass >= CLASS_SCOUT && iClass <= CLASS_SPY )
	{
		SetVisible(true);

		cl_entity_t *ent = GetEntity();

		if(m_modPlayerModels[iClass] == NULL)
		{
			SetModel(g_sPlayerModel[iClass]);
			LoadModel();
			m_modPlayerModels[iClass] = ent->model;
		}
		else
		{
			LoadModel(m_modPlayerModels[iClass]);
		}
		//as default weapon, even if we don't use this
		if(iClass == CLASS_HEAVY || iClass == CLASS_DEMOMAN)
			ent->curstate.weaponmodel = engine->pEventAPI->EV_FindModelIndex("models/CKF_III/wp_group_2bone.mdl");
		else
			ent->curstate.weaponmodel = engine->pEventAPI->EV_FindModelIndex("models/CKF_III/wp_group_rf.mdl");
		ent->curstate.scale = g_iWeaponBody[iClass];
		ent->curstate.sequence = (iClass == CLASS_MEDIC) ? 19 : 45;

		ent->curstate.skin = (iTeam == TEAM_RED) ? 0 : 1;
		ent->curstate.colormap = (iTeam == TEAM_RED) ? (1 | (1<<8)) : (140 | (140<<8));

		cl_entity_t *pLocalPlayer = engine->GetLocalPlayer();
		ent->index = pLocalPlayer->index;
		ent->player = 1;
		ent->curstate.gaitsequence = 1;
		ent->curstate.iuser1 = 128;
		ent->curstate.iuser2 = 128;

		if(m_CustomClassData[iClass-1].use)
		{
			VectorCopy(m_CustomClassData[iClass-1].origin, m_entity.origin);
			VectorCopy(m_CustomClassData[iClass-1].angles, m_entity.angles);
		}
		else
		{
			VectorCopy(m_origin, m_entity.origin);
			VectorCopy(m_angles, m_entity.angles);
		}
	}
	else
	{
		SetVisible(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

TFClassImage::TFClassImage( Panel *parent, const char *name ) : ImagePanel( parent, name )
{
	char imageName[64];

	for (int i = 0; i < 2; i++ )
	{
		for(int j = 0; j < 9; ++j)
		{
			Q_snprintf(imageName, sizeof(imageName), "resource/tga/class_%s_%s", g_sClassName[j], (!i) ? "red" : "blu");
			m_pImageClassImage[i][j] = scheme()->GetImage( imageName , true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFClassImage::SetClass( int iTeam, int iClass )
{
	if ( iTeam >= TEAM_RED && iTeam <= TEAM_BLUE && iClass >= CLASS_SCOUT && iClass <= CLASS_SPY )
	{
		SetVisible(true);
		SetImage( m_pImageClassImage[iTeam-1][iClass-1] );
	}
	else
	{
		SetVisible(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerClass::CTFHudPlayerClass( Panel *parent, const char *name ) : EditablePanel( parent, name )
{
	m_pClassImage = new TFClassImage( this, "PlayerStatusClassImage" );
	m_pSpyImage = new ImagePanel( this, "PlayerStatusSpyImage" );
	m_pSpyOutlineImage = new ImagePanel( this, "PlayerStatusSpyOutlineImage" );
	m_pClassImageBG = new ImagePanel( this, "PlayerStatusClassImageBG" );
	m_pPlayerModel = new TFPlayerModelPanel( this, "PlayerStatusPlayerModel" );

	m_nTeam = TEAM_UNASSIGNED;
	m_nClass = CLASS_UNASSIGNED;
	m_nDisguise = 0;
	m_nDisguiseTeam = TEAM_UNASSIGNED;
	m_nDisguiseClass = CLASS_UNASSIGNED;
	m_flNextThink = 0.0f;
	m_nShow3DHUD = 0;

	SetVisible(true);
}

void CTFHudPlayerClass::Reset()
{
	m_nTeam = TEAM_UNASSIGNED;
	m_nClass = CLASS_UNASSIGNED;
	m_nDisguise = 0;
	m_nDisguiseTeam = TEAM_UNASSIGNED;
	m_nDisguiseClass = CLASS_UNASSIGNED;
	m_flNextThink = 0.0f;
	m_nShow3DHUD = 0;

	m_flNextThink = engine->GetClientTime() + 0.05f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::ApplySchemeSettings( IScheme *pScheme )
{
	// load control settings...
	LoadControlSettings( "resource/UI/HudPlayerClass.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::OnThink()
{
	if ( m_flNextThink < engine->GetClientTime() )
	{
		bool bClassChange = false;
		bool bTeamChange = false;
		bool bDisguiseChange = false;
		bool bShow3dhudChange = false;

		// set our background colors
		if ( m_nTeam != (*gCKFVars.g_iTeam) )
		{
			bTeamChange = true;
			m_nTeam = (*gCKFVars.g_iTeam);
		}

		if ( m_nClass != (*gCKFVars.g_iClass) )
		{
			bClassChange = true;
			m_nClass = (*gCKFVars.g_iClass);
		}

		if((int)cl_3dhud->value != m_nShow3DHUD)
		{
			bShow3dhudChange = true;
			m_nShow3DHUD = (int)cl_3dhud->value;
		}

		if( (*gCKFVars.g_iClass) == CLASS_SPY)
		{
			if(m_nDisguiseClass != gCKFVars.g_Player->m_iDisguiseClass)
			{
				m_nDisguiseClass = gCKFVars.g_Player->m_iDisguiseClass;
				bDisguiseChange = true;
			}
			if(m_nDisguiseTeam != gCKFVars.g_Player->m_iDisguiseTeam)
			{
				m_nDisguiseTeam = gCKFVars.g_Player->m_iDisguiseTeam;
				bDisguiseChange = true;
			}
			if(m_nDisguise != gCKFVars.g_Player->m_iDisguise)
			{
				m_nDisguise = gCKFVars.g_Player->m_iDisguise;
				bDisguiseChange = true;
			}
		}

		// set our class image
		if ( bClassChange || bTeamChange || bDisguiseChange || bShow3dhudChange)
		{
			if ( m_nClass == CLASS_SPY && m_nDisguise )
			{
				m_nDisguiseTeam = gCKFVars.g_Player->m_iDisguiseTeam;
				m_nDisguiseClass = gCKFVars.g_Player->m_iDisguiseClass;
			}
			else
			{
				m_nDisguiseTeam = TEAM_UNASSIGNED;
				m_nDisguiseClass = CLASS_UNASSIGNED;
			}

			if ( m_pClassImage && m_pSpyImage )
			{
				if ( m_nDisguiseTeam != TEAM_UNASSIGNED || m_nDisguiseClass != CLASS_UNASSIGNED )
				{					
					m_pSpyImage->SetImage((m_nTeam == TEAM_RED) ? "resource/tga/class_spy_red" : "resource/tga/class_spy_blu");
					m_pSpyImage->SetVisible( true );
					if(m_nShow3DHUD)
					{
						m_pPlayerModel->SetClass( m_nDisguiseTeam, m_nDisguiseClass );
						m_pClassImage->SetVisible(false);
					}
					else
					{
						m_pPlayerModel->SetVisible(false);
						m_pClassImage->SetClass( m_nDisguiseTeam, m_nDisguiseClass );
					}
				}
				else
				{
					m_pSpyImage->SetVisible( false );
					if(m_nShow3DHUD)
					{
						m_pPlayerModel->SetClass( m_nTeam, m_nClass );
						m_pClassImage->SetVisible(false);
					}
					else
					{
						m_pPlayerModel->SetVisible(false);
						m_pClassImage->SetClass( m_nTeam, m_nClass );
					}
				}
				m_pClassImageBG->SetImage((m_nTeam == TEAM_RED) ? "resource/tga/mask_class_red" : "resource/tga/mask_class_blu");
			}
		}

		if(m_pPlayerModel->IsVisible())
		{
			cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
			cl_entity_t *pPlayerEntity = m_pPlayerModel->GetEntity();
			
			if(gCKFVars.g_Player->m_iDisguise)
			{
				pPlayerEntity->curstate.weaponmodel = gCKFVars.g_Player->m_iDisguiseWeapon;
				pPlayerEntity->curstate.scale = gCKFVars.g_Player->m_iDisguiseWeaponBody;
				pPlayerEntity->curstate.sequence = gCKFVars.g_Player->m_iDisguiseSequence;
			}
			else if(gCKFVars.g_Player->m_pActiveItem != NULL)
			{
				//we don't use gCKFVars.g_Player->pev.weaponmodel since there is a little lag while changing weaponmodel after disguised
				pPlayerEntity->curstate.weaponmodel = engine->GetLocalPlayer()->curstate.weaponmodel;//gCKFVars.g_Player->pev.weaponmodel;
				pPlayerEntity->curstate.sequence = gCKFVars.g_Player->pev.sequence;
				pPlayerEntity->curstate.scale = pPlayer->curstate.scale;
			}
			else
			{
				pPlayerEntity->curstate.weaponmodel = 0;
				pPlayerEntity->curstate.scale = 0;
				pPlayerEntity->curstate.sequence = 45;
			}
		}

		m_flNextThink = engine->GetClientTime() + 0.05f;
	}
}

int CTFHudPlayerClass::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	if(0 != strcmp(pszName, "DrawFX"))
		return 0;

	BEGIN_READ(pbuf, iSize);

	int type = READ_BYTE();

	if(type != FX_DISGUISEHINT)
		return 0;

	if ( m_pSpyImage && m_pSpyOutlineImage )
	{
		bool bFadeIn = READ_BYTE() ? true : false;

		if ( bFadeIn )
		{
			m_pSpyImage->SetAlpha( 0 );
		}
		else
		{
			m_pSpyImage->SetAlpha( 255 );
		}

		m_pSpyOutlineImage->SetAlpha( 0 );

		m_pSpyImage->SetVisible( true );
		m_pSpyOutlineImage->SetVisible( true );

		GetAnimationController()->StartAnimationSequence( this, bFadeIn ? "HudSpyDisguiseFadeIn" : "HudSpyDisguiseFadeOut" );
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
TFHealthPanel::TFHealthPanel( Panel *parent, const char *name ) : ImagePanel( parent, name )
{
	m_flHealth = 1.0f;

	m_iMaterialIndex = surface()->DrawGetTextureId( "resource/tga/health_color" );
	if ( m_iMaterialIndex == -1 ) // we didn't find it, so create a new one
	{
		m_iMaterialIndex = surface()->CreateNewTextureID();	
		surface()->DrawSetTextureFile( m_iMaterialIndex, "resource/tga/health_color", true, false );
	}

	m_iDeadMaterialIndex = surface()->DrawGetTextureId( "resource/tga/health_dead" );
	if ( m_iDeadMaterialIndex == -1 ) // we didn't find it, so create a new one
	{
		m_iDeadMaterialIndex = surface()->CreateNewTextureID();	
		surface()->DrawSetTextureFile( m_iDeadMaterialIndex, "resource/tga/health_dead", true, false );
	}	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TFHealthPanel::Paint()
{
	BaseClass::Paint();

	int x, y, w, h;
	GetBounds( x, y, w, h );

	Vertex_t vert[4];	
	float uv1 = 0.0f;
	float uv2 = 1.0f;
	int xpos = 0, ypos = 0;

	if ( m_flHealth <= 0 )
	{
		// Draw the dead material
		surface()->DrawSetTexture( m_iDeadMaterialIndex );
		
		vert[0].Init( Vector2D( xpos, ypos ), Vector2D( uv1, uv1 ) );
		vert[1].Init( Vector2D( xpos + w, ypos ), Vector2D( uv2, uv1 ) );
		vert[2].Init( Vector2D( xpos + w, ypos + h ), Vector2D( uv2, uv2 ) );				
		vert[3].Init( Vector2D( xpos, ypos + h ), Vector2D( uv1, uv2 ) );

		surface()->DrawSetColor( Color(255,255,255,255) );
	}
	else
	{
		float flDamageY = h * ( 1.0f - m_flHealth );

		// blend in the red "damage" part
		surface()->DrawSetTexture( m_iMaterialIndex );

		Vector2D uv11( uv1, uv2 - m_flHealth );
		Vector2D uv21( uv2, uv2 - m_flHealth );
		Vector2D uv22( uv2, uv2 );
		Vector2D uv12( uv1, uv2 );

		vert[0].Init( Vector2D( xpos, flDamageY ), uv11 );
		vert[1].Init( Vector2D( xpos + w, flDamageY ), uv21 );
		vert[2].Init( Vector2D( xpos + w, ypos + h ), uv22 );				
		vert[3].Init( Vector2D( xpos, ypos + h ), uv12 );

		surface()->DrawSetColor( GetDrawColor() );
	}

	surface()->DrawTexturedPolygon( 4, (float *)vert );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerHealth::CTFHudPlayerHealth( Panel *parent, const char *name ) : EditablePanel( parent, name )
{
	m_pHealthImage = new TFHealthPanel( this, "PlayerStatusHealthImage" );	
	m_pHealthImageBG = new ImagePanel( this, "PlayerStatusHealthImageBG" );
	m_pHealthBonusImage = new ImagePanel( this, "PlayerStatusHealthBonusImage" );
	m_pHealthValue = new Label( this, "PlayerStatusHealthValue", "" );

	m_nBonusHealthOrigX = 0;
	m_nBonusHealthOrigY = 0;
	m_nBonusHealthOrigW = 0;
	m_nBonusHealthOrigH = 0;
	m_nHealth = 0;
	m_nMaxHealth = 0;

	m_flNextThink = 0.0f;

	SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::Reset()
{
	m_flNextThink = engine->GetClientTime() + 0.05f;
	m_nHealth = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::ApplySchemeSettings( IScheme *pScheme )
{
	// load control settings...
	LoadControlSettings( GetResFileName() );

	m_flNextThink = 0.0f;

	if ( m_pHealthBonusImage )
	{
		m_pHealthBonusImage->GetBounds( m_nBonusHealthOrigX, m_nBonusHealthOrigY, m_nBonusHealthOrigW, m_nBonusHealthOrigH );
	}

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::SetHealth( int iNewHealth, int iMaxHealth, int	iMaxBuffedHealth )
{
	int nPrevHealth = m_nHealth;

	// set our health
	m_nHealth = iNewHealth;
	m_nMaxHealth = iMaxHealth;

	if(iMaxHealth <= 0)
	{
		m_nHealth = 0;
		m_nMaxHealth = 0;
	}
	else
	{
		m_pHealthImage->SetHealth( (float)(m_nHealth) / (float)(m_nMaxHealth) );
	}

	if ( m_pHealthImage )
	{
		m_pHealthImage->SetDrawColor( Color( 255, 255, 255, 255 ) );
	}

	if ( m_nHealth <= 0 )
	{
		if ( m_pHealthImageBG->IsVisible() )
		{
			m_pHealthImageBG->SetVisible( false );
		}
		HideHealthBonusImage();
	}
	else
	{
		if ( !m_pHealthImageBG->IsVisible() )
		{
			m_pHealthImageBG->SetVisible( true );
		}

		// are we getting a health bonus?
		if ( m_nHealth > m_nMaxHealth )
		{
			if ( m_pHealthBonusImage )
			{
				if ( !m_pHealthBonusImage->IsVisible() )
				{
					m_pHealthBonusImage->SetVisible( true );
					GetAnimationController()->StartAnimationSequence( this, "HudHealthBonusPulse" );
				}

				m_pHealthBonusImage->SetDrawColor( Color( 255, 255, 255, 255 ) );

				// scale the flashing image based on how much health bonus we currently have
				float flBoostMaxAmount = ( iMaxBuffedHealth ) - m_nMaxHealth;
				float flPercent = ( m_nHealth - m_nMaxHealth ) / flBoostMaxAmount;

				int nPosAdj = RoundFloatToInt( flPercent * m_nHealthBonusPosAdj );
				int nSizeAdj = 2 * nPosAdj;

				m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX - nPosAdj, 
					m_nBonusHealthOrigY - nPosAdj, 
					m_nBonusHealthOrigW + nSizeAdj,
					m_nBonusHealthOrigH + nSizeAdj );
			}
		}
		// are we close to dying?
		else if ( m_nHealth < m_nMaxHealth * m_flHealthDeathWarning )
		{
			if ( m_pHealthBonusImage )
			{
				if ( !m_pHealthBonusImage->IsVisible() )
				{
					m_pHealthBonusImage->SetVisible( true );
					GetAnimationController()->StartAnimationSequence( this, "HudHealthDyingPulse" );
				}

				m_pHealthBonusImage->SetDrawColor( m_clrHealthDeathWarningColor );

				// scale the flashing image based on how much health bonus we currently have
				float flBoostMaxAmount = m_nMaxHealth * m_flHealthDeathWarning;
				float flPercent = ( flBoostMaxAmount - m_nHealth ) / flBoostMaxAmount;

				int nPosAdj = RoundFloatToInt( flPercent * m_nHealthBonusPosAdj );
				int nSizeAdj = 2 * nPosAdj;

				m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX - nPosAdj, 
					m_nBonusHealthOrigY - nPosAdj, 
					m_nBonusHealthOrigW + nSizeAdj,
					m_nBonusHealthOrigH + nSizeAdj );
			}

			if ( m_pHealthImage )
			{
				m_pHealthImage->SetDrawColor( m_clrHealthDeathWarningColor );
			}
		}
		// turn it off
		else
		{
			HideHealthBonusImage();
		}
	}

	// set our health display value
	if ( nPrevHealth != m_nHealth )
	{
		if ( m_nHealth > 0 )
		{
			SetDialogVariable( "Health", m_nHealth );
		}
		else
		{
			SetDialogVariable( "Health", "" );
		}	
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::HideHealthBonusImage( void )
{
	if ( m_pHealthBonusImage && m_pHealthBonusImage->IsVisible() )
	{
		m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX, m_nBonusHealthOrigY, m_nBonusHealthOrigW, m_nBonusHealthOrigH );
		m_pHealthBonusImage->SetVisible( false );
		GetAnimationController()->StartAnimationSequence( this, "HudHealthBonusPulseStop" );
		GetAnimationController()->StartAnimationSequence( this, "HudHealthDyingPulseStop" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::OnThink()
{
	if ( m_flNextThink < engine->GetClientTime() )
	{
		SetHealth( (*gCKFVars.g_iHealth), (*gCKFVars.g_iMaxHealth), GetBuffedMaxHealth( (*gCKFVars.g_iMaxHealth) ) );

		m_flNextThink = engine->GetClientTime() + 0.05f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: PlayerStatus Panel
//-----------------------------------------------------------------------------

CTFHudPlayerStatus::CTFHudPlayerStatus() : CHudElement(), BaseClass( NULL, "HudPlayerStatus" ) 
{
	SetProportional(true);

	SetScheme("ClientScheme");

	m_pHudPlayerClass = new CTFHudPlayerClass( this, "HudPlayerClass" );
	m_pHudPlayerHealth = new CTFHudPlayerHealth( this, "HudPlayerHealth" );

	SetHiddenBits(HIDEHUD_HEALTH);
}

void CTFHudPlayerStatus::Reset(void)
{
	if ( m_pHudPlayerClass )
	{
		m_pHudPlayerClass->Reset();
	}

	if ( m_pHudPlayerHealth )
	{
		m_pHudPlayerHealth->Reset();
	}
}

void CTFHudPlayerStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CTFHudPlayerStatus::Init(void)
{
	SetVisible(true);
}

void CTFHudPlayerStatus::VidInit(void)
{
}

int CTFHudPlayerStatus::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	if(m_pHudPlayerClass->FireMessage(pszName, iSize, pbuf))
		return 1;

	return 0;
}

bool CTFHudPlayerStatus::ShouldDraw( void )
{ 
	if(gHUD.IsHidden( m_iHiddenBits ) || g_iUser1)
		return false;
	if(!(*gCKFVars.g_iMaxHealth) || !(*gCKFVars.g_iClass))
		return false;
	if((*gCKFVars.g_iTeam) != TEAM_RED && (*gCKFVars.g_iTeam) != TEAM_BLUE)
		return false;
	return true;
}

}