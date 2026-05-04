/*
   brk_vending.c 
   プット自動販売機

   1999/12/26 T.Morita
   $Id: brk_vending.h,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#include "../brk_hzd/brk_hazard.h"

#include "../brk_utl/brk_utl.x"



#define BRK_VND_BOUNCE   1.40f
#define BRK_VND_GRAVITY  7.0f
#define BRK_VND_VEL_R    0.8f 

#define BRK_VND_INACTIVE 0x8000
#define BRK_VND_LIFE     3

#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)


typedef struct work_t   Work ;
typedef struct can_t    CAN ;
typedef struct vender_t VENDER ;


/* 缶 構造体 */
struct can_t
{
    TARGET       target ;
    POWER_TARGET power  ;

    FMATRIX  lights[2] ;

    FVECTOR  pos   ;
    FVECTOR  pos_v ;
    SVECTOR  rot   ;
    SVECTOR  rot_v ;

    DG_OBJS *objs   ;

    short    tic   ;
    short    flag  ;
    Work    *work  ;
} ;


/* 自動販売機構造体 */
struct vender_t
{
    TARGET       target ;
    POWER_TARGET power  ;

    DG_OBJS     *objs    ;
    DG_DEF      *vnd_brk ;
    short        life    ;
    short        id      ;
    short        can_num ;
    short        flag    ;

    int          proc   ;
    Work        *work   ;
} ;

struct work_t
{
    GV_ACT       actor  ;

    int          where  ;
    int          name   ;
    LIT_DEF     *lit    ;
    HZD_BOX     *hzd    ;

    /* 缶 */
    DG_DEF      *can_brk ;
    DG_DEF      *can_nrm ;
    CAN         *can     ;
    int          n_can   ;
    int          tic     ;

    /* 自動販売機本体 */
    VENDER      *vender   ;
    int          n_vender ;
} ;


static inline void RotateMatrixYXY( FMATRIX *out, SVECTOR *rot )
{
    _sceVu0RotMatrixY( out, &DG_UnitMatrix, ANGtoRAD(rot->vz) ) ;
    _sceVu0RotMatrixX( out,  out          , ANGtoRAD(rot->vx) ) ;
    _sceVu0RotMatrixY( out,  out          , ANGtoRAD(rot->vy) ) ;
}



/* brk_vnd_act.c */
extern void BRK_VND_ActCan( Work *work ) ;

/* brk_vnd_clb.c */
extern void BRK_VND_VenderCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_VND_CanCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_vnd_ini.c */
extern int      BRK_VND_InitHazard( Work *work ) ;
extern int      BRK_VND_GetOptions( Work *work, int where ) ;
extern int      BRK_VND_InitVending( Work *work, int where ) ;
extern int      BRK_VND_InitCan( Work *work, int where ) ;
extern DG_OBJS *BRK_VND_ChangeModel( Work *work, DG_DEF *def, DG_OBJS *old, FMATRIX *lights ) ;
extern void     BRK_VND_InitTarget( TARGET *t, POWER_TARGET *p, int where,
				    DG_OBJS *objs,
				    void *callback, void *arg ) ;
