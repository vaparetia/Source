//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_bullet.c
   フォーチュン専用 弾発射キャラ

   2000/10/03 T. Morita
   $Id: fort_bullet.c,v 1.1.1.3 2002/11/19 11:45:57 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

extern FVECTOR FRT_LNR_NozzleOfst ;

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

/* 弾 */
void *NewFortBullet( FMATRIX *world, u_int type, u_int side, u_int size, 
		     u_int damage, u_int length, u_int speed, int weapon ) ;

void *NewFortBulletDemo( DG_OBJS *weapon, FVECTOR *aim ) 
{
    FMATRIX dir = DG_UnitMatrix ;

    _sceVu0ApplyMatrix( (FVECTOR*)dir.m[W], &weapon->world, &FRT_LNR_NozzleOfst ) ;
    _sceVu0SubVector( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[W], aim ) ;
    _sceVu0Normalize( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[Y] ) ;
    return NewFortBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE, PLAYER_SIDE,
			  25, 20, 15000, 800, WP_Nikita ) ;
}
