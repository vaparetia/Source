//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xlight.c
	光源データ管理ルーチン

	2002/03/13 K.Takabe
	$Id: xlight.c,v 1.1.1.3 2002/11/19 11:42:35 Yoshizawa1 Exp $

*/
/*
	光源データ管理ルーチン

	void		DG_InitLightSystem()

		モジュールリセット


	void		DG_SetAmbient( r, g, b )
	int		r, g, b ;	ＲＧＢ色

		環境光をセット


	void		DG_SetMainLightDir( x, y, z )
	int		x, y, z ;	方向ベクトル

		主光源の方向をセット


	void		DG_SetMainLightCol( r, g, b )
	int		r, g, b ;	ＲＧＢ色

		主光源の色をセット


	void	DG_SetFixedLightMap( LIT_DEF *light, int map_id )
	LIT_DEF	*light ;	光源データ
	int		map_id ;	設定マップＩＤ

		固定光源データをセット


	LIT_DEF*	DG_GetFixedLight( int map_id )
	int		map_id ;	取得マップＩＤ

		現在のマップＩＤに対応する固定光源データを取得する
		map_id は GM_GetDGGroupID( map )などとして求めること

	------------------------------------------------

	void		DG_ClearTmpLight()

		フレーム開始処理
		バッファリングされた一時光源をクリアする

	int		DG_SetTmpLight( point, r_range, e_range )
	FVECTOR		*point ;	光源中心位置
	float		r_range ;	明度基準キョリ
	float		e_range ;	最大到達キョリ

		一時光源をセット


	int		DG_SetTmpLight2( point, r_range, e_range, color, flag )
	FVECTOR		*point ;	光源中心位置
	float		r_range ;	明度基準キョリ（r_range*2で明るさ０、r_rangeで明るさが０．７５になる）
	float		e_range ;	光源有効ボックスの大きさ（r_range*2が理想）
	int			color ;		色
	int			flag ;		光源フラグ（LIT_FLAG_BGONLY、LIT_FLAG_CHARAONLY）

		一時光源をセット２
		光源は距離がr_range*2の場所で明るさが０となり、r_rangeで明るさが０．７５、
		距離０付近で１．５倍になる。


	int		DG_SetTmpSpotLight( *point, *dir, range, umbra, penumbra, color, flag )
	FVECTOR		*point ;	スポット中心位置
	FVECTOR		*dir ;		スポット先端位置
	float		range ;		
	float		umbra ;		
	float		penumbra ;	
	int			color ;		
	int			flag ;

		一時光源をセット２
		光源は距離がr_range*2の場所で明るさが０となり、r_rangeで明るさが０．７５、
		距離０付近で１．５倍になる。

	------------------------------------------------

	int		DG_GetLightMatrix( pos, light )
	FVECTOR		*pos ;		測定位置
	FMATRIX		*light ;	光源マトリクス×２（出力）

		測定位置での光源マトリクスを計算


	void		DG_SetLightMatrix( objs, light )
	DG_OBJS		*objs ;		物体ハンドラ
	FMATRIX		*light ;	光源マトリクス×２

		物体固有の光源を持たせる
		デフォルトでは平行光源のみ
*/

#ifdef KP_XBOX //BP


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

/*----------------------------------------------------------------*/

	/*
		ローカルライトマトリクス（デフォルト）
	*/
FMATRIX		DG_LightMatrix = {
	{
		{ 0.347F, 0.0F, 0.0F, 0.0F },	/* Ｘ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
		{-0.625F, 0.0F, 0.0F, 0.0F },	/* Ｙ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
		{ 0.694F, 0.0F, 0.0F, 0.0F },	/* Ｚ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
		{ 0.0F, 0.0F, 0.0F, 0.0F }	/* 未使用 */
	}
} ;

	/*
		ローカルカラーマトリクス（デフォルト）
	*/
FMATRIX		DG_ColorMatrix = {
	{
		{64.0F, 64.0F, 96.0F, 0.0F },	/* 平行光源カラー */
		{0.0F, 0.0F, 0.0F, 0.0F },	/* 点光源１カラー */
		{0.0F, 0.0F, 0.0F, 0.0F },	/* 点光源２カラー */
		{24.0F, 32.0F, 46.0F, 0.0F }		/* 環境光カラー */
	}
} ;

/*----------------------------------------------------------------*/

//#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#define FABS(_x) DG_FABS(_x)

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

/*----------------------------------------------------------------*/

	/*
		固定光源バッファ（通常はロードデータを参照）
	*/

typedef struct {
	int		map_id ;
	int		flag ;
	int		pad ;
	LIT_DEF *FixedLights;
} FIXLIGHT;

#define MAX_FIX_LIGHTS 16

static FIXLIGHT fix_lights[ MAX_FIX_LIGHTS ];

/* 固定光源バッファ中の管理フラグ */
enum {
	FIXLIGHT_CHANGE		= 0x00000001,	/* 光源状態変化 */
} ;


	/*
		一時光源バッファ
	*/
DG_TLIGHT	DG_TLights[ 2 ] ;
int			DG_LightClock ;
static FVECTOR	max_vec = {99999999,99999999,99999999,0},min_vec = {-99999999,-99999999,-99999999,0};

/*----------------------------------------------------------------*/
static inline void CopyMatrixColumn( FMATRIX *mat, int to, int from )
{
	mat->m[0][to] = mat->m[0][from] ;
	mat->m[1][to] = mat->m[1][from] ;
	mat->m[2][to] = mat->m[2][from] ;
	mat->m[3][to] = mat->m[3][from] ;
}

static inline void CopyMatrixLine( FMATRIX *mat, int to, int from )
{
	*(u_long128*)&mat->m[to][0] = *(u_long128*)&mat->m[from][0] ;
}

static inline void BoundResize( FVECTOR *max, FVECTOR *min, FVECTOR *vec )
{
#if 1
	max->vx = DG_MAX( max->vx, vec->vx );
	max->vy = DG_MAX( max->vy, vec->vy );
	max->vz = DG_MAX( max->vz, vec->vz );
	min->vx = DG_MIN( min->vx, vec->vx );
	min->vy = DG_MIN( min->vy, vec->vy );
	min->vz = DG_MIN( min->vz, vec->vz );
#else
#endif
}

/*----------------------------------------------------------------*/

void		DG_InitLightSystem()
{
	int		i, j ;
/*	N_FixedLights = 0 ;*/
	DG_LightClock = 0 ;
	DG_ResetFixedLight();
	for ( i = 0 ; i < 2 ; i++ ){
		DG_TLights[ i ].TmpLightDef.n_lit_group = 3 ;
		for ( j = 0 ; j < 3 ; j++ ){
			DG_TLights[ i ].TmpLightGrp[ j ].n_lights = 0 ;
			DG_TLights[ i ].TmpLightGrp[ j ].bound_max = min_vec ;
			DG_TLights[ i ].TmpLightGrp[ j ].bound_min = max_vec ;
		}
	}
	for ( i = 0 ; i < 2 ; i++ ){
		DG_TLights[ i ].TmpLightGrp[0].lit = DG_TLights[ 0 ].points ;
		DG_TLights[ i ].TmpLightGrp[0].type = LIT_TYPE_POINT ;
		DG_TLights[ i ].TmpLightGrp[1].lit = DG_TLights[ 0 ].spots ;
		DG_TLights[ i ].TmpLightGrp[1].type = LIT_TYPE_SPOT ;
		DG_TLights[ i ].TmpLightGrp[2].lit = DG_TLights[ 0 ].blacks ;
		DG_TLights[ i ].TmpLightGrp[2].type = LIT_TYPE_BLACKPOINT ;
	}
	DG_SetAmbient( 0, 0, 0 ) ;
}

