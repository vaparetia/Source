//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   online_util.c
   
   ラインを分割してオンラインチェックしてくれる act
   2000/06/06 T.Shibata

   $Id: online_util.c,v 1.1.1.3 2002/11/19 11:48:54 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"

#define		CHECK_HZX		(3)

#define		MAX_TURN		(32)
#define		MAX_FHASE		(32)

#define		NO_OBSTACLE	(0)
#define		OBSTACLE	(1)
typedef struct __Work{
	GV_ACT_EX	 	actor;
	
	int				map_id;
	FVECTOR			*from;
	FVECTOR			*to;
	int				chk_flag;
	int				seg_flag;
	int				flr_flag;
	
	int				*result;
	
//	FVECTOR			*point;
//	void			*hzx_info;
//	void			*hxz_atr;
	
	float			max_len;
	int 			turn;
	int				phase;
	int				t_cnt;
	int				p_cnt;

	int				turn_info;
	int				phase_info;

}Work;

//int 	HZX_OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR *from, FVECTOR *to,
//							  int chk_flag, int seg_flag, int flr_flag )


static void PositionDataSet(Work *work, FVECTOR *from, FVECTOR *to )
{
	FVECTOR		dvec;
	
	_sceVu0SubVector( &dvec, work->to, work->from );

//printf("Scale from %f\n",(float)work->p_cnt/(float)work->phase);
//printf("Scale to %f\n\n",(float)(work->p_cnt+1)/(float)work->phase);
	
	_sceVu0ScaleVectorXYZ( from, &dvec, (float)work->p_cnt/(float)work->phase);
	_sceVu0ScaleVectorXYZ(   to, &dvec, (float)(work->p_cnt+1)/(float)work->phase);
	_sceVu0AddVector( from, from, work->from );
	_sceVu0AddVector( to, to, work->from );

// バウンディングボックス内に、ある点が存在するかどうかのチェック
//   バウンディングボックス内に存在すれば１、しなければ０を返す。*/
//static inline int vu0_CheckBoundingBox(const VECTOR * const vertex,const VECTOR * const v0,const VECTOR * const v1)
	
	if( !vu0_CheckBoundingBox( to, work->from, work->to ) ){
		DG_COPY_VEC(to,work->to);
	}
}


//static
static void Act_CheckOnline_Static(Work *work)
{
	int			hzx_result,check,info;//,tmp;
	FVECTOR		from,to;
//printf("from vx %f: vy %f: vz = %f\n",work->from->vx,work->from->vy,work->from->vz);
//printf("to vx %f: vy %f: vz = %f\n\n",work->to->vx,work->to->vy,work->to->vz);
	PositionDataSet(work, &from, &to );

#if 0
	{		
//		FVECTOR		verts[2];
//printf("from vx %f: vy %f: vz = %f\n",from.vx,from.vy,from.vz);
//printf("to vx %f: vy %f: vz = %f\n\n",to.vx,to.vy,to.vz);
		DG_COPY_VEC( &verts[0],&from );
		DG_COPY_VEC( &verts[1],&to );
		NewLineView( verts, 2, 0x80, 0, 0 );
	}
#endif
//	tmp = HZX_CurrentGroupID ;
//	HZX_CurrentGroupID = 0 ;
	hzx_result = HZX_OnlineHazardCheck( GM_GetHzxGroupID( work->map_id ), &from, &to,
										work->chk_flag, work->seg_flag, work->flr_flag );
//	HZX_CurrentGroupID = tmp ;
	
//	HZX_GetOnlinePoint( work->point );
//	HZX_GetOnlineHazard( work->hzx_info, work->hxz_atr );

	work->phase_info = (work->phase_info << 1) | ((hzx_result & CHECK_HZX)?OBSTACLE:NO_OBSTACLE);
	check = ~((-1) << work->phase);
//	printf("phase_check = %x\n",phase_check);
	info = work->phase_info & check;
//	printf("phase_info = %x\n",phase_info);
#if 1
	work->turn_info = (work->turn_info << 1) | ( info? OBSTACLE: NO_OBSTACLE );
	check = ~((-1) << work->turn);
	info = work->turn_info & check;
	//二段階チェック
	if(info == 0){
		//可視
		*work->result = 1;
	}else if(info == check){
		//不可視
		*work->result = -1;
	}else{
		//グレー
		*work->result = 0;

	}
#else
	if(phase_info == 0){
		//可視
		*work->result = 1;
	}else if(info == check){
		//不可視
		*work->result = -1;
	}else{
		*work->result = -1;
	}
#endif
	if( ++work->p_cnt >= work->phase ) work->p_cnt = 0;
	if( ++work->t_cnt >= work->turn ) work->t_cnt = 0;
}

