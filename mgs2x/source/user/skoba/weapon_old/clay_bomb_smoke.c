//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	smoke.c
	ポインタ参照先に接続する力積付き煙 
	2001/03/28 S.Kobayashi
	$Id: clay_bomb_smoke.c,v 1.1.1.3 2002/11/19 11:50:33 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"

#include	"../../okajima/etc/ok_util.h"

#define	MULTIPLE	(4.0f)

#define	GRAVI_RAND_LEN	(3.0f)

#define N_PRIMS		(16)
#define N_VERTS		(16)

#define	RGB_SMOKE		(84)
#define	ALPHA_MIN		(24.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE_MIN		(32.0f)
#define	SIZE_RND		(32.0f)

#define	ZOOM_ADD		(4.0f)
#define	ANGLE_ADD		( 1.0f * PI / 360.f)
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
	FVECTOR     hit_point; // Hit する場所 
	char        hit_flag;
} Unit ;

typedef	struct _work {
	GV_ACT_EX		actor ;

	FMATRIX		world;
	FVECTOR		shift;
	FVECTOR     power;
	FVECTOR		before_pos;
	FVECTOR     from;
	FVECTOR     to;

	DG_PRIM2	*prim ;
	DG_TEX      *tex;
	int			life;
	int         num;
	int         mode;
	int         alpha_sub;
	float		size_rnd;
	float       alpha;
	float       patern;
	FVECTOR     range;
	Unit		*unit;

	int         flag;
	int         all_hit;
	FVECTOR     hit_point;  // 最終到達地点 
	FVECTOR     pole; // 反射用法線ベクトル 
	int         hit_sign; // 反射 
	float	   scale_u,scale_v,pic_scale_u,pic_scale_v ;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_MODE_NORMAL = 0 , 
	SK_MODE_GRAY ,
	SK_MODE_FIRE ,
	SK_MODE_FIRE2 ,
	SK_MODE_WHITE ,
};

enum {
	SK_HIT_NON = 0, 
	SK_HIT_ONECE ,
	SK_HIT_CHACK_SKIP ,
};

enum {
	SK_EFFECT_KILL_OK = 0x0,
	SK_EFFECT_KILL_NO = 0x1,
};


extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

extern 	void	PutWallSusu( FVECTOR * );

#define SUSU_SIZE (100.0f)
/*----------------------------------------------------------------*/

extern void *NewTest_Sprite( FVECTOR *pPos , int size ); // 場所をたしかめるのに便利かも 

