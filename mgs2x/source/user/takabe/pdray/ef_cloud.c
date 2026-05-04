//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_cloud.c
	量産型ＲＡＹ戦用雲エフェクト

	2001/06/28 K.Takabe
	$Id: ef_cloud.c,v 1.1.1.3 2002/11/19 11:51:20 Yoshizawa1 Exp $

*/
/*



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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */

#if 0
#define POLY_SIZE	(20000)
#define AREA_SIZE	(100000)
//#define HEIGHT		(2500)
#define HEIGHT		(4200)
#define HEIGHT_OFFSET		(RND(200)-100)
#else
#define POLY_SIZE	(8000)
#define AREA_SIZE	(30000)
#define HEIGHT		(4200)
#define HEIGHT_OFFSET		(0)
#endif

#define MAP_X			(64)
#define MAP_Z			(64)
#define MAP_SCALE_X		( (float)MAP_X / (float)( AREA_SIZE * 2 ) )
#define MAP_SCALE_Z		( (float)MAP_Z / (float)( AREA_SIZE * 2 ) )

/* ---------------------------------------------------------------- */
#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* ---------------------------------------------------------------- */
/* プリミティブパラメータ */
#define PRIM_FLAG		(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_VERTS			(32)
#define N_PRIMS			(32)
#define TOTAL_POS		(N_PRIMS*(N_VERTS/4))

#define MAX_CALC_POS	(256)							/* 一度に処理する頂点数 */
#define MAX_CALC_NUM	(MAX_CALC_POS/(N_VERTS/4))		/* 一度に処理するポリゴン数 */
#define N_GROUPS		(TOTAL_POS / MAX_CALC_NUM)		/* 処理する回数 */

#define TEXTURE		(7361654)		/* "w00_sky_add_alp" */

/* ---------------------------------------------------------------- */
/* スクラッチパッド使用状況 */
typedef struct _scrpad{/* 合計１６ＫＢ以下であること！ */
	FVECTOR				base_pos[ 4 ];
	FVECTOR				offset[ MAX_CALC_NUM ] ;
	FVECTOR				pos[ MAX_CALC_POS ];
	DG_PRIM2_UVRGB		uvrgb[ MAX_CALC_POS ] ;
	u_char				map[ MAP_X * MAP_Z ] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR		vvec ;
	FMATRIX		mat ;
} GROUP_DATA ;

typedef struct {
	GV_ACT_EX	actor ;
	int			map ;

	DG_PRIM2	*prim ;
	FVECTOR		offset[ TOTAL_POS ] ;
	GROUP_DATA	group_data[ N_GROUPS ] ;

	/* 濃度マップ */
	u_char		density_map[ MAP_X * MAP_Z ] ;
	u_char		max_density_map[ MAP_X * MAP_Z ] ;
	u_char		sub_density_map[ MAP_X * MAP_Z ] ;

	/* 当たり判定用 */
	TARGET			target ;
	POWER_TARGET	power_target ;
	FVECTOR			target_force ;
	FVECTOR			damage_pos ;
	int				target_damage_flag ;
	int				invinsible_time ;

} Work ;

static Work		*work_ptr ;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
static void TargetCallback( TARGET *offence, TARGET *defence, Work *work )
{
	POWER_TARGET	*power ;
	if ( work->invinsible_time ) return ;
	if ( offence->power == NULL ) return ;
	power = offence->power ;
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
		if ( !( offence->weapon_type & WP_CLAYMORE ) ){
			work->damage_pos = offence->center ;
			work->invinsible_time = 1 ;
			work->target_damage_flag = 1 ;
		}
		break ;
	  case POWER_ONCE:
	  case POWER_DECREASE:
	  case POWER_THRESHOLD:
	  case POWER_CONST:
	  default:
		break ;
	}
}

