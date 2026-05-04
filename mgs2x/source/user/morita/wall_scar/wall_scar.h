/*
        wall_scar.h
            壁／床 弾痕

        1999/11/10 T.Morita
        $Id: wall_scar.h,v 1.1.1.3 2002/11/19 11:46:38 Yoshizawa1 Exp $
*/

#define WLLSCR_MAX_UNIT (10)
#define WLLSCR_MAX_DIFF (5)
#define WLLSCR_SIZE 50


#define WALLSCR_PRIM_FLAG (DG_PRIM2_POLY      |\
			   DG_PRIM2_SHADE     |\
			   DG_PRIM2_TEX       |\
			   DG_PRIM2_SINGLEBUFF|\
			   DG_PRIM2_CULLPOLY  |\
			   DG_PRIM2_ALPHA     | DG_PRIM2_CCW)

#define PUSH_UP 3.0f
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"

typedef struct scar_t SCAR ;
typedef struct work_t Work ;
struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       flag   ; /* 制御フラグ */

    short     n_unit ;
    short     n_scar ;
    struct scar_t
    {
	DG_PRIM2 *prim   ;
	short     n_pos  ;
	short     front  ;
    } *scar ;
} ;

#define MIN(_a,_b) ((_a)< (_b)?(_a):(_b))
#define MAX(_a,_b) ((_a)>=(_b)?(_a):(_b))

extern void *NewWallScarDust( FVECTOR *p,  float vx, float vz ) ;

