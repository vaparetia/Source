/*
        brk_object.h
	ガラス破壊用 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_box.h,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/util.h"
#include "../include/libdg_x.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct {
        short  r, g, b, a ;
}RGBA ;



#define BRK_HZX_SPHERE   2000
#define BRK_HZX_R_SPHERE 200
#define BRK_DUST_GRAVITY 1
#define BRK_GRAVITY      6
#define BRK_MAX_MDL      4

#define BRK_N_PIECE 10
#define BRK_BOX_BOXVITALITY 3

#define BRK_BOUND_X  300000.0f
#define BRK_BOUND_Y  300000.0f
#define BRK_BOUND_Z  300000.0f

#define BRK_PVEL_R 16
#define BRK_PROT_R 16
#define BRK_VEL_R  8
#define BRK_ROT_R  16

#define BRK_BOUNCE 0.28f

#define BRK_BOX_BREATH_TIME 256
#define BRK_POS_HEAD   400
#define BRK_BRTH_RAD   (600.0f*600.0f)
#define BRK_POTATO_RAD 80.0f


enum brk_msg_t
{
    BRK_BOX_M_REGIST_FOOT = 0,
    BRK_BOX_M_GET_BOX_VITALITY,
    BRK_BOX_M_GET_POTATO_POS,
    BRK_BOX_M_GET_POTATO_NUM,
    BRK_BOX_M_GET_BOX_POS,
} ;

enum 
{
    BRK_FLG_NONE     = 0,
    BRK_FLG_FINISHED = 1,
    BRK_FLG_STICK,
} ;

typedef struct box_t  BOX  ;
typedef struct work_t Work ;

struct box_t
{
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */

    int    (*act)( Work*, BOX* ) ;

    HZX_SEG seg[2] ;
    HZX_FLR flr[2] ;

    DG_OBJS *objs  ;

    TARGET  *target ;
} ;

struct work_t
{
    GV_ACT  actor  ;

    int     name   ;
    BOX    *box    ;
    int     n_box  ;

    BOX    *foot   ;
    int     n_foot ;

    TARGET       target[10] ;
    POWER_TARGET power[10]  ;

    HZX_GROUP_ID hzx ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */




/* brk_box_act.c */
extern int  BRK_BOX_ActNone( Work *work, BOX *p ) ;
extern int  BRK_BOX_Act( Work *work, BOX *p ) ;
extern int  BRK_BOX_ActBreak( Work *work, BOX *p ) ;
extern int  BRK_BOX_ActOnFoot( Work *work, BOX *p ) ;

/* brk_box_box.c */
extern void BRK_BOX_ActBox( Work *work ) ;

/* brk_box_clb.c */
extern void BRK_BOX_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_BOX_PartsTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_BOX_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_box_msg.c */
extern void BRK_BOX_ReceiveMessage( Work *work ) ;

/* brk_box_ini.c */
extern int  BRK_BOX_InitBox( Work *work ) ;
extern int  BRK_BOX_GetOptions( Work * ) ;
extern int  BRK_BOX_InitHazard( Work *, int map ) ;
extern void BRK_BOX_InitPart( BOX *p, DG_MDL *mdl, DG_OBJ *obj, FVECTOR *pos,
			      void *act,
			      void (*callback)( TARGET *off, TARGET *def, void *ptr ) ) ;

/* brk_box_hzx.c */
extern void BRK_BOX_HzdCheck( Work *work, HZX_SEG *seg, HZX_FLR *flr, FVECTOR *pos ) ;
extern int  BRK_BOX_HzdSegLineCheck( Work *work, HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 ) ;
extern void BRK_BOX_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  ) ;
extern void BRK_BOX_HzdBoxCheck( Work *work, float rad ) ;


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
static inline void RotateMatrixXZ( FMATRIX *out, FMATRIX *in, short rot_x, short rot_z )
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

#if 0//BP
static inline void SwapVector( FVECTOR *a, FVECTOR *b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    sqc2        vf2, 0(%0)
    sqc2        vf1, 0(%1)
    " : : "r"(a), "r"(b) ) ;
}
static inline void AddVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vadd.xyzw   vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
}

static inline void SubVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vsub.xyz    vf1, vf1, vf2
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
}

static inline void MaxVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmax.xyz    vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
}
static inline void MinVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmini.xyz   vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
}
#endif