void		DG_SetAmbient( int r, int g, int b )
{
	ASSERT( r < 256 ) ;
	ASSERT( g < 256 ) ;
	ASSERT( b < 256 ) ;
	DG_ColorMatrix.m[3][0] = (float)r ;
	DG_ColorMatrix.m[3][1] = (float)g ;
	DG_ColorMatrix.m[3][2] = (float)b ;
	//gte_SetBackColor( r, g, b ) ;
}

void		DG_SetMainLightDir( int x, int y, int z )
{
	FVECTOR		vec ;

	vec.vx = (float)x ;
	vec.vy = (float)y ;
	vec.vz = (float)z ;
	_sceVu0Normalize( &vec, &vec );
	DG_LightMatrix.m[ 0 ][ 0 ] = vec.vx ;
	DG_LightMatrix.m[ 1 ][ 0 ] = vec.vy ;
	DG_LightMatrix.m[ 2 ][ 0 ] = vec.vz ;
}

void		DG_SetMainLightCol( int r, int g, int b )
{
	DG_ColorMatrix.m[ 0 ][ 0 ] = (float)r ;
	DG_ColorMatrix.m[ 0 ][ 1 ] = (float)g ;
	DG_ColorMatrix.m[ 0 ][ 2 ] = (float)b ;
}

/*----------------------------------------------------------------*/
	/*
		固定光源データを初期化する
	*/
void DG_ResetFixedLight( void )
{
	int i;
	FIXLIGHT *p;

	p = fix_lights;

	for( i = MAX_FIX_LIGHTS; i > 0; i--, p++ ){
		p->map_id = 0 ;
		p->flag = 0 ;
		p->FixedLights = NULL;
	}
}

	/*
		固定光源データを登録する（マップ指定付き）
	*/
void		DG_SetFixedLightMap( LIT_DEF *light, int map_id )
{
	void DG_LightFlagClear( LIT_DEF *lit_def );
	int i;
	FIXLIGHT *p;
	/* 空き探索:同じものが設定されていたらスキップ */
	/* MAX_FIX_LIGHTS - 1 以上は検索されない */
	p = fix_lights;

	for( i = MAX_FIX_LIGHTS - 1; i > 0; i--, p++ ){
		if( p->FixedLights == light ){
			p->map_id |= map_id ;
			return;
		}
		if( p->FixedLights == NULL ){
			p->map_id = map_id ;
			p->flag = 0 ;
			p->FixedLights = light ;
			DG_LightFlagClear( light );
			DG_ColorMatrix.m[ 3 ][ 0 ] = (float)light->ambient.r ;
			DG_ColorMatrix.m[ 3 ][ 1 ] = (float)light->ambient.g ;
			DG_ColorMatrix.m[ 3 ][ 2 ] = (float)light->ambient.b ;
			DG_LightMatrix.m[ 0 ][ 0 ] = light->dir.vx ;
			DG_LightMatrix.m[ 1 ][ 0 ] = light->dir.vy ;
			DG_LightMatrix.m[ 2 ][ 0 ] = light->dir.vz ;
			DG_LightMatrix.m[ 3 ][ 0 ] = light->dir.vw ;
			DG_ColorMatrix.m[ 0 ][ 0 ] = (float)light->color.r ;
			DG_ColorMatrix.m[ 0 ][ 1 ] = (float)light->color.g ;
			DG_ColorMatrix.m[ 0 ][ 2 ] = (float)light->color.b ;
			return;
		}
	}
}

#if 0
	/*
		固定光源データを登録する
	*/
void		DG_SetFixedLight( LIT_DEF *light )
{
	//DG_SetFixedLightMap( light, 0x7fffffff );
	DG_SetFixedLightMap( light, 0x00000000 );
}
#endif

	/*
		現在のマップＩＤに対応する固定光源データを取得する
		map_id は GM_GetDGGroupID( map )などとして求めること
	*/
LIT_DEF*	DG_GetFixedLight( int map_id )
{
	int i;
	FIXLIGHT *p;

	p = fix_lights;
	for( i = MAX_FIX_LIGHTS - 1; i > 0; i--, p++ ){
		if( p->map_id & map_id ){
			return ( p->FixedLights );
		}
	}
	return ( NULL );
}

	/*
		ライトデータの無効フラグをクリアする
	*/
void DG_LightFlagClear( LIT_DEF *lit_def )
{
	LIT_GRP	*lit_grp ;
	int		i, j ;

	if ( lit_def == NULL ) return ;
	lit_grp = lit_def->grp ;
	for ( i = lit_def->n_lit_group ; i > 0 ; i--, lit_grp++ ){
		/* きちんとダイナミックフラグを見るように改良したほうがいいかも・・・ */
		if ( lit_grp->type & LIT_TYPE_POINT ){
			/* 点光源について処理 */
			LIT_POINT	*light ;
			light = lit_grp->lit ;
			for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
				light->flag &= ~LIT_FLAG_DISABLE ;
			}
		} else if ( lit_grp->type & LIT_TYPE_POINT ){
			/* スポット光源について処理 */
			LIT_SPOT	*light ;
			light = lit_grp->lit ;
			for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
				light->flag &= ~LIT_FLAG_DISABLE ;
			}
		} else if ( lit_grp->type & LIT_TYPE_LINE ){
			/* 線光源について処理 */
			LIT_LINE	*light ;
			light = lit_grp->lit ;
			for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
				light->flag &= ~LIT_FLAG_DISABLE ;
			}
		} else if ( lit_grp->type & LIT_TYPE_BLACKPOINT ){
			/* 黒点光源について処理 */
			LIT_BLACKPOINT	*light ;
			light = lit_grp->lit ;
			for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
				light->flag &= ~LIT_FLAG_DISABLE ;
			}
		}

	}
}

/*----------------------------------------------------------------*/
	/*
		一時光源バッファをクリアする（フレーム毎に実行）
	*/
void		DG_ClearTmpLight()
{
	DG_TLIGHT		*tlp ;

	if ( GV_PauseLevel != 0 ) return ;
	DG_LightClock = 1 - DG_LightClock ;
	tlp = DG_TLights + DG_LightClock ;
	tlp->TmpLightGrp[0].n_lights = 0 ;
	tlp->TmpLightGrp[1].n_lights = 0 ;
	tlp->TmpLightGrp[2].n_lights = 0 ;
}

	/*
		一時点光源（白色固定）をセットする
	*/
int		DG_SetTmpLight( FVECTOR *point, float r_range, float e_range )
{
	DG_TLIGHT		*tlp ;
	LIT_GRP		*grp ;
	LIT_POINT	*lit ;
	FVECTOR		max, min ;
	int		nl ;

	tlp = DG_TLights + DG_LightClock ;
	grp = &tlp->TmpLightGrp[0] ;
	nl = grp->n_lights ;
	if ( nl >= MAX_TMPLIGHTS ) return -1 ;
	grp->n_lights = nl + 1 ;
	lit = ( (LIT_POINT*)grp->lit ) + nl ;
	lit->point = *point ;
	lit->point.vw = 1.5f ;
	lit->r_range = r_range ;
	lit->e_range = e_range ;
	lit->flag = LIT_FLAG_CHARAONLY ;
	*(int*)&lit->color = 0x00ffffff ;
	/* バウンディング設定 */
	max.vx = point->vx + e_range ;
	max.vy = point->vy + e_range ;
	max.vz = point->vz + e_range ;
	min.vx = point->vx - e_range ;
	min.vy = point->vy - e_range ;
	min.vz = point->vz - e_range ;
	BoundResize( &grp->bound_max, &grp->bound_min, &max );
	BoundResize( &grp->bound_max, &grp->bound_min, &min );
	return 0 ;
}

	/*
		一時点光源をセットする
	*/
