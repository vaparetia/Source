//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_blood.c
	デモ血飛沫
	2000/03/23 S.Okajima
	$Id: d_blood.c,v 1.1.1.3 2002/11/19 11:46:51 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"
#include        "libutl.h"
#include	"blood.h"

extern void *NewBlood1_Demo( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int size );
extern void *NewBlood2_Demo( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int size );

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX		*world;
	FVECTOR		force;
	int			size;
	int			time;
	int			time_max;

	int			mode;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	fvtemp;
	FVECTOR	l_force;
	int		size;

	DG_COPY_VEC( &fvtemp,(FVECTOR *)work->world->m[3] );
	if( work->time > 0 ){
		work->time--;
		size = work->size;
		_sceVu0ScaleVector( &l_force.vx, &work->force, (float)work->time / (float)work->time_max );

		NewBlood1_Demo( work->world, &fvtemp, &l_force, size );
		if( work->mode==0 ){
			NewBlood1_Demo( work->world, &fvtemp, &l_force, size );
			NewBlood2_Demo( work->world, &fvtemp, &l_force, size );
		}
	}else{
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( Work *work )
{
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world, FVECTOR *force, int size, int time )
{
	work->world = world;
	if( work->world==NULL ) return -1;

	DG_COPY_VEC( &work->force, force );

	work->size  = size;
	if( work->size < 0 )    return -1;

	work->time  = time;
	if( work->time < 0 )    return -1;
	work->time_max = time;

	return 0 ;
}

/*

[world]:[ポインタ保存]付随させる位置、方向。メタル座標系Ｚ軸方向に飛ぶ。
[force]:吹き出し方向
[size ]:大きさ
[time ]:発生継続フレーム（実際は少々長くなる）
[pat  ]:パターン。0:（現在未使用）

*/
void *NewBlood_Demo( FMATRIX *world, FVECTOR *force, int size, int time, int pat )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {

		work->mode = 0;

		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, force, size, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*

[world]:[ポインタ保存]付随させる位置、方向。メタル座標系Ｚ軸方向に飛ぶ。
[force]:吹き出し方向
[size ]:大きさ
[time ]:発生継続フレーム（実際は少々長くなる）
[pat  ]:パターン。0:（現在未使用）

ソニー対策。色見、量を更に半分にした。

*/
void *NewBlood_Demo2( FMATRIX *world, FVECTOR *force, int size, int time, int pat )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {

		work->mode = 1;

		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, force, size, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
