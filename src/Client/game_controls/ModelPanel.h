#ifndef TFRICHTEXT_H
#define TFRICHTEXT_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

namespace vgui
{

class ModelPanel : public Panel
{
public:
	DECLARE_CLASS_SIMPLE( ModelPanel, vgui::Panel );

	ModelPanel( vgui::Panel *parent, const char *panelName );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	virtual void PaintTraverse(bool Repaint, bool allowForce = true);
	virtual void Think();
	void SetModel(void);
	void SetAnimation(const char *szAnim);
	void InitEntity(void);
	cl_entity_t *GetEntity(void){return &_entity;};

private:
	char m_szModel[64];
	cl_entity_t _entity;
	vec3_t _origin;
	vec3_t _angles;
};

}

#endif