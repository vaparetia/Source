//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	fl_light_target.c
	2000/02/18  H.TANAKA
	2000/10/18 S.Okajima
	$Id: fl_light_target.c,v 1.1.1.3 2002/11/19 11:47:45 Yoshizawa1 Exp $
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

static  void  Flashlight_Break(Work *work, TARGET *t)
{
	DG_MDL	  *mdl ;
	FVECTOR	 t_size ;
	FVECTOR	 pos ;
//	TARGET	  *new_t ;
	FLASHLIGHT  *fl ;


//printf("Flashlight_Break:a1\n");


	/* 音源を選ぶ */
	FL_LIGHT_SetSound(SD_A_PLIGHT01, &t->hit, GM_SEMODE_NORMAL) ;

	/* モデル切り替え */
	DG_DequeueObjs(work->objs) ;
	DG_FreeObjs(work->objs) ;
	work->objs = NULL ;

#if 0
	/* ターゲット初期化 */
	GM_ClearTargetDamage(t) ;
	GM_FreeTarget(t) ;
#endif

	work->target_flag = 2;

	mdl = work->brk_def->models ;
	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly) * 0.5F ;	
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;
	t_size.vw = 1.0F ;

	/* ターゲットの位置 */
	pos.vx = 0.0F ;
	pos.vy = t_size.vy ;
	pos.vz = -20.0F ;

/*
	new_t = &work->target ;
	GM_SetTarget(t,TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER | TARGET_SEEK, 0, ENEMY_SIDE, &t_size, &pos) ;
	GM_PutTarget(new_t) ;
	GM_MoveTarget2(new_t, &work->brk_objs->world) ;
*/

	DG_VisibleObjs(work->brk_objs) ;
	DG_SetLightMatrix(work->brk_objs,work->light) ;
	DG_GetLightMatrix((FVECTOR*)work->brk_objs->world.m[3],work->light) ;
//	NewTargetView(new_t, 255, 255, 0) ;

	/* その他の処理 */
	fl = &work->flashlight ;
	fl->act = (void *)FL_LIGHT_Break_Act ;
	fl->break_count = MAX_BREAK_COUNT ;
	fl->fire_count  = 0 ;

	/* ハレーションを見えなくする */
	DG_InvisiblePrim2(fl->prim_hl_sub) ;
	DG_InvisiblePrim2(fl->prim_hl) ;
	DG_InvisiblePrim2(fl->prim_hl_add) ;

	/* GCLに信号を返す */
	if( work->id > 0 ){
		GCL_ExecProc( work->id, NULL );
		work->id = -1;
	}

	/* 破片部分の初期化 */
	if(FL_LIGHT_InitGlass(work)) FL_LIGHT_FreeGlass(work) ;

	GM_SeSetMode( SD_A_PLIGHT01 , (FVECTOR*)work->brk_objs->world.m[3], GM_SEMODE_NORMAL ) ;

}

void  FL_LIGHT_TargetCallBack(TARGET * off, TARGET *def, void *ptr)
{
	Work *work = (Work *)ptr ;

	if(work->objs){
		if(def->damaged & TARGET_POWER){
			if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST|WP_PUNCH)){
				printf("hit on flashlight \n") ;
				Flashlight_Break(work,def) ;
			}
		}
	}
	def->weapon_type = 0 ;
}
