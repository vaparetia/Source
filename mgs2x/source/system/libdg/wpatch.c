/*
	xpatch.c
	チャンネル処理ユニット／パッチ曲面オブジェクト処理ルーチン

	2002/02/25 K.Takabe
	$Id: wpatch.c,v 1.16 2003/01/09 16:04:10 takaki Exp $

*/
/*
	チャンネル処理ユニット／パッチ曲面オブジェクト処理ルーチン

	このパッチ曲面オブジェクトとはグループ化された四角形パッチ曲面を
	管理するオブジェクトです。曲面生成には頂点及び各頂点での傾きベクトル
	が必要になります。


	void		DG_PatchChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされたパッチ曲面オブジェクトのパケット作成などを行なう


	DG_PATCH*	DG_MakePatch( int flag, int n_patch, int n_verts )
	int			flag ;			処理フラグ
	int			n_patch ;		パッチ曲面数
	int			chanl ;			処理チャンネル

	パッチ曲面オブジェクトを作成する（作成後はDG_QueuePatchObjs()で登録すること）
	実際にはこの関数を使用せずに下のDG_MakePatchMesh()関数を使う方いい。


	void	DG_FreePatch( DG_PATCH *patch )
	DG_PATCH	*patch ;		開放オブジェクト

	メモリを破棄する（これを呼ぶ前にDG_DequeuePatchObjs()でシステムから削除しておくこと）

	------------------------------------------------

	DG_PATCH*	DG_MakePatchMesh( int flag, int n_vert_s, int n_verts_t )
	int		flag ;			初期化フラグ
	int		n_vert_s ;		Ｓ軸方向頂点数（２以上）
	int		n_verts_t ;		Ｔ軸方向頂点数（２以上）

	メッシュの頂点数で確保＆自動初期化


	void DG_SetupPatchMeshBounding( DG_PATCH *patch )
	DG_PATCH	*patch ;		対象オブジェクト

	バウンディング及び中心座標自動設定（とても遅い）


	void DG_ConfigPatchLOD( DG_PATCH *patch, int level )
	DG_PATCH	*patch ;		対象オブジェクト
	int			level ;			ＬＯＤ基準距離（2^nで指定する）

	ＬＯＤパラメータ設定

	------------------------------------------------

	static	inline	void	DG_SwitchBuffPatch( DG_PATCH *patch )
	DG_PATCH	*patch ;		対象オブジェクト

	頂点バッファの切り替え


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"
#include	"utl_dma.h"
#include	"sse_inline.h"

#include "shader.h"

/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(3300559)	/* "patch" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(32)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	PatchPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginPatch( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &PatchPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &PatchPlugin );

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginPatch( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &PatchPlugin );
		DG_FreePlugin( &PatchPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクト登録
	*/
void DG_QueuePatchObjs( DG_PATCH *patch )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_QueueUserObject( PatchPlugin.obj_buffer, patch );
}

	/*
		オブジェクト削除
	*/
void DG_DequeuePatchObjs( DG_PATCH *patch )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( PatchPlugin.obj_buffer, patch );
}

/* ---------------------------------------------------------------- */



/*
 * patch 用頂点シェーダ
 */

static DG_VERTEXSHADER	DG_PatchVertexShader[ 4 ];
extern unsigned char VERTEX_SHADER_patch[];
extern unsigned char VERTEX_SHADER_patch_emap[];
extern unsigned char VERTEX_SHADER_patch2_emap[];

void DG_InitPatchVertexShader(void)
{
	DWORD dwPatchDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0, D3DVSDT_FLOAT3),   // position
		D3DVSD_REG(2, D3DVSDT_FLOAT3),   // normal
		D3DVSD_REG(3, D3DVSDT_D3DCOLOR), // diffuse
		D3DVSD_REG(7, D3DVSDT_FLOAT2),   // uv0
		D3DVSD_END()
	};

	if( DG_CheckPatchUseVertexShader() )
	{
		DG_MakeVertexShader( &DG_PatchVertexShader[0], VSHT_wpatch, dwPatchDecl );
		DG_MakeVertexShader( &DG_PatchVertexShader[1], VSHT_wpatch_emap, dwPatchDecl );
		DG_MakeVertexShader( &DG_PatchVertexShader[3], VSHT_wpatch2_emap, dwPatchDecl );
	}
}

void DG_ReleasePatchVertexShader(void)
{
	DG_KillVertexShader( &DG_PatchVertexShader[0] );
	DG_KillVertexShader( &DG_PatchVertexShader[1] );
	DG_KillVertexShader( &DG_PatchVertexShader[3] );
}

void DG_ReleasePatchVertexBuffer(void)
{
}

/* ---------------------------------------------------------------- */

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

typedef struct _obj_matrix{
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		option ;
} OBJ_MATRIX ;

typedef struct _matrix_packet{
	DG_DMATAG	trans_mat ;
	OBJ_MATRIX	obj_matrix ;
	DG_DMATAG	trans_tex ;
} MATRIX_PACKET ;

typedef struct _patch_packet{
	DG_DMATAG	verts_trans[4] ;
	DG_DMATAG	flag_trans ;
	struct {
		int		n_verts ;		/* ストリップ後の頂点数（内部で計算） */
		int		edge_flag ;		/* エッジフラグ */
		int		n_skip ;		/* カウンタスキップ値（内部で計算） */
		int		lod_level ;		/* ＬＯＤレベル（0~4） */
	} data ;
	DG_VIFCODE	prog_exec ;
} PATCH_PACKET ALIGN16;

typedef struct _patch_info {
	DG_PATCH_VERT	*v_list[4] ;
	int				n_verts ;
	int				edge_flag ;
	int				n_skip ;
	int				lod_level ;
} PATCH_INFO;

typedef struct _scrpad_work {
	FMATRIX		eye_pers ;
	FMATRIX		view ;
	FMATRIX		eye ;
	FMATRIX		world ;
	FMATRIX		screen ;
	FMATRIX		light[2] ;
	FMATRIX		eye_xpers ;
	FMATRIX		xpers ;
#ifdef _WINDOWS
	//FMATRIX		cp_eye ;
	FMATRIX		emv_eye_inv ;
#endif
	FVECTOR		scale ;
	FVECTOR		tmp_vec ;
	u_long128		dma_buffer[64] ;
	MATRIX_PACKET	matrix_packet ;
	PATCH_PACKET	patch_packet[2] ;
	int			buffer_switch ;
	int			last_type ;
	float		cam_screen ;
	float		cam_screen_inv ;
#ifdef LIBDG_PERFORMANCE
	int			n_verts, n_polys ;
#endif
	u_long128	local_work[0] ;
} ScrpadWork ;

int			*func_list ;




/*----------------------------------------------------------------*/
/* Ｖｕ０レジスタ使用スクラッチパッド間のみ使用可能バージョン */
#define Vu0CopyMatrix( _a, _b ) { *(_a) = *(_b) ; }

/*----------------------------------------------------------------*/
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
	FMATRIX		tmp_mat ;
	tmp_mat = *world ;
	tmp_mat.m[3][0] = 0 ;
	tmp_mat.m[3][1] = 0 ;
	tmp_mat.m[3][2] = 0 ;
	//_sceVu0MulMatrix( res_light, light, &tmp_mat );
	MulMatrix( res_light, light, &tmp_mat );
}
/*----------------------------------------------------------------*/
static int BoundCheck( FMATRIX *mat, FVECTOR *bound, FVECTOR *scale, int check_flag )
{
	int		and_flag, or_flag, flag ;
	FVECTOR		verts, tmp_v ;
	float		w, total_w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;
	total_w = 0 ;

	_SetMatrix( mat );
	for ( i = 0 ; i < 8 ; i++ ){
		verts.vx = ( i & 1 ) ? bound[0].vx : bound[1].vx ;
		verts.vy = ( i & 2 ) ? bound[0].vy : bound[1].vy ;
		verts.vz = ( i & 4 ) ? bound[0].vz : bound[1].vz ;
		verts.vw = 1.0F ;
		//_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
		_RotTrans(&tmp_v, &verts );
		total_w += tmp_v.vw ;
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		//if ( tmp_v.vz >  tmp_v.vw ) flag |= CLIP_Z0_FLAG;	/* XBOXはPS2とは違うので注意 */
		//if ( tmp_v.vz < 0.0f ) flag |= CLIP_Z1_FLAG;	/* XBOXはPS2とは違うので注意 */
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		and_flag &= flag ;
		or_flag |= flag ;

	}
	/* バウンディング中央値のＺ座標取得 */
	scale->vw = total_w / 8.0f ;

	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */

	return ( 0 );	/* 完全画面内 */

}
/*----------------------------------------------------------------*/
	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/
