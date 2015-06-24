#include "EngineInterface.h"
#include "OptionsSubVideo.h"
#include "CvarSlider.h"
#include "BasePanel.h"
#include "IGameUIFuncs.h"
#include "modes.h"
#include "filesystem.h"
#include "GameUI_Interface.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/QueryBox.h"
#include "CvarToggleCheckButton.h"
#include "tier1/KeyValues.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "tier0/ICommandLine.h"
#include "ModInfo.h"
#include "windows.h"
#include "Video.h"
#include "plugins.h"
#include "configs.h"

using namespace vgui;

struct RatioToAspectMode_t
{
	int anamorphic;
	float aspectRatio;
};

RatioToAspectMode_t g_RatioToAspectModes[] =
{
	{ 0, 4.0f / 3.0f },
	{ 1, 16.0f / 9.0f },
	{ 2, 16.0f / 10.0f },
	{ 2, 1.0f },
};

int GetScreenAspectMode(int width, int height)
{
	float aspectRatio = (float)width / (float)height;

	float closestAspectRatioDist = 99999.0f;
	int closestAnamorphic = 0;

	for (int i = 0; i < ARRAYSIZE(g_RatioToAspectModes); i++)
	{
		float dist = fabs(g_RatioToAspectModes[i].aspectRatio - aspectRatio);

		if (dist < closestAspectRatioDist)
		{
			closestAspectRatioDist = dist;
			closestAnamorphic = g_RatioToAspectModes[i].anamorphic;
		}
	}

	return closestAnamorphic;
}

void GetResolutionName(vmode_t *mode, char *sz, int sizeofsz)
{
	Q_snprintf(sz, sizeofsz, "%i x %i", mode->width, mode->height);
}

class CGammaDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CGammaDialog, vgui::Frame);

public:
	CGammaDialog(vgui::VPANEL hParent) : BaseClass(NULL, "OptionsSubVideoGammaDlg")
	{
		SetTitle("#GameUI_AdjustGamma_Title", true);
		SetSize(400, 260);
		SetDeleteSelfOnClose(true);

		m_pGammaSlider = new CCvarSlider(this, "Gamma", "#GameUI_Gamma", 1.6f, 2.6f, "bte_monitorgamma");
		m_pGammaLabel = new Label(this, "Gamma label", "#GameUI_Gamma");
		m_pGammaEntry = new TextEntry(this, "GammaEntry");

		Button *ok = new Button(this, "OKButton", "#vgui_ok");
		ok->SetCommand(new KeyValues("OK"));

		LoadControlSettings("resource/OptionsSubVideoGammaDlg.res");
		MoveToCenterOfScreen();
		SetSizeable(false);

		m_pGammaSlider->SetTickCaptions("#GameUI_Light", "#GameUI_Dark");
	}

	MESSAGE_FUNC_PTR(OnGammaChanged, "SliderMoved", panel)
	{
		if (panel == m_pGammaSlider)
			m_pGammaSlider->ApplyChanges();
	}

	virtual void Activate(void)
	{
		BaseClass::Activate();
		m_flOriginalGamma = m_pGammaSlider->GetValue();
		UpdateGammaLabel();

		input()->SetAppModalSurface(GetVPanel());
	}

	MESSAGE_FUNC(OnOK, "OK")
	{
		m_flOriginalGamma = m_pGammaSlider->GetValue();
		Close();
	}

	virtual void OnClose(void)
	{
		m_pGammaSlider->SetValue(m_flOriginalGamma);
		m_pGammaSlider->ApplyChanges();

		BaseClass::OnClose();
	}

	MESSAGE_FUNC_PTR(OnControlModified, "ControlModified", panel)
	{
		if (panel == m_pGammaSlider && m_pGammaSlider->HasBeenModified())
			UpdateGammaLabel();
	}

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel)
	{
	}

	void UpdateGammaLabel(void)
	{
		char buf[64];
		Q_snprintf(buf, sizeof(buf), " %.1f", m_pGammaSlider->GetSliderValue());
		m_pGammaEntry->SetText(buf);
	}

private:
	CCvarSlider *m_pGammaSlider;
	vgui::Label *m_pGammaLabel;
	vgui::TextEntry *m_pGammaEntry;
	float m_flOriginalGamma;
};

class COptionsSubVideoAdvancedDlg : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(COptionsSubVideoAdvancedDlg, vgui::Frame);

