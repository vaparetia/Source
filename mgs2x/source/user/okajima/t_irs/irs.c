//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	赤外線  main.c
	1999/??/?? TANAKA
	2000/10/27 S.Okajima
	$Id: irs.c,v 1.1.1.3 2002/11/19 11:47:45 Yoshizawa1 Exp $
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
#include "libutl.h"

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
#include	"../etc/ok_util.h"
/* 
    赤外線

*/

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SENSE_LENGTH   (5000.0f)


#define N_IRS_SENSOR   (2)
#define N_IRS_SENSOR_RAY (10)
#define N_RAY_VERTS    (2)
#define MAX_PATERN   (100)
#define LAMP_N_PRIMS    (1)
#define LAMP_N_VERTS    (2)
#define MAX_BRIGHT      (64.0f)

#define INIT_COUNT       (60*1)
#define MAX_COUNT       (60*60)
#define FINISH_COUNT       (60*55)

#define N_P_MODELS    (21)       /* 人のモデル数 */
#define TOP_INTERVAL  (5)
#define INTERVAL      (5)


#define BODY_FLAG (DG_FLAG_TEXT | DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_ONEPIECE)
#define PRIM_TYPE      ( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING)
#define LAMP_PRIM_TYPE (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define MEM_LAMP_POS      ((void *)(SCRPAD_ADDR + 0x0000))        /* ～ 0x0020 */
#define MEM_LAMP_UVRGBWH  ((void *)(SCRPAD_ADDR + 0x0100))        /* ～ 0x0140 */
#define MEM_RAY_POS       ((void *)(SCRPAD_ADDR + 0x0200))        /* ～ 0x0400 */
#define MEM_RAY_UVRGB     ((void *)(SCRPAD_ADDR + 0x0500))        /* ～ 0x0700 */

extern  FVECTOR  IRS_Box_Color ;

/* ユーティリティ */
extern void _BigScrCopy(void *dst, void *src, int size, int num) ;
extern void _BigMemCopy(void *dst, void *src, int size, int num) ;

extern void *NewIRS_Box_Break_Effect(FVECTOR  *center, SVECTOR  *rot) ;

extern void  *NewSpark1(int n_packets, FVECTOR *center, float min_speed, 
                       float speed_wide, float gravity, SVECTOR *rot, 
		       SVECTOR *rot_wide, FVECTOR *color, float length, int count );

extern void  Big_TmpLight2(FVECTOR  *pos,float    r_range,float    e_range,
			   int      color,int      flag ) ;



typedef struct _box_lamp    BOX_LAMP ;
typedef struct _sensor_ray  SENSOR_RAY ;
typedef struct _sensor      SENSOR ;
typedef struct _box         BOX ;
typedef struct _work        Work ;


/*********************************************************************/
extern int OK_IRS_OnFlag;

/* センサーの穴の位置 */
static FVECTOR   Irs_Sensor_Hole[] =
{
	{ 0.0F, 2280.0F, 75.0F, 1.0F},{ 0.0F, 2075.0F, -60.0F, 1.0F},
	{ 0.0F, 1870.0F, 75.0F, 1.0F},{ 0.0F, 1675.0F, -60.0F, 1.0F},
	{ 0.0F, 1470.0F, 75.0F, 1.0F},
	{ 0.0F, 1150.0F, 75.0F, 1.0F},{ 0.0F, 935.0F, -60.0F, 1.0F},
	{ 0.0F,  730.0F, 75.0F, 1.0F},{ 0.0F, 525.0F, -60.0F, 1.0F},
	{ 0.0F,  310.0F, 75.0F, 1.0F}
} ;

/* ボックスの色 */
FVECTOR  IRS_Box_Color = {40.0F, 255.0F, 100.0F, 0.0F} ;

/* ランプの位置 */
static FVECTOR  IRS_BoxLamp_Pos = { -50.0F,120.0F,90.0F,1.0F } ;



enum
{
    IRS_NORMAL_FLAG   = 0x0000,
    IRS_VISIBLE_FLAG  = 0x0001
} ;

struct _box_lamp
{
    DG_PRIM2  *prim ;
    FVECTOR   base_pos ;               /* スイッチのランプの中心位置 */
    DG_TEX    *tex ;
    FVECTOR   color ;                 /* ランプの色 */
} ;

struct _sensor_ray
{
    DG_PRIM2     *prim ;
    float        patern[N_IRS_SENSOR_RAY] ;    /* テクスチャパターン */
    float        speed[N_IRS_SENSOR_RAY] ;    /* テクスチャ間の移動スピード */
    DG_TEX       *tex ;                   
    int          count ;               /* 表示時間カウント数 */
    int          a_flag ;              /* アルファ値の増減をチェック */
    int          a_flag2 ;              /* アルファ値の増減をチェック */
    float        bright ;              /* 赤外線の明るさ */
    /* ターゲットを見る */
    FVECTOR     center ;               /* 当たり判定の中心 */
    FVECTOR     size ;                 /* 大きさ */

#ifdef DEBUG_MODE
   /* デバッグ用 */
    TARGET      target ;
#endif

};

struct _sensor
{
    FVECTOR   pos[N_IRS_SENSOR] ;
    SVECTOR   rot[N_IRS_SENSOR] ;
    
    DG_OBJS   *objs[N_IRS_SENSOR] ;
    FMATRIX   light[N_IRS_SENSOR][2] ;

    TARGET    target[N_IRS_SENSOR] ;
} ;

struct _box
{
    TARGET         target ;
    POWER_TARGET   power ;

    FVECTOR        pos ;
    SVECTOR        rot ;

    int            ( *act )(  Work *, BOX * ) ;

    DG_OBJS        *objs ;
    DG_MDL         *mdl ;
    DG_OBJS        *brk_objs ;
    FMATRIX        light[2] ;         /*  スイッチ全体のライト */
    FMATRIX        lamplight[2] ;      /*  ランプ用のライト */
    FVECTOR        color ;             
    int            interval ;             /* 間隔  */ 
    int            sign ;              /* 符号 */
    int            hit_flag ;          /* 当たり判定用のフラグ */
    int            count ;

    BOX_LAMP        *lamp ;
} ;  

struct _work
{
    GV_ACT_EX   actor ;
    int      map   ;   /* 現在のマップ情報 */

    BOX      *box ;
    
    SENSOR   *sensor ;

    SENSOR_RAY      *ray ;