static int PatchObjsPacketInit( void *tag_addr, DG_CHANL *cp )
{
	return ( 0 );
}

/*----------------------------------------------------------------*/
static void InitMatrixPacket( MATRIX_PACKET *packet )
{
}
static void InitPatchPacket( PATCH_PACKET *packet )
{
}

/*----------------------------------------------------------------*/
/* t^3, t^2, t, 1.0f */
static const FVECTOR t_table[] = {
	{1.0           , 1.0       , 1.0   , 1.0},		// t=16/16
	{0.823974609375, 0.87890625, 0.9375, 1.0},		// t=15/16
	{0.669921875   , 0.765625  , 0.875 , 1.0},		// t=14/16
	{0.536376953125, 0.66015625, 0.8125, 1.0},		// t=13/16
	{0.421875      , 0.5625    , 0.75  , 1.0},		// t=12/16
	{0.324951171875, 0.47265625, 0.6875, 1.0},		// t=11/16
	{0.244140625   , 0.390625  , 0.625 , 1.0},		// t=10/16
	{0.177978515625, 0.31640625, 0.5625, 1.0},		// t= 9/16
	{0.125         , 0.25      , 0.5   , 1.0},		// t= 8/16
	{0.083740234375, 0.19140625, 0.4375, 1.0},		// t= 7/16
	{0.052734375   , 0.140625  , 0.375 , 1.0},		// t= 6/16
	{0.030517578125, 0.09765625, 0.3125, 1.0},		// t= 5/16
	{0.015625      , 0.0625    , 0.25  , 1.0},		// t= 4/16
	{0.006591796875, 0.03515625, 0.1875, 1.0},		// t= 3/16
	{0.001953125   , 0.015625  , 0.125 , 1.0},		// t= 2/16
	{0.000244140625, 0.00390625, 0.0625, 1.0},		// t= 1/16
	{0.0           , 0.0       , 0.0   , 1.0},		// t= 0/16
};

/* 3t^2, 2t, 1.0f, 0.0f */
static const FVECTOR dt_table[] = {
	{3.0       , 2.0  , 1.0, 0.0},		// t=16/16
	{2.63671875, 1.875, 1.0, 0.0},		// t=15/16
	{2.296875  , 1.75 , 1.0, 0.0},		// t=14/16
	{1.98046875, 1.625, 1.0, 0.0},		// t=13/16
	{1.6875    , 1.5  , 1.0, 0.0},		// t=12/16
	{1.41796875, 1.375, 1.0, 0.0},		// t=11/16
	{1.171875  , 1.25 , 1.0, 0.0},		// t=10/16
	{0.94921875, 1.125, 1.0, 0.0},		// t= 9/16
	{0.75      , 1.0  , 1.0, 0.0},		// t= 8/16
	{0.57421875, 0.875, 1.0, 0.0},		// t= 7/16
	{0.421875  , 0.75 , 1.0, 0.0},		// t= 6/16
	{0.29296875, 0.625, 1.0, 0.0},		// t= 5/16
	{0.1875    , 0.5  , 1.0, 0.0},		// t= 4/16
	{0.10546875, 0.375, 1.0, 0.0},		// t= 3/16
	{0.046875  , 0.25 , 1.0, 0.0},		// t= 2/16
	{0.01171875, 0.125, 1.0, 0.0},		// t= 1/16
	{0.0       , 0.0  , 1.0, 0.0},		// t= 0/16
};

static const MATRIX mat_hermite = {
	2.0f,-2.0f, 1.0f, 1.0f,
	-3.0f, 3.0f,-2.0f,-1.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 0.0f,
};

/* エルミート補間行列を生成する */
static void MakeHermiteMatrix( FMATRIX *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1 )
{
	FMATRIX	data_mat;
	*(VECTOR*)data_mat.m[0] = *q0 ;
	*(VECTOR*)data_mat.m[1] = *q1 ;
	*(VECTOR*)data_mat.m[2] = *d0 ;
	*(VECTOR*)data_mat.m[3] = *d1 ;
	//_sceVu0MulMatrix( res, &data_mat, &mat_hermite );
	MulMatrix( res, &data_mat, (FMATRIX *)&mat_hermite );
}

static void LerpVec(VECTOR *res, VECTOR *v1, VECTOR *v2, float t)
{
	res->vx = (v2->vx - v1->vx) * t + v1->vx;
	res->vy = (v2->vy - v1->vy) * t + v1->vy;
	res->vz = (v2->vz - v1->vz) * t + v1->vz;
	res->vw = (v2->vw - v1->vw) * t + v1->vw;
}


static void DrawPatch( int lod_level, int edge_flag,
					  DG_PATCH_VERT *v0, DG_PATCH_VERT *v1, DG_PATCH_VERT *v2, DG_PATCH_VERT *v3 )
{
/*
	 MGS2と同等の補間アルゴリズムを実装する
	 1.  v0->v2(t0軸), v1->v3(t1軸)の補間を行う
	 2.  S軸方向の補間を行う

	 v0_________v1 →S軸
	  |         |
	  |  patch  |
	  |         |
	  |_________|
	 v2         v3
	  ↓
	 T軸
*/
	DG_PATCHVERTEX	v[2*17] ;
#ifdef _WINDOWS
	DG_PATCHVERTEX	*vbuff ;
	DG_PATCHVERTEX	*vbuff_ptr ;
#endif
	DG_PATCHVERTEX	*write_vert ;
	int				i, j ;
	int				div_count, n_skip, n_verts ;
	FMATRIX			hermite_t0, hermite_t1, hermite_s ;
	const FVECTOR	*t_tvec, *t_dtvec, *s_tvec, *s_dtvec ;
	FVECTOR			tmp_vec ;
	DWORD			sofs ;

	div_count = 1 << ( 4 - lod_level ) ;	/* 分割数 */
	n_skip = 16 / div_count ;				/* パラメータのスキップ数 */
	n_verts = div_count * 2 + 2 ;

	MakeHermiteMatrix( &hermite_t0, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
	MakeHermiteMatrix( &hermite_t1, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );

#ifdef _WINDOWS
	vbuff     = DG_GetWorkBuffer() ;	// 作業用メモリ
	vbuff_ptr = vbuff ;
#endif
	t_tvec = t_table ;		/* Ｔ軸方向補間用補間係数取得ポインタ */
	t_dtvec = dt_table ;	/* Ｔ軸方向補間用微分済み補間係数取得ポインタ */
	for ( i = 0 ; i <= div_count ; i++, t_tvec+=n_skip, t_dtvec+=n_skip){
		/* Ｓ方向ループ */
		DG_PATCH_VERT	start, end ;

		/* T0軸頂点算出 */
		//_sceVu0ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );

		/* T0軸頂点算出 */
		ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点Ｓ座標傾き算出 */
		LerpVec( &start.pos_ds, &v0->pos_ds, &v2->pos_ds, t_tvec->vz );
		/* T0軸頂点テクスチャ座標算出 */
		LerpVec( &start.uv, &v0->uv, &v2->uv, t_tvec->vz );

		/* T1軸頂点算出 */
		//_sceVu0ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &end.pos_dt, &hermite_t0, t_dtvec );

		/* T1軸頂点算出 */
		ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &end.pos_dt, &hermite_t1, t_dtvec );
		/* T1軸頂点Ｓ座標傾き算出 */
		LerpVec( &end.pos_ds, &v1->pos_ds, &v3->pos_ds, t_tvec->vz );
		/* T1軸頂点テクスチャ座標算出 */
		LerpVec( &end.uv, &v1->uv, &v3->uv, t_tvec->vz );


		MakeHermiteMatrix( &hermite_s, &start.pos, &end.pos, &start.pos_ds, &end.pos_ds );

		s_tvec = t_table ;		/* Ｓ軸方向補間用補間係数取得ポインタ */
		s_dtvec = dt_table ;	/* Ｓ軸方向補間用微分済み補間係数取得ポインタ */
		write_vert = &v[ i & 1 ];
		for ( j = 0 ; j <= div_count ; j++, s_tvec+=n_skip, s_dtvec+=n_skip ){
			/* Ｔ方向ループ */
			/* 頂点算出 */
			//_sceVu0ApplyMatrix( &tmp_vec, &hermite_s, s_tvec );
			ApplyMatrix( &tmp_vec, &hermite_s, s_tvec );

			write_vert->v.x = tmp_vec.vx ;
			write_vert->v.y = tmp_vec.vy ;
			write_vert->v.z = tmp_vec.vz ;

			{
				FVECTOR		v0, v1 ;

				/* 頂点Ｓ軸傾き算出 */
				//_sceVu0ApplyMatrix( &v0, &hermite_s, s_dtvec );
				ApplyMatrix( &v0, &hermite_s, s_dtvec );

				/* 頂点Ｔ軸傾き算出 */
				LerpVec( &v1, &start.pos_dt, &end.pos_dt, s_tvec->vz );

				//_sceVu0OuterProduct( &tmp_vec, &v0, &v1 );
				OuterProduct( &tmp_vec, &v0, &v1 );

				/* 頂点シェーダー内で正規化を行うので必要なし */
				//_sceVu0Normalize( &tmp_vec, &tmp_vec );

				write_vert->n.x = tmp_vec.vx ;
				write_vert->n.y = tmp_vec.vy ;
				write_vert->n.z = tmp_vec.vz ;
			}

			write_vert->rgba = 0x80808080 ;

			/* 頂点テクスチャ座標算出 */
			LerpVec( &tmp_vec, &start.uv, &end.uv, s_tvec->vz );

			write_vert->tu0 = tmp_vec.vx ;
			write_vert->tv0 = tmp_vec.vy ;
			write_vert += 2 ;
		}
		/* エッジ接続フラグのチェック（Ｓ軸方向接線） */
		if (  ( ( edge_flag & 0x0008 ) && ( i == 0 ) )
		   || ( ( edge_flag & 0x0004 ) && ( i == div_count ) ) ){
			write_vert = &v[ i & 1 ];
			for ( j  = 0 ; j < div_count ; j+=2 ){
				write_vert[2] = write_vert[0];
				write_vert += 4 ;
			}
		}
		/* エッジ接続フラグのチェック（Ｔ軸方向接線） */
		if ( i & 1 ){
			if ( edge_flag & 0x0002 ){
				v[ 1 ] = v[ 0 ];
			}
			if ( edge_flag & 0x0001 ){
				v[ n_verts-1 ] = v[ n_verts-2 ];
			}
		}

		if ( i == 0 ) continue ;
		memcpy(vbuff_ptr, v, sizeof(DG_PATCHVERTEX) * n_verts) ;
		vbuff_ptr += n_verts ;
	}

	{
		DWORD	n_prims ;

		DG_SetDynamicVertexBuffer(vbuff, sizeof(DG_PATCHVERTEX),
							n_verts * div_count, &sofs) ;
		n_prims = n_verts-2 ;
		for(i=div_count; i>0; i--)	// 上処理では、i==0の時は描画していないのでi>0でOK
		{
			DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, n_prims) ;
			sofs += n_verts ;
		}
	}
}
/*----------------------------------------------------------------*/
	/*
		Vertex Shaderを使用しないバージョン
	*/

