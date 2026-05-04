//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	clay_eff.c
	クレイモア爆発（内容物エフェクト）
	2001/05/03 S.Kobayashi
	$Id: clay_eff.c,v 1.1.1.3 2002/11/19 11:50:27 Yoshizawa1 Exp $

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"
#include    "../test/etc.h"



#define N_PRIMS		(16)
#define N_VERTS		(32)

#define	RAISE_SMOKE			(1000)
#define SMOKE_FLAG  ( DG_PRIM2_RSPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )
#define POLY_FLAG   ( DG_PRIM2_POLY | DG_PRIM2_TEX | DG_PRIM2_SHADE | DG_PRIM2_ALPHA )
#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD)
#define	TARGET_CLASS2	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN)

//
#define WIND_MAX ( 5.f )
/*----------------------------------------------------------------*/
typedef	struct	{
	FVECTOR		pos; // 場所
	SVECTOR     rot; // 回転角度
	FVECTOR     speed; // スピード
	float		alpha;
	float       patern;
	int			count;
	int         offset; // DG_PRIMのオフセット
	int         position; // DG_PRIMの場所
	int         max_position; // DG_PRIMの天井
	char        non_target; // ターゲットを使わない
} Unit ; // グループの先頭?

typedef	struct _work {
	GV_ACT_EX		actor ;
	DG_PRIM2	    *prim ;
	DG_TEX          *tex;
	DG_PRIM2_UVRGB	*uvrgb ;

	FMATRIX		world;
	FVECTOR		shift;
	FVECTOR     power;
	TARGET      target; // 当たり用
    POWER_TARGET	power_target ;
	FVECTOR     target_pos;
	FVECTOR     target_size;
	FVECTOR     target_trans;

	int			life;
	int         alpha; // ベース
	int         num;
	int         time;
	int         all_hit;
	int         target_time;
	
	long64      weapon_flag;
	Unit		*unit;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_TARGET_ON = 0,
	SK_TARGET_OFF = 0x1,
};


// 外部変数
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

extern void *NewSpark2( int , FMATRIX * , float , float	, float , SVECTOR * , SVECTOR * , FVECTOR * , float , int );

/*----------------------------------------------------------------*/
/* コールバック */
static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*pWork ;
	FVECTOR		dc ;

	pWork = ( Work * )ptr ;

	GM_TargetGetCenter( &dc, def ) ;
	if ( HZX_OnlineHazardCheck( GM_GetHzxGroupID( off->map ), &off->center, &dc,
							    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE, 
							    HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) ) {
		GM_TargetHitCancel( off, def ) ;
	}
}

static	FVECTOR target_pos = { -1800.0f , 0.0f , 800.0f , 1.0f };
static	void	InitTarget( Work *pWork, float size, u_int damage )
{
	TARGET		 *t ;
	POWER_TARGET *p ;
	FVECTOR		 vec ;
	FVECTOR      hit_point;
	FMATRIX      fmtmp;
	int          tmp_flag;

	t = &( pWork->target ) ;
	p = &( pWork->power_target ) ;
	vec.vx = -1000.0f;
	vec.vy =     0.0f;
	vec.vz =     0.0f;//( float )size ;

	// online target
	DG_SetPos( &pWork->world );
	DG_MovePos( &target_pos ) ;
	DG_MovePos( &vec ) ;
	DG_GetPos( &fmtmp );
	tmp_flag = HZX_OnlineHazardCheck( GM_GetHzxGroupID( GM_CurrentMap ) , ( FVECTOR * )pWork->world.m[ 3 ]  , ( FVECTOR * )fmtmp.m[ 3 ] ,
									  HZX_CHK_ALL ,
									  HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE , 0 );
//									  HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ;
	switch( tmp_flag ){
	case 1 : // 壁
		HZX_GetOnlinePoint( &hit_point );
		_sceVu0SubVector( &vec , &hit_point , ( FVECTOR * )pWork->world.m [ 3 ] ); // 直径
		_sceVu0DivVector( &vec , &vec , 2.0f ); // 半径
		// 符号はずし
		vec.vx = DG_FABS( vec.vx );
		vec.vy = DG_FABS( vec.vy );
		vec.vz = DG_FABS( vec.vz );
		// 大きさに変換
		pWork->target_size.vx = fpu_Sqrt( _sceVu0InnerProduct( &vec , &vec ) );
		pWork->target_size.vx = pWork->target_size.vx < 1 ? 1 : pWork->target_size.vx;
		pWork->target_size.vy = 600.0f;
		pWork->target_size.vz = 800.0f;
		pWork->target_trans.vx = pWork->target_size.vx > 0 ? -pWork->target_size.vx - 300.0f : pWork->target_size.vx - 300.0f;
		pWork->target_trans.vy = 0;
		pWork->target_trans.vz = 800;
	  	break;
	default :
		pWork->target_size.vx = 1000.0f;
		pWork->target_size.vy = 600.0f;
		pWork->target_size.vz = 800.0f;
		pWork->target_trans.vx = pWork->target_size.vx > 0 ? -pWork->target_size.vx - 300.0f : pWork->target_size.vx - 300.0f;
		pWork->target_trans.vy = 0;
		pWork->target_trans.vz = 800;
	  	break;
	}

	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE | TARGET_POWER , GM_CurrentStageMap, BOTH_SIDE , &pWork->target_size , &DG_ZeroVector ) ;

	GM_SetTargetWeaponType( t, ( I64(1) << WP_Claymore ) | ( pWork->weapon_flag ) ) ;

	// 力の調整
	_sceVu0DivVector( &vec , &pWork->unit[ 0 ].speed , 32.0f );
	GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, damage, &vec );
	GM_PutTarget( t ) ;
	GM_SetTargetCallBack( t, Hit, pWork );
	pWork->target_time = 0;
}

