#include "hud.h"
#include "cl_util.h"
#include "configs.h"

void Game_HookEvents(void);

void EV_HookEvents(void)
{
	Game_HookEvents();
}

void EVS_About(void);
void EVS_List(void);
void EVS_Compile(void);
void EVS_CompileAllScript(void);

void EV_Init(void)
{
	EV_HookEvents();

	gEngfuncs.pfnAddCommand("evs_about", EVS_About);
	gEngfuncs.pfnAddCommand("evs_list", EVS_List);
	gEngfuncs.pfnAddCommand("evs_compile", EVS_Compile);
	gEngfuncs.pfnAddCommand("evs_compileall", EVS_CompileAllScript);
}