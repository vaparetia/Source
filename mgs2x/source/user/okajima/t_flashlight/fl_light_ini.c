//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	flashlight_ini.c
	初期化部 
	2000/02/18   H.TANAKA
	2000/10/18 S.Okajima
	$Id: fl_light_ini.c,v 1.1.1.3 2002/11/19 11:47:45 Yoshizawa1 Exp $
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


/* ------------------------------------------------------------- */
/*				   破片										*/
/* ------------------------------------------------------------- */
void  FL_LIGHT_FreeGlass(Work *work)
{
	GLASS	   *glass ;
	
	glass = work->glass ;
	if(glass != NULL){
		glass->prim = OK_FreePrim2(glass->prim) ;
		if(glass->base != NULL){
			GV_Free(glass->base) ;
		}
		GV_Free(glass) ;
	}
}

static  void  Decide_Base(FVECTOR *base, FVECTOR *speed,DG_MDL *mdl, FMATRIX *world)
{
	FVECTOR   fvtmp ;
	FVECTOR   *p_base,*p_speed ;
	FVECTOR	 center ;
	float	 range_vx, range_vy ;
	int	   i ;

	range_vx = mdl->ux - mdl->lx ;
	range_vy = mdl->uy - mdl->ly ;
	fvtmp.vx = range_vx * 0.7F ;
	fvtmp.vy = range_vy * 0.7F ;
	fvtmp.vz = mdl->lx + range_vx * 0.15F ;
	fvtmp.vw = mdl->ly + range_vy * 0.15F ;

	center.vx = mdl->lx + range_vx * 0.6F ;
	center.vy = mdl->ly + range_vy * 0.6F ;
	center.vz = mdl->lx + range_vx * 0.4F ;
	center.vw = mdl->ly + range_vy * 0.4F ;

	p_base = base ;
	p_speed = speed ;
	for(i = 0; i < N_GLASS_NUM; i++){
		base->vx = fvtmp.vz + rnd() * fvtmp.vx ;
		base->vy = fvtmp.vw + rnd() * fvtmp.vy ;
		base->vz = -213.0F ;

		if(base->vx > center.vx){
			speed->vx = rnd() * 10.0F ;
		}else if(base->vx < center.vz){
			speed->vx = - rnd() * 10.0F ;
		}else{
			speed->vx = 0.0F ;
		}

		if(base->vy > center.vy){
			speed->vy = rnd() * 10.0F ;
		}else if(base->vy < center.vw){
			speed->vy = - rnd() * 10.0F ;
		}else{
			speed->vy = 0.0F ;
		}

		speed->vz = - 10.0F - rnd() * 20.0F ;
		speed ++ ;
		base ++ ;
	}

	base = p_base ;
	speed = p_speed ;
	DG_SetPos(world) ;
	DG_PutVector(base,base,N_GLASS_NUM) ;
	DG_RotVector(speed,speed, N_GLASS_NUM) ;
}


static  void  Init_Glass(Work *work, DG_TEX *tex)
{
	DG_PRIM2	   *prim ;				  /* ガラスのプリミティブ */
	GLASS		  *gls ;				

	FVECTOR		*pos_a, *pos_b ;		 /* 位置 */
	DG_PRIM2_UVRGB *uvrgb_a, *uvrgb_b ;	 /* uvrgb */
	FVECTOR		*base, *speed ;		  /* 基準値とスピード */
	SVECTOR		*rot, *const_rot ;				   /* 回転 */
	int			i ;		   
	int			alpha ;

	/* 基準点と速度を決定 */
	gls  = work->glass ;
	base = gls->base ;
	speed = gls->speed ;
	Decide_Base(base,speed,work->brk_def->models,&work->brk_objs->world) ;

	/* 回転度 */
	prim = gls->prim ;
	alpha = 24 + irnd() % 48 ;

	rot   = gls->rot ;
	const_rot = gls->const_rot ;
	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;
	uvrgb_a = prim->uvrgb[0] ;
	uvrgb_b = prim->uvrgb[1] ;

	for(i = 0; i < N_GLASS_NUM; i++)
	{
	rot->vx = irnd() % 512 ;
	if(rot->vx % 2)
		rot->vx *= -1 ;
	const_rot->vx = rot->vx ;
	rot->vy = irnd() % 512 ;
	if(rot->vy % 2)
		rot->vy *= -1 ;
	const_rot->vy = rot->vy ;
	rot->vz = irnd() % 4096 ;
	const_rot->vz = 0 ;

	DG_SetPos2(base,rot) ;
	DG_PutVector(FL_LIGHT_Triangle,pos_a,N_TRIANGL_VERTS) ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;

	uvrgb_a->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x8fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	uvrgb_a->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x8fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	uvrgb_a->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x0fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	
	base  ++ ;
	rot   ++ ;
	const_rot ++ ;
	}
}

