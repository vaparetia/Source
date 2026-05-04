/*
   vanime_buller.h
   頂点アニメブラー

   2000/04/15 T. Morita
   $Id: vanime_buller.h,v 1.1.1.3 2002/11/19 11:46:37 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"

#define PERROR(_s...) ( { printf( (_s) ) ; return -1 ; } )



/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct holowork_t
{
    GV_ACT_EX actor ;

    FMATRIX  *root  ;
    FMATRIX   world ;

    FVECTOR   pos   ;
    FVECTOR   dst   ;
    FVECTOR   pos_v ;

    float     rot   ;
    float     rot_v ;

    int       flag  ;

    int       key   ; /* キーフレーム */
    int       n_key ;
    DG_OBJS  *objs  ;
    VERTEX_ANIME_WORK *anime ;
    int       base  ; /*ベースカウンタ*/
    int       rand  ; /*乱数幅        */
} Work ;

extern void *NewOpticalCamouflage( DG_OBJS *objs, int flag ) ;/* 光学迷彩効果 */
extern void AN_Test_Eye2( FVECTOR *, int ) ;

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