static void DrawPatchNVS_Normal( int lod_level, int edge_flag,
					  DG_PATCH_VERT *v0, DG_PATCH_VERT *v1, DG_PATCH_VERT *v2, DG_PATCH_VERT *v3 )
{
	DG_PATCHVERTEX	v[2*17] ;
#ifdef _WINDOWS
	DG_PATCHVERTEX	*vbuff ;
	DG_PATCHVERTEX	*vbuff_ptr ;
#endif
	DG_PATCHVERTEX	*write_vert ;
	int				i, j ;
	int				div_count, n_skip, n_verts ;
	FMATRIX			hermite_t0, hermite_t1, hermite_s ;
	const FVECTOR	*t_tvec, *t_dtvec, *s_tvec, *s_dtvec ;
	FVECTOR			tmp_vec ;
	DWORD			sofs ;

	div_count = 1 << ( 4 - lod_level ) ;	/* 分割数 */
	n_skip = 16 / div_count ;				/* パラメータのスキップ数 */
	n_verts = div_count * 2 + 2 ;

	MakeHermiteMatrix( &hermite_t0, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
	MakeHermiteMatrix( &hermite_t1, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );

#ifdef _WINDOWS
	vbuff     = DG_GetWorkBuffer() ;	// 作業用メモリ
	vbuff_ptr = vbuff ;
#endif
	t_tvec = t_table ;		/* Ｔ軸方向補間用補間係数取得ポインタ */
	t_dtvec = dt_table ;	/* Ｔ軸方向補間用微分済み補間係数取得ポインタ */
	for ( i = 0 ; i <= div_count ; i++, t_tvec+=n_skip, t_dtvec+=n_skip){
		/* Ｓ方向ループ */
		DG_PATCH_VERT	start, end ;

		/* T0軸頂点算出 */
		//_sceVu0ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点算出 */
		ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点Ｓ座標傾き算出 */
		LerpVec( &start.pos_ds, &v0->pos_ds, &v2->pos_ds, t_tvec->vz );
		/* T0軸頂点テクスチャ座標算出 */
		LerpVec( &start.uv, &v0->uv, &v2->uv, t_tvec->vz );

		/* T1軸頂点算出 */
		//_sceVu0ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &end.pos_dt, &hermite_t0, t_dtvec );

		/* T1軸頂点算出 */
		ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &end.pos_dt, &hermite_t1, t_dtvec );
		/* T1軸頂点Ｓ座標傾き算出 */
		LerpVec( &end.pos_ds, &v1->pos_ds, &v3->pos_ds, t_tvec->vz );
		/* T1軸頂点テクスチャ座標算出 */
		LerpVec( &end.uv, &v1->uv, &v3->uv, t_tvec->vz );


		MakeHermiteMatrix( &hermite_s, &start.pos, &end.pos, &start.pos_ds, &end.pos_ds );

		s_tvec = t_table ;		/* Ｓ軸方向補間用補間係数取得ポインタ */
		s_dtvec = dt_table ;	/* Ｓ軸方向補間用微分済み補間係数取得ポインタ */
		write_vert = &v[ i & 1 ];
		for ( j = 0 ; j <= div_count ; j++, s_tvec+=n_skip, s_dtvec+=n_skip ){
			/* Ｔ方向ループ */
			/* 頂点算出 */
			//_sceVu0ApplyMatrix( &tmp_vec, &hermite_s, s_tvec );
			ApplyMatrix( &tmp_vec, &hermite_s, s_tvec );

			write_vert->v.x = tmp_vec.vx ;
			write_vert->v.y = tmp_vec.vy ;
			write_vert->v.z = tmp_vec.vz ;

			{
				FVECTOR		v0, v1 ;

				/* 頂点Ｓ軸傾き算出 */
				//_sceVu0ApplyMatrix( &v0, &hermite_s, s_dtvec );
				ApplyMatrix( &v0, &hermite_s, s_dtvec );

				/* 頂点Ｔ軸傾き算出 */
				LerpVec( &v1, &start.pos_dt, &end.pos_dt, s_tvec->vz );

				//_sceVu0OuterProduct( &tmp_vec, &v0, &v1 );
				OuterProduct( &tmp_vec, &v0, &v1 );

				/* 正規化 */
				_sceVu0Normalize( &tmp_vec, &tmp_vec );
				write_vert->n.x = tmp_vec.vx ;
				write_vert->n.y = tmp_vec.vy ;
				write_vert->n.z = tmp_vec.vz ;
			}

			write_vert->rgba = 0x80808080 ;

			/* 頂点テクスチャ座標算出 */
			LerpVec( &tmp_vec, &start.uv, &end.uv, s_tvec->vz );

			write_vert->tu0 = tmp_vec.vx ;
			write_vert->tv0 = tmp_vec.vy ;

			write_vert += 2 ;
		}
		/* エッジ接続フラグのチェック（Ｓ軸方向接線） */
		if (  ( ( edge_flag & 0x0008 ) && ( i == 0 ) )
		   || ( ( edge_flag & 0x0004 ) && ( i == div_count ) ) ){
			write_vert = &v[ i & 1 ];
			for ( j  = 0 ; j < div_count ; j+=2 ){
				write_vert[2] = write_vert[0];
				write_vert += 4 ;
			}
		}
		/* エッジ接続フラグのチェック（Ｔ軸方向接線） */
		if ( i & 1 ){
			if ( edge_flag & 0x0002 ){
				v[ 1 ] = v[ 0 ];
			}
			if ( edge_flag & 0x0001 ){
				v[ n_verts-1 ] = v[ n_verts-2 ];
			}
		}

		if ( i == 0 ) continue ;
		memcpy(vbuff_ptr, v, sizeof(DG_PATCHVERTEX) * n_verts) ;
		vbuff_ptr += n_verts ;
	}

	{
		DWORD	n_prims ;

		DG_SetDynamicVertexBuffer(vbuff, sizeof(DG_PATCHVERTEX),
							n_verts * div_count, &sofs) ;
		n_prims = n_verts-2 ;
		for(i=div_count; i>0; i--)	// 上処理では、i==0の時は描画していないのでi>0でOK
		{
			DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, n_prims) ;
			sofs += n_verts ;
		}
	}
}

