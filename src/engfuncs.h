extern cl_enginefunc_t *g_pEngfuncs;

void GetEngfuncs(void);
struct model_s *LoadMapSprite(const char *filename);
void HookEvent(char *name, void (*pfnEvent)(struct event_args_s *args));
int CheckParm(char *parm, char **ppnext);
int GetScreenInfo(SCREENINFO *pscrinfo);
void GetPlayerInfo(int ent_num, hud_player_info_t *pinfo);
void SetCrosshair(HSPRITE hspr, wrect_t rc, int r, int g, int b);