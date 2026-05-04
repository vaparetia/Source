/*
        brk_radle.h
	レードル揺れ用 定義ファイル

        1999/12/13 T.Morita
        $Id: brk_radle.h,v 1.1.1.3 2002/11/19 11:45:43 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#define MAKING 0

#define BRK_VEL_R 16
#define BRK_ROT_R 16

#define BRK_RDL_INACTIVE   0x8000000
#define BRK_RDL_MOVE_RATE  0.2f
#define BRK_RDL_MOVE_FORCE 10.0f
enum 
{
    BRK_MOD_FINISHED = 0,
    BRK_MOD_MOVE,
} ;

typedef struct work_t  Work ;
typedef struct radle_t RADLE ;

struct radle_t
{
    short     rot_x,  rot_y,  rot_z  ;/*現在の角度*/
    short     rot_dx, rot_dy, rot_dz ;/*目標角度*/
    short     rot_vx, rot_vy, rot_vz ;/*角速度  */
    FVECTOR   pos   ; /* 位置     */
    FVECTOR   scale ; /* スケール */

    Work     *work  ;
    int       mode  ;
    int       name  ;
    int       where  ;
    DG_OBJS  *objs  ;

    int      col_id ;/* 今回当たったID */
    FVECTOR  col_p  ;/*当たり中心位置（動いたら計算する）*/

    int      se_id ;/* 当たった時のSEID */

    TARGET       target ;
    POWER_TARGET power  ;

#if MAKING
    void *wireframe ;
#endif

} ;

struct work_t
{
    GV_ACT   actor   ;
    int      where   ;
    int      flag    ;

    int      n_radle ;
    RADLE   *radle   ;
} ;


/* brk_rdl_clb.c */
extern void BRK_RDL_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_RDL_SetSound( RADLE *radle, FVECTOR *pos ) ;

/* brk_rdl_ini.c */
extern int  BRK_RDL_GetOptions( Work *work, int name, int where ) ;

/* brk_rdl_act.c */
extern void BRK_RDL_Act( Work *work ) ;
extern void BRK_RDL_CheckCollide( Work *work ) ;

extern void AN_Test_Eye3( FVECTOR *mov ) ; /* for test */

static inline void MakeObjWorld( FMATRIX *out, RADLE *radle, int where )
{
    *out = DG_UnitMatrix ;
    _sceVu0RotMatrixY( out, out, radle->rot_y*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixX( out, out, radle->rot_x*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixZ( out, out, radle->rot_z*(float)M_PI/32768.0F ) ;
    out->m[3][X] += radle->pos.vx ;
    out->m[3][Y] += radle->pos.vy ;
    out->m[3][Z] += radle->pos.vz ;
    GM_MoveTarget2Map( &radle->target, out, where ) ;
    _sceVu0ScaleVector( (FVECTOR*)out->m[X], (FVECTOR*)out->m[X], radle->scale.vx ) ;
    _sceVu0ScaleVector( (FVECTOR*)out->m[Y], (FVECTOR*)out->m[Y], radle->scale.vy ) ;
    _sceVu0ScaleVector( (FVECTOR*)out->m[Z], (FVECTOR*)out->m[Z], radle->scale.vz ) ;
}
#if 1
static inline void MakeInvWorld( FMATRIX *out, RADLE *radle )
{
    FMATRIX scl ;

    *out = DG_UnitMatrix ;
    out->m[3][X] -= radle->pos.vx ;
    out->m[3][Y] -= radle->pos.vy ;
    out->m[3][Z] -= radle->pos.vz ;

    _sceVu0RotMatrixZ( out, out, -radle->rot_z*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixX( out, out, -radle->rot_x*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixY( out, out, -radle->rot_y*(float)M_PI/32768.0F ) ;

    scl = DG_UnitMatrix ;
    scl.m[X][X] /= radle->scale.vx ;
    scl.m[Y][Y] /= radle->scale.vy ;
    scl.m[Z][Z] /= radle->scale.vz ;

    _sceVu0MulMatrix( out, &scl, out ) ;
}
#else
static inline void MakeInvWorld( FMATRIX *out, RADLE *radle )
{
    *out = DG_UnitMatrix ;
    out->m[3][X] -= radle->pos.vx ;
    out->m[3][Y] -= radle->pos.vy ;
    out->m[3][Z] -= radle->pos.vz ;
    _sceVu0RotMatrixY( out, out, -radle->rot_y*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixX( out, out, -radle->rot_x*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixZ( out, out, -radle->rot_z*(float)M_PI/32768.0F ) ;
}
#endif
