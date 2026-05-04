//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	画面上のオブジェクトに 数字と色をつけて画面上に表示する
	デバッグ用。
	1999/12/10     H.TANAKA
	2000/10/18 S.Okajima
	$Id: pri_objnum.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $
*/


/* 
   type

   
   0 .. 1    色のタイプ ( 0   赤   1 青  2 黄 )
   2         表示のon,off  ( on  1, off 0 )
   3 .. 31   番号

*/
#ifdef  DEBUG
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"gameheader.h"
#include	"def_dma.h"

UCVECTOR  Big_pri_objcolor[4] =
{
    { 192,0,0,255},{0,0,255,255},{255,255,0,255},{255,255,255,255}
} ;

typedef struct
{
    GV_ACT_EX     actor ;

    FVECTOR    *pos ;
    int        *type ;
} Work ;


static  void  SeparateType(
int *type,
int *num,
int *visible_flag,
int *color_flag
)
{
    int tmp ;
	
    tmp = *type ;
    *color_flag = tmp & 0x0003 ;
    tmp >>= 2 ;
    *visible_flag = tmp & 0x0001 ;
    tmp >>= 1 ;
    *num = tmp ;
}

static  void  Act(Work *work)
{
    int  num,color_flag,visible_flag ;
    FVECTOR   *pos ;
    float     tmp ;
    FVECTOR   ret ;   
    int       x,y ;   /* 表示座標 */
    DG_CHANL  *cp ;
   

    if(work->type == NULL)
    {
	printf("pri_objnum.c : type address is NULL on Act\n") ;
	return ;
    }

    if(work->pos == NULL)
    {
	printf("pri_objnum.c : pos address is NULL on Act\n") ;
	return ;
    }

    SeparateType(work->type,&num,&visible_flag,&color_flag) ;

    /* 表示関係 */
    if(visible_flag == 0)
    {
	return ;
    }

    /* 位置決定 */
    cp = DG_Chanl(0) ;
    pos = work->pos ;
    tmp = pos->vw ;
    pos->vw = 1.0F ;
    _sceVu0ApplyMatrix(&ret, &cp->eye_pers, pos) ;
    pos->vw = tmp ;

    /* カメラの範囲内に入っているかどうか */
    if(ret.vz > ret.vw)
    {
	return ;
    }
    if(ret.vw < 0) ret.vw = - ret.vw ;
    
    if((ret.vx > ret.vw) || (ret.vx < (-ret.vw)))
    {
	return ;
    }
    
    if((ret.vy > ret.vw) || (ret.vy < (-ret.vw)))
    {
	return ;
    }
    
    x = (int)((ret.vx / ret.vw) * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1) ;
    y = (int)((ret.vy / ret.vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) ;
    DEBUG_Locate(x,y,0) ;
    DEBUG_Color(
		Big_pri_objcolor[color_flag].r,
		Big_pri_objcolor[color_flag].g,
		Big_pri_objcolor[color_flag].b,
		Big_pri_objcolor[color_flag].a
		) ;
    DEBUG_Printf("%1d\n",num) ;
	DEBUG_ResetColor();
}

static  void  Die(Work *work)
{
    
}

static  int  GetResources(Work  *work)
{
    printf("pos_address in init %08x \n",(int)work->pos) ;
    if(work->pos == NULL)
    {
	printf(" pri_objnum.c : pos address is NULL\n") ;
	return -1 ;
    }
    if(work->type == NULL)
    {
	printf(" pri_objnum.c : type address is NULL\n") ;
	return -1 ;
    }

    return  0 ;
    
}

void *NewBig_Add_Objnum( FVECTOR *pos,int *type)
{
    Work *work ;

    work = (Work *)GV_NewEffect(GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if( work != NULL){
		GV_SetActor( &(work->actor), Act,Die) ;
		GV_ActorEX( &work->actor )

		work->pos = pos ;
		work->type = type ;

		if( GetResources(work) < 0){
		    GV_DestroyActor( work ) ;
		    return NULL ;
		}
	}
	return (void *)work ;
}

#endif
