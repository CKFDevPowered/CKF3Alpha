#ifndef BUYMOUSEOVERPANELBUTTON_H
#define BUYMOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <KeyValues.h>
#include <FileSystem.h>
#include "hud.h"
#include "shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>

class BuyMouseOverPanelButton : public MouseOverPanelButton
{
	typedef MouseOverPanelButton BaseClass;

public:
	BuyMouseOverPanelButton(vgui::Panel *parent, const char *panelName, vgui::EditablePanel *panel) : BaseClass(parent, panelName, panel)
	{
		m_iPrice = 0;
		m_iASRestrict = 0;
		m_iDEUseOnly = 0;
	}

	~BuyMouseOverPanelButton(void)
	{
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		KeyValues *kv = resourceData->FindKey("cost", false);

		if (kv)
			m_iPrice = kv->GetInt();

		kv = resourceData->FindKey("as_restrict", false);

		if (kv)
			m_iASRestrict = kv->GetInt();

		kv = resourceData->FindKey("de_useonly", false);

		if (kv)
			m_iDEUseOnly = kv->GetInt();

		BaseClass::ApplySettings(resourceData);
	}

	int GetASRestrict(void) { return m_iASRestrict; }
	int GetDEUseOnly(void) { return m_iDEUseOnly; }

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();

#ifndef CS_SHIELD_ENABLED
		if (!Q_stricmp(GetName(), "shield"))
		{
			SetVisible(false);
			SetEnabled(false);
		}
#endif
	}

	void SetCurrentPrice(int iPrice)
	{
		m_iPrice = iPrice;
	}

	int GetCurrentPrice(void)
	{
		return m_iPrice;
	}

protected:
	int m_iPrice;
	int m_iKeyboard;
	int m_iASRestrict;
	int m_iDEUseOnly;
};

#endif