int		DG_SetTmpLight2( FVECTOR *point, float r_range, float e_range, int color, int flag )
{
	DG_TLIGHT		*tlp ;
	LIT_GRP		*grp ;
	LIT_POINT	*lit ;
	FVECTOR		max, min ;
	int		nl ;

	tlp = DG_TLights + DG_LightClock ;
	grp = &tlp->TmpLightGrp[0] ;
	nl = grp->n_lights ;
	if ( nl >= MAX_TMPLIGHTS ) return -1 ;
	grp->n_lights = nl + 1 ;
	lit = ( (LIT_POINT*)grp->lit ) + nl ;
	lit->point = *point ;
	lit->point.vw = 1.5f ;
	lit->r_range = r_range ;
	lit->e_range = e_range ;
	lit->flag = flag ;
	*(int*)&lit->color = color ;
	/* バウンディング設定 */
	max.vx = point->vx + e_range ;
	max.vy = point->vy + e_range ;
	max.vz = point->vz + e_range ;
	min.vx = point->vx - e_range ;
	min.vy = point->vy - e_range ;
	min.vz = point->vz - e_range ;
	BoundResize( &grp->bound_max, &grp->bound_min, &max );
	BoundResize( &grp->bound_max, &grp->bound_min, &min );
	return 0 ;
}

	/*
		一時スポット光源をセットする
	*/
