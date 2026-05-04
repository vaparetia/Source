//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	 irs_break.c	破壊時のエフェクト 

	1999/12/16  H.TANAKA
	2000/10/18 S.Okajima
	$Id: irs_break.c,v 1.1.1.3 2002/11/19 11:47:45 Yoshizawa1 Exp $
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

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	MAX_COUNT			  90
#define	FIRE_MAX_COUNT		 30
#define	SMOKE_START_COUNT	  40
#define	MAX_PATERN			 30

#define	N_PRIMS		   1
#define	N_VERTS		   10
#define	PRIM_TYPE		 (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

/*   pos total size = 0x0200	uvrgbwh total size = 0x0400 */
#define   MEM_SMOKE_POS	  ((void *)(SCRPAD_ADDR + 0x0000))
#define   MEM_SMOKE_UVRGBWH  ((void *)(SCRPAD_ADDR + 0x0500))


extern void  *NewSpark1(int n_packets, FVECTOR *center, float min_speed, 
					   float speed_wide, float gravity, SVECTOR *rot, 
			   SVECTOR *rot_wide, FVECTOR *color, float length, int count );

//extern void  Big_TmpLight2(FVECTOR *pos,float r_range,float e_range,int color,int flag) ;

FVECTOR	IRS_BOX_FIRE_AREA[] = {
	{-110.0F,20.0F,75.0F,0.0F},
	{110.0F,180.0F,115.0F,0.0F}
} ;

typedef  struct _work  Work ;
typedef  struct _smoke SMOKE ;

struct _smoke
{
	DG_PRIM2   *prim ;
	DG_TEX	 *tex ;
	int		count ;
	int		( *smoke_act )( Work *,SMOKE *) ;

	int		patern[N_VERTS] ;
	float	  pic_scale_u,pic_scale_v ;
	float	  scale_u,scale_v ;
} ;

struct _work
{
	GV_ACT_EX	 actor ;

	/* 火花のエフェクト用 */
	int		( *fire_act )( Work * ) ;
	int		fire_count ;

	/* 煙のエフェクト用 */
	SMOKE	  smoke ;

	int		all_count ;
	FVECTOR	center ;
	SVECTOR	rot ;
	
} ;

/* -----------------------------------------------------------------*/
				  /* 
					  プログラム使用サブルーチン
				  */

extern void  _BigScrCopy(void *dst, void *src, int size, int num) ;
extern void  _BigMemCopy(void *dst, void *src, int size, int num) ;

/* -----------------------------------------------------------------*/
static   int  None_Act( Work *work )
{
	return 0 ;
}

