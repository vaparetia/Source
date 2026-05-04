//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	spot.c
	スポットライトエフェクト

	1999/10/20 K.Takabe
	$Id: spot.c,v 1.1.1.3 2002/11/19 11:51:13 Yoshizawa1 Exp $

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
typedef	struct	{
	GV_ACT		actor ;
	DG_SPOT		spot ;
	FMATRIX		*root ;
	float		umbra ;
	float		penumbra ;
	float		range ;
	float		angle ;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	static FVECTOR	light_offset = {0.0f,-300.0f,0.0f,1.0f};

	work->spot.world = *( work->root ) ;
	_sceVu0ApplyMatrix( (FVECTOR*)&work->spot.world.m[3][0], &work->spot.world, &light_offset );
	*(FVECTOR*)work->spot.world.m[1] = *(FVECTOR*)work->root->m[2] ;
	*(FVECTOR*)work->spot.world.m[2] = *(FVECTOR*)work->root->m[1] ;
	work->spot.world.m[2][0] *= -1.0f ;
	work->spot.world.m[2][1] *= -1.0f ;
	work->spot.world.m[2][2] *= -1.0f ;
	work->spot.range = work->range ;
	work->spot.angle = cosf( work->angle ) / sinf( work->angle ) ;
	//work->spot.color = 0x80404040 ;

	DG_SetTmpSpotLight( (FVECTOR*)work->spot.world.m[3], (FVECTOR*)work->spot.world.m[2],
					   work->range, 1.0f, cosf( work->angle ), work->spot.color, LIT_FLAG_CHARAONLY );

#if 0
	float	angle ;
	FMATRIX	view_inv ;

	sceVu0InversMatrix( &view_inv, &work->spot.world );
	cp = DG_Chanl( 1 );
	DG_SetDrawEnv( cp, 160, 50, 128, 64 );
	cp->bg_clear_flag = 1 ;
	cp->flag = 1 ;
	cp->eye = work->spot.world ;
	cp->eye_inv = view_inv ;

	angle = cosf( 30.0f * 3.14159265f / 180.0f ) / ( sinf( 30.0f * 3.14159265f / 180.0f ) * 2.0f );

	/*
		透視変換行列を生成する
	*/
	sceVu0ViewScreenMatrix( &cp->pers,
						   angle,
						   1.0f, .5f, 0.0f, 0.0f,
						   (float)0.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	sceVu0ViewScreenMatrix( &cp->pers2,
						   angle*128.0f,
						   1.0f, 0.5f, 2048.0f, 2048.0f,
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	sceVu0ViewScreenMatrix( &cp->raise_pers,
						   angle,
						   1.0f, 0.5f, 0.0f, 0.0f,
						   (float)0.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
	sceVu0ViewScreenMatrix( &cp->raise_pers2,
						   angle*128.0f,
						   1.0f, 0.5f, 2048.0f, 2048.0f,
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
	
	/*
		カメラ透視変換行列を生成する
	*/
	sceVu0MulMatrix( &cp->eye_pers, &cp->pers, &view_inv );
	sceVu0MulMatrix( &cp->eye_pers2, &cp->pers2, &view_inv );
	sceVu0MulMatrix( &cp->raise_eye_pers, &cp->raise_pers, &view_inv );
	sceVu0MulMatrix( &cp->raise_eye_pers2, &cp->raise_pers2, &view_inv );
#endif
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeueSpotObjs( &work->spot );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *root, float umbra, float penumbra, float range, int color )
{

	work->root = root ;
	work->umbra = umbra ;
	work->penumbra = penumbra ;
	work->range = range ;
	//work->angle = 30.0f * 3.14159265 / 180.0f ;
	work->angle = penumbra ;

	work->spot.world = *root ;
	DG_QueueSpotObjs( &work->spot );

	work->spot.range = range ;
	work->spot.angle = cosf( work->angle ) / sinf( work->angle ) ;
	work->spot.color = color ;
	work->spot.shadow_id = 355453/* "投影モデル" */ ;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewSpotLight( FMATRIX *mat, float umbra, float penumbra, float range, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, mat, umbra, penumbra, range, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
