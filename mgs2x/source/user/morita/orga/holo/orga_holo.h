/*
   orga_hol.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_holo.h,v 1.1.1.3 2002/11/19 11:46:24 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#include "../include/orga_se.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

/* orga.hにも反映させること */
enum
{
    ORGA_HOL_COVERED=0,/* horo_def.anm */
    ORGA_HOL_ORG_OPEN, /* horo_bo.anm  */
    ORGA_HOL_ORG_FREE, /* horo_bod.anm */
    ORGA_HOL_ORG_GONE, /* horo_box.anm */
    ORGA_HOL_PLY_OPNL, /* horo_lo.anm */
    ORGA_HOL_PLY_FREL, /* horo_lod.anm */
    ORGA_HOL_PLY_GONL, /* horo_lox.anm */
    ORGA_HOL_PLY_OPNR, /* horo_ro.anm */
    ORGA_HOL_PLY_FRER, /* horo_rod.anm */
    ORGA_HOL_PLY_GONR, /* horo_rox.anm */

    ORGA_HOL_N_FLAGS,
} ;


#ifdef PSX2
#define ORGA_HOL_N_VERTS 263
#else
#define ORGA_HOL_N_VERTS 279 /* CVYの頂点数とＣＶ２の頂点数がちがっていた */
#endif
#define ORGA_HOL_BUFSIZE ( sizeof(short) * XYZ * ORGA_HOL_N_VERTS )

#define ORG_HOLO_RAD 400.0f




typedef struct holowork_t
{
    GV_ACT       actor  ;

    int          map    ;
    int          name   ;
    CV2_DEF     *cdef   ;
    DG_VERTS_ANIME v_anm ;

    int          flag   ;
    int          id[ORGA_HOL_N_FLAGS] ;
    int          next   ;

    short        key    ;
    short        key_idx ;
    short        ratio  ;
    u_char       rate   ;
    u_char       tgl    ;
    int          tics   ;

    DG_OBJS     *objs   ;
    DG_OBJS     *objs_r ;

    short       *buffer[2] ; /* 解凍バッファ   */
    void        *lzh       ; /* 解凍ハンドラー */

    TARGET       bul_wall ;/* 弾を止めるためのターゲット */
    TARGET       target ;
    POWER_TARGET power  ;
    FVECTOR      himo_l ;
    FVECTOR      himo_r ;
    FVECTOR      vel    ;
    FVECTOR      pos    ;

    short        life_l ; /* 紐の耐久力 */
    short        life_r ;

    void        *splash ;

    int          pr_name ;
#if DEBUG_MODE
    FMATRIX      org_mtx ;
#endif
} Work ;


typedef struct key_t
{
    short  key_frm ;
    short  nxt_idx ;
    u_char rate    ;
    u_char nxt_flg ;
} KEY ;


extern void AN_Test_Eye2( FVECTOR *, int ) ;
extern void *NewObjectSplash( DG_OBJS *objs, int model_id, DG_VERTS_ANIME *anm ) ;
extern void ORG_SPH_VisibleSplash( void *, int ) ;

/* orga_hol_msg.c */
extern void ORG_HOL_Message( Work *work ) ;

/* orga_hol_act.c */
extern void ORG_HOL_ActHoloDestroy( Work *work ) ;
extern void ORG_HOL_ActSound( Work *work ) ;
extern void ORG_HOL_ActDecode( Work *work ) ;
extern void ORG_HOL_ActTurnOffMirror( Work *work ) ;
extern void ORG_HOL_ActHoloGone( Work *work ) ;

/* orga_hol_ini.c */
extern int ORG_HOL_InitHoloParam( Work *work ) ;
extern int ORG_HOL_InitLzsh( Work *work, int id ) ;
extern int ORG_HOL_GetOptions( Work *work, int name, int where ) ;

/* orga_hol_clb.c */
extern void ORG_HOL_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;



static inline void ZeroVector( FVECTOR *a )
{
    vu0_Clrv0() ;
    vu0_Stv0( a ) ;
}



