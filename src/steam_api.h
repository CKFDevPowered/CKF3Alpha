#ifndef STEAM_API_H
#define STEAM_API_H
#ifdef _WIN32
#pragma once
#endif

#define STEAM_API_NODLL

#include "isteamclient.h"
#include "isteamuser.h"
#include "isteamfriends.h"
#include "isteamutils.h"
#include "isteammatchmaking.h"
#include "isteamuserstats.h"
#include "isteamapps.h"
#include "isteamnetworking.h"
#include "isteamremotestorage.h"
#include "isteamscreenshots.h"
#include "isteamhttp.h"
#include "isteamunifiedmessages.h"
#include "isteamcontroller.h"

#if defined (_PS3)
#include "steamps3params.h"
#endif

#if defined (_WIN32) && !defined (_X360)
	#if defined (STEAM_API_EXPORTS)
	#define S_API extern "C" __declspec(dllexport)
	#elif defined (STEAM_API_NODLL)
	#define S_API extern "C"
	#else
	#define S_API extern "C" __declspec(dllimport)
	#endif
#elif defined (GNUC)
	#if defined (STEAM_API_EXPORTS)
	#define S_API extern "C" __attribute__ ((visibility("default")))
	#else
	#define S_API extern "C"
	#endif
#else
	#if defined (STEAM_API_EXPORTS)
	#define S_API extern "C"
	#else
	#define S_API extern "C"
	#endif
#endif

S_API void SteamAPI_Shutdown(void);
S_API bool SteamAPI_IsSteamRunning(void);
S_API bool SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID);
S_API void SteamAPI_WriteMiniDump(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID);
S_API void SteamAPI_SetMiniDumpComment(const char *pchMsg);

S_API ISteamClient *SteamClient(void);

#ifdef VERSION_SAFE_STEAM_API_INTERFACES
S_API bool SteamAPI_InitSafe(void);
#else

#if defined(_PS3)
S_API bool SteamAPI_Init(SteamPS3Params_t *pParams);
#else
S_API bool SteamAPI_Init(void);
#endif

S_API ISteamUser *SteamUser(void);
S_API ISteamFriends *SteamFriends(void);
S_API ISteamUtils *SteamUtils(void);
S_API ISteamMatchmaking *SteamMatchmaking(void);
S_API ISteamUserStats *SteamUserStats(void);
S_API ISteamApps *SteamApps(void);
S_API ISteamNetworking *SteamNetworking(void);
S_API ISteamMatchmakingServers *SteamMatchmakingServers(void);
S_API ISteamRemoteStorage *SteamRemoteStorage(void);
S_API ISteamScreenshots *SteamScreenshots(void);
S_API ISteamHTTP *SteamHTTP(void);
S_API ISteamUnifiedMessages *SteamUnifiedMessages(void);
#ifdef _PS3
S_API ISteamPS3OverlayRender * SteamPS3OverlayRender(void);
#endif
#endif

S_API void SteamAPI_RunCallbacks(void);
S_API void SteamAPI_RegisterCallback(class CCallbackBase *pCallback, int iCallback);
S_API void SteamAPI_UnregisterCallback(class CCallbackBase *pCallback);
S_API void SteamAPI_RegisterCallResult(class CCallbackBase *pCallback, SteamAPICall_t hAPICall);
S_API void SteamAPI_UnregisterCallResult(class CCallbackBase *pCallback, SteamAPICall_t hAPICall);

class CCallbackBase
{
public:
	CCallbackBase(void) { m_nCallbackFlags = 0; m_iCallback = 0; }

	virtual void Run(void *pvParam) = 0;
	virtual void Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;
	int GetICallback(void) { return m_iCallback; }
	virtual int GetCallbackSizeBytes(void) = 0;

protected:
	enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };
	uint8 m_nCallbackFlags;
	int m_iCallback;
	friend class CCallbackMgr;
};

template <class T, class P>
class CCallResult : private CCallbackBase
{
public:
	typedef void (T::*func_t)(P *, bool);

	CCallResult(void)
	{
		m_hAPICall = k_uAPICallInvalid;
		m_pObj = NULL;
		m_Func = NULL;
		m_iCallback = P::k_iCallback;
	}

	void Set(SteamAPICall_t hAPICall, T *p, func_t func)
	{
		if (m_hAPICall)
			SteamAPI_UnregisterCallResult(this, m_hAPICall);

		m_hAPICall = hAPICall;
		m_pObj = p;
		m_Func = func;

		if (hAPICall)
			SteamAPI_RegisterCallResult(this, hAPICall);
	}

	bool IsActive(void) const
	{
		return (m_hAPICall != k_uAPICallInvalid);
	}

	void Cancel(void)
	{
		if (m_hAPICall != k_uAPICallInvalid)
		{
			SteamAPI_UnregisterCallResult(this, m_hAPICall);
			m_hAPICall = k_uAPICallInvalid;
		}
	}