static void Die(Work *work)
{

}

static int GetResources( Work *work, int map_id,
						 FVECTOR *from, FVECTOR *to,
						 int chk_flag, int seg_flag, int flr_flag,
						 int *result,
						 int turn, int phase, float max_len )
{
	work->map_id = map_id;

	if(!from) return -1;
	if(!to) return -1;
	if(!result) return -1;
	if( max_len <= 0.0f ) return -1;
	
	work->from = from;
	work->to = to;
	
	work->chk_flag = chk_flag;
	work->seg_flag = seg_flag;
	work->flr_flag = flr_flag;
	
	work->result = result;
	
	work->turn = turn;
	work->phase = phase;
	work->max_len = max_len;

	work->t_cnt = 0;
	work->p_cnt = 0;
	work->turn_info = -1;
	work->phase_info = -1;
	return 1;
}

/*
    void *NewOnlineCheck_Static: 	//HZX_OnlineHazardCheckの引数flagが呼出の時から変化しない

	int				map_id;			//まっぷID
	FVECTOR			*from;			//HZX_OnlineHazardCheckの引数
	FVECTOR			*to;			//同上
	int				chk_flag;		//同上
	int				seg_flag;		//同上
	int				flr_flag;		//同上
	
	int				*result;		//答え(-1:見えない 0:半分くらい 1:見える)

	int 			turn;			//
	int				phase;			//ライン分割数
	float 			max_len;		//チェックする最大の長さ
*/
void *NewOnlineCheck_Static( int map_id,
							 FVECTOR *from, FVECTOR *to,
							 int chk_flag, int seg_flag, int flr_flag,
							 int *result,
							 int turn, int phase, float max_len  )
{
	Work *work = NULL;
	
//	printf("OnlineCheck set\n");

	work = (Work*)GV_NewEffect( GV_ACTOR_USER, sizeof(Work) );

	if( !work ) return NULL;

	GV_SetActor( &work->actor,Act_CheckOnline_Static,Die );
	GV_ActorEX( &work->actor );
	if(GetResources( work, map_id,
					 from, to,
					 chk_flag, seg_flag, flr_flag,
					 result,
					 turn, phase, max_len ) < 0){
		
		GV_DestroyActor( work ) ;
		return NULL ;
	}

//	printf("OnlineCheck set ok\n");
	return (void *)work ;
}

/*
    void *NewOnlineCheck_Dynamic: 	//HZX_OnlineHazardCheckの引数flagが変わる

	int				map_id;			//まっぷID
	FVECTOR			*from;			//HZX_OnlineHazardCheckの引数
	FVECTOR			*to;			//同上
	int				*chk_flag;		//同上
	int				*seg_flag;		//同上
	int				*flr_flag;		//同上
	
	int				*result;		//HZX_OnlineHazardCheckの戻値のポインタ
	FVECTOR			*point;			//HZX_GetOnlinePointの引数
	void			*hzx_info;		//HZX_GetOnlineHazardの引数
	void			*hzx_atr;		//同上

	float 			max_len;		//チェックする最大の長さ
*/
#if 0
void *NewOnlineCheck_Dynamic( int map_id,
							  FVECTOR *from, FVECTOR *to,
							  int *chk_flag, int *seg_flag, int *flr_flag,
							  int *result,
							  FVECTOR *point, void *hzx_info, void *hxz_atr,
							  float max_len )
{

}
#endif
