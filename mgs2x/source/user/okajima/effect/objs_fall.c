//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	objs_fall.c
	物体落下跳ね返り
	2000/02/02 S.Okajima
	$Id: objs_fall.c,v 1.1.1.3 2002/11/19 11:47:08 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"

//#define		GRAVITY		( 6.0f ) 
#define		FALL_TIME	( 60*60*1 ) 

#define		MAX_PAT		( 1 ) 
#define		MAX_MAG		( 32 ) 

extern float GM_WaterLevel;

typedef	struct	{
	GV_ACT_EX		actor ;

	/* カートリッジ */
	DG_OBJS		*objs ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		vec ;
	float		bound_ratio;
	int			life;

	int			count;

	int			flag;

} Work ;


static	void Act( Work *work )
{
	FVECTOR fvtemp0;
	FVECTOR fvtemp1;
	float	f_abs;
	int		flag;
	HZX_SEG		*seg ;
	HZX_FLR		flr ;
	u_int		atr ;
	int		map_id;

	if( work->life!=-1 ){
		if( --work->life <= 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
	}
	GM_GroupObjs( work->objs, GM_CurrentStageMap );
	DG_GetLightMatrix( &work->pos, work->lights );

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	if( work->flag==0 ){
		work->rot.vx += 128;
		work->rot.vy += 128;
		work->rot.vz += 128;

		work->vec.vy += P_GRAVITY;
		_sceVu0AddVector( &fvtemp0, &work->pos, &work->vec ) ;
		if( work->vec.vy < 0.0f ){
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_FIX,
				HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_RECOIL | HZX_FLOOR_RECOIL_TYPE );
		}else{
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_F_SEGMENT,
				HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_RECOIL | HZX_FLOOR_RECOIL_TYPE );
		}

		if( flag==1 ){	/* 壁 */
			GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
			HZX_GetOnlinePoint( &fvtemp1 );
			HZX_GetOnlineHazard( &flr, &atr ) ;
			seg=(HZX_SEG *)&flr;
			fvtemp0.vx = seg->p2.z - seg->p1.z ;
			fvtemp0.vy = 0.0f ;
			fvtemp0.vz = seg->p1.x - seg->p2.x ;
			DG_ReflectVector( &fvtemp0, &work->vec, &work->vec );
			work->vec.vx *= work->bound_ratio;
			work->vec.vy *= work->bound_ratio;
			work->vec.vz *= work->bound_ratio;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->vec ) ;
			work->rot.vx += 128;
			work->rot.vy += 128;
			work->rot.vz += 128;
		}else if( flag==2 ){	/* 床 */

			if( work->pos.vy < GM_WaterLevel ){
				GV_DestroyActor( work ) ;
				return;
			}

			GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
			HZX_GetOnlinePoint( &fvtemp1 );
			work->vec.vx += rnd()*work->vec.vy*0.5f - work->vec.vy*0.25f;
			work->vec.vz += rnd()*work->vec.vy*0.5f - work->vec.vy*0.25f;
			work->vec.vy *= -work->bound_ratio;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->vec ) ;
			f_abs=( work->vec.vy > 0 )?work->vec.vy:-work->vec.vy;
//			if( f_abs < 10.0f ){
			if( f_abs < DG_FABS(P_GRAVITY)*2.0f ){
				work->pos.vy += 15.0f;
				work->rot.vx = 0;
				work->rot.vy = (short)(4096.0f*rnd());
				work->rot.vz = 1024;
				DG_GetLightMatrix( &work->pos, work->lights );
				work->flag=1;
			}else{
				work->rot.vx += 128;
				work->rot.vy += 128;
				work->rot.vz += 128;
			}
		}else{	/* 当たらなかった */
			DG_COPY_VEC( &work->pos, &fvtemp0 );
		}
	}

//AN_Test_Eye2( &work->pos, 2 );
//printf("%f %f %f\n",work->pos.vx,work->pos.vy,work->pos.vz);

	DG_SetPos2( &work->pos, &work->rot );
	DG_PutObjs( work->objs );

}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

static	int GetResources( Work *work, FVECTOR *pos, SVECTOR *rot, FVECTOR *vec, int model_name, float bound_ratio, int life )
{
	int	i;
	DG_DEF		*def ;

	DG_COPY_VEC( &work->pos, pos );
	work->rot.vx = rot->vx;
	work->rot.vy = rot->vy;
	work->rot.vz = rot->vz;
	DG_COPY_VEC( &work->vec, vec );
	work->bound_ratio = bound_ratio;
	work->life = life;

	work->flag=0;
	def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );


	/* ０番以外は不可視 */
	for(i=1; i<work->objs->n_models; i++){
		work->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
	}


	work->count=FALL_TIME;

	DG_GetLightMatrix( &work->pos, work->lights );
	DG_SetLightMatrix( work->objs, work->lights );

	return 0 ;
}

void *NewObjectFall( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec, int model_name, float bound_ratio, int life )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, rot, vec, model_name, bound_ratio, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