	~CCallResult(void)
	{
		Cancel(void);
	}

	void SetGameserverFlag(void) { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

private:
	virtual void Run(void *pvParam)
	{
		m_hAPICall = k_uAPICallInvalid;
		(m_pObj->*m_Func)((P *)pvParam, false);
	}

	void Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall)
	{
		if (hSteamAPICall == m_hAPICall)
		{
			m_hAPICall = k_uAPICallInvalid;
			(m_pObj->*m_Func)((P *)pvParam, bIOFailure);
		}
	}

	int GetCallbackSizeBytes(void)
	{
		return sizeof(P);
	}

	SteamAPICall_t m_hAPICall;
	T *m_pObj;
	func_t m_Func;
};

template <class T, class P, bool bGameServer>
class CCallback : protected CCallbackBase
{
public:
	typedef void (T::*func_t)(P *);

	CCallback(T *pObj, func_t func) : m_pObj(pObj), m_Func(func)
	{
		if (pObj && func)
			Register(pObj, func);
	}

	~CCallback(void)
	{
		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();
	}

	void Register(T *pObj, func_t func)
	{
		if (!pObj || !func)
			return;

		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();

		if (bGameServer)
		{
			m_nCallbackFlags |= k_ECallbackFlagsGameServer;
		}

		m_pObj = pObj;
		m_Func = func;

		SteamAPI_RegisterCallback(this, P::k_iCallback);
	}

	void Unregister(void)
	{
		SteamAPI_UnregisterCallback(this);
	}

	void SetGameserverFlag(void) { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

protected:
	virtual void Run(void *pvParam)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	virtual void Run(void *pvParam, bool, SteamAPICall_t)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	int GetCallbackSizeBytes(void)
	{
		return sizeof( P );
	}

	T *m_pObj;
	func_t m_Func;
};

template <class T, class P, bool bGameServer>
class CCallbackManual : public CCallback<T, P, bGameServer>
{
public:
	CCallbackManual(void) : CCallback<T, P, bGameServer>(NULL, NULL) {}
};

#define STEAM_CALLBACK(thisclass, func, param, var) CCallback<thisclass, param, false> var; void func(param *pParam)
#define STEAM_CALLBACK_MANUAL(thisclass, func, param, var) CCallbackManual<thisclass, param, false> var; void func(param *pParam)

#ifdef _WIN32
#pragma warning(disable: 4355)
#endif

S_API void Steam_RunCallbacks(HSteamPipe hSteamPipe, bool bGameServerCallbacks);
S_API void Steam_RegisterInterfaceFuncs(void *hModule);
S_API HSteamUser Steam_GetHSteamUserCurrent(void);
S_API const char *SteamAPI_GetSteamInstallPath(void);
S_API HSteamPipe SteamAPI_GetHSteamPipe(void);
S_API void SteamAPI_SetTryCatchCallbacks(bool bTryCatchCallbacks);
S_API HSteamPipe GetHSteamPipe(void);
S_API HSteamUser GetHSteamUser(void);

#ifdef VERSION_SAFE_STEAM_API_INTERFACES
S_API HSteamUser SteamAPI_GetHSteamUser(void);

class CSteamAPIContext
{
public:
	CSteamAPIContext(void);

	void Clear(void);
	bool Init(void);