int  FL_LIGHT_InitGlass(Work *work)
{
	GLASS	   *glass ;
	DG_PRIM2	*prim ;
	DG_TEX	  *tex ;
	int		 data_size ;

	/* 領域確保 */
	glass = work->glass = GV_Malloc( sizeof(GLASS) ) ;
	if(glass == NULL)
	{
	return 1 ;
	}
	data_size = (sizeof(FVECTOR) + sizeof(SVECTOR)) * N_GLASS_NUM * 2;
	glass->base = GV_Malloc(data_size) ;
	if(glass->base == NULL)
	{
	return 1 ;
	}
	glass->speed = (FVECTOR *)(glass->base + N_GLASS_NUM) ;
	glass->rot   = (SVECTOR *)(glass->speed + N_GLASS_NUM) ;
	glass->const_rot = (SVECTOR *)(glass->rot + N_GLASS_NUM) ;

/*	
	printf("fvector  %d, svector %d\n",sizeof(FVECTOR),sizeof(SVECTOR)) ;
	printf("base  %08x\n",glass->base) ;
	printf("speed %08x\n",glass->speed) ;
	printf("rot   %08x\n",glass->rot) ;
	printf("con   %08x\n",glass->const_rot) ;
*/

	/* プリミティブをセットする*/
	prim = glass->prim = GM_MakePrim2(GLASS_PRIM_TYPE,N_GLASS_PRIMS,N_GLASS_VERTS) ;
	if(prim == NULL)
	{
	return 1 ;
	}

	tex = DG_GetTexture(8617368 /*"col128_add"*/) ;
	if(tex == NULL)
	{
	printf("not texture\n") ;
	return -1 ;
	}
	DG_ConfigPrim2Tex(prim,tex) ;

	/* 初期化部分 */
	Init_Glass(work,tex) ;

	glass->act = (void *)FL_LIGHT_Gls_Act ;
	glass->glass_count = MAX_GLS_COUNT ;

	return 0 ;
	
}

/* ------------------------------------------------------------- */
/*				   ボンボリ部分								*/
/* ------------------------------------------------------------- */
/* メイン部分 */
static  int  FL_LIGHT_InitMainLight(Work *work)
{
	FLASHLIGHT		 *flashlight ;
	DG_PRIM2		   *prim ;
	DG_TEX			 *tex ;
	FVECTOR			*pos_a, *pos_b ;
	DG_PRIM2_UVRGBWH   *uvrgbwh_a, *uvrgbwh_b ;
	int				i ;
	FVECTOR			shift ;
	
	/* ボンボリを用意 */
	flashlight = &work->flashlight ;
	prim = flashlight->prim = GM_MakePrim2(MAIN_PRIM_TYPE,N_MAIN_PRIMS,N_MAIN_VERTS) ;
	if(prim == NULL)
	{
	printf("flashlight.c(lamp); no prims\n") ;
	return 1 ;
	}
	/* テクスチャを用意する */
	tex = DG_GetTexture(15442411 /*"light03_msk"*/) ;
	if( tex == NULL)
	{
	printf("trap_c4.c(lamp) : no texture\n") ;
	return 1 ;
	}
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0) ;
	
	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;
	uvrgbwh_a = prim->uvrgb[0] ;
	uvrgbwh_b = prim->uvrgb[1] ;

	DG_SetPos2(&work->pos, &work->rot) ;
	shift.vx = 0.0F ;
	shift.vy = 107.0F ;
	shift.vz = -213.0F ;
	shift.vw = 0.0F ;

	/* 位置決定 */
	for(i = 0; i < N_MAIN_VERTS; i++)
	{
	DG_PutVector(&shift,pos_a,1) ;
	*pos_b = *pos_a ;
	shift.vy -= 30.0F * 0.04F ;
	shift.vz -= 30.0F ;
	pos_a ++ ;
	pos_b ++ ;
	}
	
	for(i = 0; i < (N_MAIN_VERTS - 1); i++)
	{
	uvrgbwh_a->u0 = FTOI12(tex->u_offset) ;
	uvrgbwh_a->v0 = FTOI12(tex->v_offset) ;
	uvrgbwh_a->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh_a->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh_a->q0 = 4096 ;
	uvrgbwh_a->q1 = 4096 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ;
	/* WH値は整数 */
	uvrgbwh_a->w = uvrgbwh_a->h = 80 + 30 * i ;
	uvrgbwh_a->r = 100 ;
	uvrgbwh_a->g = 128 ;
	uvrgbwh_a->b = 128 ;
	uvrgbwh_a->a = 64 - i*3;
		/* もう一方は表示しない */
	*uvrgbwh_b = *uvrgbwh_a ;
	uvrgbwh_a ++ ;
	uvrgbwh_b ++ ;
	}

	uvrgbwh_a->u0 = (uvrgbwh_a-1)->u0 ;
	uvrgbwh_a->v0 = (uvrgbwh_a-1)->v0 ;
	uvrgbwh_a->u1 = (uvrgbwh_a-1)->u1 ;
	uvrgbwh_a->v1 = (uvrgbwh_a-1)->v1 ;
	uvrgbwh_a->q0 = (uvrgbwh_a-1)->q0 ;
	uvrgbwh_a->q1 = (uvrgbwh_a-1)->q1 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ; 
	uvrgbwh_a->w = uvrgbwh_a->h = 80 + 30 * (i - 1) ;
	uvrgbwh_a->r = 100 ;
	uvrgbwh_a->g = 128 ;
	uvrgbwh_a->b = 128 ;
	uvrgbwh_a->a = 64 - i * 3;
	*uvrgbwh_b   = *uvrgbwh_a ;

	return 0 ;
}

