//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	武器のマズルフラッシュ 
	2000/01/11   H.TANAKA
	2000/10/18 S.Okajima
	$Id: gunsmoke_single2.c,v 1.1.1.3 2002/11/19 11:50:34 Yoshizawa1 Exp $
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

#include "libutl.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"

#include	"big_weapon.h"
#include    "../test/etc.h"


/* ---------------------------------------------------------------- */
	/*
		補助マクロ 
	*/

#define	FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	N_PRIMS	  5
#define	N_VERTS	  1
#define	PRIM_TYPE	( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA )
#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define ALPHA_DOWN  (irnd() % 1 + 1);	
#define WIND_MAX    (2)                // 風が煙に与える最大の力 
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

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
	float    angle ;
	int		 patern;
	int        life;
	float     size;
	float    alpha;
	int		 ( *act )( Work * ) ;

} ;

static  int   Act_Spread1(Work *work)
{
	FVECTOR		   *pre_pos, *pos ;
	DG_PRIM2_UVRGBWH  *pre_uvrgbwh, *uvrgbwh ;
	DG_PRIM2		  *prim ;
	DG_TEX			  *tex ;
	int				 x , y , i;
	float			 offset_u,offset_v ;
	FVECTOR          wind_tmp;
	int				 tmp ;
	int              tmp_count;
	int              alpha; // 計算用 

	prim = work->prim;
	pre_pos     = prim->pos[ prim->buffer_clock ];
	pre_uvrgbwh = prim->uvrgb[ prim->buffer_clock ];
	
	/* 描画開始 */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos = prim->pos[prim->buffer_clock] ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;

	work->angle += ( 1.f * PI / 360.f);
	if ( work->angle > 180.f * PI / 360.f){
		work->angle = -180.f * PI / 180.f;
	}
	work->patern += 1;
	if ( work->patern >= 30 ){
		work->patern = 0;
	}
	tmp_count = 0;
	for ( i = 0 ; i < N_PRIMS * N_VERTS ; i++ ){ 
		if ( uvrgbwh->a == 0 ){
			tmp_count++;
			continue;
		}
		/* 位置 */
		_sceVu0AddVector(pos,pre_pos,&work->speed) ;
		// 風の影響 
		_sceVu0Normalize( &wind_tmp , &G_wind );
		_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
		_sceVu0AddVector( pos , pos , &wind_tmp );
		/* サイズ ＋ 色 */
		uvrgbwh->w = ( work->size ) * cosf( work->angle ) ;
		uvrgbwh->h = ( work->size ) * sinf( work->angle );
		alpha = ( int )pre_uvrgbwh->a - ALPHA_DOWN;
		if ( alpha < 0.f ){
			uvrgbwh->a = 0;
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			continue;
		} else {
			uvrgbwh->a = alpha;
		}
		work->size += 1.f;
		tmp = (int)pre_uvrgbwh->r - 1 ;
		if ( tmp < 0 ){
			uvrgbwh->r = 0 ;
		} else {
			uvrgbwh->r = (u_char)tmp ;
		}
		tmp = (int)pre_uvrgbwh->g - 1 ;
		if( tmp < 0 ){
			uvrgbwh->g = 0 ;
		} else {
			uvrgbwh->g = (u_char)tmp ;
		}
		tmp = (int)pre_uvrgbwh->b - 1 ;
		if( tmp < 0 ){
			uvrgbwh->b = 0 ;
		} else {
			uvrgbwh->b = (u_char)tmp ;
		}

		/* パターン */
		x = work->patern % 8;
		y = work->patern / 8;
		tex = work->tex;
		offset_u = tex->u_offset + x * (32.0F * work->pic_scale_u);
		offset_v = tex->v_offset + y * (64.0F * work->pic_scale_v);
		uvrgbwh->u0 = FTOI12(offset_u);
		uvrgbwh->v0 = FTOI12(offset_v);
		uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
		uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );

		pos++;
		pre_pos++;
		uvrgbwh++;
		pre_uvrgbwh++;
	}

	if( tmp_count >= N_PRIMS ){
		return 1 ;
	}

	return 0 ;
}

