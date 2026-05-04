/*
  brk_plate.h
  皿破壊用 定義ファイル
  
  2000/04/25 T. Morita
  $Id: brk_plate.h,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
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
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

#define BRK_DUST_GRAVITY   1
#define BRK_GRAVITY        4.0f

#define BRK_PLT_N_BRKPIECE 10
#define BRK_PLT_N_PIECE   (BRK_PLT_N_BRKPIECE * 3)
#define BRK_PLT_N_PLATE    3 /* 1 pileにつき3枚の皿がある */
#define BRK_PLT_N_DUST     10

#define BRK_PVEL_R 0.8f
#define BRK_PROT_R 16
#define BRK_VEL_R  0.85f
#define BRK_VEL_B  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE      1.68f
#define BRK_BOUNCE_WALL 1.38f

#define BRK_DUST_SPHERE   (0.0f)

#define BRK_PLT_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4

#define BRK_PLT_SE_DFLT  11 /* SEがならない フレーム数*/
#define BRK_PLT_SE2_DFLT 8  /* SEがならない フレーム数*/


enum flag_t
{
    BRK_F_IS_PARENT = 0x0001, /* for  struct piled_t */
    BRK_F_HAS_CHILD = 0x0002, /* for  struct piled_t */
    BRK_F_IS_ACTIVE = 0x0004, /* for  struct piled_t */
    BRK_F_IS_BROKEN = 0x0008, /* for  struct piled_t */
} ;

typedef struct parts_t  PART  ;
typedef struct plate_t  PLATE ;
typedef struct piled_t  PILED ;
typedef struct work_t   Work  ;

typedef struct move_t
{
    FVECTOR  pos    ; /* 位置 */
    FVECTOR  pos_v  ; /* 速度 */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
} MOVE ;

struct parts_t
{
    MOVE     mov  ;
    int      flag ;
    DG_MDL  *mdl  ;

    DG_COMDL_POS *pos ;
} ;

struct plate_t
{
    MOVE         mov    ;
    int        (*act)( Work*, PLATE* ) ;
    DG_OBJS     *objs   ; /* 皿一枚モデル */

    PART        *parts  ; /* 破片へのポインタ */
    Work        *work   ; /* コールバックで必要 */

    FMATRIX      lights[2] ; /* シェード用ライトマトリックス */
} ;

struct piled_t
{
    TARGET       target ;
    POWER_TARGET power  ;

    int          flag   ;/* enum flag_t を参照*/
    float        vel_y  ;
    DG_OBJS     *objs   ;/* 5枚重ねモデル */
    Work        *work   ;

    FMATRIX      lights[2] ; /* シェード用ライトマトリックス */
} ;

struct work_t
{
    GV_ACT     actor  ;

    DG_DEF    *m_def  ;/* 本体モデル       */
    DG_DEF    *p_def  ;/* 壊れパーツモデル */
    DG_DEF    *d_def  ;/* 壊れコモデル */

    int        name    ;
    int        where   ;
    int        flag    ;
    int        proc_id ; /*壊れプロック*/

    DG_PRIM2  *dust    ; /* 埃プリミティブ */
    FVECTOR    dust_v[BRK_PLT_N_DUST] ;
    int        n_dust  ;

    DG_COMDL  *piece_l ; /* 破片のコモデル */
    DG_COMDL  *piece_s ;
    PART       piece[BRK_PLT_N_PIECE] ;
    int        n_piece ;

    DG_COMDL **comdl   ;
    PART      *part    ;
    int        n_part  ;
    PLATE     *plate   ;
    int        n_plate ;

    PILED     *piled   ;
    int        n_piled ;

    ENEFIND    ene_find ;/*敵兵 見つかり用*/

    LIT_DEF   *lit     ;
    HZD_BOX   *hzd     ;
} ;

extern int BRK_PLT_SeState  ;/* 音が重なってならないように */
extern int BRK_PLT_SeState2 ;/* 音が重なってならないように */
extern int BRK_PLT_SeCount  ;/* 音の種類を奇数と偶数で変える */


extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

/* brk_plt_act.c */
extern void BRK_PLT_GetLightMatrix( FVECTOR *pos, FMATRIX *lights, float intense ) ;
extern void BRK_PLT_ActPiece( Work *work, int i, PART *p ) ;
extern void BRK_PLT_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
				   float width, float pop ) ;
extern void BRK_PLT_ActPart( Work *work, int i, PART *p, int flag ) ;
extern void BRK_PLT_StartActPart( Work *work, PLATE *plt, FVECTOR *pos_v,
				  float width, float pop ) ;
extern void BRK_PLT_ActPlate( Work *work, int i, PLATE *p ) ;
extern PLATE *BRK_PLT_StartActPlate( Work *work, FVECTOR *pos, FVECTOR *pos_v,
				     float width, float pop ) ;
extern void BRK_PLT_ActDust( Work *work, int i ) ;
extern void BRK_PLT_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
				  float width, float pop ) ;
extern void BRK_PLT_ActPiled( Work *work, int i, PILED *p ) ;

/* brk_plt_clb.c */
extern void BRK_PLT_PiledTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_PLT_PlateTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_plt_msg.c */
extern void BRK_PLT_ReceiveMessage( Work *work ) ;
extern void BRK_PLT_AlreadyMessy( Work *work ) ;

/* brk_plt_ini.c */
extern void BRK_PLT_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_PLT_GetOptions( Work *work, int where ) ;
extern int  BRK_PLT_InitPrimitive( Work *work ) ;
extern int  BRK_PLT_InitHazard( Work *work ) ;
extern int  BRK_PLT_InitPiece( Work *work, int where ) ;
extern int  BRK_PLT_InitPlate( Work *work, int where ) ;
extern int  BRK_PLT_InitPart( Work *work, int where ) ;
extern int  BRK_PLT_InitTarget( TARGET *t, POWER_TARGET *p, int map,
				DG_DEF *def, FVECTOR *pos,
				void (*callback)( TARGET *off, TARGET *def, void *ptr ),
				void *ptr ) ;
extern void BRK_PLT_InitEneFind( Work *work ) ;
extern void BRK_PLT_InitBroken( Work *work ) ;

/* extern variable */
extern FVECTOR BRK_PLT_Gravity ;
extern FVECTOR BRK_PLT_Bounce  ;
extern FVECTOR BRK_PLT_Size    ;

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

static inline float BackFaceCulling( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a ;

    _sceVu0OuterProduct( &a, v1, v2 ) ;
    return _sceVu0InnerProduct( &a, v3 ) ;
}