int		DG_SetTmpSpotLight( FVECTOR *point, FVECTOR *dir, float range, float umbra, float penumbra, int color, int flag )
{
	DG_TLIGHT		*tlp ;
	LIT_GRP		*grp ;
	LIT_SPOT	*lit ;
	//FVECTOR		max, min ;
	int		nl ;
	float	r ;

	tlp = DG_TLights + DG_LightClock ;
	grp = &tlp->TmpLightGrp[1] ;
	nl = grp->n_lights ;
	if ( nl >= MAX_TMPLIGHTS ) return -1 ;
	grp->n_lights = nl + 1 ;
	lit = ( (LIT_SPOT*)grp->lit ) + nl ;
	lit->point = *point ;
	lit->point.vw = 1.5f ;
	_sceVu0Normalize( &lit->dir, dir );
	lit->dir.vw = range ;
	lit->umbra = umbra ;
	lit->penumbra = penumbra ;
	lit->flag = flag ;
	*(int*)&lit->color = color ;
	/* バウンディング設定 */
#if 0
	/* バウンディング非対応バージョン */
	max.vx = point->vx + range * 2 ;
	max.vy = point->vy + range * 2 ;
	max.vz = point->vz + range * 2 ;
	min.vx = point->vx - range * 2 ;
	min.vy = point->vy - range * 2 ;
	min.vz = point->vz - range * 2 ;
	BoundResize( &grp->bound_max, &grp->bound_min, &max );
	BoundResize( &grp->bound_max, &grp->bound_min, &min );
#else
	/* バウンディング対応バージョン */
#if 1
	r = bp_sqrtf( 1.0f - penumbra * penumbra ) * range * 2 ; //BP_MATH - emulate PS2 sqrtf
	{
		FVECTOR	end_pos ;
		end_pos.vx = point->vx + lit->dir.x * range * 2 ;
		end_pos.vy = point->vy + lit->dir.y * range * 2 ;
		end_pos.vz = point->vz + lit->dir.z * range * 2 ;
#if 0
		lit->bound_max.vx = DG_MAX( lit->point.vx, end_pos.vx ) + r ;
		lit->bound_max.vy = DG_MAX( lit->point.vy, end_pos.vy ) + r ;
		lit->bound_max.vz = DG_MAX( lit->point.vz, end_pos.vz ) + r ;
		lit->bound_min.vx = DG_MIN( lit->point.vx, end_pos.vx ) - r ;
		lit->bound_min.vy = DG_MIN( lit->point.vy, end_pos.vy ) - r ;
		lit->bound_min.vz = DG_MIN( lit->point.vz, end_pos.vz ) - r ;
#endif
		lit->bound_max.vx = end_pos.vx + r ;
		lit->bound_max.vy = end_pos.vy + r ;
		lit->bound_max.vz = end_pos.vz + r ;
		lit->bound_min.vx = end_pos.vx - r ;
		lit->bound_min.vy = end_pos.vy - r ;
		lit->bound_min.vz = end_pos.vz - r ;
		BoundResize( &lit->bound_max, &lit->bound_min, &lit->point );
	}
#else
	DG_ADDA( 1.0f, 0.0f );
	r = DG_SQRT( DG_MSUB( penumbra, penumbra ) );
	asm( "
		mfc1			$8,%4			# 
		lqc2			vf04,0x00(%0)	# vf04 = 始点
		lqc2			vf05,0x00(%1)	# vf05xyz = 方向単位ベクトル, vf05w = 距離（の１／２）
		qmtc2			$8,vf01		# vf01x = スポット照射面の距離比（sin(angle)）
		vaddax.xyz		ACC ,vf04,vf00	# ACC = 始点
		vmaddaw.xyz		ACC ,vf05,vf05	# ACC = 始点 + 方向 * 距離 / 2
		vmaddw.xyz		vf05,vf05,vf05	# vf05 = 終点
		vmulaw.x		ACC ,vf01,vf05	# ACC = sin(angle) * 距離 / 2
		vmaddw.x		vf01,vf01,vf05	# vf01x = スポット最大有効半径
		vmax.xyz		vf06,vf04,vf05	# vf06 = バウンディング最大値
		vmini.xyz		vf07,vf04,vf05	# vf07 = バウンディング最小値
		vaddx.xyz		vf06,vf06,vf01	# vf06 = バウンディング最大値（スポット半径補正あり）
		vsubx.xyz		vf07,vf07,vf01	# vf07 = バウンディング最小値（スポット半径補正あり）
		sqc2			vf06,0x00(%2)	# 
		sqc2			vf07,0x00(%3)	# 
	"::"r"(&lit->point),"r"(&lit->dir),"r"(&lit->bound_max),"r"(&lit->bound_min),"f"(r):"$8");
#endif
	//printf("%7f %7f %7f %7f %7f %7f\n",
	//	   lit->bound_max.vx - lit->point.vx, lit->bound_max.vy - lit->point.vy, lit->bound_max.vz - lit->point.vz,
	//	   lit->bound_min.vx - lit->point.vx, lit->bound_min.vy - lit->point.vy, lit->bound_min.vz - lit->point.vz );
	BoundResize( &grp->bound_max, &grp->bound_min, &lit->bound_max );
	BoundResize( &grp->bound_max, &grp->bound_min, &lit->bound_min );
#endif
	return 0 ;
}

/*----------------------------------------------------------------*/
	/*
		光源ベクトルを算出（内部使用）
	*/
static inline float	GetLightVector( FVECTOR *vec, float r_range, FVECTOR *light )
{
	float		sq, sq2, pw, rr, rrrr ;

#if 0
	rr = vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz ;
#else
	DG_MULA( vec->vx, vec->vx );
	DG_MADDA( vec->vy, vec->vy );
	rr = DG_MADD( vec->vz, vec->vz );
#endif
	rrrr = r_range * r_range ;
	if ( rr >= rrrr ) return (0.0F);
	sq = bp_sqrtf( rr ) ;   //BP_MATH - emulate PS2 sqrtf
	if ( sq < 1.0F ) sq = 1.0F ;
	sq2 = r_range - sq ;
	if ( sq2 < 0.0F ) return (0.0F) ;
	pw = sq2 / r_range ;
	rrrr = 1.0F / sq ;
	light->vx = vec->vx * rrrr ;
	light->vy = vec->vy * rrrr ;
	light->vz = vec->vz * rrrr ;
	return (pw) ;
}

	/*
		登録されている光源データから一番影響の強い２つを平行光源データとして取得する
		残りの１つはステージ固有の平行光源が必ず最後に入る
	*/
static int		GetLightMatrix( FVECTOR *pos, FMATRIX *light, int fix_flag )
{
	FVECTOR		fvec ;
	int		offset ;
	int		i, j, max_lit ;
	float		r, d, p ;
	FIXLIGHT *flp;
	LIT_DEF	*def ;
	LIT_GRP	*grp ;
	LIT_BLACKPOINT	*black = NULL ;
	int			n_group, map ;

	light[0] = DG_LightMatrix ;
	light[1] = DG_ColorMatrix ;
	max_lit = 0 ;
	flp = fix_lights;

	if ( fix_flag != 0 ){
		/* 一時光源は含めない */
#if 0
		def = flp->FixedLights ;
		grp = def->grp ;
		n_group = def->n_lit_group ;
		flp++ ;
#else
		def = NULL ;
		grp = NULL ;
		n_group = -1 ;
#endif
	} else {
		/* 一時光源対応処理 */
		/*
			一回目はTmpLight, 二回目以降はFixLightについて計算。
		*/
		def = &DG_TLights[ 1 - DG_LightClock ].TmpLightDef ;
		grp = def->grp ;
		n_group = def->n_lit_group ;
	}
	for ( ;; ) {
		/* バウンディングチェック */
		if ( def != NULL &&
			grp->bound_max.vx > pos->vx && grp->bound_min.vx < pos->vx &&
			grp->bound_max.vy > pos->vy && grp->bound_min.vy < pos->vy &&
			grp->bound_max.vz > pos->vz && grp->bound_min.vz < pos->vz ){

			/* 光源の種類毎に処理 */
			if ( grp->type & LIT_TYPE_POINT ){

				/* 点光源処理 */
				LIT_POINT	*lights ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; -- j, lights ++ ) {
					if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					r = lights->e_range ;
					//r = lights->r_range * 2.0f ;
					fvec.vx = d = pos->vx - lights->point.vx ;
					if ( FABS(d) > r ) continue ;
					fvec.vy = d = pos->vy - lights->point.vy ;
					if ( FABS(d) > r ) continue ;
					fvec.vz = d = pos->vz - lights->point.vz ;
					if ( FABS(d) > r ) continue ;
					p = GetLightVector( &fvec, lights->r_range * 2, &fvec ) ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * 1.5f * lights->point.vw ;
					_sceVu0ScaleVector( &fvec, &fvec, p * 1.5f );
					if ( p <= 0.0f ) continue ;
					/* 光の強い順に並ぶようにする */
					for ( offset = 0 ; offset < max_lit ; offset++ ){
						if ( light[ 0 ].m[3][offset] < p ) break ;
					}
					if ( offset == 2 ) continue ;
					/* 弱い光をスライドさせる */
					if ( max_lit != 2 )	i = 2 ;
					else				i = 1 ;
					for ( ; i > offset ; i-- ){
						light[ 0 ].m[0][i] = light[ 0 ].m[0][i-1] ;
						light[ 0 ].m[1][i] = light[ 0 ].m[1][i-1] ;
						light[ 0 ].m[2][i] = light[ 0 ].m[2][i-1] ;
						light[ 0 ].m[3][i] = light[ 0 ].m[3][i-1] ;
						*(u_long128*)&light[ 1 ].m[i][0] = *(u_long128*)&light[ 1 ].m[i-1][0] ;
					}
					if ( max_lit < 2 ) max_lit++ ;
					light[ 0 ].m[0][offset] = fvec.vx ;
					light[ 0 ].m[1][offset] = fvec.vy ;
					light[ 0 ].m[2][offset] = fvec.vz ;
					light[ 0 ].m[3][offset] = p ;
					light[ 1 ].m[offset][0] = lights->color.r ;
					light[ 1 ].m[offset][1] = lights->color.g ;
					light[ 1 ].m[offset][2] = lights->color.b ;
				}

			} else if ( grp->type & LIT_TYPE_SPOT ){

				/* スポット光源処理 */
				LIT_SPOT	*lights ;
				float		cos ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; j--, lights++ ){
					if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					r = lights->dir.vw * 2;
#if 0
					/* バウンディングボックス非対応バージョン */
					fvec.vx = d = pos->vx - lights->point.vx ;
					if ( FABS(d) > r ) continue ;
					fvec.vy = d = pos->vy - lights->point.vy ;
					if ( FABS(d) > r ) continue ;
					fvec.vz = d = pos->vz - lights->point.vz ;
					if ( FABS(d) > r ) continue ;
#else
					/* バウンディングボックス対応バージョン */
					if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
					if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
					if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
					fvec.vx = pos->vx - lights->point.vx ;
					fvec.vy = pos->vy - lights->point.vy ;
					fvec.vz = pos->vz - lights->point.vz ;
#endif
					p = GetLightVector( &fvec, r, &fvec ) ;
					if ( p <= 0.0f ) continue ;
					cos = _sceVu0InnerProduct( &lights->dir, &fvec );
					if ( cos >= lights->umbra ) cos = 1.0f ;
					else if ( cos >= lights->penumbra ) cos = ( cos - lights->penumbra ) / ( lights->umbra - lights->penumbra ) ;
					else continue ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * cos * 1.5f * lights->point.vw ;
					_sceVu0ScaleVector( &fvec, &fvec, p );
					/* 光の強い順に並ぶようにする */
					for ( offset = 0 ; offset < max_lit ; offset++ ){
						if ( light[ 0 ].m[3][offset] < p ) break ;
					}
					if ( offset == 2 ) continue ;
					/* 弱い光をスライドさせる */
					if ( max_lit != 2 )	i = 2 ;
					else				i = 1 ;
					for ( ; i > offset ; i-- ){
						light[ 0 ].m[0][i] = light[ 0 ].m[0][i-1] ;
						light[ 0 ].m[1][i] = light[ 0 ].m[1][i-1] ;
						light[ 0 ].m[2][i] = light[ 0 ].m[2][i-1] ;
						light[ 0 ].m[3][i] = light[ 0 ].m[3][i-1] ;
						*(u_long128*)&light[ 1 ].m[i][0] = *(u_long128*)&light[ 1 ].m[i-1][0] ;
					}
					if ( max_lit < 2 ) max_lit++ ;
					light[ 0 ].m[0][offset] = fvec.vx ;
					light[ 0 ].m[1][offset] = fvec.vy ;
					light[ 0 ].m[2][offset] = fvec.vz ;
					light[ 0 ].m[3][offset] = p ;
					light[ 1 ].m[offset][0] = lights->color.r ;
					light[ 1 ].m[offset][1] = lights->color.g ;
					light[ 1 ].m[offset][2] = lights->color.b ;
				}

			} else if ( grp->type & LIT_TYPE_LINE ){

				/* 線光源処理 */
				LIT_LINE	*lights ;
				FVECTOR		point ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; j--, lights++ ){
					if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
					if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
					if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
#if 1
					/* 終点算出 */
					point.vx = lights->point.vx + lights->dir.vx * lights->dir.vw ;
					point.vy = lights->point.vy + lights->dir.vy * lights->dir.vw ;	
					point.vz = lights->point.vz + lights->dir.vz * lights->dir.vw ;
					{
						float		n1, n2, n3 ;
						/* 各点での距離算出 */
						n1 = _sceVu0InnerProduct( &lights->dir, &lights->point );
						n2 = _sceVu0InnerProduct( &lights->dir, &point );
						n3 = _sceVu0InnerProduct( &lights->dir, pos );
						if ( n3 <= n1 ){
							/* 始点位置における点光源計算を行なう */
							point = lights->point ;
						} else if ( n3 >= n2 ){
							/* 終点位置における点光源計算を行なう */
							//point = point ;
						} else {
							/* 最近点を算出し、その位置において点光源計算を行なう */
							n3 = n3 - n1 ;
							point.vx = lights->point.vx + lights->dir.vx * n3 ;
							point.vy = lights->point.vy + lights->dir.vy * n3 ;
							point.vz = lights->point.vz + lights->dir.vz * n3 ;
						}
					}
					fvec.vx = pos->vx - point.vx ;
					fvec.vy = pos->vy - point.vy ;
					fvec.vz = pos->vz - point.vz ;
#else
					asm("
						lqc2			vf04,0x00(%0)	# vf04 = start_point
						lqc2			vf06,0x00(%1)	# vf06 = pos
						lqc2			vf07,0x00(%2)	# vf07 = dir
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddw.xyz		vf05,vf07,vf07	# vf05 = end_point
						vaddy.x			vf08,vf00,vf07	# vf08x = dir.y
						vaddz.x			vf09,vf00,vf07	# vf09x = dir.z
						vmulax.x		ACC ,vf07,vf04	# 
						vmadday.x		ACC ,vf08,vf04	# 
						vmaddz.x		vf10,vf09,vf04	# vf10x = n1
						vmulax.x		ACC ,vf07,vf05	# 
						vmadday.x		ACC ,vf08,vf05	# 
						vmaddz.x		vf11,vf09,vf05	# vf11x = n2
						vmulax.x		ACC ,vf07,vf06	# 
						vmadday.x		ACC ,vf08,vf06	# 
						vmaddz.x		vf12,vf09,vf06	# vf12x = n3
					"::"r"(&lights->point),"r"(pos),"r"(&lights->dir));
					asm ("
						qmfc2			$8,vf10		# 
						qmfc2			$9,vf11		# 
						qmfc2			$10,vf12		# 
						mtc1			$8,$f0		# vf00 = n1
						mtc1			$9,$f1		# vf01 = n2
						mtc1			$10,$f2		# vf02 = n3
						c.le.s			$f2,$f0		# 
						bc1t			0f				# if ( n1 > n3 ) goto 0
						c.le.s			$f1,$f2		# 
						bc1t			1f				# if ( n2 < n3 ) goto 1
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddax.xyz		ACC ,vf07,vf12	# 
						vmsubx.xyz		vf03,vf07,vf10	# vf03 = near_point
						b				9f				# アセンブラの入れるディレイスロットに期待

0:						vmove.xyz		vf03,vf04		# vf03 = start_point
						b				9f				# アセンブラの入れるディレイスロットに期待

1:						vmove.xyz		vf03,vf05		# vf03 = end_point


9:						vsub.xyz		vf02,vf06,vf03	# vf02 = pos - point
						sqc2			vf02,0x00(%0)
						sqc2			vf03,0x00(%1)
					"::"r"(&fvec),"r"(&point):"$8","$9","$10","$f0","$f1","$f2" );
#endif
					p = GetLightVector( &fvec, lights->r_range * 2, &fvec ) ;
					if ( p <= 0.0f ) continue ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * 1.5f * lights->point.vw ;
					_sceVu0ScaleVector( &fvec, &fvec, p * 1.5f );
					/* 光の強い順に並ぶようにする */
					for ( offset = 0 ; offset < max_lit ; offset++ ){
						if ( light[ 0 ].m[3][offset] < p ) break ;
					}
					if ( offset == 2 ) continue ;
					/* 弱い光をスライドさせる */
					if ( max_lit != 2 )	i = 2 ;
					else				i = 1 ;
					for ( ; i > offset ; i-- ){
						light[ 0 ].m[0][i] = light[ 0 ].m[0][i-1] ;
						light[ 0 ].m[1][i] = light[ 0 ].m[1][i-1] ;
						light[ 0 ].m[2][i] = light[ 0 ].m[2][i-1] ;
						light[ 0 ].m[3][i] = light[ 0 ].m[3][i-1] ;
						*(u_long128*)&light[ 1 ].m[i][0] = *(u_long128*)&light[ 1 ].m[i-1][0] ;
					}
					if ( max_lit < 2 ) max_lit++ ;
					light[ 0 ].m[0][offset] = fvec.vx ;
					light[ 0 ].m[1][offset] = fvec.vy ;
					light[ 0 ].m[2][offset] = fvec.vz ;
					light[ 0 ].m[3][offset] = p ;
					light[ 1 ].m[offset][0] = lights->color.r ;
					light[ 1 ].m[offset][1] = lights->color.g ;
					light[ 1 ].m[offset][2] = lights->color.b ;
				}

			} else if ( grp->type & LIT_TYPE_BLACKPOINT ){

				/* 黒光源処理 */
				LIT_BLACKPOINT	*lights ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; ( j > 0 ) && ( black == NULL ) ; j--, lights++ ){
					if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
					if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
					if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
					black = lights ;					
				}

			}

		}
		grp++ ;
		if ( --n_group <= 0 ){
			extern int GM_CurrentMap ;
			do {//map_id support
				if ( ( def = flp->FixedLights ) == NULL ) break ;
				grp = def->grp ;
				n_group = def->n_lit_group ;
				map = flp->map_id ;
				flp++ ;
			} while ( ( GM_CurrentMap != 0 && ( map & GM_CurrentMap ) == 0 ) || ( n_group == 0 ) ) ;//map_id support
			if ( def == NULL ) break ;//map_id support
		}
	}

	/* 黒光源による減衰処理 */
	if ( black != NULL ){
		fvec.vx = pos->vx - black->point.vx ;
		fvec.vy = pos->vy - black->point.vy ;
		fvec.vz = pos->vz - black->point.vz ;
		p = GetLightVector( &fvec, black->r_range * 2, &fvec ) ;
		p = DG_MAX( p, 0.0f );
		p = 1.0f - p ;
		_sceVu0ScaleVector( &light[1].m[0][0], &light[1].m[0][0], p );
		_sceVu0ScaleVector( &light[1].m[1][0], &light[1].m[1][0], p );
		_sceVu0ScaleVector( &light[1].m[2][0], &light[1].m[2][0], p );
		_sceVu0ScaleVector( &light[1].m[3][0], &light[1].m[3][0], p );
	}

	return 2 ;
}

	/*
		登録されている光源データから一番影響の強い３つを平行光源データとして取得する
	*/
