/*
  brk_magazine.h
  雑誌破壊用 定義ファイル

  1999/06/20 T.Morita
  $Id: brk_magazine.h,v 1.1.1.3 2002/11/19 11:45:35 Yoshizawa1 Exp $
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
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

#define BRK_GRAVITY      2.0f
#define BRK_MGZ_GRAVITY  6.0f

#define BRK_MGZ_N_PIECE 60
#define BRK_MGZ_N_PAGE  20
#define BRK_MGZ_N_DUST  60

#define BRK_MGZ_SPOOLTIME 10

#define BRK_VEL_R  0.85f
#define BRK_VEL_B  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE      1.68f
#define BRK_BOUNCE_WALL 1.38f

#define BRK_DUST_SPHERE 5.0f
#define BRK_MGZER_RAD 48.0f*2.0f

#define BRK_MGZ_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4

#define DEG2RAD(_s) ((_s)*(float)M_PI/180.0f)

typedef struct move_t     MOVE  ;
typedef struct piece_t    PIECE ;
typedef struct page_t     PAGE  ;
typedef struct magazine_t MAGAZINE ;
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
#if MAKING
    void *pursuit ;
#endif
} ;

struct page_t
{
    int      flag ;

    float    open ;
    float    open_v ;

    MAGAZINE *book ;/* 雑誌の情報 */

    VERTEX_ANIME_WORK *anime ;
    DG_OBJS *objs  ;/* ページモデル */
} ;

struct magazine_t
{
    MOVE         mov    ;
    int          flag   ;
    DG_OBJS     *objs   ;/* 現在の雑誌モデル */

    float        open   ;
    float        open_v ;
	
    FMATRIX      lights[2] ;

    TARGET       target[2] ;
    POWER_TARGET power[2]  ;
    DG_VERTS_ANIME		v_anime[3];
#if MAKING
    void *view[3] ;
#endif
	
    DG_DEF      *def    ;/* 雑誌開きモデル */
    Work        *work   ;
    short	mgz_type ;
    short       non_damg ;
    float		y_offset;
} ;

struct work_t
{
    GV_ACT    actor   ;

    int       name    ;
    int       where   ;
    int       proc_id ; /*壊れプロック*/

    DG_PRIM2  *dust   ; /* 埃プリミティブ */
    FVECTOR    dust_v[BRK_MGZ_N_DUST] ;
    int        n_dust ;

    DG_COMDL *piece_s ;
    DG_COMDL *piece_l ;

    PIECE     piece[BRK_MGZ_N_PIECE] ;
    int       n_piece ;
    PAGE      page[BRK_MGZ_N_PAGE]   ;
    int       n_page ;
    MAGAZINE *magazine ;
    int       n_magazine ;
    DG_DEF   *n_def   ;/* ページモデル */
    DG_DEF   *p_def   ;/* 破片コモデル */
    CV2_DEF  *c_def   ;
    int       flag    ;

    LIT_DEF  *lit     ;

    int       hzd_id  ;
    HZD_BOX  *hzd     ;
    ENEFIND   ene_find ;
//	float	 rad[BRK_MGZ_N_DUST];
//	float	 size[BRK_MGZ_N_DUST];
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* brk_magazine.c */
extern void BRK_MGZ_FreeMagazine( MAGAZINE *p ) ;
extern void BRK_MGZ_FreePage( PAGE *p ) ;

/* brk_mgz_act.c */
extern float BRK_MGS_GetCurrentSize( MAGAZINE *mgz, FMATRIX *m, FVECTOR *size, FVECTOR *cen ) ;
extern void BRK_MGZ_DispMagazine( MAGAZINE *m ) ;
extern void BRK_MGZ_ActMagazine( Work *work ) ;
extern void BRK_MGZ_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_MGZ_ActPiece( Work *work ) ;
extern void BRK_MGZ_ActDust( Work *work ) ;
extern void BRK_MGZ_StartActDust( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void BRK_MGZ_ActPage( Work *work ) ;
extern void BRK_MGZ_StartActPage( Work *work, MAGAZINE *m, int frame ) ;

/* brk_mgz_clb.c */
extern void BRK_MGZ_ChangeBook( MAGAZINE *p, int offset ) ;
extern int  BRK_MGZ_ChangeOpenBook( MAGAZINE *p ) ;
extern void BRK_MGZ_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_mgz_msg.c */
extern void BRK_MGZ_ReceiveMessage( Work *work ) ;
extern void BRK_MGZ_AlreadyMessy( Work *work ) ;

/* brk_mgz_ini.c */
extern void BRK_MGZ_InitEneFind( Work *work ) ;
extern void BRK_MGZ_InitBroken( Work *work ) ;
extern void BRK_MGZ_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_MGZ_GetOptions( Work *, int where ) ;
extern int  BRK_MGZ_InitHazard( Work * ) ;
extern int  BRK_MGZ_InitPiece( Work *work, int where ) ;
extern int  BRK_MGZ_InitPrimitive( Work *work ) ;
extern int  BRK_MGZ_InitPage( Work *work ) ;
extern int  BRK_MGZ_InitTarget( MAGAZINE *mgz, TARGET *t, POWER_TARGET *p, int where, int flag,
				float *lx, float *ux, FVECTOR *pos ) ;

extern void *NewTs_Min_Fog( FVECTOR *pos ); /*霧を出すキャラ*/


/* extern variable */
extern FVECTOR BRK_MGZ_Gravity ;
extern FVECTOR BRK_MGZ_Bounce  ;
extern FVECTOR BRK_MGZ_Size    ;


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

