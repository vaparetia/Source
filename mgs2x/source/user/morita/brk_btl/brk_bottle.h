/*
        brk_bottle.h
	瓶破壊用 定義ファイル

        1999/12/02 T.Morita
        $Id: brk_bottle.h,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
*/

#define MAKING 0

#if MAKING
extern void AN_Test_Eye3( FVECTOR * ) ;
extern void AN_Test_Eye2( FVECTOR *, int ) ;
extern void *NewDrawWireframe( DG_OBJ *obj ) ;
#endif

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for debug
#include  "../include/util.h"
// for box hzd
#include  "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

#define BRK_HZX_SPHERE   40
#define BRK_GRAVITY      3
#define BRK_N_PIECE     20

#define BRK_BOUND_X  3000 
#define BRK_BOUND_Z  3000 

#define BRK_VEL_R 0.6f
#define BRK_ROT_R 16
#define BRK_PVEL_R 0.9f

#define BRK_BOUNCE 0.38f
#define BRK_PIECE_ACTIVE    0x80000000
#define BRK_BOTTLE_INACTIVE 0x80000000

enum
{
    BRK_BTL_F_COL_R  = 0x00ff0000,
    BRK_BTL_F_COL_G  = 0x0000ff00,
    BRK_BTL_F_COL_B  = 0x000000ff,
    BRK_BTL_F_COLMSK = 0x00ffffff,

    BRK_BTL_F_NOWINE = 0x01000000,
} ;

enum
{
    BTK_BTL_HZD_BALL  = -3,
    BTK_BTL_HZD_TUNNEL,
    BTK_BTL_HZD_BOX,
    BTK_BTL_HZD_DISK,
} ;

typedef struct bottle_t  BOTTLE  ;
typedef struct parts_t   PART    ;
typedef struct work_t    Work    ;
typedef struct piece_t   PIECE   ;
typedef struct bottle_type_t BTL_TYP ;

struct bottle_type_t
{
    DG_DEF *bf_def  ;/*壊れ前*/
    DG_DEF *af_def  ;/*壊れ後*/
    char   *bf_name ;/*壊れ前name*/
    char   *af_name ;/*壊れ後name*/
    int     color  ;
    int     se_num ;
} ;

struct piece_t
{
    DG_COMDL_POS *cmdl_p ;

    int      flag   ;
    short    rot_x , rot_y  ;
    short    rot_vx, rot_vy ;
    FVECTOR  pos    ; /* 位置 */
    FVECTOR  pos_v  ; /* 速度 */
} ;

struct parts_t
{
    SVECTOR  rot    ; /* 回転角度  except <rot->pad> is the starting frame*/
    SVECTOR  rot_v  ; /* 角速度   */
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */

    DG_OBJ  *obj    ;

    BOTTLE  *bottle ;
    TARGET  *target ;

    void   (*act)( Work*, PART* ) ;

#if MAKING
    void *wireframe ;
#endif

} ;

struct bottle_t
{
    SVECTOR       rot    ; /* 回転角度  except <rot->pad> is the starting frame*/
    SVECTOR       rot_v  ; /* 角速度   */
    FVECTOR       pos    ; /* 位置     */
    FVECTOR       pos_v  ; /* 速度     */
    FVECTOR       center ;

    int           flag   ; /* フラグと瓶の色情報  (flg:r:g:b)=(8:8:8:8) */

    DG_OBJS      *objs   ; /* 現在の表示ハンドラ */
    DG_DEF       *brk_bf ; /* 壊れ前のモデル情報 */
    DG_DEF       *brk_af ; /* 壊れ後のモデル情報 */
    PART         *parts  ; /* PART array comes */

    int         (*act)( Work*, BOTTLE* ) ;

    FMATRIX      lights[2] ;

    Work         *work   ;
    TARGET       *target ;
    POWER_TARGET *power  ;

    ENEFIND       ene_find ;

#if MAKING
    void *wireframe ;
#endif

} ;