static void NormalAct( Work *work )
{
	static FVECTOR SK_w_reflect = { 2.0f , 2.0f , 2.0f };//{ 0.5f , 0.3f , 0.5f }; 
	FVECTOR	now_pos;
	FVECTOR	dif_pos;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR wind_tmp;
	FMATRIX	fmtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	float offset_u , offset_v;
  	float rnd_tmp;
	int	x , y , i;
	int       tmp;
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
			if ( unit->hit_flag != SK_HIT_ONECE ){
				_sceVu0AddVector( pos , pos_before , &unit->vec );
				// 風の影響 
				_sceVu0Normalize( &wind_tmp , &G_wind );
				_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
				_sceVu0AddVector( pos , pos , &wind_tmp );
			} else {
				_sceVu0AddVector( pos , pos_before , &unit->vec );
			}
			// hit or non_hit
			{
				FVECTOR to;
				float   size;

				if ( ( unit->hit_flag != SK_HIT_ONECE ) && !( work->flag & SK_HIT_CHACK_SKIP ) ){ // 一回だけ 
					_sceVu0SubVector( &to , &work->hit_point , pos );
					size = to.vx + to.vy + to.vz;
					if ( work->hit_sign < 0 ){
						if ( size <= 0.0f ){
							DG_COPY_VEC( pos , &work->hit_point );
							DG_ReflectVector( &work->pole , &unit->vec, &unit->vec );
							_sceVu0MulVector( &unit->vec , &unit->vec , &SK_w_reflect );
							unit->hit_flag = SK_HIT_ONECE;
							work->all_hit = 1;
						}
					} else {
						if ( size >= 0.0f ){
							DG_COPY_VEC( pos , &work->hit_point );
							DG_ReflectVector( &work->pole , &unit->vec, &unit->vec );
							_sceVu0MulVector( &unit->vec , &unit->vec , &SK_w_reflect );
							unit->hit_flag = SK_HIT_ONECE;
							work->all_hit = 1;
						}
					}
				}
			}

			unit->alpha += ( 0 - unit->alpha ) / ( work->alpha_sub );
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
				rnd_tmp = rnd() * 4.f;
				rnd_tmp *= PI / 360.f;
				unit->angle += ( rnd_tmp * unit->sign );
				if ( unit->angle > 180.f * PI / 360.f ){
					unit->angle = -180.f * PI / 360.f;
				}
				if ( unit->angle < -180.f * PI / 360.f ){
					unit->angle = 180.f * PI / 360.f;
				}
				uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
				uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
				/* パターン */
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

			rnd_tmp = rnd(); 
			rnd_tmp += 1.2f;
			_sceVu0DivVector( &unit->vec , &unit->vec , rnd_tmp );
		} else if( work->life > 0 ){
		  	{
				float tmp2;
				float rnd_tmp;
				SVECTOR rot;


				// 加速度の計算 
				tmp2 = frnd();
				tmp2 = tmp2 < 0 ? -tmp2 + 0.5 : tmp2;
				rnd_tmp = frnd() * 256.f;
				rot.vz = ( short )rnd_tmp;
				rnd_tmp = rnd() * 256.f;
				rot.vy = ( short )rnd_tmp;
				rot.vx = 0;
				DG_SetPos2( &DG_ZeroVector , &rot );
				DG_RotVector( &work->power , &unit->vec , 1 ); 
				// 加速度の計算(正しい方向に修正) 
				DG_SetPos( &work->world );
				DG_RotVector( &unit->vec , &unit->vec , 1 );
				_sceVu0ScaleVector( &unit->vec , &unit->vec , tmp2 );
				DG_COPY_VEC( pos, &now_pos );
				unit->angle  = rnd() * 60.f * PI / 360.f;
				unit->sign   = frnd() * 1.5f;
				unit->size   = SIZE_MIN  + work->size_rnd * rnd();
				unit->alpha  = work->alpha * rnd();
				unit->alpha_sub = 2.0f;
				uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
				uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
				uvrgbwh->a = (int)unit->alpha;
				/* パターン */
				uvrgbwh->pad0 = uvrgbwh->pad1 = irnd() % 30;
				x = uvrgbwh->pad0 % 8;
				y = uvrgbwh->pad0 / 8;
				offset_u = work->tex->u_offset + x * (32.0F * work->pic_scale_u);
				offset_v = work->tex->v_offset + y * (64.0F * work->pic_scale_v);
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
				if ( i != 0 ){
					unit->hit_point = DG_ZeroVector;
					unit->hit_flag = SK_HIT_NON;
					unit++;
					pos++;
					pos_before++;
					uvrgbwh++;
					uvrgbwh_before++;
					continue;
				}
				{
					// 先に計算 
					FVECTOR to;
					HZX_SEG		*seg;
					HZX_FLR		flr[2];
					int			atr[2];
					int tmp_flag;

					_sceVu0ScaleVector( &to , &unit->vec , 2.f );
					_sceVu0AddVector( &to , pos , &to );
					tmp_flag = HZX_OnlineHazardCheck( GM_GetHzxGroupID( GM_CurrentMap ) , pos , &to , HZX_CHK_ALL ,
													  HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE , 0 );
					switch( tmp_flag ){
					case 1 : // 壁 
						HZX_GetOnlinePoint( &work->hit_point );
						// hit sign 出し 
						{
							FVECTOR hit_diff;

							_sceVu0SubVector( &hit_diff , &work->hit_point , pos );
							if ( ( hit_diff.vx + hit_diff.vy + hit_diff.vz ) < 0 ){
								work->hit_sign = 1;
							} else {
								work->hit_sign = -1;
							}
						}
						HZX_GetOnlineHazard( flr, atr ) ;
						seg=(HZX_SEG *)flr;
						work->pole.vx = seg->p2.z - seg->p1.z ;
						work->pole.vy = 0.0f ;
						work->pole.vz = seg->p1.x - seg->p2.x ;
						unit->hit_flag = SK_HIT_NON;
					  	break;
					default :
						work->hit_point = DG_ZeroVector;
						work->flag = SK_HIT_CHACK_SKIP;
					  	break;
					}
				}
			}
		}else{
			count++;
		}
		unit++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	work->life--;
	if( ( count >= tmp ) && ( work->life <= 0 ) ){
		GV_DestroyActor( work );
	}
	work->patern += -work->patern / 20.f;
	if ( work->patern < 0.0f ){
		work->patern = 0.0f;
	}
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
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FMATRIX		fmtemp;
	Unit		*unit;
	int     tmp;
	int		x , y;
	float	offset_u,offset_v;
	int		i , j;

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
	DG_ConfigPrim2Tex( prim, tex );
	switch ( pWork->mode ){ 
	case SK_MODE_NORMAL :
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		prim->raise = RAISE_SMOKE / 2;
		break;
	case SK_MODE_GRAY :
	case SK_MODE_FIRE :
	case SK_MODE_FIRE2 :
	case SK_MODE_WHITE :
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		prim->raise = RAISE_SMOKE;
		break;
	}

	pWork->pic_scale_u = tex->u_scale / 255.0F;
	pWork->pic_scale_v = tex->v_scale / 255.0F;
	pWork->scale_u	 = pWork->pic_scale_u * 31.0F;
	pWork->scale_v	 = pWork->pic_scale_v * 63.0F;
	pWork->patern = 3.0f;
	// flag 
	pWork->flag = 0;
	pWork->all_hit = 0;

	unit       = pWork->unit;
	pos        = MEM_ADDR1 ;
	uvrgbwh   = MEM_ADDR1 ;

	DG_COPY_VEC( &pWork->from , ( FVECTOR * )fmtemp.m[ 3 ] );
	{
		FVECTOR *tmp_pos;
		FVECTOR *tmp_pos_pre;
		int mem_count;
		int trans_count;
		int point;

		mem_count = ( tmp / SCR_FVECTOR_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_pos = prim->pos[ 0 ];
		tmp_pos_pre = prim->pos[ 1 ];
		for ( i = 0 ; i < mem_count ; i++ ){
			for ( j = 0 ; j < SCR_FVECTOR_MAX ; j++ ){
				DG_COPY_VEC( pos , &pWork->before_pos );
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					unit++;
					pos++;
					trans_count++;
					break;
				}
				unit++;
				pos++;
			}
			point = ( ( trans_count - 1 ) % SCR_FVECTOR_MAX ) + 1;
			_MemCopy( tmp_pos, MEM_ADDR1, sizeof( FVECTOR ), point  );
			_MemCopy( tmp_pos_pre, MEM_ADDR1, sizeof( FVECTOR ), point  );
			tmp_pos += point;
			tmp_pos_pre += point;
			pos = MEM_ADDR1;
			trans_count = 0;
		}		
	}
	{
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh_pre;
		int mem_count;
		int trans_count;
		int point;

		mem_count = ( tmp / SCR_UVRGBWH_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 0 ];
		tmp_uvrgbwh_pre = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 1 ];
		for ( i = 0 ; i < mem_count ; i++ ){
			for ( j = 0 ; j < SCR_UVRGBWH_MAX ; j ++ ){
				uvrgbwh->pad0 = irnd() % 30;
				x = uvrgbwh->pad0 % 8 ;
				y = uvrgbwh->pad0 / 8 ;
				offset_u = pWork->tex->u_offset + (float)x * (32.0F * pWork->pic_scale_u) ;
				offset_v = pWork->tex->v_offset + (float)y * (64.0F * pWork->pic_scale_v) ;
				uvrgbwh->u0 = offset_u;
				uvrgbwh->v0 = offset_v;
				uvrgbwh->u1 = pWork->scale_u + offset_u;
				uvrgbwh->v1 = pWork->scale_v + offset_v;
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->w  = 0;
				uvrgbwh->h  = 0;
				uvrgbwh->a  = 0;
				switch ( pWork->mode ){ 
				case SK_MODE_NORMAL :
				case SK_MODE_GRAY :
				case SK_MODE_FIRE :
				case SK_MODE_FIRE2 :
					uvrgbwh->r = 118;
					uvrgbwh->g = 75;
					uvrgbwh->b = 45;
					break;
				case SK_MODE_WHITE :
					uvrgbwh->r = uvrgbwh->g = uvrgbwh->b = RGB_SMOKE;
					break;
				}
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					uvrgbwh++;
					trans_count++;
					break;
				}
				uvrgbwh++;
			}
			point = ( ( trans_count - 1 ) % SCR_UVRGBWH_MAX ) + 1;
			_MemCopy( tmp_uvrgbwh, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );
			_MemCopy( tmp_uvrgbwh_pre, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );
			tmp_uvrgbwh += point;
			tmp_uvrgbwh_pre += point;
			uvrgbwh = MEM_ADDR1;
			trans_count = 0;
		}
	}

	return 0;
}