static void DrawPatchNVS_EMap( int lod_level, int edge_flag,
					  DG_PATCH_VERT *v0, DG_PATCH_VERT *v1, DG_PATCH_VERT *v2, DG_PATCH_VERT *v3 )
{
	DG_PATCHVERTEX	v[2*17] ;
#ifdef _WINDOWS
	DG_PATCHVERTEX	*vbuff ;
	DG_PATCHVERTEX	*vbuff_ptr ;
#endif
	DG_PATCHVERTEX	*write_vert ;
	int				i, j ;
	int				div_count, n_skip, n_verts ;
	FMATRIX			hermite_t0, hermite_t1, hermite_s ;
	const FVECTOR	*t_tvec, *t_dtvec, *s_tvec, *s_dtvec ;
	FVECTOR			pos ;
	FVECTOR			nrm ;
	FVECTOR			tmp_vec ;
	DWORD			sofs ;
	ScrpadWork		*scrpad ;

	scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	div_count = 1 << ( 4 - lod_level ) ;	/* 分割数 */
	n_skip = 16 / div_count ;				/* パラメータのスキップ数 */
	n_verts = div_count * 2 + 2 ;

	MakeHermiteMatrix( &hermite_t0, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
	MakeHermiteMatrix( &hermite_t1, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );

	vbuff     = DG_GetWorkBuffer() ;	// 作業用メモリ
	vbuff_ptr = vbuff ;

	t_tvec = t_table ;		/* Ｔ軸方向補間用補間係数取得ポインタ */
	t_dtvec = dt_table ;	/* Ｔ軸方向補間用微分済み補間係数取得ポインタ */
	for ( i = 0 ; i <= div_count ; i++, t_tvec+=n_skip, t_dtvec+=n_skip){
		/* Ｓ方向ループ */
		DG_PATCH_VERT	start, end ;

		/* T0軸頂点算出 */
		//_sceVu0ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点算出 */
		ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点Ｓ座標傾き算出 */
		LerpVec( &start.pos_ds, &v0->pos_ds, &v2->pos_ds, t_tvec->vz );
		/* T0軸頂点テクスチャ座標算出 */
		LerpVec( &start.uv, &v0->uv, &v2->uv, t_tvec->vz );

		/* T1軸頂点算出 */
		//_sceVu0ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &end.pos_dt, &hermite_t0, t_dtvec );

		/* T1軸頂点算出 */
		ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &end.pos_dt, &hermite_t1, t_dtvec );
		/* T1軸頂点Ｓ座標傾き算出 */
		LerpVec( &end.pos_ds, &v1->pos_ds, &v3->pos_ds, t_tvec->vz );
		/* T1軸頂点テクスチャ座標算出 */
		LerpVec( &end.uv, &v1->uv, &v3->uv, t_tvec->vz );


		MakeHermiteMatrix( &hermite_s, &start.pos, &end.pos, &start.pos_ds, &end.pos_ds );

		s_tvec = t_table ;		/* Ｓ軸方向補間用補間係数取得ポインタ */
		s_dtvec = dt_table ;	/* Ｓ軸方向補間用微分済み補間係数取得ポインタ */
		write_vert = &v[ i & 1 ];
		for ( j = 0 ; j <= div_count ; j++, s_tvec+=n_skip, s_dtvec+=n_skip ){
			/* Ｔ方向ループ */
			/* 頂点算出 */
			//_sceVu0ApplyMatrix( &pos, &hermite_s, s_tvec );
			ApplyMatrix( &pos, &hermite_s, s_tvec );

			write_vert->v.x = pos.vx ;
			write_vert->v.y = pos.vy ;
			write_vert->v.z = pos.vz ;

			{
				FVECTOR	v0, v1 ;
				float	inp ;

				/* 頂点Ｓ軸傾き算出 */
				//_sceVu0ApplyMatrix( &v0, &hermite_s, s_dtvec );
				ApplyMatrix( &v0, &hermite_s, s_dtvec );

				/* 頂点Ｔ軸傾き算出 */
				LerpVec( &v1, &start.pos_dt, &end.pos_dt, s_tvec->vz );

				//_sceVu0OuterProduct( &tmp_vec, &v0, &v1 );
				OuterProduct( &tmp_vec, &v0, &v1 );

				/* 環境マップ計算 */
				/* 法線正規化 */
				_sceVu0Normalize(&nrm, &tmp_vec) ;
				nrm.vw = 0.0f ;
				write_vert->n.x = nrm.vx ;
				write_vert->n.y = nrm.vy ;
				write_vert->n.z = nrm.vz ;

				/* カメラ空間座標系に変換 */
				ApplyMatrix(&pos, &scrpad->emv_eye_inv, &pos) ;
				ApplyMatrix(&nrm, &scrpad->emv_eye_inv, &nrm) ;

				/* 視線ベクトル計算 */
				pos.vw = 0.0f ;
				_sceVu0Normalize(&pos, &pos) ;

				/* 反射ベクトル計算 */
				inp = -2.0f * _sceVu0InnerProduct(&pos, &nrm) ;
				nrm.vx = pos.vx + (inp * nrm.vx) ;
				nrm.vy = pos.vy + (inp * nrm.vy) ;
			}

			write_vert->rgba = 0x80808080 ;

			/* 頂点テクスチャ座標算出 */
			write_vert->tu0 = nrm.vx ;
			write_vert->tv0 = nrm.vy ;
			write_vert += 2 ;
		}
		/* エッジ接続フラグのチェック（Ｓ軸方向接線） */
		if (  ( ( edge_flag & 0x0008 ) && ( i == 0 ) )
		   || ( ( edge_flag & 0x0004 ) && ( i == div_count ) ) ){
			write_vert = &v[ i & 1 ];
			for ( j  = 0 ; j < div_count ; j+=2 ){
				write_vert[2] = write_vert[0];
				write_vert += 4 ;
			}
		}
		/* エッジ接続フラグのチェック（Ｔ軸方向接線） */
		if ( i & 1 ){
			if ( edge_flag & 0x0002 ){
				v[ 1 ] = v[ 0 ];
			}
			if ( edge_flag & 0x0001 ){
				v[ n_verts-1 ] = v[ n_verts-2 ];
			}
		}

		if ( i == 0 ) continue ;
		memcpy(vbuff_ptr, v, sizeof(DG_PATCHVERTEX) * n_verts) ;
		vbuff_ptr += n_verts ;
	}

	{
		DWORD	n_prims ;

		DG_SetDynamicVertexBuffer(vbuff, sizeof(DG_PATCHVERTEX),
							n_verts * div_count, &sofs) ;
		n_prims = n_verts-2 ;
		for(i=div_count; i>0; i--)	// 上処理では、i==0の時は描画していないのでi>0でOK
		{
			DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, n_prims) ;
			sofs += n_verts ;
		}
	}
}


