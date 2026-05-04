/*
        brk_object.h
	ガラス破壊用 定義ファイル

        1999/11/26 T.Morita
	2000/10/16 1.24 T.Morita 
        $Id: brk_potato.h,v 1.1.1.3 2002/11/19 11:45:41 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define MAKING 0

//for DEBUG
#include "../include/util.h"

// for BOX hzd
#include "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"

#include "bp_math.h"//BP_ASM

#define BRK_DUST_GRAVITY 1
#define BRK_GRAVITY      6.0f

#define BRK_N_PIECE 10
#define BRK_PTO_BOXVITALITY 3

#define BRK_BOUND_X  300000.0f
#define BRK_BOUND_Y  300000.0f
#define BRK_BOUND_Z  300000.0f

#define BRK_PVEL_R 0.8f
#define BRK_PROT_R 16
#define BRK_VEL_R  0.85f
#define BRK_VEL_B  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE      1.48f
#define BRK_BOUNCE_WALL 1.38f

#define BRK_PTO_BREATH_TIME 256
#define BRK_POS_HEAD   400
#define BRK_BRTH_RAD   (600.0f*600.0f)
#define BRK_POTATO_RAD (48.0f*2.0f)

#define BRK_HZX_SPHERE   (BRK_POTATO_RAD/2.0f)
#define BRK_HZX_S_SPHERE 500

#define BRK_PTO_INACTIVE 0x8000000

#define BRK_N_POLY_VERTS 4
#define BRK_N_PRIM  5
#define BRK_N_VERTS (BRK_N_POLY_VERTS * 4)
#define BRK_N_SCAR  (BRK_N_PRIM * BRK_N_VERTS / 4)

#define BRK_PTO_N_NEEDLE 4

enum brk_msg_t
{
    BRK_PTO_M_REGIST_FOOT = 0,
    BRK_PTO_M_ALREADY_MESSY,
    BRK_PTO_M_GET_POTATO_POS,
    BRK_PTO_M_GET_POTATO_NUM,
    BRK_PTO_M_GET_BOX_POS,
} ;

enum 
{
    BRK_FLG_NONE     = 0,
    BRK_FLG_BOTHSIDE = 0x00000001,
} ;

typedef struct parts_t  PART   ;
typedef struct potato_t POTATO ;
typedef struct work_t   Work   ;
typedef struct box_t    BOX    ;

struct parts_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;

    int    (*act)( Work*, PART* ) ;

    DG_OBJ  *obj    ;

    POTATO  *potato ;/* ポテトへのポインタ */
} ;


struct potato_t
{
    FVECTOR  pos     ; /* 位置     */
    FVECTOR  pos_v   ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;

    int    (*act)( Work*, POTATO* ) ;
    DG_OBJS *objs    ;/* じゃがいもモデル */
    Work    *work    ;

    short    n_parts  ;/* 0以外の時 壊れパーツを持つ*/
    short    n_bounce ;/*  */
    union {
	PART    *prof ;/* 壊れた時の壊れパーツへのポインタ */
	int      flag ;/* お互いに衝突した時のフラグ 衝突:１ ／非衝突:０ */
	FVECTOR *foot ;/* 足として登録された位置へのポインタ */
    } parts ;

    FMATRIX      lights[2] ;

    TARGET       target ;
    POWER_TARGET power  ;

#if MAKING
    void *wireframe ;
#endif

} ;

struct box_t
{
    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR   pos  ;            /* 箱の位置 */
    FMATRIX   lights[2] ;       /* 箱のシェード用のライト*/
    SVECTOR   brot, brot_v ;    /* 箱の向きと角速度 */

    DG_OBJS  *objs ;            /*箱のモデル*/

    DG_PRIM2 *scar[5] ;         /* 弾痕用のプリミティブ */
    short     n_scar[5] ;       /* 弾痕のリングバッファID */
    short     rot[5], rot_v[5] ;/* 蓋の動き用   */
    short     r_max, r_min ;    /* 蓋の限界角度 */

    void    (*act)( Work* ) ;   /*箱のアクト関数*/

    int       flag      ;       /* フラグ */
    float     fill_up   ;       /* ジャガイモが埋まっている高さ（初期位置計算用）*/
    float     floor     ;       /* 床の高さは,固定 */
    short     vitality  ;       /* 箱の耐久力 2で倒れ 1でジャガイモを出す */
    short     non_dmg   ;       /* 箱の無敵時間 */

} ;

struct work_t
{
    GV_ACT    actor    ;

