/*

	precamera.h
	デザイナープレビュー環境用：カメラ制御
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: precamera.h,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $

*/


#ifndef _precamera_h_
#define _precamera_h_


#define	CAM_DEFAULT_RX	(-512)
#define	CAM_DEFAULT_RY	(0)
#define	CAM_DEFAULT_RZ	(0)
#define	CAM_DEFAULT_ZM	(1.0f)
#define	CAM_DEFAULT_LEN	(8000)

#define	CAM_DEFAULT_TRG_X	(0.0f)
#define	CAM_DEFAULT_TRG_Y	(0.0f)
#define	CAM_DEFAULT_TRG_Z	(0.0f)
#define	CAM_DEFAULT_CAM_X	(0.0f)
#define	CAM_DEFAULT_CAM_Y	(8000.0f*0.7071f)
#define	CAM_DEFAULT_CAM_Z	(8000.0f*0.7071f)


typedef struct {
    int cam_disp_flag;

    int first_in_flag;
    int mode,mode2;
    int speed_mode;

    FVECTOR ftrg_pos;
    FVECTOR fcam_pos;
    float zoom;
    float cam_len;

    GM_CameraSet *camera;

    int tcontrol_index;

	FVECTOR pre_ctrl_mov;
} PreviewCamera_Param;


typedef struct {
    FVECTOR ftrg_pos;
    FVECTOR fcam_pos;
    float zoom;
} CameraSaveFormat;


#ifdef _WORK_DECLARED_
PreviewCamera_Param PreviewCamera;
#else
extern PreviewCamera_Param PreviewCamera;
#endif


void Preview_SetDefaultCameraData(void);
void Preview_GetNowCameraData(void);

void CopyCameraToSavework(void);
void CopyCameraFromSavework(void);

void Camera_DebugPrint(void);
void Camera_DebugCursole(void);

void Preview_InitCamera(void);
void Preview_CameraAct(void);
void *NewPreviewCamera(void);


#endif