#define STR_SMOKE (16238439) // smoke_msk
#define STR_FIRE (5275610) // bombpowder6_msk.pic 
#define STR_FIRE2 (6324186) // bombpowder4_msk.pic  
/*----------------------------------------------------------------*/

static int GetResources( Work *pWork )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	/* 煙 */
	switch( pWork->mode ){
	case SK_MODE_NORMAL :
	case SK_MODE_GRAY :
	case SK_MODE_WHITE :
		tex = pWork->tex = DG_GetTexture( STR_SMOKE );
		break;
	case SK_MODE_FIRE :
		tex = pWork->tex = DG_GetTexture( STR_FIRE );
		break;
	case SK_MODE_FIRE2 :
		tex = pWork->tex = DG_GetTexture( STR_FIRE2 );
		break;
	default :
		tex = NULL;
		break;
	}
	prim = pWork->prim = GM_MakePrim2( SMOKE_FLAG , pWork->num , N_VERTS );
	if(prim==NULL){
		pWork->unit = NULL;
		return ( -1 );
	}
	if ( InitPacket( pWork, prim, tex ) < 0 ){
		return ( -1 );
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewClaySmoke( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub , int mode )
{
	Work		*pWork ;

	if ( num <= 0 ){
		return NULL;
	}
	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->world = *world;
		DG_COPY_VEC( &pWork->shift, shift );
		DG_COPY_VEC( &pWork->power , pPower );
		pWork->range = *range;
		pWork->life = life;
		pWork->size_rnd = size_rnd;
		pWork->num = num;
		pWork->alpha = alpha;
		pWork->alpha_sub = alpha_sub;
		pWork->mode = mode;
		pWork->act = ( void * )NormalAct;

		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

// saver
void *NewClaySmokeNormal( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub )
{
	return ( NewClaySmoke( num , world , shift , pPower , life , size_rnd , range , alpha , alpha_sub , SK_MODE_NORMAL ) );
}

void *NewClaySmokeGray( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub)
{
	return ( NewClaySmoke( num , world , shift , pPower , life , size_rnd , range , alpha , alpha_sub , SK_MODE_GRAY ) );
}

void *NewClaySmokeFire( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub )
{
	return ( NewClaySmoke( num , world , shift , pPower , life , size_rnd , range , alpha , alpha_sub , SK_MODE_FIRE ) );
}

void *NewClaySmokeFire2( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub )
{
	return ( NewClaySmoke( num , world , shift , pPower , life , size_rnd , range , alpha , alpha_sub , SK_MODE_FIRE2 ) );
}

void *NewClaySmokeWhite( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , FVECTOR *range , float alpha , int alpha_sub )
{
	return ( NewClaySmoke( num , world , shift , pPower , life , size_rnd , range , alpha , alpha_sub , SK_MODE_WHITE ) );
}

