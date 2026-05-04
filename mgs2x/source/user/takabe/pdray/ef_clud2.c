//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_clud2.c
	量産型ＲＡＹ戦用床フォグエフェクト

	2001/08/03 K.Takabe
	$Id: ef_clud2.c,v 1.1.1.3 2002/11/19 11:51:20 Yoshizawa1 Exp $

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



#if 1
#define HEIGHT		(4010)
#define SIZE_X		(1800)
#define SIZE_Z		(1800)
#define DIV_X		(5)
#define DIV_Z		(5)
#define VERTS_X		(DIV_X+1)
#define VERTS_Z		(DIV_Z+1)
#define N_MESH_X	(8)
#define N_MESH_Z	(8)
#define MESH_WIDTH_X	(SIZE_X*DIV_X)
#define MESH_WIDTH_Z	(SIZE_Z*DIV_Z)
#define N_PLANE		(1)
#else
#define HEIGHT		(4100)
#define SIZE_X		(2400)
#define SIZE_Z		(2400)
#define DIV_X		(5)
#define DIV_Z		(5)
#define VERTS_X		(DIV_X+1)
#define VERTS_Z		(DIV_Z+1)
#define N_MESH_X	(6)
#define N_MESH_Z	(6)
#define MESH_WIDTH_X	(SIZE_X*DIV_X)
#define MESH_WIDTH_Z	(SIZE_Z*DIV_Z)
#define N_PLANE		(2)
#endif

#define AREA_SIZE_X	(MESH_WIDTH_X*N_MESH_X/2)
#define AREA_SIZE_Z	(MESH_WIDTH_Z*N_MESH_Z/2)

#define MAP_X			(64)
#define MAP_Z			(64)
#define MAP_SCALE_X		( (float)MAP_X / (float)( AREA_SIZE_X*2 ) )
#define MAP_SCALE_Z		( (float)MAP_Z / (float)( AREA_SIZE_Z*2 ) )

/* ---------------------------------------------------------------- */
#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* ---------------------------------------------------------------- */
/* プリミティブパラメータ */
#define PRIM_FLAG		(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_VERTS			(VERTS_X*2*DIV_Z)
#define N_PRIMS			(N_MESH_X*N_MESH_Z*N_PLANE)

//#define TEXTURE		(7361654)		/* "w00_sky_add_alp" */
//#define TEXTURE		(3182109)		/* "smoke_lp3_alp" *//* MGS2 */
#define TEXTURE		(4230685)		/* "smoke_lp4_alp" *//* MGS2 SUBSTANCE */

/* ---------------------------------------------------------------- */
/* スクラッチパッド使用状況 */
typedef struct _scrpad{/* 合計１６ＫＢ以下であること！ */
	FVECTOR				pos[ N_VERTS ];
	DG_PRIM2_UVRGB		uvrgb[ N_VERTS ] ;
	FVECTOR				src_pos[ VERTS_X * VERTS_Z ] ;
	DG_PRIM2_UVRGB		src_uvrgb[ VERTS_X * VERTS_Z ] ;
	FVECTOR				base_pos[ VERTS_X * VERTS_Z ] ;
	u_char				map[ MAP_X * MAP_Z ] ;
	u_char				max_map[ MAP_X * MAP_Z ] ;
	u_char				sub_map[ MAP_X * MAP_Z ] ;
	u_char				index_table[N_VERTS] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */

typedef struct {
	GV_ACT_EX	actor ;
	int			map ;

	DG_TEX		*tex ;
	DG_PRIM2	*prim ;
	FVECTOR		offset[N_PLANE] ;
	FVECTOR		old_offset[N_PLANE] ;
	FVECTOR		normalize_offset[N_PLANE] ;

	/* 濃度マップ */
	u_char		density_map[ MAP_X * MAP_Z ] ;
	u_char		max_density_map[ MAP_X * MAP_Z ] ;
	u_char		sub_density_map[ MAP_X * MAP_Z ] ;

	/* その他パラメータ */
	int			count ;

	/* 当たり判定用 */
	TARGET			target ;
	POWER_TARGET	power_target ;
	FVECTOR			target_force ;
	FVECTOR			damage_pos ;
	int				target_damage_flag ;
	int				invinsible_time ;

} Work ;

static Work		*work_ptr ;
static char	index_table[N_VERTS] ;

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
			pos.vx = (float)j / MAP_SCALE_X - AREA_SIZE_X ;
			pos.vy = 0 ;
			pos.vz = (float)i / MAP_SCALE_Z - AREA_SIZE_Z ;
			flag = 0 ;
			alpha = 255 ;
			for ( k = 0 ; k < 6 ; k++ ){
				float	len ;
				len = GTE_InnerProduct( &dir[k], &pos );
				if ( len > ( 25000.0f - 3000.0f - 3000 ) ){
					int		a ;
					len -= 25000.0f - 3000.0f  - 3000 ;
					a = DG_FTOI( 255.0f * len / 3000.0f ) ;
					if ( a > 255 ) a = 255 ;
					a = 255 - a ;
					if ( a < alpha ) alpha = a ;
					flag |= 1 ;
				}
			}
			//printf("%c", 'a' + alpha / 16 );
			*map = alpha ;
			map++ ;
		}
		//printf("\n");
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
#if 1
		s = s - s / 8 - 1 ;
