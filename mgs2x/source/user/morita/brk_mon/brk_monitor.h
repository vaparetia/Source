/*
   brk_monitor.h
   制御室モニター壊れ

   1999/12/13 T. Morita
   $Id: brk_monitor.h,v 1.1.1.3 2002/11/19 11:45:37 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define BRK_MON_N_TYPE  8
#define BRK_MON_N_PRIMS 18

typedef struct motitor_t
{
    float    laster ;

    short    change   ;
    short    type     ;
    short    type_old ;
    u_char  *table    ;
} MON ;

typedef struct work_t
{
    GV_ACT        actor  ;

    TARGET        target ; /* ターゲット */
    POWER_TARGET  power  ;

    int           name   ;
    int           where  ;
    int           proc   ;
    int           flag   ;

    FVECTOR       center ;
    MON           monitor[BRK_MON_N_PRIMS] ;
    short         n_monitor ;
    short         se_tic    ;
    short         se_tgl    ;

    DG_PRIM2     *back ;
    DG_PRIM2     *prim[BRK_MON_N_TYPE] ;
    DG_PRIM2     *line ;

} Work ;


/* brk_mon_ini.c */
extern int  BRK_MON_InitLaster( Work *work, int name, int where ) ;
extern int  BRK_MON_InitMonitor( Work *work, int name, int where ) ;
extern void BRK_MON_MakeShape( FVECTOR *pos, FVECTOR *shape,
			       int i, int j,
			       float offset_h, float offset_v ) ;
extern int  BRK_MON_InitParam( Work *work, int name, int where ) ;
extern void BRK_MON_SetRGB( Work *work, int type, int i, int clock, int on ) ;
extern int BRK_MON_InitTarget( Work *work, int name, int where ) ;
extern void BRK_MON_LasterRGB( Work *work, int type, int i ) ;

/* brk_mon_act.c */
extern void BRK_MON_ActLaster( Work *work ) ;
extern void BRK_MON_ActMonitor( Work *work ) ;

/* brk_mon_clb.c */
extern void BRK_MON_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


#ifdef __MAIN_FILE__


FVECTOR  BRK_MOT_FrameL[] = {
    {-1171.873f,3859.375f,-9484.374f,1.0f},
    {  -78.124f,3859.375f,-9046.874f,1.0f},
    {-1171.873f,2590.625f,-9703.124f,1.0f},
    {  -78.123f,2590.625f,-9265.624f,1.0f},
} ;

FVECTOR BRK_MOT_FrameR[] = {
    { 1171.875f,3859.375f,-9484.374f,1.0f},
    {   78.125f,3859.375f,-9046.874f,1.0f}, 
    { 1171.874f,2590.625f,-9703.124f,1.0f},
    {   78.124f,2590.625f,-9265.624f,1.0f}, 
} ;


#else

extern FVECTOR BRK_MOT_FrameL[] ;
extern FVECTOR BRK_MOT_FrameR[] ;


#endif /* __MAIN_FILE__ */
