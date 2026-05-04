//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	patch.c
	チャンネル処理ユニット／パッチ曲面オブジェクト処理ルーチン

	2000/07/11 K.Takabe
	$Id: patch.c,v 1.1.1.3 2002/11/19 11:42:17 Yoshizawa1 Exp $

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
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"
#include	"utl_dma.h"

#include "bp_matrix.h"

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



#define GS_REGS_0()
#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_2(r0, r1) \
((r0) << 0x00 | (r1) << 0x04)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_4(r0, r1, r2, r3) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)
#define GS_REGS_6(r0, r1, r2, r3, r4, r5) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10 | (r5) << 0x14)

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

/* ＶＵ１ワークパラメータ */
typedef struct _vu1_work {
	FVECTOR	clamp_min ;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max ;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4 ;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip ;	/* 各種定数 */
	FVECTOR	scale ;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset ;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4 ;					/* 各種定数２ */
	FVECTOR	pad ;
	FMATRIX	pers ;			/* 透視変換マトリクス */
} Vu1Work ;

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

typedef ALIGN16_DECL(struct) _patch_packet{
	DG_DMATAG	verts_trans[4] ;
	DG_DMATAG	flag_trans ;
	struct {
		int		n_verts ;		/* ストリップ後の頂点数（内部で計算） */
		int		edge_flag ;		/* エッジフラグ */
		int		n_skip ;		/* カウンタスキップ値（内部で計算） */
		int		lod_level ;		/* ＬＯＤレベル（0~4） */
	} data ;
	DG_VIFCODE	prog_exec ;
} PATCH_PACKET ;

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


