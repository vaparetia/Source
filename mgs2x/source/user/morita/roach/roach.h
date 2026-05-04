/*
  roach.h
  ゴキブリ ヘッダファイル

  2000/04/23 T. Morita
  $Id: roach.h,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#include "../brk_hzd/brk_hazard.h"

#define RCH_DANGER_RAD 3000
#define RCH_AIMED_RAD  1000
#define RCH_HIDE_RAD   500

#define RCH_WANDER_SPEED  10.0f
#define RCH_RUNAWAY_SPEED 15.0f
#define RCH_FLY_SPEED     8.0f

#define RCH_MAX_E_SPOT 10
#define RCH_MAX_H_SPOT 10

#define PERROR(...) { printf(__VA_ARGS__) ; return -1 ; }
#define BODYWORLD(a,b) ((a)->objs->objs[(b)].world)

enum
{
    RCH_F_MOVE_PEEIR  = 0x00000001,
    RCH_F_MOVE_WINGS  = 0x00000002,
} ;


typedef struct work_t  Work  ;
typedef struct roach_t Roach ;

struct roach_t
{
    Work     *work   ;
    DG_OBJS  *objs   ;

    void (*act)( Work *work, Roach *, int id ) ;

    short     rot_x,  rot_y  ;
    short     rot_vx, rot_vy ;
    FVECTOR   pos    ; /* 位置     */
    FVECTOR   pos_v  ; /* 速度     */

    FMATRIX   floor  ;

    u_int     flag   ;
    int       tics   ;

    FMATRIX   lights[2] ;

    TARGET    target ;
    POWER_TARGET power  ;
} ;


struct work_t
{
    GV_ACT   actor   ;

    DG_DEF  *def     ;

    int      flag    ;
    int      name    ;
    float    danger  ;

    int      n_roach ;
    Roach   *roach   ;

    FVECTOR e_spot[RCH_MAX_E_SPOT]   ; /* 発生場所 */
    int     n_e_spot ;
    FVECTOR h_spot[RCH_MAX_H_SPOT]   ; /* 隠れ場所 */
    int     n_h_spot ;

    HZD_BOX *hzd     ;
} ;


/*
  Roach 
 */


/* roach_act.c */
extern void RCH_ActThink( Work *work, Roach *r, int id ) ;

/* roach_ini.c */
extern FMATRIX RCH_MatrixTable[16][16] ;
extern int RCH_InitMatrixTable( Work *work ) ;
extern int RCH_InitRoaches( Work *work, int map ) ;
extern int RCH_GetOptions( Work *work, int name, int where ) ;

/* roach_mgs.c */
extern void RCH_Message( Work *work ) ;

/* roach_rch.c */
extern void RCH_Recognize( Work *work, Roach *r, int id ) ;

/* roach_clb.c */
extern void RCH_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */
extern void AN_HeadMark( FVECTOR *pos, int id ) ;


static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_z )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_z & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, float *pos  )
{
    mtx->m[3][X] += pos[X] ;
    mtx->m[3][Y] += pos[Y] ;
    mtx->m[3][Z] += pos[Z] ;
}
