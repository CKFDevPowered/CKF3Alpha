#include <custom.h>

void CL_ClearCaches(void);
int CL_IsThirdPerson(void);
void CL_CameraOffset(float *ofs);
void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
void CL_QueueHTTPDownload(const char *filename);
int CL_FindModelIndex(const char *pmodel);
struct model_s *CL_GetModelByIndex(int index);
void CL_AddToResourceList(resource_t *pResource, resource_t *pList);
void CL_FireEvents(void);
int CL_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);

void CL_Init(void);
void CL_VidInit(void);
void CL_Frame(void);
void CL_Shutdown(void);