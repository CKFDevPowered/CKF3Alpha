//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "pch_tier0.h"

#include "tier0/valve_off.h"
#ifdef _X360
#include "xbox/xbox_console.h"
#include "xbox/xbox_vxconsole.h"
#elif defined( _WIN32 )
#include <windows.h>
#elif _LINUX
char *GetCommandLine();
#endif
#include "resource.h"
#include "tier0/valve_on.h"
#include "tier0/threadtools.h"

// -------------------------------------------------------------------------------- //
// Interface functions.
// -------------------------------------------------------------------------------- //


DBG_INTERFACE bool ShouldUseNewAssertDialog()
{
	return false;
}


DBG_INTERFACE bool DoNewAssertDialog( const tchar *pFilename, int line, const tchar *pExpression )
{
	return false;
}