public:
	COptionsSubVideoAdvancedDlg(vgui::Panel *parent) : BaseClass(parent , "OptionsSubVideoAdvancedDlg")
	{
		SetTitle("#GameUI_VideoAdvanced_Title", true);
		SetSize(260, 400);
		SetSizeable(false);
		SetDeleteSelfOnClose(true);

		m_pAnisotropicFiltering = new ComboBox(this, "AnisotropicFiltering", 5, false);
		m_pAnisotropicFiltering->AddItem("1X", NULL);
		m_pAnisotropicFiltering->AddItem("2X", NULL);
		m_pAnisotropicFiltering->AddItem("4X", NULL);
		m_pAnisotropicFiltering->AddItem("8X", NULL);
		m_pAnisotropicFiltering->AddItem("16X", NULL);

		m_pDetailTexture = new CheckButton(this, "DetailTexture", "#GameUI_Detail_Texture");
		m_pWaterReflect = new CheckButton(this, "WaterReflect", "#GameUI_Water_Reflect");
		m_pWaitForVSync = new CheckButton(this, "WaitForVSync", "#GameUI_Wait_For_VSync");

		LoadControlSettings("resource/OptionsSubVideoAdvancedDlg.res");

		if (!engine->pfnGetCvarPointer("gl_ansio"))
			m_pAnisotropicFiltering->SetEnabled(false);

		if (!engine->pfnGetCvarPointer("r_detailtextures"))
			m_pDetailTexture->SetEnabled(false);

		if (!engine->pfnGetCvarPointer("gl_reflectwater"))
			m_pWaterReflect->SetEnabled(false);

		if (!engine->pfnGetCvarPointer("gl_vsync"))
			m_pWaitForVSync->SetEnabled(false);
	}

	virtual void Activate(void)
	{
		OnResetData();
		BaseClass::Activate();

		input()->SetAppModalSurface(GetVPanel());
	}

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel)
	{
	}

	MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden")
	{
		Close();
	}

	MESSAGE_FUNC(ResetDXLevelCombo, "ResetDXLevelCombo")
	{
	}

	MESSAGE_FUNC(OK_Confirmed, "OK_Confirmed")
	{
		Close();
	}

	void ApplyChangesToConVar(const char *pConVarName, int value)
	{
		char szCmd[256];
		Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
		engine->pfnClientCmd(szCmd);
	}

	virtual void ApplyChanges(void)
	{
		int activateItem = m_pAnisotropicFiltering->GetActiveItem();

		switch (activateItem)
		{
			case 0: ApplyChangesToConVar("gl_ansio", 1); break;
			case 1: ApplyChangesToConVar("gl_ansio", 2); break;
			case 2: ApplyChangesToConVar("gl_ansio", 4); break;
			case 3: ApplyChangesToConVar("gl_ansio", 8); break;
			case 4: ApplyChangesToConVar("gl_ansio", 16); break;
		}

		ApplyChangesToConVar("r_detailtextures", m_pDetailTexture->IsSelected());
		ApplyChangesToConVar("gl_reflectwater", m_pWaterReflect->IsSelected());
		ApplyChangesToConVar("gl_vsync", m_pWaitForVSync->IsSelected());
	}

	virtual void OnResetData(void)
	{
		static cvar_t *gl_ansio = engine->pfnGetCvarPointer("gl_ansio");
		static cvar_t *r_detailtextures = engine->pfnGetCvarPointer("r_detailtextures");
		static cvar_t *gl_reflectwater = engine->pfnGetCvarPointer("gl_reflectwater");
		static cvar_t *gl_vsync = engine->pfnGetCvarPointer("gl_vsync");

		if (!gl_ansio)
			m_pAnisotropicFiltering->SetEnabled(false);

		if (!r_detailtextures)
			m_pDetailTexture->SetEnabled(false);

		if (!gl_reflectwater)
			m_pWaterReflect->SetEnabled(false);

		if (!gl_vsync)
			m_pWaitForVSync->SetEnabled(false);

		if (gl_ansio)
		{
			int ansio = gl_ansio->value;

			if (ansio >= 16)
				m_pAnisotropicFiltering->ActivateItem(4);
			else if (ansio >= 8)
				m_pAnisotropicFiltering->ActivateItem(3);
			else if (ansio >= 4)
				m_pAnisotropicFiltering->ActivateItem(2);
			else if (ansio >= 2)
				m_pAnisotropicFiltering->ActivateItem(1);
			else
				m_pAnisotropicFiltering->ActivateItem(0);
		}
		else
		{
			m_pAnisotropicFiltering->SetEnabled(false);
		}

		if (r_detailtextures)
			m_pDetailTexture->SetSelected(r_detailtextures->value != 0);
		else
			m_pDetailTexture->SetEnabled(false);

		if (gl_reflectwater)
			m_pWaterReflect->SetSelected(gl_reflectwater->value != 0);
		else
			m_pWaterReflect->SetEnabled(false);

		if (gl_vsync)
			m_pWaitForVSync->SetSelected(gl_vsync->value != 0);
		else
			m_pWaitForVSync->SetEnabled(false);
	}

	virtual void OnCommand(const char *command)
	{
		if (!stricmp(command, "OK"))
		{
			ApplyChanges();
			Close();
		}
		else
		{
			BaseClass::OnCommand(command);
		}
	}

	bool RequiresRestart(void)
	{
		return false;
	}