int		DG_GetLightMatrix( FVECTOR *pos, FMATRIX *light )
{
	return ( GetLightMatrix( pos, light, 0 ) );
}

	/*
		登録されている光源データから一番影響の強い３つを平行光源データとして取得する
		（一時光源含まず）
	*/
int		DG_GetLightMatrixFix( FVECTOR *pos, FMATRIX *light )
{
	return ( GetLightMatrix( pos, light, 1 ) );
}



	/*
		影フラグのついているライトを取得する（実験用）
	*/
int DG_GetShadowLight( FVECTOR *lit_pos, FVECTOR *lit_dir, float *lit_power, FVECTOR *pos, int fix_flag )
{
	FVECTOR		fvec ;
	int		j, ret = 0 ;
	float		r, d, p, max_power = 0.0f ;
	FIXLIGHT *flp;
	LIT_DEF	*def ;
	LIT_GRP	*grp ;
	int			n_group, map ;

	flp = fix_lights;

	if ( fix_flag != 0 ){
		/* 一時光源は含めない */
		def = NULL ;
		grp = NULL ;
		n_group = -1 ;
	} else {
		/* 一時光源対応処理 */
		/*
			一回目はTmpLight, 二回目以降はFixLightについて計算。
		*/
		def = &DG_TLights[ 1 - DG_LightClock ].TmpLightDef ;
		grp = def->grp ;
		n_group = def->n_lit_group ;
	}
	for ( ;; ) {
		/* バウンディングチェック */
		if ( def != NULL &&
			grp->bound_max.vx > pos->vx && grp->bound_min.vx < pos->vx &&
			grp->bound_max.vy > pos->vy && grp->bound_min.vy < pos->vy &&
			grp->bound_max.vz > pos->vz && grp->bound_min.vz < pos->vz ){

			/* 光源の種類毎に処理 */
			if ( grp->type & LIT_TYPE_POINT ){

				/* 点光源処理 */
				LIT_POINT	*lights ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; -- j, lights ++ ) {
					if ( !( lights->flag & LIT_FLAG_SHADOW ) ) continue ;
					//if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					r = lights->e_range ;
					//r = lights->r_range * 2.0f ;
					fvec.vx = d = pos->vx - lights->point.vx ;
					if ( FABS(d) > r ) continue ;
					fvec.vy = d = pos->vy - lights->point.vy ;
					if ( FABS(d) > r ) continue ;
					fvec.vz = d = pos->vz - lights->point.vz ;
					if ( FABS(d) > r ) continue ;
					p = GetLightVector( &fvec, lights->r_range * 2, &fvec ) ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					if ( p <= 0.0f ) continue ;
					p = p * 1.5f * lights->point.vw ;
					if ( p < max_power ) continue ;
					*lit_pos = lights->point ;
					*lit_dir = fvec ;
					*lit_power = p ;
					max_power = p ;
					ret = (int)lights ;
				}

			} else if ( grp->type & LIT_TYPE_SPOT ){

				/* スポット光源処理 */
				LIT_SPOT	*lights ;
				float		cos ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; j--, lights++ ){
					if ( !( lights->flag & LIT_FLAG_SHADOW ) ) continue ;
					//if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					r = lights->dir.vw * 2;
#if 0
					/* バウンディングボックス非対応バージョン */
					fvec.vx = d = pos->vx - lights->point.vx ;
					if ( FABS(d) > r ) continue ;
					fvec.vy = d = pos->vy - lights->point.vy ;
					if ( FABS(d) > r ) continue ;
					fvec.vz = d = pos->vz - lights->point.vz ;
					if ( FABS(d) > r ) continue ;
#else
					/* バウンディングボックス対応バージョン */
					if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
					if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
					if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
					fvec.vx = pos->vx - lights->point.vx ;
					fvec.vy = pos->vy - lights->point.vy ;
					fvec.vz = pos->vz - lights->point.vz ;
#endif
					p = GetLightVector( &fvec, r, &fvec ) ;
					if ( p <= 0.0f ) continue ;
					cos = _sceVu0InnerProduct( &lights->dir, &fvec );
					if ( cos >= lights->umbra ) cos = 1.0f ;
					else if ( cos >= lights->penumbra ) cos = ( cos - lights->penumbra ) / ( lights->umbra - lights->penumbra ) ;
					else continue ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * cos * 1.5f * lights->point.vw ;
					if ( p < max_power ) continue ;
					*lit_pos = lights->point ;
					*lit_dir = fvec ;
					*lit_power = p ;
					max_power = p ;
					ret = (int)lights ;
				}

			} else if ( grp->type & LIT_TYPE_LINE ){

				/* 線光源処理 */
				LIT_LINE	*lights ;
				FVECTOR		point ;
				lights = grp->lit ;
				for ( j = grp->n_lights ; j > 0 ; j--, lights++ ){
					if ( !( lights->flag & LIT_FLAG_SHADOW ) ) continue ;
					//if ( !( lights->flag & LIT_FLAG_CHARAONLY ) ) continue ;
					if ( lights->flag & LIT_FLAG_DISABLE ) continue ;
					if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
					if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
					if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
#if 1
					/* 終点算出 */
					point.vx = lights->point.vx + lights->dir.vx * lights->dir.vw ;
					point.vy = lights->point.vy + lights->dir.vy * lights->dir.vw ;	
					point.vz = lights->point.vz + lights->dir.vz * lights->dir.vw ;
					{
						float		n1, n2, n3 ;
						/* 各点での距離算出 */
						n1 = _sceVu0InnerProduct( &lights->dir, &lights->point );
						n2 = _sceVu0InnerProduct( &lights->dir, &point );
						n3 = _sceVu0InnerProduct( &lights->dir, pos );
						if ( n3 <= n1 ){
							/* 始点位置における点光源計算を行なう */
							point = lights->point ;
						} else if ( n3 >= n2 ){
							/* 終点位置における点光源計算を行なう */
							//point = point ;
						} else {
							/* 最近点を算出し、その位置において点光源計算を行なう */
							n3 = n3 - n1 ;
							point.vx = lights->point.vx + lights->dir.vx * n3 ;
							point.vy = lights->point.vy + lights->dir.vy * n3 ;
							point.vz = lights->point.vz + lights->dir.vz * n3 ;
						}
					}
					fvec.vx = pos->vx - point.vx ;
					fvec.vy = pos->vy - point.vy ;
					fvec.vz = pos->vz - point.vz ;
#else
					asm("
						lqc2			vf04,0x00(%0)	# vf04 = start_point
						lqc2			vf06,0x00(%1)	# vf06 = pos
						lqc2			vf07,0x00(%2)	# vf07 = dir
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddw.xyz		vf05,vf07,vf07	# vf05 = end_point
						vaddy.x			vf08,vf00,vf07	# vf08x = dir.y
						vaddz.x			vf09,vf00,vf07	# vf09x = dir.z
						vmulax.x		ACC ,vf07,vf04	# 
						vmadday.x		ACC ,vf08,vf04	# 
						vmaddz.x		vf10,vf09,vf04	# vf10x = n1
						vmulax.x		ACC ,vf07,vf05	# 
						vmadday.x		ACC ,vf08,vf05	# 
						vmaddz.x		vf11,vf09,vf05	# vf11x = n2
						vmulax.x		ACC ,vf07,vf06	# 
						vmadday.x		ACC ,vf08,vf06	# 
						vmaddz.x		vf12,vf09,vf06	# vf12x = n3
					"::"r"(&lights->point),"r"(pos),"r"(&lights->dir));
					asm ("
						qmfc2			$8,vf10		# 
						qmfc2			$9,vf11		# 
						qmfc2			$10,vf12		# 
						mtc1			$8,$f0		# vf00 = n1
						mtc1			$9,$f1		# vf01 = n2
						mtc1			$10,$f2		# vf02 = n3
						c.le.s			$f2,$f0		# 
						bc1t			0f				# if ( n1 > n3 ) goto 0
						c.le.s			$f1,$f2		# 
						bc1t			1f				# if ( n2 < n3 ) goto 1
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddax.xyz		ACC ,vf07,vf12	# 
						vmsubx.xyz		vf03,vf07,vf10	# vf03 = near_point
						b				9f				# アセンブラの入れるディレイスロットに期待

0:						vmove.xyz		vf03,vf04		# vf03 = start_point
						b				9f				# アセンブラの入れるディレイスロットに期待

1:						vmove.xyz		vf03,vf05		# vf03 = end_point


9:						vsub.xyz		vf02,vf06,vf03	# vf02 = pos - point
						sqc2			vf02,0x00(%0)
						sqc2			vf03,0x00(%1)
					"::"r"(&fvec),"r"(&point):"$8","$9","$10","$f0","$f1","$f2" );
#endif
					p = GetLightVector( &fvec, lights->r_range * 2, &fvec ) ;
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * 1.5f * lights->point.vw ;
					if ( p < max_power ) continue ;
					*lit_pos = point ;
					*lit_dir = fvec ;
					*lit_power = p ;
					max_power = p ;
					ret = (int)lights ;
				}

			}

		}
		grp++ ;
		if ( --n_group <= 0 ){
			extern int GM_CurrentMap ;
			do {//map_id support
				  if ( ( def = flp->FixedLights ) == NULL ) break ;
				grp = def->grp ;
				n_group = def->n_lit_group ;
				map = flp->map_id ;
				flp++ ;
			} while ( GM_CurrentMap != 0 && ( map & GM_CurrentMap ) == 0 ) ;//map_id support
			if ( def == NULL ) break ;//map_id support
		}
	}


#if 0
	/* デバッグ！！！ */
	if ( !( GV_PadData[1].status & PAD_A ) ){

		/* 線光源処理 */
		LIT_LINE	*lights, lb ;
		FVECTOR		point ;
		lb.point.vx = -4000 ;
		lb.point.vy = 5750 ;
		lb.point.vz = -18600 ;
		lb.point.vw = 1 ;
		lb.dir.vx = 1.0f ;
		lb.dir.vy = 0.0f ;
		lb.dir.vz = 0.0f ;
		lb.dir.vw = 8000.0f ;
		lb.r_range = 2000 ;
		lb.color.r = 255 ;
		lb.color.g = 255 ;
		lb.color.b = 255 ;
		lb.color.cd = 255 ;

		lights = &lb ;
		for ( i = 0 ; i < 1 ; i++ ){
			//if ( pos->vx > lights->bound_max.vx || pos->vx < lights->bound_min.vx ) continue ;
			//if ( pos->vy > lights->bound_max.vy || pos->vy < lights->bound_min.vy ) continue ;
			//if ( pos->vz > lights->bound_max.vz || pos->vz < lights->bound_min.vz ) continue ;
#if 1
			/* 終点算出 */
			point.vx = lights->point.vx + lights->dir.vx * lights->dir.vw ;
			point.vy = lights->point.vy + lights->dir.vy * lights->dir.vw ;	
			point.vz = lights->point.vz + lights->dir.vz * lights->dir.vw ;
			{
				float		n1, n2, n3 ;
				/* 各点での距離算出 */
				n1 = _sceVu0InnerProduct( &lights->dir, &lights->point );
				n2 = _sceVu0InnerProduct( &lights->dir, &point );
				n3 = _sceVu0InnerProduct( &lights->dir, pos );
				printf("%f %f %f\n", n1, n2, n3 );
				if ( n3 <= n1 ){
					/* 始点位置における点光源計算を行なう */
					point = lights->point ;
				} else if ( n3 >= n2 ){
					/* 終点位置における点光源計算を行なう */
					//point = point ;
				} else {
					/* 最近点を算出し、その位置において点光源計算を行なう */
					n3 = n3 - n1 ;
					point.vx = lights->point.vx + lights->dir.vx * n3 ;
					point.vy = lights->point.vy + lights->dir.vy * n3 ;
					point.vz = lights->point.vz + lights->dir.vz * n3 ;
				}
			}
			fvec.vx = pos->vx - point.vx ;
			fvec.vy = pos->vy - point.vy ;
			fvec.vz = pos->vz - point.vz ;
#else
					asm("
						lqc2			vf04,0x00(%0)	# vf04 = start_point
						lqc2			vf06,0x00(%1)	# vf06 = pos
						lqc2			vf07,0x00(%2)	# vf07 = dir
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddw.xyz		vf05,vf07,vf07	# vf05 = end_point
						vaddy.x			vf08,vf00,vf07	# vf08x = dir.y
						vaddz.x			vf09,vf00,vf07	# vf09x = dir.z
						vmulax.x		ACC ,vf07,vf04	# 
						vmadday.x		ACC ,vf08,vf04	# 
						vmaddz.x		vf10,vf09,vf04	# vf10x = n1
						vmulax.x		ACC ,vf07,vf05	# 
						vmadday.x		ACC ,vf08,vf05	# 
						vmaddz.x		vf11,vf09,vf05	# vf11x = n2
						vmulax.x		ACC ,vf07,vf06	# 
						vmadday.x		ACC ,vf08,vf06	# 
						vmaddz.x		vf12,vf09,vf06	# vf12x = n3
					"::"r"(&lights->point),"r"(pos),"r"(&lights->dir));
					asm ("
						qmfc2			$8,vf10		# 
						qmfc2			$9,vf11		# 
						qmfc2			$10,vf12		# 
						mtc1			$8,$f00		# vf00 = n1
						mtc1			$9,$f01		# vf01 = n2
						mtc1			$10,$f02		# vf02 = n3
						c.le.s			$f02,$f00		# 
						bc1t			0f				# if ( n1 > n3 ) goto 0
						c.le.s			$f01,$f02		# 
						bc1t			1f				# if ( n2 < n3 ) goto 1
						vaddax.xyz		ACC ,vf04,vf00	# 
						vmaddax.xyz		ACC ,vf07,vf12	# 
						vmsubx.xyz		vf03,vf07,vf10	# vf03 = near_point
						b				9f				# アセンブラの入れるディレイスロットに期待

0:						vmove.xyz		vf03,vf04		# vf03 = start_point
						b				9f				# アセンブラの入れるディレイスロットに期待

1:						vmove.xyz		vf03,vf05		# vf03 = end_point


9:						vsub.xyz		vf02,vf06,vf03	# vf02 = pos - point
						sqc2			vf02,0x00(%0)
						sqc2			vf03,0x00(%1)
					"::"r"(&fvec),"r"(&point):"$8","$9","$10","$f0","$f1","$f2" );
#endif
			p = GetLightVector( &fvec, lights->r_range * 2, &fvec ) ;
			printf("%f\n", p );
			/* 光源ベクトルに距離影響度×１．５を乗算する */
			p = p * 1.5f * lights->point.vw ;
			//if ( p < max_power ) continue ;
			*lit_pos = point ;
			*lit_dir = fvec ;
			*lit_power = p ;
			max_power = p ;
			ret = 1 ;
			
		}
	}
	printf("%f %f %f %f\n", lit_pos->vx, lit_pos->vy, lit_pos->vz, *lit_power );
	printf("%f %f %f\n", lit_dir->vx, lit_dir->vy, lit_dir->vz );
#endif

	return ret ;
}


/*----------------------------------------------------------------*/
	/*
		オブジェクトに光源マトリクスを設定する
	*/
void		DG_SetLightMatrix( DG_OBJS *objs, FMATRIX *light )
{
	int		i ; 
	DG_OBJ	*obj ;

	objs->light = light ;
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; i-- ){
		obj->light = light ;
		obj++ ;
	}
}


/*----------------------------------------------------------------*/
	/*
		光源の消失処理
		座標と半径で指定した領域内に中心があるライトを消す
		まだ線光源には未対応
	*/
void DG_SwitchLightSphere( FVECTOR *pos, float r_range, int flag )
{
	FIXLIGHT *lits ;
	LIT_DEF	*lit_def ;
	LIT_GRP	*lit_grp ;
	int		i, j, k ;
	FVECTOR	tmp_vec, max, min ;
	float	lenlen ;
	int		check_flag ;

	check_flag = ( flag ) ? 0 : LIT_FLAG_DISABLE ;

	r_range = r_range * r_range ;
	lits = fix_lights ;
	for( k = MAX_FIX_LIGHTS; k > 0; k--, lits++ ){
		if ( lits->FixedLights == NULL ) continue ;
		lit_def = lits->FixedLights ;
		lit_grp = lit_def->grp ;
		for ( i = lit_def->n_lit_group ; i > 0 ; i--, lit_grp++ ){

			max.vx = pos->vx + r_range ;
			max.vy = pos->vy + r_range ;
			max.vz = pos->vz + r_range ;
			min.vx = pos->vx - r_range ;
			min.vy = pos->vy - r_range ;
			min.vz = pos->vz - r_range ;
			if ( lit_grp->bound_max.vx < min.vx || lit_grp->bound_min.vx > max.vx ||
				lit_grp->bound_max.vy < min.vy || lit_grp->bound_min.vy > max.vy ||
				lit_grp->bound_max.vz < min.vz || lit_grp->bound_min.vz > max.vz ) continue ;

			if ( lit_grp->type & LIT_TYPE_POINT ){
				/* 点光源について処理 */
				LIT_POINT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					_sceVu0SubVector( &tmp_vec, &light->point, pos );
					lenlen = tmp_vec.vx * tmp_vec.vx + tmp_vec.vy * tmp_vec.vy + tmp_vec.vz * tmp_vec.vz ;
					if ( lenlen < r_range ){
						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_SPOT ){
				/* スポット光源について処理 */
				LIT_SPOT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					_sceVu0SubVector( &tmp_vec, &light->point, pos );
					lenlen = tmp_vec.vx * tmp_vec.vx + tmp_vec.vy * tmp_vec.vy + tmp_vec.vz * tmp_vec.vz ;
					if ( lenlen < r_range ){
						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_LINE ){
				/* スポット光源について処理 */
				LIT_LINE	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					_sceVu0SubVector( &tmp_vec, &light->point, pos );
					lenlen = tmp_vec.vx * tmp_vec.vx + tmp_vec.vy * tmp_vec.vy + tmp_vec.vz * tmp_vec.vz ;
					if ( lenlen < r_range ){
						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_BLACKPOINT ){
				/* 点光源について処理 */
				LIT_BLACKPOINT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					_sceVu0SubVector( &tmp_vec, &light->point, pos );
					lenlen = tmp_vec.vx * tmp_vec.vx + tmp_vec.vy * tmp_vec.vy + tmp_vec.vz * tmp_vec.vz ;
					if ( lenlen < r_range ){
						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			}

		}

	}
}

	/*
		光源の消失処理
		バウンディングボックスで指定した領域内に中心があるライトを消す
		まだ線光源には未対応
	*/
void DG_SwitchLightBound( FVECTOR *max, FVECTOR *min, int flag )
{
	FIXLIGHT *lits ;
	LIT_DEF	*lit_def ;
	LIT_GRP	*lit_grp ;
	int		i, j ;
	int		check_flag ;

	check_flag = ( flag ) ? 0 : LIT_FLAG_DISABLE ;

	lits = fix_lights ;
	for( j = MAX_FIX_LIGHTS; j > 0; j--, lits++ ){
		lit_def = lits->FixedLights ;		/* １フレームで最大１固定光源バッファ分しか処理しない */
		lit_grp = lit_def->grp ;
		for ( i = lit_def->n_lit_group ; i > 0 ; i--, lit_grp++ ){

			if ( lit_grp->bound_max.vx < min->vx || lit_grp->bound_min.vx > max->vx ||
				lit_grp->bound_max.vy < min->vy || lit_grp->bound_min.vy > max->vy ||
				lit_grp->bound_max.vz < min->vz || lit_grp->bound_min.vz > max->vz ) continue ;

			if ( lit_grp->type & LIT_TYPE_POINT ){
				/* 点光源について処理 */
				LIT_POINT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					if ( light->point.vx < max->vx && light->point.vx > min->vx &&
						light->point.vy < max->vy && light->point.vy > min->vy &&
						light->point.vz < max->vz && light->point.vz > min->vz ){

						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_SPOT ){
				/* スポット光源について処理 */
				LIT_SPOT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					if ( light->point.vx < max->vx && light->point.vx > min->vx &&
						light->point.vy < max->vy && light->point.vy > min->vy &&
						light->point.vz < max->vz && light->point.vz > min->vz ){

						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_LINE ){
				/* 線光源について処理 */
				LIT_LINE	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					if ( light->point.vx < max->vx && light->point.vx > min->vx &&
						light->point.vy < max->vy && light->point.vy > min->vy &&
						light->point.vz < max->vz && light->point.vz > min->vz ){

						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			} else if ( lit_grp->type & LIT_TYPE_BLACKPOINT ){
				/* 点光源について処理 */
				LIT_BLACKPOINT	*light ;

				light = lit_grp->lit ;
				for ( j = lit_grp->n_lights ; j > 0 ; j--, light++ ){
					if ( ( light->flag & LIT_FLAG_DISABLE ) == check_flag ) continue ;

					if ( light->point.vx < max->vx && light->point.vx > min->vx &&
						light->point.vy < max->vy && light->point.vy > min->vy &&
						light->point.vz < max->vz && light->point.vz > min->vz ){

						light->flag = ( light->flag & ~LIT_FLAG_DISABLE ) | check_flag ;
						/* プリシェードに影響する場合にはフラグを立てる */
						if ( light->flag & LIT_FLAG_BGONLY ){
							lit_grp->type |= LIT_TYPE_CHANGE ;
							lits->flag |= FIXLIGHT_CHANGE ;
						}
					}

				}

			}

		}

		lits->flag &= ~FIXLIGHT_CHANGE ;	/* フラグを消す */
	}
}

/*----------------------------------------------------------------*/
	/*
		光源の消失処理
		座標と半径で指定した領域内に中心があるライトを消す
		まだ線光源には未対応
	*/
void DG_DestroyLightSphere( FVECTOR *pos, float r_range )
{
	DG_SwitchLightSphere( pos, r_range, 0 );
}

	/*
		光源の消失処理
		バウンディングボックスで指定した領域内に中心があるライトを消す
		まだ線光源には未対応
	*/
void DG_DestroyLightBound( FVECTOR *max, FVECTOR *min )
{
	DG_SwitchLightBound( max, min, 0 );
}

/*----------------------------------------------------------------*/
	/*
		プリシェードの再計算が必要なオブジェクトを検索し、プリシェードをし直す
	*/
void DG_LightReshadeChanl( DG_CHANL *cp, int which )
{
	DG_OBJ_QUEUE	*queue ;
	DG_OBJS	**oque, *objs ;
	DG_OBJ_BUFFER	*obj_buff ;
	int			i, j, start_flag = 0, time ;
	FIXLIGHT *lits ;
	LIT_DEF	*lit_def = NULL ;
	FMATRIX	lit_backup[2] ;

	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->objs_buffer ;

	GV_GET_PRFC_CLOCK();
	/* 状態変化のあったライトが存在するかチェック */
	lits = fix_lights ;
	for( j = MAX_FIX_LIGHTS; j > 0; j--, lits++ ){
		if ( lits->FixedLights == NULL ) continue ;
		if ( lits->flag & FIXLIGHT_CHANGE ){
			start_flag = 1 ;
			lit_def = lits->FixedLights ;		/* １フレームで最大１固定光源バッファ分しか処理しない */
			lits->flag &= ~FIXLIGHT_CHANGE ;	/* フラグを消す */
			break ;
		}
	}
	if ( start_flag == 0 ) return ;

	lit_backup[0] = DG_LightMatrix ;
	lit_backup[1] = DG_ColorMatrix ;
	DG_ColorMatrix.m[ 3 ][ 0 ] = (float)lit_def->ambient.r ;
	DG_ColorMatrix.m[ 3 ][ 1 ] = (float)lit_def->ambient.g ;
	DG_ColorMatrix.m[ 3 ][ 2 ] = (float)lit_def->ambient.b ;
	DG_LightMatrix.m[ 0 ][ 0 ] = lit_def->dir.vx ;
	DG_LightMatrix.m[ 1 ][ 0 ] = lit_def->dir.vy ;
	DG_LightMatrix.m[ 2 ][ 0 ] = lit_def->dir.vz ;
	DG_LightMatrix.m[ 3 ][ 0 ] = lit_def->dir.vw ;
	DG_ColorMatrix.m[ 0 ][ 0 ] = (float)lit_def->color.r ;
	DG_ColorMatrix.m[ 0 ][ 1 ] = (float)lit_def->color.g ;
	DG_ColorMatrix.m[ 0 ][ 2 ] = (float)lit_def->color.b ;

	/* 状態変化のあったライトが存在した場合該当オブジェクトの再計算を行なう */
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i-- ){
		objs = *oque++ ;
		if ( !( objs->flag & DG_FLAG_PAINT ) ) continue ;
		if ( !( objs->flag & DG_FLAG_AUTOREPAINT ) ) continue ;
		if ( objs->fix_light != lit_def ) continue ;

		/* 部分再プリシェード */
		DG_MakePreshadeParts( objs, lit_def );
	}

	DG_LightMatrix = lit_backup[0] ;
	DG_ColorMatrix = lit_backup[1] ;

	/* ライトグループ内の変化フラグも解除しておく */
	for ( i = 0 ; i < lit_def->n_lit_group ; i++ ){
		lit_def->grp[ i ].type &= ~( LIT_TYPE_CHANGE ) ;
	}

	time = GV_GET_PRFC_CLOCK();
	//printf("total represhade time : %d\n", time);
	
}

#endif
