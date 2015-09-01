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

extern ICKFClient *g_pCKFClient;

using namespace vgui;

extern engine_studio_api_t IEngineStudio;

ModelPanel::ModelPanel( Panel *parent, const char *name ) : Panel( parent, name )
{
	m_szModel[0] = '\0';
	InitEntity();

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModelPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy(m_szModel, inResourceData->GetString("model"), sizeof(m_szModel));
	_entity.curstate.skin = atoi(inResourceData->GetString("skin", "0"));
	sscanf(inResourceData->GetString("origin", "0 0 0"), "%f %f %f", &_origin[0], &_origin[1], &_origin[2]);
	sscanf(inResourceData->GetString("angles", "0 0 0"), "%f %f %f", &_angles[0], &_angles[1], &_angles[2]);

	VectorCopy(_origin, _entity.origin);
	VectorCopy(_angles, _entity.angles);

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
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
	Q_strncpy(m_szModel, szModel, 63);
	m_szModel[63] = '\0';
}

void ModelPanel::LoadModel(model_t *mod)
{
	_entity.model = mod;
	if(_entity.model)
	{
		_entity.model->needload = 3;
	}
}

void ModelPanel::LoadModel(void)
{
	if(!IEngineStudio.Mod_ForName || !m_szModel[0])
		return;

	_entity.model = IEngineStudio.Mod_ForName(m_szModel, false);
	if(_entity.model)
	{
		_entity.model->needload = 3;
	}
}

void ModelPanel::SetAnimation(const char *szAnim)
{
	if(!_entity.model || !IEngineStudio.Mod_Extradata)
		return;
	_entity.curstate.frame = 0;
	_entity.curstate.animtime = gEngfuncs.GetClientTime();
	_entity.curstate.framerate = 1;
	_entity.curstate.sequence = LookupSequence(IEngineStudio.Mod_Extradata(_entity.model), szAnim);
}

void ModelPanel::InitEntity(void)
{
	memset(&_entity, 0, sizeof(cl_entity_t));
	_entity.curstate.rendermode = kRenderNormal;
	_entity.curstate.renderfx = kRenderFxNone;
	_entity.curstate.renderamt = 255;
	_entity.curstate.framerate = 1;
	_entity.curstate.frame = 0;
	_entity.curstate.skin = 0;
	_entity.curstate.body = 0;
	_entity.curstate.sequence = 0;
	_entity.curstate.solid = SOLID_NOT;
	_entity.curstate.movetype = MOVETYPE_NOCLIP;	
	_entity.curstate.entityType = 0;
	_entity.curstate.vuser1[0] = 0;
	_entity.curstate.vuser1[1] = 0;
	_entity.curstate.vuser1[2] = 0;
#define EF_3DMENU					(1<<13)
	_entity.curstate.effects |= EF_3DMENU;
	_entity.curstate.iuser1 = 128;
	_entity.curstate.iuser2 = 128;
	_entity.model = NULL;
	_entity.player = 0;
}

void ModelPanel::Paint(void)
{
	if(_entity.model)
	{
		int clipRect[4];
		ipanel()->GetClipRect( GetVPanel(), clipRect[0], clipRect[1], clipRect[2], clipRect[3] );
		g_pCKFClient->Draw3DHUDStudioModel(&_entity, clipRect[0], clipRect[1], clipRect[2]-clipRect[0], clipRect[3]-clipRect[1], true);
	}
}