/* デフォルトのＶＵ１ワークパラメータ */
static Vu1Work	default_vu1_work = {
#if 1 //BP_GCC
   { 0.0f, 0.0f, 0.0f, -1.0f }, //clamp_min
   {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f }, //clamp_max
   0, //fog_param1
   0, //fog_param2
   255, //fog_param3
   0, //fog_param4
   255.0f, //color_clip
   128.0f, //poly_alpha
   0.0f, //specular_mul
   0.8f, //specular_clip
   { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f }, //scale
   { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f }, //offset
   0, //param1
   0, //param2
   0.5, //param3
   0, //param4
#else
	.clamp_min = {.vx = 0.0f, .vy = 0.0f, .vz = 0.0f, .vw = -1.0f },
	.clamp_max = {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f },
	.fog_param1 = 0, .fog_param2 = 0, .fog_param3 = 255, .fog_param4 = 0,
	.color_clip = 255.0f, .poly_alpha = 128.0f, .specular_mul = 0.0f, .specular_clip = 0.8f, 
	.scale = { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
	.offset = { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f },
	.param1 = 0, .param2 = 0, .param3 = 0.5, .param4 = 0,
#endif
};

//#define PATCH_DEBUG
#ifdef PATCH_DEBUG
extern int	Vu1DrawCurveSurface_Debug[] ;
#define Vu1DrawCurveSurface Vu1DrawCurveSurface_Debug
static DG_TEX	PatchDebugTexture = {
	0.0f, 0.0f, 1.0f, 1.0f,
	0, 0, 0, {0},
	{
		{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
		{ 0, SCE_GS_NOP },
		{ 0, SCE_GS_NOP },
		{ SCE_GS_SET_TEST( 1, 2, 127, 0, 0, 0, 1, 2 ), SCE_GS_TEST_1 },
		{ SCE_GS_SET_ALPHA( 2, 2, 2, 0, 128 ), SCE_GS_ALPHA_1 },
		{ 0, SCE_GS_NOP },
		{0,0,0,0},
		{0,0,0,0}
	}
};

#endif

#if 0 //BP_PS2
extern int	Vu1DrawCurveSurface[] ;
extern int	Vu1DrawCurveSurface_Func[] ;
extern int	Vu1DrawReflectSurface[] ;
extern int	Vu1DrawReflectSurface_Func[] ;
//extern int	Vu1DrawBumpCurveSurface[] ;

/* パッチ曲面処理用ＶＵマイクロプログラム指定 */
void	*DG_CurveSurfaceVuCode[ 3 ] = {
	Vu1DrawCurveSurface,
	Vu1DrawReflectSurface,
	Vu1DrawCurveSurface
};
#endif

/*----------------------------------------------------------------*/
/* Ｖｕ０レジスタ使用スクラッチパッド間のみ使用可能バージョン */
#define Vu0CopyMatrix( _a, _b ) \
	asm volatile ("\
		lqc2		vf04,0x00(%1);\
		lqc2		vf05,0x10(%1);\
		lqc2		vf06,0x20(%1);\
		lqc2		vf07,0x30(%1);\
		sqc2		vf04,0x00(%0);\
		sqc2		vf05,0x10(%0);\
		sqc2		vf06,0x20(%0);\
		sqc2		vf07,0x30(%0);\
	"::"r"(_a),"r"(_b));
/*----------------------------------------------------------------*/
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
	/* 書き込み先はスクラッチパッドでなければならない */
	asm volatile ("
		lqc2		vf4,0x00(%2)
		lqc2		vf5,0x10(%2)
		lqc2		vf6,0x20(%2)
		lqc2		vf7,0x30(%2)
		li			$7,4
0:
		lqc2		vf8,0x0(%1)
		vmulax.xyz		ACC,vf4,vf8
		vmadday.xyz		ACC,vf5,vf8
		vmaddaz.xyz		ACC,vf6,vf8
		vmaddw.xyz		vf12,vf7,vf8
		sqc2		vf12,0x0(%0)
		addi		$7,-1
		addi		%1,0x10
		addi		%0,0x10
		bne			$0,$7,0b
	":"+r"(res_light),"+r"(world):"r"(light):"$7");
}
/*----------------------------------------------------------------*/
static int BoundCheck( FMATRIX *mat, FVECTOR *bound, FVECTOR *scale, int check_flag )
{
	int		and_flag, or_flag, prim_over_flag ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	/* バウンディングチェックマイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	lqc2		vf3,0(%2)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(bound),"r"(scale) );
	/* 計算結果の取得 */
	asm volatile ("
	cfc2.i		%0,$2 /*vi2*/
	cfc2		%1,$3 /*vi3*/
	cfc2		%2,$1 /*vi1*/
	sqc2		vf31,0(%3)
	":"=&r"(and_flag),"=&r"(or_flag),"=&r"(prim_over_flag):"r"(&scrpad->tmp_vec) );

	/* バウンディング中央値のＺ座標取得 */
	scrpad->scale.vw = scrpad->tmp_vec.vw ;

	/* バウンディング判定 */
	if ( flag == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
	return ( 0 );	/* 完全画面内 */

}
/*----------------------------------------------------------------*/
	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/
static int PatchObjsPacketInit( void *tag_addr, DG_CHANL *cp )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/*
		ＶＩＦ初期化＆ＶＵ用定数＆マイクロプログラムロード設定パケットの登録
	*/
#if 0 //BP_PS2
#if 1
	*(u_long128*)tag = *(u_long128*)Vu1DrawCurveSurface ;
#else
	*(u_long128*)tag = *(u_long128*)Vu1DrawReflectSurface ;
#endif
#endif
	tag++ ;

	/*
		基本パラメータ設定パケットの登録
	*/
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/*
		パラメータの書き込み
	*/
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}

/*----------------------------------------------------------------*/
static void InitMatrixPacket( MATRIX_PACKET *packet )
{
	packet->trans_mat.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(OBJ_MATRIX) );
	packet->trans_mat.vifcode[0] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	packet->trans_mat.vifcode[1] = SCE_VIF1_SET_UNPACK( 104, SIZEOF_QWORD(OBJ_MATRIX), VIF_DATA128, 0 ) ;
	packet->trans_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	packet->trans_tex.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->trans_tex.vifcode[1] = SCE_VIF1_SET_UNPACK( 120, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	
}
static void InitPatchPacket( PATCH_PACKET *packet )
{
	packet->verts_trans[0].qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_PATCH_VERT) );
	packet->verts_trans[0].vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->verts_trans[0].vifcode[1] = SCE_VIF1_SET_UNPACKR( 0, SIZEOF_QWORD(DG_PATCH_VERT), VIF_DATA128, 0 ) ;
	packet->verts_trans[1].qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_PATCH_VERT) );
	packet->verts_trans[1].vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->verts_trans[1].vifcode[1] = SCE_VIF1_SET_UNPACKR( 4, SIZEOF_QWORD(DG_PATCH_VERT), VIF_DATA128, 0 ) ;
	packet->verts_trans[2].qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_PATCH_VERT) );
	packet->verts_trans[2].vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->verts_trans[2].vifcode[1] = SCE_VIF1_SET_UNPACKR( 8, SIZEOF_QWORD(DG_PATCH_VERT), VIF_DATA128, 0 ) ;
	packet->verts_trans[3].qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_PATCH_VERT) );
	packet->verts_trans[3].vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->verts_trans[3].vifcode[1] = SCE_VIF1_SET_UNPACKR( 12, SIZEOF_QWORD(DG_PATCH_VERT), VIF_DATA128, 0 ) ;
	packet->flag_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	packet->flag_trans.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->flag_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 16, 1, VIF_DATA128, 0 ) ;
	packet->prog_exec.vifcode[0] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	packet->prog_exec.vifcode[1] = SCE_VIF1_SET_MSCAL( 0, 0 ) ;
	packet->prog_exec.vifcode[2] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->prog_exec.vifcode[3] = SCE_VIF1_SET_NOP( 0 ) ;
	
}
/*----------------------------------------------------------------*/
static void ChainObj( DG_PATCH *patch )
{
	ScrpadWork	*scrpad ;
	PATCH_PACKET	*packet ;
	DG_PATCH_PARTS	*parts ;
	DG_PATCH_VERT	*verts ;
	int			i ;

	scrpad = SCRPAD_ADDR ;

	/* ＶＵプログラムの読み込みチェック */
	if ( scrpad->last_type != ( patch->flag & DG_PATCH_TYPEMASK ) ){
		scrpad->last_type = patch->flag & DG_PATCH_TYPEMASK ;
		switch ( scrpad->last_type ){
		  case DG_PATCH_NORMAL:
			//DG_SendCacheFIFO( scrpad->local_work, Vu1DrawCurveSurface, 1 );
			DG_SendCacheFIFO( scrpad->local_work, DG_CurveSurfaceVuCode[0], 1 );
			break ;
		  case DG_PATCH_REFLECTPLANE:
			//DG_SendCacheFIFO( scrpad->local_work, Vu1DrawReflectSurface, 1 );
			DG_SendCacheFIFO( scrpad->local_work, DG_CurveSurfaceVuCode[1], 1 );
			break ;
		  case DG_PATCH_BUMPMAP:
			//DG_SendCacheFIFO( scrpad->local_work, Vu1DrawBumpCurveSurface, 1 );
			DG_SendCacheFIFO( scrpad->local_work, DG_CurveSurfaceVuCode[2], 1 );
			break ;
		}
	}

	Vu0CopyMatrix( &scrpad->world, &patch->world );
	if ( patch->root != NULL ){
		Vu0CopyMatrix( &scrpad->world, patch->root );
		Vu0CopyMatrix( &patch->world, &scrpad->world );
	}
	SetLightMatrix( &scrpad->light[0], &patch->light[0], &scrpad->world );
	Vu0CopyMatrix( &scrpad->light[1], &patch->light[1] );
#ifdef PATCH_DEBUG
	GV_ZeroMemory( &scrpad->light[1], sizeof(FMATRIX) );
	scrpad->light[1].m[3][0] = 200 ;
	scrpad->light[1].m[3][1] = 200 ;
	scrpad->light[1].m[3][2] = 200 ;
	scrpad->light[1].m[3][3] = 128 ;
#endif

	_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &scrpad->world );

	/* 全体バウンディングのチェック */
	if ( BoundCheck( &scrpad->screen, &patch->max, &scrpad->scale, 1 ) == 2 ){
		return ;
	}

	Vu0CopyMatrix( &scrpad->matrix_packet.obj_matrix.screen, &scrpad->screen );
	Vu0CopyMatrix( &scrpad->matrix_packet.obj_matrix.local_light, &scrpad->light[0] );
	Vu0CopyMatrix( &scrpad->matrix_packet.obj_matrix.local_color, &scrpad->light[1] );
	_sceVu0MulMatrix( &scrpad->matrix_packet.obj_matrix.option, &scrpad->view, &scrpad->world );
	/* 環境マッピング用マトリクス補正 */
	_sceVu0MulMatrix( &scrpad->matrix_packet.obj_matrix.option,
					 &patch->envmap_correct, &scrpad->matrix_packet.obj_matrix.option );

	scrpad->matrix_packet.trans_tex.addr = &patch->tex->tex_trans ;