#ifdef _MAIN_FILE_
KEY ORG_HOL_Frames[]=
{
    /* ORGA_HOL_COVERED   0 */
    { 43, 0, 256/4, ORGA_HOL_COVERED },
    /* ORGA_HOL_ORG_OPEN  1 */
    { 1, 2, 256/4, ORGA_HOL_ORG_OPEN }, { 146/2, 3, 256/4, ORGA_HOL_ORG_FREE },
    /* ORGA_HOL_ORG_FREE  3 */
    { 57, 3, 256/16, ORGA_HOL_ORG_FREE },
    /* ORGA_HOL_ORG_GONE  4 */
    { 1, 5, 256/8, ORGA_HOL_ORG_GONE },  {111/2, -1, 256/8, ORGA_HOL_ORG_GONE/*ORGA_HOL_COVERED*/ },

    /* ORGA_HOL_PLY_OPNL  6 */
    { 1, 7, 256/8, ORGA_HOL_PLY_OPNL }, { 39/2, 8, 256/4 , ORGA_HOL_PLY_FREL },
    /* ORGA_HOL_PLY_FREL  8 */
    { 58, 8, 256/4 , ORGA_HOL_PLY_FREL },
    /* ORGA_HOL_PLY_GONL  9 */
    { 1, 10, 256/8, ORGA_HOL_PLY_GONL },  {110/2, -1, 256/8, ORGA_HOL_PLY_GONL/*ORGA_HOL_COVERED*/ },

    /* ORGA_HOL_PLY_OPNR 11 */
    { 1,12, 256/8, ORGA_HOL_PLY_OPNR }, { 47/2,13, 256/4 , ORGA_HOL_PLY_FRER },
    /* ORGA_HOL_PLY_FRER 13 */
    { 61,13, 256/4 , ORGA_HOL_PLY_FRER },
    /* ORGA_HOL_PLY_GONR 14 */
    { 1, 15, 256/8, ORGA_HOL_PLY_GONR },  {115/2, -1, 256/8, ORGA_HOL_PLY_GONR/*ORGA_HOL_COVERED*/ },

} ;

int ORG_HOL_PlyFreeRgtIdx[] = {
    11,/* ORGA_HOL_COVERED  */
    0 ,/* ORGA_HOL_ORG_OPEN */
    4 ,/* ORGA_HOL_ORG_FREE */
    0 ,/* ORGA_HOL_ORG_GONE */
    0 ,/* ORGA_HOL_PLY_OPNL */
    9 ,/* ORGA_HOL_PLY_FREL */
    0 ,/* ORGA_HOL_PLY_GONL */
    0 ,/* ORGA_HOL_PLY_OPNR */
    0 ,/* ORGA_HOL_PLY_FRER */
    0 ,/* ORGA_HOL_PLY_GONR */
} ;

int ORG_HOL_PlyFreeLftIdx[] = {
    6 ,/* ORGA_HOL_COVERED  */
    0 ,/* ORGA_HOL_ORG_OPEN */
    0 ,/* ORGA_HOL_ORG_FREE */
    0 ,/* ORGA_HOL_ORG_GONE */
    0 ,/* ORGA_HOL_PLY_OPNL */
    0 ,/* ORGA_HOL_PLY_FREL */
    0 ,/* ORGA_HOL_PLY_GONL */
    14,/* ORGA_HOL_PLY_OPNR */
    14,/* ORGA_HOL_PLY_FRER */
    0 ,/* ORGA_HOL_PLY_GONR */
} ;

int ORG_HOL_OrgaFreeIdx[] = {
    1,/* ORGA_HOL_COVERED  */
    0,/* ORGA_HOL_ORG_OPEN */
    0,/* ORGA_HOL_ORG_FREE */
    0,/* ORGA_HOL_ORG_GONE */
    0,/* ORGA_HOL_PLY_OPNL */
    0,/* ORGA_HOL_PLY_FREL */
    0,/* ORGA_HOL_PLY_GONL */
    0,/* ORGA_HOL_PLY_OPNR */
    0,/* ORGA_HOL_PLY_FRER */
    0,/* ORGA_HOL_PLY_GONR */
} ;

Work *ORG_HOL_Work = NULL ;

#else

extern KEY ORG_HOL_Frames[] ;

extern int ORG_HOL_PlyFreeRgtIdx[] ;
extern int ORG_HOL_PlyFreeLftIdx[] ;
extern int ORG_HOL_OrgaFreeIdx[]   ;

extern Work *ORG_HOL_Work ;

#endif
