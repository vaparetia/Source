//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	smoke.c
	ポインタ参照先に接続する力積付き煙
	2001/03/28 S.Kobayashi
	$Id: smoke.c,v 1.1.1.3 2002/11/19 11:50:36 Yoshizawa1 Exp $

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

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"
#include	"../../okajima/etc/ok_util.h"

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	MULTIPLE	(4.0f)


#define	GRAVI_RAND_LEN	(3.0f)

#define N_PRIMS		(16)
#define N_VERTS		(1)

#define	RGB_SMOKE		(255)
#define	ALPHA_MIN		(6.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE_MIN		(4.0f)
#define	SIZE_RND		(16.0f)

#define	ZOOM_ADD		(4.0f)
#define	ANGLE_ADD		(rnd() * 6.f * PI / 360.f)
#define	RAISE_SMOKE			(1000)
#define SMOKE_FLAG  ( DG_PRIM2_RSPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define WIND_MAX ( 5.f )
/*----------------------------------------------------------------*/
typedef	struct	{
	int			count;
	FVECTOR		vec;
	float		angle;
	float		size;
	float		alpha;
	float       alpha_sub;
	float       sign; // 回転の方向 
} Unit ;

typedef	struct _work {
	GV_ACT_EX		actor ;

	FMATRIX		world;	/* 銃のマズルスモークは、worldコピー式に変更 T.morita 2002.04.19 */
	FVECTOR		shift;
	FVECTOR     power;
	FVECTOR		before_pos;

	DG_PRIM2	*prim ;
	DG_TEX      *tex;
	int			life;
	int         num;
	float		size_rnd;
	float       alpha;
	float       patern;
	FVECTOR     range;
	Unit		*unit;//[N_PRIMS*N_VERTS]; 
	float	   scale_u,scale_v,pic_scale_u,pic_scale_v ;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_MODE_NORMAL = 0 , 
	SK_MODE_DEMO = 1 ,
};
 
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離) */
extern	SVECTOR	G_wind_rot ;			/* 風向  */
extern	int		G_wind_intense ;		/* 強さ  */
extern	int		G_wind_intense_max ;	/* 強さ  */
extern	int		G_wind_sw ;				/* 突風発生中に立つ  */
extern	FMATRIX	G_wind_matrix ;

