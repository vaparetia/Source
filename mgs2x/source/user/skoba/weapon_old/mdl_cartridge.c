//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mdl_cartridge.c
	薬莢の３Dモデル
	2000/04/10 S.Okajima
	2001/03/29 S.Kobayashi
	$Id: mdl_cartridge.c,v 1.1.1.3 2002/11/19 11:50:36 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include    "../test/etc.h"


#define	  COMDL_FLAG		(DG_COMDL_DEFAULT | DG_COMDL_SEMITRANS)
#define		BULLET_E			(0.6F)
#define   N_OBJS			(32)
#define   MAX_COUNT			(3600)
#define   CRTRDG_HZX_SPHERE	(2000)
#define   ROTATION_MIN		(32)
#define   ROTATION_RND		(128)
#define	  DECAY_RATIO		(0.5f)
#define	  DECAY_RATIO_W		(0.8f)
#define   DEFAULT_TIME		(20.0f)

#define   CRTRDG_MOVE		(0x01)
#define   CRTRDG_STOP		(0x02)
#define   CRTRDG_READY		(0x04)
#define   CRTRDG_INVISIBLE	(0x08)

#define   VEC_LIMIT			(-250.0f)

#define   BOUND_DISTANCE	(5000.0f)

extern int  DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
extern void *NewConnectSmoke_SKOBA( FVECTOR *pos, SVECTOR *rot, FVECTOR *shift, u_char *mode, float size_rnd, CVECTOR col );

typedef struct
{
	FVECTOR		pos ;			/* 位置 */
	FVECTOR	  speed ;		/* スピード */
	SVECTOR	  rot ;			/* 回転 */
	SVECTOR	  const_rot;	/* 回転スピード */
	float     time;         // 時間 
	float     point_time;   // 何かしらのタイミング 
	u_char	  mode;			/* モードをチェック */		  
	int		   map;
} UNIT ;

typedef struct
{
	GV_ACT_EX		actor ;
	HZX_GROUP_ID	hzx_id ;
	DG_COMDL		*comdl ;
	int				count_all ;
	int				kind;
	int				init_check_count;
	UNIT			unit[N_OBJS] ;
} Work ;

// スケールデータ入力ヅミ 2001/04/14 S.Kobayashi 
enum {
	AMO_KIND_USP = 0,
	AMO_KIND_FMS,
	AMO_KIND_P90,
	AMO_KIND_GLK,
	AMO_KIND_M4A1,
	AMO_KIND_ABK,
	AMO_KIND_AKS,
	AMO_KIND_PSG,
	AMO_KIND_SAA,
	AMO_KIND_M4_DEMO,
	AMO_KIND_M4_DEMO_GUN,
	AMO_KIND_SOCOM,
	AMO_KIND_SPS,
	AMO_KIND_MECA,
};

Work   *BIG_CARTRIDGE_USP_WORK = NULL ;
Work   *BIG_CARTRIDGE_SOCOM_WORK = NULL ;
Work   *BIG_CARTRIDGE_AKS_WORK = NULL ;
Work   *BIG_CARTRIDGE_FMS_WORK = NULL ;
Work   *BIG_CARTRIDGE_SPS_WORK = NULL ;
Work   *BIG_CARTRIDGE_P90_WORK = NULL ;
Work   *BIG_CARTRIDGE_GLK_WORK = NULL ;
Work   *BIG_CARTRIDGE_PSG_WORK = NULL ;
Work   *BIG_CARTRIDGE_M4A1_WORK  = NULL ;
Work   *BIG_CARTRIDGE_ABK_WORK  = NULL ;
Work   *BIG_CARTRIDGE_MECA_WORK  = NULL ;
Work   *BIG_CARTRIDGE_M4_DEMO_WORK  = NULL ;
Work   *BIG_CARTRIDGE_M4_DEMO_GUN_WORK  = NULL ;