    /* シナリオ用 */
    int        command ;
    int        name ;              /* nameを取ってくる */

	int			mode;

	int			mes_flag;
	int			another_flag;
	int			tabac_count;

	int			exec_once;
	int			switch_break_proc;

} ;
/*********************************************************************/
/*********************************************************************/
/*----------------------------------------*/
/* ボンボリの位置 */
/* スクラッチパット使用 */
static void Scr_BoxLamp(BOX *box,BOX_LAMP *lamp)
{
	DG_CHANL		   *cp ;
	FVECTOR			*tmp_pos ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	float			  tmp ;
	int				i ;

	pos	 = MEM_LAMP_POS ;
	uvrgbwh = MEM_LAMP_UVRGBWH ;
	tmp_pos = (void *)(SCRPAD_ADDR + 0x3000) ;

	/* このパートは システム変更次第 書き直し */
	cp = DG_Chanl(0) ;
	*tmp_pos = *(FVECTOR *)cp->eye.m[3] ;
	/*-----------------------------------*/

	/* ライトの位置 */
	/* カメラ方向に１５ｃｍずらす */
	_sceVu0SubVector(tmp_pos,tmp_pos,&lamp->base_pos) ;
	tmp_pos->vw = 0.0F ;
	_sceVu0Normalize(tmp_pos,tmp_pos) ;
	_sceVu0ScaleVectorXYZ(tmp_pos,tmp_pos,150.0F) ;

	for(i = 0 ; i < LAMP_N_VERTS; i++){
		_sceVu0AddVector(pos,&lamp->base_pos,tmp_pos) ;
		pos ++ ;
	}

	/* アルファ値の決定 */
	tmp = box->color.vy / IRS_Box_Color.vy ;
	uvrgbwh[0].w = (short)(50.0F + (lamp->base_pos.vw - 50.0F) * tmp) ;
	uvrgbwh[1].w = (short)(uvrgbwh[0].w * 4.0F) ;
	uvrgbwh[0].h = uvrgbwh[0].w ;
	uvrgbwh[1].h = uvrgbwh[1].w ;
	uvrgbwh[0].a  = (u_short)(72.0F * tmp) ;
	uvrgbwh[1].a  = (u_short)(uvrgbwh[0].a / 4) ;
}

/*********************************************************************/
/* スイッチのライトのエフェクト */
static void IRS_BOXLAMP_Act(BOX *box, BOX_LAMP *lamp)
{
	DG_PRIM2		 *prim ;
	
	/* データ転送 */
	prim = lamp->prim ;
	_BigScrCopy( MEM_LAMP_UVRGBWH, prim->uvrgb[prim->buffer_clock], sizeof(DG_PRIM2_UVRGBWH),LAMP_N_VERTS) ;
	/* スクラッチパット使用 */
	Scr_BoxLamp(box,lamp) ;
	/* 切り替え */
	DG_SwitchBuffPrim2(prim) ;
	/* データ転送 */
	_BigMemCopy(prim->pos[prim->buffer_clock],MEM_LAMP_POS,sizeof(FVECTOR),LAMP_N_VERTS) ;
	_BigMemCopy(prim->uvrgb[prim->buffer_clock],MEM_LAMP_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),LAMP_N_VERTS) ;
	
}

/*********************************************************************/
/* 何もしないときの動作 */
static int IRS_BOX_NONE_Act( Work *work, BOX *box )
{
	BOX_LAMP  *lamp ;

	DG_GetLightMatrix((FVECTOR*)box->brk_objs->world.m[3],box->light) ;
	box->lamplight[0] = box->light[0] ;

	*(FVECTOR *)box->lamplight[0].m[0] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[0].m[1] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[0].m[2] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[0].m[3] = DG_ZeroVector ;

	*(FVECTOR *)box->lamplight[1].m[0] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[1].m[1] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[1].m[2] = DG_ZeroVector ;
	*(FVECTOR *)box->lamplight[1].m[3] = DG_ZeroVector ;
	/* 色決定 */
	box->brk_objs->objs[1].light = box->lamplight ;

	lamp = box->lamp ;
	if(lamp != NULL){
		lamp->prim = OK_FreePrim2( lamp->prim ) ;
		GV_DelayedFree(lamp) ;
		box->lamp = NULL ;
	}

	return 0 ;
}


/* 破壊時の動作 (後編)*/
static int IRS_BreakBOX_Act2( Work *work, BOX *box )
{
	BOX_LAMP   *lamp ;
	float	  min_color,max_color ;
	float	  tmp ;

	lamp = box->lamp ;
	min_color = 0.0F;
	max_color = IRS_Box_Color.vy ;

	/* 色を設定 */
	box->color.vy -= 3.0F ;
	if(box->color.vy > min_color){
		tmp =  (IRS_Box_Color.vz - min_color) * (box->color.vy - min_color) / (max_color - min_color) ;
		box->color.vz = min_color + tmp ;
		tmp =  (IRS_Box_Color.vx - min_color) * (box->color.vy - min_color) / (max_color - min_color) ;
		box->color.vx = min_color + tmp ;
	}else{
		box->color.vx = min_color ;
		box->color.vy = min_color ;
		box->color.vz = min_color ;
		box->act	  = (void *)IRS_BOX_NONE_Act ;
	}
	/* スイッチのエフェクト */
	IRS_BOXLAMP_Act(box,lamp) ;
	DG_GetLightMatrix((FVECTOR*)box->brk_objs->world.m[3],box->light) ;

	box->lamplight[0] = box->light[0] ;
	box->lamplight[1] = box->light[1] ;

	/* 色決定 */
	box->lamplight[1].m[3][0] = box->color.vx ;
	box->lamplight[1].m[3][1] = box->color.vy ;
	box->lamplight[1].m[3][2] = box->color.vz ;   
	box->brk_objs->objs[1].light = box->lamplight ;

	return 0 ;
}

