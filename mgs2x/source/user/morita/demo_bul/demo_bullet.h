/*
  demo_bullet.h
  デモ用銃弾

  2000/04/18 T.Morita
  $Id: demo_bullet.h,v 1.1.1.3 2002/11/19 11:45:55 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#include "../include/util.h"

#define DEMO_BUL_RED    128
#define DEMO_BUL_GREEN  128
#define DEMO_BUL_BLUE   128

#define DEMO_BUL_N_TRAIL (64*4)
#define DEMO_BUL_SIZE     4096

#define DEMO_BUL_AGE     1

typedef struct work_t
{
    GV_ACT_EX actor  ;

    FVECTOR   from   ; /* 弾丸位置     */
    FVECTOR   to     ; /* 弾丸目標     */
    FVECTOR   pos    ; /* 弾丸位置     */
    FVECTOR   pos_v  ; /* 弾丸速度     */
    FMATRIX   world  ; /* マトリックス */
    float     rot_y,  rot_vy ;/*弾丸ヨー回転速度*/

    float     speed  ;
    int       flag   ;

    DG_OBJS  *objs   ;
    DG_PRIM2 *trail  ;

    char      rot  [DEMO_BUL_N_TRAIL] ;
    char      size [DEMO_BUL_N_TRAIL] ;
    char      alpha[DEMO_BUL_N_TRAIL] ;
    FVECTOR   vel[DEMO_BUL_N_TRAIL] ;

} Work ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}