/*----------------------------------------------------------------*/
// 床面を消しても残る問題用 
static void BoundCallBack( Work *work )
{
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		work->init_check_count = 2;
	}
}

/* -----------------------------------------------------------*/
static void CheckAndClear( Work *work )
{
	int		  i ;
	UNIT	  		*unit ;
	DG_COMDL		*comdl ;
	DG_COMDL_POS	*cmdl_pos ;

	work->init_check_count--;

	unit     = work->unit ;
	comdl    = work->comdl ;
	cmdl_pos = comdl->pos;
	for(i = 0 ; i < N_OBJS; i++ ){
		if ( UTL_EFT_CheckBound( (FVECTOR *)cmdl_pos->world.m[3] ) >= 0 ){	/*バウンドに入る？ */
			unit->mode = 0 ;
			cmdl_pos->color.vx = 0 ;
			cmdl_pos->color.vy = 0 ;
			cmdl_pos->color.vz = 0 ;
			cmdl_pos->color.vw = 0 ;
		}
		unit++;
		cmdl_pos++;
	}
}

#if 0
/* USPモデルの呼び出し */
void *MdlCartridg_USP(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	int			count_all ;
	UNIT		   *unit ;
	DG_COMDL_POS   *comdl_pos ;

	if(BIG_CARTRIDGE_USP_WORK == NULL){
		printf("BIG_CARTRIDGE_USP_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}
	count_all = BIG_CARTRIDGE_USP_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_USP_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed  = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128-64;
	unit->rot.vy = irnd()%128-64;
	unit->rot.vz = irnd()%128-64 ;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_USP_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
 	}
	BIG_CARTRIDGE_USP_WORK->count_all = count_all ;
	return 0 ;
}

/* Socomモデルの呼び出し */
void *MdlCartridg_SOCOM(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	int			count_all ;
	UNIT		   *unit ;
	DG_COMDL_POS   *comdl_pos ;

	if(BIG_CARTRIDGE_SOCOM_WORK == NULL){
		printf("BIG_CARTRIDGE_SOCOM_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}
	count_all = BIG_CARTRIDGE_SOCOM_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_SOCOM_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed  = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128-64;
	unit->rot.vy = irnd()%128-64;
	unit->rot.vz = irnd()%128-64 ;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_SOCOM_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
 	}
	BIG_CARTRIDGE_SOCOM_WORK->count_all = count_all ;
	return 0 ;
}

/* FMSモデルの呼び出し */
void *MdlCartridg_FMS(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	int			count_all ;
	UNIT		   *unit ;
	DG_COMDL_POS   *comdl_pos ;

	if(BIG_CARTRIDGE_FMS_WORK == NULL){
		printf("BIG_CARTRIDGE_FMS_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_FMS_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_FMS_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed  = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128-64 ;
	unit->rot.vy = irnd()%128-64 ;
	unit->rot.vz = irnd()%128-64 ;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_FMS_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_FMS_WORK->count_all = count_all ;
	return 0 ;
}


/* AKSモデルの呼び出し */
void *MdlCartridg_AKS(
FVECTOR  *pos,							   
FVECTOR  *speed,			  /* スピード */
SVECTOR  *rot,
OBJECT   *weapon,
CONTROL  *control			  /* 人体コントロール */
)
{
	int  count_all ;
	UNIT *unit ;
	DG_COMDL_POS *comdl_pos ;
	if(BIG_CARTRIDGE_AKS_WORK == NULL){
		printf("BIG_CARTRIDGE_AKS_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}
	count_all = BIG_CARTRIDGE_AKS_WORK->count_all ;

	/* 初期化 */
	unit = &(BIG_CARTRIDGE_AKS_WORK->unit[count_all]) ;
	unit->pos   = pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.90f ;
		unit->speed.vy *= 1.20f ;
		unit->speed.vz *= 0.90f ;
	}

	unit->rot.vx = irnd()%128-64 ;
	unit->rot.vy = irnd()%128-64 ;
	unit->rot.vz = irnd()%128-64 ;
	unit->const_rot = rot[0] ;
//	unit->const_rot.vx = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1);
//	unit->const_rot.vy = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1) ;
//	unit->const_rot.vz = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1) ;

	unit->mode = CRTRDG_MOVE;

	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_AKS_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_AKS_WORK->count_all = count_all ;

	return 0 ;
}

/* SPSモデルの呼び出し */
void *MdlCartridg_SPS(
FVECTOR  *pos,							   
FVECTOR  *speed,			  /* スピード */
SVECTOR  *rot,
OBJECT   *weapon,
CONTROL  *control			  /* 人体コントロール */
)
{
	int  count_all ;
	UNIT *unit ;
	DG_COMDL_POS *comdl_pos ;
	CVECTOR color;
	float ftmp;

	if(BIG_CARTRIDGE_AKS_WORK == NULL){
		printf("BIG_CARTRIDGE_AKS_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_SPS_WORK->count_all ;

	/* 初期化 */
	unit = &(BIG_CARTRIDGE_SPS_WORK->unit[count_all]) ;
	unit->pos   = pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	ftmp = rnd() * 20.0f;
	unit->point_time = DIRECT_TICK( ftmp );
	{
		unit->speed.vx *= 0.90f ;
		unit->speed.vy *= 1.20f ;
		unit->speed.vz *= 0.90f ;
	}

	unit->rot.vx = irnd()%128-64 ;
	unit->rot.vy = irnd()%128-64 ;
	unit->rot.vz = irnd()%128-64 ;
	unit->const_rot = rot[0] ;
//	unit->const_rot.vx = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1);
//	unit->const_rot.vy = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1) ;
//	unit->const_rot.vz = (ROTATION_RND * irnd() + ROTATION_MIN) * (irnd()%2*2-1) ;

	unit->mode = CRTRDG_MOVE;

	color.r = 74;
	color.g = 74;
	color.b = 74;
	NewConnectSmoke_SKOBA( &unit->pos , &unit->rot , &DG_ZeroVector , &unit->mode , 120.f , color );

	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_SPS_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_SPS_WORK->count_all = count_all ;

	return 0 ;
}

/* P90モデルの呼び出し 薬莢 */	
void *MdlCartridg_P90(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	int			count_all;
	UNIT		   *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_P90_WORK == NULL){
		printf("BIG_CARTRIDGE_P90_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_P90_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_P90_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128 - 64;
	unit->rot.vy = irnd()%128 - 64;
	unit->rot.vz = irnd()%128 - 64;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_P90_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_P90_WORK->count_all = count_all ;
	return 0 ;
}

/* PSGモデルの呼び出し 薬莢 */	
void *MdlCartridg_PSG(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	int			count_all;
	UNIT		   *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_PSG_WORK == NULL){
		printf("BIG_CARTRIDGE_PSG_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_PSG_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_PSG_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128 - 64;
	unit->rot.vy = irnd()%128 - 64;
	unit->rot.vz = irnd()%128 - 64;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_PSG_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_PSG_WORK->count_all = count_all ;
	return 0 ;
}

/* GLKモデルの呼び出し 薬莢 */	
void *MdlCartridg_GLK(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	int			count_all;
	UNIT	 	    *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_GLK_WORK == NULL){
		printf("BIG_CARTRIDGE_GLK_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_GLK_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_GLK_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%1024 - 512;
	unit->rot.vy = irnd()%512 - 256;
	unit->rot.vz = irnd()%1024 - 512;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_GLK_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_GLK_WORK->count_all = count_all ;
	return 0 ;
}

// M4A1モデルの呼び出し 薬莢 (ライデン用)	 
void *MdlCartridg_M4A1(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	int			count_all;
	UNIT	 	    *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_M4A1_WORK == NULL){
		printf("BIG_CARTRIDGE_M4A1_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_M4A1_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_M4A1_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%1024 - 512;
	unit->rot.vy = irnd()%512 - 256;
	unit->rot.vz = irnd()%1024 - 512;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_M4A1_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_M4A1_WORK->count_all = count_all ;
	return 0 ;
}

// MECAモデルの呼び出し 薬莢 (Cayphe用)	 
void *MdlCartridg_MECA(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	int			count_all;
	UNIT	 	    *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_MECA_WORK == NULL){
		printf("BIG_CARTRIDGE_MECA_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_MECA_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_MECA_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%1024 - 512;
	unit->rot.vy = irnd()%512 - 256;
	unit->rot.vz = irnd()%1024 - 512;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_MECA_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_MECA_WORK->count_all = count_all ;
	return 0 ;
}

// ABAKANモデルの呼び出し	 
void *MdlCartridg_ABK( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	int			count_all;
	UNIT	 	    *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_ABK_WORK == NULL){
		printf("BIG_CARTRIDGE_ABK_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_ABK_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_ABK_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%1024 - 512;
	unit->rot.vy = irnd()%512 - 256;
	unit->rot.vz = irnd()%1024 - 512;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_ABK_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_ABK_WORK->count_all = count_all ;
	return 0 ;
}

/* M4モデルの呼び出し 薬莢 */	
void *MdlCartridg_M4_demo(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	int			count_all;
	UNIT		   *unit;
	DG_COMDL_POS   *comdl_pos;
	CVECTOR        color;

	if(BIG_CARTRIDGE_M4_DEMO_WORK == NULL){
		printf("BIG_CARTRIDGE_M4_DEMO_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_M4_DEMO_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_M4_DEMO_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( 20.0f );
	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%128 - 64;
	unit->rot.vy = irnd()%128 - 64;
	unit->rot.vz = irnd()%128 - 64;
	color.r = 74;
	color.g = 74;
	color.b = 74;

	NewConnectSmoke_SKOBA( &unit->pos , &unit->rot , &DG_ZeroVector , &unit->mode , 80.f , color );

	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_M4_DEMO_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_M4_DEMO_WORK->count_all = count_all ;
	return 0 ;
}

// M4_demo_gunモデルの呼び出し 薬莢 	 
void *MdlCartridg_M4_demo_gun(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	int			count_all;
	UNIT	 	    *unit;
	DG_COMDL_POS   *comdl_pos;

	if(BIG_CARTRIDGE_M4_DEMO_GUN_WORK == NULL){
		printf("BIG_CARTRIDGE_M4_DEMO_GUN_WORK is NULL!!\n");
		printf("stdch.h に 薬莢の常駐記述がされていません。\n");
//		ASSERT(0);
		return NULL ;
	}

	count_all = BIG_CARTRIDGE_M4_DEMO_GUN_WORK->count_all ;
	/* 初期化 */
	unit = &(BIG_CARTRIDGE_M4_DEMO_GUN_WORK->unit[count_all]) ;
	unit->pos	= pos[0] ;
	unit->speed = speed[0] ;
	unit->time = 0.f;
	unit->point_time = DIRECT_TICK( DEFAULT_TIME );

	{
		unit->speed.vx *= 0.70f ;
		unit->speed.vy *= 0.90f ;
		unit->speed.vz *= 0.70f ;
	}

	unit->rot.vx = irnd()%1024 - 512;
	unit->rot.vy = irnd()%512 - 256;
	unit->rot.vz = irnd()%1024 - 512;
	unit->const_rot = rot[0] ;
	unit->mode = CRTRDG_MOVE;
	/* 色決定 */
	comdl_pos = &BIG_CARTRIDGE_M4_DEMO_GUN_WORK->comdl->pos[count_all] ;
	comdl_pos->color.vx = 128 ;
	comdl_pos->color.vy = 128 ;
	comdl_pos->color.vz = 128 ;
	comdl_pos->color.vw = 128 ;

	count_all ++ ;
	if(count_all >= N_OBJS){
		count_all = 0 ;
	}
	BIG_CARTRIDGE_M4_DEMO_GUN_WORK->count_all = count_all ;
	return 0 ;
}
#endif
/* ------------------------------------------------------------------- */
/*	サウンド */
static  void  SetSound(FVECTOR *speed,int se, FVECTOR  *pos, int mode)
{
	float	len ;
	int	  pan ;
	int	  vol ;
   float bp_angle;

	len = GV_VecLen3F(speed) ;

	if(len >= 40.0F){
		vol = GM_SeGetVol(pos, mode) ;
		pan = GM_SeGetPan(pos, mode, &bp_angle) ;
		if(len <= 200.0F) vol = (int)( (float)vol * (len - 40.0F) / 160.0F) ;
		GM_SeSet3D(pan,vol,se, bp_angle) ;
	}
}

static void ModelScale( FMATRIX *pMat , int kind ) // 薬莢スケーリング 
{
	FVECTOR scale;

	switch( kind ){
	case AMO_KIND_USP :
	case AMO_KIND_FMS : 
	case AMO_KIND_SOCOM : 
		scale.vx = 1.0f;
		scale.vy = 1.0f;
		scale.vz = 1.0f;
		break;
	case AMO_KIND_AKS :
	case AMO_KIND_ABK : 
	case AMO_KIND_M4_DEMO_GUN : 
		scale.vx = 1.0f;
		scale.vy = 0.866f;
		scale.vz = 1.0f;
		break;
	case AMO_KIND_P90 : 
		scale.vx = 1.0f;
		scale.vy = 0.58f;
		scale.vz = 1.0f;
		break;
	case AMO_KIND_PSG : 
	case AMO_KIND_SAA : 
		scale.vx = 1.0f;
		scale.vy = 2.0f;
		scale.vz = 1.0f;
		break;
	default           : 
		scale.vx = 1.0f;
		scale.vy = 1.0f;
		scale.vz = 1.0f;
		break;
	}
	scale.vw = 1.0f;

	DG_SetPos( pMat );
	DG_ScalePos( &scale );
	DG_GetPos( pMat );
}

/* ---------------------------------------------------------   */
static  void  Act(Work *work)
{
	UNIT			*unit ;
	DG_COMDL		*comdl ;
	DG_COMDL_POS	*cmdl_pos ;
	FVECTOR			*pos ;
	FVECTOR			*vec ;
	FVECTOR			fvtemp0 ;
	FVECTOR			fvtemp1 ;
	FVECTOR			cam_pos ;
	HZX_SEG			*seg ;
	HZX_FLR			flr[2] ;
	int				map_id;
	int				i;
	int				atr[2] ;
	int				h_flag;
	float			f_abs;
	float			ftemp=0.0f;
	float			angle=0.0f;
	float			bound_dis;


	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls[0].eye.m[3] );

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );


	if( work->init_check_count ) CheckAndClear( work );

	comdl    = work->comdl ;

	GM_GroupObject( comdl, GM_CurrentStageMap ) ;


	bound_dis = BOUND_DISTANCE * DG_Chanls->screen;
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		unit     = work->unit ;
		cmdl_pos = comdl->pos;
		for(i = 0; i < N_OBJS; i++,unit++,cmdl_pos++ ){
			pos = &unit->pos;
			if( DG_FABS(pos->vx-cam_pos.vx) > bound_dis
			       || DG_FABS(pos->vy-cam_pos.vy) > bound_dis
			       || DG_FABS(pos->vz-cam_pos.vz) > bound_dis ){
				unit->mode &= ~CRTRDG_MOVE;
				cmdl_pos->color.vw = 0;
			}
		}
	}

	unit     = work->unit ;
	cmdl_pos = comdl->pos;
	for(i = 0; i < N_OBJS; i++,unit++,cmdl_pos++ ){ /* 非表示なら何もしない */
		pos = &unit->pos;
		vec = &unit->speed;

		if( unit->mode & CRTRDG_STOP ){
			unit->mode = CRTRDG_MOVE;
		}

		/* 動いていないとき */
		if(!(unit->mode & CRTRDG_MOVE)){
			continue ;
		}

		if ( unit->time < unit->point_time ){
			unit->time += 1.0f;
		} else {
			if ( ( work->kind == AMO_KIND_M4_DEMO || work->kind == AMO_KIND_SPS ) && !( unit->mode & CRTRDG_INVISIBLE ) ){
				unit->mode = CRTRDG_STOP;
			}
		}

		/* ----------------------------------------------*/
		/* 位置,スピード,回転を更新 */
		if( vec->vy < VEC_LIMIT ){
			unit->mode &= ~CRTRDG_MOVE;
			cmdl_pos->color.vw = 0;
			continue;
		}

		_sceVu0AddVector(&fvtemp0,pos,vec) ;

		if( vec->vy < 0.0f ){
			h_flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
				map_id,
				pos,
				&fvtemp0,
				HZX_CHK_FIX,
				HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD | HZX_FLOOR_RECOIL_TYPE );
		}else{
			h_flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
				map_id,
				pos,
				&fvtemp0,
				HZX_CHK_F_SEGMENT,
				HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD | HZX_FLOOR_RECOIL_TYPE );
			}
		if( h_flag & 1 ){	/* 壁 */
			SetSound(vec, SD_W_CASE03, pos, GM_SEMODE_NORMAL) ;
			HZX_GetOnlinePoint( &fvtemp1 );
//			_sceVu0SubVector( &fvtemp2, &fvtemp1, &fvtemp0 ) ;
			HZX_GetOnlineHazard( flr, atr ) ;
			seg=(HZX_SEG *)flr;
			fvtemp0.vx = seg->p2.z - seg->p1.z ;
			fvtemp0.vy = 0.0f ;
			fvtemp0.vz = seg->p1.x - seg->p2.x ;
			DG_ReflectVector( &fvtemp0, vec, vec );
			vec->vx *= DECAY_RATIO;
			vec->vy *= DECAY_RATIO;
			vec->vz *= DECAY_RATIO;
//			_sceVu0AddVector( pos, &fvtemp1, vec ) ;
			pos->vx = fvtemp1.vx + vec->vx;
			pos->vy = fvtemp1.vy;
			pos->vz = fvtemp1.vz + vec->vz;
			unit->rot.vx = irnd()%512-256 ;
			unit->rot.vy = irnd()%512-256 ;
			unit->rot.vz = irnd()%512-256 ;
		} else if( h_flag & 2 ){	/* 床 */
			HZX_GetOnlinePoint( &fvtemp1 );
			HZX_GetOnlineHazard( flr, atr ) ;
			f_abs=( vec->vy > 0.0f )?vec->vy:-vec->vy;
			if( f_abs > 10.0f ) SetSound(vec, SD_W_CASE03, pos, GM_SEMODE_NORMAL) ;
//if(i==0) printf("%f %f\n",f_abs,P_GRAVITY);
			if( f_abs < DG_FABS(P_GRAVITY)*2.0f ){
				vec->vx = 0.0f;
				vec->vy = 0.0f;
				vec->vz = 0.0f;
				pos->vy = fvtemp1.vy;
				unit->rot.vx = 0 ;
				unit->rot.vy = 0 ;
				unit->rot.vz = 1024 ;
				unit->const_rot.vx = 0;
				unit->const_rot.vz = 0;
				unit->mode &= ~CRTRDG_MOVE;
				if( atr[0] & HZX_FLOOR_NO_OBJECT ){	// 停止後の可視決定 
//					cmdl_pos->color.vw = 0;
//					unit->mode |= CRTRDG_INVISIBLE;
				}
			}else{
				if( (irnd()>>8)%4==0  &&  f_abs > 10.0f ){
					ftemp = bp_sqrtf( vec->vx * vec->vx  +  vec->vz * vec->vz );   //BP_MATH - emulate PS2 sqrtf
					angle = rnd()*TPI;
					vec->vx = ftemp * sinf( angle ) * DECAY_RATIO_W;
					vec->vz = ftemp * cosf( angle ) * DECAY_RATIO_W;
				}else{
					vec->vx*= DECAY_RATIO_W;
					vec->vz*= DECAY_RATIO_W;
				}
				if( (irnd()>>8)%64==0  &&  f_abs > 10.0f ){
					vec->vy  = -vec->vy;
				}else{
					vec->vy *= -DECAY_RATIO;
				}
				pos->vx = fvtemp1.vx;
				pos->vy = fvtemp1.vy + vec->vy;
				pos->vz = fvtemp1.vz;
				unit->rot.vx = irnd()%512-256 ;
				unit->rot.vy = irnd()%512-256 ;
				unit->rot.vz = irnd()%512-256 ;
			}
		} else {//		if( !(h_flag & 3) ){	/* 当たらなかった */ 
			DG_COPY_VEC( pos, &fvtemp0 );

			vec->vy += P_GRAVITY;
		}
		if( unit->rot.vx > 0 ){
			unit->rot.vx--;
		}else if( unit->rot.vx < 0 ){
			unit->rot.vx++;
		}
		if( unit->rot.vy > 0 ){
			unit->rot.vy--;
		}else if( unit->rot.vy < 0 ){
			unit->rot.vy++;
		}
		if( unit->rot.vz > 0 ){
			unit->rot.vz--;
		}else if( unit->rot.vz < 0 ){
			unit->rot.vz++;
		}
		unit->const_rot.vx += unit->rot.vx;
		unit->const_rot.vy += unit->rot.vy;
		unit->const_rot.vz += unit->rot.vz;

		DG_SetPos2( &unit->pos, &unit->const_rot) ;
		DG_GetPos( &comdl->pos[i].world) ;
		ModelScale( &comdl->pos[ i ].world , work->kind );
		//場合によってはスケール掛ける 
	}

}

static  void  Die(Work *work)
{
	UTL_EFT_DelCallback( work ) ;

	if(work->comdl != NULL){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
	}

	if( BIG_CARTRIDGE_USP_WORK == work ) BIG_CARTRIDGE_USP_WORK = NULL;
	if( BIG_CARTRIDGE_SOCOM_WORK  == work ) BIG_CARTRIDGE_SOCOM_WORK  = NULL;
	if( BIG_CARTRIDGE_AKS_WORK == work ) BIG_CARTRIDGE_AKS_WORK = NULL;
	if( BIG_CARTRIDGE_FMS_WORK == work ) BIG_CARTRIDGE_FMS_WORK = NULL;
	if( BIG_CARTRIDGE_SPS_WORK == work ) BIG_CARTRIDGE_SPS_WORK = NULL;
	if( BIG_CARTRIDGE_P90_WORK == work ) BIG_CARTRIDGE_P90_WORK = NULL;
	if( BIG_CARTRIDGE_PSG_WORK == work ) BIG_CARTRIDGE_PSG_WORK = NULL;
	if( BIG_CARTRIDGE_GLK_WORK == work ) BIG_CARTRIDGE_GLK_WORK = NULL;
	if( BIG_CARTRIDGE_M4A1_WORK == work ) BIG_CARTRIDGE_M4A1_WORK = NULL;
	if( BIG_CARTRIDGE_ABK_WORK == work ) BIG_CARTRIDGE_ABK_WORK = NULL;
	if( BIG_CARTRIDGE_M4_DEMO_WORK  == work ) BIG_CARTRIDGE_M4_DEMO_WORK  = NULL;
	if( BIG_CARTRIDGE_M4_DEMO_GUN_WORK  == work ) BIG_CARTRIDGE_M4_DEMO_GUN_WORK  = NULL;
	if( BIG_CARTRIDGE_MECA_WORK == work ) BIG_CARTRIDGE_MECA_WORK = NULL;
}

/* -----------------------------------------------------------*/
static  int  GetResources(Work *work, int model_id, int name, int map)
{
	DG_COMDL  	*comdl ;
	DG_DEF		*def ;
	UNIT	  	*unit ;
	int		  i ;

	def = (DG_DEF *)GV_GetCache(GV_CacheID(model_id,'k')) ;
	if(def == NULL){
		printf("not %d strcode --> please runtime find\n", model_id ) ;
		return -1 ;
	}
	comdl = work->comdl = DG_MakeComdl(def->models[0].packs, COMDL_FLAG, N_OBJS, 0);
	if(comdl == NULL) return -1 ;
	DG_QueueComdlObjs( comdl ) ;
	comdl->flag &= 0x0f;
	work->count_all = 0 ;
	/* 初期化 */
	unit = work->unit ;
	for(i = 0 ; i < N_OBJS; i++,unit++){
		/* モード初期化 */
		unit->mode = 0 ;
		/* ワールド座標初期化 */
		comdl->pos[i].world = DG_UnitMatrix ;
		/* 色を初期化 */
		comdl->pos[i].color.vx = 0 ;
		comdl->pos[i].color.vy = 0 ;
		comdl->pos[i].color.vz = 0 ;
		comdl->pos[i].color.vw = 0 ;	
	}

	UTL_EFT_AddCallback( BoundCallBack, work ) ;

	return 0 ;
}

/* ------------------------------------------------- */

static	void  *New(int model_id, int name, int map)
{
	Work *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_EFFECT, sizeof(Work)) ;
//	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if( GetResources( work, model_id, name, map ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void  *NewCartridgeUspALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("usp_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_USP;
	BIG_CARTRIDGE_USP_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeAksALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_FMS;
	BIG_CARTRIDGE_AKS_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeFmsALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_FMS;
	BIG_CARTRIDGE_FMS_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeSpsALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("sps_emb"),name,map) ;
printf("=============================================== NULL:: \n");
	if ( work == NULL ){
printf("=============================================== NULL::0\n");
		work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
		if ( work == NULL ){
printf("=============================================== NULL::1\n");
			return ( NULL );
		}
	}
	work->kind = AMO_KIND_SPS;
	BIG_CARTRIDGE_SPS_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeP90ALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_P90;

	BIG_CARTRIDGE_P90_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeGLKALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("usp_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_GLK;

	BIG_CARTRIDGE_GLK_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeM4A1ALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_M4A1;

	BIG_CARTRIDGE_M4A1_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeABKALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_ABK;

	BIG_CARTRIDGE_ABK_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeM4_demoALL(int name, int map)
{
	Work	 *work;

	work = (Work *)New(GV_StrCode("demo_m4_grn_emb"),name,map);
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_M4_DEMO;

	BIG_CARTRIDGE_M4_DEMO_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeM4_demo_gunALL(int name, int map)
{
	Work	 *work;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map);
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_M4_DEMO_GUN;

	BIG_CARTRIDGE_M4_DEMO_GUN_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeSOCOMALL(int name, int map)
{
	Work	 *work;

	work = (Work *)New(GV_StrCode("usp_emb"),name,map);
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_SOCOM;

	BIG_CARTRIDGE_SOCOM_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgePSGALL(int name, int map)
{
	Work	 *work;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map);
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_PSG;

	BIG_CARTRIDGE_PSG_WORK = work ;

	return (void *)work ;
}

void  *NewCartridgeMECAALL(int name, int map)
{
	Work	 *work ;

	work = (Work *)New(GV_StrCode("fms_emb"),name,map) ;
	if ( work == NULL ){
		return ( NULL );
	}
	work->kind = AMO_KIND_MECA;

	BIG_CARTRIDGE_MECA_WORK = work ;

	return (void *)work ;
}