/* 破壊時の動作 */
static int IRS_BreakBOX_Act( Work *work, BOX *box )
{
	BOX_LAMP   *lamp ;
	SVECTOR	rot ;
	SVECTOR	rot_wide ;
	FVECTOR	color ;
	float	 min_color ;
	float	 max_color ;
	float	 tmp ;

	lamp = box->lamp ;
	/* 点滅させる */
	if( box->count < 40 ){
		box->count ++ ;
		min_color = IRS_Box_Color.vx ;
		max_color = IRS_Box_Color.vy ;

		/* 色を設定 */
		if(box->interval == 0){
			box->color.vy += 220.0F * box->sign ;
			if(box->color.vy >= max_color ){
				box->color.vy = max_color ;
				if(box->sign == 1){
					box->interval = 1 ;
					box->sign = -1 ;
				}
			}

			if(box->color.vy <= min_color){
				box->color.vy = min_color ;
				if(box->sign == -1){
						box->interval = irnd() % 20 ;
					box->sign = 1 ;
				}
			}
			tmp = (IRS_Box_Color.vz - min_color) * (box->color.vy - min_color) / (max_color - min_color);
			box->color.vz = min_color + tmp ;
		}else{
			box->interval -- ;
		}
	}else{
		/* 最後に火花 */
		box->color = IRS_Box_Color ;
		box->act = (void *)IRS_BreakBOX_Act2 ;

		/* 火花のエフェクトを飛ばす */
		rot.vx = 0 ;
		rot.vy = box->rot.vy;
		rot.vz = 0 ;
		rot_wide.vx = 376;
		rot_wide.vy = 376;
		rot_wide.vz = 0;
		color.vx = 255.0F ;
		color.vy = 128.0F ;
		color.vz = 128.0F ;
		color.vw = 50.0F ;
		/* スクラッチパッド使用 */
		NewSpark1(24,&lamp->base_pos,36.0F,36.0F,0.0F,&rot,&rot_wide,&color,0.5F,20) ;
		DG_SetTmpLight2(
			&lamp->base_pos,
			400.0f,
			800.0f,
			80 | 40<<8 | 40 <<16,
			LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;
	}

	/* スイッチのエフェクト */
	/* スクラッチパット使用 */
	IRS_BOXLAMP_Act(box,lamp) ;
	DG_GetLightMatrix((FVECTOR*)box->brk_objs->world.m[3],box->light) ;

	box->lamplight[0] = box->light[0] ;
	box->lamplight[1] = box->light[1] ;

	/* 色決定 */
	box->lamplight[1].m[3][0] = box->color.vx ;
	box->lamplight[1].m[3][1] = box->color.vy ;
	box->lamplight[1].m[3][2] = box->color.vz ;

	box->brk_objs->objs[1].light = box->lamplight ;

	return 0 ;
}

/*  普通時の動作 */
static int IRS_NORMALBOX_Act( Work *work, BOX *box )
{
	BOX_LAMP  *lamp ;
	float	 min_color ;
	float	 max_color ;
	float	 tmp ;

	/* 色の初期 */
	min_color = IRS_Box_Color.vx ;
	max_color = IRS_Box_Color.vy ;

	/* 色を決める */
	if(box->interval == 0){
		box->color.vy += 5.0F * box->sign ;
		if(box->color.vy >= max_color ){
			box->color.vy = max_color ;
			if(box->sign == 1){
				box->interval = TOP_INTERVAL ;
				box->sign = -1 ;
			}
		}
		if(box->color.vy <= min_color){
			box->color.vy = min_color ;
			if(box->sign == -1){
				box->interval = INTERVAL ;
				box->sign = 1 ;
			}
		}
		tmp = (IRS_Box_Color.vz - min_color) * (box->color.vy - min_color) / (max_color - min_color);
		box->color.vz = min_color + tmp ;
	}else{
		box->interval -- ;
	}

	/* スイッチのエフェクト */
	/* この関数はスクラッチパットを使用 */
	lamp = box->lamp ;
	IRS_BOXLAMP_Act(box,lamp) ;
	
	/* 色決定 */
	DG_GetLightMatrix((FVECTOR*)box->objs->world.m[3],box->light) ;
	box->lamplight[0] = box->light[0] ;
	box->lamplight[1] = box->light[1] ;
	box->lamplight[1].m[3][0] = box->color.vx ;
	box->lamplight[1].m[3][1] = box->color.vy ;
	box->lamplight[1].m[3][2] = box->color.vz ;

	box->objs->objs[1].light = box->lamplight ;

	return 0 ;
}

/*********************************************************************/
/* スイッチ用のターゲット */
static void IRS_Box_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	BOX *box = (BOX *)ptr ;

	if( box->hit_flag == 0 ){
//		if( (def->damaged & TARGET_POWER)  &&  (def->weapon_type & (WP_BULLET|WP_M92)) ){
		if( (def->damaged & TARGET_POWER)  &&  (def->weapon_type & (WP_BULLET)) ){
			printf(" hit on irs_box \n") ;
			box->hit_flag = 1 ;
			box->act  = (void *)IRS_BreakBOX_Act ;

			/* 壊れ時のエフェクト発動 */
			/* スクラッチパット使用 */
			NewIRS_Box_Break_Effect(&box->pos,&box->rot) ;

			/* 通常モデルの登録を破棄してフリーする */
			DG_DequeueObjs(box->objs) ;
			DG_FreeObjs(box->objs) ;
			box->objs = NULL ;

			/* -------------------------------- */
			/* 壊れオブジェクトを表示 */
			/* 壊れモデル表示 */
			/* 大きさは変わらないので、前回のターゲットをそのまま残す */
			DG_VisibleObjs(box->brk_objs) ;
			DG_SetLightMatrix(box->brk_objs,box->light) ;
			DG_GetLightMatrix((FVECTOR*)box->brk_objs->world.m[3],box->light) ;

		}
	}
}
/*********************************************************************/
/* スイッチのランプ */
/*********************************************************************/
static  void  InitLampScrPad(BOX *box,DG_TEX *tex,FVECTOR *color)
{
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	int			   i ;

	pos = MEM_LAMP_POS ;
	uvrgbwh = MEM_LAMP_UVRGBWH ;
	
	DG_SetPos2(&box->pos,&box->rot) ;	/* 位置決め */
	for(i = 0; i < LAMP_N_VERTS; i++)
	{
	DG_PutVector(&IRS_BoxLamp_Pos,pos,1) ;
	pos ++ ;
	}

	/* ランプのボンボリの大きさを設定する */
	uvrgbwh[0].u0 = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh[0].v0 = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh[0].u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh[0].v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh[0].q0 = 4096 ;
	uvrgbwh[0].q1 = 4096 ;
	uvrgbwh[0].f0 = 0x0fff ;
	uvrgbwh[0].f1 = 0x0fff ;
	/* WH値は整数なので注意 ! */
	uvrgbwh[0].w = 90 ;
	uvrgbwh[0].h = 90 ;
	uvrgbwh[0].r = (u_short)color->vx ;
	uvrgbwh[0].g = (u_short)color->vy ;
	uvrgbwh[0].b = (u_short)color->vz ;
	uvrgbwh[0].a = (u_short)color->vw ;

	uvrgbwh[1].u0 = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh[1].v0 = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh[1].u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh[1].v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh[1].q0 = 4096 ;
	uvrgbwh[1].q1 = 4096 ;
	uvrgbwh[1].f0 = 0x0fff ;
	uvrgbwh[1].f1 = 0x0fff ;
	/* WH値は整数なので注意 ! */
	uvrgbwh[1].w = 90;
	uvrgbwh[1].h = 90;
	uvrgbwh[1].r = (u_short)color->vx ;
	uvrgbwh[1].g = (u_short)color->vy ;
	uvrgbwh[1].b = (u_short)color->vz ;
	uvrgbwh[1].a = (u_short)color->vw ;
	
}


