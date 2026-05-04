//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	primtest.c
	プリミティブ表示実験プログラム

	1999/07/07 K.Takabe
	$Id: piyori_anes.c,v 1.1.1.3 2002/11/19 11:48:53 Yoshizawa1 Exp $

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


/* ------------------------------------------------------ */
           /* 
               補助マクロ
           */

#define   FTOI12(_f)        ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define  N_PRIMS	 1
#define  N_VERTS 	 4
#define  PRIM_TYPE       (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
			     
#define    MEM_ADDR1    ((void *)( SCRPAD_ADDR + 0x0000 ))
#define    MEM_ADDR2    ((void *)( SCRPAD_ADDR + 0x0400 ))
#define    MEM_FVECTOR  ((void *)( SCRPAD_ADDR + 0x2000 ))
#define    MAX_COUNT    1800

typedef struct  _work  Work ;

struct _work	{
    GV_ACT_EX		actor ;
    
    DG_TEX          *tex ;
    DG_PRIM2	    *prim ;
    FMATRIX	    *world ;
    int             count[N_VERTS] ;         /* カウント */
    void             (*act[N_VERTS])(Work *, int) ;

    int             all_count ;
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
static  void  DecidePos(FMATRIX *world, FVECTOR *pos, int count)
{
    FVECTOR  *shift ;
    FVECTOR  *center ;
    FVECTOR  *base ;
    SVECTOR  ang ;

    shift = MEM_FVECTOR ;
    center = shift + 1 ;
    base   = center + 1;

    shift->vx = 1.2F * count ;
    shift->vy = 0.0F ;
    shift->vz = 0.0F ;
    shift->vw = 1.0F ;
    ang.vx = 0 ;
    ang.vy = (64 * count) % 4096 ;
    ang.vz = 0 ;

    DG_SetPos2(&DG_ZeroVector, &ang) ;
    DG_PutVector(shift, base,1) ;

    /* 移動距離 */
    shift->vx = 0.0F ;
    shift->vy = 120.0F ;
    shift->vz = 300.0F ;
    shift->vw = 1.0F ;
    DG_SetPos(world) ;
    DG_RotVector(shift,shift,1) ;
    
    center->vx = shift->vx + world->m[3][0] ;
    center->vy = 200.0F + 2.5F * count + world->m[3][1] ;
    center->vz = shift->vz + world->m[3][2] ;

    ang.vx = 0 ;
    ang.vy = 0 ;
    ang.vz = 0 ;
    DG_SetPos2(center,&ang) ;
    DG_PutVector(base,pos,1) ;
}

static  u_short  CheckColor(short tmp)
{
    if(tmp > 255)
	tmp = 255 ;
    if(tmp < 0)
	tmp = 0 ;
    
    return (u_short)tmp ;
}

static  void  DecideUVRGBWH(DG_PRIM2_UVRGBWH *uvrgbwh, int  count,SVECTOR *color)
{
    short  tmp ;
    
    tmp = (short)uvrgbwh->r + color->vx ;
    uvrgbwh->r = CheckColor(tmp) ;
    tmp = (short)uvrgbwh->g + color->vy ;
    uvrgbwh->g = CheckColor(tmp) ;
    tmp = (short)uvrgbwh->b + color->vz ;
    uvrgbwh->b = CheckColor(tmp) ;
    uvrgbwh->w = (short)(0.5F * count) ;
    uvrgbwh->h = (short)(0.5F * count) ;
}

/* -------------------------------------------------------------------- */ 
static  void  Act_FIRST(Work *work, int num) ;
static  void  Act_SECOND(Work *work, int num) ;
static  void  Act_THIRD(Work *work, int num) ;

static  void  Act_THIRD(Work *work, int num)
{
    int               count ;
    FVECTOR           *pos ;
    DG_PRIM2_UVRGBWH  *uvrgbwh ;
    SVECTOR           color ;

    count = work->count[num] ;


    pos = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;
    pos += num ;
    uvrgbwh += num ;
    if(count <= 160)
    {
	DecidePos(work->world,pos,count) ;
	color.vx = 0;
	color.vy = 0;
	color.vz = 0;
	DecideUVRGBWH(uvrgbwh,count,&color) ;
	work->count[num] = count + 1 ;
    }
    else
    {
	uvrgbwh->r = 0 ;	
	uvrgbwh->g = 0 ;
	uvrgbwh->b = 0 ;
	*pos = DG_ZeroVector ;
	work->count[num] = 0 ;
	work->act[num] = Act_FIRST ;
    }
}

static  void  Act_SECOND(Work *work, int num)
{
    int                count ;
    FVECTOR            *pos ;
    DG_PRIM2_UVRGBWH   *uvrgbwh ;
    SVECTOR            color ;

    count = work->count[num] ;

    pos = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;
    pos += num ;
    uvrgbwh += num ;
    color.vx = 2 ;
    color.vy = 3 ;
    color.vz = 3 ;
    DecidePos(work->world,pos,count) ;
    DecideUVRGBWH(uvrgbwh,count,&color) ;
    if(count >= 50)	
	work->act[num] = Act_THIRD ;
    work->count[num] = count + 1 ;
}

static  void  Act_FIRST(Work *work, int num)
{
    int               count ;
    FVECTOR           *pos ;
    DG_PRIM2_UVRGBWH  *uvrgbwh ;
    SVECTOR           color ;
 
    count = work->count[num] ;

    pos = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;
    pos += num ;
    uvrgbwh += num ;
    if(count >= 0)
    {
	color.vx = 0;
	color.vy = 1;
	color.vz = 15;
	DecidePos(work->world,pos,count) ;
	DecideUVRGBWH(uvrgbwh,count,&color) ;
	work->act[num] = Act_SECOND ;
	
    }    
    work->count[num] = count + 1;

}
 	
static  void  Act(Work *work)
{
    int  i ;
    DG_PRIM2  *prim ;
    int    addr ;

    addr = (int)work ;
    if(work->all_count > MAX_COUNT)
    {
	GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
	GV_DestroyActor(work) ;
	return ;
    }
    
    prim = work->prim ;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
    /* uvrgbwh情報を転送 */
    _BigScrCopy(MEM_ADDR2, prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;

    for(i = 0; i < N_VERTS; i++)
    {
	(*work->act[i])(work,i) ;
    }

    DG_SwitchBuffPrim2( prim ) ;
    _BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
    _BigMemCopy(prim->uvrgb[prim->buffer_clock],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;

//    work->all_count ++ ;
}

static  void  Die(Work *work)
{
    if(work->prim != NULL) GM_FreePrim2(work->prim) ;
}
     
/* -------------------------------------------------------------------- */
static void Init_Scr(Work *work, DG_TEX *tex)
{
    FVECTOR           *pos ;
    DG_PRIM2_UVRGBWH  *uvrgbwh ;
    int               *count ;
    int               i ;
    
    pos     = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;
    
    count = work->count ;
    for(i = 0; i < N_VERTS; i++)
    {
	*count = - i * 40 ;
	pos = &DG_ZeroVector ;
	uvrgbwh->u0 = FTOI12( tex->u_offset ) ;
	uvrgbwh->v0 = FTOI12( tex->v_offset ) ;
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;
	uvrgbwh->w  = 0;
	uvrgbwh->h  = 0;
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

static int GetResources( Work *work )
{
	DG_TEX	       *tex ;
	DG_PRIM2       *prim ;

printf("anes\n") ;
	if(work->world == NULL)
	    return -1 ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS);
	if(prim == NULL) return -1 ;
//	tex = DG_GetTexture(GV_StrCode("zzz_msk"));
	tex = DG_GetTexture(13722182);
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 )) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;/* 加算半透明 */

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	Init_Scr(work,tex) ;
	_BigMemCopy(prim->pos[0],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	_BigMemCopy(prim->pos[1],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
//printf("anes end\n") ;
	/* シグナル設定 */
//	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

//	work->all_count ++;
	return 0;
}

/* -------------------------------------------------------------------- */
void *NewPiyori_Anes(FMATRIX *world)
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) 
	{
	    work->world = world ;
	    GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
	    if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	    }
	}
	return (void *)work ;
}