	ISteamUser *SteamUser(void) { return m_pSteamUser; }
	ISteamFriends *SteamFriends(void) { return m_pSteamFriends; }
	ISteamUtils *SteamUtils(void) { return m_pSteamUtils; }
	ISteamMatchmaking *SteamMatchmaking(void) { return m_pSteamMatchmaking; }
	ISteamUserStats *SteamUserStats(void) { return m_pSteamUserStats; }
	ISteamApps *SteamApps(void) { return m_pSteamApps; }
	ISteamMatchmakingServers *SteamMatchmakingServers(void) { return m_pSteamMatchmakingServers; }
	ISteamNetworking *SteamNetworking(void) { return m_pSteamNetworking; }
	ISteamRemoteStorage *SteamRemoteStorage(void) { return m_pSteamRemoteStorage; }
	ISteamScreenshots *SteamScreenshots(void) { return m_pSteamScreenshots; }
	ISteamHTTP *SteamHTTP(void) { return m_pSteamHTTP; }
	ISteamUnifiedMessages *SteamUnifiedMessages(void) { return m_pSteamUnifiedMessages; }
#ifdef _PS3
	ISteamPS3OverlayRender *SteamPS3OverlayRender(void) { return m_pSteamPS3OverlayRender; }
#endif

private:
	ISteamUser *m_pSteamUser;
	ISteamFriends *m_pSteamFriends;
	ISteamUtils *m_pSteamUtils;
	ISteamMatchmaking *m_pSteamMatchmaking;
	ISteamUserStats *m_pSteamUserStats;
	ISteamApps *m_pSteamApps;
	ISteamMatchmakingServers *m_pSteamMatchmakingServers;
	ISteamNetworking *m_pSteamNetworking;
	ISteamRemoteStorage *m_pSteamRemoteStorage;
	ISteamScreenshots *m_pSteamScreenshots;
	ISteamHTTP *m_pSteamHTTP;
	ISteamUnifiedMessages *m_pSteamUnifiedMessages;
	ISteamController *m_pController;
#ifdef _PS3
	ISteamPS3OverlayRender *m_pSteamPS3OverlayRender;
#endif
};

inline CSteamAPIContext::CSteamAPIContext(void)
{
	Clear(void);
}

inline void CSteamAPIContext::Clear(void)
{
	m_pSteamUser = NULL;
	m_pSteamFriends = NULL;
	m_pSteamUtils = NULL;
	m_pSteamMatchmaking = NULL;
	m_pSteamUserStats = NULL;
	m_pSteamApps = NULL;
	m_pSteamMatchmakingServers = NULL;
	m_pSteamNetworking = NULL;
	m_pSteamRemoteStorage = NULL;
	m_pSteamHTTP = NULL;
	m_pSteamScreenshots = NULL;
	m_pSteamUnifiedMessages = NULL;
#ifdef _PS3
	m_pSteamPS3OverlayRender = NULL;
#endif
}

inline bool CSteamAPIContext::Init(void)
{
	if (!SteamClient())
		return false;

	HSteamUser hSteamUser = SteamAPI_GetHSteamUser();
	HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe();

	m_pSteamUser = SteamClient()->GetISteamUser(hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION);

	if (!m_pSteamUser)
		return false;

	m_pSteamFriends = SteamClient()->GetISteamFriends(hSteamUser, hSteamPipe, STEAMFRIENDS_INTERFACE_VERSION);

	if (!m_pSteamFriends)
		return false;

	m_pSteamUtils = SteamClient()->GetISteamUtils(hSteamPipe, STEAMUTILS_INTERFACE_VERSION);

	if (!m_pSteamUtils)
		return false;

	m_pSteamMatchmaking = SteamClient()->GetISteamMatchmaking(hSteamUser, hSteamPipe, STEAMMATCHMAKING_INTERFACE_VERSION);

	if (!m_pSteamMatchmaking)
		return false;

	m_pSteamMatchmakingServers = SteamClient()->GetISteamMatchmakingServers(hSteamUser, hSteamPipe, STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION);

	if (!m_pSteamMatchmakingServers)
		return false;

	m_pSteamUserStats = SteamClient()->GetISteamUserStats(hSteamUser, hSteamPipe, STEAMUSERSTATS_INTERFACE_VERSION);

	if (!m_pSteamUserStats)
		return false;

	m_pSteamApps = SteamClient()->GetISteamApps(hSteamUser, hSteamPipe, STEAMAPPS_INTERFACE_VERSION);

	if (!m_pSteamApps)
		return false;

	m_pSteamNetworking = SteamClient()->GetISteamNetworking(hSteamUser, hSteamPipe, STEAMNETWORKING_INTERFACE_VERSION);

	if (!m_pSteamNetworking)
		return false;

	m_pSteamRemoteStorage = SteamClient()->GetISteamRemoteStorage(hSteamUser, hSteamPipe, STEAMREMOTESTORAGE_INTERFACE_VERSION);

	if (!m_pSteamRemoteStorage)
		return false;

	m_pSteamScreenshots = SteamClient()->GetISteamScreenshots(hSteamUser, hSteamPipe, STEAMSCREENSHOTS_INTERFACE_VERSION);

	if (!m_pSteamScreenshots)
		return false;

	m_pSteamHTTP = SteamClient()->GetISteamHTTP(hSteamUser, hSteamPipe, STEAMHTTP_INTERFACE_VERSION);

	if (!m_pSteamHTTP)
		return false;

	m_pSteamUnifiedMessages = SteamClient()->GetISteamUnifiedMessages(hSteamUser, hSteamPipe, STEAMUNIFIEDMESSAGES_INTERFACE_VERSION);

	if (!m_pSteamUnifiedMessages)
		return false;

#ifdef _PS3
	m_pSteamPS3OverlayRender = SteamClient()->GetISteamPS3OverlayRender(void);
#endif

	return true;
}

#endif

#if defined(USE_BREAKPAD_HANDLER) || defined(STEAM_API_EXPORTS)
S_API void SteamAPI_UseBreakpadCrashHandler(char const *pchVersion, char const *pchDate, char const *pchTime, bool bFullMemoryDumps, void *pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback);
S_API void SteamAPI_SetBreakpadAppID(uint32 unAppID);
#endif
#endif