/* スイッチのライト部のエフェクト */
static int IRS_InitBoxLamp(BOX *box)
{
	BOX_LAMP		*lamp ;
	DG_PRIM2		*prim ;
	DG_TEX		  *tex ;

	/* 領域確保 */
	lamp = box->lamp = GV_Malloc(sizeof(BOX_LAMP) ) ;
	if(lamp == NULL)
	{
	printf("irs_init.c : no memory \n") ;
	return -1 ;
	}
	/* ランプのプリミティブを用意する*/
	prim = lamp->prim = GM_MakePrim2(LAMP_PRIM_TYPE,LAMP_N_PRIMS,LAMP_N_VERTS) ;
	if(prim == NULL)
	{
	printf("irs_init.c(lamp) : no prim\n") ;
	return -1 ;
	}
	/* テクスチャを用意する */
	tex = lamp->tex = DG_GetTexture(4059425 /*"xlit01b_msk"*/) ;
	if(tex == NULL)
	{
	printf("irs_init.c(lamp) : no texture\n") ;	
	return -1 ;
	}
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0))  ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0) ; /* 加算半透明 */

	/* スクラッチパット上で初期化したデータを実際のワークにコピー */
	/* 色設定 */
	lamp->color.vx = IRS_Box_Color.vx ;
	lamp->color.vy = (IRS_Box_Color.vy + IRS_Box_Color.vx) *0.5f ;
	lamp->color.vz = (IRS_Box_Color.vz + IRS_Box_Color.vx) *0.5f ;
	lamp->color.vw = 96.0F ;
	InitLampScrPad(box,tex,&lamp->color) ;
	_BigMemCopy( prim->pos[0],MEM_LAMP_POS,sizeof(FVECTOR),LAMP_N_VERTS) ;
	_BigMemCopy( prim->uvrgb[0],MEM_LAMP_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),LAMP_N_VERTS) ;
	_BigMemCopy( prim->pos[ 1 ], MEM_LAMP_POS, sizeof(FVECTOR), LAMP_N_VERTS) ;
	_BigMemCopy( prim->uvrgb[ 1 ], MEM_LAMP_UVRGBWH, sizeof(DG_PRIM2_UVRGBWH), LAMP_N_VERTS ) ;
	_BigMemCopy( &lamp->base_pos,MEM_LAMP_POS,sizeof(FVECTOR),1) ;
	lamp->base_pos.vw = 90.0F ;

	return 0 ;
}

/*------------------------------------------------------------------------ */ 
						  /* スイッチ */
/* スイッチのターゲットを用意する */
static void InitTarget( Work *work, BOX *box)
{
	TARGET   *t ;
	DG_MDL   *mdl ;
	FVECTOR  t_size ;
	FVECTOR  pos ;

	t = &box->target ;
	mdl = box->mdl ;

	/* ターゲットのサイズを決定 */

	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly - 120.0F) * 0.5F ;

	/* ターゲットの位置 */
	pos.vx = 0.0F ;
	pos.vy = t_size.vy ;
	pos.vz = 0.0F ;

	/* ターゲット配置 */
	GM_SetTarget(t, TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER | TARGET_SEEK, work->map, ENEMY_SIDE, &t_size, &pos) ;
//	GM_SetTarget(t, TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER, work->map, ENEMY_SIDE, &t_size, &pos) ;
	GM_SetPowerTarget(t,&box->power,POWER_ONCE,1,0,0,&DG_ZeroVector) ;
	GM_SetTargetCallBack(t,IRS_Box_TargetCallBack, box) ;
	GM_PutTarget(t) ;
	GM_CurrentMap = work->map;	//act control で やるものの代用
	GM_MoveTarget2(t,&box->objs->world) ;

//	NewTargetView(t, 255, 255, 0) ;
}

static int IRS_InitBox(Work *work,FVECTOR *pos, SVECTOR *rot)
{
	BOX	   *box ;
	DG_DEF	*def ;
	DG_OBJS   *objs ;

	box = work->box = GV_Malloc(sizeof( BOX )) ;
	if(box == NULL)
	return -1 ;

	/* ---------------------------------------*/
	/* 通常モデルを取得 */
	/* モデル情報 */
	def = (DG_DEF *)GV_GetCache(GV_CacheID(6369667 /*"irs_box"*/,'k')) ;
	if(def == NULL){
		printf("not irs_box.kms\n") ;
		return -1 ;
	}
	/* モデルを取得 */
	objs = box->objs = DG_MakeObjs(def,BODY_FLAG, 0) ;
	if(objs == NULL) return -1 ;
	DG_QueueObjs((DG_OBJS*)box->objs) ;   /* モデル登録 */
	GM_GroupObjs( box->objs, work->map );
	box->mdl = def->models ;
	
	/* 位置、回転, 光源初期化 */
	box->pos = *pos ;
	box->rot = *rot ;
	box->light[0] = box->light[1] = DG_UnitMatrix ;	   /* 全体のライト */
	box->lamplight[0] = box->light[1] = DG_UnitMatrix ;   /* ランプのライト */
	box->color = IRS_Box_Color ;						  /* 基準カラー */
	box->interval = 0 ;
	box->sign = 1 ;
	box->hit_flag = 0 ;
	box->count	= 0 ;
	box->act   = (void *)IRS_NORMALBOX_Act ;
	
	DG_SetPos2(&box->pos,&box->rot) ;
	DG_PutObjs(objs) ;
	InitTarget( work, box ) ;									 /* ターゲット決定 */
	/* ライトをセット */
	DG_SetLightMatrix(box->objs,box->light) ;
	/* ---------------------------------------- */

	/* ---------------------------------------- */
	/* 破壊時のモデル */
	/* 壊れモデルを準備しておく */
	def = (DG_DEF *)GV_GetCache(GV_CacheID(2502814 /*"irs_box2"*/,'k')) ;
	if(def == NULL)
	{
	printf("not irs_box2.kms\n") ;
	return -1 ;
	}
	box->brk_objs = DG_MakeObjs(def,BODY_FLAG, 0) ;
	if(box->brk_objs == NULL)
	return -1 ;
	DG_QueueObjs((DG_OBJS*)box->brk_objs) ;
	GM_GroupObjs( box->brk_objs, work->map );
	DG_SetPos2(&box->pos, &box->rot) ;
	DG_PutObjs(box->brk_objs) ;
	DG_InvisibleObjs(box->brk_objs) ;
	/* --------------------------------------- */
   
	/* --------------------------------------- */
	/* スイッチのランプ部の初期化 */
	IRS_InitBoxLamp(box) ;
	return 0 ;	
}

