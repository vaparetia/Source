//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   hazard.c
   壁ユーティリティ
   
   2000/10/28 M.Sonoyama
   $Id: hazard.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <devgif.h>

#if 0 //BP_PS2
#include 	<devvif0.h>
#include 	<devvu0.h>
#endif

#endif

#include	"gameheader.h"

#include "bp_vector.h"

/* 重複頂点チェック（Ｃ言語版） */
int			GM_HzxCheckConflict( int n_touches, FVECTOR *mov, HZX_HZD *segs, 
								FVECTOR *near_vecs, signed char *is_edges )
{
	FVECTOR		tmp ;
	int			d1, d2 ;

	if ( n_touches < 2 ) return n_touches ;
	n_touches = 2 ;
	if ( is_edges[ 1 ] == 0 ) {
		if ( is_edges[ 0 ] == 0 ) {
			d1 = GV_VecDir2( near_vecs ) ;
			d2 = GV_VecDir2( near_vecs + 1 ) ;
			if ( GV_DiffDirAbs( d1, d2 ) > 4 ) return n_touches ;
//			printf( "hzxcheckconflict : near_vecs have same dir\n" ) ;
			goto only_one ;
		}
		return n_touches ;
	}
	if ( is_edges[ 0 ] == 1 ) {
		if ( ( DG_FABS( near_vecs[ 0 ].vx - near_vecs[ 1 ].vx ) > 0.10F ) ||
			 ( DG_FABS( near_vecs[ 0 ].vz - near_vecs[ 1 ].vz ) > 0.10F ) ) {
			return n_touches ;
		}
	} else {
		_sceVu0AddVector( &tmp, mov, &near_vecs[ 1 ] ) ;
		if ( ( DG_FABS( tmp.vx - segs[ 0 ].p1.x ) > 0.10F ||
			   DG_FABS( tmp.vz - segs[ 0 ].p1.z ) > 0.10F ) &&
			 ( DG_FABS( tmp.vx - segs[ 0 ].p2.x ) > 0.10F ||
			   DG_FABS( tmp.vz - segs[ 0 ].p2.z ) > 0.10F ) ) {
			return n_touches ;
		}
	}
only_one :
	n_touches = 1 ;
	return n_touches ;
}

/* 反発ベクトル計算（Ｃ言語版） */
void	GM_GetHazardReaction( FVECTOR *react, int n_vecs, FVECTOR *vecs, int r_sphere )
{
	float	len1, len2, spr ;
	float	aoa, bob, aob, axb ;
	FVECTOR	v1, v2, r1, r2 ;

	DG_COPY_VEC( react, &DG_ZeroVector ) ;
	if ( n_vecs == 0 ) return ;
	DG_COPY_VEC( &v1, vecs ) ;
	DG_COPY_VEC( &v2, vecs + 1 ) ;
	v1.vy = v2.vy = 0.0F ;
	len1 = GV_VecLen3F( &v1 ) ;
	spr = ( float )r_sphere ;
	if ( len1 >= spr || len1 <= 0.0F ) return ;
	if ( n_vecs == 2 ) {
		len2 = GV_VecLen3F( &v2 ) ;
		if ( len2 >= spr ) goto single ;
		GV_LenVec3F( &v1, &r1, len1, spr ) ;
		GV_LenVec3F( &v2, &r2, len2, spr ) ;
		_sceVu0SubVector( &r1, &r1, &v1 ) ;
		_sceVu0SubVector( &r2, &r2, &v2 ) ;
		aoa = _sceVu0InnerProduct( &r1, &r1 ) ;
		bob = _sceVu0InnerProduct( &r2, &r2 ) ;
		aob = _sceVu0InnerProduct( &r1, &r2 ) ;
		if ( aoa > aob && bob > aob ) {
			axb = -( r1.vz * r2.vx - r2.vz * r1.vx ) ;
			if ( DG_FABS( axb ) > 0.10F ) {
				react->vx = ( r1.vz * bob - r2.vz * aoa ) / axb ;
				react->vz = ( r2.vx * aoa - r1.vx * bob ) / axb ;
			} else {
				goto single ;
			}
			return ;
		}
	}
single :
	GV_LenVec3F( &v1, react, len1, spr ) ;
	_sceVu0SubVector( react, &v1, react ) ;
}

/*----------------------------------------------------------------*/

#ifdef DEBUG_MODE

typedef	struct	{
	GV_ACT		actor ;
	int			n_touches ;
	FVECTOR		pos ;
	HZX_HZD		segs[ 2 ] ;
} VNI_Work ;

static	void	VNI_Act( VNI_Work *work )
{
	if ( GV_PadData[ 1 ].press & PAD_B ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	DEBUG_Locate( 16, 360, 0 ) ;
	DEBUG_Printf( "%.2f, %.2f, %.2f\n", work->pos.vx, work->pos.vy, work->pos.vz ) ;
	DEBUG_Printf( "%.0f %.0f %.0f %.0f - ", work->segs[ 0 ].p1.x, work->segs[ 0 ].p1.y,
				   work->segs[ 0 ].p1.z, work->segs[ 0 ].p1.h ) ;
	DEBUG_Printf( "%.0f %.0f %.0f %.0f\n", work->segs[ 0 ].p2.x, work->segs[ 0 ].p2.y,
				   work->segs[ 0 ].p2.z, work->segs[ 0 ].p2.h ) ;
	HZX_ViewSegment( &work->segs[ 0 ] ) ;
	if ( work->n_touches < 2 ) return ;
	DEBUG_Printf( "%.0f %.0f %.0f %.0f - ", work->segs[ 1 ].p1.x, work->segs[ 1 ].p1.y,
				   work->segs[ 1 ].p1.z, work->segs[ 1 ].p1.h ) ;
	DEBUG_Printf( "%.0f %.0f %.0f %.0f\n", work->segs[ 1 ].p2.x, work->segs[ 1 ].p2.y,
				   work->segs[ 1 ].p2.z, work->segs[ 1 ].p2.h ) ;
	HZX_ViewSegment( &work->segs[ 1 ] ) ;
}

void	*GM_ViewNarrowInfo( int n_touches, FVECTOR *mov, HZX_HZD *segs ) 
{
	VNI_Work	*work ;

	if ( !GM_DebugModeEnable ) return NULL ;
	work = ( VNI_Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( VNI_Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, VNI_Act, NULL ) ;
		DG_COPY_VEC( &work->pos, mov ) ;
		work->n_touches = n_touches ;
		work->segs[ 0 ] = segs[ 0 ] ;
		if ( n_touches > 1 ) work->segs[ 1 ] = segs[ 1 ] ;
	}
	return work ;
}

#endif