static void DrawPatchNVS_EMap2( int lod_level, int edge_flag,
					  DG_PATCH_VERT *v0, DG_PATCH_VERT *v1, DG_PATCH_VERT *v2, DG_PATCH_VERT *v3 )
{
	DG_PATCHVERTEX	v[2*17] ;
#ifdef _WINDOWS
	DG_PATCHVERTEX	*vbuff ;
	DG_PATCHVERTEX	*vbuff_ptr ;
#endif
	DG_PATCHVERTEX	*write_vert ;
	int				i, j ;
	int				div_count, n_skip, n_verts ;
	FMATRIX			hermite_t0, hermite_t1, hermite_s ;
	const FVECTOR	*t_tvec, *t_dtvec, *s_tvec, *s_dtvec ;
	FVECTOR			pos ;
	FVECTOR			nrm ;
	FVECTOR			tmp_vec ;
	DWORD			sofs ;
	ScrpadWork		*scrpad ;

	scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	div_count = 1 << ( 4 - lod_level ) ;	/* 分割数 */
	n_skip = 16 / div_count ;				/* パラメータのスキップ数 */
	n_verts = div_count * 2 + 2 ;

	MakeHermiteMatrix( &hermite_t0, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
	MakeHermiteMatrix( &hermite_t1, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );

	vbuff     = DG_GetWorkBuffer() ;	// 作業用メモリ
	vbuff_ptr = vbuff ;

	t_tvec = t_table ;		/* Ｔ軸方向補間用補間係数取得ポインタ */
	t_dtvec = dt_table ;	/* Ｔ軸方向補間用微分済み補間係数取得ポインタ */
	for ( i = 0 ; i <= div_count ; i++, t_tvec+=n_skip, t_dtvec+=n_skip){
		/* Ｓ方向ループ */
		DG_PATCH_VERT	start, end ;

		/* T0軸頂点算出 */
		//_sceVu0ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点算出 */
		ApplyMatrix( &start.pos, &hermite_t0, t_tvec );
		/* T0軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &start.pos_dt, &hermite_t0, t_dtvec );
		/* T0軸頂点Ｓ座標傾き算出 */
		LerpVec( &start.pos_ds, &v0->pos_ds, &v2->pos_ds, t_tvec->vz );
		/* T0軸頂点テクスチャ座標算出 */
		LerpVec( &start.uv, &v0->uv, &v2->uv, t_tvec->vz );

		/* T1軸頂点算出 */
		//_sceVu0ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		//_sceVu0ApplyMatrix( &end.pos_dt, &hermite_t0, t_dtvec );

		/* T1軸頂点算出 */
		ApplyMatrix( &end.pos, &hermite_t1, t_tvec );
		/* T1軸頂点Ｔ座標傾き算出 */
		ApplyMatrix( &end.pos_dt, &hermite_t1, t_dtvec );
		/* T1軸頂点Ｓ座標傾き算出 */
		LerpVec( &end.pos_ds, &v1->pos_ds, &v3->pos_ds, t_tvec->vz );
		/* T1軸頂点テクスチャ座標算出 */
		LerpVec( &end.uv, &v1->uv, &v3->uv, t_tvec->vz );


		MakeHermiteMatrix( &hermite_s, &start.pos, &end.pos, &start.pos_ds, &end.pos_ds );

		s_tvec = t_table ;		/* Ｓ軸方向補間用補間係数取得ポインタ */
		s_dtvec = dt_table ;	/* Ｓ軸方向補間用微分済み補間係数取得ポインタ */
		write_vert = &v[ i & 1 ];
		for ( j = 0 ; j <= div_count ; j++, s_tvec+=n_skip, s_dtvec+=n_skip ){
			/* Ｔ方向ループ */
			/* 頂点算出 */
			//_sceVu0ApplyMatrix( &pos, &hermite_s, s_tvec );
			ApplyMatrix( &pos, &hermite_s, s_tvec );

			write_vert->v.x = pos.vx ;
			write_vert->v.y = 0.0f ;	// 実際に描画する座標（Ｙ＝０）
			write_vert->v.z = pos.vz ;

			{
				FVECTOR	v0, v1 ;
				float	inp ;

				/* 頂点Ｓ軸傾き算出 */
				//_sceVu0ApplyMatrix( &v0, &hermite_s, s_dtvec );
				ApplyMatrix( &v0, &hermite_s, s_dtvec );

				/* 頂点Ｔ軸傾き算出 */
				LerpVec( &v1, &start.pos_dt, &end.pos_dt, s_tvec->vz );

				//_sceVu0OuterProduct( &tmp_vec, &v0, &v1 );
				OuterProduct( &tmp_vec, &v0, &v1 );

				/* 環境マップ計算 */
				/* 法線正規化 */
				_sceVu0Normalize(&nrm, &tmp_vec) ;
				nrm.vw = 0.0f ;
				write_vert->n.x = nrm.vx ;
				write_vert->n.y = nrm.vy ;
				write_vert->n.z = nrm.vz ;

				/* カメラ空間座標系に変換 */
				ApplyMatrix(&pos, &scrpad->emv_eye_inv, &pos) ;
				ApplyMatrix(&nrm, &scrpad->emv_eye_inv, &nrm) ;

				/* 視線ベクトル計算 */
				pos.vw = 0.0f ;
				_sceVu0Normalize(&pos, &pos) ;

				/* 反射ベクトル計算 */
				inp = -2.0f * _sceVu0InnerProduct(&pos, &nrm) ;
				nrm.vx = pos.vx + (inp * nrm.vx) ;
				nrm.vy = pos.vy + (inp * nrm.vy) ;
			}

			write_vert->rgba = 0x80808080 ;

			/* 頂点テクスチャ座標算出 */
			write_vert->tu0 = nrm.vx ;
			write_vert->tv0 = nrm.vy ;
			write_vert += 2 ;
		}
		/* エッジ接続フラグのチェック（Ｓ軸方向接線） */
		if (  ( ( edge_flag & 0x0008 ) && ( i == 0 ) )
		   || ( ( edge_flag & 0x0004 ) && ( i == div_count ) ) ){
			write_vert = &v[ i & 1 ];
			for ( j  = 0 ; j < div_count ; j+=2 ){
				write_vert[2] = write_vert[0];
				write_vert += 4 ;
			}
		}
		/* エッジ接続フラグのチェック（Ｔ軸方向接線） */
		if ( i & 1 ){
			if ( edge_flag & 0x0002 ){
				v[ 1 ] = v[ 0 ];
			}
			if ( edge_flag & 0x0001 ){
				v[ n_verts-1 ] = v[ n_verts-2 ];
			}
		}

		if ( i == 0 ) continue ;
		memcpy(vbuff_ptr, v, sizeof(DG_PATCHVERTEX) * n_verts) ;
		vbuff_ptr += n_verts ;
	}

	{
		DWORD	n_prims ;

		DG_SetDynamicVertexBuffer(vbuff, sizeof(DG_PATCHVERTEX),
							n_verts * div_count, &sofs) ;
		n_prims = n_verts-2 ;
		for(i=div_count; i>0; i--)	// 上処理では、i==0の時は描画していないのでi>0でOK
		{
			DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, n_prims) ;
			sofs += n_verts ;
		}
	}
}
/*----------------------------------------------------------------*/