static int IRS_InitSensor(Work *work, FVECTOR *pos, SVECTOR *rot )
{
	SENSOR  *sensor ;
	DG_DEF  *def ;
	int	 i ;
	FVECTOR  *sen_pos ;
	SVECTOR  *sen_rot ;

	sensor = work->sensor = GV_Malloc(sizeof( SENSOR )) ;
	if( sensor == NULL )
	return - 1 ;

	/* モデル情報 */
	def = (DG_DEF *) GV_GetCache(GV_CacheID(111283 /*"irs"*/,'k')) ;
	if(def == NULL)
	{
	printf("not irs_sensor.kms\n") ;
	return - 1 ;
	}

	/* モデルを呼び出す */	
	for(i = 0 ; i < N_IRS_SENSOR; i++){
		sensor->objs[i] = DG_MakeObjs(def,BODY_FLAG,0) ;
		if(sensor->objs[i] == NULL){
			printf("don't make sensor obj \n") ;
			return - 1 ;
		}
		DG_QueueObjs((DG_OBJS*)sensor->objs[i]) ;
		GM_GroupObjs( sensor->objs[i], work->map );
	}

	sen_pos = sensor->pos ;
	sen_rot = sensor->rot ;
	/* ターゲットを設定 */
	/* 位置を設定 */
	for(i = 0; i < N_IRS_SENSOR; i++)
	{
	*sen_pos = *pos ;
	*sen_rot = *rot ;
	DG_SetPos2(sen_pos,sen_rot) ;
	DG_PutObjs(sensor->objs[i]) ;
	pos ++ ;
	rot ++ ;
	sen_pos ++ ;
	sen_rot ++ ;
	}

	/* ライトをセット */
	for(i = 0; i < N_IRS_SENSOR; i++){
		DG_SetLightMatrix(sensor->objs[i],sensor->light[i]) ;
	}

	return 0 ;
}

/*------------------------------------------------------------*/
				   /* 赤外線 */

/* センサー初期化 */
static void Init_RayUvrgbScrPad(SENSOR_RAY *ray,DG_TEX *tex)
{
	int			i ;
	float		  *p_patern ;
	float		  *p_speed  ;
	DG_PRIM2_UVRGB *uvrgb ;

	p_patern = ray->patern ;
	p_speed  = ray->speed ;
	uvrgb	= MEM_RAY_UVRGB ;
	for(i = 0; i < N_IRS_SENSOR_RAY; i++)
	{
	*p_patern = rnd() * 1.0F ;
	*p_speed  = rnd() * 3.0F + 1.0F ;
	uvrgb[0].u = FTOI12( tex->u_offset ) ;
	uvrgb[0].v = FTOI12( tex->v_offset ) ;
	uvrgb[0].q = 4096 ;
	uvrgb[0].f = 0x8fff ;
#if 1
	uvrgb[0].r = 0 ;
	uvrgb[0].g = 0 ;
	uvrgb[0].b = 0 ;
#else
	uvrgb[0].r = 48 ;
	uvrgb[0].g = 48 ;
	uvrgb[0].b = 48 ;
#endif
	uvrgb[0].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
	uvrgb[1].u = FTOI12( tex->u_scale ) ;
	uvrgb[1].v = FTOI12( tex->v_scale + tex->v_offset ) ;
	uvrgb[1].q = 4096 ;
	uvrgb[1].f = 0x0fff ;
#if 1
	uvrgb[1].r = 0 ;
	uvrgb[1].g = 0 ;
	uvrgb[1].b = 0 ;
#else
	uvrgb[1].r = 48 ;
	uvrgb[1].g = 48 ;
	uvrgb[1].b = 48 ;
#endif
	
	uvrgb[1].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
	uvrgb += N_RAY_VERTS ;
	p_patern ++ ;
	p_speed ++ ;
	}
	
	
}

