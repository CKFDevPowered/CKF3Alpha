extern cvar_t *vgui_emulatemouse;
extern cvar_t *vgui_stenciltest;

void VGui_Startup(void);
void VGui_Shutdown(void);
void VGui_CallEngineSurfaceAppHandler(void *event, void *userData);
void VGui_ReleaseMouse(void);
void VGui_GetMouse(void);
void VGui_SetVisible(int state);