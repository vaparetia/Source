//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	sps_amo.c
	スパス薬莢
	2000/06/29 S.Okajima
	$Id: sps_amo.c,v 1.1.1.3 2002/11/19 11:50:37 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"

 /* GV_StrCode( "sps_emb" */
#define		SPS_AMO	( 0x613fa9 )

#define		SPEED	( 30.0f ) 

#define		FALL_TIME	( 60*60*1 ) 
#define		MAX_AMO		( 32 ) 
#define		DECAY_RATIO	( 0.6f ) 

static int ok_sps_amo_num = 0;

typedef	struct	{
	GV_ACT_EX		actor ;

	/* カートリッジ */
	DG_OBJS		*objs ;
	FMATRIX		mat ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos ;
	FVECTOR		vec ;
	SVECTOR		rot ;
	SVECTOR		rot_add ;
	int			count;
	int			pat;

	int			se_flag;
	int			amo_num;
	int			flag;

} Work ;


static	void Act( Work *work )
{

	FVECTOR fvtemp0;
	FVECTOR fvtemp1;
//	FVECTOR fvtemp2;
	float	f_abs;
	int		flag;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	int			atr[2] ;
	int			map_id;


//AN_Test_Eye2( &work->pos, 2 );


	if( work->count-- < 0
	 || work->amo_num == ok_sps_amo_num){
		GV_DestroyActor( work ) ;
		return;
	}

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	if( work->flag==0 ){
		work->rot.vx += work->rot_add.vx;
		work->rot.vz += work->rot_add.vz;

		work->vec.vy += P_GRAVITY;
		_sceVu0AddVector( &fvtemp0, &work->pos, &work->vec ) ;
		if( work->vec.vy < 0.0f ){
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_FIX,
		        HZX_SEG_NO_PLAYER,
		        HZX_FLOOR_NO_PLAYER );
		}else{
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_F_SEGMENT,
		        HZX_SEG_NO_PLAYER,
		        HZX_FLOOR_NO_PLAYER );
		}

		if( flag==1 ){	/* 壁 */
			GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
			HZX_GetOnlinePoint( &fvtemp1 );
			HZX_GetOnlineHazard( flr, atr ) ;
			seg=(HZX_SEG *)flr;
			fvtemp0.vx = seg->p2.z - seg->p1.z ;
			fvtemp0.vy = 0.0f ;
			fvtemp0.vz = seg->p1.x - seg->p2.x ;
			DG_ReflectVector( &fvtemp0, &work->vec, &work->vec );
			work->vec.vx *= DECAY_RATIO;
			work->vec.vy *= DECAY_RATIO;
			work->vec.vz *= DECAY_RATIO;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->vec ) ;
			work->rot_add.vx = irnd()%256-128;
			work->rot_add.vz = irnd()%256-128;
		}else if( flag==2 ){	/* 床 */
			if(work->se_flag==0 || work->se_flag==1){	/* 床は2回だけなる */
				GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
				work->se_flag++;
			}
			HZX_GetOnlinePoint( &fvtemp1 );
			work->vec.vx = rnd()*work->vec.vy - work->vec.vy*0.5f;
			work->vec.vz = rnd()*work->vec.vy - work->vec.vy*0.5f;
			work->vec.vy *= -DECAY_RATIO;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->vec ) ;
			f_abs=( work->vec.vy > 0 )?work->vec.vy:-work->vec.vy;
			if( f_abs < 10.0f ){
				work->pos.vy += 15.0f;
				work->rot.vx = 0;
				work->rot.vy = irnd()%4096;
				work->rot.vz = 1024;
				work->flag=1;
			}else{
				work->rot_add.vx = irnd()%256-128;
				work->rot_add.vz = irnd()%256-128;
			}
			DG_GetLightMatrix( &work->pos, work->lights );
		}else{	/* 当たらなかった */
			DG_COPY_VEC( &work->pos, &fvtemp0 );
		}
		DG_SetPos2( &work->pos, &work->rot );
		DG_GetPos( &work->mat );
	}else{
		DG_SetPos( &work->mat );
	}
	DG_PutObjs( work->objs );

}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

static	FVECTOR shift_center={ 0.0f, -50.0f, 0.0f, 0.0f };
static	int GetResources( Work *work )
{
	DG_DEF		*def ;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( SPS_AMO, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );
	GM_GroupObjs( work->objs, GM_CurrentStageMap );

	work->vec.vx += frnd()*0.1f;
	work->vec.vy += frnd()*0.1f;
	work->vec.vz += frnd()*0.1f;

	_sceVu0ScaleVector( &work->vec, &work->vec, SPEED );

	DG_SetLightMatrix( work->objs, work->lights );
	DG_GetLightMatrix( &work->pos, work->lights );

	work->amo_num=ok_sps_amo_num;
	ok_sps_amo_num++;
	if( ok_sps_amo_num >= MAX_AMO ) ok_sps_amo_num=0;

	DG_SetPos2( &work->pos, &work->rot );
	DG_PutObjs( work->objs );

	work->se_flag=0;

	work->count = FALL_TIME;

	work->rot_add.vx = irnd()%32;
	work->rot_add.vy = irnd()%32;
	work->rot_add.vz = 192 + irnd()%32;

	{
		extern void *NewConnectSmoke( FMATRIX *world, FVECTOR *shift, int life, float size_rnd );
		GV_SetActorChild( work, NewConnectSmoke( &work->objs->world, &shift_center, 120, 16.0f ) );
	}


	return 0 ;
}

void *NewSPS_EjectAmo( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos, pos);
		work->rot.vx = rot->vx;
		work->rot.vy = rot->vy;
		work->rot.vz = rot->vz;
		DG_COPY_VEC( &work->vec, vec);

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
