#ifndef INEWSURFACE_H
#define INEWSURFACE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/ISurface.h>

namespace vgui
{
class INewSurface : public ISurface
{
public:
	virtual void SetLanguage(const char *pchLang) = 0;
	virtual const char *GetLanguage(void) = 0;
	virtual void DeleteTextureByID(int id) = 0;
	virtual void DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall) = 0;
	virtual void DrawSetTextureBGRA(int id, const unsigned char *rgba, int wide, int tall) = 0;
	virtual void CreateBrowser(VPANEL panel, IHTMLResponses *pBrowser, bool bPopupWindow, const char *pchUserAgentIdentifier) = 0;
	virtual void RemoveBrowser(VPANEL panel, IHTMLResponses *pBrowser) = 0;
	virtual IHTMLChromeController *AccessChromeHTMLController(void) = 0;
};
}

#endif