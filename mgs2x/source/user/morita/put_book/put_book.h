/*
  put_book.h
  雑誌破壊用 定義ファイル

  1999/06/20 T.Morita
  $Id: put_book.h,v 1.1.1.3 2002/11/19 11:46:30 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for DEBUG
#include "../include/util.h"
// for DG-LIB
#include "../include/libdg_x.h"

#define PUT_GRAVITY      2.0f
#define PUT_BOK_GRAVITY  6.0f

#define PUT_BOK_N_PIECE 60
#define PUT_BOK_N_PAGE  20

#define PUT_BOK_SPOOLTIME 10

#define PUT_VEL_R  0.85f
#define PUT_VEL_B  0.6f
#define PUT_ROT_R  16

#define PUT_BOUNCE      1.68f
#define PUT_BOUNCE_WALL 1.38f

#define PUT_DUST_SPHERE 5.0f
#define PUT_BOKER_RAD 48.0f*2.0f

#define PUT_BOK_INACTIVE 0x8000000

#define PUT_N_SCAR  10 
#define PUT_N_VERTS 4

#define DEG2RAD(_s) (float)((_s)*(float)M_PI/180.0f)
#define RAD2ANG(_s) (short)((_s)*2048.0f/(float)M_PI)
#define ANG2RAD(_s) (float)((_s)*(float)M_PI/2048.0f)

typedef struct move_t  MOVE  ;
typedef struct piece_t PIECE ;
typedef struct page_t  PAGE  ;
typedef struct book_t  BOOK ;
typedef struct work_t  Work  ;

struct move_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    SVECTOR  rot   ;
    SVECTOR  rot_v ;
} ;

struct piece_t
{
    MOVE          mov  ;
    int           flag ;
    DG_COMDL_POS *pos  ;/* 塵モデル位置情報 */
} ;

struct page_t
{
    int      flag ;

    float    open ;
    float    open_v ;

    BOOK    *book ;/* 雑誌の情報 */

    VERTEX_ANIME_WORK *anime ;
    DG_OBJS *objs  ;/* ページモデル */
} ;

struct book_t
{
    MOVE         mov    ;

    TARGET       target[2] ;
    POWER_TARGET power[2]  ;

    FMATRIX      lights[2] ;

    SVECTOR      rot    ;/* 元にもどす回転 */

    int          flag   ;
    DG_OBJS     *objs   ;/* 現在の雑誌モデル */
    DG_DEF      *def    ;/* 雑誌開きモデル */
    Work        *work   ;

    int          se_tic ;

    float	 y_offset;
    float	 y_pos   ;
    float        open    ;/* 開いている角度 */
    float        open_v  ;
} ;

struct work_t
{
    GV_ACT_EX    actor ;

    int          name  ;
    int          where ;
    HZX_GROUP_ID hzx   ;

#if 0
    DG_COMDL *piece_s ;
    DG_COMDL *piece_l ;

    PIECE     piece[PUT_BOK_N_PIECE] ;
    int       n_piece ;
#endif

    PAGE      page[PUT_BOK_N_PAGE]   ;
    int       n_page ;
    BOOK      book ;

    DG_DEF   *n_def   ;/* ページモデル     */
    CV2_DEF  *c_def   ;/* ページ共有モデル */
    float     floor ;
    int       flag    ;
    short     blink   ;

    short          fall    ;
    HZX_HZD        seg     ; /* 落し穴関係 */
    HZX_D_CALLBACK dyn_clb ;

    int            non_dmg ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* put_book.c */
extern void PUT_BOK_FreeBook( BOOK *p ) ;
extern void PUT_BOK_FreePage( PAGE *p ) ;

/* put_mgz_act.c */
extern float PUT_MGS_GetCurrentSize( BOOK *mgz, FMATRIX *m, FVECTOR *size, FVECTOR *cen ) ;
extern void PUT_BOK_DispBook( BOOK *m ) ;
extern void PUT_BOK_ActBook( Work *work ) ;
extern void PUT_BOK_StartActPiece( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v, float width, float pop ) ;
extern void PUT_BOK_ActPiece( Work *work ) ;
extern void PUT_BOK_ActPage( Work *work ) ;
extern void PUT_BOK_StartActPage( Work *work, BOOK *m, int frame ) ;

/* put_mgz_clb.c */
extern void PUT_BOK_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* put_mgz_msg.c */
extern void PUT_BOK_ReceiveMessage( Work *work ) ;

/* put_mgz_ini.c */
extern void PUT_BOK_InitHazard( Work *work ) ;
extern void PUT_BOK_InitBroken( Work *work ) ;
extern int  PUT_BOK_InitPiece( Work *work, int where ) ;
extern int  PUT_BOK_InitPrimitive( Work *work ) ;
extern int  PUT_BOK_InitPage( Work *work ) ;
extern int  PUT_BOK_InitTarget( BOOK *bok, int where, DG_MDL *mdl, int idx ) ;
extern int  PUT_BOK_InitParam( Work *work, DG_OBJS *objs, int where, SVECTOR *rot ) ;

extern void *NewTs_Min_Fog( FVECTOR *pos ); /*霧を出すキャラ*/


static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
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