#ifdef PATCH_DEBUG
	scrpad->matrix_packet.trans_tex.addr = &PatchDebugTexture.tex_trans ;
#endif

	DG_SendCacheFIFO( scrpad->local_work, &scrpad->matrix_packet, SIZEOF_QWORD(MATRIX_PACKET) );

	/* 強制黒フォグ処理 */
	if ( patch->flag & DG_PATCH_FOGBLACK ){
		int		size ;
		size = DG_WriteFogChangePacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}
	/* ミップマップ処理 */
	if ( patch->mipmap == NULL ){
		int		size ;
		size = DG_WriteMipmapSettingPacks( scrpad->dma_buffer, NULL );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	} else {
		int		size ;
		DG_SetMipmapLodParam( patch->mipmap, scrpad->cam_screen );
		size = DG_WriteMipmapSettingPacks( scrpad->dma_buffer, patch->mipmap );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

	//if ( !( patch->flag & DG_PATCH_NOCHECK ) || GV_PadData[0].status & PAD_X ){
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
			asm volatile ( "plzcw %0,%1" : "=r"(l) : "r"(z) ) ;
			l = 31 - l ;
			/* ＬＯＤレベルの決定処理 */
			//l -= 12 ;
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
		//DG_PATCH_VERT	**v_list ;
		packet = &scrpad->patch_packet[scrpad->buffer_switch] ;

		if ( parts->flag != 0 ) continue ;
		packet->verts_trans[ 0 ].addr = &verts[ parts->v_index[ 0 ] ] ;
		packet->verts_trans[ 1 ].addr = &verts[ parts->v_index[ 1 ] ] ;
		packet->verts_trans[ 2 ].addr = &verts[ parts->v_index[ 2 ] ] ;
		packet->verts_trans[ 3 ].addr = &verts[ parts->v_index[ 3 ] ] ;
		//packet->data.n_verts = 4 ;
		//packet->data.n_skip = 16 ;
		packet->data.edge_flag = parts->edge_flag ;
		{
			int	div ;
			div = 1 << ( 4 - parts->lod_level ) ;
			packet->data.n_verts = div * 2 + 2 ;
			packet->data.n_skip = 16 / div ;
#ifdef LIBDG_PERFORMANCE
			scrpad->n_verts += packet->data.n_verts * div ;
			scrpad->n_polys += div * 2 * div ;
#endif
		}

		/* 実行プログラム設定 */
		packet->prog_exec.vifcode[1] = SCE_VIF1_SET_MSCAL( 0, 0 );

		DG_SendCacheFIFO( scrpad->local_work, packet, SIZEOF_QWORD(PATCH_PACKET) );
		scrpad->buffer_switch = 1 - scrpad->buffer_switch ;	
	}

	/* 強制黒フォグ処理 */
	if ( patch->flag & DG_PATCH_FOGBLACK ){
		int		size ;
		size = DG_WriteFogChangePacks( scrpad->dma_buffer, *(int*)&DG_FogColor );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}
}

