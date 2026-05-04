/*
        brk_object.h
	ガラス破壊用 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_roll.h,v 1.1.1.3 2002/11/19 11:45:44 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for DEBUG
#include "../include/util.h"

// for BOX hzd
#include "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"

#define BRK_DUST_GRAVITY 1
#define BRK_GRAVITY      6.0f
#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f
#define BRK_HZX_SPHERE   12.0f
#define BRK_ROLL_RAD    (BRK_HZX_SPHERE*2.0f)

#define BRK_ROL_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4
#define BRK_N_DUST  10


typedef struct move_t   MOVE   ;
typedef struct rol_t    ROLL ;
typedef struct work_t   Work   ;

struct move_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_vx ;
    short    rot_y,  rot_vy ;
    short    roll ,  roll_v ;
    short    limit[3] ;
} ;

struct rol_t
{
    MOVE         mov  ;
    int          flag ;

    DG_OBJS     *objs ;/* 氷モデル */
    Work        *work ;

    FMATRIX      lights[2] ;

    TARGET       target ;
    POWER_TARGET power  ;
} ;

struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       where  ;

    ROLL     *roll   ;
    int       n_roll ;

    HZD_BOX   *hzd   ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

/* brk_roll.c */
extern void BRK_ROL_FreeRoll( Work *work ) ;

/* brk_rol_act.c */
extern void BRK_ROL_ActRoll( Work *work ) ;

/* brk_rol_ini.c */
extern int  BRK_ROL_GetOptions( Work *, int where ) ;
extern int  BRK_ROL_InitHazard( Work * ) ;


static inline void RotateMatrixYXY( FMATRIX *out, FMATRIX *in, short roll, short rot_x, short rot_y )
{
    int r ;

    r = roll  & 0x0fff ;
    _sceVu0RotMatrixY( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0AddVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W], pos ) ;
    mtx->m[W][W] = 1.0f ;
}