static void UvAnimePos( Work *pWork , DG_PRIM2_UVRGB *uvrgb , DG_PRIM2_UVRGB *pre_uvrgb , int position , int point ) // UVAnimetion
{
	float uv_pos;
	int   tmp;

	uv_pos = ( float )( position % 2 );
	( uvrgb + position )->u = FTOI12( uv_pos * pWork->tex->u_scale + pWork->tex->u_offset ) ;
	( pre_uvrgb + position )->u = FTOI12( uv_pos * pWork->tex->u_scale + pWork->tex->u_offset ) ;
	uv_pos = ( float )( ( ( float )position  - ( float )pWork->unit[ point ].offset ) / (( float )pWork->unit[ point ].max_position - ( float )pWork->unit[ point ].offset ) );
	tmp = ( int )(uv_pos * 10.0f);
	tmp %= 4;
	uv_pos = (( float )tmp * (10.0f / 4.0f)) / 10.f;
	( uvrgb + position )->v = FTOI12( uv_pos * pWork->tex->v_scale + pWork->tex->v_offset ) ;
	( pre_uvrgb + position )->v = FTOI12( uv_pos * pWork->tex->v_scale + pWork->tex->v_offset ) ;
}

static void AlphaMng( Work *pWork , int point )
{
	DG_PRIM2_UVRGB	*uvrgb , *pre_uvrgb;	// イロカエ用
	float alpha , bak_alpha;
	int clock;
	int position;
	int rag;
	int i;

	clock     = pWork->prim->buffer_clock;
	uvrgb     = pWork->prim->uvrgb[ clock ];
	pre_uvrgb = pWork->prim->uvrgb[ 1 - clock ];

	// 元αの計算
	pWork->unit[ point ].alpha = pWork->unit[ point ].alpha - ( pWork->unit[ point ].alpha / 10.0f ); 
	bak_alpha = alpha = pWork->unit[ point ].alpha = pWork->unit[ point ].alpha < 2.0f ? 0.0f : pWork->unit[ point ].alpha;
	position = pWork->unit[ point ].position;
	rag = 0;
	for ( i = 0 ; i < pWork->unit[ point ].max_position - pWork->unit[ point ].offset ; i++ ){
		// 入力
		( uvrgb + position )->a = alpha;
		( pre_uvrgb + position )->a = alpha;
		position = position + 1 >= pWork->unit[ point ].max_position ? pWork->unit[ point ].offset : position + 1 ;
		if ( rag == 0 ){ // 通常
			alpha /= 2;
		}
		if ( rag > 1 ){ // 元に戻す
			alpha = bak_alpha;
			rag = 0;
		}
		if ( !( i % 3 ) ){ // 切る
			alpha = 0.0f;
		}
		if ( alpha <= 0.0f ){ // モード中
			rag++;
		}
	}
}