/*----------------------------------------------------------------*/
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	extern void DG_StartBoundingCheckSupport( void );
	DG_PATCH		*patch, **que ;
	int		i, j, size, n, time, old_tex_code, invisible_flag ;
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		last_semi_trans = 0 ;

	scrpad = SCRPAD_ADDR ;
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

#if 1 //BP_RENDER
   BP_RENDER_TODO_BREAK;
   return;
#endif

	MARK( "patch.c" );

	DG_StartBoundingCheckSupport();
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->view = cp->eye_inv ;
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
	scrpad->scale.vx = cp->width / 2 ;
	scrpad->scale.vy = cp->height / 2 ;
	scrpad->scale.vz = 1.0f / cp->pers.m[0][0] ;/* ＝投影距離（screen） */
	scrpad->cam_screen = cp->screen ;
	scrpad->cam_screen_inv = 1.0f / cp->screen ;
	scrpad->last_type = 0 ;
	InitMatrixPacket( &scrpad->matrix_packet );
	InitPatchPacket( &scrpad->patch_packet[0] );
	InitPatchPacket( &scrpad->patch_packet[1] );

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	size = PatchObjsPacketInit( scrpad->dma_buffer, cp );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

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
			if ( !( patch->group_id & c_gid ) ) continue ;

			/* テクスチャのセットアップ */
			if ( old_tex_code == -1 ){
				old_tex_code = tex_list->code ;
#ifdef LIBDG_PERFORMANCE
				DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
				tex_list->flag = 1 ;
				//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
				size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
			/* 強制半透明描画チェック */
			if ( patch->flag & DG_PATCH_SEMITRANS ){
				if ( last_semi_trans == 0 ){
					size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 1 );
					DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
				}
				last_semi_trans = 1 ;
			} else {
				if ( last_semi_trans != 0 ){
					size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
					DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
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
		size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

	{/* ミップマップ無効化 */
		int		size ;
		size = DG_WriteMipmapSettingPacks( scrpad->dma_buffer, NULL );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

	/* 終了パケットの設定 */
	size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
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
void DG_SetupPatchMeshBounding( DG_PATCH *patch )
{
	DG_PATCH_PARTS	*parts ;
	DG_PATCH_VERT	*verts ;
	//FVECTOR			max, min, center, g_max, g_min ;
	static FVECTOR	def_max = {  1.0e+40,  1.0e+40,  1.0e+40, 1.0 };
	static FVECTOR	def_min = { -1.0e+40, -1.0e+40, -1.0e+40, 1.0 };
	static FVECTOR	const_data = { 0.25, 0, 0, 0 };
	int			i, j ;

	asm ("
		lqc2		vf31,0(%0)
		lqc2		vf05,0(%1)		# g_max = def_min
		lqc2		vf06,0(%2)		# g_min = def_max
	"::"r"(&const_data),"r"(&def_min),"r"(&def_max));

	parts = patch->parts ;
	verts = patch->verts[patch->buffer_clock] ;
	for ( i = patch->n_patch ; i > 0 ; parts++, i-- ){
		asm volatile("
			lqc2		vf01,0(%0)		# max = def_min
			lqc2		vf02,0(%1)		# min = def_max
			vmove.xyzw	vf03,vf00		# center = 0
		"::"r"(&def_min),"r"(&def_max));
		for ( j = 0 ; j < 4 ; j++ ){
		   	DG_PATCH_VERT	*v ;
			v = &verts[ parts->v_index[j] ] ;
			asm ("
				lqc2		vf04,0(%0)
				vmax.xyz	vf01,vf01,vf04	# max = fmax( max, v )
				vmini.xyz	vf02,vf02,vf04	# min = fmin( min, v )
				vadd.xyz	vf03,vf03,vf04	# center += v
			"::"r"(&v->pos));
		}
		asm volatile("
			vmulx.xyz	vf03,vf03,vf31	# center /= 4
			vsuba.xyz	ACC ,vf01,vf03	# 
			vmaddw.xyz	vf01,vf01,vf00	# max = ( max - center ) + max
			vsuba.xyz	ACC ,vf02,vf03	# 
			vmaddw.xyz	vf02,vf02,vf00	# min = ( min - center ) + min
			vmax.xyz	vf05,vf05,vf01	# g_max = fmax( g_max, max )
			vmini.xyz	vf06,vf06,vf02	# g_min = fmin( g_min, min )
			sqc2		vf01,0(%0)
			sqc2		vf02,0(%1)
		"::"r"(&parts->max),"r"(&parts->min) );
	}
	asm ("
		sqc2		vf05,0(%0)		# g_max = def_min
		sqc2		vf06,0(%1)		# g_min = def_max
	"::"r"(&patch->max),"r"(&patch->min));

	return ;
}

	/*
		ＬＯＤパラメータ設定
	*/
void DG_ConfigPatchLOD( DG_PATCH *patch, int level )
{
	patch->lod_z_bias = (float)( 1 << level );
	patch->lod_level_bias = level ;
}
