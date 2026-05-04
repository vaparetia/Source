/*
	tst_ptch.c
	パッチ曲面オブジェクト表示実験プログラム

	2000/07/17 K.Takabe
	$Id: tst_ptch.c,v 1.1.1.3 2002/11/19 11:51:32 Yoshizawa1 Exp $

*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"


#include	"utl_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define _RND(n)	( ( (rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 32 )				/* Ｘ頂点数 */
#define MESH_Z	( 32 )				/* Ｚ頂点数 */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( 4000 )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( 4000 )	/* メッシュ間隔（Ｚ軸方向） */
#define MESH_HEIGHT		( -6000 )	/* メッシュを展開するＹ座標 */

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))


/* ---------------------------------------------------------------- */
typedef struct _wave_work{
	short	count ;
	short	dcount ;
	short	rand0 ;
	short	rand1 ;
} WaveWork;

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT		actor ;

	FMATRIX		light[2] ;
	DG_PATCH	*patch ;
	int			map ;
	int			tri_id ;
	int			tex_id ;
	WaveWork	waves[ MESH_X * MESH_Z ] ;
	float		tex_scroll ;

	int			count ;
} Work ;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/
/* 頂点データ初期化 */
static void InitVertex( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int				i, j ;

	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->pos.vx = j * MESH_WIDTH_X - ( MESH_X * MESH_WIDTH_X ) / 2 + ( waves->rand0 * 500 >> 15 ) - 250 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_Z * MESH_WIDTH_Z ) / 2 + ( waves->rand1 * 500 >> 15 ) - 250 ;
			verts->pos.vy = MESH_HEIGHT ;
			verts->pos.vw = 1.0f ;
			verts->uv.vx = j / 4.0f ;
			verts->uv.vy = i / 4.0f  ;
			verts->uv.vz = 1.0f ;
			verts->uv.vw = 0.0f ;
			verts++ ;
			waves++ ;
		}
	}
}
/* 頂点間の傾き計算 */
static void CalcVertsInclination( Work *work, DG_PATCH_VERT *mesh_verts )
{
	int		x, z ;

	/* メッシュの補間パラメータ計算処理 */
	for ( z = 0 ; z < MESH_Z ; z++ ){
		for ( x = 0 ; x < MESH_X ; x++ ){
			DG_PATCH_VERT	*verts, *prev, *next ;
			verts = &mesh_verts[ x + z * ( MESH_DIV_X + 1 ) ] ;
			if ( x == 0 ){
				prev = &mesh_verts[ ( x - 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
			} else if ( x == MESH_DIV_X ){
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
			} else {
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
				_sceVu0ScaleVector( &verts->pos_ds, &verts->pos_ds, 0.5f );	
			}
			if ( z == 0 ){
				prev = &mesh_verts[ x + ( z - 0 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
			} else if ( z == MESH_DIV_Z ){
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 0 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
			} else {
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
				_sceVu0ScaleVector( &verts->pos_dt, &verts->pos_dt, 0.5f );	
			}
		}
	}
}
/* 頂点アニメーション処理 */
static void ActVertex( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int			i, j ;
	float		angle ;
	FVECTOR		tmp ;

	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			angle = ( waves->count - 2048 ) / 2048.0f * 3.14159265f ;
			tmp.vx = angle ;
			MT_SinX4( &tmp, &tmp );
			verts->pos.vy = MESH_HEIGHT + tmp.vx * 1000.f ;
			//verts->pos.vx = j * MESH_WIDTH_X - ( MESH_X * MESH_WIDTH_X ) / 2 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_Z * MESH_WIDTH_Z ) / 2 + tmp.vx * 1000.f ;
			verts->uv.vx = j / 4.0f ;
			verts->uv.vy = i / 4.0f - work->tex_scroll + tmp.vx * 0.25f * 0.25f ;
			//verts->uv.vz = 1.0f + tmp.vx * 0.01f ;
			waves->count = ( waves->count + waves->dcount ) & 4095 ;
			verts++ ;
			waves++ ;
		}
	}
	work->tex_scroll += 0.0050f ;
	if ( work->tex_scroll > 1.0f ) work->tex_scroll -= 1.0f ;
}


/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	DG_SwitchBuffPatch( work->patch );
	ActVertex( work, work->patch->verts[ work->patch->buffer_clock ] );
	CalcVertsInclination( work, work->patch->verts[ work->patch->buffer_clock ] );
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	DG_DequeuePatchObjs( work->patch );
	DG_FreePatch( work->patch );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex ;
	DG_PATCH	*patch ;
	int		i, j, k ;

	work->map = where ;
	GM_CurrentMap = where ;

	work->tri_id = TRI_ID ;
	work->tex_id = TEX_ID ;

	{/* 波頂点アニメーションパラメータ初期化 */
		WaveWork	*waves ;
		waves = work->waves ;
		for ( i = MESH_X * MESH_Z ; i > 0 ; waves++, i-- ){
			waves->count = _RND( 4096 );
			waves->dcount = _RND( 10 ) + 22 ;
			waves->rand0 = _RND( 32768 );
			waves->rand1 = _RND( 32768 );
		}
	}

	/* テクスチャ取得 */
	tex = DG_GetTexture2( work->tri_id, work->tex_id );
	/* 強制的にリピートテクスチャにする */
	tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, 0x7f, 0x00, 0x7f, 0x00 );

	/* オブジェクト作成 */
	work->patch = patch = DG_MakePatchMesh( 0, MESH_X, MESH_Z );
	DG_ConfigPatchLOD( patch, 12 );
	patch->tri_id = work->tri_id ;
	patch->tex = tex ;
	DG_QueuePatchObjs( patch );

	/* 頂点初期化 */
	InitVertex( work, patch->verts[ 0 ] );
	InitVertex( work, patch->verts[ 1 ] );
	CalcVertsInclination( work, patch->verts[ 0 ] );
	CalcVertsInclination( work, patch->verts[ 1 ] );
	DG_SetupPatchMeshBounding( patch );

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動キャラ定義
	*/
void *NewPatchTestProgram( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