/* ---------------------------------------------------------------- */
static void MapAdjust( u_char *map )
{
	static FVECTOR	dir[6] = {
		{0,0,1,0},
		{0,0,-1,0},
		{1.732058/2,0,0.5,0},
		{1.732058/2,0,-0.5,0},
		{-1.732058/2,0,0.5,0},
		{-1.732058/2,0,-0.5,0}
	};
	FVECTOR		pos ;
	int		i, j, k, flag, alpha ;
	for ( i = 0 ; i < MAP_Z ; i++ ){
		for ( j = 0 ; j < MAP_X ; j++ ){
			pos.vx = (float)j / MAP_SCALE_X - AREA_SIZE ;
			pos.vy = 0 ;
			pos.vz = (float)i / MAP_SCALE_Z - AREA_SIZE ;
			flag = 0 ;
			alpha = 255 ;
			for ( k = 0 ; k < 6 ; k++ ){
				float	len ;
				len = GTE_InnerProduct( &dir[k], &pos );
				if ( len > ( 25000.0f - POLY_SIZE/2 - 3000 ) ){
					int		a ;
					len -= 25000.0f - POLY_SIZE/2 - 3000 ;
					a = DG_FTOI( 255.0f * len / 3000.0f ) ;
					if ( a > 255 ) a = 255 ;
					a = 255 - a ;
					if ( a < alpha ) alpha = a ;
					flag |= 1 ;
				}
			}
			*map = alpha ;
			map++ ;
		}
	}
}

static void CheckDensity( u_char *map, u_char *max_map, u_char *sub_map )
{
	int		i, d, s ;

	for ( i = 0 ; i < MAP_X * MAP_Z ; i++ ){
		if ( *map < *max_map ){
			*map += 1 ;
		}
		s = *sub_map ;
		d = *map ;
		d -= s ;
		if ( d < 0 ) d = 0 ;
		*map = d ;
		s = s - s / 8 - 1 ;
		if ( s < 0 ) s = 0 ;
		*sub_map = s ;
		map++ ;
		max_map++ ;
		sub_map++ ;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos, *old_pos, *spr_pos ;
	DG_PRIM2_UVRGB		*uvrgb, *old_uvrgb, *spr_uvrgb ;
	GROUP_DATA	*group_data ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int			i, j ;

	GM_CurrentMap = work->map ;

	/* 当たり判定処理 */
	if ( work->target_damage_flag ){
		extern void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size );
		PDRAY_CLOUD_DeleteDensity( &work->damage_pos, 5000 );
		work->target_damage_flag = 0 ;
	}
	if ( work->invinsible_time ) work->invinsible_time-- ;
	GM_ClearTargetDamage( &work->target );
#if 0
	if ( GV_PadData[0].press & PAD_X ){
		extern void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size );
		FVECTOR		tmp_vec ;
		tmp_vec = GM_PlayerPosition ;
		PDRAY_CLOUD_DeleteDensity( &tmp_vec, 5000 );
	}
