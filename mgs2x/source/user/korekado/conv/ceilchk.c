//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ceilchk.c
	天井君の判定

	2002/07/17 Y.Korekado
	$Id: ceilchk.c,v 1.1.1.3 2002/11/19 11:44:01 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*KR_CeilingCheck( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-h	高さ
----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

/*----------------------------------------------------------------*/
#define MAX_CAMERA	(4)

/*----------------------------------------------------------------*/
static int CameraCheck( FVECTOR *plane, int camera )
{
    GM_CameraSet	*cam ;
    float 	t ;
	int		tmp_flag ;

	cam = GM_GetCurrentCamera( camera ) ;

	t = plane->vx * cam->position.vx + plane->vy * cam->position.vy + 
	  plane->vz * cam->position.vz + plane->vw ;
	tmp_flag = ( t > 0.0f ) ? 1 : 0 ;

	return tmp_flag ;
}

static int Ceiling( FVECTOR *plane )
{
    int i, flag ;
    int invisible ;

	invisible = 0 ;
	for ( i=0; i<MAX_CAMERA; i++ ) {
		if ( GM_CheckCameraActive( i ) ) {
			flag = DG_FLAG_INVISIBLE0 << i ;
			if ( CameraCheck( plane, i ) ) {	/* 天井表示 */
			} else {								/* 天井非表示 */
				invisible |= flag ;
			}
		}
	} 
	
	return invisible ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
int	KR_CeilingCheck( float height )
{
	int	n_plane ;
	FVECTOR check_pos, norm, plane ;
	
	n_plane = 1 ;

	check_pos.vx = 0.0f ;
	check_pos.vy = height ;
	check_pos.vz = 0.0f ;

	/* デフォルトのＹ軸負方向チェックとして設定 */
	norm.vx = 0.0f ;
	norm.vy = -1.0f ;
	norm.vz = 0.0f ;

	/* 平面の定数を求める */
	plane = norm ;
	plane.vw = - ( ( plane.vx * check_pos.vx ) +
							   ( plane.vy * check_pos.vy ) +
							   ( plane.vz * check_pos.vz ) ) ;
//	printf("ceiling.c: no.%d  a=%f b=%f c=%f d=%f\n", i,
//	   work->plane[i].vx, work->plane[i].vy, work->plane[i].vz, work->plane[i].vw );

	return Ceiling( &plane ) ;

}