/* SCRPAD は 0x1000 まで使用中 */
static void Init_RayPosScrPad( Work *work, FVECTOR *base_pos, SVECTOR *rot)
{
	FVECTOR			 *pos ;
	FVECTOR			 *local_pos1,*local_pos2 ;
	int				 tmp[3] ;
	int				 r ;
	int				 i,j ;
	
	/* 位置決め */
	/*-------------------------------------------------*/
	pos = MEM_RAY_POS ;
	local_pos1 = (void *)(SCRPAD_ADDR + 0x2000) ;
	local_pos2 = (void *)(SCRPAD_ADDR + 0x2200) ;

	/* センサーホールの位置を決める*/
	DG_SetPos2(base_pos,rot) ;
	DG_PutVector(Irs_Sensor_Hole,local_pos1,N_IRS_SENSOR_RAY) ;
	base_pos ++ ;
	rot ++ ;
	
	DG_SetPos2(base_pos,rot) ;
	DG_PutVector(Irs_Sensor_Hole,local_pos2,N_IRS_SENSOR_RAY) ;

	if(work->mode){
		DG_COPY_VEC( &local_pos1[8], &local_pos2[8] );
		DG_COPY_VEC( &local_pos1[9], &local_pos2[9] );
	}

	/* 1列目はそろえる */
	*pos = *local_pos1 ;   pos ++ ; local_pos1 ++ ;
	*pos = *local_pos2 ;   pos ++ ; local_pos2 ++ ;

	/* 2列目～4列目まではランダム */
	for(i = 0; i < 3; i ++)
	tmp[i] = i ;
	
	for(i = 3 ; i > 0; i--)
	{
	*pos = *local_pos1 ;  pos ++; local_pos1 ++ ;
	r = irnd() % i ;
	*pos = local_pos2[tmp[r]] ;  pos ++ ;
	for(j = r; j < i; j++)
		tmp[j] = tmp[j + 1] ;
	}
	local_pos2 += 3 ;
	
	/* 5,6列目は同じ */
	*pos = *local_pos1 ;   pos ++ ; local_pos1 ++ ;
	*pos = *local_pos2 ;   pos ++ ; local_pos2 ++ ;
	*pos = *local_pos1 ;   pos ++ ; local_pos1 ++ ;
	*pos = *local_pos2 ;   pos ++ ; local_pos2 ++ ;
	
	/* 7列目から9列目はランダム */
	for(i = 0 ; i < 3 ; i++)
	{
	tmp[i] = i ;
	}
	for(i = 3 ; i > 0 ; i --)
	{
	*pos = *local_pos1 ;  pos ++ ; local_pos1 ++ ;	
	r = irnd() % i ;
	*pos = local_pos2[tmp[r]] ; pos ++ ;
	for(j = r; j < i; j++)
	{
		tmp[j] = tmp[j + 1] ;
	}
	}
	local_pos2 += 3 ; 

	/* 10列目は同じ */
	*pos = *local_pos1 ;   pos ++ ; local_pos1 ++ ;
	*pos = *local_pos2 ;   pos ++ ; local_pos2 ++ ;
}

/* 赤外線の当たりの範囲を決定 */
static  void IRS_InitSensorRay_Bound(FVECTOR *base, SENSOR_RAY  *ray)
{
	float	 min,max ;
	int	   i ;
	FVECTOR   *pos ;
	FVECTOR   *bound ;

	bound = (void *)(SCRPAD_ADDR + 0x2000) ;
	pos = MEM_RAY_POS ;
	min = max = pos->vx ;
	pos ++ ;
	for(i = 1; i < 4; i++){
		if( min > pos->vx) min = pos->vx ;
		if( max < pos->vx) max = pos->vx ;
		pos ++ ;
	}

	pos = MEM_RAY_POS ;
	bound[0].vx = min ;
	bound[1].vx = max ;
	bound[0].vy = base->vy ;
	bound[1].vy = pos->vy ;

	min = max = pos->vz ;
	pos ++ ;
	for(i = 1; i < 4; i++){
		if(min > pos->vz) min = pos->vz ;
		if(max < pos->vz) max = pos->vz ;
		pos ++ ;
	}

	bound[0].vz = min ;
	bound[1].vz = max ;

	ray->center.vx = (bound[0].vx + bound[1].vx) *0.5f ;
	ray->center.vy = (bound[0].vy + bound[1].vy) *0.5f ;
	ray->center.vz = (bound[0].vz + bound[1].vz) *0.5f ;
	ray->size.vx = (bound[1].vx - bound[0].vx) *0.5f;
	ray->size.vy = (bound[1].vy - bound[0].vy) *0.5f;
	ray->size.vz = (bound[1].vz - bound[0].vz) *0.5f;
}

static int IRS_InitSensorRay(Work *work, FVECTOR *pos, SVECTOR *rot )
{
	SENSOR_RAY  *ray ;
	DG_PRIM2	*prim ;
	DG_TEX	  *tex ;

	/* 赤外線の領域を確保 */
	ray = work->ray = GV_Malloc(sizeof( SENSOR_RAY )) ;
	if( ray == NULL ){
		printf("don't memory get for ray_line\n") ;
		return 0 ;
	}
	
	/* プリミティブをセット */
	prim = ray->prim = GM_MakePrim2(PRIM_TYPE,N_IRS_SENSOR_RAY,N_RAY_VERTS) ;
	if(prim == NULL){
		printf("don't make prim \n") ;
		return -1 ;
	}

	/* テクスチャの取得 */
	tex = ray->tex = DG_GetTexture(6792560 /*"lsight_msk"*/) ;
	if(tex == NULL){
		printf("not texture irs_sensor.c\n") ;
		return -1 ;
	}

	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 )) ;

	/* uvrgb 決め */
	Init_RayUvrgbScrPad(ray,tex) ;
	/* 位置決め */
	Init_RayPosScrPad( work, pos,rot) ;
	/* ターゲット用のバウンディングボックスを求める */
	IRS_InitSensorRay_Bound(pos,ray) ;

	_BigMemCopy(prim->pos[0],MEM_RAY_POS,sizeof(FVECTOR),N_IRS_SENSOR_RAY * N_RAY_VERTS ) ;
	_BigMemCopy(prim->uvrgb[0],MEM_RAY_UVRGB,sizeof(DG_PRIM2_UVRGB),N_IRS_SENSOR_RAY * N_RAY_VERTS ) ;
	_BigMemCopy(prim->pos[1],MEM_RAY_POS,sizeof(FVECTOR),N_IRS_SENSOR_RAY * N_RAY_VERTS ) ;
	_BigMemCopy(prim->uvrgb[1],MEM_RAY_UVRGB,sizeof(DG_PRIM2_UVRGB),N_IRS_SENSOR_RAY * N_RAY_VERTS ) ;

	/* カウントセット */
	ray->count = 0 ;
	ray->a_flag  = 0 ;
	ray->a_flag2 = 0 ;
	ray->bright = 0.0F ;
	/* デバッグ用 */
#if  0
	ray->target.center = ray->center ;
	ray->target.size   = ray->size ;
	ray->target.class  = 0 ;
	NewTargetView(&ray->target, 0, 255, 255) ;
#endif
	return 0 ;
}

