//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	武器のマズルフラッシュ 
	2000/01/11   H.TANAKA
	2000/10/18 S.Okajima
	$Id: gunsmoke_single.c,v 1.1.1.3 2002/11/19 11:50:34 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"big_weapon.h"

#include "libutl.h"
#include	"../../okajima/etc/ok_util.h"
#include	"utl_dma.h"
#include	"def_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ 
	*/

#define	FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	N_PRIMS	  1
#define	N_VERTS	  1
#define	PRIM_TYPE	( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA )

/* ---------------------------------------------------------------- */
typedef struct _work   Work ;

struct _work
{
	GV_ACT_EX	  actor ;
	
	DG_PRIM2	*prim ;
	DG_TEX	  *tex ;
	int		 count ;
	float	   scale_u,scale_v,pic_scale_u,pic_scale_v ;
	FVECTOR	 speed ;
	u_char	  mode  ;
	int		 patern ;
	int		 ( *act )( Work * ) ;

} ;

/* 排莢部の煙  */
static  int   Act_Spread1(Work *work)
{
	FVECTOR		   *pre_pos, *pos ;
	DG_PRIM2_UVRGBWH  *pre_uvrgbwh, *uvrgbwh ;
	DG_PRIM2		  *prim ;
	DG_TEX			  *tex ;
	int				 x,y ;
	float			   offset_u,offset_v ;
	int				 tmp ;

	prim = work->prim ;
	pre_pos = prim->pos[prim->buffer_clock] ;
	pre_uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	
	/* 描画開始  */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;

	/* 位置  */
	_sceVu0AddVector(pos,pre_pos,&work->speed) ;

	/* サイズ ＋ 色  */
	uvrgbwh->w = pre_uvrgbwh->w + 5 ;
	uvrgbwh->h = pre_uvrgbwh->h + 5 ;
	tmp = (int)pre_uvrgbwh->r - 1 ;
	if(tmp < 0)
	uvrgbwh->r = 0 ;
	else
	uvrgbwh->r = (u_char)tmp ;
	tmp = (int)pre_uvrgbwh->g - 1 ;
	if(tmp < 0)
	uvrgbwh->g = 0 ;
	else
	uvrgbwh->g = (u_char)tmp ;
	tmp = (int)pre_uvrgbwh->b - 1 ;
	if(tmp < 0)
	uvrgbwh->b = 0 ;
	else
	uvrgbwh->b = (u_char)tmp ;

	/* パターン  */
	work->patern += 1 ;
	if(work->patern >= 30)
	work->patern = 0 ;
	x = work->patern % 8 ;
	y = work->patern / 8 ;
	tex = work->tex ;
	offset_u = tex->u_offset + x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + y * (64.0F * work->pic_scale_v) ;
	uvrgbwh->u0 = FTOI12(offset_u) ;
	uvrgbwh->v0 = FTOI12(offset_v) ;
	uvrgbwh->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh->v1 = FTOI12( work->scale_v + offset_v ) ;

	if(work->count-- <= 0)
	{
	return 1 ;
	}

	return 0 ;
}

/* マズル時の煙  */
static  int   Act_Spread0(Work *work)
{
	FVECTOR		   *pre_pos, *pos ;
	DG_PRIM2_UVRGBWH  *pre_uvrgbwh, *uvrgbwh ;
	DG_PRIM2		  *prim ;
	DG_TEX			  *tex ;
	int				 x,y ;
	float			   offset_u,offset_v ;
	int				 tmp ;

	prim = work->prim ;
	pre_pos = prim->pos[prim->buffer_clock] ;
	pre_uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	
	/* 描画開始  */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;

	/* 位置   */
	_sceVu0AddVector(pos,pre_pos,&work->speed) ;

	/* サイズ ＋ 色  */
	uvrgbwh->w = pre_uvrgbwh->w + 15 ;
	uvrgbwh->h = pre_uvrgbwh->h + 15 ;
	tmp = (int)pre_uvrgbwh->r - 1 ;
	if(tmp < 0)
	uvrgbwh->r = 0 ;
	else
	uvrgbwh->r = (u_char)tmp ;

	tmp = (int)pre_uvrgbwh->g - 1 ;
	if(tmp < 0)
	uvrgbwh->g = 0 ;
	else
	uvrgbwh->g = (u_char)tmp ;

	tmp = (int)pre_uvrgbwh->b - 1 ;
	if(tmp < 0)
	uvrgbwh->b = 0 ;
	else
	uvrgbwh->b = (u_char)tmp ;

	/* パターン  */
	work->patern += 1 ;
	if(work->patern >= 30)
	work->patern = 0 ;
	x = work->patern % 8 ;
	y = work->patern / 8 ;
	tex = work->tex ;
	offset_u = tex->u_offset + (float)x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + (float)y * (64.0F * work->pic_scale_v) ;
	uvrgbwh->u0 = FTOI12(offset_u) ;
	uvrgbwh->v0 = FTOI12(offset_v) ;
	uvrgbwh->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh->v1 = FTOI12( work->scale_v + offset_v ) ;

	if(work->count-- <= 0)
	{
	return 1 ;
	}

	return 0 ;
}

