#include "metahook.h"
#include "cvar.h"
#include "vgui_int.h"

cvar_t *vgui_emulatemouse = NULL;
cvar_t *vgui_stenciltest = NULL;

void VGui_Startup(void)
{
	vgui_emulatemouse = Cvar_FindVar("vgui_emulatemouse");
}

void VGui_Shutdown(void)
{
}