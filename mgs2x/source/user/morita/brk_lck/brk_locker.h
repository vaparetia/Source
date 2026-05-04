/*
        brk_locker.h
	ロッカー壊れ用 定義ファイル
	
        1999/12/08 T.Morita
        $Id: brk_locker.h,v 1.1.1.3 2002/11/19 11:45:34 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/util.h"
#include "../include/libdg_x.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define BRK_MAX_FLOAT 600000000.0f
#define BRK_N_DBG_LN  1200


typedef struct poly_t
{
    short    flag ;
    short    i0,  i1,  i2 ;
    FVECTOR *v0, *v1, *v2 ;
    SVECTOR *n0, *n1, *n2 ;
    FVECTOR  nrm ;
} POLY ;

typedef struct door_t
{
    DG_VERTS_ANIME v_anm ;
    DG_MDL        *mdl   ;
    DG_OBJ        *obj   ;
    CV2_MDL       *cv2   ;
    FMATRIX       *world ;
    POLY          *poly  ;
    int            n_poly ;

    float          dir_x, dir_y, dir_z ; /*当たった時の変化割合*/

    POWER_TARGET   power ;
} DOOR ;

typedef struct work_t
{
    GV_ACT    actor   ;

    FMATRIX   world   ;
    DG_OBJS  *obj     ;
    CV2_DEF  *cv2     ;
    FVECTOR   sht_pos ;
    FVECTOR   sht_frc ;

    DOOR     *doors   ;
    TARGET   *child   ;
    int       n_doors ;

    DG_PRIM  *dbg     ;

    POLY     *poly    ;

    TARGET       target ;
    POWER_TARGET power  ;
} Work ;


extern FVECTOR dbg_shape[BRK_N_DBG_LN*2] ;

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

extern void BRK_LCK_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

extern int BRK_LCK_GetOptions( Work * ) ;
extern int BRK_LCK_InitTarget( Work * ) ;
extern int BRK_LCK_SetupPolydata( Work *work ) ;
extern int BRK_LCK_InitDbgPrimitive( Work *work ) ;

static inline void ZeroVector( FVECTOR *a )
{
    vu0_Clrv0() ;
    vu0_Stv0( a ) ;
}

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

static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