static   int  Fire_Act( Work *work )
{
	SVECTOR	local_rot ;
	SVECTOR	rot_wide ;
	FVECTOR	color ;
	FVECTOR	pos ;
	int		n ;
	float	  ftmp ;
	float	  sloop ;

	if(work->all_count > FIRE_MAX_COUNT )
	{	
	work->fire_act = (void *)None_Act ;
	return 0 ;
	}
	
	if(work->fire_count -- < 0)
	{
	ftmp = IRS_BOX_FIRE_AREA[1].vx - IRS_BOX_FIRE_AREA[0].vx;
	pos.vx = IRS_BOX_FIRE_AREA[0].vx + rnd() * ftmp ;
	ftmp = IRS_BOX_FIRE_AREA[1].vy - IRS_BOX_FIRE_AREA[0].vy ;
	pos.vy = IRS_BOX_FIRE_AREA[0].vy + rnd() * ftmp ;
	
	/* 直線 (75,180) - (115,20) */
	sloop = ftmp / (IRS_BOX_FIRE_AREA[0].vz - IRS_BOX_FIRE_AREA[1].vz) ;
	
	pos.vz = (pos.vy - IRS_BOX_FIRE_AREA[0].vy) / sloop + IRS_BOX_FIRE_AREA[1].vz ;
	pos.vw = 1.0F ;

	DG_SetPos2(&work->center,&work->rot) ;
	DG_PutVector(&pos,&pos,1) ;

	local_rot.vx = - 128 + irnd() % 256 ;
	local_rot.vy = work->rot.vy - 128 + irnd() % 256 ;
	local_rot.vz = 0 ;

	rot_wide.vx = 512 ;
	rot_wide.vy = 512 ;
	rot_wide.vz = 0 ;

	color.vx = 255.0F ;
	color.vy = 128.0F ;
	color.vz = 128.0F ;
	color.vw = 50.0F ;
	
	n = 16 + irnd() % 8 ;
	NewSpark1(n,&pos,36.0F,36.0F,(rnd() * 3.0F),&local_rot,&rot_wide,&color,0.5F,10) ;
	GM_SeSetMode( SD_E_HIBANA02 , &pos, GM_SEMODE_NORMAL ) ;

//	Big_TmpLight2(&pos,300.0F,600.0F,0x404080,3) ;
	DG_SetTmpLight2(
		&pos,
		300.0f,
		600.0f,
		80 | 40<<8 | 40 <<16,
		LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;

	work->fire_count = irnd() % 10 + 5 ;
	}
	
	return 0 ;
}

/* ----------------------------------------------------------------- */
static void Smoke_Act(Work *work,SMOKE *smoke)
{
	DG_PRIM2		  *prim ;
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	FVECTOR		   shift ;
	int			   *patern ;
	int			   i ;
	int			   x,y ;
	float			 offset_u,offset_v ;
	DG_TEX			*tex ;

	prim = smoke->prim ;
	_BigScrCopy(MEM_SMOKE_POS,prim->pos[prim->buffer_clock],sizeof(FVECTOR),N_VERTS) ;
	_BigScrCopy(MEM_SMOKE_UVRGBWH,prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	
	pos = MEM_SMOKE_POS ;
	uvrgbwh = MEM_SMOKE_UVRGBWH ;

	if(work->all_count > (SMOKE_START_COUNT + 12)){
		/* アルファを減らし  大きくする */
		for(i = 0; i < N_VERTS; i++){	
			if(uvrgbwh->a < 1){
				uvrgbwh->a = 0 ;
			}else{
				uvrgbwh->a -= 1 ;
			}
			uvrgbwh->h += 5 ;
			uvrgbwh->w += 5 ;
			pos->vy += 3.5F ;
			pos ++ ;
			uvrgbwh ++ ;
		}
	}else{
		/* 煙を大きくする*/
		DG_SetPos2(&work->center,&work->rot) ;
		for(i = 0; i < N_VERTS; i++){
			uvrgbwh->w += 20 ;
			uvrgbwh->h += 20 ;
			shift.vx = 0.0F ;
			shift.vy = 4.0F ;
			shift.vz = (4.0F - 0.3F * (work->all_count - SMOKE_START_COUNT)) ;
			shift.vw = 1.0F ;
			DG_RotVector(&shift,&shift,1) ;
			_sceVu0AddVector(pos,pos,&shift) ;
			pos ++ ;
			uvrgbwh ++ ;
		}
	}

	/* テクスチャパターンチェック */
	if((work->all_count % 3) == 0){
		patern = smoke->patern ;
		uvrgbwh = MEM_SMOKE_UVRGBWH ;
		tex	 = smoke->tex ;
		for(i = 0; i < N_VERTS; i++){
			*patern = (*patern + 1) % MAX_PATERN ;
			x = *patern % 8 ;
			y = *patern / 8 ;
			offset_u = tex->u_offset + x * (32.0F * smoke->pic_scale_u) ;
			offset_v = tex->v_offset + y * (64.0F * smoke->pic_scale_v) ;
			uvrgbwh->u0 = FTOI12( offset_u ) ;
			uvrgbwh->v0 = FTOI12( offset_v ) ;
			uvrgbwh->u1 = FTOI12( smoke->scale_u + offset_u ) ;
			uvrgbwh->v1 = FTOI12( smoke->scale_v + offset_v ) ;
			uvrgbwh ++ ;
		}
	}

	/* バッファチャンネル変更 */
	DG_SwitchBuffPrim2(prim ) ;

	_BigMemCopy( prim->pos[prim->buffer_clock],MEM_SMOKE_POS,sizeof(FVECTOR),N_VERTS ) ;
	_BigMemCopy( prim->uvrgb[prim->buffer_clock],MEM_SMOKE_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),N_VERTS ) ;
}

static   int  None_Act_S(Work *work, SMOKE *smoke)
{
	DG_PRIM2		  *prim ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	int			   i ;

	if(work->all_count == SMOKE_START_COUNT){
		prim = smoke->prim ;

		DG_SwitchBuffPrim2(prim) ;
		uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
		smoke->smoke_act = (void *)Smoke_Act ;
		for(i = 0 ; i < N_VERTS; i++){
			uvrgbwh->r  = 24 + irnd() % 4 ;
			uvrgbwh->g  = 24 + irnd() % 4 ;
			uvrgbwh->b  = 24 + irnd() % 4 ;
			uvrgbwh->a  = 24 ;
			uvrgbwh->w  = (short)(50 + irnd() % 50) ;
			uvrgbwh->h  = uvrgbwh->w ;
			uvrgbwh ++ ;
		}
		smoke->smoke_act = (void *)Smoke_Act ;	
	}

	return 0 ;
}

/* ----------------------------------------------------------------- */

static  void  Act(Work *work)
{
	SMOKE  *smoke ;
	
	if(work->all_count > MAX_COUNT)
	{
	GV_DestroyActor( work ) ;
	return ;
	}

	(*work->fire_act)(work) ;

	smoke = &work->smoke ;
	(*smoke->smoke_act)(work,smoke) ;
	
	work->all_count ++ ;
}

/* ----------------------------------------------------------------- */
static  void  Die(Work *work)
{
	SMOKE  *smoke ;

	smoke = &work->smoke ;
	smoke->prim = OK_FreePrim2( smoke->prim ) ;
}

/* -----------------------------------------------------------------*/
/* 煙のスクラッチパット初期化 */
static void InitSmokeScrPad(Work *work,SMOKE  *smoke,DG_TEX *tex)
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				*patern ;
	float			  x,y ;
	float			  offset_u,offset_v ;
	int				i ;
	float			  ftmp_vx,ftmp_vy,sloop ;

	pos	 = MEM_SMOKE_POS ;
	uvrgbwh = MEM_SMOKE_UVRGBWH ;
	patern  = smoke->patern ;

	ftmp_vx = IRS_BOX_FIRE_AREA[1].vx - IRS_BOX_FIRE_AREA[0].vx ;
	ftmp_vy = IRS_BOX_FIRE_AREA[1].vy - IRS_BOX_FIRE_AREA[0].vy + 20.0F ;
	sloop = ftmp_vy / (IRS_BOX_FIRE_AREA[0].vz - IRS_BOX_FIRE_AREA[1].vz) ;

	for(i = 0; i < N_VERTS; i++)
	{
	/* 位置情報を格納 */
	pos->vx = IRS_BOX_FIRE_AREA[0].vx + rnd() * ftmp_vx ;
	pos->vy = IRS_BOX_FIRE_AREA[0].vy + rnd() * ftmp_vy ;
   
	/* 直線 (75,180) - (115,20) */	
	pos->vz = (pos->vy - IRS_BOX_FIRE_AREA[0].vy) / sloop + IRS_BOX_FIRE_AREA[1].vz ;
	pos->vy += 40.0F ;	   /* 水増し */
	pos->vz += 40.0F ;	   /* 水増し */
	pos->vw = 1.0F ;
				
	/* uvrgbwhデータを格納 */
	*patern = irnd() % MAX_PATERN ;
	x = *patern % 8 ;
	y = *patern / 8 ;
	offset_u = tex->u_offset + x * (32.0F * smoke->pic_scale_u) ;
	offset_v = tex->v_offset + y * (64.0F * smoke->pic_scale_v) ;

	uvrgbwh->u0 = FTOI12( offset_u ) ;
	uvrgbwh->v0 = FTOI12( offset_v ) ;
	uvrgbwh->u1 = FTOI12( smoke->scale_u + offset_u ) ;
	uvrgbwh->v1 = FTOI12( smoke->scale_v + offset_v ) ;
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;

	/* WH値は整数なので注意 ! */
	uvrgbwh->w = 0 ;
	uvrgbwh->h = 0 ;
	uvrgbwh->r = 0 ;
	uvrgbwh->g = 0 ;
	uvrgbwh->b = 0 ;
	uvrgbwh->a = 0 ;

	pos ++ ;
	uvrgbwh ++ ;
	patern ++ ;
	}

	DG_SetPos2(&work->center,&work->rot) ;
	pos = MEM_SMOKE_POS ;
	DG_PutVector(pos,pos,N_VERTS) ;
}

