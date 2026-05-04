//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	zzz.c
	1999/12/15 T.Tanaka
	$Id: zzz.c,v 1.1.1.3 2002/11/19 11:48:53 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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



/* ------------------------------------------------------ */
		   /* 
			   補助マクロ
		   */

#define   FTOI12(_f)		( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define   N_PRIMS	 1
#define   N_VERTS	 3
#define   INTERVAL   300

#define   PRIM_TYPE   (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	MEM_ADDR1	((void *)( SCRPAD_ADDR + 0x0000 ))
#define	MEM_ADDR2	((void *)( SCRPAD_ADDR + 0x0400 ))

typedef  struct _work  Work ;
struct _work
{
	GV_ACT_EX	actor ;
	DG_TEX	*tex ;
	DG_PRIM2   *prim ;
	FMATRIX   *world ;
	int	   count[N_VERTS] ;
	void   (*act[N_VERTS])(Work *, int) ;

	int		mode;
	int		mode_count;
} ;

/* -------------------------------------------------------------------- */ 
/* メインメモリから スクラッチパッドへの転送 */
/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/* ------------------------------------------------------------------ */ 

#if 0
static  int  ReceiveSignal( void *workp, int signal, int value)
{
	Work *work = workp ;
	int  addr  ;

	addr = (int)work ;

	/* デストロイ */
	if(signal == GV_SIGNAL_KILL)
	{
	if((addr == value) || (addr < 0))
		GV_DestroyActor(work) ;
	}	
	
}
#endif

/* -------------------------------------------------------------------- */ 
static  u_short  CheckColor(short tmp)
{
	if(tmp > 255)
	tmp = 255 ;
	if(tmp < 0)
	tmp = 0;

	return (u_short)tmp ;
}

static  void  DecideUVRGBWH(DG_PRIM2_UVRGBWH *uvrgbwh,int count,SVECTOR *color)
{
	short  tmp ;

	tmp = (short)uvrgbwh->r + color->vx  ;
	uvrgbwh->r = CheckColor(tmp) ;

	tmp = (short)uvrgbwh->g + color->vy ;
	uvrgbwh->g = CheckColor(tmp) ;

	tmp = (short)uvrgbwh->b + color->vz;
	uvrgbwh->b = CheckColor(tmp) ;

}

static  void  DecidePos(FVECTOR *pos,FVECTOR *center,int count)
{
	pos->vx = center->vx + 2.0F * count ;
	pos->vy = center->vy + 10.0F * count ;
	pos->vz = center->vz ;
	pos->vw = 1.0F ;
}

/* -------------------------------------------------------------------- */
static  void  Act_FIRST(Work *work, int num) ;
static  void  Act_SECOND(Work *work, int num) ;
static  void  Act_THIRD(Work *work, int num) ;

static void  Act_THIRD(Work *work, int num)
{
	int			   count ;
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	FVECTOR		   *center ;

	count = work->count[num] ;
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	pos += num ;
	uvrgbwh += num ;
	
	center = (FVECTOR *)work->world->m[3] ;
	if(count <= 80){
		DecidePos(pos,center,count) ;
		uvrgbwh->w = (short)(2 + count * 1.5F) ;
		uvrgbwh->h = (short)(2 + count * 1.5F) ;
		work->count[num] = count + 1 ;
	}else{
		work->count[num] = - INTERVAL ;
		pos->vx = 0.0F ;
		pos->vy = 0.0F ;
		pos->vz = 0.0F ;
		pos->vw = 1.0F ;
		uvrgbwh->w  = 2;
		uvrgbwh->h  = 2;
		uvrgbwh->r  = 0 ;
		uvrgbwh->g  = 0 ;
		uvrgbwh->b  = 0 ;

		if( work->mode == 0 ){
			work->act[num] = Act_FIRST ;
		}else{
			work->mode_count++;
			if( work->mode_count >= N_VERTS ){
				GV_DestroyActor( work ) ;
			}
		}
	}

}