static void ChainObj( DG_PATCH *patch )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
	ScrpadWork	*scrpad ;
	DG_PATCH_PARTS	*parts ;
	DG_PATCH_VERT	*verts ;
	int			i ;
	void		(*draw_ptch_func)(int,int,DG_PATCH_VERT*,DG_PATCH_VERT*,DG_PATCH_VERT*,DG_PATCH_VERT*) ;
	BOOL	use_tex_lin ;

	if( !patch->tex ){ return ; }

	scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	Vu0CopyMatrix( &scrpad->world, &patch->world );
	if ( patch->root != NULL ){
		Vu0CopyMatrix( &scrpad->world, patch->root );
		Vu0CopyMatrix( &patch->world, &scrpad->world );
	}

	_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &scrpad->world );

	/* 全体バウンディングのチェック */
	if ( BoundCheck( &scrpad->screen, &patch->max, &scrpad->scale, 1 ) == 2 ){
		return ;
	}

	/* 各種マトリクスの設定 */

	use_tex_lin = patch->flag & DG_PATCH_TEX_LIN ;
	if( !use_tex_lin )
	{
		DG_SetTextureDirect( 0, patch->tex->tex_trans.ptex );
		DG_SetAlphaMode(patch->tex->tex_trans.alpha.data);
	}
	else
	{
		/* DG_TEX_LIN形式 */
		DG_TEX_LIN	*tex_lin ;

		tex_lin = (DG_TEX_LIN *)patch->tex ;
		if( DG_LinerTextureGetImageDirty(tex_lin) )
		{
			/* イメージ更新 */
			DG_LinerTextureFlushImage(tex_lin) ;
			DG_LinerTextureClearImageDirty(tex_lin) ;
		}

		DG_SetTextureDirect(0, tex_lin->d3dtexture) ;
		DG_SetAlphaMode(0);
	}

	if( DG_CheckPatchUseVertexShader() )
	{
		{/* 環境マッピング用マトリクスの設定 */
			FMATRIX		mat ;
			_sceVu0MulMatrix( &mat, &patch->envmap_correct, &scrpad->view );/* 環境マップ補正 */
			DG_SetVertexShaderConstant(CV_EYE_INV, &mat, 4 );
		}
		{/* フォグ＆光源計算ありバージョン */
			DG_SelectVertexShader( &DG_PatchVertexShader[0] );
			//DG_SelectVertexShader( &DG_PatchVertexShader[1] );
			DG_SetPixelShader( NULL );
			DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
			DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
			DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
			DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
			DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
			DG_SetVertexShaderConstant(CV_WORLD0, &patch->world, 4);
			DG_SetVertexShaderConstant(CV_EYEPERS, &scrpad->eye_xpers, 4);
			DG_SetVertexShaderConstant(CV_LIGHTVEC, &patch->light[0], 4);
			DG_SetVertexShaderConstant(CV_LIGHTCOL, &patch->light[1], 4);
		}
		/* シェーダーセレクト */
		draw_ptch_func = DrawPatch ;
		//if ( scrpad->last_type != ( patch->flag & DG_PATCH_TYPEMASK ) )
		{
			FVECTOR		vec1, vec2 ;

			scrpad->last_type = patch->flag & DG_PATCH_TYPEMASK ;
			switch ( scrpad->last_type ){
			  case DG_PATCH_NORMAL:
				DG_SelectVertexShader( &DG_PatchVertexShader[0] );
				if( !use_tex_lin )
				{
					vec1 = patch->tex->tex_trans.vec1 ;
					vec2 = patch->tex->tex_trans.vec2 ;
					vec1.vx *= 32767.0f ;	// wtext.cでの正規化を無効に
					vec1.vy *= 32767.0f ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}
				DG_SetVertexShaderConstant( CV_TEX0_SCALE,  &vec1, 1 );
				DG_SetVertexShaderConstant( CV_TEX0_OFFSET, &vec2, 1 );
				break ;
			  case DG_PATCH_REFLECTPLANE:
				DG_SelectVertexShader( &DG_PatchVertexShader[3] );
				/* 現状ではパッチ曲面で使用している環境マップテクスチャがきちんと設定されていないので */
				if( !use_tex_lin )
				{
					vec1.vx = -0.4999f * patch->tex->u_scale ;
					vec1.vy = -0.4999f * patch->tex->v_scale ;
					vec1.vz = 1.0f ;
					vec1.vw = 1.0f ;
					vec2.vx = -0.5f * patch->tex->u_scale + patch->tex->u_offset ;
					vec2.vy = -0.5f * patch->tex->v_scale + patch->tex->v_offset ;
					vec2.vz = 0.0f ;
					vec2.vw = 0.0f ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}
				DG_SetVertexShaderConstant( CV_TEX0_SCALE, &vec1, 1 );
				DG_SetVertexShaderConstant( CV_TEX0_OFFSET, &vec2, 1 );
				break ;
			  case DG_PATCH_REFLECTPLANE2:
				DG_SelectVertexShader( &DG_PatchVertexShader[1] );
				/* 現状ではパッチ曲面で使用している環境マップテクスチャがきちんと設定されていないので */
				if( !use_tex_lin )
				{
					vec1.vx = -0.4999f * patch->tex->u_scale ;
					vec1.vy = -0.4999f * patch->tex->v_scale ;
					vec1.vz = 1.0f ;
					vec1.vw = 1.0f ;
					vec2.vx = -0.5f * patch->tex->u_scale + patch->tex->u_offset ;
					vec2.vy = -0.5f * patch->tex->v_scale + patch->tex->v_offset ;
					vec2.vz = 0.0f ;
					vec2.vw = 0.0f ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}

				DG_SetVertexShaderConstant( CV_TEX0_SCALE, &vec1, 1 );
				DG_SetVertexShaderConstant( CV_TEX0_OFFSET, &vec2, 1 );
				break ;
			  case DG_PATCH_BUMPMAP:
				ASSERT(0) ;
				//DG_SelectVertexShader( &DG_PatchVertexShader[1] );
				break ;
			}
		}
	}
	else
	{
		FMATRIX	mtx ;

		/* VertexShader非対応版 */
#if FALSE
		mtx = DG_UnitMatrix ;
		mtx.m[0][0] = patch->tex->tex_trans.vec1.x ;
		mtx.m[2][0] = patch->tex->tex_trans.vec2.x ;
		mtx.m[1][1] = patch->tex->tex_trans.vec1.y ;
		mtx.m[2][1] = patch->tex->tex_trans.vec2.y ;
		DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;
#endif

		/* 環境マッピング用マトリクスの設定 */
		_sceVu0MulMatrix( &mtx, &patch->envmap_correct, &scrpad->view );
		_sceVu0MulMatrix( &scrpad->emv_eye_inv, &mtx, &patch->world );
																		/* 環境マップ補正 */

		/* フォグ＆光源計算ありバージョン */
		
		DG_SetVertexShader( D3DFVF_PATCHVERTEX );

		DG_SetPixelShader( NULL );

		//DG_SetTransform(D3DTS_PROJECTION,     &scrpad->xpers);
		//DG_SetTransform(D3DTS_VIEW,           &scrpad->view);
		DG_SetTransform(D3DTS_WORLDMATRIX(0), &patch->world) ;

		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
		DG_SetRenderState(D3DRS_LIGHTING , TRUE) ;
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;
		DG_SetLightMatrix2Direct3D(&patch->light[0], &patch->light[1]) ;

		/* シェーダーセレクト */
		mtx = DG_UnitMatrix ;
		//if ( scrpad->last_type != ( patch->flag & DG_PATCH_TYPEMASK ) )
		{
			FVECTOR		vec1, vec2 ;
			scrpad->last_type = patch->flag & DG_PATCH_TYPEMASK ;
			switch ( scrpad->last_type ){
			  case DG_PATCH_NORMAL:
				draw_ptch_func = DrawPatchNVS_Normal ;

				if( !use_tex_lin )
				{
					vec1 = patch->tex->tex_trans.vec1 ;
					vec2 = patch->tex->tex_trans.vec2 ;
					vec1.vx *= 32767.0f ;	// wtext.cでの正規化を無効に
					vec1.vy *= 32767.0f ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}
				mtx.m[0][0] = vec1.vx ;
				mtx.m[2][0] = vec2.vx ;
				mtx.m[1][1] = vec1.vy ;
				mtx.m[2][1] = vec2.vy ;
				DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;
				break ;
			  case DG_PATCH_REFLECTPLANE:
				draw_ptch_func = DrawPatchNVS_EMap2 ;
#if 1
				/* 現状ではパッチ曲面で使用している環境マップテクスチャがきちんと設定されていないので */
				if( !use_tex_lin )
				{
					vec1.vx = -0.4999f * patch->tex->u_scale ;
					vec1.vy = -0.4999f * patch->tex->v_scale ;
					vec2.vx = -0.5f * patch->tex->u_scale + patch->tex->u_offset ;
					vec2.vy = -0.5f * patch->tex->v_scale + patch->tex->v_offset ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}
				mtx.m[0][0] = vec1.x ;
				mtx.m[2][0] = vec2.x ;
				mtx.m[1][1] = vec1.y ;
				mtx.m[2][1] = vec2.y ;
				DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;
#endif
				break ;
			  case DG_PATCH_REFLECTPLANE2:
				draw_ptch_func = DrawPatchNVS_EMap ;
#if 1
				/* 現状ではパッチ曲面で使用している環境マップテクスチャがきちんと設定されていないので */
				if( !use_tex_lin )
				{
					vec1.vx = -0.4999f * patch->tex->u_scale ;
					vec1.vy = -0.4999f * patch->tex->v_scale ;
					vec2.vx = -0.5f * patch->tex->u_scale + patch->tex->u_offset ;
					vec2.vy = -0.5f * patch->tex->v_scale + patch->tex->v_offset ;
				}
				else
				{
					vec1 = ((DG_TEX_LIN *)patch->tex)->tex_param ;
					vec1.vx *= 16384.0f ;
					vec1.vy *= 16384.0f ;
					vec2 = DG_ZeroVector ;
				}

				mtx.m[0][0] = vec1.x ;
				mtx.m[2][0] = vec2.x ;
				mtx.m[1][1] = vec1.y ;
				mtx.m[2][1] = vec2.y ;
				DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;
#endif
				break ;
			  case DG_PATCH_BUMPMAP:
				break ;
			}
		}
	}

	/* 強制黒フォグ処理 */
	if ( patch->flag & DG_PATCH_FOGBLACK ){
		DG_SetRenderState( D3DRS_FOGCOLOR, 0 );
	}

	if ( !( patch->flag & DG_PATCH_NOCHECK ) ){
		/* パッチ曲面のバウンディングチェック及びＬＯＤレベル計算 */
		parts = patch->parts ;
		for ( i = patch->n_patch ; i > 0 ; parts++, i-- ){
			float	f ;
			int		z, l ;
			parts->flag &= ~0x000f ;
			if ( BoundCheck( &scrpad->screen, &parts->max, &scrpad->scale, 1 ) == 2 ){
				parts->flag |= 2 ;
			} else {
				parts->flag |= 0 ;
			}
			/* カメラからの距離を求める（拡大率も考慮する） */
			//f = scrpad->scale.vw * scrpad->scale.vz + (float)( 1 << 12 ) ;
			f = scrpad->scale.vw * scrpad->scale.vz + patch->lod_z_bias ;
			f = DG_MAX( f, 0.0f );
			f = DG_MIN( f, 999999.0f );
			z = DG_FTOI( f ) ;
			/* log(z)/log(2)相当を計算 */
			//asm volatile ( "plzcw %0,%1" : "=r"(l) : "r"(z) ) ;
			{
				int n = 0;
				int tmp = z ;
				if ( !( tmp & 0x80000000 ) ) tmp = ~tmp ;
				for ( n = 0 ; ( tmp & 0x40000000 ) && ( n < 32 ) ; tmp <<= 1, n++ ) ;
				l = n + 0;  /* この数値を大きくすると、デフォルトの分割が細かくなる。 */
			}
			l = 31 - l ;
			/* ＬＯＤレベルの決定処理 */
			l -= patch->lod_level_bias ;
			if ( l > 4 ) l = 4 ;
			if ( l < 0 ) l = 0 ;
			parts->lod_level = l ;
		}

		/* パッチ曲面のエッジ補正チェック */
		parts = patch->parts ;
		for ( i = patch->n_patch ; i > 0 ; parts++, i-- ){
			int		index ;
			parts->edge_flag = 0 ;
			index = parts->parts_index[0] ;
			if ( index != -1 ){
				if ( parts->lod_level < patch->parts[ index ].lod_level ) parts->edge_flag |= 0x0001 ;
			}
			index = parts->parts_index[1] ;
			if ( index != -1 ){
				if ( parts->lod_level < patch->parts[ index ].lod_level ) parts->edge_flag |= 0x0002 ;
			}
			index = parts->parts_index[2] ;
			if ( index != -1 ){
				if ( parts->lod_level < patch->parts[ index ].lod_level ) parts->edge_flag |= 0x0004 ;
			}
			index = parts->parts_index[3] ;
			if ( index != -1 ){
				if ( parts->lod_level < patch->parts[ index ].lod_level ) parts->edge_flag |= 0x0008 ;
			}
		}
	}

	/* パッチオブジェクトの転送 */
	parts = patch->parts ;
	verts = patch->verts[ patch->buffer_clock ] ;
	for ( i = patch->n_patch ; i > 0 ; parts++, i-- ){
		if ( parts->flag != 0 ) continue ;
#ifdef LIBDG_PERFORMANCE
		{
			int	div ;
			div = 1 << ( 4 - parts->lod_level ) ;
			scrpad->n_verts += ( div * 2 + 2 ) * div ;
			scrpad->n_polys += div * 2 * div ;
		}
#endif
		draw_ptch_func( parts->lod_level, parts->edge_flag,
				  &verts[ parts->v_index[ 0 ] ],
				  &verts[ parts->v_index[ 1 ] ],
				  &verts[ parts->v_index[ 2 ] ],
				  &verts[ parts->v_index[ 3 ] ] );
	}

	/* 強制黒フォグ処理 */
	if ( patch->flag & DG_PATCH_FOGBLACK ){
		DG_SetDxFogColor();
	}

	DG_SetPixelShader( NULL );


}

