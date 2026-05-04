#ifndef _mtn_blend_h_
#define _mtn_blend_h_

#include "mtnlib.h"

#define FACE_JOINTS  32
#define BODY_JOINTS  21

/* 1フレーム分の顔モーション */
typedef struct _face_motion {
  FVECTOR  rots[ FACE_JOINTS ];
  FVECTOR  trans[ FACE_JOINTS ];
} FACE_MOTION;

void SetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res );
void GetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res );
void BlendMotion(FACE_MOTION * res,
		 FACE_MOTION *m0, FACE_MOTION *m1, float rate);
void BlendBodyMotion(FACE_MOTION * res,
		     FACE_MOTION *m0, FACE_MOTION *m1, float rate);

#endif /* _mtn_blend_h_ */
