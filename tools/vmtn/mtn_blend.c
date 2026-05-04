#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _mtn_blend_c_
#include "mtn_blend.h"


/* -------------------------------------------------------------------------
 * 指定したフレームのモーションを設定
 * ------------------------------------------------------------------------- */
void SetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res )
{
  MTN_SetMotionData( motion, frame, NULL, res->trans, res->rots );
}

/* -------------------------------------------------------------------------
 * 指定したフレームのモーションを取得
 * ------------------------------------------------------------------------- */
void GetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res )
{
  MTN_GetMotionData( motion, frame, NULL, res->trans, res->rots );
}

/* -------------------------------------------------------------------------
 * ２つの顔モーションの中間モーションを生成
 * ------------------------------------------------------------------------- */
void BlendMotion( FACE_MOTION *res, FACE_MOTION *m0, FACE_MOTION *m1, float t )
{
  int i ;
  
  for(i = 0; i < FACE_JOINTS; i++)
    {
      /* 移動成分の補間処理 */
      GTE_InterpolateVector(&res->trans[ i ],
			    &m0->trans[ i ], &m1->trans[ i ], t );
      /* 回転成分の補間処理 */
      MT_QuatSlerp(&res->rots[ i ], &m0->rots[ i ], &m1->rots[ i ], t);
    }
}

/* -------------------------------------------------------------------------
 * ２つの全身モーションの中間モーションを生成
 * ------------------------------------------------------------------------- */
void BlendBodyMotion( FACE_MOTION *res, FACE_MOTION *m0, FACE_MOTION *m1, float t )
{
  int i ;

  for(i = 0; i < BODY_JOINTS; i++)
    {
      /* 移動成分の補間処理 */
      GTE_InterpolateVector(&res->trans[ i ],
			    &m0->trans[ i ], &m1->trans[ i ], t );
      /* 回転成分の補間処理 */
      MT_QuatSlerp(&res->rots[ i ], &m0->rots[ i ], &m1->rots[ i ], t);
    }
}
