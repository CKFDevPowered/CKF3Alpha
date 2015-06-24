#ifndef ICHROMEHTMLWRAPPER_H
#define ICHROMEHTMLWRAPPER_H

#ifdef _WIN32
#pragma once
#endif

#include "HTMLMessages.h"

class CUtlString;
class IHTMLResponses;
class IHTMLResponses;
struct HTMLCommandBuffer_t;

class IHTMLChromeController
{
public:
	virtual ~IHTMLChromeController(void) {}

public:
	virtual bool Init(const char *pchHTMLCacheDir, const char *pchCookiePath) = 0;
	virtual void Shutdown(void) = 0;
	virtual bool RunFrame(void) = 0;

	virtual void SetWebCookie(const char *pchHostname, const char *pchKey, const char *pchValue, const char *pchPath, uint32 nExpires = 0) = 0;
	virtual void GetWebCookiesForURL(CUtlString *pstrValue, const char *pchURL, const char *pchName) = 0;

	virtual void SetClientBuildID(uint64 ulBuildID) = 0;

	virtual bool BHasPendingMessages(void) = 0;

	virtual void CreateBrowser(IHTMLResponses *pBrowser, bool bPopupWindow, const char *pchUserAgentIdentifier) = 0;
	virtual void RemoveBrowser(IHTMLResponses *pBrowser) = 0;

	virtual void WakeThread(void) = 0;
	virtual HTMLCommandBuffer_t *GetFreeCommandBuffer(EHTMLCommands eCmd, int iBrowser) = 0;
	virtual void PushCommand(HTMLCommandBuffer_t *) = 0;

	virtual void SetCefThreadTargetFrameRate(uint32 nFPS) = 0;

	virtual IHTMLSerializer *CreateSerializer(IHTMLResponses *pResponseTarget) = 0;
};

#define CHROMEHTML_CONTROLLER_INTERFACE_VERSION "ChromeHTML_Controller_001"

#endif