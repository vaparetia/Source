//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vrspot.c
	スポットライトエフェクト

	2002/04/15 Y.Korekado
	$Id: vrspot.c,v 1.1.1.3 2002/11/19 11:44:06 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

/* ---------------------------------------------------------------- */
static float Penumbra = 0.99998496 ; /* cosf( 18.0F / 180.0F * 3.14159 ) ; */
static float Range = 8000.0F ;
static	short	LightR = 230, LightG = 190, LightB = 120 ;
/* ---------------------------------------------------------------- */

static FVECTOR MirrorPos= {0.0f, 0.0f, 0.0f, 0.0f } ;
static int     MirrorAxis  = 0 ;
static float   MirrorRange = 0.0f ;


typedef	struct	{
	GV_ACT		actor ;
	DG_SPOT		spot ;
	FMATRIX		*root ;
	float		range ;
	float		angle ;
} Work ;
/* ---------------------------------------------------------------- */
int SCN_SetVRSpotParam( void )
{
	float	f_value ;
	
	if ( GCL_GetOption( 'p' ) != NULL ) {
		f_value = ( float )GCL_GetNextInt() ;
		Penumbra = cosf( f_value / 180.0F * 3.14159 ) ;
	}
	if ( GCL_GetOption( 'r' ) != NULL ) {
		Range = ( float )GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ) {
		LightR = ( short )GCL_GetNextInt() ;
		LightG = ( short )GCL_GetNextInt() ;
		LightB = ( short )GCL_GetNextInt() ;
	}

	MirrorRange = 0.0f ;//鏡は切っておく

	return 1 ;
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	static FVECTOR	light_offset = {0.0f,-300.0f,0.0f,1.0f};
	FMATRIX *spot = &work->spot.world ;

	work->spot.world = *( work->root ) ;
	_sceVu0ApplyMatrix( (FVECTOR*)&spot->m[3][0], &work->spot.world, &light_offset );
	*(FVECTOR*)spot->m[1] = *(FVECTOR*)work->root->m[2] ;
	*(FVECTOR*)spot->m[2] = *(FVECTOR*)work->root->m[1] ;
	spot->m[2][0] *= -1.0f ;
	spot->m[2][1] *= -1.0f ;
	spot->m[2][2] *= -1.0f ;

	work->angle = Penumbra ;
	work->range = Range ;
	work->spot.color = LightR | ( LightG << 8 ) | ( LightB << 16 ) | ( 0x80 << 24 ) ;

	DG_SetTmpSpotLight( (FVECTOR*)spot->m[3], (FVECTOR*)spot->m[2],
					   work->range, 1.0f, cosf( work->angle ), work->spot.color, LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY );

	/* 鏡処理 */
	if ( MirrorRange > 0.0f ) {
		FVECTOR pos, dir ;
		float leng ;

		_sceVu0CopyVector( &dir, (FVECTOR*)spot->m[2] ) ;
		switch( MirrorAxis ) {
		  case 0: //X-Axis
			leng = spot->m[3][0] - MirrorPos.vx ;
			_sceVu0ScaleVector( &pos, &dir, leng ) ;
			_sceVu0AddVector( &pos, &pos, spot->m[3] ) ;
			dir.vx *= -1.0f ;
			break ;
			
		  case 1: //Y-Axis
			leng = spot->m[3][1] - MirrorPos.vy ;
			_sceVu0ScaleVector( &pos, &dir, leng ) ;
			_sceVu0AddVector( &pos, &pos, spot->m[3] ) ;
			dir.vy *= -1.0f ;
			break ;
			
		  case 2: //Z-Axis
			leng = spot->m[3][2] - MirrorPos.vz ;
			_sceVu0ScaleVector( &pos, &dir, leng ) ;
			_sceVu0AddVector( &pos, &pos, spot->m[3] ) ;
			dir.vz *= -1.0f ;
			break ;
		}
		if ( MirrorRange > leng ) {
			DG_SetTmpSpotLight( &pos, &dir, work->range, 1.0f,
							   cosf( work->angle ), work->spot.color,
							   LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY );
		}
	}
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *root )
{
	work->root = root ;
	work->spot.world = *root ;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewUSPLight( FMATRIX *mat, float umbra, float penumbra, float range, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

int NewCOM_USPMirror( void )
{

	if ( GCL_GetOption( 'p' ) != NULL ) {
		MirrorPos.vx = ( float )GCL_GetNextInt() ;
		MirrorPos.vy = ( float )GCL_GetNextInt() ;
		MirrorPos.vz = ( float )GCL_GetNextInt() ;
	} else {
		MirrorPos.vx =
		MirrorPos.vy =
		MirrorPos.vz = 0.0f ;
	}

	if ( GCL_GetOption( 'a' ) != NULL ) {
		MirrorAxis = GCL_GetNextInt() ;
	} else {
		MirrorAxis = 0 ;
	}

	if ( GCL_GetOption( 'r' ) != NULL ) {
		MirrorRange = ( float )GCL_GetNextInt() ;
	}else {
		MirrorRange = 4000.0f ;
	}
	return 0 ;
}
