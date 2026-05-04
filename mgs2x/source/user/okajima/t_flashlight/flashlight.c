//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	flashlight.c
	管理部分
	2000/01/21  H.TANAKA
	2000/10/18 S.Okajima
	$Id: flashlight.c,v 1.1.1.3 2002/11/19 11:47:45 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"flashlight.h"
#include	"../etc/ok_util.h"


FVECTOR  FL_LIGHT_Triangle[N_TRIANGL_VERTS] = {
	{36.0F, 0.0F, 0.0F, 1.0F},
	{-18.0F, 20.0F, 0.0F, 1.0F},
	{-18.0F, -20.0F, 0.0F, 1.0F}
} ; 
	 
/* util */
void   FL_LIGHT_SetSound(int se, FVECTOR *pos, int mode)
{
	int	pan, vol ;
   float bp_angle;

	vol = GM_SeGetVol(pos, mode) ;
	pan = GM_SeGetPan(pos, mode, &bp_angle) ;
	GM_SeSet3D(pan,vol,se, bp_angle) ;
}

/* ------------------------------------------------------ */
static  void  Act(Work *work)
{
	FLASHLIGHT  *fl ;
	GLASS	   *gls ;


	if( work->target_flag == 2 ){
		GM_ClearTargetDamage( &work->target ) ;
		GM_FreeTarget(&work->target) ;
		work->target_flag = 0;
	}


	if(work->objs != NULL){
		GM_GroupObjs( work->objs, GM_CurrentStageMap );
		DG_GetLightMatrix((FVECTOR *)work->objs->world.m[3],work->light) ;
	}else{
		GM_GroupObjs( work->brk_objs, GM_CurrentStageMap );
		DG_GetLightMatrix((FVECTOR *)work->brk_objs->world.m[3],work->light) ;
	}

	fl = &work->flashlight ;
	(* fl->act)(work) ;

	gls = work->glass ;
	if(gls != NULL){
		if( (*gls->act)(work) ) FL_LIGHT_FreeGlass(work) ;
	}
}

static  void  Die(Work *work)
{
	FLASHLIGHT  *fl ;

	if(work->objs != NULL){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs ) ;
	}
	if(work->brk_objs != NULL){
		DG_DequeueObjs( work->brk_objs ) ;
		DG_FreeObjs( work->brk_objs ) ;
	}

	fl = &work->flashlight ;
	fl->prim        = OK_FreePrim2( fl->prim ) ;
	fl->prim_hl_sub = OK_FreePrim2( fl->prim_hl_sub ) ;
	fl->prim_hl     = OK_FreePrim2( fl->prim_hl ) ;
	fl->prim_hl_add = OK_FreePrim2( fl->prim_hl_add ) ;

	FL_LIGHT_FreeGlass(work) ;

	if( work->target_flag ) GM_FreeTarget( &work->target ) ;
}

/*----------------------------------------------------------------*/

static   int  GetResources(Work *work, int map)
{
	work->target_flag = 0;

	/* オプション取得 */
	if(FL_LIGHT_GetOptionValue(work)) return -1 ;

	/* モデルを取得 */
	if(FL_LIGHT_InitModel(work)) return -1 ;

	if( work->brk_flag==0 ){	// 壊れていない
		/* ライトを取得 */
		if(FL_LIGHT_InitLight(work,map)) return -1 ;
	}

	return 0 ;
}

void  *NewFlashLight(int name, int map)
{
	Work   *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL ){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->map = map;

		if( GetResources( work, map ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}
