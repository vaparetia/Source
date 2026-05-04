/*
        brk_paper.h
	紙 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_paper.h,v 1.1.1.3 2002/11/19 11:45:39 Yoshizawa1 Exp $
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

#define BRK_PAP_N_PAPER 6
#define BRK_PAP_N_PIECE 20
#define BRK_PAP_N_DUST  12

#define BRK_VEL_R  0.85f
#define BRK_VEL_B  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE      1.68f
#define BRK_BOUNCE_WALL 1.38f

#define BRK_DUST_SPHERE 40.0f
#define BRK_PAPER_RAD 48.0f*2.0f

#define BRK_PAP_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4

typedef struct move_t     MOVE  ;
typedef struct piece_t    PIECE ;
typedef struct paper_t    PAPER ;
typedef struct piled_up_t PILED ;
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
    MOVE     mov    ;
    int      flag   ;
    DG_COMDL_POS *pos ;/* 塵モデル位置情報 */
} ;

struct paper_t
{
    MOVE     mov    ;
    int      flag  ;
    DG_OBJS *objs  ;/* 一枚紙モデル */

    FMATRIX  lights[2] ;
} ;

struct piled_up_t
{
    TARGET       target ;
    POWER_TARGET power  ;
    FMATRIX      lights[2] ;

    DG_OBJS     *objs   ;/* 積み上がっている紙 */
    Work        *work   ;
} ;

struct work_t
{
    GV_ACT    actor   ;

    int       name    ;
    int       where   ;
    int       proc_id ; /*壊れプロック*/

    DG_PRIM2  *dust   ; /* 埃プリミティブ */
    FVECTOR    dust_v[BRK_PAP_N_DUST] ;
    int        n_dust ;

    DG_COMDL *piece_s ;
    DG_COMDL *piece_l ;

    PAPER     paper[BRK_PAP_N_PAPER] ;
    int       n_paper ;
    PIECE     piece[BRK_PAP_N_PIECE] ;
    int       n_piece ;
    PILED    *piled   ;
    int       n_piled ;
    DG_DEF   *m_def   ;/* 本体モデル       */
    DG_DEF   *p_def   ;/* 壊れパーツモデル */
    int       flag    ;

    LIT_DEF  *lit     ;

    HZD_BOX  *hzd     ;
    ENEFIND   ene_find ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* brk_pto_act.c */
extern void BRK_PAP_StartActPaper( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_PAP_ActPaper( Work *work ) ;
extern void BRK_PAP_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_PAP_ActPiece( Work *work ) ;
extern void BRK_PAP_ActDust( Work *work ) ;
extern void BRK_PAP_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_PAP_ActPiled( Work *work ) ;

/* brk_pto_clb.c */
extern void BRK_PAP_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_pto_msg.c */
extern void BRK_PAP_ReceiveMessage( Work *work ) ;
extern void BRK_PAP_AlreadyMessy( Work *work ) ;

/* brk_pto_ini.c */
extern void BRK_PAP_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_PAP_GetOptions( Work *, int map ) ;
extern int  BRK_PAP_InitHazard( Work * ) ;
extern int  BRK_PAP_InitTarget( PILED *pap, int map ) ;
extern int  BRK_PAP_InitPaper( Work *work, int where ) ;
extern int  BRK_PAP_InitPiece( Work *work, int where ) ;
extern int  BRK_PAP_InitPrimitive( Work *work ) ;
extern void BRK_PAP_InitEneFind( Work *work ) ;
extern void BRK_PAP_InitBroken( Work *work ) ;

static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, in, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}

