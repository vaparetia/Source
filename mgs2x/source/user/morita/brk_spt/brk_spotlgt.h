
#define ORGA_N_VITAL 2

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

enum
{
    BRK_SPT_F_TARGETSHOW  = 0x00000001,
    BRK_SPT_PUTW00B       = 0x00000002,
    BRK_SPT_PUTW03A       = 0x00000004,
    BRK_SPT_F_VOL_SHADOW  = 0x00000100,
    BRK_SPT_F_NO_EFFECT   = 0x00000200,
    BRK_SPT_F_OBJS_TARGET = 0x00000400,
    BRK_SPT_F_OBJS_OFFSET = 0x00000800,
    BRK_SPT_F_OBJS_FITALL = 0x00001000,
    BRK_SPT_F_SET_TARGET  = 0x10000000,
    BRK_SPT_F_ALREADYBRK  = 0x20000000,
    BRK_SPT_F_ALREADYOFF  = 0x40000000,
    BRK_SPT_F_PUT_C4      = 0x80000000,
} ;

enum
{
    BRK_SPT_M_BREAKDOWN,
    BRK_SPT_M_FIXUP    ,
    BRK_SPT_M_INVISBLE ,
    BRK_SPT_M_VISIBLE  ,
    BRK_SPT_M_CHANGFLAG,
    BRK_SPT_M_TURNON   ,
    BRK_SPT_M_TURNOFF  ,
} ;


typedef struct work_t
{
    GV_ACT       actor  ;

    DG_DEF      *off    ;
    DG_DEF      *on     ;
    DG_DEF      *broken ;
    DG_OBJS     *objs   ;

    int          where   ;
    int          name    ;
    int          flag    ;
    int          lt_name ;
    int          pr_name ;
    GCL_ARGS     pr_arg  ; /* プロック引数*/


    FVECTOR      trgt   ;
    float        radius ;

    LIT_DEF     *lit    ;

    void        *fade_tex ;

    TARGET       trg_bllt ;
    POWER_TARGET pow_bllt ;
    TARGET       trg_bomb ;
    POWER_TARGET pow_bomb ;
} Work ;

#define MAX_N_LIST 40
static Work *BRK_SPTLGT_List[MAX_N_LIST+1] ; // Added +1 for BRK_SPLGT_AddWorkList
static int   BRK_SPTLGT_n_List = 0 ;

extern void OK_LightPos_PutCameraDrop( FVECTOR *fvec ) ;       /* 岡嶋さんの主観カメラ水滴 */
extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot ) ; /* okajima/effect/lit_man.c   */
extern void OK_RemoveDynamicLight( FVECTOR *pos );             /* okajima/effect/lit_man.c   */
extern void *NewSpotRain() ;
extern void *NewPutTexFadeOnLight( int name, int where,        /* ボンボリキャラ */
				   int id_t, int id_u, int id_v, int alpha,
				   float radius0, float radius1, float radius_core,
				   FVECTOR *center, float zoom_max, int prim_flag ) ;
extern void *NewBreakLight( FVECTOR *pos, FVECTOR *dir, int where ) ;
