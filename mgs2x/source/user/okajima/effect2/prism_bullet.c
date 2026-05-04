//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	prism_bullet.c
	弾道エフェクト：光学迷彩使用

	2001/01/25 S.Okajima
	$Id: prism_bullet.c,v 1.1.1.3 2002/11/19 11:47:22 Yoshizawa1 Exp $

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
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


//#define MODEL_NAME ( 123600 /*"usp"*/ )
//#define MODEL_NAME ( 12130941 /*"shock1"*/ )
//#define MODEL_NAME ( 12130942 /*"shock2"*/ )
//#define MODEL_NAME ( 2202283 /*"water_sphere01"*/ )
//#define MODEL_NAME ( 2202284 /*"water_sphere02"*/ )
//#define MODEL_NAME ( 1114067 /*"water_dome01"*/ )
//#define MODEL_NAME ( 16735093 /*"water_dome01x2"*/ )
//#define MODEL_NAME ( 1114068 /*"water_dome02"*/ )
#define MODEL_NAME ( 16736117 /*"water_dome02x2"*/ )

#define MODEL_SIZE ( 4000.0f )
#define ACT_SCALE  ( 1.03f )

#define COL_R  ( 64.0f )	/* 後で１２８が足される */
#define COL_G  ( 127.0f )	/* 後で１２８が足される */
#define COL_B  ( 64.0f )	/* 後で１２８が足される */
#define INITIAL_ALPHA  ( 255.0f - 64.0f )	/* 何故か６４以上と未満は挙動が違う。後で１２８が足される */

#define SHIFT  ( 256.0f )
#define DIFF_X ( SHIFT )
#define DIFF_Y ( SHIFT / PIXEL_ASPECT )

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;
	int			life_max;
	int			num;

	FMATRIX		optcmf_param ;
	int			flag ;
	int			old_flag ;


	OBJECT		object[0];
} Work ;