static void NormalAct( Work *pWork )
{
	static FVECTOR SK_f_reflect = { 1.0f , -0.5f , 1.0f , 1.0f };
	static FVECTOR SK_w_reflect = { 0.5f , 0.4f , 0.5f , 1.0f };
	FVECTOR *pos , *pre_pos;
	DG_PRIM2_UVRGB	*uvrgb , *pre_uvrgb;	// イロカエ用
	SVECTOR         rot;
	FVECTOR       speed;
	int position;
	int clock;
	int count;
	int i;

	DG_VisiblePrim2( pWork->prim ) ;
	GM_GroupPrim2( pWork->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( pWork->prim ) )
   {
      return;
   }
	clock = pWork->prim->buffer_clock;
	pos       = pWork->prim->pos[ clock ];
	pre_pos   = pWork->prim->pos[ 1 - clock ];
	uvrgb     = pWork->prim->uvrgb[ clock ];
	pre_uvrgb = pWork->prim->uvrgb[ 1 - clock ];

	count = 0;
	for ( i = 0 ; i < pWork->num ; i++ ){
		if ( pWork->unit[ i ].alpha <= 0 ){
			count++;
			continue;
		}
		{
			FVECTOR ftmp_speed;

			ftmp_speed = DG_ZeroVector;
			ftmp_speed.vz = P_GRAVITY;
			// 計算
			DG_SetPos( &pWork->world );
			DG_RotVector( &ftmp_speed , &ftmp_speed , 1 ); 
			_sceVu0AddVector( &pWork->unit[ i ].speed , &pWork->unit[ i ].speed , &ftmp_speed ); // 基礎への加算
		}
		// ストリップ処理
		// 格納場所の取得
		// max_position には何もはいっていないことに注意
		position = pWork->unit[ i ].position;
		if ( position >= pWork->unit[ i ].max_position ){ // 特殊動作
			rot.vx = 0;
			rot.vy = ( 64 ) * ( position % 2 == 0 ? -1 : 1);
			rot.vz = 0;
			// 計算
			DG_SetPos2( &DG_ZeroVector , &rot );
			DG_PutVector( &pWork->unit[ i ].speed , &speed , 1 ); 
			position = pWork->unit[ i ].offset;
			// 後３点を先頭に
			*( pos + position ) = *( pos + pWork->unit[ i ].max_position - 3 );
			*( pos + position + 1 ) = *( pos + pWork->unit[ i ].max_position - 2 );
			*( pos + position + 2 ) = pWork->unit[ i ].pos;
			*( pre_pos + position ) = *( pos + pWork->unit[ i ].max_position - 3 );
			*( pre_pos + position + 1 ) = *( pos + pWork->unit[ i ].max_position - 2 );
			*( pre_pos + position + 2 ) = pWork->unit[ i ].pos;
			// 表示非表示
			( uvrgb + position )->f = 0x8fff ; // ストリップ
			( uvrgb + position + 1 )->f = 0x8fff ; // ストリップ
			( uvrgb + position + 2 )->f = 0x0fff ; // 描画
            // 繋がらないように
			( uvrgb + position + 3 )->f = 0x8fff ; // ストリップ
			( uvrgb + position + 4 )->f = 0x8fff ; // ストリップ
			// 表示非表示
			( pre_uvrgb + position )->f = 0x8fff ; // ストリップ
			( pre_uvrgb + position + 1 )->f = 0x8fff ; // ストリップ
			( pre_uvrgb + position + 2 )->f = 0x0fff ; // 描画
            // 繋がらないように
			( pre_uvrgb + position + 3 )->f = 0x8fff ; // ストリップ
			( pre_uvrgb + position + 4 )->f = 0x8fff ; // ストリップ
			UvAnimePos( pWork , uvrgb , pre_uvrgb , position , i );
			UvAnimePos( pWork , uvrgb , pre_uvrgb , position + 1 , i );
			UvAnimePos( pWork , uvrgb , pre_uvrgb , position + 2 , i );
			pWork->unit[ i ].position = pWork->unit[ i ].offset + 3;
		} else {
			// 通常動作
			rot.vx = 0.0f;
			rot.vy = ( 64 ) * ( position % 2 == 0 ? -1 : 1);
			rot.vz = 0.0f;
			// 計算
			DG_SetPos2( &DG_ZeroVector , &rot );
			DG_PutVector( &pWork->unit[ i ].speed , &speed , 1 ); 
			*( pos + position ) = pWork->unit[ i ].pos;
			*( pre_pos + position ) = pWork->unit[ i ].pos;
			UvAnimePos( pWork , uvrgb , pre_uvrgb , position , i );
			if ( position > pWork->unit[ i ].offset + 1 ){
				( uvrgb + position )->f = 0x0fff ; // 描画
				( pre_uvrgb + position )->f = 0x0fff ; // 描画
				if ( position < pWork->unit[ i ].max_position ){ // 特殊動作
					( uvrgb + position + 1 )->f = 0x8fff ; // ストリップ
					( uvrgb + position + 2 )->f = 0x8fff ; // ストリップ
					( uvrgb + position + 3 )->f = 0x8fff ; // ストリップ
					( pre_uvrgb + position + 1 )->f = 0x8fff ; // ストリップ
					( pre_uvrgb + position + 2 )->f = 0x8fff ; // ストリップ
					( pre_uvrgb + position + 3 )->f = 0x8fff ; // ストリップ
				}
			}
			pWork->unit[ i ].position++;
		}
		{
			SK_StepCheckHzd2( &pWork->unit[ i ].pos , &pWork->unit[ i ].pos , 
						 &speed , 0.2 , 64  , GM_GetHzxGroupID( GM_CurrentMap ) , &SK_f_reflect , &SK_w_reflect );
#if 0
			if ( 1 ){//i == 0 ){
				SK_StepCheckHzd2( &pWork->unit[ i ].pos , &pWork->unit[ i ].pos , 
							 &speed , 0.2 , 64  , GM_GetHzxGroupID( GM_CurrentMap ) , &SK_f_reflect , &SK_w_reflect );
			} else if ( pWork->all_hit == 0 ){ // 先頭？があたったらその集団は全員当たったことにする
				_sceVu0AddVector( &pWork->unit[ i ].pos , &pWork->unit[ i ].pos , &speed );
			} else {
				switch ( pWork->all_hit ){
				case 1 : // 壁
					_sceVu0MulVector( &speed , &speed , &SK_w_reflect );
					_sceVu0AddVector( &pWork->unit[ i ].pos , &pWork->unit[ i ].pos , &speed );
					pWork->all_hit = 0;
					break;
				case 2 : // 床
					_sceVu0MulVector( &speed , &speed , &SK_f_reflect );
					_sceVu0AddVector( &pWork->unit[ i ].pos , &pWork->unit[ i ].pos , &speed );
					pWork->all_hit = 0;
					break;
				}
			}
			if ( pWork->all_hit && !( pWork->time % 4 ) ){ // 火花を出すのは1/2
				rot.vx = rot.vy = rot.vz = 0;
				color.vx = 220.0f;
				color.vy = 160.0f;
				color.vz = 80.0f;
				color.vw = 1.0f;
				rot_wide.vx = 512;
				rot_wide.vy = 512;
				rot_wide.vz = 512;
				DG_SetPos2( &pWork->unit[ i ].pos , &rot );
				DG_GetPos( &fmtmp ); 
				NewSpark2( 5 , &fmtmp , 36.0F , 16.0F , 0.5F , &rot , &rot_wide , &color , 1.0F , 4 );
				pWork->unit[ i ].alpha -= 20;
				pWork->unit[ i ].alpha = pWork->unit[ i ].alpha < 0 ? 0 : pWork->unit[ i ].alpha;
			}
#endif
		}
		// おもい
		AlphaMng( pWork , i );
		DG_COPY_VEC( &pWork->unit[ i ].speed , &speed );
		//		pWork->unit[ i ].speed = speed;
	}
	if ( pWork->target_time >= 0 ){
		{
			// target
			FMATRIX fmtmp;
	
			DG_SetPos( &pWork->world );
			DG_MovePos( &pWork->target_trans );
			DG_GetPos( &fmtmp );
			GM_MoveTarget2Map( &pWork->target, &fmtmp, GM_StageMapAll ) ;
			GM_PutTarget( &pWork->target ) ;
//			NewTargetView2( &pWork->target , 128 , 128 , 160 ); // ターゲットの表示
		}
		pWork->target_time--;
	}

	pWork->time++; // palは気にしなくていい
	// good by
	if ( count >= pWork->num ){
		GV_DestroyActor( pWork );
	}
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
	GM_FreeTarget( &pWork->target ) ;
}

