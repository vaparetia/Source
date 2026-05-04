/*
        brk_radle.h
	レードル揺れ用 定義ファイル

        1999/12/13 T.Morita
        $Id: demo_equip.h,v 1.1.1.3 2002/11/19 11:45:58 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;



#define PERROR(...) { printf(__VA_ARGS__) ; return -1 ; }

#define BRK_VEL_R 16
#define BRK_ROT_R 16

#define BRK_RDL_INACTIVE 0x8000000

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
    DG_OBJS  *objs  ;

    int      col_id ;/* 今回当たったID */
    FVECTOR  col_p  ;/*当たり中心位置（動いたら計算する）*/

    TARGET       target ;
    POWER_TARGET power  ;
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

/* brk_rdl_ini.c */
extern int  BRK_RDL_GetOptions( Work *work, int name, int where ) ;

/* brk_rdl_act.c */
extern void BRK_RDL_Act( Work *work ) ;
extern void BRK_RDL_CheckCollide( Work *work ) ;

extern void AN_Test_Eye3( FVECTOR *mov ) ; /* for test */

static inline void MakeObjWorld( FMATRIX *out, RADLE *radle )
{
    int r ;

    *out = DG_UnitMatrix ;
    out->m[X][X] *= radle->scale.vx ;
    out->m[Y][Y] *= radle->scale.vy ;
    out->m[Z][Z] *= radle->scale.vz ;

    r = radle->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = radle->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = radle->rot_z & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;

    out->m[3][X] += radle->pos.vx ;
    out->m[3][Y] += radle->pos.vy ;
    out->m[3][Z] += radle->pos.vz ;
}
static inline void MakeInvWorld( FMATRIX *out, RADLE *radle )
{
    int r ;
    FMATRIX scl ;

    *out = DG_UnitMatrix ;
    out->m[3][X] -= radle->pos.vx ;
    out->m[3][Y] -= radle->pos.vy ;
    out->m[3][Z] -= radle->pos.vz ;

    r = -radle->rot_z & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = -radle->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = -radle->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;

    scl = DG_UnitMatrix ;
    scl.m[X][X] /= radle->scale.vx * radle->scale.vx ;
    scl.m[Y][Y] /= radle->scale.vy * radle->scale.vy ;
    scl.m[Z][Z] /= radle->scale.vz * radle->scale.vz ;
    _sceVu0MulMatrix( out, &scl, out ) ;
}

