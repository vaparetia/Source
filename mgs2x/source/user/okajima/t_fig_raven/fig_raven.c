//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	fig_raven_init.c
	管理部分
	2000/02/17  H.TANAKA
	2000/10/18 S.Okajima
	$Id: fig_raven.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"fig_raven.h"
#include	"../etc/ok_util.h"

static  void  FigRaven_MoveWeak( Work *work )
{
	work->rot_move.vy += irnd()%128 - 64 + 4 ;

	work->move.vx = work->pos.vx + frnd()*8.0f;
	work->move.vz = work->pos.vz + frnd()*8.0f;
	if( work->move.vx < work->boundary_min.vx ){
		work->move.vx = work->boundary_min.vx;
	}else if( work->move.vx > work->boundary_max.vx ){
		work->move.vx = work->boundary_max.vx;
	}
	if( work->move.vz < work->boundary_min.vz ){
		work->move.vz = work->boundary_min.vz;
	}else if( work->move.vz > work->boundary_max.vz ){
		work->move.vz = work->boundary_max.vz;
	}
}

static   void  Act(Work *work)
{
	FMATRIX	mat;
	int		itemp;


	if( (work->map & GM_CurrentStageMap) != 0 ){
		work->objs->flag &= ~DG_FLAG_INVISIBLE ;
		itemp = GV_Time%320;
		if( work->hit_count>=1 && (GV_Time%20==0) ){
			if( (itemp!=60) && (itemp!=140) && (itemp!=300) ){
				New_BB_Dan(&work->objs->world, work->map) ;
				FigRaven_MoveWeak( work );
				/* ＢＢ弾発射音 */
				GM_SeSetMode( SD_A_BBSHOT01 , &work->pos, GM_SEMODE_NORMAL ) ;
			}else{
				/* バルカン叫び声 */
				GM_SeSetMode( SD_A_DOLLV01 , &work->pos, GM_SEMODE_NORMAL ) ;
			}
		}
	}else{
		work->objs->flag |=  DG_FLAG_INVISIBLE ;
//		return;
	}


	if( work->next_available_count > 0 ){
		work->next_available_count--;

		if( work->next_available_count > STABLE_COUNT/2 ){
			work->pos.vy += rnd()*200.0f;
			work->rot.vx += irnd()%300;
			work->rot.vz += irnd()%300;
		}
	}

//	GM_GroupObjs( work->objs, work->map );
	DG_GetLightMatrix(&work->pos,work->light) ;


	work->pos.vx = (work->pos.vx + work->move.vx)*0.5f;
	work->pos.vy = (work->pos.vy + work->move.vy)*0.5f;
	work->pos.vz = (work->pos.vz + work->move.vz)*0.5f;

	work->rot.vx = (work->rot.vx + work->rot_move.vx)/2;
	work->rot.vy = (work->rot.vy + work->rot_move.vy)/2;
	work->rot.vz = (work->rot.vz + work->rot_move.vz)/2;

	GM_CurrentMap = work->map;	//act control で やるものの代用
	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs(work->objs) ;
	DG_GetPos( &mat ) ;
	GM_MoveTarget2( &work->target, &mat );


	DG_PutObjs(work->object.objs) ;


}

static   void  Die(Work *work)
{
	/* ダミー削除 */
	GM_FreeObject(             &work->object  );
	GM_FreeControl( (CONTROL *)&work->control );

	if(work->objs != NULL){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs(work->objs) ;
		return ;
	}
}

/* ------------------------------------------------------- */
static   int  GetResources(Work *work)
{

	/* オプション取得 */
	if( FigRaven_GetOptionValue(work) < 0 ){
		GV_DestroyActor(work) ;
		return -1  ;
	}

	/* レイブンモデル設定 */
	if(FigRaven_InitRaven(work)){
		GV_DestroyActor(work) ;
		return -1 ;
	}

	return 0 ;
}

void  *New_Figure_Raven(int name, int map)
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->name = name;
		work->map = map;

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
