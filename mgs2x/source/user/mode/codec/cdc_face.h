#ifndef _cdc_face_h_
#define _cdc_face_h_

#define EYES_BLANK_MIN    1200    /* 目パチの最低間隔       */
#define EYES_BLANK_RND     800    /* 目パチのランダム性の幅 */

#define EYES_CLOSE_SPEED  15   /* 閉じスピード */
#define EYES_OPEN_SPEED   23   /* 開きスピード */

#define EYES_STAT_OPEN    0
#define EYES_STAT_CLOSING 1
#define EYES_STAT_CLOSED  2
#define EYES_STAT_OPENING 3


#define CODEC_FACE_LEFT  0
#define CODEC_FACE_RIGHT 1

#define CDC_FACE_RECT_L  0x009d3c5b   /* 左側矩形 */
#define CDC_FACE_RECT_R  0x009d3c61   /* 右側矩形 */
#define CDC_LAYOUT_ROOT  0x002a4634   /* 基準位置 */


/* #define BASE 0 */
#define BASE 2


/* VGA スケールから、表示画面スケールへの変換マクロ */
/*
  #define xV2S(x)  (((x) * DRAW_WIDTH) / 640)
  #define yV2S(y)  (((y) * DRAW_HEIGHT) / 480)
*/
/* VGA スケールから、スプライト座標系スケールへの変換マクロ */
#define xV2SP(x)   ((x) * 512.0F / 640.0F)
#define yV2SP(y)   ((y) * 384.0F / 480.0F)

#define FACE_SMALL

#define FACE_WIDTH      xV2S(180)
#define FACE_HEIGHT     yV2S(250)

/* 顔表示領域の高さ */
#define FACE_POS_Y  yV2S(CODEC_PARTS_LEVEL + 2)

#define OUTFRAME_WIDTH  xV2SP(180)
#define OUTFRAME_HEIGHT yV2SP(180)


int  codecGetMotionStatus(int side);
void codecEyeAnimationSwitch(int side, int sw);
void codecSetEyeCloseMotion(int side, int name);
void codecCameraControlSwitch(int side, int sw);

void codecSetupHair(int side, void * model, int sample_num,
		    void * bound, int hit_param, int ambient, int nowind);
void codecSetupBugHair(int side,
		       void * model, int sample_num,
		       void * bound, int hit_param, int ambient, int nowind);
void codecBugEyesSwitch(int side, int sw);
void   codecSetupScreen(void);
void   codecRecoveryScreen(void);
void * NewCodecFace(int name, int dummy_bone,
		    int side, int model_id, int speaker_id);
void * codecSetupFace(int name, int side);
void   codecFaceDisp(int side, int sw, int fade);
void   codecFaceChange(int side, int model_id, int speaker_id);
void   codecSetFaceAnimation(void *work, int motions[7], int eyes);
int    codecSetFaceLightVector(int side, float x, float y, float z);
int    codecSetFaceLightColor(int side, int r, int g, int b);
int    codecSetFaceAmbient(int side, int r, int g, int b);
void   codecSetDefaultMotion(int side, int motion);

int    codecFaceIsClosed(void * work);
void   codecCloseFace(void * workp);
int    cdcFaceSetObject(int side, int model, int spk_id, int skel_num);
int cdcFaceCamera(int side,
		  float far, float zoom,
		  float heading, float pan,
		  float pitch,   float gain);

int    cdcFaceVoiceCancel(int side);

#endif /* _cdc_face_h_ */

