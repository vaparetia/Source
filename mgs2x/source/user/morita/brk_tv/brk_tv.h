/*
        brk_radle.h
	レードル揺れ用 定義ファイル

        1999/12/13 T.Morita
        $Id: brk_tv.h,v 1.1.1.3 2002/11/19 11:45:49 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define FTOI12(_f) ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define BRK_TV_SCREEN   "tv_00c"
#define BRK_TV_SPECULAR "tv_00a"
#define BRK_TV_WHITE    "tv_white"
#define BRK_TV_WIPE     "tv_black_sub_alp"

#define BRK_GRAVITY 6

#define BRK_TV_WIPE_SPEED   (24.0f)

typedef struct work_t
{
    GV_ACT        actor  ;
    int           name   ;

    DG_DEF       *def    ;
    DG_PRIM2     *scn_al ;
    DG_PRIM2     *scn_iv ;
    DG_PRIM2     *inv    ;
    DG_PRIM2     *spec   ;

    DG_PRIM2     *wipe   ;
    int           count  ;

    FVECTOR       hit     ;
    float         scroll  ;
    float         speed   ;
    int           proc_id ; /*壊れプロック*/

    TARGET        target ;
    POWER_TARGET  power  ;
} Work ;

extern int BRK_TV_InitTarget( Work *work, DG_DEF *def, int where ) ;
extern int BRK_TV_GetOptions( Work *work, int name, int where ) ;
extern int BRK_TV_InitPrimitive( DG_PRIM2 **prim, DG_TEX *t, int i, int alpha,
				 float raise, float *uvs,
				 float lz, float uz, float ly, float uy, float lx ) ;

extern void BRK_TV_ActMoveImage( Work *work ) ;
extern void BRK_TV_ActBreak( Work *work ) ;
extern void BRK_TV_ActWipeOut( Work *work ) ;

extern void BRK_TV_ReceiveMessage( Work *work ) ;


static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