/* 煙初期化 */
static int InitSmoke( Work *work)
{
	SMOKE			*smoke ;
	DG_PRIM2		 *prim ;
	DG_TEX		   *tex ;

	smoke = &work->smoke ;
	prim  = smoke->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL)
	return -1 ;

	/* テクスチャデータ取得 */
	tex = smoke->tex = DG_GetTexture(16238439 /*"smoke_msk"*/) ;
	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;

	smoke->pic_scale_u = tex->u_scale / 255.0F ;
	smoke->pic_scale_v = tex->v_scale / 255.0F ;
	smoke->scale_u	 = smoke->pic_scale_u * 31.0F ;
	smoke->scale_v	 = smoke->pic_scale_v * 63.0F ;

	InitSmokeScrPad(work,smoke,tex) ;
	_BigMemCopy( prim->pos[0],MEM_SMOKE_POS,sizeof(FVECTOR),N_VERTS ) ;
	_BigMemCopy( prim->uvrgb[0],MEM_SMOKE_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	_BigMemCopy( prim->pos[1],MEM_SMOKE_POS,sizeof(FVECTOR),N_VERTS ) ;
	_BigMemCopy( prim->uvrgb[1],MEM_SMOKE_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	smoke->smoke_act = None_Act_S ;

	return 0 ;
}

static int GetResources( Work *work )
{
	work->all_count = 0 ;
	work->fire_count = irnd() % 10 + 1;
	work->fire_act = (void *)Fire_Act ;

	return (InitSmoke( work )) ;

}

void *NewIRS_Box_Break_Effect(FVECTOR  *center,SVECTOR *rot)
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->center = *center ;
		work->rot	= *rot ;
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