#else
		s = 0 ;
#endif
		if ( s < 0 ) s = 0 ;
		*sub_map = s ;
		map++ ;
		max_map++ ;
		sub_map++ ;
	}
}

/* ---------------------------------------------------------------- */
static void MakeBaseData( Work *work )
{
	/* 基本メッシュデータの生成 */
	ScrpadWork		*scrpad = SCRPAD_ADDR ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX	*tex = work->tex ;
	int		u, v, i, j ;
	float	fu, fv ;

	pos = scrpad->base_pos ;
	uvrgb = scrpad->src_uvrgb ;
	for ( i = 0 ; i < VERTS_Z ; i++ ){
		for ( j = 0 ; j < VERTS_X ; j++ ){
			pos->vx = j * SIZE_X - ( SIZE_X * DIV_X / 2 ) ;
			pos->vy = 1.0f ;
			pos->vz = i * SIZE_Z - ( SIZE_Z * DIV_Z / 2 ) ;
			pos->vw = 1.0f ;
			pos++ ;
			fu = (float)j / (float)DIV_X ;
			fv = (float)i / (float)DIV_Z ;
			u = FTOI12( fu * tex->u_scale + tex->u_offset ) ;
			v = FTOI12( fv * tex->v_scale + tex->v_offset ) ;
			uvrgb->u = u ;
			uvrgb->v = v ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = 80 ;
			uvrgb->g = 80 ;
			uvrgb->b = 80 ;
			uvrgb->a = 32 ;
			uvrgb++ ;
		}
	}
}
static void InitPacket( FVECTOR *dst_pos, DG_PRIM2_UVRGB *dst_uvrgb,
					   FVECTOR *src_pos, DG_PRIM2_UVRGB *src_uvrgb, u_char *index )
{
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i ;

	uvrgb = dst_uvrgb ;
	for ( i = 0 ; i < N_VERTS ; i++ ){
#if 1
		GV_PREFECH( &dst_pos[4] );
		GV_PREFECH( &uvrgb[4] );
#endif
		*dst_pos = src_pos[ *index ] ;
		*uvrgb = src_uvrgb[ *index ] ;
		dst_pos++ ;
		uvrgb++ ;
		index++ ;
	}
	/* ストリップを切断する */
	for ( i = 0 ; i < DIV_Z ; i++ ){
		dst_uvrgb[ i * VERTS_X * 2 + 0 ].f = 0x8fff ;
		dst_uvrgb[ i * VERTS_X * 2 + 1 ].f = 0x8fff ;
	}
	
	
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int			i, j, k, l ;
	FVECTOR		offset, tmp_vec ;

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
	SprLoad( scrpad->max_map, work->max_density_map, MAP_X * MAP_Z / sizeof(u_long128) );
	SprLoad( scrpad->sub_map, work->sub_density_map, MAP_X * MAP_Z / sizeof(u_long128) );

	CheckDensity( scrpad->map, (u_char*)scrpad->max_map, (u_char*)scrpad->sub_map );

	FlushCache( 0 );
	SprStore( scrpad->map, work->density_map, MAP_X * MAP_Z / sizeof(u_long128) );
	SprStore( scrpad->sub_map, work->sub_density_map, MAP_X * MAP_Z / sizeof(u_long128) );

	/* プリミティブ関連処理 */
	prim = work->prim ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;

	/* プリミティブ頂点データのダブルバッファ切り替え */
	DG_SwitchBuffPrim2( prim );
	/* 書き込み用頂点データアドレス取得 */
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	MakeBaseData( work );
	for ( i = 0 ; i < N_VERTS ; i++ ) scrpad->index_table[i] = index_table[i] ;

	for ( l = 0 ; l < N_PLANE ; l++ ){/* プリミティブデータの初期化 */
		FMATRIX		mat ;
		FVECTOR			*scrpad_pos ;
		DG_PRIM2_UVRGB	*scrpad_uvrgb ;


		work->offset[l].vx = sinf( GTE_PS2RAD( work->count ) + l * 300 ) * 3.0f ;
		work->offset[l].vz = cosf( GTE_PS2RAD( work->count ) + l * 300 ) * 3.0f ;
		GTE_SubVector( &tmp_vec, &work->old_offset[l], &work->offset[l] );
		work->old_offset[l] = work->offset[l] ;
		GTE_AddVector( &work->normalize_offset[l], &work->normalize_offset[l], &tmp_vec );
		if ( work->normalize_offset[l].vx < -0.5f ) work->normalize_offset[l].vx += 1.0f ;
		if ( work->normalize_offset[l].vx > 0.5f ) work->normalize_offset[l].vx -= 1.0f ;
		if ( work->normalize_offset[l].vz < -0.5f ) work->normalize_offset[l].vz += 1.0f ;
		if ( work->normalize_offset[l].vz > 0.5f ) work->normalize_offset[l].vz -= 1.0f ;
		GTE_ScaleVector( &offset, &work->normalize_offset[l], MESH_WIDTH_X );

		GTE_UnitMatrix( &mat );
		for ( i = 0 ; i < N_MESH_X ; i++ ){
			for ( j = 0 ; j < N_MESH_Z ; j++ ){
				mat.m[3][0] = i * MESH_WIDTH_X - MESH_WIDTH_X * ( N_MESH_X - 1 ) / 2 ;
				mat.m[3][1] = l * 100 ;
				mat.m[3][2] = j * MESH_WIDTH_Z - MESH_WIDTH_Z * ( N_MESH_Z - 1 ) / 2 ;
				mat.m[3][0] += offset.vx ;
				mat.m[3][2] += offset.vz ;
				GTE_LoadMatrix( &mat );
				GTE_PutVector( scrpad->src_pos, scrpad->base_pos, VERTS_X * VERTS_Z );
				scrpad_pos = scrpad->src_pos ;
				scrpad_uvrgb = scrpad->src_uvrgb ;
				for ( k = 0 ; k < VERTS_X * VERTS_Z ; k++ ){
					int		alpha ;
					{
						int		index, x, z ;
						int		index1, index2, index3 ;
						int		rate0, rate1, rate2, rate3 ;
						x = DG_FTOI( scrpad_pos->vx * MAP_SCALE_X * 256 ) + 32 * 256 ;
						z = DG_FTOI( scrpad_pos->vz * MAP_SCALE_Z * 256 ) + 32 * 256 ;
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
#if 0
						alpha = alpha * 128 / ( 255 * 255 * 256 ) ;
#else
						alpha = alpha >> 17 ;
#endif
						//alpha = scrpad->map[ index ] * 32 / 256 ;
					}
					scrpad_uvrgb->r = alpha ;
					scrpad_uvrgb->g = alpha ;
					scrpad_uvrgb->b = alpha ;
					scrpad_pos++ ;
					scrpad_uvrgb++ ;
				}
#if 0
				InitPacket( scrpad->pos, scrpad->uvrgb, scrpad->src_pos, scrpad->src_uvrgb, scrpad->index_table );
				FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
				SprStore( scrpad->pos, pos, SIZEOF_QWORD(FVECTOR) * N_VERTS );
				SprStore( scrpad->uvrgb, uvrgb, SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_VERTS );
#else
				InitPacket( pos, uvrgb, scrpad->src_pos, scrpad->src_uvrgb, scrpad->index_table );
#endif
				pos += N_VERTS ;
				uvrgb += N_VERTS ;
			}
		}
	}

	work->count += 1 ;

#ifndef PSX2
	if ( DG_Chanl(0)->eye.m[3][1] < HEIGHT ){
		work->prim->flag |= DG_PRIM2_INVISIBLE ;
	} else {
		work->prim->flag &= ~DG_PRIM2_INVISIBLE ;
	}
#endif

}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GM_FreePrim2( work->prim );
	GM_FreeTarget( &work->target );

	work_ptr = NULL ;
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			i, j, k ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	work->map = GM_CurrentMap ;

	{/* プリミティブ用インデックステーブル初期化 */
		u_char	*index ;

		index = index_table ;
		for ( i = 0 ; i < DIV_Z ; i++ ){
			for ( j = 0 ; j <= DIV_X ; j++ ){
				index[0] = j + ( i + 0 ) * VERTS_X ;
				index[1] = j + ( i + 1 ) * VERTS_Z ;
				index += 2 ;
			}
		}
	}

	/* テクスチャの取得 */
	work->tex = tex = DG_GetTexture( TEXTURE );

	/* プリミティブの確保 */
	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	prim->raise = -SIZE_X ;
	if ( prim == NULL ) return ( -1 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA(0,2,2,1,32) );
	prim->flag |= DG_PRIM2_INVISIBLE ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world.m[3][1] = HEIGHT ;

	MakeBaseData( work );

	for ( k = 0 ; k < N_PLANE ; k++ ){/* プリミティブデータの初期化 */
		FMATRIX		mat ;
		FVECTOR			*pos0, *pos1 ;
		DG_PRIM2_UVRGB	*uvrgb0, *uvrgb1 ;

		pos0 = prim->pos[ 0 ];
		pos1 = prim->pos[ 1 ];
		uvrgb0 = prim->uvrgb[ 0 ];
		uvrgb1 = prim->uvrgb[ 1 ];
		GTE_UnitMatrix( &mat );
		for ( i = 0 ; i < N_MESH_X ; i++ ){
			for ( j = 0 ; j < N_MESH_Z ; j++ ){
				mat.m[3][0] = i * MESH_WIDTH_X - MESH_WIDTH_X * N_MESH_X / 2 ;
				mat.m[3][1] = k + 100 ;
				mat.m[3][2] = j * MESH_WIDTH_Z - MESH_WIDTH_Z * N_MESH_Z / 2 ;
				GTE_LoadMatrix( &mat );
				GTE_PutVector( scrpad->src_pos, scrpad->base_pos, VERTS_X * VERTS_Z );
				InitPacket( scrpad->pos, scrpad->uvrgb, scrpad->src_pos, scrpad->src_uvrgb, index_table );
				FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
				SprStore( scrpad->pos, pos0, SIZEOF_QWORD(FVECTOR) * N_VERTS );
				SprStore( scrpad->pos, pos1, SIZEOF_QWORD(FVECTOR) * N_VERTS );
				SprStore( scrpad->uvrgb, uvrgb0, SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_VERTS );
				SprStore( scrpad->uvrgb, uvrgb1, SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_VERTS );
				pos0 += N_VERTS ;
				pos1 += N_VERTS ;
				uvrgb0 += N_VERTS ;
				uvrgb1 += N_VERTS ;
			}
		}
	}


	MapAdjust( work->density_map );
	MapAdjust( work->max_density_map );

	{/* あたり判定設定 */
		FVECTOR		size, trg_pos ;
		int			class ;

		class = TARGET_POWER|TARGET_DEFENSE|TARGET_THROUGH|TARGET_CALL_CALLBACK_THROUGH_HIT ;
		size.vx = AREA_SIZE_X ;
		size.vy = 1000.0f ;
		size.vz = AREA_SIZE_Z ;
		GTE_SetVector( &trg_pos, 0, 4000 - 1000 + 10, 0, 1 );
		GM_SetTarget( &work->target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->target, &work->power_target, POWER_CONST, 1, 0, 0, &work->target_force );
		GM_MoveTarget( &work->target, &trg_pos );
		GM_PutTarget( &work->target );
		GM_SetTargetCallBack( &work->target, (TARGET_CALLBACK)TargetCallback, work );
	}

	for ( i = 0 ; i < N_PLANE ; i++ ){
		work->normalize_offset[i].vx += (float)i / (float)( N_PLANE + 1 ) ;
		work->normalize_offset[i].vz += (float)i / (float)( N_PLANE + 1 ) ;
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewRAYStageCloud( void )
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
void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size )
{
	Work	*work = work_ptr ;
	int		x0, z0, index, i, j, d, x, z ;
	float	size_inv ;

	if ( work_ptr == NULL ) return ;

	if ( pos->vy < 3800.0f || pos->vy > 4500.0f ) return ;

	x0 = DG_FTOI( pos->vx * MAP_SCALE_X ) + 32 ;
	z0 = DG_FTOI( pos->vz * MAP_SCALE_Z ) + 32 ;
#if 1
	size_inv = 1.0f / ( (float)size * MAP_SCALE_X ) ;
	size = DG_FTOI( (float)size * MAP_SCALE_X ) + 1 ;
#else
	size = DG_FTOI( (float)size * MAP_SCALE_X ) ;
	size_inv = 1.0f / (float)size  ;
	size += 1 ;
#endif

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
			d = work->sub_density_map[ index ] ;
			r = 1.0f - DG_SQRT( (float)i * i + j * j ) * size_inv ;
			r = DG_MAX( r, 0.0f );
			d += r * 192 ;
			if ( d > 255 ) d = 255 ;
			work->sub_density_map[ index ] = d ;
		}
	}
}

