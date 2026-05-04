//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	冷却エフェクト  main.c
	2001/03/20 S.Kobayashi
	$Id: c4_ice.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "etc.h"

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define N_PRIMS (512)
#define N_VERTS	(16)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define SPRITE_FLAG (DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA)
#define ALPHA (128)
#define LIFE   (60)
#define ICE_MAX (60)
#define UNLIMIT (0x7fff)
#define ICE_PUT_TEX (9868771)
#define ICE_DUST    (15638496)
#define ICE_LOST    (6684577)
#define WIND_MAX     (2)
#define SPRITE_PLUS ( 4 )

#define	SIZE_MIN		(4.0f)
#define	SIZE_RND		(16.0f)

#define	ZOOM_ADD		(38.0f)
#define	ANGLE_ADD		(rnd() * 0.4f * PI / 360.f)
#define	RAISE_SMOKE			(1000)
#define SMOKE_FLAG  ( DG_PRIM2_RSPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )

#define SK_R (128)
#define SK_G (140)
#define SK_B (148)

enum {
	LIFE_OFF = 0 ,
	LIFE_ON
};

enum {
	ICE_SPRAY = 0 ,
	ICE_PUT
};

enum {
	SK_EFFECT_NORMAL  = 0x1,
	SK_EFFECT_ADD     = 0x2,
	SK_EFFECT_KILL_OK = 0x4,
	SK_EFFECT_KILL_NO = 0x8,
	SK_EFFECT_DESTROY = 0x10,
	SK_EFFECT_HOKAN   = 0x20,
	SK_EFFECT_ADD_END  = 0x40,
};

typedef struct {
	FVECTOR speed;
	FVECTOR rspeed; // 乱気流 
	float	angle;
	float	size;
	float	alpha;
	float   sign; // 回転の方向
	char    flag;
} Unit ;	

typedef	struct _work {
	GV_ACT_EX	actor;

	DG_PRIM2	*prim;
	DG_TEX      *tex;
	
	FVECTOR       pos;	
	FVECTOR      wind;
	FMATRIX   *pWorld;
	FVECTOR     shift;
	FVECTOR      from;
	FVECTOR        to;
	Unit        *unit;
	float       alpha; // 基礎
	int          time;
	int		      map;
	int			count;
	int          *num;
	int        invers; // 反転フラグ
	int max_speed_num;
	char    stop_flag;	
    u_int       *pTrg;
	char        *flag; // good by flag 
	void ( *act )(struct _work *);
} Work;

typedef struct {
	int *num;
	int strcode;
	int alpha;
} SK_TEX_INFO;

extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

static void SpeedIn( FVECTOR *pVec , FMATRIX *pWorld )
{
	static FVECTOR speed = { 0.0f , -60.0f , 0.0f , 0.0f };
	SVECTOR rot;

	rot.vx = irnd() % 128 - 64;
	rot.vy = -irnd() % 128;
	rot.vz = 0;
	DG_SetPos2( &DG_ZeroVector , &rot );
	DG_RotVector( &speed , pVec , 1 );
	DG_SetPos( pWorld );
	DG_RotVector( pVec , pVec , 1 );
}

static void rSpeedIn( FMATRIX *pMat , FVECTOR *pVec , float rand_num )
{
	FVECTOR ftmp;
	float    tmp;

	tmp = frnd() * rand_num;
	ftmp.vx = pMat->m[ 1 ][ 0 ] * tmp;
	ftmp.vy = pMat->m[ 1 ][ 1 ] * tmp;
	ftmp.vz = pMat->m[ 1 ][ 2 ] * tmp;

	*pVec = DG_ZeroVector;
}

static int KillSearch( Work *pWork ) // 使用可能な新しい領域の確保？ err - 0xffff
{
	int i;

	for ( i = 0 ; i < 6 * N_VERTS ; i ++ ){
		if ( pWork->unit[ i ].flag & SK_EFFECT_KILL_OK ){
			return ( i );
		}
	}
	return ( 0xffff );
}