private:
	bool m_bUseChanges;
	vgui::ComboBox *m_pAnisotropicFiltering;
	vgui::CheckButton *m_pDetailTexture, *m_pWaterReflect, *m_pWaitForVSync;
};

COptionsSubVideo::COptionsSubVideo(vgui::Panel *parent) : PropertyPage(parent, NULL)
{
	memset(&m_OrigSettings, 0, sizeof(m_OrigSettings));
	memset(&m_CurrentSettings, 0, sizeof(m_CurrentSettings));

	m_pBrightnessSlider = new CCvarSlider(this, "Brightness", "#GameUI_Brightness", 0.0f, 2.0f, "brightness");
	m_pGammaSlider = new CCvarSlider(this, "Gamma", "#GameUI_Gamma", 1.0f, 3.0f, "gamma");

	m_bRequireRestart = false;

	GetVidSettings();

	m_pMode = new ComboBox(this, "Resolution", 8, false);
	m_pAspectRatio = new ComboBox(this, "AspectRatio", 6, false);
	m_pAdvanced = new Button(this, "AdvancedButton", "#GameUI_AdvancedEllipsis");
	m_pAdvanced->SetCommand(new KeyValues("OpenAdvanced"));

	char pszAspectName[3][64];
	wchar_t *unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectNormal");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[0], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x9");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[1], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x10");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[2], 32);

	int iNormalItemID = m_pAspectRatio->AddItem(pszAspectName[0], NULL);
	int i16x9ItemID = m_pAspectRatio->AddItem(pszAspectName[1], NULL);
	int i16x10ItemID = m_pAspectRatio->AddItem(pszAspectName[2], NULL);

	int iAspectMode = GetScreenAspectMode(m_CurrentSettings.w, m_CurrentSettings.h);

	switch (iAspectMode)
	{
		default:
		case 0:
		{
			m_pAspectRatio->ActivateItem(iNormalItemID);
			break;
		}

		case 1:
		{
			m_pAspectRatio->ActivateItem(i16x9ItemID);
			break;
		}

		case 2:
		{
			m_pAspectRatio->ActivateItem(i16x10ItemID);
			break;
		}
	}

	m_pColorDepth = new ComboBox(this, "ColorDepth", 2, false);
	m_pColorDepth->AddItem("#GameUI_MediumBitDepth", NULL);
	m_pColorDepth->AddItem("#GameUI_HighBitDepth", NULL);
	m_pColorDepth->SetVisible(false);

	m_pWindowed = new CheckButton(this, "Windowed", "#GameUI_Windowed");

	if (!gConfigs.bEnableRenderer)
		m_pAdvanced->SetEnabled(false);

	LoadControlSettings("Resource\\OptionsSubVideo.res");
	PrepareResolutionList();
}

void COptionsSubVideo::PrepareResolutionList(void)
{
	char sz[256];
	m_pMode->GetText(sz, 256);

	int currentWidth = 0, currentHeight = 0;
	sscanf(sz, "%i x %i", &currentWidth, &currentHeight);

	m_pMode->DeleteAllItems();
	m_pAspectRatio->SetItemEnabled(1, false);
	m_pAspectRatio->SetItemEnabled(2, false);

	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	bool bFoundWidescreen = false;
	int selectedItemID = -1;

	for (int i = 0; i < count; i++, plist++)
	{
		char sz[256];
		GetResolutionName(plist, sz, sizeof(sz));

		int itemID = -1;
		int iAspectMode = GetScreenAspectMode(plist->width, plist->height);

		if (iAspectMode > 0)
		{
			m_pAspectRatio->SetItemEnabled(iAspectMode, true);
			bFoundWidescreen = true;
		}

		if (iAspectMode == m_pAspectRatio->GetActiveItem())
		{
			itemID = m_pMode->AddItem(sz, NULL);
		}

		if (plist->width == currentWidth && plist->height == currentHeight)
		{
			selectedItemID = itemID;
		}
		else if (selectedItemID == -1 && plist->width == m_CurrentSettings.w && plist->height == m_CurrentSettings.h)
		{
			selectedItemID = itemID;
		}
	}

	m_pAspectRatio->SetEnabled(bFoundWidescreen);
	m_nSelectedMode = selectedItemID;

	if (selectedItemID != -1)
	{
		m_pMode->ActivateItem(selectedItemID);
	}
	else
	{
		sprintf(sz, "%d x %d", m_CurrentSettings.w, m_CurrentSettings.h);
		m_pMode->SetText(sz);
	}
}

