#ifndef _cdc_face_h_
#define _cdc_face_h_

#define CODEC_FACE_LEFT  0
#define CODEC_FACE_RIGHT 1

void codecSetupHair(int side, void * model, int sample_num,
		    void * bound, int hit_param, int ambient);
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

#endif /* _cdc_face_h_ */

