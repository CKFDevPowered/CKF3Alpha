#ifndef TFMODELPANEL_H
#define TFMODELPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <com_model.h>

namespace vgui
{

class ModelPanel : public Panel
{
public:
	DECLARE_CLASS_SIMPLE( ModelPanel, Panel );

	ModelPanel( Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint(void);

	void LoadModel(void);
	void LoadModel(model_t *mod);
	void SetModel(const char *szModel);
	void SetAnimation(const char *szAnim);
	void InitEntity(void);

	MESSAGE_FUNC(OnTick, "Tick");

	cl_entity_t *GetEntity(void){return &m_entity;};

protected:
	char m_szModel[64];
	cl_entity_t m_entity;
	vec3_t m_origin;
	vec3_t m_angles;
};

}

#endif