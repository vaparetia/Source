/*
   fort_obj.h
   フォーチュン戦 ライデン隠れ専用プットオブジェ

   2000/12/14 T.Morita
   $Id: fort_obj.h,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#define MAKING 0

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

// for BRK_HZD
#include "../../brk_hzd/brk_hazard.h"
#include "../../brk_utl/brk_utl.x"
#include "fort_sig.h"

#include "../include/fort_flag.h"


#define FRT_MAX_SNAHIDE 50


#define FRT_OBJ_GetTypeMasked(_h)  ((_h)->type & FRT_TYP_TYPEMASK)
#define FRT_OBJ_GetType(_h)  (&FRT_OBJ_Type[FRT_OBJ_GetTypeMasked(_h)])
#define FRT_OBJ_GetTypeModel(_h)   (FRT_OBJ_GetType(_h)->model)
#define FRT_OBJ_GetTypeFlag(_h)    (FRT_OBJ_GetType(_h)->flag )
#define FRT_OBJ_GetTypeFunc(_h)    (FRT_OBJ_GetType(_h)->func )
#define FRT_OBJ_GetTypeFront(_h)   (FRT_OBJ_GetType(_h)->front)
#define FRT_OBJ_GetTypeRear(_h)    (FRT_OBJ_GetType(_h)->rear )
#define FRT_OBJ_GetTypeAimFlag(_h) (FRT_OBJ_GetType(_h)->aim_flag)
#define FRT_OBJ_GetTypeHazard(_h)  (FRT_OBJ_GetType(_h)->hazard)
#define FRT_OBJ_GetTypeShadow(_h)  (FRT_OBJ_GetType(_h)->shadow)

enum proc_t
{
    FRT_OBJ_P_DESTROY  = 0,
    FRT_OBJ_P_CHANGED,
    FRT_OBJ_N_PROCS,
} ;

enum type_t
{
    FRT_TYP_DUMMY  = 0,
    FRT_TYP_WOODBOX0,
    FRT_TYP_WOODBOX0_BRK,

    FRT_TYP_IRONBOX1M0_A,
    FRT_TYP_IRONBOX1M1_B,
    FRT_TYP_IRONBOX1M2_C,
    FRT_TYP_IRONBOX1M3_D,

    FRT_TYP_IRONBOX2M_A,
    FRT_TYP_IRONBOX2M_B,
    FRT_TYP_IRONBOX2M_C,
    FRT_TYP_IRONBOX2M_D,
    FRT_TYP_IRONBOX2M_E,
    FRT_TYP_IRONBOX2M_F,
    FRT_TYP_IRONBOX2M_G,
    FRT_TYP_IRONBOX2M_H,
    FRT_TYP_IRONBOX2M_I,

    FRT_TYP_EXPLCAN1M_A,
    FRT_TYP_EXPLCAN1M_B,
    FRT_TYP_EXPLCAN1M_C,
    FRT_TYP_EXPLCAN1M_BRK,

    FRT_TYP_FORKLIFT_A,
    FRT_TYP_FORKLIFT_B,
    FRT_TYP_FORKLIFT_C,
    FRT_TYP_FORKLIFT_D,

    FRT_TYP_EXPLDRUM_RA,
    FRT_TYP_EXPLDRUM_RB,
    FRT_TYP_EXPLDRUM_RC,
    FRT_TYP_EXPLDRUM_RBRK,

    FRT_TYP_IRONBOX1M0_E,
    FRT_TYP_IRONBOX1M1_F,
    FRT_TYP_IRONBOX1M2_G,

    FRT_TYP_EXPLDRUM_LA,
    FRT_TYP_EXPLDRUM_LB,
    FRT_TYP_EXPLDRUM_LC,
    FRT_TYP_EXPLDRUM_LBRK,

    FRT_TYP_IRONBOX1M0_H,/*不滅の鉄箱1M*/
    FRT_TYP_IRONBOX1M1_I,

    FRT_N_TYPES,

    FRT_TYP_FLAG_SHADOW01   = 0x00000100,
    FRT_TYP_FLAG_SHADOW02   = 0x00000200,
    FRT_TYP_FLAG_SHADOW10   = 0x00000300,
    FRT_TYP_FLAG_SHADOW     = 0x00000f00,

    FRT_TYP_FLAG_SYMETRY    = 0x00001000,
    FRT_TYP_FLAG_SLIDEOFF   = 0x00002000,
    FRT_TYP_FLAG_ON_ME      = 0x00004000,
    FRT_TYP_FLAG_UPPER_DECK = 0x00008000,
    FRT_TYP_FLAG_LASTOBJ    = 0x00010000,

    FRT_TYP_FLAGMASK      = 0xffffff00,
    FRT_TYP_TYPEMASK      = ~FRT_TYP_FLAGMASK,
} ;


