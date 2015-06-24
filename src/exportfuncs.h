#include <r_studioint.h>
#include <usercmd.h>

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void *ClientFactory(void);

void HUD_Init(void);
int HUD_VidInit(void);
int HUD_Redraw(float time, int intermission);
int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src);
void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
void HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
void HUD_DrawNormalTriangles(void);
void HUD_DrawTransparentTriangles(void);
int HUD_UpdateClientData(client_data_t *pcldata, float flTime);
void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);
void HUD_Frame(double time);
void HUD_PlayerMove(struct playermove_s *ppmove, int server);
char HUD_PlayerMoveTexture(char *name);
void HUD_Shutdown(void);
void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp));
int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
void HUD_CreateEntities(void);
void HUD_PlayerMoveInit(struct playermove_s *ppmove);
int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
int HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
void HUD_DirectorMessage(int iSize, void *pbuf);
int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding);
void HUD_VoiceStatus(int entindex, qboolean bTalking);
void HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client);