static void StoreKillList( Work *pWork , int point )
{
	pWork->unit[ point ].flag = SK_EFFECT_KILL_OK | SK_EFFECT_ADD_END;
}

static void hokan( Work *pWork ) // 補完?
{
 	FVECTOR pos;
	FVECTOR speed;
	FVECTOR *tmp_pos;
	FVECTOR from , to;
	DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
	Unit *tmp_unit;
	u_short u_tmp[ 2 ] , v_tmp[ 2 ];
	int i;
	int point;
	int clock;

	DG_COPY_VEC( &from , &pWork->from );
	DG_COPY_VEC( &to , &pWork->to );

	clock = pWork->prim->buffer_clock;
	_sceVu0SubVector( &speed , &to , &from );
	_sceVu0DivVector( &speed , &speed , ( float )SPRITE_PLUS ); // 十枚
	DG_COPY_VEC( &pos , &from );
	u_tmp[ 0 ] = FTOI12( 0.0F * pWork->tex->u_scale + pWork->tex->u_offset ); // 左上
	v_tmp[ 0 ] = FTOI12( 0.0F * pWork->tex->v_scale + pWork->tex->v_offset );// 左上
	u_tmp[ 1 ] = FTOI12( 1.0F * pWork->tex->u_scale + pWork->tex->u_offset );// 右下
	v_tmp[ 1 ] = FTOI12( 1.0F * pWork->tex->v_scale + pWork->tex->v_offset );// 右下
	for( i = 0 ; i < SPRITE_PLUS ; i ++ ){
		// 追加
		point = KillSearch( pWork );
		if ( point == 0xffff ){
			return;
		}
		tmp_pos = pWork->prim->pos[ clock ];
		tmp_pos += point;
		DG_COPY_VEC( tmp_pos , &pos );
		tmp_pos = pWork->prim->pos[ 1 - clock ];
		tmp_pos += point;
		DG_COPY_VEC( tmp_pos , &pos );
		_sceVu0AddVector( &pos , &pos , &speed );


		tmp_unit = &pWork->unit[ point ];
		tmp_unit->flag = SK_EFFECT_NORMAL | SK_EFFECT_HOKAN | SK_EFFECT_ADD_END;
		tmp_unit->angle  = frnd() * 60.f * PI / 360.f;
		tmp_unit->sign   = frnd() * 1.5f;
		tmp_unit->size   = 20.0f * rnd() + 12.0f;
		tmp_unit->alpha  = pWork->alpha * rnd();
		SpeedIn( &tmp_unit->speed , pWork->pWorld );

		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )pWork->prim->uvrgb[ clock ];
		tmp_uvrgbwh += point;
		tmp_uvrgbwh->u0 = u_tmp[ 0 ];
		tmp_uvrgbwh->v0 = v_tmp[ 0 ];
		tmp_uvrgbwh->u1 = u_tmp[ 1 ];
		tmp_uvrgbwh->v1 = v_tmp[ 1 ];
		tmp_uvrgbwh->q0 = 4096 ;
		tmp_uvrgbwh->q1 = 4096 ;
		tmp_uvrgbwh->f0 = 0x0fff ;
		tmp_uvrgbwh->f1 = 0x0fff ;
		tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
		tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));
		tmp_uvrgbwh->a = (int)tmp_unit->alpha;
		tmp_uvrgbwh->r = SK_R;
		tmp_uvrgbwh->g = SK_G;
		tmp_uvrgbwh->b = SK_B;

		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )pWork->prim->uvrgb[ 1 - clock ];
		tmp_uvrgbwh += point;
		tmp_uvrgbwh->u0 = u_tmp[ 0 ];
		tmp_uvrgbwh->v0 = v_tmp[ 0 ];
		tmp_uvrgbwh->u1 = u_tmp[ 1 ];
		tmp_uvrgbwh->v1 = v_tmp[ 1 ];
		tmp_uvrgbwh->q0 = 4096 ;
		tmp_uvrgbwh->q1 = 4096 ;
		tmp_uvrgbwh->f0 = 0x0fff ;
		tmp_uvrgbwh->f1 = 0x0fff ;
		tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
		tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));
		tmp_uvrgbwh->a = (int)tmp_unit->alpha;
		tmp_uvrgbwh->r = SK_R;
		tmp_uvrgbwh->g = SK_G;
		tmp_uvrgbwh->b = SK_B;
	}
}