/***********************************************************************/
static int IRS_GetOption(FVECTOR *pos, SVECTOR *rot, Work *work)
{
	int   buf[3], i ;

	/* 下部空ける？ */
	if(GCL_GetOption('a') != NULL){
		work->mode = GCL_GetNextInt() ;
	}else{
		work->mode = 0 ;
	}

	/* ポジションを決定 */
	if(GCL_GetOption('p') != NULL){
		for(i = 0;  i < 3 ; i ++){
			GCL_GetIV(GCL_NextStr(),buf) ;
			GV_IVtoFV(buf,(float *)pos,3) ;
			pos ++ ;
		}
	}else{
		printf("irs_init.c : not position\n") ;
		return -1 ;
	}


	/* 回転を決定 */
	if( GCL_GetOption('r') != NULL){
		for(i = 0 ; i < 3 ; i++){
			GCL_GetIV(GCL_NextStr(),buf) ;
			rot->vx = buf[0] ;
			rot->vy = buf[1] ;
			rot->vz = buf[2] ;
			rot ++ ;
		}
	}else{
		printf("irs_init.c : not rotation\n") ;
		return -1 ;
	}

	/* プロックIDを取得 */
	if( (GCL_GetOption('e')) != NULL){
		work->command = GCL_GetNextInt() ;
	}else{
		work->command = 0x80000000 ;
	}

	return 0 ;
}


/* ----------------------------------------------------------- */

/* ----------------------------------------------------------- */
				  /* 赤外線 */

/* 赤外線に触れているかの判別 */
static int Check_Touch_Laser( Work *work, FVECTOR *center, FVECTOR *size)
{
//	FVECTOR *p_pos ; 
	FVECTOR bound_min, bound_max ;
	CONTROL	**list ;
	int		i, n ;


	_sceVu0SubVector(&bound_min, center, size) ;
	_sceVu0AddVector(&bound_max, center, size) ;

	if( work->mode ){
		bound_min.vy += 500.0f;
	}

	list = GM_WhereList ;
	n = GM_N_WhereList ;
	for ( i = n ; i > 0 ; -- i, list++ ) {
		if( (*list)->map != work->map ){
			continue;
		}

		if( (*list)!=NULL ){
#if 0
			object = (OBJECT *)((*list) + 1);
			if( object==NULL ){
				if( vu0_CheckBoundingBox( &(*list)->mov, &bound_min, &bound_max ) ){
					return 0;
				}
			}else{
				num = object->objs->n_models-1;
				obj = object->objs->objs;
				for( j=0; j<num; j++, obj++){
					if( vu0_CheckBoundingBox( (FVECTOR *)obj->world.m[3], &bound_min, &bound_max ) ){
						return 0;
					}
				}
			}
#else
			if( vu0_CheckBoundingBox( &(*list)->mov, &bound_min, &bound_max ) ){
				return 0;
			}
#endif
		}
	}

	return 1;

}

/* 赤外線テクスチャデータ更新 */
static void IRS_Move_Ray(Work *work,SENSOR_RAY *ray)
{
	int			i ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	float		*p_patern,*p_speed ;
	DG_PRIM2_UVRGB	*uvrgb ;
	GV_MSG		*msg ;
	int			mes_num ;
	int			num ;
	int			visible_flag ;
	int			item_id ;
	FVECTOR		fvtemp;
	float		length;
	float		max_alpha0;
	float		max_alpha1;


	prim = ray->prim ;
	tex  = ray->tex ;

	p_patern = ray->patern ;
	p_speed  = ray->speed ;
	uvrgb	= prim->uvrgb[prim->buffer_clock] ;

	_sceVu0SubVector( &fvtemp, &work->sensor->pos[0], &GM_PlayerPosition ) ;
	length = GV_VecLen3F( &fvtemp );
	visible_flag = 0;
	if(length < SENSE_LENGTH){
		/* たばこを持っているかどうかをチェック*/
		item_id = PL_GetPlayerItem() ;
		if(item_id == IT_Tabacco){
			visible_flag = 1 ;
		}
	}

	/* メッセージをチェックし カラーを決める  */
	mes_num = GV_ReceiveMessage( work->name, &msg ) ;
	msg += mes_num - 1;
	while( --mes_num >= 0 ){
		num = msg->message[0] ;
		if(num == IRS_VISIBLE_FLAG){
			ray->a_flag  = 1;
			ray->a_flag2 = 1;
		}
	}


	/* 外部プログラムからＯＮ指令が来たか？ */
	if( OK_IRS_OnFlag != 0 ){
		OK_IRS_OnFlag = 0;
		ray->a_flag  = 1;
		ray->a_flag2 = 1;
	}


	/* フラグが立ったとき */
	if( ray->a_flag ){
		ray->count++;
		if(ray->count > MAX_COUNT){
			ray->count = 0;
			ray->a_flag = 0 ;
		}
	}


	max_alpha0 = 0.0f;
	max_alpha1 = 0.0f;
	if( ray->count > 0 ){
		if( ray->count < INIT_COUNT ){
			max_alpha0 = MAX_BRIGHT * (float)ray->count / (float)INIT_COUNT;
		}else if( ray->count > FINISH_COUNT ){
			max_alpha0 = MAX_BRIGHT * (float)(MAX_COUNT - ray->count) / (float)(MAX_COUNT - FINISH_COUNT);
		}else{
			max_alpha0 = MAX_BRIGHT;
		}
	}

	if( visible_flag ){
		work->tabac_count++ ;
		if( work->tabac_count > MAX_COUNT ) work->tabac_count = MAX_COUNT;
	}else{
		work->tabac_count-- ;
		if( work->tabac_count < 0 ) work->tabac_count = 0;
	}
	if( work->tabac_count < INIT_COUNT ){
		max_alpha1 = MAX_BRIGHT * (float)work->tabac_count / (float)INIT_COUNT;
	}else if( work->tabac_count > FINISH_COUNT ){
		max_alpha1 = MAX_BRIGHT * (float)(FINISH_COUNT - work->tabac_count) / (float)FINISH_COUNT;
	}else{
		max_alpha1 = MAX_BRIGHT;
	}
	max_alpha1 *= (SENSE_LENGTH-length) / SENSE_LENGTH;

	ray->bright = (max_alpha0 > max_alpha1)? max_alpha0: max_alpha1;

//	printf("ray->count %d\n",ray->count) ;
//	printf("ray->bright %f\n",ray->bright) ;
	/* 赤外線のパターンをチェック */
	for(i = 0 ; i < N_IRS_SENSOR_RAY ; i++){
		*p_patern = *p_patern + 1.0F / 256.0F * (*p_speed) ;
		if(*p_patern >= 1.0F){
			*p_patern = 0.0F ;
			*p_speed = rnd() * 2.0F + 1.0F;
		}
		uvrgb[0].u = FTOI12(tex->u_offset) ;
		uvrgb[0].v = FTOI12(*p_patern * tex->v_scale + tex->v_offset) ;
		uvrgb[1].u = FTOI12(tex->u_scale + tex->u_offset) ;
		uvrgb[1].v = FTOI12(*p_patern * tex->v_scale + tex->v_offset) ;
		uvrgb[0].r = uvrgb[0].g = uvrgb[0].b = (u_short)ray->bright ;
		uvrgb[1].r = uvrgb[1].g = uvrgb[1].b = (u_short)ray->bright ;
		uvrgb += N_RAY_VERTS ;
		p_patern ++ ;
		p_speed ++ ;
	}
}