static  void  Act_SECOND(Work *work, int num)
{
	int				count ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	SVECTOR			color ;
	FVECTOR			*center ;

	count = work->count[num] ;
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	pos += num ;
	uvrgbwh += num ;

	color.vx = 2 ;
	color.vy = 3 ;
	color.vz = 3 ;
	center = (FVECTOR *)work->world->m[3] ;
	DecidePos(pos,center,count) ;
	uvrgbwh->w = (short)(2 + count * 1.5F) ;
	uvrgbwh->h = (short)(2 + count * 1.5F) ;
	DecideUVRGBWH(uvrgbwh,count,&color) ;
	if(count >= 50) work->act[num] = Act_THIRD ;
	work->count[num] = count + 1 ;
}

static  void  Act_FIRST(Work *work, int num)
{
	int			   count ;
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	SVECTOR		   color ;
	FVECTOR		   *center ;

	count = work->count[num] ;
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	pos += num ;
	uvrgbwh += num ;
	center = (FVECTOR *)work->world->m[3] ;
	if(count >= 0){
		color.vx = 2;
		color.vy = 3;
		color.vz = 3;
		DecidePos(pos,center,count) ;
		uvrgbwh->w = (short)(2 + count * 1.5F) ;
		uvrgbwh->h = (short)(2 + count * 1.5F) ;
		DecideUVRGBWH(uvrgbwh,count,&color) ;
		work->act[num] = Act_SECOND ;
	}
	work->count[num] = count + 1 ;
}

static  void  Act(Work *work)
{
	DG_PRIM2		  *prim ;
	int			   i ;
	int			   addr ;

	if( work->world == NULL ){
		/* 死ぬ前にメッセージを送る */
		addr = (int)work ;
		GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
		GV_DestroyActor(work) ;
		return ;
	}

	prim = work->prim ;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	_BigScrCopy(MEM_ADDR1,prim->pos[prim->buffer_clock],sizeof(FVECTOR),N_VERTS) ;
	_BigScrCopy(MEM_ADDR2, prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;

	for(i = 0; i < N_VERTS; i++){
		(*work->act[i])(work,i) ;
	}

	DG_SwitchBuffPrim2(prim) ;
	_BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[prim->buffer_clock],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
}

static  void  Die(Work *work)
{
	if(work->prim != NULL){
		GM_FreePrim2(work->prim) ;
	}
}

/* -------------------------------------------------------------------- */
static  void  Init_Scr(Work *work, DG_TEX *tex)
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				*count ;
	int				i ;
	FVECTOR			*center ;

	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	
	center = (FVECTOR *)work->world->m[3] ;
	count = work->count ;
	for(i = 0; i < N_VERTS; i++){
		*count = - i * 30 ;
		pos->vx = 0.0F ;
		pos->vy = 0.0F ;
		pos->vz = 0.0F ;
		pos->vw = 1.0F ;
		uvrgbwh->u0 = FTOI12( tex->u_offset ) ;
		uvrgbwh->v0 = FTOI12( tex->v_offset ) ;
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->w  = 2;
		uvrgbwh->h  = 2;
		uvrgbwh->r  = 0 ;
		uvrgbwh->g  = 0 ;
		uvrgbwh->b  = 0 ;
		uvrgbwh->a  = 128 ;
		work->act[i] = Act_FIRST ;
		pos ++ ;
		uvrgbwh ++ ;
		count ++ ;
	}
}

static  int  GetResources( Work *work )
{
	DG_TEX		  *tex ;
	DG_PRIM2		 *prim ;

	if(work->world == NULL) return -1 ;

	work->mode_count=0;


	prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS) ;
	if(prim == NULL) return -1 ;
//	tex = work->tex = DG_GetTexture(GV_StrCode("zzz_msk")) ;
	tex = DG_GetTexture(13722182);
//	tex = work->tex = DG_GetTexture(GV_StrCode("zzz")) ;
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0) ;/* 加算半透明 */

	/* データを初期化 */
	Init_Scr(work,tex) ;
	_BigMemCopy(prim->pos[0],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	_BigMemCopy(prim->pos[1],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;

	/* シグナル設定 */
//	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

	return 0 ;
}

/* -------------------------------------------------------------------- */
void  *New_Zzz( FMATRIX  *world)
{
	Work  *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL){

		work->mode  = 0;		/* 自殺はしない */
		work->world = world ;

		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if( GetResources( work ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


void  *New_Zzz_Demo( FMATRIX *world )
{
	Work  *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL){

		work->mode  = 1;		/* 自殺する */
		work->world = world ;

		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if( GetResources( work ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


