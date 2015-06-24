#ifndef OPTIONS_SUB_VOICE_H
#define OPTIONS_SUB_VOICE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

typedef struct IVoiceTweak_s IVoiceTweak;

class CCvarSlider;
class CCvarToggleCheckButton;

class COptionsSubVoice : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSubVoice, vgui::PropertyPage);

public:
	COptionsSubVoice(vgui::Panel *parent);
	~COptionsSubVoice(void);

public:
	virtual void OnResetData(void);
	virtual void OnApplyChanges(void);

protected:
	virtual void OnThink(void);

private:
	virtual void OnCommand(const char *command);

private:
	MESSAGE_FUNC(OnPageHide, "PageHide");
	MESSAGE_FUNC_INT(OnSliderMoved, "SliderMoved", position);
	MESSAGE_FUNC_INT(OnCheckButtonChecked, "CheckButtonChecked", state);
	MESSAGE_FUNC(OnControlModified, "ControlModified");

private:
	void StartTestMicrophone(void);
	void EndTestMicrophone(void);
	void UseCurrentVoiceParameters(void);
	void ResetVoiceParameters(void);

private:
	IVoiceTweak *m_pVoiceTweak;
	vgui::CheckButton *m_pMicBoost;

	vgui::ImagePanel *m_pMicMeter;
	vgui::ImagePanel *m_pMicMeter2;
	vgui::Button *m_pTestMicrophoneButton;
	vgui::Label *m_pMicrophoneSliderLabel;
	vgui::Slider *m_pMicrophoneVolume;
	vgui::Label *m_pReceiveSliderLabel;
	CCvarSlider *m_pReceiveVolume;
	CCvarToggleCheckButton *m_pVoiceEnableCheckButton;

	int m_nMicVolumeValue;
	bool m_bMicBoostSelected;
	float m_fReceiveVolume;
	int m_nReceiveSliderValue;
	bool m_bVoiceOn;
};

#endif