static int InitPacket( Work *pWork, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR speed;
	FVECTOR *pos;
	DG_PRIM2_UVRGB	*uvrgb ;	/* スプライト用 */
	FMATRIX		fmtemp;
	SVECTOR rot;
	int     next_offset;
	int     tmp;
	int		i , j;

	GM_GroupPrim2( pWork->prim, GM_CurrentStageMap ) ;

	DG_SetPos( &pWork->world );
	DG_MovePos( &pWork->shift ); // 銃口にもっていく
	DG_GetPos( &fmtemp );        // マトリクスの取得

	tmp = pWork->num;
	pWork->time = 0;
	pWork->all_hit = 0;
	pWork->unit = GV_Malloc( sizeof( Unit ) * tmp );
	if ( pWork->unit == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( pWork->unit , sizeof( Unit ) * tmp );
	// 加速度の計算 加速度は将来的にランダムになる予定
	{
		float rnd_tmp;
		for ( i = 0 ; i < tmp ; i++ ){
			rnd_tmp = frnd() * 256.f;
			rot.vz = ( short )rnd_tmp;
			rnd_tmp = rnd() * 256.f;
			rot.vy = ( short )rnd_tmp;
			rot.vx = 0;
			DG_SetPos2( &DG_ZeroVector , &rot );
			DG_RotVector( &pWork->power , &speed , 1 );
			DG_SetPos( &pWork->world );
			DG_RotVector( &speed , &pWork->unit[ i ].speed , 1 );
			_sceVu0ScaleVector( &pWork->power , &pWork->power , 0.9f );
		}
	}
	// データの入力
	pos        = MEM_ADDR1 ;
	uvrgb      = MEM_ADDR1 ;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE_SMOKE;
	next_offset = 0;
	// シフトの考慮
	DG_SetPos( &pWork->world );
	DG_MovePos( &pWork->shift );
	DG_GetPos( &pWork->world );
	for ( i = 0 ; i < tmp ; i++ ){ // 無駄使いをなくしました
		DG_COPY_VEC( &pWork->unit[ i ].pos , (FVECTOR *)pWork->world.m[ 3 ] ); // 最初の位置を親としてコピー
		pWork->unit[ i ].pos.vx += frnd() * 100; // 最初の位置を親としてコピー
		pWork->unit[ i ].pos.vy += frnd() * 100; // 最初の位置を親としてコピー
		pWork->unit[ i ].pos.vz += frnd() * 100; // 最初の位置を親としてコピー
		pWork->unit[ i ].offset       = next_offset;
		pWork->unit[ i ].position     = pWork->unit[ i ].offset;
		pWork->unit[ i ].max_position = pWork->unit[ i ].offset + ( irnd() % ( N_VERTS - 20 ) ) + 16; // default size 16
		next_offset                   = pWork->unit[ i ].max_position + 4; // 次のと繋がらないように刷るため + 3
		pWork->unit[ i ].alpha = pWork->alpha + 16.f; 
//		pWork->unit[ i ].alpha = pWork->alpha + 128.f; 
	}
	tmp = pWork->num * N_VERTS;
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
				DG_COPY_VEC( pos , &DG_ZeroVector );
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					pos++;
					trans_count++;
					break;
				}
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
		DG_PRIM2_UVRGB *tmp_uvrgb;
		DG_PRIM2_UVRGB *tmp_uvrgb_pre;
		int mem_count;
		int trans_count;
		int point;

		mem_count = ( tmp / SCR_UVRGBWH_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_uvrgb = ( DG_PRIM2_UVRGB * )prim->uvrgb[ 0 ];
		tmp_uvrgb_pre = ( DG_PRIM2_UVRGB * )prim->uvrgb[ 1 ];
		for ( i = 0 ; i < mem_count ; i++ ){
			for ( j = 0 ; j < SCR_UVRGBWH_MAX ; j ++ ){
				uvrgb->r = 128;
				uvrgb->g = 128;
				uvrgb->b = 128;
				uvrgb->a = 128;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ; // ストリップの都合上
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					uvrgb++;
					trans_count++;
					break;
				}
				uvrgb++;
			}
			point = ( ( trans_count - 1 ) % SCR_UVRGBWH_MAX ) + 1;
			_MemCopy( tmp_uvrgb, MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), point );
			_MemCopy( tmp_uvrgb_pre, MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), point );
			tmp_uvrgb += point;
			tmp_uvrgb_pre += point;
			uvrgb = MEM_ADDR1;
			trans_count = 0;
		}
	}
	InitTarget( pWork , 500.0f , 20 );

	return 1;
}

#define STR_SMOKE (13268797) // fire
#define STR_FIRE (6324186)
#define STR_FIRE2 (4044236)
/*----------------------------------------------------------------*/
static int GetResources( Work *pWork )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	tex = pWork->tex = DG_GetTexture( STR_SMOKE );	// 煙り

	prim = pWork->prim = GM_MakePrim2( POLY_FLAG , pWork->num , N_VERTS );
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

void *NewClayBomb( int num , FMATRIX *world, FVECTOR *shift , FVECTOR *pPower , int life, float alpha , long64 flag )
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
		pWork->num = num;
		pWork->life = life;
		pWork->alpha = alpha;
		pWork->weapon_flag = flag;
		pWork->act = ( void * )NormalAct;

		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}