static void EffectAdd( Work *pWork )
{
	int point;
	u_short u_tmp[ 2 ] , v_tmp[ 2 ];
	int clock;
	FMATRIX fmtemp;

	DG_SetPos( pWork->pWorld );
	DG_MovePos( &pWork->shift );
	DG_GetPos( &fmtemp );

	point = KillSearch( pWork );
	clock = pWork->prim->buffer_clock;
	u_tmp[ 0 ] = FTOI12( 0.0F * pWork->tex->u_scale + pWork->tex->u_offset );// 左上
	v_tmp[ 0 ] = FTOI12( 0.0F * pWork->tex->v_scale + pWork->tex->v_offset );// 左上
	u_tmp[ 1 ] = FTOI12( 1.0F * pWork->tex->u_scale + pWork->tex->u_offset );// 右下
	v_tmp[ 1 ] = FTOI12( 1.0F * pWork->tex->v_scale + pWork->tex->v_offset );// 右下
	if( point != 0xffff ){ // 生成
		{
			FVECTOR *tmp_pos;
			DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
			Unit *tmp_unit;

			tmp_pos = pWork->prim->pos[ clock ];
			tmp_pos += point;
			DG_COPY_VEC( tmp_pos , (FVECTOR *)fmtemp.m[ 3 ] ); 
			tmp_pos = pWork->prim->pos[ 1 - clock ];
			tmp_pos += point;
			DG_COPY_VEC( tmp_pos , (FVECTOR *)fmtemp.m[ 3 ] ); 

			tmp_unit = &pWork->unit[ point ];
			tmp_unit->flag = SK_EFFECT_NORMAL;
			tmp_unit->angle  = frnd() * 60.f * PI / 360.f;
			tmp_unit->sign   = frnd() * 1.5f;
			tmp_unit->size   = 40.0f * rnd();
			tmp_unit->alpha  = pWork->alpha * rnd() + ( 22.0f * (*pWork->num) );

			tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )pWork->prim->uvrgb[ clock ];
			tmp_uvrgbwh += point;
			tmp_uvrgbwh->u0 = u_tmp[ 0 ];
			tmp_uvrgbwh->v0 = v_tmp[ 0 ];
			tmp_uvrgbwh->u1 = u_tmp[ 1 ];
			tmp_uvrgbwh->v1 = v_tmp[ 1 ];
			tmp_uvrgbwh->q0 = 4096 ;
			tmp_uvrgbwh->q1 = 4096 ;
			tmp_uvrgbwh->f0 = 0x0fff ;
			tmp_uvrgbwh->f1 = 0x0fff ;
			tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
			tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));

			tmp_uvrgbwh->r = SK_R;
			tmp_uvrgbwh->g = SK_G;
			tmp_uvrgbwh->b = SK_B;
			tmp_uvrgbwh->a = 0;

			tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )pWork->prim->uvrgb[ 1 - clock ];
			tmp_uvrgbwh += point;
			tmp_uvrgbwh->u0 = u_tmp[ 0 ];
			tmp_uvrgbwh->v0 = v_tmp[ 0 ];
			tmp_uvrgbwh->u1 = u_tmp[ 1 ];
			tmp_uvrgbwh->v1 = v_tmp[ 1 ];
			tmp_uvrgbwh->q0 = 4096 ;
			tmp_uvrgbwh->q1 = 4096 ;
			tmp_uvrgbwh->f0 = 0x0fff ;
			tmp_uvrgbwh->f1 = 0x0fff ;
			tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
			tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));

			tmp_uvrgbwh->r = SK_R;
			tmp_uvrgbwh->g = SK_G;
			tmp_uvrgbwh->b = SK_B;
			tmp_uvrgbwh->a = 0;

			// speed
			SpeedIn( &tmp_unit->speed , pWork->pWorld );