static  void  InitPrimData(Work *work, DG_PRIM2 *prim, DG_TEX *tex, int rgb)
{
	DG_PRIM2_UVRGB		*uvrgb ;

	int			i,j,k ;
	for(i = 0; i < 2 ; i++)
	{
	uvrgb = prim->uvrgb[i] ;
	for(j = 0; j < N_HL_PRIMS; j++)
	{
		for(k = 0; k < (N_HL_VERTS / 4); k++)
		{
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = 0 ;
		uvrgb++;
		}
	}
	}
}

static  int  FL_LIGHT_InitHalation(Work *work,int map)
{
	FLASHLIGHT	 *fl   ;  /* フラッシュライト */
	DG_PRIM2	   *prim ;
	DG_TEX		 *tex ;

	/* 減算ハレーション */
	fl   = &work->flashlight ;
	prim = fl->prim_hl_sub = GM_MakePrim2(HL_PRIM_TYPE, N_HL_PRIMS, N_HL_VERTS );
	if(prim == NULL){ printf("null prim\n"); return 1; }
	
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	if(tex == NULL){ printf("no texture  drop01 in fl_light_ini.c\n") ; return 1 ; }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(2,0,0,1,0x00)) ;
	
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
	InitPrimData(work,prim,tex,64) ;
//  prim_sc->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_InvisiblePrim2(prim) ;
	GM_GroupPrim2( prim, map );

	/* テクスチャのαで明暗強調 */
	prim = fl->prim_hl = GM_MakePrim2( HL_PRIM_TYPE, N_HL_PRIMS, N_HL_VERTS );
	if(prim == NULL){ printf("null prim\n"); return 1; }
	tex = DG_GetTexture( 9776993 /*"drop01_alp2_mod1021"*/ );
	if(tex == NULL){ printf("no texture  drop01 in fl_light_ini.c\n") ; return 1 ; }
	DG_ConfigPrim2Tex( prim, tex );
	InitPrimData( work, prim, tex, 255 );
//  prim_sc->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_InvisiblePrim2(prim) ;
	GM_GroupPrim2( prim, map );

	/* つうじょうのハレーション */
	prim = fl->prim_hl_add = GM_MakePrim2( HL_PRIM_TYPE, N_HL_PRIMS, N_HL_VERTS );
	if(prim == NULL){ printf("null prim\n") ; return 1 ; }
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	if(tex == NULL){ printf("no texture  drop01 in fl_light_ini.c\n") ; return 1 ; }
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;

	InitPrimData( work, prim, tex, 64 );
//	prim_sc->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_InvisiblePrim2(prim) ;
	GM_GroupPrim2( prim, map );

	return 0 ;
}

int   FL_LIGHT_InitLight(Work *work,int map)
{
	if(FL_LIGHT_InitMainLight(work))
	return 1 ;
	
	if(FL_LIGHT_InitHalation(work, map))
	return 1 ;

	return 0 ;
}


/* ------------------------------------------------------------- */
/*				   モデル部分								  */
/* ------------------------------------------------------------- */
/* ターゲットの作成 */
static  void  FL_LIGHT_InitTarget(Work *work, FMATRIX *world, DG_MDL *mdl )
{
	TARGET   *t ;
	FVECTOR  t_size ;
	FVECTOR  pos ;

	t = &work->target ;

	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly) * 0.5F ;
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;

	pos.vx = 0.0F ;
	pos.vy = t_size.vy ;
	pos.vz = - 20.0F ;

	GM_SetTarget(t,TARGET_DEFENSE | TARGET_ROTATE, work->map, ENEMY_SIDE, &t_size, &pos) ;
	GM_SetPowerTarget(t,&work->power,POWER_ONCE,1,0,0,&DG_ZeroVector) ;
	GM_SetTargetCallBack(t,FL_LIGHT_TargetCallBack, work) ;
	GM_PutTarget(t) ;

	GM_CurrentMap = work->map;	//act control で やるものの代用
	GM_MoveTarget2(t,&work->objs->world) ;