COptionsSubVideo::~COptionsSubVideo(void)
{
	if (m_hOptionsSubVideoAdvancedDlg.Get())
		m_hOptionsSubVideoAdvancedDlg->MarkForDeletion();
}

void COptionsSubVideo::OnResetData(void)
{
	m_bRequireRestart = false;

	m_pBrightnessSlider->Reset();
	m_pGammaSlider->Reset();
	m_pWindowed->SetSelected(m_CurrentSettings.windowed ? true : false);

	SetCurrentResolutionComboItem();
}

void COptionsSubVideo::SetCurrentResolutionComboItem(void)
{
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	int resolution = -1;

	for (int i = 0; i < count; i++, plist++)
	{
		if (plist->width == m_CurrentSettings.w && plist->height == m_CurrentSettings.h)
		{
			resolution = i;
			break;
		}
	}

	if (resolution != -1)
	{
		char sz[256];
		GetResolutionName(plist, sz, sizeof(sz));
		m_pMode->SetText(sz);
	}

	if (m_CurrentSettings.bpp > 16)
		m_pColorDepth->ActivateItemByRow(1);
	else
		m_pColorDepth->ActivateItemByRow(0);
}

void COptionsSubVideo::OnApplyChanges(void)
{
	bool bChanged = m_pBrightnessSlider->HasBeenModified() || m_pGammaSlider->HasBeenModified();

	m_pBrightnessSlider->ApplyChanges();
	m_pGammaSlider->ApplyChanges();

	if (RequiresRestart())
	{
	}

	ApplyVidSettings(bChanged);
}

void COptionsSubVideo::GetVidSettings(void)
{
	CVidSettings *p = &m_OrigSettings;

	gameuifuncs->GetCurrentVideoMode(&p->w, &p->h, &p->bpp);

	VID_GetCurrentRenderer(p->renderer, sizeof(p->renderer), &p->windowed);

	m_CurrentSettings = m_OrigSettings;
}

void COptionsSubVideo::RevertVidSettings(void)
{
	m_CurrentSettings = m_OrigSettings;
}

void COptionsSubVideo::ApplyVidSettings(bool bForceRefresh)
{
	if (m_pMode)
	{
		char sz[256], colorDepth[256];

		if (m_nSelectedMode == -1)
			m_pMode->GetText(sz, 256);
		else
			m_pMode->GetItemText(m_nSelectedMode, sz, 256);

		m_pColorDepth->GetText(colorDepth, sizeof(colorDepth));

		int w, h;
		sscanf(sz, "%i x %i", &w, &h);
		m_CurrentSettings.w = w;
		m_CurrentSettings.h = h;

		if (strstr(colorDepth, "32"))
			m_CurrentSettings.bpp = 32;
		else
			m_CurrentSettings.bpp = 16;
	}

	if (m_pWindowed)
	{
		m_CurrentSettings.windowed = m_pWindowed->IsSelected() != false;
	}

	if (m_CurrentSettings.w != m_OrigSettings.w || m_CurrentSettings.h != m_OrigSettings.h || m_CurrentSettings.bpp != m_OrigSettings.bpp || m_CurrentSettings.windowed != m_OrigSettings.windowed)
	{
		VID_SetVideoMode(m_CurrentSettings.w, m_CurrentSettings.h, m_CurrentSettings.bpp);
		VID_SetRenderer(m_CurrentSettings.renderer, m_CurrentSettings.windowed);
		VID_Restart();
	}
}

void COptionsSubVideo::OnButtonChecked(Panel *panel)
{
	if (panel == m_pWindowed)
	{
		OnDataChanged();
	}
}

void COptionsSubVideo::OnTextChanged(Panel *pPanel, const char *pszText)
{
	if (pPanel == m_pMode)
	{
		m_nSelectedMode = m_pMode->GetActiveItem();

		int w = 0, h = 0;
		sscanf(pszText, "%i x %i", &w, &h);

		if (m_CurrentSettings.w != w || m_CurrentSettings.h != h)
			OnDataChanged();
	}
	else if (pPanel == m_pAspectRatio)
	{
		PrepareResolutionList();
	}
	else if (pPanel == m_pWindowed)
	{
		OnDataChanged();
	}
}

void COptionsSubVideo::OnDataChanged(void)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

bool COptionsSubVideo::RequiresRestart(void)
{
	if (m_hOptionsSubVideoAdvancedDlg.Get() && m_hOptionsSubVideoAdvancedDlg->RequiresRestart())
		return true;

	return m_bRequireRestart;
}

void COptionsSubVideo::OpenAdvanced(void)
{
	if (!m_hOptionsSubVideoAdvancedDlg.Get())
		m_hOptionsSubVideoAdvancedDlg = new COptionsSubVideoAdvancedDlg(GetParent());

	m_hOptionsSubVideoAdvancedDlg->Activate();
}