#endif

	GTE_InitGTE();

	/* 濃度関連処理 */
	FlushCache( 0 );
	SprLoad( scrpad->map, work->density_map, MAP_X * MAP_Z / sizeof(u_long128) );
	SprLoad( scrpad->pos, work->max_density_map, MAP_X * MAP_Z / sizeof(u_long128) );
	SprLoad( scrpad->uvrgb, work->sub_density_map, MAP_X * MAP_Z / sizeof(u_long128) );

	CheckDensity( scrpad->map, (u_char*)scrpad->pos, (u_char*)scrpad->uvrgb );

	FlushCache( 0 );
	SprStore( scrpad->map, work->density_map, MAP_X * MAP_Z / sizeof(u_long128) );
	SprStore( scrpad->uvrgb, work->sub_density_map, MAP_X * MAP_Z / sizeof(u_long128) );


	/* プリミティブ関連処理 */
	prim = work->prim ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;

	/* 以前の頂点データを参照するためアドレスを記憶 */
	old_pos = prim->pos[ prim->buffer_clock ] ;
	old_uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	/* プリミティブ頂点データのダブルバッファ切り替え */
	DG_SwitchBuffPrim2( prim );
	/* 書き込み用頂点データアドレス取得 */
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	/* スクラッチパッド上でのデータアドレス取得 */
	spr_pos = scrpad->pos ;
	spr_uvrgb = scrpad->uvrgb ;

	FlushCache( 0 );
	SprLoad( scrpad->uvrgb, uvrgb, MAX_CALC_NUM * 4 );
	{
		FMATRIX		mat ;
		FVECTOR		*offset ;
		GTE_SetVector( scrpad->base_pos + 0, -POLY_SIZE, 0, -POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 1, POLY_SIZE, 0, -POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 2, -POLY_SIZE, 0, POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 3, POLY_SIZE, 0, POLY_SIZE, 1 );
		group_data = work->group_data ;
		for ( i = 0 ; i < TOTAL_POS ; i+= MAX_CALC_NUM ){
			FlushCache( 0 );
			SprLoad( scrpad->offset, work->offset + i, MAX_CALC_NUM );
			spr_pos = scrpad->pos ;
			spr_uvrgb = scrpad->uvrgb ;
			offset = scrpad->offset ;
			//GTE_UnitMatrix( &mat );
			mat = group_data->mat ;
			GTE_LoadMatrix( &mat );
			for ( j = 0 ; j < MAX_CALC_NUM ; j++ ){
				int		alpha ;
				float	len ;
				GTE_AddVector( offset, offset, &group_data->vvec );
				if ( offset->vx > AREA_SIZE ) offset->vx -= AREA_SIZE*2 ;
				if ( offset->vz > AREA_SIZE ) offset->vz -= AREA_SIZE*2 ;
				if ( offset->vx < -AREA_SIZE ) offset->vx += AREA_SIZE*2 ;
				if ( offset->vz < -AREA_SIZE ) offset->vz += AREA_SIZE*2 ;
				len = GTE_InnerProduct( offset, offset ) ;
				len = 1.0f - len / ((float)AREA_SIZE*(float)AREA_SIZE) ;
				len = DG_MAX( len, 0.0f );
				{
					int		index, x, z ;
					int		index1, index2, index3 ;
					int		rate0, rate1, rate2, rate3 ;
					x = DG_FTOI( offset->vx * MAP_SCALE_X * 256 ) + 32 * 256 ;
					z = DG_FTOI( offset->vz * MAP_SCALE_Z * 256 ) + 32 * 256 ;
					if ( x < 0 ) x = 0 ;
					if ( z < 0 ) z = 0 ;
					if ( x > (MAP_X-2)*256 ) x = (MAP_X-2)*256 ;
					if ( z > (MAP_Z-2)*256 ) z = (MAP_Z-2)*256 ;
					rate3 = ( x & 255 ) * ( z & 255 ) ;
					rate2 = ( ( x & 255 ) ^ 255 ) * ( z & 255 ) ;
					rate1 = ( x & 255 ) * ( ( z & 255 ) ^ 255 ) ;
					rate0 = ( ( x & 255 ) ^ 255 ) * ( ( z & 255 ) ^ 255 ) ;
					index = (x>>8) + (z>>8) * MAP_X ;
					index1 = index + 1 ;
					index2 = index + MAP_X ;
					index3 = index + MAP_X + 1 ;
					alpha = scrpad->map [ index ] * rate0 + scrpad->map[ index1 ] * rate1 +
					  scrpad->map[ index2 ] * rate2 + scrpad->map[ index3 ] * rate3 ;
					alpha = alpha * 32 / ( 255 * 255 * 256 ) ;
					//alpha = scrpad->map[ index ] * 32 / 256 ;
				}
				//alpha = len * 32 ;
#ifndef BP_PSX2_ASM
				*(FVECTOR*)mat.m[3] = *offset ;		/* 回転はそのままで座標オフセットのみを変更 */
				GTE_LoadMatrix( &mat );
				GTE_PutVector( spr_pos, scrpad->base_pos, 4 );
#else
				asm volatile ("lqc2 vf7,0(%0)"::"r"(offset));	/* 回転はそのままで座標オフセットのみを変更 */
				GTE_PutVector( spr_pos, scrpad->base_pos, 4 );
#endif
				spr_uvrgb[0].a = alpha ;
				spr_uvrgb[1].a = alpha ;
				spr_uvrgb[2].a = alpha ;
				spr_uvrgb[3].a = alpha ;
				spr_pos += 4 ;
				spr_uvrgb += 4 ;
				offset++ ;
			}
			FlushCache( 0 );
			SprStore( scrpad->pos, pos , MAX_CALC_NUM * 4 );
			SprStore( scrpad->uvrgb, uvrgb , MAX_CALC_NUM * 4 );
			SprStore( scrpad->offset, work->offset + i, MAX_CALC_NUM );
			pos += MAX_CALC_NUM * 4 ;
			uvrgb += MAX_CALC_NUM * 4 ;
			group_data++ ;
		}
	}

}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
	GM_FreeTarget( &work->target );

	work_ptr = NULL ;
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX		*tex ;
	FVECTOR		*pos ;
	GROUP_DATA	*group_data ;
	int			i, j ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	work->map = GM_CurrentMap ;

	/* テクスチャの取得 */
	tex = DG_GetTexture( TEXTURE );

	/* プリミティブの確保 */
	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	if ( prim == NULL ) return ( -1 );
	DG_ConfigPrim2Tex( prim, tex );
	//DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA(2,0,0,1,0) );
	prim->flag |= DG_PRIM2_INVISIBLE ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world.m[3][1] = HEIGHT ;

	for ( i = 0 ; i < N_GROUPS ; i++ ){
		int		rot_y ;
		rot_y = GTE_REGULAR_ANGLE( RND(4096) ) ;
		GTE_MakeRotate( &work->group_data[ i ].mat, 0, 1, 0, GTE_PS2RAD( rot_y ) );
		work->group_data[ i ].vvec.vx = RND(100) - 50 ;
		work->group_data[ i ].vvec.vz = RND(100) - 50 ;
	}

	/* 中心位置の初期化 */
	pos = work->offset ;
	for ( i = 0 ; i < TOTAL_POS ; i++ ){
		pos->vx = RND( AREA_SIZE*2 ) - AREA_SIZE ;
		pos->vy = HEIGHT_OFFSET ;
		pos->vz = RND( AREA_SIZE*2 ) - AREA_SIZE ;
		pos->vw = 1.0f ;
		pos++ ;
	}

	{
		FMATRIX		mat ;
		FVECTOR		*offset ;
		GTE_SetVector( scrpad->base_pos + 0, -POLY_SIZE, 0, -POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 1, POLY_SIZE, 0, -POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 2, -POLY_SIZE, 0, POLY_SIZE, 1 );
		GTE_SetVector( scrpad->base_pos + 3, POLY_SIZE, 0, POLY_SIZE, 1 );
		group_data = work->group_data ;
		for ( i = 0 ; i < TOTAL_POS ; i+= MAX_CALC_NUM ){
			FlushCache( 0 );
			SprLoad( scrpad->offset, work->offset + i, MAX_CALC_NUM );
			pos = scrpad->pos ;
			offset = scrpad->offset ;
			//GTE_UnitMatrix( &mat );
			mat = group_data->mat ;
			GTE_LoadMatrix( &mat );
			for ( j = 0 ; j < MAX_CALC_NUM ; j++ ){
#ifndef BP_PSX2_ASM
				*(FVECTOR*)mat.m[3] = *offset ;		/* 回転はそのままで座標オフセットのみを変更 */
				GTE_LoadMatrix( &mat );
				GTE_PutVector( pos, scrpad->base_pos, 4 );
#else
				asm volatile ("lqc2 vf7,0(%0)"::"r"(offset));	/* 回転はそのままで座標オフセットのみを変更 */
				GTE_PutVector( pos, scrpad->base_pos, 4 );
#endif
				pos += 4 ;
				offset++ ;
			}
			FlushCache( 0 );
			SprStore( scrpad->pos, prim->pos[0] + i * 4, MAX_CALC_NUM * 4 );
			SprStore( scrpad->pos, prim->pos[1] + i * 4, MAX_CALC_NUM * 4 );
			group_data++ ;
		}
	}

	{
		int		u0, v0, u1, v1 ;
		DG_PRIM2_UVRGB	*uvrgb0, *uvrgb1 ;
		u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
		v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
		u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
		v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;
		uvrgb = scrpad->uvrgb ;
		for ( j = 0 ; j < MAX_CALC_NUM ; j++ ){
			/* ＵＶ値設定 */
			SET_UVRGB_UV( uvrgb + 0, u0, v0 );	/* 左上 */
			SET_UVRGB_UV( uvrgb + 1, u1, v0 );	/* 右上 */
			SET_UVRGB_UV( uvrgb + 2, u0, v1 );	/* 左下 */
			SET_UVRGB_UV( uvrgb + 3, u1, v1 );	/* 右下 */
			/* 頂点カラー設定 */
			SET_UVRGB_COL( uvrgb + 0, 128, 128, 128, 0 );
			SET_UVRGB_COL( uvrgb + 1, 128, 128, 128, 0 );
			SET_UVRGB_COL( uvrgb + 2, 128, 128, 128, 0 );
			SET_UVRGB_COL( uvrgb + 3, 128, 128, 128, 0 );
			/* フラグの設定 */
			uvrgb[0].f = 0x8fff ;	/* それ以降は描画キックを行う */
			uvrgb[1].f = 0x8fff ;	/* それ以降は描画キックを行う */
			uvrgb[2].f = 0x0fff ;	/* それ以降は描画キックを行う */
			uvrgb[3].f = 0x0fff ;	/* それ以降は描画キックを行う */
			uvrgb += 4 ;
		}
		uvrgb0 = prim->uvrgb[ 0 ];
		uvrgb1 = prim->uvrgb[ 1 ];
		for ( i = 0 ; i < TOTAL_POS ; i+= MAX_CALC_NUM ){
			/* スクラッチパッド上のデータをメモリへ書き出す */
			FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
			SprStore( scrpad->uvrgb, uvrgb0, SIZEOF_QWORD(DG_PRIM2_UVRGB) * MAX_CALC_NUM * 4 );
			SprStore( scrpad->uvrgb, uvrgb1, SIZEOF_QWORD(DG_PRIM2_UVRGB) * MAX_CALC_NUM * 4 );
			uvrgb0 += MAX_CALC_NUM * 4 ;
			uvrgb1 += MAX_CALC_NUM * 4 ;
		}
	}

	MapAdjust( work->density_map );
	MapAdjust( work->max_density_map );

	{/* あたり判定設定 */
		FVECTOR		size, trg_pos ;
		int			class ;

		class = TARGET_POWER|TARGET_DEFENSE|TARGET_THROUGH|TARGET_CALL_CALLBACK_THROUGH_HIT ;
		size.vx = AREA_SIZE ;
		size.vy = 1000.0f ;
		size.vz = AREA_SIZE ;
		GTE_SetVector( &trg_pos, 0, 4000 - 1000 + 10, 0, 1 );
		GM_SetTarget( &work->target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->target, &work->power_target, POWER_CONST, 1, 0, 0, &work->target_force );
		GM_MoveTarget( &work->target, &trg_pos );
		GM_PutTarget( &work->target );
		GM_SetTargetCallBack( &work->target, (TARGET_CALLBACK)TargetCallback, work );
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *_NewRAYStageCloud( void )
{
	Work		*work ;

	OPERATOR() ;

	if ( work_ptr != NULL ) return ( NULL ) ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		//GV_SetActorSignalFunc( work, ReceiveSignal );
		work_ptr = work ;
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
/* 濃度を変化させる */
void _PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size )
{
	Work	*work = work_ptr ;
	int		x0, z0, index, i, j, d, x, z ;
	float	size_inv ;

	if ( work_ptr == NULL ) return ;

	if ( pos->vy < 3800.0f || pos->vy > 4500.0f ) return ;

	x0 = DG_FTOI( pos->vx * MAP_SCALE_X ) + 32 ;
	z0 = DG_FTOI( pos->vz * MAP_SCALE_Z ) + 32 ;
	size = DG_FTOI( (float)size * MAP_SCALE_X ) ;

	size_inv = 1.0f / ( (float)size ) ;
	for ( i = -size ; i <= size ; i++ ){
		for ( j = -size ; j <= size ; j++ ){
			float	r ;
			x = x0 + j ;
			z = z0 + i ;
			if ( x < 0 ) continue ;
			if ( z < 0 ) continue ;
			if ( x >= MAP_X ) continue ;
			if ( z >= MAP_Z ) continue ;
			index = x + z * MAP_X ;
			d = work->density_map[ index ] ;
			r = DG_SQRT( (float)i * j ) * size_inv ;
			r = DG_MIN( r, 1.0f );
			d += r * 64 ;
			if ( d > 255 ) d = 255 ;
			work->sub_density_map[ index ] = d ;
		}
	}
}