/*----------------------------------------------------------------*/
static void NormalAct( Work *work )
{
	FVECTOR	now_pos;
	FVECTOR	dif_pos;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR wind_tmp;
	FMATRIX	fmtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	float offset_u , offset_v;
	int	x , y , i;
	int       tmp;
	int		clock;
	int		count;
	Unit	*unit;

	DG_VisiblePrim2( work->prim ) ;

#if 0
/// by yano
	DG_InvisiblePrim2Chanl( work->prim, 1 );
#endif

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	DG_SetPos( &work->world );
	DG_MovePos( &work->shift );
	DG_GetPos( &fmtemp );
	DG_COPY_VEC( &now_pos, (FVECTOR *)fmtemp.m[3] );

#ifdef SCE_LIB
	_sceVu0SubVector( &dif_pos, &now_pos, &work->before_pos ) ;
#else
	dif_pos.vx = now_pos.vx - work->before_pos.vx;
	dif_pos.vy = now_pos.vy - work->before_pos.vy;
	dif_pos.vz = now_pos.vz - work->before_pos.vz;
	dif_pos.vw = now_pos.vw - work->before_pos.vw;
#endif

	unit = work->unit;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	count = 0;
	tmp = N_VERTS * work->num;
	for( i = 0 ; i < tmp ; i++ ){
		if( unit->count-- > 0){
		} else if( unit->alpha > 0 ){
			pos_before->vy -= work->power.vy;
			_sceVu0AddVector( pos, pos_before, &unit->vec );
			// 風の影響 
			_sceVu0Normalize( &wind_tmp , &G_wind );
			_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
			_sceVu0AddVector( pos , pos , &wind_tmp );
			unit->alpha += ( 0 - unit->alpha ) / ( 26.f + ( float )i );
			if ( unit->alpha < 1 ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
				count++;
				unit++;
				pos++;
				pos_before++;
				uvrgbwh++;
				uvrgbwh_before++;
				continue;
			} else {
				// パターンの更新 
				uvrgbwh->pad0 = uvrgbwh_before->pad0 + ( int )work->patern;
				if ( uvrgbwh->pad0 >= 30 ){
					uvrgbwh->pad0 = 0;
				}
				uvrgbwh->pad1 = uvrgbwh->pad0;
				unit->alpha_sub -= 0.2f;
				if ( unit->alpha_sub < 0.4f ){
					unit->alpha_sub = 0.4f;
				}
				unit->size += ZOOM_ADD;
				unit->angle += ( ANGLE_ADD * unit->sign );
				if ( unit->angle > 180.f * PI / 360.f ){
					unit->angle = -180.f * PI / 360.f;
				}
				if ( unit->angle < -180.f * PI / 360.f ){
					unit->angle = 180.f * PI / 360.f;
				}
				uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
				uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
				/* パターン  */
				x = uvrgbwh->pad0 % 8;
				y = uvrgbwh->pad0 / 8;
				offset_u = work->tex->u_offset + x * (32.0F * work->pic_scale_u);
				offset_v = work->tex->v_offset + y * (64.0F * work->pic_scale_v);
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
			}
			uvrgbwh->a = (int)unit->alpha;
			_sceVu0DivVector( &unit->vec , &unit->vec , ( rnd() + 1.2f ) );
		}else if( work->life > 0 ){
			now_pos.vx += frnd() * work->range.vx;
			now_pos.vy += frnd() * work->range.vy;
			now_pos.vz += frnd() * work->range.vz;
			DG_COPY_VEC( pos, &now_pos );
			unit->angle  = rnd() * 60.f * PI / 360.f;
			unit->sign   = frnd() * 1.5f;
			unit->size   = SIZE_MIN  + work->size_rnd * rnd();
			unit->alpha  = work->alpha * rnd();
			unit->alpha_sub = 2.0f;
		}else{
			count++;
		}
		unit++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	if( count >= tmp ){
		GV_DestroyActor( work );
	}
	work->life--;
	work->patern += -work->patern / 20.f;
	if ( work->patern < 0.0f ){
		work->patern = 0.0f;
	}
	DG_COPY_VEC( &work->before_pos, &now_pos );

}

static	void ConvectionAct( Work *work )  // オセロット用 
{
	FVECTOR	now_pos;
	FVECTOR	dif_pos;
	FVECTOR	wind_tmp;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FMATRIX	fmtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	float offset_u , offset_v;
	int	x , y , i;
	int       tmp;
	int      tmp2;
	int		clock;
	int		count;
	Unit	*unit;

	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	DG_SetPos( &work->world );
	DG_MovePos( &work->shift );
	DG_GetPos( &fmtemp );
	DG_COPY_VEC( &now_pos, (FVECTOR *)fmtemp.m[3] );

	_sceVu0SubVector( &dif_pos, &now_pos, &work->before_pos ) ;

	unit = work->unit;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	count = 0;
	tmp = N_VERTS * work->num;
	for( i = 0 ; i < tmp ; i++ ){
		if( unit->alpha > 0 ){
			pos_before->vy -= work->power.vy;
			_sceVu0AddVector( pos, pos_before, &unit->vec );
			// 風の影響 
			_sceVu0Normalize( &wind_tmp , &G_wind );
			_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
			_sceVu0AddVector( pos , pos , &wind_tmp );
			unit->alpha += ( 0 - unit->alpha ) / ( rnd() * 60.f + 40.f );  			
			if ( unit->alpha < 1 ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
				count++;
				unit++;
				pos++;
				pos_before++;
				uvrgbwh++;
				uvrgbwh_before++;
				continue;
			} else {
				// パターンの更新 
				uvrgbwh->pad0 = uvrgbwh_before->pad0 + 1;
				if ( uvrgbwh->pad0 >= 30 ){
					uvrgbwh->pad0 = 0;
				}
				uvrgbwh->pad1 = uvrgbwh->pad0;

				unit->size  += rnd() * 10 * ZOOM_ADD;
				unit->angle += ( ANGLE_ADD * unit->sign );
				if ( unit->size > 255.f ){
					unit->size = 255.f;
				}
				if ( unit->angle > 180.f * PI / 360.f ){
					unit->angle = -180.f * PI / 360.f;
				}
				if ( unit->angle < -180.f * PI / 360.f ){
					unit->angle = 180.f * PI / 360.f;
				}
				uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
				uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
				/* パターン  */
				x = uvrgbwh->pad0 % 8;
				y = uvrgbwh->pad0 / 8;
				offset_u = work->tex->u_offset + x * (32.0F * work->pic_scale_u);
				offset_v = work->tex->v_offset + y * (64.0F * work->pic_scale_v);
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
				tmp2 = (int)uvrgbwh_before->r - 1 ;
				if ( tmp2 < 0 ){
					uvrgbwh->r = 0 ;
				} else {
					uvrgbwh->r = (u_char)tmp2 ;
				}
				tmp2 = (int)uvrgbwh_before->g - 1 ;
				if( tmp2 < 0 ){
					uvrgbwh->g = 0 ;
				} else {
					uvrgbwh->g = (u_char)tmp2 ;
				}
				tmp2 = (int)uvrgbwh_before->b - 1 ;
				if( tmp2 < 0 ){
				  	uvrgbwh->b = 0 ;
				} else {
					uvrgbwh->b = (u_char)tmp2 ;
				}
			}
			uvrgbwh->a = (int)unit->alpha;
			_sceVu0DivVector( &unit->vec , &unit->vec , ( rnd() + 1.2f ) );
		} else if( work->life > 0 ){
			now_pos.vx += frnd() * work->range.vx;
			now_pos.vy += frnd() * work->range.vy;
			now_pos.vz += frnd() * work->range.vz;
			DG_COPY_VEC( pos, &now_pos );
			unit->angle  = frnd() * 60.f * PI / 360.f;
			unit->sign   = frnd() * 1.5f;
			unit->size   = SIZE_MIN  + work->size_rnd * rnd();
			unit->alpha  = work->alpha * rnd();//ALPHA_MIN * 4 * rnd(); 
			unit->alpha_sub = 2.0f;
		}else{
			count++;
		}
		unit++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	if( count >= work->num ){
		GV_DestroyActor( work );
	}
	work->life--;

	DG_COPY_VEC( &work->before_pos, &now_pos );

}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
	if ( pWork->unit != NULL ){
		GV_DelayedFree( pWork->unit );
	}
	if ( pWork->prim != NULL ){
		pWork->prim = OK_FreePrim2( pWork->prim );
	}
}

static int InitPacket( Work *pWork, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR *pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用  */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用  */
	FMATRIX		fmtemp;
	Unit		*unit;
	int     tmp;
	int		x , y;
	float	offset_u,offset_v;
	int		i ;
	float   tmp2;

	tmp = N_VERTS * pWork->num;
	DG_SetPos( &pWork->world );
	DG_MovePos( &pWork->shift ); // 銃口にもっていく 
	DG_GetPos( &fmtemp );        // マトリクスの取得 
	DG_COPY_VEC( &pWork->before_pos, (FVECTOR *)fmtemp.m[3] ); // トランスの取得 

	pWork->unit = GV_Malloc( sizeof( Unit ) * tmp );
	if ( pWork->unit == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( pWork->unit , sizeof( Unit ) * tmp );
	// 加速度の計算 
	unit       = pWork->unit;
	unit->vec.vx = pWork->world.m[ 2 ][ 0 ];
	unit->vec.vy = pWork->world.m[ 2 ][ 1 ];
	unit->vec.vz = pWork->world.m[ 2 ][ 2 ];
	pos        = MEM_ADDR1 ;
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE_SMOKE;

	pWork->pic_scale_u = tex->u_scale / 255.0F;
	pWork->pic_scale_v = tex->v_scale / 255.0F;
	pWork->scale_u	 = pWork->pic_scale_u * 31.0F;
	pWork->scale_v	 = pWork->pic_scale_v * 63.0F;
	pWork->patern = 3.0f;
	for ( i = 0 ; i < tmp ; i++ ){
		unit->count  = i/(int)MULTIPLE;
		// 加速度の計算 
		tmp2 = frnd();
		tmp2 = tmp2 < 0 ? -tmp2 + 1 : tmp2 + 1 ;
		_sceVu0ScaleVector( &unit->vec , (FVECTOR *)pWork->world.m[ 1 ] , pWork->power.vz );
		_sceVu0ScaleVector( &unit->vec , &unit->vec , tmp2 );
		DG_COPY_VEC( pos , &pWork->before_pos );

		uvrgbwh0->pad0 = uvrgbwh1->pad0 = irnd() % 30;
		x = uvrgbwh0->pad0 % 8 ;
		y = uvrgbwh0->pad0 / 8 ;
		offset_u = pWork->tex->u_offset + (float)x * (32.0F * pWork->pic_scale_u) ;
		offset_v = pWork->tex->v_offset + (float)y * (64.0F * pWork->pic_scale_v) ;
		uvrgbwh0->u0 = uvrgbwh1->u0 = offset_u;
		uvrgbwh0->v0 = uvrgbwh1->v0 = offset_v;
		uvrgbwh0->u1 = uvrgbwh1->u1 = pWork->scale_u + offset_u;
		uvrgbwh0->v1 = uvrgbwh1->v1 = pWork->scale_v + offset_v;
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;
		uvrgbwh0->w  = uvrgbwh1->w  = 0;
		uvrgbwh0->h  = uvrgbwh1->h  = 0;
		uvrgbwh0->a  = uvrgbwh1->a  = 0;
		uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = 
		uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_SMOKE;
		pos++;
		unit++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}
	_MemCopy( prim->pos[ 0 ], MEM_ADDR1, sizeof(FVECTOR), N_VERTS * tmp );
	_MemCopy( prim->pos[ 1 ], MEM_ADDR1, sizeof(FVECTOR), N_VERTS * tmp );

	return 1;
}

#define STR_SMOKE (16238439) // smoke_msk 
/*----------------------------------------------------------------*/
static int GetResources( Work *pWork )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	/* 煙 */
	tex = pWork->tex = DG_GetTexture( STR_SMOKE );
  // "chi01_msk" 
	prim = pWork->prim = GM_MakePrim2( SMOKE_FLAG , pWork->num , N_VERTS );
	if( prim == NULL ){
		pWork->unit = NULL;
		return ( -1 );
	}
	if ( InitPacket( pWork, prim, tex ) < 0 ){
		return ( -1 );
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSmoke( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int mode )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );

		DG_COPY_MAT( &pWork->world, world );
		DG_COPY_VEC( &pWork->shift, shift );
		DG_COPY_VEC( &pWork->power , pPower );
		pWork->range = *range;
		pWork->life = life;
		pWork->size_rnd = size_rnd;
		pWork->num = num;
		pWork->alpha = alpha;
		switch ( mode ){
		case SK_MODE_DEMO :
			pWork->act = ( void * )ConvectionAct;
			break;
		default : 
			pWork->act = ( void * )NormalAct;
			break;
		}

      // HACK FOR w32a - LESS SMOKE
      if ( gAS_DG_HackArea == kDGHA_w32a )
      {
         pWork->num /= 2;
         pWork->life /= 4;
      }

		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewSmokeNormal( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha )
{
	return ( NewSmoke( num , world , shift , pPower , life , size_rnd , range , alpha , SK_MODE_NORMAL ) );
}

void *NewSmokeDemo( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha )
{
	return ( NewSmoke( num , world , shift , pPower , life , size_rnd , range , alpha , SK_MODE_DEMO ) );
}
