/*
  brk_big_glass.h
  ガラス破壊用 定義ファイル
  
  1999/11/26 T.Morita
  $Id: brk_big_glass.h,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum
{
    BRK_BGLS_M_BREAKDOWN,
    BRK_BGLS_M_ALREADY_MESSY,
} ;

#define BRK_SPHERE    10.0f
#define BRK_GRAVITY   6.0f

#define BRK_N_PIECE 40

#define BRK_VEL_R   0.8f
#define BRK_ROT_R   16
#define BRK_BOUNCE  0.68f

#define BRK_COMDL_W   282
#define BRK_COMDL_H   287
#define BRK_COMDL_M   45
#define BRK_N_HAZARD  10

#define BRK_BGLS_N_SCAR 12
#define BRK_BGLS_SCAR_W 350.0f

#define BRK_SPEED 2
#define BRK_BGLS_N_MESSY 40

#define FTOI12(_f) ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define BRK_BGLS_INACTIVE 0x8000000

#define UNIT { { {1.0f,0.0f,0.0f,0.0f},\
                 {0.0f,1.0f,0.0f,0.0f},\
                 {0.0f,0.0f,1.0f,0.0f},\
                 {0.0f,0.0f,0.0f,1.0f} } }

typedef struct hzd_t
{
    int      flag   ;
    float    size[XYZ] ;
    FVECTOR  size_inv ;
    FVECTOR  center ;
    FMATRIX  world  ;
} HZD_BOX ;

typedef struct move_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
} MOVE ;

typedef struct parts_t
{
    DG_COMDL_POS *pos   ;
    MOVE          mov   ;
    int           flag  ;
} PART ;

typedef struct piece_t
{
    FMATRIX *world ;
    MOVE     mov   ;
    int      flag  ;
} PIECE ;

typedef struct work_t Work ;
struct work_t
{
    GV_ACT     actor   ;

    void     (*act)( Work* ) ;
    DG_PRIM2  *frac[2] ;
    int        n_frac  ;
    DG_OBJS   *objs    ;

    DG_COMDL  *piece_l ; /* 破片のコモデル */
    DG_COMDL  *piece_s ;
    PIECE     *piece ;
    int        n_piece ;

    int        life    ;
    int        life_acs ;
    int        se_time ;

    DG_DEF    *def     ; /* 壊れガラスのKMS（fractコマンドで生成されたKMS） */
    int        n_parts ;
    PART      *parts   ;
    int        n_comdl ; /* タイル式におかれる */
    DG_COMDL **comdl   ;

    MOVE      *move    ; /*移動物構造体*/
    int        name   ;
    int        where  ;
    int        pr_name; /*壊れプロック名*/
    int        n_hzd  ;
    HZD_BOX   *hzd    ;
    ENEFIND    ene_find ; /*  */
    TARGET       target ;
    POWER_TARGET power  ;
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
extern FMATRIX BRK_BGLS_MatrixTable[16][16] ;/* 回転計算済みマトリックス */

/* brk_big_glass.c */
extern void BRK_BGLS_FreeComdl( Work *work ) ;
extern void BRK_BGLS_FreePiece( Work *work ) ;

/* brk_b_gls_act.c */
extern void BRK_BGLS_ActPieces( Work *work, int, PIECE * ) ;
extern void BRK_BGLS_StartActPieces( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width ) ;
extern void BRK_BGLS_ActFracture( Work *work ) ;
extern void BRK_BGLS_ActFractureWeb( Work *work ) ;
extern void BRK_BGLS_ActBreakDown( Work *work ) ;
extern void BRK_BGLS_SendMatrixTable( Work *work ) ;

/* brk_b_gls_clb.c */
extern void BRK_BGLS_PutGlassWeb( Work *work, FVECTOR *hit ) ;
extern void BRK_BGLS_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_b_gls_ini.c */
extern int BRK_BGLS_GetOptions( Work *, int where ) ;
extern int BRK_BGLS_InitTarget( Work *, int where ) ;
extern int BRK_BGLS_InitHazard( Work *, int map ) ;
extern int BRK_BGLS_InitPrimitive( Work * ) ;
extern int BRK_BGLS_InitPart( Work *work ) ;
extern int BRK_BGLS_InitMatrixTable( Work *work ) ;
extern void BRK_BGLS_InitEneFind( Work *work ) ;
extern void BRK_BGLS_InitBroken( Work *work ) ;

/* brk_b_gls_hzd.c */
extern int BRK_BGLS_HzdCheck( Work *work, FVECTOR *pos, FVECTOR *pos_v, float bounce ) ;

/* brk_b_gls_msg.c */
extern void BRK_BGLS_ReceiveMessage( Work *work ) ;
extern int  BRK_BGLS_AlreadyMessy( Work *work ) ;


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
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
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