/* 拡散動作 */
static  int   Act_Deep(Work *work)
{
	FVECTOR			    *pre_pos,*pos ;
	DG_PRIM2_UVRGBWH	*pre_uvrgbwh, *uvrgbwh ;
	DG_PRIM2			*prim ;
	DG_TEX			    *tex ;
	int				     x , y ,i;
	float			     offset_u,offset_v ;

	prim = work->prim ;
	pre_pos = prim->pos[prim->buffer_clock] ;
	pre_uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	
	/* 描画開始 */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos     = prim->pos[ prim->buffer_clock ];
	uvrgbwh = prim->uvrgb[ prim->buffer_clock ];
	
	for ( i = 0 ; i < N_PRIMS * N_VERTS ; i++ ){ 
		/* 位置セット */
		_sceVu0AddVector( pos , pre_pos , &work->speed );
		/* カラーセット */
		uvrgbwh->r = pre_uvrgbwh->r + 2 ;
		uvrgbwh->g = pre_uvrgbwh->g + 2 ;
		uvrgbwh->b = pre_uvrgbwh->b + 2 ;

		/* パターン */
		work->patern += 1 ;
		if(work->patern >= 30){
			work->patern = 0;
		}
		x = work->patern % 8;
		y = work->patern / 8;
		tex = work->tex;
		offset_u = tex->u_offset + (float)x * ( 32.0F * work->pic_scale_u );
		offset_v = tex->v_offset + (float)y * ( 64.0F * work->pic_scale_v );
		uvrgbwh->u0 = FTOI12(offset_u);
		uvrgbwh->v0 = FTOI12(offset_v);
		uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
		uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );

		pos++;
		pre_pos++;
		uvrgbwh++;
		pre_uvrgbwh++;
	}
	if(work->count-- <= 0) {
		work->speed.vx = 0.0F ;
		work->speed.vy = 1.0F ;
		work->speed.vz = 0.0F ;
		work->speed.vw = 0.0F ;
		work->count = work->life;
		work->act = (void *)Act_Spread1;
	}
	return 0 ;
}

/* 最初の動き */
static int Act_Visible( Work *work )
{
	DG_VisiblePrim2( work->prim );
// yano
//	DG_InvisiblePrim2Chanl( work->prim, 1 );

	work->count = work->life / 2;

	work->act = (void *)Act_Deep;

	return 0;
}

/* ---------------------------------------------------------------------- */
static  void   Act(Work *work)
{
	if( (*work->act)( work ) ){
		GV_DestroyActor( work );
	}
}

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

static  int GetResources(Work *work , FMATRIX *pWorld , FVECTOR *speed, float size )
{
	DG_PRIM2		  *prim ;
	DG_TEX			  *tex ;
	FVECTOR		      *pos;
	DG_PRIM2_UVRGBWH  *uvrgbwh;
	FVECTOR		      ftmp;
	int			      x , y, i;
	float			  offset_u,offset_v;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL){
		return ( -1 );
	}
	tex = work->tex = DG_GetTexture( GV_StrCode("smoke_msk")) ;
	if(tex == NULL){
		return ( -1 );
	}
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	work->pic_scale_u = tex->u_scale / 255.0F;
	work->pic_scale_v = tex->v_scale / 255.0F;
	work->scale_u	 = work->pic_scale_u * 31.0F;
	work->scale_v	 = work->pic_scale_v * 63.0F;
	work->count   = 0;
	work->speed = *speed;
	work->size = size;
	/* 位置決定 */
	pos = MEM_ADDR1;
	uvrgbwh = MEM_ADDR2;

	/* uvrbwh決定 */
	work->patern = irnd() % 30 ;
	x = work->patern % 8 ;
	y = work->patern / 8 ;
	offset_u = tex->u_offset + (float)x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + (float)y * (64.0F * work->pic_scale_v) ;

	work->angle = frnd() * 180.f * PI / 360.f;
	for ( i = 0 ; i < N_PRIMS * N_VERTS ; i++ ){
		ftmp.vx = 0;
		ftmp.vy = frnd() * 20;
		ftmp.vz = frnd() * 10;
		DG_SetPos( pWorld );
		DG_PutVector( &ftmp , pos , 1 ); 		
		uvrgbwh->u0 = FTOI12( offset_u );
		uvrgbwh->v0 = FTOI12( offset_v );
		uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
		uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;
		uvrgbwh->f0 = 0x0fff;
		uvrgbwh->f1 = 0x0fff;
		uvrgbwh->w = ( int )( size * cosf( work->angle ) );
		uvrgbwh->h = ( int )( size * sinf( work->angle ) );
		uvrgbwh->r = 6;
		uvrgbwh->g = 6;
		uvrgbwh->b = 6;
		uvrgbwh->a = work->alpha;
		uvrgbwh++;
		pos++;
	}
	_MemCopy( prim->pos[0] , MEM_ADDR1 , sizeof( FVECTOR ) , N_PRIMS * N_VERTS );
	_MemCopy( prim->pos[1] , MEM_ADDR1 , sizeof( FVECTOR ) , N_PRIMS * N_VERTS );
	_MemCopy( prim->uvrgb[0] , MEM_ADDR2 , sizeof( DG_PRIM2_UVRGBWH ) , N_PRIMS * N_VERTS );
	_MemCopy( prim->uvrgb[1] , MEM_ADDR2 , sizeof( DG_PRIM2_UVRGBWH ) , N_PRIMS * N_VERTS );

	work->act = (void *)Act_Visible;
	DG_InvisiblePrim2(prim) ;
	
	return 0 ;
}

void *NewGunSmoke2(FMATRIX *pWorld , FVECTOR *speed, float size , int life , float alpha )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->life = life;
		work->alpha = alpha;
		if ( GetResources( work, pWorld , speed, size ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;

}