static int IRS_SENSORRAY_Act( Work *work, SENSOR_RAY *ray)
{
	int  flag ;	
	FVECTOR	fvtemp;

	/* 判定チェック */

	if( (GM_Item == IT_CBBox)
	 || (GM_Item == IT_CBBoxB)
	 || (GM_Item == IT_CBBoxC)
	 || (GM_Item == IT_CBBoxWet)
	  ){
		fvtemp.vx = 500.0f;
		fvtemp.vy = 500.0f;
		fvtemp.vz = 500.0f;
		_sceVu0AddVector( &fvtemp, &ray->size, &fvtemp );
	}else{
		DG_COPY_VEC( &fvtemp, &ray->size );
	}

	flag = 0;
	if( work->exec_once == 0 ){
		flag = Check_Touch_Laser( work, &ray->center, &fvtemp ) ;
		if(flag){
			if(!(work->command & 0x80000000)){
				GCL_ExecProc(work->command,NULL) ;
				work->exec_once = 1;
			}
		}
	}
	/* 赤外線の移動 */
	/* 切り替え */
	DG_SwitchBuffPrim2( ray->prim ) ;
	IRS_Move_Ray(work,ray) ;

	return flag ;
}

/*
   SCRPADの使用状況 
   火花や壊れ時のエフェクトにも SCRPAD 使用注意	


   ● irs_act.c
		 static  void  Scr_BoxLamp  で
		 SCRPAD の 0x3000 を使用

	
*/  



/* スイッチ用のターゲット */
static void IrsCrush( Work *work )
{
	if( work->box->hit_flag != 1 ) return;
	work->box->hit_flag = 2 ;
	GM_ClearTargetDamage( &work->box->target ) ;
	GM_FreeTarget( &work->box->target ) ;
	if( work->switch_break_proc > 0 ){
		GCL_ExecProc( work->switch_break_proc, NULL );
		work->switch_break_proc = -1;
	}
}


/* -------------------------------------------------------------------------- */
static  void Act( Work *work )
{
	BOX   *box ;
	SENSOR	 *sensor ;
	SENSOR_RAY *ray ;

	IrsCrush( work );

	/* キャラがマップにないときは 何も処理をしない */
	if(!(work->map & GM_CurrentStageMap)) return ;
	  
	/* センサーの動作 */
	sensor = work->sensor ;
	DG_GetLightMatrix((FVECTOR*)sensor->objs[0]->world.m[3],sensor->light[0]) ;
	DG_GetLightMatrix((FVECTOR*)sensor->objs[1]->world.m[3],sensor->light[1]) ;

	/* 箱の動作 */
	box = work->box ;
	(*box->act)(work,box) ;

	/* 赤外線の動作 */
	ray = work->ray ;
	if(box->hit_flag == 0) {
		/* 赤外線動作 */
		IRS_SENSORRAY_Act(work,ray) ;
	}else{
		/* 領域開放 */
		if(ray != NULL){
			ray->prim = OK_FreePrim2( ray->prim ) ;
			GV_DelayedFree(ray) ;
			work->ray = NULL ;
		}
	}
}

static  void Die( Work *work )
{
	BOX	*box ;
	BOX_LAMP *lamp ;
	SENSOR *sensor ;
	SENSOR_RAY *ray ;
	int	i ;

	box = work->box ;
	/* ボックスの領域開放 */
	if(box != NULL){
		/* 通常モデル開放 */
		if(box->objs != NULL){
			DG_DequeueObjs( box->objs ) ;
			DG_FreeObjs( box->objs ) ;
		}
		/* 壊れモデル開放*/
		if(box->brk_objs != NULL){
			DG_DequeueObjs( box->brk_objs ) ;
			DG_FreeObjs( box->brk_objs ) ;
		}
		/* ランプ部分開放 */
		lamp = box->lamp ;
		if(lamp != NULL){
			lamp->prim = OK_FreePrim2( lamp->prim ) ;
			GV_DelayedFree(lamp) ;
		}
		GV_DelayedFree(box) ;
	}

	/* センサー開放 */
	sensor = work->sensor ;
	if(sensor != NULL){
		for(i = 0 ; i < N_IRS_SENSOR; i ++){
			if(sensor->objs[i] != NULL){
				DG_DequeueObjs( sensor->objs[i] ) ;
				DG_FreeObjs( sensor->objs[i] ) ;
			}
		}
		GV_DelayedFree(sensor) ;
	}

	/* 赤外線開放 */
	ray = work->ray ;
	if(ray != NULL){
		ray->prim = OK_FreePrim2( ray->prim ) ;
		GV_DelayedFree(ray) ;
	}
}


static int GetResources(Work *work)
{
	FVECTOR pos[3];
	SVECTOR rot[3];

	OK_IRS_OnFlag = 0;

	work->exec_once = 0;

	work->mes_flag = 0;
	work->another_flag = 0 ;
	work->tabac_count = 0 ;

	/* オプション取得 */
	if( IRS_GetOption(pos, rot, work)){
		printf("irs.c : IRS_GetOption()\n") ;
		return -1 ;
	}

	/* ボックス部分 */
	if( IRS_InitBox(work,&pos[0],&rot[0]) ){
		printf("irs.c : IRS_InitWork()\n") ;
		return -1 ;
	}

	/* センサー部分 */
	if( IRS_InitSensor(work,&pos[1], &rot[1]) ){
		printf("irs.c : IRS_InitSensor() \n") ;
		return -1 ;
	}

	/* 赤外線部分 */
	if( IRS_InitSensorRay(work,&pos[1], &rot[1]) ){
		printf("irs.c : IRS_InitSensorRay() \n") ;
		return -1 ;
	}

	work->switch_break_proc = -1;
	if ( GCL_GetOption( 's' ) != NULL ){
		work->switch_break_proc = GCL_GetNextInt();
	}

	return 0 ;
}

void  *NewIRS(int name, int map)
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->map = map ;
		work->name = name ;
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
