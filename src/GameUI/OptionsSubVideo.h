#ifndef OPTIONS_SUB_VIDEO_H
#define OPTIONS_SUB_VIDEO_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/PropertyPage.h>
#include "EngineInterface.h"
#include "IGameUIFuncs.h"
#include "URLButton.h"

class CCvarSlider;

class COptionsSubVideo : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubVideo, vgui::PropertyPage);

public:
	COptionsSubVideo(vgui::Panel *parent);
	~COptionsSubVideo(void);

public:
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);

private:
	void GetVidSettings(void);
	void RevertVidSettings(void);
	void ApplyVidSettings(bool bForceRefresh);
	void SetCurrentResolutionComboItem(void);
	void PrepareResolutionList(void);
	bool RequiresRestart(void);

private:
	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC_PTR(OnButtonChecked, "CheckButtonChecked", panel);
	MESSAGE_FUNC_PTR_CHARPTR(OnTextChanged, "TextChanged", panel, text);
	MESSAGE_FUNC(OpenAdvanced, "OpenAdvanced");

private:
	struct CVidSettings
	{
		int w, h;
		int bpp;
		int windowed;
		char renderer[128];
	};

	CVidSettings m_OrigSettings;
	CVidSettings m_CurrentSettings;

private:
	int m_nSelectedMode;
	bool m_bRequireRestart;

	CCvarSlider *m_pBrightnessSlider;
	CCvarSlider *m_pGammaSlider;

	char m_pszRenderNames[3][32];
	char m_pszAspectName[2][32];

	vgui::ComboBox *m_pColorDepth;
	vgui::ComboBox *m_pMode;
	vgui::CheckButton *m_pWindowed;
	vgui::ComboBox *m_pAspectRatio;
	vgui::Button *m_pAdvanced;

	vgui::DHANDLE<class COptionsSubVideoAdvancedDlg> m_hOptionsSubVideoAdvancedDlg;
};

#endif