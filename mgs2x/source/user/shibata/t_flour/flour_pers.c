//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
 /*
	   flour_pers.c 
	   小麦粉の透視変換バージョン 

	   2000/02/15  H.Tanaka
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

#include	"def_dma.h"


#include	"utl_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define N_PRIMS		 1
#define N_VERTS		 32
#define N_NUM		   (N_PRIMS * N_VERTS)

/*------------------------------------------------------------------*/
#define	PRIM_TYPE	( DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA | DG_PRIM2_ON_CAMERA)

#ifdef BP_PS2
#define	MEM_POS	  ((void *)SCRPAD_ADDR)
#define	MEM_UVRGBWH  ((void *)SCRPAD_ADDR + sizeof(FVECTOR) * N_NUM)
#define	MEM_BASE	 ((void *)MEM_UVRGBWH + sizeof(DG_PRIM2_UVRGBWH) * N_NUM)
#else
#define	MEM_POS	  ((void *)SCRPAD_ADDR)
#define	MEM_UVRGBWH  ((void *)( (char *)SCRPAD_ADDR + sizeof(FVECTOR) * N_NUM ))
#define	MEM_BASE	 ((void *)( (char *)MEM_UVRGBWH + sizeof(DG_PRIM2_UVRGBWH) * N_NUM ))
#endif

#define	MAX_ALPHA	64
#define	MAX_COUNT	1100
#define		 SCREEN_NEAR	 ( 51.0f )
#define	MAX_SIZE   60

enum {
	NORMAL = 0,
	FLOUR_VISIBLE_FLAG
} ;

typedef struct
{
	GV_ACT_EX	actor ;
	
	DG_PRIM2  *prim ;
	DG_TEX	*tex ;	
	FVECTOR   base[N_NUM] ;
	FVECTOR   base_range ;
	FVECTOR   center ;
	float	 alpha ;
	int	   a_flag ;
	int	   count ;
	int	   interval_count ;

	int	   name ;
	int	   map ;
} Work ;


extern  void _BigScrCopy( void *dst, void *src, int size, int num );
extern  void _BigMemCopy( void *dst, void *src, int size, int num );

/* -------------------------------------------------------------- */
/* メッセージチェック */
static  int  Check_Message(Work *work)
{
	GV_MSG	*msg ;
	int	   mes_num ;
	int	   num ;

	mes_num = GV_ReceiveMessage( work->name, &msg ) ;
	msg	+= mes_num -1 ;

	while( --mes_num >= 0 ){
		num = msg->message[0] ;
		if(num == FLOUR_VISIBLE_FLAG ){
			return 1 ;
		}
	}
	return 0 ;
}

/* 実行中のスクラッチパッド */
static  void  Act_ScrPad(Work *work)
{
	FVECTOR   *pos, *base ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	int	   i,j ;
	int	   flag ;
	int	   step ;
	float	screen_near_x, screen_near_y ;
	FVECTOR   *center, *range ;
	FVECTOR   double_range ;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	/* メッセージチェック */
	flag = Check_Message(work) ;

	/* アルファ値チェック */
	if(flag){
		work->a_flag = 1 ;
		work->alpha += 0.5F ;

		/* 初期化を行う */
		if(work->count <= MAX_COUNT / 2){
			center = &work->center ;
			range = &work->base_range ;
			double_range.vx = range->vx * 2.0F;
			double_range.vy = range->vy * 2.0F;
			double_range.vz = range->vz * 2.0F;
			double_range.vw = 1.0F ;
			base = MEM_BASE ;
			for(i = 0; i < N_PRIMS; i++){
				for(j = 0; j < N_VERTS ; j++){
					base->vx = center->vx - range->vx + rnd() * double_range.vx ;
					base->vy = center->vy + rnd() * double_range.vy ;
					base->vz = center->vz - range->vz + rnd() * double_range.vz ;
					base->vw = 1.0F ;
					base ++ ;
				}
			}
		}

		if(work->alpha >= MAX_ALPHA){
			work->alpha = MAX_ALPHA ;
			work->a_flag = 0 ;
			work->count = MAX_COUNT ;
		}
	}else{
		if(work->a_flag){
			work->alpha += 0.5F ;
			if(work->alpha >= MAX_ALPHA){
				work->alpha = MAX_ALPHA ;
				work->a_flag = 0 ;
				work->count = MAX_COUNT ;
			}
		}else{
			/* カウントが半分を過ぎたら、減色 */
			if(work->count <= (MAX_COUNT / 3)){
				step = (MAX_COUNT / 3) / (int)MAX_ALPHA ;
				if((work->count % step) == 0){
					work->alpha -= 1.0F ;
					if(work->alpha <= 0.0F) work->alpha = 0.0F ;
				}
			}
		}
		work->count -- ;
		if(work->count <= 0) work->count = 0 ;
	}

	/* baseを左右に動かす */
	base = MEM_BASE ;
	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++){
			base->vx += (-7.5F + rnd() * 15.0F) ;
			base->vz += (-7.5F + rnd() * 15.0F) ;
			base ++ ;
		}
	}

	/* baseを元に透視変換を行う */
	base = MEM_BASE ;
	pos  = MEM_POS ;

	/* 透視変換 */
	DG_SetPos( &DG_Chanls->eye_pers ) ;
	DG_PutVector(base, pos, N_NUM) ;
	screen_near_x = SCREEN_NEAR / (ASPECT_X()) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	/* 色を決定 */
	/* uvrgbwh 更新 */
