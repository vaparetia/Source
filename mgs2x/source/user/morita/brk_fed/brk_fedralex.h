/*
  brk_fedral.h
  雑誌破壊用 定義ファイル

  1999/06/20 T.Morita
  $Id: brk_fedralex.h,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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
#include "../brk_utl/brk_utl.x"

#define BRK_GRAVITY      2.0f
#define BRK_FED_GRAVITY  6.0f

#define BRK_FED_N_PIECE 16
#define BRK_FED_N_SCAR  6*2

#define BRK_FED_SPOOLTIME 10

#define BRK_VEL_R  0.85f
#define BRK_VEL_B  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE      1.68f
#define BRK_BOUNCE_WALL 1.38f

#define BRK_DUST_SPHERE 5.0f
#define BRK_FEDER_RAD 48.0f*2.0f

#define BRK_FED_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4

#define DEG2RAD(_s) ((_s)*(float)M_PI/180.0f)

typedef struct move_t     MOVE  ;
typedef struct piece_t    PIECE ;
typedef struct fedral_t FEDRAL ;
typedef struct work_t     Work  ;

struct move_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
} ;

struct piece_t
{
    MOVE          mov  ;
    int           flag ;
    DG_COMDL_POS *pos  ;/* 塵モデル位置情報 */
} ;

struct fedral_t
{
    MOVE          mov   ;
    int           flag  ;
    DG_OBJS      *objs  ; /* 現在の雑誌モデル */

    FMATRIX      lights[2] ;

    TARGET         target  ;
    POWER_TARGET   power   ;
    DG_VERTS_ANIME v_anime ;

    DG_PRIM2     *scar   ; /* 弾痕 */
    int           n_scar ; /* 弾痕 */

    Work        *work   ;
    int		fed_type ;
    float	y_offset;
} ;

struct work_t
{
    GV_ACT    actor   ;

    int       name    ;
    int       where   ;
    int       proc_id ; /* 壊れプロック   */

#if 0
    DG_PRIM2 *dust    ; /* 埃プリミティブ */
    FVECTOR   dust_v[BRK_FED_N_DUST] ;
    int       n_dust  ;
#endif

    DG_COMDL *piece_s  ;
    DG_COMDL *piece_l  ;
    PIECE     piece[BRK_FED_N_PIECE] ;
    int       n_piece  ;

    FEDRAL   *fedral   ;
    int       n_fedral ;
    DG_DEF   *p_def    ;/* 破片コモデル */
    int       flag     ;

    LIT_DEF  *lit      ;

    HZD_BOX  *hzd      ;
    ENEFIND   ene_find ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* brk_fedral.c */
extern void BRK_FED_FreeFedral( FEDRAL *p ) ;

/* brk_fed_act.c */
extern float BRK_MGS_GetCurrentSize( FEDRAL *fed, FMATRIX *m, FVECTOR *size, FVECTOR *cen ) ;
extern void BRK_FED_DispFedral( FEDRAL *m ) ;
extern void BRK_FED_ActFedral( Work *work ) ;
extern void BRK_FED_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_FED_ActPiece( Work *work ) ;
extern void BRK_FED_ActDust( Work *work ) ;
extern void BRK_FED_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;

/* brk_fed_clb.c */
extern void BRK_FED_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_fed_msg.c */
extern void BRK_FED_ReceiveMessage( Work *work ) ;
extern void BRK_FED_AlreadyMessy( Work *work ) ;

/* brk_fed_ini.c */
extern void BRK_FED_ChangeBook( FEDRAL *p ) ;
extern void BRK_FED_InitEneFind( Work *work ) ;
extern void BRK_FED_InitBroken( Work *work ) ;
extern void BRK_FED_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_FED_GetOptions( Work *, int where ) ;
extern int  BRK_FED_InitHazard( Work * ) ;
extern int  BRK_FED_InitPiece( Work *work, int where ) ;
extern int  BRK_FED_InitPrimitive( Work *work ) ;
extern int  BRK_FED_InitTarget( FEDRAL *fed, int where ) ;

extern void *NewTs_Min_Fog( FVECTOR *pos ); /*霧を出すキャラ*/


/* extern variable */
extern FVECTOR BRK_FED_Gravity ;
extern FVECTOR BRK_FED_Bounce  ;
extern FVECTOR BRK_FED_Size    ;


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
static inline void RotateMatrixZY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0AddVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W], pos ) ;
    mtx->m[W][W] = 1.0f ;
}

#if 0 //BP
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
static inline void ScaleVector( FVECTOR *r, FVECTOR *a, float b  )
{
    asm volatile ("
    lqc2        vf1, 0(%0)
    qmtc2       %1 , vf2
    vmulx.wxyz  vf1, vf1, vf2x
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
}

static inline void SVector2FVector( FVECTOR *a, SVECTOR *t )
{
    asm volatile( "
         ld           $8 ,0(%0)      /* load <vel[i]> to $8      */
         pextlh       $8 ,$8 ,$0     /* SVECTOR -> IVECTOR(16bitfix) */
         psraw        $8 ,$8 ,4      /* IVECTOR(16) -> IVECTOR(12bitfix) */
         qmtc2.ni     $8 ,vf1        /* send to cop2             */
         vitof12.xyz  vf1,vf1        /* IVECTOR(12) -> FVECTOR   */
         sqc2         vf1,0(%1)      /* store <pos[i]>           */
        " : : "r"(t), "r"(a) : "$8" ) ;
}

static inline void ApplyMatrixXYZ( FVECTOR *a, FMATRIX *m, FVECTOR *v )
{
    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    vmulax.xyz  ACC,vf2,vf1x
    vmadday.xyz ACC,vf3,vf1y
    vmaddz.xyz  vf2,vf4,vf1z
    sqc2        vf2,0x00(%2)
    " : : "r"(m), "r"(v), "r"(a) );
    
}

static inline void MinMaxVector( FVECTOR *min, FVECTOR *max, FVECTOR *a  )
{
    asm volatile ("
    lqc2        vf3, 0(%2)
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmini.xyz   vf1, vf1, vf3
    vmax.xyz    vf2, vf2, vf3
    sqc2        vf1, 0(%0)
    sqc2        vf2, 0(%1)
    " : : "r"(min), "r"(max), "r"(a) ) ;
}
#endif //BP