//			_sceVu0ScaleVector( &tmp_unit->speed , &tmp_unit->speed , ( float )*pWork->num );
		}
	}
	*pWork->flag = SK_EFFECT_NORMAL;
}

static void Act( Work *pWork )
{
   if( !DG_SwitchBuffPrim2( pWork->prim ) )
   {
      return;
   }
	pWork->act(pWork);
	if ( PL_GetPlayerItem() == IT_Thermal ){
		DG_SetPrim2Alpha( pWork->prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	} else {
		DG_SetPrim2Alpha( pWork->prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
}

static void SprayAct( Work *pWork )
{
	static FVECTOR w_reflect = { 0.0f , 0.0f , 0.0f };
	static FVECTOR f_reflect = { 0.0f , 0.0f , 0.0f };

	DG_PRIM2_UVRGBWH *uvrgbwh , *pre_uvrgbwh;
	FVECTOR      *pos;
	FVECTOR  *pre_pos;
   FVECTOR speed_tmp = { 0 };
	Unit        *unit;
	int         clock;
	int             i;
	int     tmp_count;
	int          ntmp;

	clock = pWork->prim->buffer_clock;
	pos = pWork->prim->pos[ clock ];
	pre_pos = pWork->prim->pos[ 1 - clock ];
	uvrgbwh = pWork->prim->uvrgb[ clock ];
	pre_uvrgbwh = pWork->prim->uvrgb[ 1 - clock ];
	unit = pWork->unit;

	tmp_count = 0;
	ntmp = 6 * N_VERTS;
	for ( i = 0 ; i < ntmp ; i++ ){      // 移動
		if ( unit->alpha <= 0.0f || unit->flag & SK_EFFECT_KILL_OK || !( unit->flag & SK_EFFECT_NORMAL ) ){ // 何もしない
		  	// etc
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			StoreKillList( pWork , i );
			// count
			tmp_count++;
			// pointer
			pos++;
			uvrgbwh++;
			pre_pos++;
			pre_uvrgbwh++;
			unit++;
			continue;
		}
		if ( ! ( unit->flag & SK_EFFECT_HOKAN ) ){
			if ( pWork->stop_flag == 0 ){
				DG_COPY_VEC( &speed_tmp , &unit->speed ); 
				if ( pWork->time > DIRECT_TICK( 300 ) ){            // 乱気流発動
					_sceVu0AddVector( &speed_tmp , &speed_tmp , &unit->rspeed );
					pWork->time += 1;
#if 0
				} else {
					{
						FVECTOR fvtmp;
						// ステージ風
						_sceVu0Normalize( &fvtmp , &G_wind ); 
	//					_sceVu0ScaleVector( &fvtmp , &fvtmp , WIND_MAX );
						_sceVu0AddVector( &speed_tmp , &speed_tmp , &fvtmp );
						DG_COPY_VEC( &pWork->wind , &fvtmp ); 
					}
#endif
				}
			}
//		if ( i == pWork->max_speed_num && pWork->stop_flag == 0 ){ // 一番スピードの早い奴だけ
//				_sceVu0AddVector( pos , pre_pos , &speed_tmp );
			pWork->stop_flag = SK_StepCheckHzd3( pos , pre_pos , &speed_tmp , 0.2 , 64  , GM_GetHzxGroupID( pWork->map ) , &f_reflect , &w_reflect );
			DG_COPY_VEC( &pWork->from , pre_pos );
			_sceVu0AddVector( pos , pre_pos , &speed_tmp );
			DG_COPY_VEC( &pWork->to , pos );
			if ( pWork->stop_flag == 0 ){
				if ( !( unit->flag & SK_EFFECT_ADD_END ) ){
					hokan( pWork );
					unit->flag |= SK_EFFECT_ADD_END;
				}
			}
		} else {
			_sceVu0AddVector( pos , pre_pos , &unit->speed );
		}
		// スピードの遅いものは拡大率を落す
		uvrgbwh->w = (int)( unit->size * cosf( unit->angle ) );
		uvrgbwh->h = (int)( unit->size * sinf( unit->angle ) );
		unit->size += ZOOM_ADD;
		unit->alpha += ( 0 - unit->alpha ) / ( pWork->alpha + 2.0f );
	    if ( unit->alpha < 1 ){
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			uvrgbwh->a = 0;
			unit->alpha = 0.0f;
			StoreKillList( pWork , i );
			tmp_count++;
		} else {
			uvrgbwh->a = ( u_short )unit->alpha;
			unit->angle += ( 8.0f * PI / 360.f ) * unit->sign;
			if ( unit->angle > 180.f * PI / 360.f ){
				unit->angle = -180.f * PI / 360.f;
			}
			if ( unit->angle < -180.f * PI / 360.f ){
				unit->angle = 180.f * PI / 360.f;
			}
		}
		pos++;
		uvrgbwh++;
		pre_pos++;
		pre_uvrgbwh++;
		unit++;
	}
	if ( pWork->actor.actor.class & GV_CLASS_CHILD &&
		 pWork->flag != NULL && *pWork->flag & SK_EFFECT_ADD && 
		 pWork->pTrg != NULL && *pWork->pTrg == TRIG_FIRE ){ // トリガーを押されていたら追加
		EffectAdd( pWork );
	}
	if ( ( ( pWork->flag != NULL && *pWork->flag & SK_EFFECT_DESTROY ) || !( pWork->actor.actor.class & GV_CLASS_CHILD ) )
		 && ntmp >= tmp_count ){
	    GV_DestroyActor( pWork );
	} else {
		pWork->time += 1;
	}
}

static void Die( Work *pWork )
{
	if ( pWork->unit != NULL ){
		GV_DelayedFree( pWork->unit );
	}
	if ( pWork->prim != NULL ){
		GM_FreePrim2( pWork->prim );
	}
}

static int GetResources( Work *pWork , SK_TEX_INFO *pTex )
{
	DG_TEX	             *tex;
	DG_PRIM2	        *prim;
	DG_PRIM2_UVRGBWH *uvrgbwh;
	FVECTOR			     *pos;
	Unit	            *unit;
	float           max_speed;
	int                   tmp;
	int				 	 i, j;
	
	pWork->map = GM_CurrentMap;
	GM_CurrentMap = GM_CurrentMap;

	max_speed = 0;
	pWork->stop_flag = 0x0;
	pWork->num = pTex->num;
	pWork->alpha = pTex->alpha;
	pWork->time = 0.F;
	pWork->wind = DG_ZeroVector;
	pWork->from = pWork->to = DG_ZeroVector;
	tmp = 6 * N_VERTS;

	pWork->unit = GV_Malloc( ( sizeof( Unit ) * tmp ) ); // 確保
	if ( pWork->unit == NULL ){
		pWork->prim = NULL;
		return ( -1 );
	}
	GV_ZeroMemory( pWork->unit , ( sizeof( Unit ) * tmp ) );

	/* テクスチャ取得 */
	tex = pWork->tex = DG_GetTexture( pTex->strcode );
	prim = pWork->prim = GM_MakePrim2( SPRITE_FLAG , 6 , N_VERTS );
	if ( prim == NULL ){
		return ( -1 );
	}
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pWork->act = (void *)SprayAct;                      // スプレイact登録

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	unit = pWork->unit;
	pos       = MEM_ADDR1 ;
	uvrgbwh   = MEM_ADDR1 ;
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
		for ( j = 0 ; j < tmp ; j++ ){
			DG_COPY_VEC( pos , (FVECTOR *)pWork->pWorld->m[ 3 ] );
			unit->angle = ( ( float )( irnd() % 20 ) * PI / 360.f );
			// 加速度の割だし
			SpeedIn( &unit->speed , pWork->pWorld );
			if ( max_speed < unit->speed.vy ){
				max_speed = unit->speed.vy;
				pWork->max_speed_num = j;
			}
			rSpeedIn( pWork->pWorld , &unit->rspeed , 0.2f );
			unit->sign = frnd();
			if ( trans_count < tmp ){
				trans_count++;
			} else {
				pos++;
				unit++;
				break;
			}
			pos++;
			unit++;
		}
#if 0
		// 複数回転送をかんがえて
		point = ( ( trans_count - 1 ) % SCR_FVECTOR_MAX ) + 1;
		_MemCopy( tmp_pos, MEM_ADDR1, sizeof(FVECTOR), point  );
		_MemCopy( tmp_pos_pre, MEM_ADDR1, sizeof(FVECTOR), point  );
		tmp_pos += point;
		tmp_pos_pre += point;
#else 
		/* 複数回転送必要ないため変更 2002.09.05 YANO */
		_MemCopy( tmp_pos, MEM_ADDR1, sizeof(FVECTOR), trans_count );
		_MemCopy( tmp_pos_pre, MEM_ADDR1, sizeof(FVECTOR), trans_count );
		tmp_pos += trans_count;
		tmp_pos_pre += trans_count;
#endif
		pos = MEM_ADDR1;
	}
	{
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh_pre;
		int mem_count;
		int trans_count;
		int point;
		u_short u_tmp[ 2 ] , v_tmp[ 2 ];

		mem_count = ( tmp / SCR_UVRGBWH_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 0 ];
		tmp_uvrgbwh_pre = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 1 ];
		u_tmp[ 0 ] = FTOI12( 0.0F * tex->u_scale + tex->u_offset ); // 左上
		v_tmp[ 0 ] = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;// 左上
		u_tmp[ 1 ] = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;// 右下
		v_tmp[ 1 ] = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;// 右下
		for ( j = 0 ; j < tmp ; j ++ ){
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			uvrgbwh->u0 = u_tmp[ 0 ];
			uvrgbwh->v0 = v_tmp[ 0 ];
			uvrgbwh->u1 = u_tmp[ 1 ];
			uvrgbwh->v1 = v_tmp[ 1 ];
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			uvrgbwh->r = SK_R;
			uvrgbwh->g = SK_G;
			uvrgbwh->b = SK_B;
			uvrgbwh->a = 0;
			if ( trans_count < tmp ){
				trans_count++;
			} else {
				uvrgbwh++;
				break;
			}
			uvrgbwh++;
		}
#if 0
		point = ( ( trans_count - 1 ) % SCR_UVRGBWH_MAX );
		_MemCopy( tmp_uvrgbwh, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );
		_MemCopy( tmp_uvrgbwh_pre, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );

		tmp_uvrgbwh += point;
		tmp_uvrgbwh_pre += point;
#else
		/* 複数回転送必要ないため変更 2002.09.05 YANO */
		_MemCopy( tmp_uvrgbwh, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), trans_count );
		_MemCopy( tmp_uvrgbwh_pre, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), trans_count );

		tmp_uvrgbwh += trans_count;
		tmp_uvrgbwh_pre += trans_count;
#endif
		uvrgbwh = MEM_ADDR1;
	}
	for ( i = 0 ; i < tmp ; i++ ){
		// KillList Initialize
		StoreKillList( pWork , i );
	}

	return (0);
}

static void *NewIceSpray( FMATRIX *pWorld , FVECTOR *pShift , SK_TEX_INFO *pTex , u_int *pTrg , char *pFlag )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pWorld = pWorld; 
		DG_COPY_VEC( &pWork->shift , pShift );
		pWork->pTrg = pTrg; // トリガー
		pWork->flag = pFlag;
		if ( GetResources( pWork , pTex ) < 0 ) {
			GV_DestroyActor( pWork );
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewC4_Ice( FMATRIX *pWorld , FVECTOR *pShift , SK_TEX_INFO *pTex , u_int *pTrg , char *pFlag )
{
    return NewIceSpray( pWorld , pShift , pTex , pTrg , pFlag );
}

