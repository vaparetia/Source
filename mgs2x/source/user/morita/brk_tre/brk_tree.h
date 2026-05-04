/*
        brk_tree.h
	植物揺れ用 定義ファイル

        1999/12/17 T.Morita
        $Id: brk_tree.h,v 1.1.1.3 2002/11/19 11:45:49 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#define BRK_LEAF_GRAVITY 2
#define BRK_N_LEAVES     8
#define BRK_MAX_DIFF     (BRK_N_LEAVES/2)

#define BRK_BOUND_X  3000 
#define BRK_BOUND_Z  3000 

#define BRK_TRE_TENDENCY 16
#define BRK_TRE_INACTIVE 0x8000000


enum brk_mod_t
{
    BRK_MOD_STILL = 0,
    BRK_MOD_SHAKE,
    BRK_MOD_MODE   = 0x0ff,

    BRK_MOD_NORMAL = 0x000,
} ;

typedef struct parts_t PART ;
typedef struct leaf_t  LEAF ;
typedef struct work_t  Work ;

struct parts_t
{
    DG_OBJ  *obj  ;
    DG_MDL  *mdl  ;

    int    (*act)( Work*, PART* ) ;
    Work    *work ;

    short   rot_x , rot_z  ; /* 回転角度  except <rot->pad> is the starting frame*/
    short   rot_vx, rot_vz ; /* 角速度   */
} ;

struct work_t
{
    GV_ACT    actor   ;
    int       name    ;
    int       where   ;

    short     tend    ;
    short     front   ;
    short     n_pos   ;
    short     leaves[BRK_N_LEAVES]  ;
    DG_COMDL *leaf    ;

    DG_OBJS  *objs    ;
    LIT_DEF  *lit     ;

    int       proc_id ; /*壊れプロック*/

    int       n_parts ;
    PART     *parts   ;

    float     floor[2] ;
    TARGET       *target ;
    POWER_TARGET *power  ;
    HZX_GROUP_ID  hzx    ;
} ;

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

#if 0
static inline void dec_vel_and_add_to_pos( FVECTOR *pos, FVECTOR *vel, FVECTOR *res  )
{
    volatile asm ( "
    lqc2      vf1, 0(%2)
    lqc2      vf2, 0(%1)
    vsubi.xyz vf3, vf1, I 
    vmul.xyz  vf2, vf3, vf2
    vdiv      Q, vf2, vf1
    " : : "r"(pos), "r"(vel), "r"(res) : "$8" ) ;
}
#endif

/* brk_tre_clb.c */
extern void BRK_TRE_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_tre_ini.c */
extern int BRK_TRE_GetOptions( Work *, int where ) ;
extern int BRK_TRE_InitTarget( Work *, int map ) ;
extern int BRK_TRE_InitHazard( Work *, int map ) ;
extern void BRK_TRE_InitBroken( Work *work ) ;

/* brk_tre_act.c */
extern void BRK_TRE_MakePosition( Work *work, PART *p ) ;
extern int BRK_TRE_ActShaking( Work *work, PART *p ) ;
extern int BRK_TRE_ActFinished( Work *work, PART *p ) ;

/* brk_tre_msg.c */
extern void BRK_TRE_ReceiveMessage( Work *work ) ;
extern void BRK_TRE_AlreadyMessy( Work *work ) ;

extern void *NewLeafFly( DG_COMDL *leaf, DG_COMDL_POS *pos,
			 FVECTOR *i_pos, FVECTOR *i_dir,
			 short *life, HZX_GROUP_ID  hzx ) ;


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

static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y  )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void RotateMatrixXZ( FMATRIX *out, FMATRIX *in, short rot_x, short rot_z  )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_z & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}


static inline void SVecToFVec( FVECTOR *a, SVECTOR *x  )
{
	SVector2FVector( a, x ) ;
}

static inline float BackFaceCulling( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a ;

    _sceVu0OuterProduct( &a, v1, v2 ) ;
    return _sceVu0InnerProduct( &a, v3 ) ;
}

