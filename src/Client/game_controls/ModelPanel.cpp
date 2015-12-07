//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include <UtlVector.h>

#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include <vgui/Cursor.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

typedef float vec3_t[3];
#include <const.h>
#include <cl_entity.h>
#include <com_model.h>
#include <studio.h>
#include <r_studioint.h>
#include <cl_util.h>
#include <ref_int.h>
#include <game_controls/ModelPanel.h>

#include <ICKFClient.h>

using namespace vgui;

extern engine_studio_api_t IEngineStudio;

DECLARE_BUILD_FACTORY( ModelPanel );

ModelPanel::ModelPanel( Panel *parent, const char *name ) : Panel( parent, name )
{
	m_szModel[0] = '\0';
	InitEntity();

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: Fix a bug that client crashes because of uncached model or invalid weaponmodel
//-----------------------------------------------------------------------------
void ModelPanel::OnTick( void )
{
	const char *mapname = gEngfuncs.pfnGetLevelName();
	if(!mapname || !mapname[0])
	{
		m_entity.model = NULL;
		m_entity.curstate.weaponmodel = 0;
	}
	else if(0 != Q_strcmp(mapname, m_currentmap))
	{
		m_entity.model = NULL;
		m_entity.curstate.weaponmodel = 0;
		Q_strncpy(m_currentmap, mapname, sizeof(m_currentmap));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModelPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy(m_szModel, inResourceData->GetString("model"), sizeof(m_szModel));
	m_entity.curstate.skin = atoi(inResourceData->GetString("skin", "0"));
	sscanf(inResourceData->GetString("origin", "0 0 0"), "%f %f %f", &m_origin[0], &m_origin[1], &m_origin[2]);
	sscanf(inResourceData->GetString("angles", "0 0 0"), "%f %f %f", &m_angles[0], &m_angles[1], &m_angles[2]);

	VectorCopy(m_origin, m_entity.origin);
	VectorCopy(m_angles, m_entity.angles);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModelPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( pScheme->GetBorder( "NoBorder" ) );
	SetBgColor( pScheme->GetColor( "Blank", Color( 0,0,0,0 ) ) );
}

int LookupSequence(void *pmodel, const char *label)
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (!Q_stricmp(pseqdesc[i].label, label))
			return i;
	}

	return 0;
}

void ModelPanel::SetModel(const char *szModel)
{
	if(szModel != NULL)
	{
		Q_strncpy(m_szModel, szModel, 63);
		m_szModel[63] = '\0';
	}
	else
	{
		m_szModel[0] = 0;
	}
}

void ModelPanel::LoadModel(model_t *mod)
{
	m_entity.model = mod;
	if(m_entity.model)
	{
		m_entity.model->needload = 3;
	}
}

void ModelPanel::LoadModel(void)
{
	if(!IEngineStudio.Mod_ForName || !m_szModel[0])
		return;

	m_entity.model = IEngineStudio.Mod_ForName(m_szModel, false);
	if(m_entity.model)
	{
		m_entity.model->needload = 3;
	}
}

void ModelPanel::SetAnimation(const char *szAnim)
{
	if(!m_entity.model || !IEngineStudio.Mod_Extradata)
		return;

	m_entity.curstate.frame = 0;
	m_entity.curstate.animtime = gEngfuncs.GetClientTime();
	m_entity.curstate.framerate = 1;
	m_entity.curstate.sequence = LookupSequence(IEngineStudio.Mod_Extradata(m_entity.model), szAnim);
}

void ModelPanel::InitEntity(void)
{
	memset(&m_entity, 0, sizeof(cl_entity_t));
	m_entity.curstate.rendermode = kRenderNormal;
	m_entity.curstate.renderfx = kRenderFxNone;
	m_entity.curstate.renderamt = 255;
	m_entity.curstate.framerate = 1;
	m_entity.curstate.frame = 0;
	m_entity.curstate.skin = 0;
	m_entity.curstate.body = 0;
	m_entity.curstate.sequence = 0;
	m_entity.curstate.solid = SOLID_NOT;
	m_entity.curstate.movetype = MOVETYPE_NOCLIP;	
	m_entity.curstate.entityType = 0;
	m_entity.curstate.vuser1[0] = 0;
	m_entity.curstate.vuser1[1] = 0;
	m_entity.curstate.vuser1[2] = 0;

	m_entity.curstate.entityType = ET_HUDENTITY;
	m_entity.curstate.iuser1 = 128;
	m_entity.curstate.iuser2 = 128;
	m_entity.model = NULL;
	m_entity.player = 0;
}

void ModelPanel::Paint(void)
{
	if(m_entity.model)
	{
		int clipRect[4];
		ipanel()->GetClipRect( GetVPanel(), clipRect[0], clipRect[1], clipRect[2], clipRect[3] );
		g_pCKFClient->Draw3DHUDStudioModel(&m_entity, clipRect[0], clipRect[1], clipRect[2]-clipRect[0], clipRect[3]-clipRect[1], true);
	}
}