//	base = MEM_BASE ;
	pos  = MEM_POS ;
	uvrgbwh = MEM_UVRGBWH ;

	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++,pos ++,uvrgbwh ++){
			uvrgbwh->h = 15 ;
			uvrgbwh->w = 15 ;

			if(pos->vz > (pos->vw) * 1.4F){
				uvrgbwh->a = 0 ;
				continue ;
			}
			if(pos->vw < 0.0F) pos->vw = - pos->vw ;
			if((pos->vx > pos->vw * 1.4F) || (pos->vx < (- pos->vw * 1.4F))){
				uvrgbwh->a = 0 ;
				continue ;
			}

			if((pos->vy > pos->vw * 1.4F) || (pos->vy < (- pos->vw * 1.4F))){
				uvrgbwh->a = 0; 
				continue ;
				}
			/* 位置を決定 */
			pos->vz = SCREEN_NEAR ;
			pos->vx = screen_near_x * pos->vx / pos->vw ;
			pos->vy = screen_near_y * pos->vy / pos->vw ;
			/* アルファ値を決定 */
			uvrgbwh->a = (u_short)work->alpha ;
		}
	}
}

static  void  Act(Work *work)
{
	DG_PRIM2   *prim ;

	prim = work->prim ;

	return ;
	/* データを転送 */
	_BigScrCopy(MEM_POS,	prim->pos[prim->buffer_clock], sizeof(FVECTOR), N_NUM) ;
	_BigScrCopy(MEM_UVRGBWH,prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH), N_NUM) ;
	_BigScrCopy(MEM_BASE, work->base, sizeof(FVECTOR), N_NUM) ;
	/* スイッチ切り替え */
	DG_SwitchBuffPrim2(prim) ;

	/* 新データ決定 */
	Act_ScrPad(work) ;

	_BigMemCopy(prim->pos[prim->buffer_clock], MEM_POS,sizeof(FVECTOR), N_NUM) ;
	_BigMemCopy(prim->uvrgb[prim->buffer_clock], MEM_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH), N_NUM) ;
	_BigMemCopy(work->base, MEM_BASE, sizeof(FVECTOR), N_NUM) ;
}

static  void  Die(Work *work)
{
	if(work->prim != NULL)
	GM_FreePrim2( work->prim ) ;
}

/* --------------------------------------------------------------- */
static  void  Init_ScrPad( Work *work, DG_TEX *tex )
{
	FVECTOR			 *pos,*base ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int				 i,j ; 
	FVECTOR			 *range ;
	FVECTOR			 double_range ;

	pos = MEM_POS ;
	uvrgbwh = MEM_UVRGBWH ;
	base = MEM_BASE ;

	range = &work->base_range ;
	double_range.vx = range->vx * 2.0F;
	double_range.vy = range->vy * 2.0F;
	double_range.vz = range->vz * 2.0F;
	double_range.vw = 1.0F ;

	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++){
			base->vx = work->center.vx - range->vx + rnd() * double_range.vx ;
			base->vy = work->center.vy + rnd() * double_range.vy ;
			base->vz = work->center.vz - range->vz + rnd() * double_range.vz ;
			base->vw = 1.0F ;
			/* 初期化 */
			*pos = DG_ZeroVector ;
			uvrgbwh->u0 = FTOI12( tex->u_offset ) ;
			uvrgbwh->v0 = FTOI12( tex->v_offset ) ;
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			uvrgbwh->w  = uvrgbwh->h = 0 ;
			uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = 36 ;
			uvrgbwh->a  = 0 ;

			uvrgbwh ++ ;
			pos	 ++ ;
			base	++ ;
		}
	}
}


static  void GetOptionValue( Work *work)
{
	int   buf[3] ;
	
	if(GCL_GetOption('p') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf,(float *)&(work->center),3) ;
	}else{
		work->center.vx = 0.0F ;
		work->center.vy = 0.0F ;
		work->center.vz = 0.0F ;
	}

	if(GCL_GetOption('r') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf, (float *)&(work->base_range),3) ;
	}else{
		work->base_range.vx = 0.0F ;
		work->base_range.vy = 0.0F ;
		work->base_range.vz = 0.0F ;
	}
}

static  int  GetResources( Work *work)
{
	DG_PRIM2		  *prim ;
	DG_TEX			*tex ;

	GetOptionValue(work) ;
	prim = work->prim = GM_MakePrim2( PRIM_TYPE, N_PRIMS, N_VERTS ) ;
	if(prim == NULL){
		printf("null prim \n") ;
		return -1 ;
	}

//	tex = work->tex = DG_GetTexture( GV_StrCode( "powfog03_add" ) );
	tex = work->tex = DG_GetTexture(11197650);
	if(tex == NULL){
		printf("null texture\n") ;
		return -1 ;
	}
	DG_ConfigPrim2Tex( prim, tex ) ;

	/* スクラッチパットで初期情報を初期化 */
	Init_ScrPad(work, tex) ;

	/* スクラッチパットから メモリーに転送する */
	_BigMemCopy(prim->pos[0], MEM_POS, sizeof(FVECTOR), N_NUM) ;
	_BigMemCopy(prim->pos[1], MEM_POS, sizeof(FVECTOR), N_NUM) ;
	_BigMemCopy(prim->uvrgb[0], MEM_UVRGBWH, sizeof(DG_PRIM2_UVRGBWH), N_NUM) ;
	_BigMemCopy(prim->uvrgb[1], MEM_UVRGBWH, sizeof(DG_PRIM2_UVRGBWH), N_NUM) ;
	_BigMemCopy(work->base, MEM_BASE, sizeof(FVECTOR), N_NUM) ;

	work->a_flag = 0 ;
	return 0 ;
}

void   *NewFlour_Pers(int name, int map)
{
	Work   *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->name = name ;
		work->map  = map ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