struct work_t
{
    GV_ACT    actor   ;

    DG_COMDL *piece_s ;
    DG_COMDL *piece_l ;
    PIECE     piece_prof[BRK_N_PIECE] ;
    int       n_piece ;

    BOTTLE   *bottles   ;
    int       n_bottles ;
    int       n_broken  ;

    LIT_DEF   *lit      ;

    int       name      ;
    int       where     ;
    int       proc_full ; /*完全壊れプロック*/
    int       proc_half ; /*半分壊れプロック*/
    HZD_BOX  *hzd_box   ;
    int       hzd_id    ;
    FVECTOR  *hzd       ;
    HZX_GROUP_ID hzx    ;

    ENEFIND   ene_find ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */


/* brk_bottle.c */
extern void BRK_BTL_FreeBottle( BOTTLE *b ) ;

/* brk_btl_act.c */
extern void BRK_BTL_FindLowestPoint( DG_OBJ *obj, FVECTOR *min ) ;
extern int  BRK_BTL_ActBreaking( Work *, BOTTLE * ) ;
extern int  BRK_BTL_ActFinished( Work *, BOTTLE * ) ;
extern int  BRK_BTL_ActBroken( Work *work, BOTTLE *b ) ;
extern void BRK_BTL_ActPartCopyParent( Work *work, PART *p ) ;
extern void BRK_BTL_ActPartFalling( Work *work, PART *p ) ;
extern void BRK_BTL_ActPartSlide( Work *work, PART *p ) ;
extern void BRK_BTL_ActPartOnShelf( Work *work, PART *p ) ;
extern void BRK_BTL_ActPartTiltingOnShelf( Work *work, PART *p ) ;

extern void BRK_BTL_ActPieces( Work *work ) ;
extern void BRK_BTL_StartActPieces( Work *work, FVECTOR *pos, float size,
				    FVECTOR *frc, int i, int col ) ;
extern void BRK_BTL_MovePart( PART *p, int where ) ;

/* brk_btl_hzd.c */
extern int BRK_BTL_HzdCheck( Work *work, FVECTOR *pos, FVECTOR *vel,
			     FVECTOR *bounce, float sphere, TARGET *t ) ;


/* brk_btl_clb.c */
extern void BRK_BTL_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_BTL_TargetCallBackParent( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_BTL_WineSplash( Work *work, BOTTLE *b ) ;

/* brk_btl_ini.c */
extern int BRK_BTL_GetOptions( Work *work, int where ) ;
extern int BRK_BTL_InitTarget( BOTTLE *b, DG_DEF *part, int where ) ;
extern int BRK_BTL_InitHazard( Work *work, int where ) ;
extern int BRK_BTL_InitPieces( Work *work, int where );
extern int BRK_BTL_InitModel( BOTTLE *b, int where, DG_DEF *def, int flag, LIT_DEF *lit ) ;
extern void BRK_BTL_InitBottleType( void ) ;
extern void BRK_BTL_InitBroken( Work *work ) ;
extern void BRK_BTL_InitEneFind( Work *work ) ;

/* brk_btl_msg.c */
extern void BRK_BTL_ReceiveMessage( Work *work ) ;
extern void BRK_BTL_AlreadyMessy( Work *work, int mask ) ;

/*external chara */
extern void *NewWineStream( FMATRIX *world, float width ) ;   /* brk_wine.c */
extern void *NewWineStreamLimit( FMATRIX *world, float width, float limit ) ;
extern void *NewWineStreamFall( FMATRIX *world, int where ) ; /* brk_winefall.c */
extern void *NewCrushDust( FVECTOR *v, FVECTOR *dir, int mode ) ;


static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y  )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
static inline void InvTransMatrix( FMATRIX *mtx, FVECTOR *pos )
{
    mtx->m[3][X] -= pos->vx ;
    mtx->m[3][Y] -= pos->vy ;
    mtx->m[3][Z] -= pos->vz ;
}