//	NewTargetView(t, 0, 255, 0) ;

}

/* 通常モデル */
static  int  FL_LIGHT_MakeObj(Work *work)
{
	int	   model_id ;
	DG_OBJS   *objs ;
	DG_DEF	*def ;

	if( GCL_GetOption( 'm' ) != NULL ){
		model_id = GCL_GetNextInt() ;
	}else{
		printf("there is not option m of flashlight \n") ;
		return 1 ;
	}

	/* ライトのモデル初期化部分 */   
	def = (DG_DEF *)GV_GetCache(GV_CacheID(model_id,'k')) ;
	if(def == NULL){
		printf("not flashlight.kms\n") ;
		return 1 ;
	}
	
	/* モデルを取得 */
	objs = work->objs = DG_MakeObjs(def,BODY_FLAG, 0) ;
	if(objs == NULL) return 1 ;
	DG_QueueObjs((DG_OBJS*)work->objs) ;   /* モデル登録 */
	/* 位置、回転、光源初期化 */
	DG_SetPos2(&work->pos,&work->rot) ;
	DG_PutObjs(objs) ;
	work->light[0] = work->light[1] = DG_UnitMatrix ;
	DG_SetLightMatrix(objs,work->light) ;
	/* ターゲット作成 */
	FL_LIGHT_InitTarget(work,&objs->world,def->models) ;

	return 0 ;
}

/* 壊れモデル */
static  int  FL_LIGHT_MakeBrkObjs(Work *work)
{
	/* 壊れモデルも用意しておく */
	work->brk_def = (DG_DEF *)GV_GetCache(GV_CacheID(10873964 /*"flashlight_koware"*/,'k')) ;
	if(work->brk_def == NULL){
		printf("not flashlight_koware.kms\n") ;
		return 1 ;
	}
	/* モデルを取得 */
	work->brk_objs = DG_MakeObjs(work->brk_def,BODY_FLAG, 0) ;
	if( work->brk_objs == NULL) return 1 ;

	DG_QueueObjs((DG_OBJS*)work->brk_objs) ;
	DG_SetPos2(&work->pos,&work->rot) ;
	DG_PutObjs(work->brk_objs) ;

	return 0 ;
}

int   FL_LIGHT_InitModel(Work *work)
{
	FLASHLIGHT  *fl ;



	if( work->brk_flag==0 ){	// 壊れていない
		/* 懐中電灯のモデル */
		if(FL_LIGHT_MakeObj(work)) return 1 ;

		/* 懐中電灯の壊れモデル */
		if(FL_LIGHT_MakeBrkObjs(work)) return 1 ;
		DG_InvisibleObjs(work->brk_objs) ;

		fl = &work->flashlight ;
		fl->act = (void *)FL_LIGHT_NormalAct;
	}else{
		/* 懐中電灯の壊れモデル */
		if(FL_LIGHT_MakeBrkObjs(work)) return 1 ;
		DG_VisibleObjs(work->brk_objs) ;

		fl = &work->flashlight ;
		fl->act = (void *)FL_LIGHT_None_Act;

	}


	return 0 ;


}

/* ------------------------------------------------------------- */
/*				   オプション								  */
/* ------------------------------------------------------------- */
int  FL_LIGHT_GetOptionValue(Work *work)
{
	int   buf[3] ;

	/* ポジションを決定 */
	if(GCL_GetOption('p') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf,(float *)(&work->pos),3) ;
	}else{
		printf("flashlight.c : not position\n") ;
		return 1 ;
	}

	/* 回転を決定 */
	if( GCL_GetOption('r') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		work->rot.vx = buf[0] ;
		work->rot.vy = buf[1] ;
		work->rot.vz = buf[2] ;
	}else{
		printf("flashlight.c : not rotation\n") ;
		return 1 ;
	}

	work->brk_flag = 0;
	if( GCL_GetOption('f') != NULL ){
		work->brk_flag = GCL_GetNextInt();
	}

	work->id = -1;
	if( work->brk_flag==0 ){	// 壊れていない
		/* プロック部分 */
		if( GCL_GetOption('e') != NULL ){
			work->id = GCL_GetNextInt() ;
		}
	}

	return 0 ;
}