/* 拡散動作  */
static  int   Act_Deep(Work *work)
{
	FVECTOR			 *pre_pos,*pos ;
	DG_PRIM2_UVRGBWH	*pre_uvrgbwh, *uvrgbwh ;
	DG_PRIM2			*prim ;
	DG_TEX			  *tex ;
	int				 x,y ;
	float			   offset_u,offset_v ;

	prim = work->prim ;
	pre_pos = prim->pos[prim->buffer_clock] ;
	pre_uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	
	/* 描画開始  */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	
	/* 位置セット  */
	_sceVu0AddVector(pos,pre_pos,&work->speed) ;

	/* カラーセット  */
	uvrgbwh->r = pre_uvrgbwh->r + 2 ;
	uvrgbwh->g = pre_uvrgbwh->g + 2 ;
	uvrgbwh->b = pre_uvrgbwh->b + 2 ;

	/* パターン  */
	work->patern += 1 ;
	if(work->patern >= 30)
	work->patern = 0 ;
	x = work->patern % 8 ;
	y = work->patern / 8 ;
	tex = work->tex ;
	offset_u = tex->u_offset + (float)x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + (float)y * (64.0F * work->pic_scale_v) ;
	uvrgbwh->u0 = FTOI12(offset_u) ;
	uvrgbwh->v0 = FTOI12(offset_v) ;
	uvrgbwh->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh->v1 = FTOI12( work->scale_v + offset_v ) ;

	if(work->count-- <= 0)
	{
	work->speed.vx = 0.0F ;
	work->speed.vy = 1.0F ;
	work->speed.vz = 0.0F ;
	work->speed.vw = 0.0F ;
	work->count = 15 ;
	if(work->mode == 0)
		work->act = (void *)Act_Spread0 ;
	else if(work->mode == 1)
		work->act = (void *)Act_Spread1 ;
		
	}
	return 0 ;
}

/* 最初の動き  */
static  int   Act_Visible(Work *work)
{
	DG_VisiblePrim2(work->prim) ;
// yano
//	DG_InvisiblePrim2Chanl( work->prim, 1 );

	work->count = 5 ;

	work->act = (void *)Act_Deep ;
	return 0 ;
}

/* ----------------------------------------------------------------------  */
static  void   Act(Work *work)
{
/*
	{
	DG_PRIM2		  *prim ;
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;

	prim = work->prim ;
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	printf("------------- \n count  %d, buff %d\n",work->count,prim->buffer_clock) ;
	printf("pos x %f, y %f, z %f\n",pos->vx,pos->vy,pos->vz) ;
	printf("uvrgbwh  r %d ,g %d, b %d, a %d\n",
				 uvrgbwh->r,uvrgbwh->g,uvrgbwh->b,uvrgbwh->a) ;
	printf("w  %d, h %d,patern  %d\n",uvrgbwh->w,uvrgbwh->h,work->patern ) ;
	}
*/												   
	if((*work->act)(work))
	{
	GV_DestroyActor(work) ;
	};
}

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

static  int GetResources(
Work		*work,
FVECTOR	 *start,
FVECTOR	 *speed,
float	   size,
u_char	  mode			
)
{
	DG_PRIM2		  *prim ;
	DG_TEX			*tex ;
	FVECTOR		   *pos_a,*pos_b ;
	DG_PRIM2_UVRGBWH  *uvrgbwh_a,*uvrgbwh_b ;
	int			   x,y ;
	float			 offset_u,offset_v ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL)
	return -1 ;
	
	tex = work->tex = DG_GetTexture( GV_StrCode("smoke_msk")) ;
	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ; 

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする  */
	work->pic_scale_u = tex->u_scale / 255.0F ;
	work->pic_scale_v = tex->v_scale / 255.0F ;
	work->scale_u	 = work->pic_scale_u * 31.0F ;
	work->scale_v	 = work->pic_scale_v * 63.0F ;
	work->count   = 0 ;
	work->speed = *speed ;
	work->mode  = mode ;
	
	/* 位置決定  */
	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;

	*pos_a = *start ;
	*pos_b = *start ;

	/* uvrbwh決定  */
	work->patern = irnd() % 30 ;
	x = work->patern % 8 ;
	y = work->patern / 8 ;
	offset_u = tex->u_offset + (float)x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + (float)y * (64.0F * work->pic_scale_v) ;

	uvrgbwh_a = prim->uvrgb[0] ;
	uvrgbwh_b = prim->uvrgb[1] ;
	uvrgbwh_a->u0 = FTOI12(offset_u) ;
	uvrgbwh_a->v0 = FTOI12(offset_v) ;
	uvrgbwh_a->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh_a->v1 = FTOI12( work->scale_v + offset_v ) ;
	uvrgbwh_a->q0 = 4096 ;
	uvrgbwh_a->q1 = 4096 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ;
	uvrgbwh_a->w = (short)size ;
	uvrgbwh_a->h = (short)size ;
	uvrgbwh_a->r = 6;
	uvrgbwh_a->g = 6;
	uvrgbwh_a->b = 6;
	uvrgbwh_a->a = 128;
	uvrgbwh_b->u0 = FTOI12(offset_u) ;
	uvrgbwh_b->v0 = FTOI12(offset_v) ;
	uvrgbwh_b->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh_b->v1 = FTOI12( work->scale_v + offset_v ) ;
	uvrgbwh_b->q0 = 4096 ;
	uvrgbwh_b->q1 = 4096 ;
	uvrgbwh_b->f0 = 0x0fff ;
	uvrgbwh_b->f1 = 0x0fff ;
	uvrgbwh_b->w = (short)size ;
	uvrgbwh_b->h = (short)size ;
	uvrgbwh_b->r = 6;
	uvrgbwh_b->g = 6;
	uvrgbwh_b->b = 6;
	uvrgbwh_b->a = 128;

	work->act = (void *)Act_Visible ;
	DG_InvisiblePrim2(prim) ;

	return 0 ;
}

void *NewGunSmoke(FVECTOR *start,FVECTOR *speed, float size,u_char mode)
{
	
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, start, speed, size,mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;

}

