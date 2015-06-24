#ifndef _CAMERA_H_
#define _CAMEA_H_

extern vec3_t cam_ofs;
extern int cam_thirdperson;

void CAM_Think(void);
void CAM_Init(void);
void CAM_ClearStates(void);
void CAM_StartMouseMove(void);
void CAM_EndMouseMove(void);

#endif