/* ---------------------------------------------------------------- */
static void SetParam( Work *work, float ratio )
{
	/* 光学迷彩パラメータの設定 */
	work->optcmf_param.m[0][0] = DRAW_WIDTH  / 2.0f - DIFF_X * ratio ;
	work->optcmf_param.m[0][1] = DRAW_HEIGHT / 2.0f - DIFF_Y * ratio ;
	work->optcmf_param.m[1][0] = DRAW_WIDTH  / 2.0f ;
	work->optcmf_param.m[1][1] = DRAW_HEIGHT / 2.0f ;
	work->optcmf_param.m[2][0] = DIFF_X * ratio ;
	work->optcmf_param.m[2][1] = DIFF_Y * ratio ;
	work->optcmf_param.m[3][0] = COL_R         * ratio + 128.0f;
	work->optcmf_param.m[3][1] = COL_G         * ratio + 128.0f;
	work->optcmf_param.m[3][2] = COL_B         * ratio + 128.0f;
	work->optcmf_param.m[3][3] = INITIAL_ALPHA * ratio + 64.0f;

printf("%4d %4d %4d %4d:",(int)work->optcmf_param.m[0][0],(int)work->optcmf_param.m[0][1],(int)work->optcmf_param.m[1][0],(int)work->optcmf_param.m[1][1]);
printf("%4d %4d %4d %4d:",(int)work->optcmf_param.m[2][0],(int)work->optcmf_param.m[2][1],(int)work->optcmf_param.m[3][0],(int)work->optcmf_param.m[3][1]);
printf("%4d %4d\n",(int)work->optcmf_param.m[3][2],(int)work->optcmf_param.m[3][3]);


}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	OBJECT		*object;
	FVECTOR		*fvtemp;
	int	i;
	float		ratio;

	object = work->object;
	for( i=0; i<work->num; i++ ){
		fvtemp = (FVECTOR *)object->objs->world.m[0];
		fvtemp->vx *= ACT_SCALE ;
		fvtemp->vy *= ACT_SCALE ;
		fvtemp->vz *= ACT_SCALE ;
		fvtemp++;
		fvtemp->vx *= ACT_SCALE ;
		fvtemp->vy *= ACT_SCALE ;
		fvtemp->vz *= ACT_SCALE ;
		fvtemp++;
		fvtemp->vx *= ACT_SCALE ;
		fvtemp->vy *= ACT_SCALE ;
		fvtemp->vz *= ACT_SCALE ;

		DG_SetPos( &object->objs->world );
		GM_ActObject( object );
		object++;
	}

	ratio = (float)(work->life) / (float)(work->life_max);
	ratio = ( ratio < 0.0f )? 0.0f: ratio;

	SetParam( work, ratio );

	if( work->life-- < 0 ) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	OBJECT		*object;
	int	i;

	object = work->object;
	for( i=0; i<work->num; i++ ){
		/* 光学迷彩ＯＦＦ */
		object->flag = work->old_flag ;
		if ( object->objs->extend_data == &work->optcmf_param ) object->objs->extend_data = NULL ;
		GM_FreeObject( object ) ;
		object++;
	}

}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *before_pos, float size, int num, int life )
{
	FVECTOR		diff;
	FVECTOR		fvtemp;
	FMATRIX		fmat;
	SVECTOR		rot;
	OBJECT		*object;
	DG_OBJS		*objs;
	int	i;
	float		ftemp;
	float		scale;

	work->num = num;
	work->life = life;
	work->life_max = life;


	OK_DirVecXY( before_pos, pos, &rot );
	rot.vx -= 1024;
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &fmat );

	scale = size / MODEL_SIZE;

	fmat.m[0][0] *= scale ;
	fmat.m[0][1] *= scale ;
	fmat.m[0][2] *= scale ;
	fmat.m[1][0] *= scale ;
	fmat.m[1][1] *= scale ;
	fmat.m[1][2] *= scale ;
	fmat.m[2][0] *= scale ;
	fmat.m[2][1] *= scale ;
	fmat.m[2][2] *= scale ;

	_sceVu0SubVector( &diff, pos, before_pos ) ;
	_sceVu0ScaleVector( &diff, &diff, 1.0f/(float)(num) );
	DG_COPY_VEC( &fvtemp, &diff );

	object = work->object;
	for( i=0; i<work->num; i++ ){
		GM_InitObject( object, MODEL_NAME, DG_FLAG_SHADE ) ;
		objs = object->objs;
		work->old_flag = objs->flag ;
		DG_VisibleObjs( objs ) ;
		/* 光学迷彩ＯＮ */
		objs->flag |= (DG_FLAG_OPTCMF) ;
		/* パラメータ設定マトリクス */
		objs->extend_data = &work->optcmf_param ;

		DG_COPY_MAT( &objs->world, &fmat );

		ftemp = rnd()+0.5f;
		objs->world.m[1][0] *= ftemp ;
		objs->world.m[1][1] *= ftemp ;
		objs->world.m[1][2] *= ftemp ;

		ftemp = rnd()+0.5f;
		objs->world.m[0][0] *= ftemp ;
		objs->world.m[0][1] *= ftemp ;
		objs->world.m[0][2] *= ftemp ;
		objs->world.m[2][0] *= ftemp ;
		objs->world.m[2][1] *= ftemp ;
		objs->world.m[2][2] *= ftemp ;

		_sceVu0AddVector( (FVECTOR *)objs->world.m[3], before_pos, &fvtemp );
		_sceVu0AddVector( &fvtemp, &fvtemp, &diff ) ;

		object++;
	}
	/* パラメータ設定 */
	SetParam( work, 1.0f );

	return (0);
}


/*
pos から before_pos までの間に 大きさ size の 光学迷彩球を num 個 置く

*/
/* ---------------------------------------------------------------- */
void *NewPrismBullet( FVECTOR *pos, FVECTOR *before_pos, float size, int num, int life )
{
	Work		*work ;
	int			buf_size;

	OPERATOR() ;

	buf_size = sizeof( Work ) + sizeof( OBJECT ) * num;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, pos, before_pos, size, num, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
