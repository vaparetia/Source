#ifndef _blend_h_
#define _blend_h_

#include "gte.h"
#include "quat.h"
#include "motion.h"

#include "vmotion.h"
#include "scedule.h"

/* =========================================================================
 *                                Macro
 * ========================================================================= */
#define SEC_VIDEO  60   /* 一秒あたりの画面フレーム数     */
#define SEC_VOICE 100   /* 一秒あたりの音声分析フレーム数 */


/* =========================================================================
 *                                Type define
 * ========================================================================= */

/* 1フレーム分の顔モーション */
typedef struct _face_motion {
  FVECTOR  rots[ FACE_JOINTS ];
  FVECTOR  trans[ FACE_JOINTS ];
} FACE_MOTION;


/* =========================================================================
 *                              Global variable
 * ========================================================================= */
#ifdef _blend_c_
#define EXT
#define INIT(n) = n
#else
#define EXT extern
#define INIT(n)
#endif /* _blend_c_ */




#undef EXT
#undef INIT(n)

/* =========================================================================
 *                                Prototype
 * ========================================================================= */

MTN_MOTION * bldCreateMotion(scScedule **sc_list, vmtnVowel *vowel);

#endif /* _blend_h_ */