/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
typedef struct Work_t      Work ;
typedef struct hide_t      HIDE ;
typedef struct hide_type_t TYPE ;

struct hide_t
{
    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR      slide  ;/*ずり落ちの場合に ここに来る*/
    SVECTOR      rot    ;/* y軸回転 */

    int       procs[FRT_OBJ_N_PROCS] ;
    int       type   ;/* 木箱だの鉄箱だの決めるタイプ enum type_t が入る */
    int       phase  ;/* フォーチュンが壊すフェーズを入れる */
    int       move   ;/* このコンテナが壊れたとき,他が動くかどうか */
    DG_OBJS  *objs   ;/* モデル */
    Work     *work   ;/* ワークへのポインタ */

    HZX_D_SEGMENT *segs[4] ;/* 箱の壁ハザード */
    HZX_D_FLOOR   *flrs    ;/* 箱の壁ハザード */
    HZD_BOX       *hzd[3]  ;/* 壊れ物ハザード */
    void          *shadow ; /* 影キャラ effect/efct_drop.c */

#if MAKING
    void         *wireframe ;
#endif

} ;

struct hide_type_t
{
    char    *model ;

    int    (*func)( HIDE *hide, TARGET *trgt, int flag ) ;
    int      flag  ;
    int      break_id ;
    int      aim_flag ;
    float    magnitude ;
    FVECTOR *front ;
    FVECTOR *rear  ;

    FVECTOR *hazard ;
    char    *shadow ;
} ;

struct Work_t
{
    GV_ACT_EX actor ;

    int       where   ;
    int       name    ;
    int       status  ;
    LIT_DEF  *lit_def ;
    int       flag    ;

    HIDE      hide[FRT_MAX_SNAHIDE] ;  

    HZD_BOX  *hzd ;
} ;



/* fort_obj.c */
extern void FRT_OBJ_FreeObject( HIDE *h, int target ) ;
extern void FRT_OBJ_FreeHzxHzd( HIDE *h, int target ) ;


/* fort_obj_ini.c */
extern int  FRT_OBJ_GetOptions( Work *work, int name, int where ) ;
extern int  FRT_OBJ_InitModel( HIDE *h, FMATRIX *mtx ) ;
extern int  FRT_OBJ_InitHazard( Work *work ) ;
extern int  FRT_OBJ_InitTargetHzxHzd( HIDE *hide ) ;
extern void FRT_OBJ_InitShadow( HIDE *h ) ;


/* fort_obj_act.c */
extern int  FRT_OBJ_ActCheckHide( Work *work ) ;
extern void FRT_OBJ_SendFortuneHideMessage( Work *work, int status ) ;


/* fort_obj_clb.c */
extern void FRT_OBJ_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


/* fort_obj_hzx.c */
extern void FRT_OBJ_FreeHzxSegAndFloor( HIDE *hide ) ;
extern void FRT_OBJ_MakeHzxSizeAndCenter( DG_DEF *def, FVECTOR *t_size, FVECTOR *t_pos ) ;
extern int FRT_OBJ_MakeHzxFromOBJS( HIDE *hide, DG_OBJS *objs, int where, FVECTOR *t_size, FVECTOR *t_pos ) ;