    int       name     ;
    int       where    ;
    int       proc_id  ; /*壊れプロック*/
    POTATO   *potato   ;
    int       n_potato ;
    DG_DEF   *m_def    ;/* 本体モデル       */
    DG_DEF   *p_def    ;/* 壊れパーツモデル */

    short     flag     ;/* フラグ */
    short     type     ;/* タイプ(りんご1,じゃがいも0) */

    POTATO   *foot     ;
    int       n_foot   ;
    int       max_foot ;

    BOX       box      ;
    DG_PRIM2 *shadow   ; /* じゃがいも用 丸影 */

    /*DG_OBJS  *needle[BRK_PTO_N_NEEDLE] ;
      int       n_needle ;*/

    ENEFIND    ene_find ;/*敵兵 見つかり用*/
    
    HZD_BOX   *hzd        ;
    HZD_BOX   *box_hzd[5] ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */




/* brk_pto_act.c */
extern void BRK_PTO_MoveRotateBox( BOX *b, int where ) ;
extern void BRK_PTO_MoveRotateFlip( BOX *b ) ;

extern int  BRK_PTO_ActPartBrokenFree( Work *work, PART *p ) ;
extern int  BRK_PTO_ActPartBroken( Work *work, PART *p ) ;
extern int  BRK_PTO_ActPartBreaking( Work *work, PART *p ) ;
extern int  BRK_PTO_ActNone( Work *work, POTATO *p ) ;
extern int  BRK_PTO_Act( Work *work, POTATO *p ) ;
extern int  BRK_PTO_ActBreak( Work *work, POTATO *p ) ;
extern int  BRK_PTO_ActOnFoot( Work *work, POTATO *p ) ;

/* brk_pto_box.c */
extern void BRK_PTO_ActBox( Work *work ) ;
extern void BRK_PTO_ActOpenedBox( Work *work ) ;
extern void BRK_PTO_ActOpenBox( Work *work ) ;
extern void BRK_PTO_ActFallingDownBox( Work *work ) ;


/* brk_pto_clb.c */
extern void BRK_PTO_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_PTO_PartsTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_PTO_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_pto_msg.c */
extern void BRK_PTO_ReceiveMessage( Work *work ) ;
extern void BRK_PTO_AlreadyMessy( Work *work ) ;

/* brk_pto_ini.c */
extern int  BRK_PTO_InitEneFind( Work *work ) ;
extern void BRK_PTO_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_PTO_InitPrims( Work *work ) ;
extern int  BRK_PTO_InitPotato( POTATO *p, int where, DG_DEF *def, int flag, void *act ) ;
extern int  BRK_PTO_InitBox( Work *work, int map ) ;
extern int  BRK_PTO_GetOptions( Work *, int map ) ;
extern int  BRK_PTO_InitHazard( Work * ) ;
extern void BRK_PTO_InitPart( PART *p, DG_MDL *mdl, DG_OBJ *obj, void *act ) ;
extern int  BRK_PTO_InitTarget( TARGET *t, POWER_TARGET *p, int map, int flag,
				float *uv, float *lv, FVECTOR *pos,
				void (*callback)( TARGET *off, TARGET *def, void *ptr ),
				void *ptr ) ;
extern void BRK_PTO_InitBroken( Work *work ) ;

/* brk_pto_hzx.c */
extern int BRK_PTO_HzdPotatoCheck( Work *work, float rad ) ;
extern int BRK_PTO_HzdPotatoOneCheck( Work *work, POTATO *o, float rad ) ;
extern int BRK_PTO_HzdCheck( Work *work,
			     FVECTOR *pos, FVECTOR *pos_v, FVECTOR *bounce, float sphere ) ;

/* extern variable */
extern FVECTOR BRK_PTO_Gravity  ;
extern FVECTOR BRK_PTO_Bounce   ;
extern FVECTOR BRK_PTO_Size[]   ;
extern FVECTOR BRK_PTO_PartSize[] ;

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
    r = rot_z & 0x0fff ;
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

static inline float BackFaceCullingXZ( FVECTOR *v1, FVECTOR *v2 )
{
    return v2->vx*v1->vz - v1->vz*v2->vx ;
}

static inline void ApplyMatrixZ( FVECTOR *a, FMATRIX *m, FVECTOR *v )
{
#ifdef BP_PSX2_ASM
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
#else
   BP_Mat_RotateVec3( a, m ,v );
#endif    
}

static inline float BackFaceCulling( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a ;

    _sceVu0OuterProduct( &a, v1, v2 ) ;
    return _sceVu0InnerProduct( &a, v3 ) ;
}
