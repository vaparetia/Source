#ifndef _SPSH_H_
#define _SPSH_H_

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#define SPSH_MAX_BRK  8
#define SPSH_MAX_FOG  4
#define SPSH_MAX_SPSH 20
#define SPSH_MAX_DRP  8

#define SPSH_PRIM_POLY_FLAG (DG_PRIM2_POLY |\
                             DG_PRIM2_SHADE|\
                             DG_PRIM2_TEX  |\
                             DG_PRIM2_ALPHA)

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
enum
{
    R=0, G, B, K, RGB=K, RGBK
} ;

typedef struct volume1_t
{
    DG_PRIM2 *prim ;
    FVECTOR   pos [SPSH_MAX_BRK*2]     ;/* <vw> will be alpha as int */
    SVECTOR   vel [SPSH_MAX_BRK]       ;/* <pad> will be next index */
    char      rgb[RGBK] ;/* K is alpha channel */
    float     width ;
    float     size  ;
    int       n_idx ;
} VOL1 ;

typedef struct volume2_t
{
    DG_PRIM2 *prim ;
    float   alph ;
    float   size ;
    int     n_idx ;
    SVECTOR vel [SPSH_MAX_SPSH]       ;/* <pad> will be next index */
} VOL2 ;

typedef	struct work_t
{
    GV_ACT_EX actor ;

    int    base_alp ;

    VOL1   spsh  ;
    VOL2   bubl  ;
    VOL2   fog  ;

    short *life ;
} Work ;

extern void SPH_ActSPSH( Work *w ) ;


#ifdef _MAIN_FILE_
#else
#endif /*_MAIN_FILE_*/

#endif  /*_SPSH_H_*/
