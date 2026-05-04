//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	z_man.c
	カメラからの相対Ｚフォーカス値を制御

	1999/09/29 S.Okajima
	$Id: z_man.c,v 1.1.1.3 2002/11/19 11:47:17 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#ifdef PSX2
#include	"def_dma.h"
#endif

/* 0.0f 以外だと本キャラが起動した後 */
float	ok_focus_z_far=0.0f;
float	ok_focus_z_near=0.0f;
float	ok_focus_z_time=60;

#define	MAX_Z_PRIORITY	(4)

#define	OK_Z_FOCUS_NEAR_MIN	(6000.0f)
#define	OK_Z_FOCUS_NEAR_MAX	(30000.0f)
#define	OK_Z_FOCUS_FAR_MIN	(OK_Z_FOCUS_NEAR_MIN + 10000.0f)
#define	OK_Z_FOCUS_FAR_MAX	(OK_Z_FOCUS_NEAR_MAX + 10000.0f)

typedef	struct	{
	GV_ACT_EX	actor ;
	int		name;

	int		count;
	int		time;

	int		enabler_before;
	int		enabler[MAX_Z_PRIORITY];
	int		time_z[MAX_Z_PRIORITY];
	float	focus_far_z[MAX_Z_PRIORITY];
	float	focus_near_z[MAX_Z_PRIORITY];

	float	focus_far;
	float	before_focus_far;
	float	focus_near;
	float	before_focus_near;

} Work ;

#define	REQ_PARAM1	(0)
#define	REQ_PARAM2	(1)
#define	REQ_PARAM3	(2)
#define	REQ_PARAM4	(3)

#define	REQ_PARAM1_ENABLE	(10)
#define	REQ_PARAM2_ENABLE	(11)
#define	REQ_PARAM3_ENABLE	(12)
#define	REQ_PARAM4_ENABLE	(13)

#define	REQ_PARAM1_DISABLE	(20)
#define	REQ_PARAM2_DISABLE	(21)
#define	REQ_PARAM3_DISABLE	(22)
#define	REQ_PARAM4_DISABLE	(23)

enum {
	REQ_PARAM=0,
	REQ_NO
};

static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int	i;

//printf("q\n");

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
//printf("param=%d\n",num);
		switch( num ){
			case REQ_PARAM1:
			case REQ_PARAM2:
			case REQ_PARAM3:
			case REQ_PARAM4:
				work->focus_far_z[num] = (float)msg->message[1];
				work->focus_near_z[num] = (float)msg->message[2];
				work->time_z[num]    = msg->message[3];
				if(work->time_z[num]<1) work->time_z[num]=1;
//printf("a1:%f\n",work->focus_far_z[num]);
//printf("a2:%f\n",work->focus_near_z[num]);
//printf("a3:%d\n",work->time_z[num]);
			  break;
			case REQ_PARAM1_ENABLE:
			case REQ_PARAM2_ENABLE:
			case REQ_PARAM3_ENABLE:
			case REQ_PARAM4_ENABLE:
				work->enabler[num-REQ_PARAM1_ENABLE]=1;
//printf("c:%d\n",num);
			  break;
			case REQ_PARAM1_DISABLE:
			case REQ_PARAM2_DISABLE:
			case REQ_PARAM3_DISABLE:
			case REQ_PARAM4_DISABLE:
				work->enabler[num-REQ_PARAM1_DISABLE]=0;
//printf("d:%d\n",num);
			  break;
			default:
			  break;
		}
		msg--;
	}

// 注！！！
// Ｚフォーカス管理は毎フレーム初期化している（デプスは変化時のみ）
	for(i=MAX_Z_PRIORITY-1; i>=0; i--){	/* 大きい方が優先高い */
		if( work->enabler[i] == 1 ){
//printf("enabler:%d\n",i);
//printf("enabler_before:%d\n",work->enabler_before);
			if( work->enabler_before != i ){
//				work->enabler_before  = i;
				work->enabler_before  = -1;
//				work->enabler[i]      = 0;
				work->count           = work->time_z[i];
				work->time            = work->time_z[i];
				ok_focus_z_time           = work->time;
				work->focus_far           = work->focus_far_z[i];
				work->before_focus_far    = ok_focus_z_far;
				work->focus_near          = work->focus_near_z[i];
				work->before_focus_near   = ok_focus_z_near;
//printf("Z_FOCUS::on:%d\n",i);
//printf( "near far %f %f\n", work->focus_near, work->focus_far ) ;
			}
			break;
		}
	}



	if( work->count > 0){
		work->count--;
//printf("%d:%f:%f:\n",work->count,ok_focus_z_far,ok_focus_z_near);
		ok_focus_z_far  += (work->focus_far  - work->before_focus_far  ) / (float)work->time;
		ok_focus_z_near += (work->focus_near - work->before_focus_near ) / (float)work->time;
	}

	ok_focus_z_near = OK_Z_FOCUS_NEAR_MIN;

/*
	if(ok_focus_z_near < OK_Z_FOCUS_NEAR_MIN) ok_focus_z_near = OK_Z_FOCUS_NEAR_MIN;
	if(ok_focus_z_near > OK_Z_FOCUS_NEAR_MAX) ok_focus_z_near = OK_Z_FOCUS_NEAR_MAX;
	if(ok_focus_z_far  < OK_Z_FOCUS_FAR_MIN)  ok_focus_z_far  = OK_Z_FOCUS_FAR_MIN;
	if(ok_focus_z_far  > OK_Z_FOCUS_FAR_MAX)  ok_focus_z_far  = OK_Z_FOCUS_FAR_MAX;
	if(ok_focus_z_far  < ok_focus_z_near  ) ok_focus_z_far  = ok_focus_z_near;
*/
}

static void Die( Work *work )
{
}

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	work->focus_far_z[0] = DRAW_Z_MIN;
	work->focus_near_z[0] = DRAW_Z_MIN;
	if ( GCL_GetOption( 'z' ) != NULL ){
		work->focus_far_z[0]   = GCL_GetNextInt();
		work->focus_near_z[0]  = GCL_GetNextInt();
	}

	work->time_z[0]=30;
	if ( GCL_GetOption( 't' ) != NULL ){
		work->time_z[0]  = GCL_GetNextInt();
	}

}

static int GetResources( Work *work, int name )
{
	int	i;

	work->name = name;

	work->time = 1;
	work->count= 0;

	ok_focus_z_far  = (float)DRAW_Z_MIN;
	ok_focus_z_near = (float)DRAW_Z_MIN;

	for(i=0; i<MAX_Z_PRIORITY; i++){
		work->enabler[i]=0;
	}
	work->enabler_before=-1;
	work->enabler[0]=1;		/* 最低優先は常にＯＮ */

	GetOptionValue( work );
	work->count             = work->time_z[0];
	work->time              = work->time_z[0];
	work->focus_far         = work->focus_far_z[0];
	work->before_focus_far  = DRAW_Z_MIN;
	work->focus_near        = work->focus_near_z[0];
	work->before_focus_near = DRAW_Z_MIN;

	return (0);
}


void *NewZman( int name ,int map )
{
	Work		*work ;

	OPERATOR() ;

//printf("name:%d\n",name);

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
