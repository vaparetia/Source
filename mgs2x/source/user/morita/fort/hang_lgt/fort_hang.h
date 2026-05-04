/*
   brk_hang_light.h
   ライト揺れ用 定義ファイル

   1999/12/13 T.Morita
   $Id: fort_hang.h,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#define MAKING 0

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define FRT_MAX_MDL      5

enum 
{
    FRT_MOD_FINISHED = 0,
    FRT_MOD_MOVE,
} ;
enum
{
    FRT_HNG_M_INVISBLE=0,
    FRT_HNG_M_VISIBLE   ,
} ;

typedef struct work_t
{
    GV_ACT_EX actor  ;

    DG_OBJS  *objs   ;
    int       mode   ;
    int       name   ;
    int       flag   ;

    short     rot_xyz[FRT_MAX_MDL][XYZ]  ;
    short     rotv_xyz[FRT_MAX_MDL][XYZ] ;
    FVECTOR   pos ; /* 位置     */
    SVECTOR   rot ; /* 位置     */

    TARGET       target ;
    POWER_TARGET power  ;

#if MAKING
    void *wireframe[8] ;
#endif

} Work ;


#ifdef _MAIN_FILE_

int FRT_HNG_CharaName[4] = { 0,0,0,0 } ;

#else

extern int FRT_HNG_CharaName[] ;

#endif

#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

static inline void RotateTransMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot, FVECTOR *pos )
{
    _sceVu0RotMatrixX( out, in , ANGtoRAD(rot->vx) ) ;
    _sceVu0RotMatrixY( out, out, ANGtoRAD(rot->vy) ) ;
    _sceVu0RotMatrixZ( out, out, ANGtoRAD(rot->vz) ) ;
    out->m[W][X] += pos->vx ;
    out->m[W][Y] += pos->vy ;
    out->m[W][Z] += pos->vz ;
}

static inline void RotateMatrixXYZ( FMATRIX *out, FMATRIX *in, short rot[XYZ] )
{
    _sceVu0RotMatrixX( out, in , ANGtoRAD(rot[X]) ) ;
    _sceVu0RotMatrixY( out, out, ANGtoRAD(rot[Y]) ) ;
    _sceVu0RotMatrixZ( out, out, ANGtoRAD(rot[Z]) ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
#if 0
    _sceVu0CopyVectorXYZ( mtx, pos ) ;
#else
    mtx->m[W][X] += pos->vx ;
    mtx->m[W][Y] += pos->vy ;
    mtx->m[W][Z] += pos->vz ;
#endif
}

