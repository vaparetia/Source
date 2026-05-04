/*
        brk_light.h
	物体破壊用 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_light.h,v 1.1.1.3 2002/11/19 11:45:34 Yoshizawa1 Exp $
*/

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define BRK_N_DUST 3
#define BRK_N_LITR 24
#define BRK_N_GLAS 48
#define BRK_N_GLAS_VERTS 3
#define BRK_GRAVITY 2
#define BRK_SIZE   15.0f

#define BRK_HZX_SPHERE   2000
#define BRK_HZX_R_SPHERE 200



typedef struct glas_t
{
    FVECTOR  pos   ;
    FVECTOR  pos_v ;
    float    gravity ;
    short    rot_x ;
    short    rot_y ;
    short    rot_vx ;
    short    rot_vy ;
} GLAS ;

typedef struct work_t
{
    GV_ACT   actor ;

    FMATRIX  root ;
    FVECTOR  vel  ;

    DG_PRIM2 *dust ;
    DG_PRIM2 *glas ;
    GLAS      glas_prof[BRK_N_GLAS] ;

    float    sn ;
    float    cs ;
    float    dx ;
    float    dz ;

    float    ddx ;/*maybe need not*/
    float    ddz ;
    float    dd ;
    int      sgn ;
} Work ;