/* Fortune System */
extern void FRT_AIM_InitHideSystem( void * handle ) ;
extern int  FRT_AIM_AddAimSpot( void *aim_id, void *nxt_id, int phase,
				FVECTOR *center,
				FVECTOR *front, FVECTOR *rear,
				u_int flag ) ;
extern int  FRT_AIM_RemoveAimSpot( void *work ) ;
/* weapon/fort_blast.c */
extern void *NewFortBlast( FVECTOR *vec, int side,
			   float range1, float range2, float damage, float faint,
			   int wp, int mode ) ;


/* Effects */
extern int   NewWoodenBoxBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewIronBoxBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewIronBoxThrough( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewIronBoxComplete( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewIronBoxBurn( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewSlideBox( HIDE *hide ) ;

extern int   NewAnythingDustScale( HIDE *hide, TARGET *trgt, int n_dust, int id,
				   float gravity, float velocity, float scale,
				   int *sound, int n_sound ) ;
extern int   NewAnythingDust( HIDE *hide, TARGET *trgt, int n_dust, int id,
			      float gravity, float velocity, int life,
			      int *sound, int n_sound ) ;
extern int   NewTankDust( HIDE *hide, TARGET *trgt, int n_dust, int id,
			  float gravity, float velocity, int life,
			  int *sound, int n_sound ) ;
extern int   NewAnythingDustScaleInit( HIDE *hide, FVECTOR *pos,
				       int n_dust, int id, float scale ) ;
extern int   NewIronBoxDust( HIDE *hide, TARGET *trgt, int n_dust ) ;
extern void *NewFortHeatIron( DG_OBJS *objs ) ;

extern int   NewContainerBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewContainerBrokenComplete( HIDE *hide, TARGET *trgt, int flag ) ;

extern int   NewGasTankBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewBombTankBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewGasBottomBroken( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewGasCap( HIDE *hide, FVECTOR *pos, 
			short rot_x, short rot_y, int flag ) ;
extern void *NewOilSpreadCalled( FVECTOR *pos, int time, int where, int life ) ;
extern void *NewOilStained( FVECTOR *pos, int where, int life ) ;

extern int   NewForkliftBrokenFall( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewForkliftBrokenUpper( HIDE *hide, TARGET *trgt, int flag ) ;
extern int   NewForkliftFrontWheel( HIDE *hide, FVECTOR *pos, int id, int frame ) ;
extern int   NewForkliftRearWheel( HIDE *hide, FVECTOR *pos, int id, FVECTOR *trgt ) ;
extern int   NewForkliftHandle( HIDE *hide, FVECTOR *pos, int id, short rotz ) ;
extern int   NewForkliftFrame( HIDE *hide, FVECTOR *pos, int id, TARGET *trgt ) ;
extern int   NewForkliftMainLift( HIDE *hide, int id, FVECTOR *pos, SVECTOR *rot ) ;
extern void *NewForkliftStick( HZD_BOX *hzd, int type, DG_OBJ *obj, short rot_x, short rot_y ) ;

extern int   NewFortObjectShadow( HIDE *hide ) ;
extern void *NewFortDropShadow( DG_OBJS *objs, float floor, char *shadow_name, int level ) ;
extern void *NewFortSideDust( FVECTOR *pos, FVECTOR *dir, int n_prim ) ;
extern void *NewFortSideDustSize( FVECTOR *pos, FVECTOR *dir, int n_dust, float size ) ;
extern void *NewFortSpark( FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd, int mode ) ;
extern void *NewFortFlame( FVECTOR *pos, FVECTOR *dir, int already ) ;
extern void *NewFortSmoke( FMATRIX *world, FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd ) ;
extern void *NewFortVibrate( int base, int high, int low ) ;

extern void FRT_LGT_SetMagnitude( FVECTOR *center, float magnitude ) ;


/* External Person */
/*           okajima/etc/shake_camera.c */
extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos );
extern void *NewShakeCamera( int chanl, int intense, int time ) ;
/*           okajima/effect/bomb.c      */
extern void *NewBombEffect( FVECTOR *pos, int mode ) ;
/*           okajima/effect/bomb.c      */
extern void *NewLineSmoke( FMATRIX *world, FVECTOR *shift, int life );
/*           okajima/effect/bomb.c      */
extern void *NewCrushLine( FVECTOR *pos, FVECTOR *force, float radius ) ;


#ifdef __MAIN_FILE__

void *FRT_OBJ_Work = NULL ;

static FVECTOR Hzd_1MBox[] =
{
    { 500.0f, 500.0f, 500.0f, 0.0f }, /*size*/
    {   0.0f, 500.0f,   0.0f, 1.0f }, /*offs*/ /*for DynamicHZX*/
    { 500.0f,1500.0f, 500.0f, 0.0f }, /*size*/
    {   0.0f,-500.0f,   0.0f, 1.0f }, /*offs*/
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
static FVECTOR Hzd_2MBox[] =
{
    {1000.0f,1000.0f,1000.0f, 0.0f },/*size*/
    {   0.0f,1000.0f,   0.0f, 1.0f },/*offs*/ /*for DynamicHZX*/ 
    {1000.0f,1000.0f,1000.0f, 0.0f },/*size*/ 
    {   0.0f,1000.0f,   0.0f, 1.0f },/*offs*/ 
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;

#if 0
static FVECTOR Hzd_ForkLift[] =
{
    { 937.0f , /*647.0f*/1200.0f, 510.0f, 0.0f },/*size*/ 
    {  62.0f , /*547.0f*/ 750.0f,   0.0f, 1.0f },/*offs*/  /*for DynamicHZX*/
    { 465.00f, 500.0f, 505.0f, 0.0f },/*size*/
    {-530.00f, 305.0f,   0.0f, 1.0f },/*offs*/ 
    { 300.73f, 430.0f, 505.0f, 0.0f },/*size*/
    {  62.27f,   0.0f,   0.0f, 1.0f },/*offs*/ 
    { 325.00f, 570.0f, 130.5f, 0.0f },/*size*/
    {  62.27f,   0.0f,   0.0f, 1.0f },/*offs*/ 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
#else
static FVECTOR Hzd_ForkLift[] =
{
    { 937.0f , 647.0f, 510.0f, 0.0f },/*size*/ 
    {  62.0f , 547.0f,   0.0f, 1.0f },/*offs*/  /*for DynamicHZX*/
    { 465.00f, 500.0f, 505.0f, 0.0f },/*size*/
    {-530.00f, 305.0f,   0.0f, 1.0f },/*offs*/ 
    { 300.73f, 430.0f, 505.0f, 0.0f },/*size*/
    {  62.27f,   0.0f,   0.0f, 1.0f },/*offs*/ 
    { 325.00f, 570.0f, 130.5f, 0.0f },/*size*/
    {  62.27f,   0.0f,   0.0f, 1.0f },/*offs*/ 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
#endif

static FVECTOR Hzd_2MBoxE[] =
{
    {1000.00f,1000.0f,1000.0f, 0.0f },/*size*/
    {   0.00f,1000.0f,   0.0f, 1.0f },/*offs*/  /*for DynamicHZX*/
    {1000.00f, 500.0f,1000.0f, 0.0f },/*size*/
    {   0.00f, 500.0f,   0.0f, 1.0f },/*offs*/ 
    { 500.00f,1000.0f,1000.0f, 0.0f },/*size*/
    {-500.00f,1000.0f,   0.0f, 1.0f },/*offs*/ 
    {    0.0f,   0.0f,   0.0f,-1.0f },
} ;
static FVECTOR Hzd_2MBoxF[] =
{
    {1000.00f,1000.0f,1000.0f, 0.0f },
    {   0.00f,1000.0f,   0.0f, 1.0f },  /*for DynamicHZX*/
    {1000.00f, 500.0f,1000.0f, 0.0f },
    {   0.00f, 500.0f,   0.0f, 1.0f }, 
    { 420.00f,1000.0f,1000.0f, 0.0f },
    {-170.00f,1000.0f,   0.0f, 1.0f }, 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
static FVECTOR Hzd_2MBoxG[] =
{
    {1000.00f, 600.0f,1000.0f, 0.0f },
    {   0.00f, 600.0f,   0.0f, 1.0f },  /*for DynamicHZX*/
    {1000.00f, 500.0f,1000.0f, 0.0f },
    {   0.00f, 500.0f,   0.0f, 1.0f }, 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
static FVECTOR Hzd_2MBoxH[] =
{
    { 500.00f, 600.0f,1000.0f, 0.0f },
    {-500.00f, 600.0f,   0.0f, 1.0f },  /*for DynamicHZX*/
    {1000.00f, 100.0f,1000.0f, 0.0f },
    {   0.00f, 100.0f,   0.0f, 1.0f }, 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;
static FVECTOR Hzd_2MBoxI[] =
{
    {   0.00f,   0.0f,   0.0f, 0.0f },
    {   0.00f,   0.0f,   0.0f, 1.0f },  /*for DynamicHZX*/
    {1000.00f, 100.0f,1000.0f, 0.0f },
    {   0.00f, 100.0f,   0.0f, 1.0f }, 
    {    0.0f,   0.0f,   0.0f,-1.0f }, 
} ;


static FVECTOR Hzd_1MDrum[] =
{
    { 250.0f, 500.0f, 250.0f, 0.0f }, /*size*/
    {   0.0f, 500.0f,   0.0f, 1.0f }, /*offs*/ /*for DynamicHZX*/
    { 200.0f, 375.0f, 200.0f, 0.0f }, 
    {   0.0f, 250.0f,   0.0f, 1.0f },
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;

static FVECTOR Hzd_1MBrkDrum[] =
{
    { 250.0f, 500.0f, 250.0f, 0.0f }, /*size*/
    {   0.0f,   0.0f,   0.0f, 1.0f }, /*offs*/ /*for DynamicHZX*/
    { 250.0f, 100.0f, 250.0f, 0.0f }, 
    {   0.0f,   0.0f,   0.0f, 1.0f },
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;

static FVECTOR Hzd_1MExplDrumR[] =
{
    { 750.0f, 625.0f, 450.0f, 0.0f }, 
    { 480.0f,   0.0f,-120.0f, 1.0f },  /*for DynamicHZX*/
    { 250.0f,1000.0f, 250.0f, 0.0f },
    {   0.0f,   0.0f,   0.0f, 1.0f },
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;

static FVECTOR Hzd_1MExplDrumL[] =
{
    { 750.0f, 625.0f, 450.0f, 0.0f }, 
    {-480.0f,   0.0f,-120.0f, 1.0f },  /*for DynamicHZX*/
    { 250.0f,1000.0f, 250.0f, 0.0f },
    {   0.0f,   0.0f,   0.0f, 1.0f },
    {   0.0f,   0.0f,   0.0f,-1.0f }, 
} ;


static FVECTOR IronBoxFront = {0.0f,250.0f, 500.0f,400.0f} ;
static FVECTOR IronBoxRear  = {0.0f,250.0f,-500.0f,300.0f} ;

static FVECTOR ForkLift1st = {0.0f,400.0f,500.0f,400.0f} ;
static FVECTOR ForkLift2nd = {0.0f,400.0f,500.0f,400.0f} ;
static FVECTOR ForkLift3rd = {0.0f,400.0f,500.0f,400.0f} ;

#define BOX_SHADOW  "w11c2_box_backclip,w11c2_box_backclip2,w11c2_box_backclip3,w11c2_box_backclip4"
#define DRUM_SHADOW "w11c2_drum_bc"
#define FORKLIFT    "w11c2_forklift_bc_a:w11c2_forklift_bc_b"
#define CONT_SHADOW "w11c2_contena_bc"
#define CON4_SHADOW "w11c2_contena_brk4_bc_b:w11c2_contena_brk4_bc_a"
#define CON5_SHADOW "w11c2_contena_brk4_bc_b:w11c2_contena_brk5_bc_a:w11c2_contena_brk5_bc_b"
#define CON6_SHADOW "w11c2_contena_brk6_bc_a:w11c2_contena_brk6_bc_b:w11c2_contena_brk5_bc_b"
#define CON7_SHADOW "w11c2_contena_brk7_bc_a:w11c2_contena_brk5_bc_b"
#define CON8_SHADOW "w11c2_contena_brk8_bc_a"


extern int BRK_FRT_OBJ_BreakInitForklift( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitIronThrough( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitWoodenBox( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitIronBox( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitGasTank( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitBombTank( HIDE *hide, FVECTOR *pos ) ;
extern int BRK_FRT_OBJ_BreakInitGasTankBroken( HIDE *hide, FVECTOR *pos ) ;

int (*BRK_FRT_OBJ_InitBreak[])( HIDE *hide, FVECTOR *pos ) =
{
    NULL,                                  /* ダミー */
    NULL, BRK_FRT_OBJ_BreakInitWoodenBox,  /* 木箱 AB      */

    NULL,                                  /* 1M鉄箱 ABCD  */
    BRK_FRT_OBJ_BreakInitIronThrough,
    BRK_FRT_OBJ_BreakInitIronThrough,
    BRK_FRT_OBJ_BreakInitIronBox,

    NULL, NULL, NULL, NULL, NULL,          /* 2M鉄箱 ABCDE */
    NULL, NULL, NULL, NULL,                /* 2M鉄箱 FGHI  */

    NULL, NULL,                            /* ドラム缶 ABC BEK */
    BRK_FRT_OBJ_BreakInitGasTank,
    BRK_FRT_OBJ_BreakInitGasTankBroken,

    NULL,                                  /* フォークリフト */
    BRK_FRT_OBJ_BreakInitForklift,
    BRK_FRT_OBJ_BreakInitForklift,
    BRK_FRT_OBJ_BreakInitForklift,

    NULL, NULL,                            /* 爆発缶（右） ABC BEK */
    BRK_FRT_OBJ_BreakInitBombTank,
    BRK_FRT_OBJ_BreakInitBombTank,

    NULL, NULL, NULL,                      /* 鉄箱1M(横壊れ編) */

    NULL, NULL,                            /* 爆発缶（左） ABC BEK */
    BRK_FRT_OBJ_BreakInitBombTank,
    BRK_FRT_OBJ_BreakInitBombTank,

    NULL, NULL,                            /* 不滅箱(1M) */
} ;

TYPE FRT_OBJ_Type[] =
{

    /* ---------- ダミー シリーズ -----------  */
    { ""  ,                /* model     モデル名                             */
      NULL,                /* func      壊れ関数(コールバック関数で呼ばれる) */
      0   ,                /* flag      フラグ  (壊れ関数の引数になる)       */
      0   ,                /* break_id  (未使用)                             */
      0   ,                /* aim_flag  狙いフラグ(Aim系)                    */
      0.0f,                /* magnitude カメラ揺れ（マグニチュード）         */
      NULL,                /* front     前穴位置                             */
      NULL,                /* rear      後ろ穴位置    (事実上,未使用)        */
      NULL,                /* hazard    ハザードモデル(HZXBOX 兼用)          */
      NULL }, /*0 dummy */ /* shadow    影                                   */


    /* ---------- 木箱１M シリーズ -----------  */
    { "w11c2_woodbox",
      NewWoodenBoxBroken ,
      0,
      0,
      FRT_AIM_TypeWoodBox,
      9000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*1 木箱１M */
    { "",
      NULL,
      0,
      0,
      0,
      0.0f,
      NULL   ,
      NULL   ,
      NULL   ,
      NULL },/*2 木箱１M  壊れ状態 */


    /* ---------- 鉄箱１M シリーズ -----------  */
    { "w11c2_ironbox"     ,
      NewIronBoxThrough ,
      0,
      2,
      FRT_AIM_TypeIronBox ,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*3 鉄箱１M */
    { "w11c2_ironbox_brk1",
      NewIronBoxThrough ,
      1,
      1,
      FRT_AIM_TypeIronBox ,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*4 鉄箱１Mへこみ */
    { "w11c2_ironbox_brk2",
      NewIronBoxComplete,
      1,
      0,
      FRT_AIM_TypeIronBox ,
      10000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*5 鉄箱１M貫通   */
    { "w11c2_ironbox_brk3",
       NULL,
      0,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      NULL,
      NULL },/*6 鉄箱１M残り   */


    /* ---------- コンテナ２M シリーズ -----------  */
    { "w11c2_contena",
      NewContainerBroken,
      0,
      3,
      FRT_AIM_TypeContainer,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBox,
      CONT_SHADOW },/*7 A 鉄箱２M       */
    { "w11c2_contena_brk1" ,
      NewContainerBroken,
      0,
      2,
      FRT_AIM_TypeContainer,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBox,
      CONT_SHADOW },/*8 B 鉄箱２Mへこみ */
    { "w11c2_contena_brk2",
      NewContainerBroken,
      0,
      1,
      FRT_AIM_TypeContainer,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBox,
      CONT_SHADOW },/*9 C 鉄箱２Mへこみ */
    { "w11c2_contena_brk3" ,
      NewContainerBroken,
      0,
      0,
      FRT_AIM_TypeContainer,
      3000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBox,
      CONT_SHADOW },/*a D 鉄箱２Mへこみ */
    { "w11c2_contena_brk4",
      NewContainerBroken,
      0,
      0,
      FRT_AIM_TypeContainer,
      3000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBoxE,
      CON4_SHADOW },/*b E 鉄箱２M貫通   */
    { "w11c2_contena_brk5",
      NewContainerBroken,
      0,
      0,
      FRT_AIM_TypeContainer,
      7000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBoxF,
      CON5_SHADOW },/*c F 鉄箱２M貫通   */
    { "w11c2_contena_brk6",
      NewContainerBroken,
      0,
      0,
      FRT_AIM_TypeContainer,
      16000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBoxG,
      CON6_SHADOW },/*d G 鉄箱２M半壊   */
    { "w11c2_contena_brk7",
      NewContainerBroken,
      0,
      0,
      FRT_AIM_TypeContainer,
      9000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_2MBoxH,
      CON7_SHADOW },/*e H 鉄箱２Mほぼ全壊*/
    { "w11c2_contena_brk8" ,
      NULL,
      1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_2MBoxI,
      CON8_SHADOW },/*f I 鉄箱２M全壊   */


    /* ---------- ドラム缶 シリーズ -----------  */
    { "w11c2_drum",
      NewGasTankBroken,
      0,
      0,
      FRT_AIM_TypeGasCan,
      6000.0f,
      NULL,
      NULL,
      Hzd_1MDrum,
      DRUM_SHADOW  },/*10 ガスタンク    */
    { "w11c2_drum_brk1",
      NULL,
      0,
      0,
      FRT_AIM_TypeGasCan,
      0.0f,
      NULL,
      NULL,
      NULL,
      DRUM_SHADOW },/*11 ガスタンク ふっ飛び */
    { "w11c2_drum_brk2",
      NewGasBottomBroken,
      0,
      0,
      FRT_AIM_TypeGasCan,
      1000.0f,
      NULL,
      NULL,
      Hzd_1MBrkDrum,
      DRUM_SHADOW  },/*12 ガスタンク 黒こげ   */
    { "",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      NULL,
      NULL },/*13 ガスタンク 壊れなくなり */


    /* ---------- フォークリフト シリーズ -----------  */
    { "w11c2_forklift",
      NewForkliftBrokenUpper,
      1,
      1,
      FRT_AIM_TypeForkLift,
      12000.0f,
      &ForkLift1st,
      NULL,
      Hzd_ForkLift,
      FORKLIFT },/*14フォークリフト */
    { "w11c2_forklift_brk1",
      NewForkliftBrokenUpper,
      1,
      0,
      FRT_AIM_TypeForkLift,
      6000.0f,
      &ForkLift2nd,
      NULL,
      Hzd_ForkLift,
      FORKLIFT },/*15フォークリフト */
    { "w11c2_forklift_brk2",
      NewForkliftBrokenFall ,
      1,
      0,
      FRT_AIM_TypeForkLift,
      16000.0f,
      &ForkLift3rd,
      NULL,
      NULL,
      FORKLIFT },/*16フォークリフト */
    { "w11c2_forklift_brk2",
      NULL,
      1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      NULL,
      FORKLIFT },/*17フォークリフト */


    /* ---------- 爆発ドラム缶（右） シリーズ -----------  */
    { "w11c2_explode_drum",
      NewBombTankBroken,
      0,
      0,
      FRT_AIM_TypeExplosiv_R,
      6000.0f,
      NULL,
      NULL,
      Hzd_1MExplDrumR,
      DRUM_SHADOW },/*18 爆破タンク（右側） */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_1MExplDrumR,
      DRUM_SHADOW },/*19 爆破タンク ふっ飛び */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_1MBrkDrum,
      DRUM_SHADOW },/*1a 爆破タンク ふっ飛び */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      NULL,
      NULL },/*1b 爆破タンク 壊れなくなり */


    /* ---------- 鉄箱１M シリーズ(横壊れ編) -----------  */
    { "w11c2_ironbox",
      NewIronBoxBroken,
      2,
      0,
      FRT_AIM_TypeIronBox,
      6000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*1c 鉄箱１M    */
    { "w11c2_ironbox_brk4",
      NewIronBoxComplete,
      1,
      0,
      FRT_AIM_TypeIronBox,
      8000.0f,
      &IronBoxFront,
      &IronBoxRear,
      Hzd_1MBox,
      BOX_SHADOW },/*1d 鉄箱１M傾き*/
    { "w11c2_ironbox_brk3",
      NULL,
      0,
      0,
      FRT_AIM_TypeIronBox,
      0.0f,
      &IronBoxFront,
      &IronBoxRear,
      NULL,
      NULL },/*1e 鉄箱１M残り*/


    /* ---------- 爆発ドラム缶（左） シリーズ -----------  */
    { "w11c2_explode_drum",
      NewBombTankBroken,
      0,
      0,
      FRT_AIM_TypeExplosiv_L,
      6000.0f,
      NULL,
      NULL,
      Hzd_1MExplDrumL,
      DRUM_SHADOW },/*1f 爆破タンク（左側） */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_1MExplDrumL,
      DRUM_SHADOW },/*20 爆破タンク ふっ飛び */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_1MBrkDrum,
      DRUM_SHADOW },/*21 爆破タンク ふっ飛び */
    { "w11c2_explode_drum_brk1",
      NULL,
      -1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      NULL,
      NULL },/*22 爆破タンク 壊れなくなり */


    /* ---------- 鉄箱１M シリーズ(壊れないで黒こげ) -----------  */
    { "w11c2_ironbox",
      NewIronBoxBurn,
      2,
      0,
      0,
      2000.0f,
      NULL,
      NULL,
      Hzd_1MBox,
      BOX_SHADOW },/*23 鉄箱１M(壊れない)*/
    { "w11c2_ironbox_brk4",
      NULL,
      1,
      0,
      0,
      0.0f,
      NULL,
      NULL,
      Hzd_1MBox,
      BOX_SHADOW },/*24 鉄箱１M傾き */

} ;

int FRT_OBJ_TypePool[50] = {
    1,1,1,1,1, 1,1,1,1,1,    1,1,1,1,1, 1,1,1,1,1,
    1,1,1,1,1, 1,1,1,1,1,    1,1,1,1,1, 1,1,1,1,1,
    1,1,1,1,1, 1,1,1,1,1,
} ;


#else

extern void *FRT_OBJ_Work ;
extern TYPE FRT_OBJ_Type[] ;
extern int FRT_OBJ_TypePool[] ;
extern int (*BRK_FRT_OBJ_InitBreak[])( HIDE *hide, FVECTOR *pos ) ;
extern int FRT_BulletEnable ;

#endif