/*----------------------------------------------------------------*/
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	extern void DG_StartBoundingCheckSupport( void );
	DG_PATCH		*patch, **que ;
	int		i, j, old_tex_code, invisible_flag ;
#ifdef LIBDG_PERFORMANCE
	int		size, n, time ;
#endif
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		last_semi_trans = 0 ;

	scrpad = (ScrpadWork *)SCRPAD_ADDR ;
#ifdef LIBDG_PERFORMANCE
	scrpad->n_verts = 0 ;
	scrpad->n_polys = 0 ;
	n = 0 ;
	time = GET_HSYNC_TIMER();
#endif

	invisible_flag = DG_PATCH_INVISIBLE0 << cp->chanl_num ;
	/*
		キュー数のチェック
	*/
	if ( obj_buff->n_queue == 0 ) return ;
	MARK( __FILE__ );

	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->eye_xpers = cp->eye_xpers ;
	scrpad->view = cp->eye_inv ;
	//if( !DG_CheckPatchUseVertexShader() ){ scrpad->cp_eye = cp->eye ; }
	//scrpad->view = cp->eye ;
#if 1
	_sceVu0UnitMatrix( &scrpad->eye );
	scrpad->eye.m[0][0] = -cp->eye.m[0][0] ;
	scrpad->eye.m[0][1] = -cp->eye.m[0][2] ;
	scrpad->eye.m[0][2] = cp->eye.m[0][1] ;
	scrpad->eye.m[1][0] = -cp->eye.m[1][0] ;
	scrpad->eye.m[1][1] = -cp->eye.m[1][2] ;
	scrpad->eye.m[1][2] = cp->eye.m[1][1] ;
	scrpad->eye.m[2][0] = -cp->eye.m[2][0] ;
	scrpad->eye.m[2][1] = -cp->eye.m[2][2] ;
	scrpad->eye.m[2][2] = cp->eye.m[2][1] ;
	_sceVu0MulMatrix( &scrpad->view, &scrpad->eye, &scrpad->view );
#endif
	scrpad->scale.vx = (float)(cp->width >> 1) ;
	scrpad->scale.vy = (float)(cp->height >> 1) ;
	scrpad->scale.vz = 1.0f / cp->pers.m[0][0] ;/* ＝投影距離（screen） */
	scrpad->cam_screen = cp->screen ;
	scrpad->cam_screen_inv = 1.0f / cp->screen ;
	scrpad->last_type = 0 ;
	//InitMatrixPacket( &scrpad->matrix_packet );
	//InitPatchPacket( &scrpad->patch_packet[0] );
	//InitPatchPacket( &scrpad->patch_packet[1] );

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 半透明用に事前にマスクを生成 */
	DG_MakeStencilMask();

	/* シェーダーの初期化 */
	if( !DG_CheckPatchUseVertexShader() )
	{
		DG_SetTransform(D3DTS_VIEW, &cp->eye_inv) ;
		DG_SetTransform(D3DTS_PROJECTION, &cp->xpers) ;
	}
	else
	{
		/* Fog 初期化 */
		float fog_start = 0.0f;
		float fog_end = 1.0f;
		DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
		DG_SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog_end);
	}
#ifdef _WINDOWS
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE) ;
#endif

	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
					DG_CheckVertexBufferSoftProcessing()) ;

	/* 初期化パケットの設定 */

	/* テクスチャ単位での処理（通常不透明オブジェクト処理と同じ） */
	for ( tex_list_p = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list_p++, j-- ){
		tex_list = *tex_list_p;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
#ifdef LIBDG_CHECK_MULTILOADTEX
		//if ( GV_Time & 2 ) if ( tex_list->flag ) continue ;
#endif
		old_tex_code = -1 ;

		/* 全オブジェクトの検索 */
		que = (DG_PATCH**)obj_buff->queue ;
		for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
			patch = *que ;

			if ( patch->flag & invisible_flag ) continue ;
			if ( patch->tri_id != tex_list->code ) continue ;
			if ( !( patch->group_id & DG_CurrentGroupID ) ) continue ;

			/* テクスチャのセットアップ */
			if ( old_tex_code == -1 ){
				old_tex_code = tex_list->code ;
#ifdef LIBDG_PERFORMANCE
				DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
				tex_list->flag = 1 ;
				/* テクスチャ転送 */
			}
			if ( patch->flag & DG_PATCH_NO_WRAP ){
				/* リピート無し */
				DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			} else {
				/* リピートあり（通常） */
				DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			}
			/* 強制半透明描画チェック */
			if ( patch->flag & DG_PATCH_SEMITRANS ){
				if ( last_semi_trans == 0 ){
					/* 半透明ＯＮ */
					DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
				}
				last_semi_trans = 1 ;
			} else {
				if ( last_semi_trans != 0 ){
					/* 半透明ＯＦＦ */
					DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
				}
				last_semi_trans = 0 ;
			}
			ChainObj( patch );
#ifdef LIBDG_PERFORMANCE
			n++ ;
#endif
		}
	}

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		/* 半透明ＯＦＦ */
		DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	}
	/* Ｚ圧縮を活用するためにステンシルをクリア */
	DG_Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0.0f, 0);

	/* ミップマップ無効化 */

	/* 終了パケットの設定 */

	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

	/* ＤＭＡバッファクローズ */
