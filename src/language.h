#ifndef LANG_H
#define LANG_H
#ifdef _WIN32
#pragma once
#endif

enum ELanguage
{
	k_Lang_None = -1,
	k_Lang_English = 0,
	k_Lang_German,
	k_Lang_French,
	k_Lang_Italian,
	k_Lang_Korean,
	k_Lang_Spanish,
	k_Lang_Simplified_Chinese,
	k_Lang_Traditional_Chinese,
	k_Lang_Russian,
	k_Lang_MAX
};

ELanguage PchLanguageToELanguage(const char *pchShortName);
const char *GetLanguageShortName(ELanguage eLang);
const char *GetLanguageVGUILocalization(ELanguage eLang);
const char *GetLanguageName(ELanguage eLang);

#endif