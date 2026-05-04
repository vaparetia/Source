/*
   brk_hang_light.h
   ライト揺れ用 定義ファイル

   1999/12/13 T.Morita
   $Id: brk_hang_light.h,v 1.1.1.3 2002/11/19 11:45:30 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;



#define BRK_HZX_SPHERE   2000
#define BRK_HZX_R_SPHERE 200
#define BRK_DUST_GRAVITY 1
#define BRK_GRAVITY      6
#define BRK_MAX_MDL      5

#define BRK_BOUND_X  3000 
#define BRK_BOUND_Z  3000 

#define BRK_VEL_R 16
#define BRK_ROT_R 16

enum 
{
    BRK_MOD_FINISHED = 0,
    BRK_MOD_MOVE,
} ;
enum
{
    BRK_HLGT_M_INVISBLE=0,
    BRK_HLGT_M_VISIBLE   ,
} ;

typedef struct work_t
{
    GV_ACT    actor  ;

    DG_OBJS  *objs   ;
    int       mode   ;
    int       name   ;
	int       where  ;

    short     rot_xyz[BRK_MAX_MDL][XYZ]  ;
    short     rotv_xyz[BRK_MAX_MDL][XYZ] ;
    FVECTOR   pos ; /* 位置     */
    SVECTOR   rot ; /* 位置     */

    TARGET       target ;
    POWER_TARGET power  ;
} Work ;


extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void RotateMatrixXYZ( FMATRIX *out, FMATRIX *in, short rot[XYZ] )
{
    int r ;
    r = rot[X] & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot[Y] & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot[Z] & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