#ifdef LIBDG_PERFORMANCE
	if ( GV_PadData[1].press & PAD_L2 ){
		printf("patch.c: obj:%d(%d) verts:%d polys:%d  hcount:%d\n",
			   n, obj_buff->n_queue, scrpad->n_verts, scrpad->n_polys,
			   GET_HSYNC_TIMER() - time );
	}
#endif

}


/*----------------------------------------------------------------*/
	/*
		パッチ曲面オブジェクトの作成
	*/
DG_PATCH*	DG_MakePatch( int flag, int n_patch, int n_verts )
{
	DG_PATCH	*patch ;
	int			size ;

	flag |= DG_PATCH_INVISIBLE2|DG_PATCH_INVISIBLE3 ;
	size = sizeof(DG_PATCH) + sizeof(DG_PATCH_PARTS) * n_patch + sizeof(DG_PATCH_VERT) * n_verts * 2 ;
	if ( ( patch = GV_Malloc( size ) ) == NULL ){
		return NULL ;
	}
	GV_ZeroMemory( patch, size );

	patch->flag = flag ;
	patch->n_verts = n_verts ;
	patch->n_patch = n_patch ;
	/*patch->chanl = chanl ;*/
	patch->tri_id = 0 ;
	patch->lod_z_bias = (float)( 1 << 12 );
	patch->lod_level_bias = 12 ;
	_sceVu0UnitMatrix( &patch->world );
	patch->light = &DG_LightMatrix ;
	patch->envmap_correct = DG_UnitMatrix ;

	patch->verts[0] = (void*)( (int)patch + sizeof(DG_PATCH) );
	patch->verts[1] = (void*)( (int)patch + sizeof(DG_PATCH) + sizeof(DG_PATCH_VERT) * n_verts * 1 );
	patch->parts = (void*)( (int)patch + sizeof(DG_PATCH) + sizeof(DG_PATCH_VERT) * n_verts * 2 );

	/* パケットの初期化 */

	return ( patch ) ;
}

	/*
		メモリ開放
	*/
void	DG_FreePatch( DG_PATCH *patch )
{
	GV_DelayedFree( patch );
}

/*----------------------------------------------------------------*/
	/*
		メッシュの頂点数で確保＆自動初期化
	*/
DG_PATCH* DG_MakePatchMesh( int flag, int n_verts_s, int n_verts_t )
{
	DG_PATCH	*patch ;
	DG_PATCH_PARTS	*parts ;
	int			n_patch, n_verts ;
	int			n_patch_s, n_patch_t ;
	int			i, j ;

	if ( n_verts_s < 2 ) n_verts_s = 2 ;
	if ( n_verts_t < 2 ) n_verts_t = 2 ;
	n_verts = n_verts_s * n_verts_t ;
	n_patch = ( n_verts_s - 1 ) * ( n_verts_t - 1 ) ;
	patch = DG_MakePatch( flag, n_patch, n_verts );
	if ( patch == NULL ) return ( NULL );

	n_patch_s = n_verts_s - 1 ;
	n_patch_t = n_verts_t - 1 ;
	for ( i = 0 ; i < n_patch_t ; i++ ){
		for ( j = 0 ; j < n_patch_s ; j++ ){
			parts = &patch->parts[ n_patch_s * i + j ] ;
			/* 頂点インデックス設定 */
			parts->v_index[ 0 ] = n_verts_s * ( i + 0 ) + ( j + 0 ) ;
			parts->v_index[ 1 ] = n_verts_s * ( i + 0 ) + ( j + 1 ) ;
			parts->v_index[ 2 ] = n_verts_s * ( i + 1 ) + ( j + 0 ) ;
			parts->v_index[ 3 ] = n_verts_s * ( i + 1 ) + ( j + 1 ) ;
			/* 隣接パッチインデックス設定 */
			if ( j > 0 ){
				parts->parts_index[0] = n_patch_s * ( i + 0 ) + ( j - 1 ) ;
			} else {
				parts->parts_index[0] = -1 ;
			}
			if ( j < ( n_patch_s - 1 ) ){
				parts->parts_index[1] = n_patch_s * ( i + 0 ) + ( j + 1 ) ;
			} else {
				parts->parts_index[1] = -1 ;
			}
			if ( i > 0 ){
				parts->parts_index[2] = n_patch_s * ( i - 1 ) + ( j + 0 ) ;
			} else {
				parts->parts_index[2] = -1 ;
			}
			if ( i < ( n_patch_t - 1 ) ){
				parts->parts_index[3] = n_patch_s * ( i + 1 ) + ( j + 0 ) ;
			} else {
				parts->parts_index[3] = -1 ;
			}
			/* その他設定 */
			parts->flag = 0 ;
		}
	}

	return ( patch );

}

	/*
		バウンディング及び中心座標自動設定（とても遅い）
	*/
void DG_SetupPatchMeshBounding(DG_PATCH *patch)
{
	DG_PATCH_PARTS	*parts;
	DG_PATCH_VERT	*verts;
	FVECTOR			max, min, center, g_max, g_min;
	static const VECTOR def_max = { FLT_MAX, FLT_MAX, FLT_MAX, 1.0 };
	static const VECTOR def_min = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0 };
	//static const VECTOR const_data = { 0.25, 0, 0, 0 };
	int			i, j;

	g_max = def_min;
	g_min = def_max;

	parts = patch->parts;
	verts = patch->verts[patch->buffer_clock];
	for (i = patch->n_patch; i > 0; parts++, i--) {
		max = def_min;
		min = def_max;
		center = DG_ZeroVector;

		for (j = 0; j < 4; j++) {
		   	DG_PATCH_VERT *v;
			v = &verts[parts->v_index[j]];
			//printf("index[%d] = %d : %f, %f, %f, %f\n", j, parts->v_index[j],
			//v->pos.vx, v->pos.vy, v->pos.vz);
			max.vx = DG_MAX(max.vx, v->pos.vx);
			max.vy = DG_MAX(max.vy, v->pos.vy);
			max.vz = DG_MAX(max.vz, v->pos.vz);
			min.vx = DG_MIN(min.vx, v->pos.vx);
			min.vy = DG_MIN(min.vy, v->pos.vy);
			min.vz = DG_MIN(min.vz, v->pos.vz);
			center.vx += v->pos.vx ;
			center.vy += v->pos.vy ;
			center.vz += v->pos.vz ;
		}

		// 中心位置を求める
		center.vx *= 0.25f;
		center.vy *= 0.25f;
		center.vz *= 0.25f;

		//printf("max   : %f, %f, %f\n", max.vx, max.vy, max.vz);
		//printf("min   : %f, %f, %f\n", min.vx, min.vy, min.vz);
		//printf("center: %f, %f, %f\n", center.vx, center.vy, center.vz);		
#if 0
		max = (max - center) + max;
		min = (min - center) + min;
#else
		max.vx = ( max.vx - center.vx ) + max.vx ;
		max.vy = ( max.vy - center.vy ) + max.vy ;
		max.vz = ( max.vz - center.vz ) + max.vz ;
		min.vx = ( min.vx - center.vx ) + min.vx ;
		min.vy = ( min.vy - center.vy ) + min.vy ;
		min.vz = ( min.vz - center.vz ) + min.vz ;
#endif

		g_max.vx = DG_MAX(g_max.vx, max.vx);
		g_max.vy = DG_MAX(g_max.vy, max.vy);
		g_max.vz = DG_MAX(g_max.vz, max.vz);			
		g_min.vx = DG_MIN(g_min.vx, min.vx);
		g_min.vy = DG_MIN(g_min.vy, min.vy);
		g_min.vz = DG_MIN(g_min.vz, min.vz);			

		parts->max = max;
		parts->min = min;
		//printf("max: %f, %f, %f\n", max.vx, max.vy, max.vz);
		//printf("min: %f, %f, %f\n", min.vx, min.vy, min.vz);
	}

	// 全体バウンディング
	patch->max = g_max;
	patch->min = g_min;
}

	/*
		ＬＯＤパラメータ設定
	*/
void DG_ConfigPatchLOD( DG_PATCH *patch, int level )
{
	patch->lod_z_bias = (float)( 1 << level );
	patch->lod_level_bias = level ;
}
