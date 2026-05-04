//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	comdl.c
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	1999/11/17 K.Takabe
	$Id: comdl.c,v 1.1.1.3 2002/11/19 11:42:03 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	この共有モデルオブジェクトとは１ストリップで構成されている
	単一のモデル対し複数のマトリクスを用意することによって
	オブジェクトを複数表示することを可能とした特殊オブジェクトです。
	シェーディング処理は行ないませんが、オブジェクト毎にＲＧＢＡを
	指定できるようになっています。
	基本的にソートなどは行なわないため、不透明モデルと同じ扱いに
	なりますが、描画結果がソートに影響しない場合に限り半透明も
	使用可能です。


	void		DG_ComdlChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた共有モデルオブジェクトのパケット作成などを行なう


	DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
	DG_MDLPACK	*mdl_pack ;		１ストリップ分のモデルデータ
	int			flag ;			処理フラグ
	int			n_comdl ;		オブジェクト数
	int			chanl ;			処理チャンネル

	共有モデルオブジェクトを作成する（作成後はDG_QueueComdlObjs()で登録すること）


	void	DG_FreeComdl( DG_COMDL *comdl )
	DG_COMDL	*comdl ;		開放オブジェクト

	メモリを破棄する（これを呼ぶ前にDG_DequeueComdlObjs()でシステムから削除しておくこと）

	------------------------------------------------


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

#include "BP_Renderer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_Debug.h"


/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(6897906)	/* "comdl" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(128)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	ComdlPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginComdl( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &ComdlPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &ComdlPlugin );

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginComdl( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &ComdlPlugin );
		DG_FreePlugin( &ComdlPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクト登録
	*/
int DG_QueueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return ( -1 );
	}
	DG_QueueUserObject( ComdlPlugin.obj_buffer, comdl );
	return ( 0 );
}

	/*
		オブジェクト削除
	*/
void DG_DequeueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( ComdlPlugin.obj_buffer, comdl );
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

typedef struct {
	DG_COMDL	*comdl ;
	int			use_tri ;
} OBJ_LIST ;

typedef struct _scrpad_work {
	FMATRIX	mat ;
	u_long128	dma_buffer[256] ;
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	packet_info ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

int			*func_list ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern char	*DG_AS_GetLocalMemoryAddress();


#if 1 //BP_GCC
static Vu1Work	default_vu1_work = {
   { 0.0f, 0.0f, 0.0f, -1.0f }, //clamp_min
   {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f }, //clamp_max
   0, //fog_param1
   0, //fog_param2
   0, //fog_param3
   0, //fog_param4
   255.0f, //color_clip
   128.0f, //poly_alpha
   0.0f, //specular_mul
   0.8f, //specular_clip
   { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f }, //scale
   { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f }, //offset
   0
};
#else
/* デフォルトのＶＵ１ワークパラメータ */
static Vu1Work	default_vu1_work = {
	.clamp_min = {.vx = 0.0f, .vy = 0.0f, .vz = 0.0f, .vw = -1.0f },
	.clamp_max = {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f },
	.fog_param1 = 0, .fog_param2 = 0, .fog_param3 = 0, .fog_param4 = 0,
	.color_clip = 255.0f, .poly_alpha = 128.0f, .specular_mul = 0.0f, .specular_clip = 0.8f, 
	.scale = { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
	.offset = { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f },
};
#endif

/*----------------------------------------------------------------*/
	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/
#if 0 //BP_PS2
static int ComdlObjsPacketInit( void *tag_addr, DG_CHANL *cp )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/*
		初期化データを設定
	*/
	tag = DG_WritePacket_VIF1Init( tag );

	/*
		マイクロプログラムロード設定パケットの登録
	*/
	tag = DG_WritePacket_Vu1ComdlProg( tag, 0, &func_list );

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
#endif
/*----------------------------------------------------------------*/
	/*
		初期化実行パケットの設定
	*/
#if 0 //BP_PS2
static int WriteObjTransPacket( void *tag_addr, u_long128 *subtag )
{
	unsigned int	*tag ;

	tag = (unsigned int*)tag_addr ;

	/* モデルデータ転送パケット接続 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	tag[1] = DMATAG_SET_ADDR( subtag );
	tag[3] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[2] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag += 4 ;

	/* プログラムの初期化実行 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;
#if 0 //BP_RENDER
	tag[3] = SCE_VIF1_SET_MSCAL( func_list[0], 0 );
#endif
	tag += 4 ;

	return ( ( (int)tag - (int)tag_addr ) / sizeof(u_long128) );
}
#endif
	/*
		各オブジェクトのＤＭＡパケットの作成
	*/
#if 0 //BP_PS2
static int WriteObjPacks( void *tag_addr, DG_COMDL_POS *pos, int n_pos )
{
	/* オブジェクトパケットのＤＭＡデータを接続する */
	unsigned int	*tag ;
	u_long128		*src, *dst ;
	int				n ;

	tag = (unsigned int*)tag_addr ;

	/* マトリクス転送タグ作成 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, n_pos * 5 + 1 );
	tag[2] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	tag[3] = SCE_VIF1_SET_UNPACK( 0x040, n_pos * 5 + 1, VIF_DATA128, 0 ) ;
	tag += 4 ;

	/* マトリクス転送数設定 */
	tag[0] = n_pos ;
	tag += 4 ;

	/* マトリクス＆ポリゴンカラーデータをストアする */
	src = (u_long128*)pos ;
	dst = (u_long128*)tag ;
	for ( n = n_pos * 5 ; n > 0 ; n-- ){
#if 0 //BP_PS2
		if ( ( (int)src & 0x3f ) == 0 ) asm("pref 0,64(%0)"::"r"(src));
#endif
		*dst++ = *src++ ;
	}
	tag = (unsigned int*)dst ;

	/* マイクロプログラムの再実行 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[3] = SCE_VIF1_SET_MSCNT( 0 ) ;
	tag += 4 ;

	return ( ( (int)tag - (int)tag_addr ) / sizeof(u_long128) );
}
#endif
/*----------------------------------------------------------------*/
#if 0
	/*
		共有モデルオブジェクト処理
	*/
void DG_ComdlChanl( DG_CHANL *cp, int which )
{
	DG_OBJ_QUEUE	*queue ;
	DG_OBJ_BUFFER	*obj_buff ;
	DG_COMDL		*comdl, **que ;
	DG_COMDL_POS	*pos ;
	int		i, j, gid, c_gid, size, n, time, old_tex_code, invisible_flag ;
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list **tex_list_p ;
	void			*buffer ;

	MARK( "comdl.c" );
	scrpad = SCRPAD_ADDR ;
#ifdef LIBDG_PERFORMANCE
	scrpad->packet_info = DG_PerformanceData.comodel ;
#endif

	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->comdl_buffer ;
	invisible_flag = DG_COMDL_INVISIBLE0 << cp->chanl_num ;
	/*
		キュー数のチェック
	*/
	if ( obj_buff->n_queue == 0 ) return ;

	scrpad->buffer_switch = 0 ;

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	/* 初期化パケットの設定 */
	buffer = scrpad->dma_buffer[scrpad->buffer_switch] ;
	size = ComdlObjsPacketInit( buffer, cp );
	DG_StartSprToMem( DG_CurrentDmaAddr, buffer, size );
	DG_CurrentDmaAddr += size ;
	scrpad->buffer_switch = 1 - scrpad->buffer_switch ;

	/* テクスチャ単位での処理（通常不透明オブジェクト処理と同じ） */
	for ( tex_list_p = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list_p++, j-- ){
		tex_list = *tex_list_p;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
		old_tex_code = -1 ;

		/* 全オブジェクトの検索 */
		que = obj_buff->queue ;
		for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
			comdl = *que ;

			if ( comdl->flag & invisible_flag ) continue ;
			if ( comdl->tri_id != tex_list->code ) continue ;

			/* テクスチャのセットアップ */
			if ( old_tex_code == -1 ){
#ifdef LIBDG_PERFORMANCE
				DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
				buffer = scrpad->dma_buffer[scrpad->buffer_switch] ;
				//size = DG_WriteTextureChangePacks( buffer, &tex_list->tex_packet[which] );
				size = DG_WriteTextureChangePacks2( buffer, tex_list, which );
				DG_EndSprToMem();
				DG_StartSprToMem( DG_CurrentDmaAddr, buffer, size );
				DG_CurrentDmaAddr += size ;
				scrpad->buffer_switch = 1 - scrpad->buffer_switch ;
				old_tex_code = tex_list->code ;
			}

			/* 初期化パケット転送 */
			buffer = scrpad->dma_buffer[scrpad->buffer_switch] ;
			size = WriteObjTransPacket( buffer, &comdl->packet[which] );
			DG_EndSprToMem();
			DG_StartSprToMem( DG_CurrentDmaAddr, buffer, size );
			DG_CurrentDmaAddr += size ;
			scrpad->buffer_switch = 1 - scrpad->buffer_switch ;

			/* 各オブジェクトのマトリクス転送パケット設定 */
			n = comdl->n_objs ;
			pos = comdl->pos ;
#ifdef LIBDG_PERFORMANCE
			scrpad->packet_info.n_obj++ ;
			scrpad->packet_info.n_packs += comdl->n_objs ;
			scrpad->packet_info.n_verts += comdl->packet[0].dg_comdl_packet_data.n_verts * comdl->n_objs ;
#endif
			while ( n > 0 ){
				int		trans_pos ;
				/* 最大転送サイズは３２個分まで */
				trans_pos = ( n > 32 ) ? 32 : n ;
				n -= trans_pos ;
				/* パケットの生成 */
				buffer = scrpad->dma_buffer[scrpad->buffer_switch] ;
				size = WriteObjPacks( buffer, pos, trans_pos );
				DG_EndSprToMem();
				DG_StartSprToMem( DG_CurrentDmaAddr, buffer, size );
				DG_CurrentDmaAddr += size ;
				scrpad->buffer_switch = 1 - scrpad->buffer_switch ;
				pos += trans_pos ;
			}
		}
	}

	/* 終了パケットの設定 */
	buffer = scrpad->dma_buffer[scrpad->buffer_switch] ;
	size = DG_WriteObjsPacketEnd( buffer );
	DG_EndSprToMem();
	DG_StartSprToMem( DG_CurrentDmaAddr, buffer, size );
	DG_CurrentDmaAddr += size ;
	scrpad->buffer_switch = 1 - scrpad->buffer_switch ;

	/* ＤＭＡ転送終了待ち */
	DG_EndSprToMem();

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.comodel = scrpad->packet_info ;
#endif
}
#else
	/*
		プラグイン実行アクター
	*/
#if 0 //BP_PS2
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_COMDL		*comdl, **que ;
	DG_COMDL_POS	*pos ;
	int		i, j, size, n, old_tex_code, invisible_flag ;
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int				max_objs, max_mem_objs ;

	scrpad = SCRPAD_ADDR ;
#ifdef LIBDG_PERFORMANCE
	scrpad->packet_info = DG_PerformanceData.comodel ;
#endif

	invisible_flag = DG_COMDL_INVISIBLE0 << cp->chanl_num ;
	/*
		キュー数のチェック
	*/
	if ( obj_buff->n_queue == 0 ) return ;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	que = (DG_COMDL**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, que++ ) {
		comdl = *que ;
		if ( comdl->flag & invisible_flag ) continue ;
		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( comdl->group_id & cp->group_id ) ) continue ;
		obj_list[max_objs].comdl = comdl ;
		obj_list[max_objs].use_tri = comdl->tri_id ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをテクスチャ毎にソートしてメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	max_mem_objs = 0 ;
	for ( tex_list_p = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list_p++, j-- ){
		tex_list = *tex_list_p;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
#ifdef LIBDG_CHECK_MULTILOADTEX
		//if ( GV_Time & 2 ) if ( tex_list->flag ) continue ;
#endif
		for ( i = 0 ; i < max_objs ; i++ ) {
			if ( tex_list->code != obj_list[i].use_tri ) continue ;
			mem_obj_list->comdl = obj_list[i].comdl ;
			mem_obj_list->use_tri = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}
	}

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	size = ComdlObjsPacketInit( scrpad->dma_buffer, cp );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	//tex_list = DG_TextureList ;
	old_tex_code = -1 ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){
		/* テクスチャのチェック */
		if ( mem_obj_list->use_tri != old_tex_code ){
			tex_list = (DG_TEXTURE_LIST*)mem_obj_list->use_tri ;
#ifdef LIBDG_PERFORMANCE
			DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
			tex_list->flag = 1 ;
			//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
			size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			old_tex_code = mem_obj_list->use_tri ;
			tex_list++ ;
		}
		comdl = mem_obj_list->comdl ;

		/* 初期化パケット転送 */
		size = WriteObjTransPacket( scrpad->dma_buffer, (void*)&comdl->packet[which] );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

		/* 各オブジェクトのマトリクス転送パケット設定 */
		n = comdl->n_objs ;
		pos = comdl->pos ;
#ifdef LIBDG_PERFORMANCE
		scrpad->packet_info.n_obj++ ;
		scrpad->packet_info.n_packs += comdl->n_objs ;
		scrpad->packet_info.n_verts += comdl->packet[0].dg_comdl_packet_data.n_verts * comdl->n_objs ;
#endif
		while ( n > 0 ){
			int		trans_pos ;
			/* 最大転送サイズは３２個分まで */
			trans_pos = ( n > 32 ) ? 32 : n ;
			n -= trans_pos ;
			/* パケットの生成 */
			size = WriteObjPacks( scrpad->dma_buffer, pos, trans_pos );
			DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			pos += trans_pos ;
		}
		mem_obj_list++ ;
	}

	/* 終了パケットの設定 */
	size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	/* ＤＭＡ転送終了待ち */
	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.comodel = scrpad->packet_info ;
#endif

}

#else

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_COMDL		*comdl, **que ;
	DG_COMDL_POS	*pos ;
	int		i, j, size, n, old_tex_code, invisible_flag ;
	ScrpadWork		*scrpad ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int				max_objs, max_mem_objs ;

	scrpad = SCRPAD_ADDR ;
	invisible_flag = DG_COMDL_INVISIBLE0 << cp->chanl_num ;
	/*
		キュー数のチェック
	*/
	if ( obj_buff->n_queue == 0 ) return ;

   //if( !(cp->flag & DG_VIEWPORT_STAGE_DISABLE_MSAA) )
   if( 1 )
   {
      BP_GetCurrentViewportInfo()->hasMSAAStuff = 1;
   }
   else
   {
      BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   }
   BP_RB_PushRegionMarker(kProfileColor_Clone, "Start Comdl");
   BP_Debug_PushCPUMarker( "Start Comdl" );
   // Send init packet
   {
      SBP_Comdl_InitPacket* pBPData = (SBP_Comdl_InitPacket*)BP_RB_Alloc(sizeof(SBP_Comdl_InitPacket));

      pBPData->pers = cp->eye_pers;
      pBPData->fogParam[0] = BP_FogParam.x;
      pBPData->fogParam[1] = BP_FogParam.y;
      pBPData->fogParam[2] = BP_FogParam.z;
      pBPData->fogParam[3] = BP_FogParam.w;

      BP_RB_AddCommand(kCmd_Comdl_InitPacket, (char*)pBPData);
   }

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	que = (DG_COMDL**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, que++ ) 
   {
		comdl = *que ;
		if ( comdl->flag & invisible_flag ) continue ;
		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( comdl->group_id & cp->group_id ) ) continue ;
		obj_list[max_objs].comdl = comdl ;
		obj_list[max_objs].use_tri = comdl->tri_id ;
		max_objs++ ;

      //TODO: bounds check?

      // Render clone object
      {
         int const posAllocSize = sizeof(DG_COMDL_POS) * comdl->n_objs;

         SBP_Comdl_Render* pBPData = (SBP_Comdl_Render*)BP_RB_Alloc(sizeof(SBP_Comdl_Render));

         pBPData->flag = comdl->flag;

         BP_RB_CopyTexture(&pBPData->tex, comdl->tex);

         pBPData->n_objs = comdl->n_objs;

         pBPData->pos = (DG_COMDL_POS*)BP_RB_Alloc(posAllocSize);
         memcpy(pBPData->pos, comdl->pos, posAllocSize);

         pBPData->n_verts = comdl->packet[which].dg_comdl_packet_data.n_verts;
         pBPData->verts = comdl->packet[which].verts_tag.addr;
         pBPData->norms = comdl->packet[which].norms_tag.addr;
         pBPData->uvs = comdl->packet[which].uvs_tag.addr;

         BP_RB_AddCommand(kCmd_Comdl_Render, (char*)pBPData);
      }
	}

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
}

#endif

#endif
/*----------------------------------------------------------------*/
	/*
		初期化ＤＭＡパケットの作成
	*/
static void SetPaket( DG_COMDL *comdl, DG_MDLPACK *mdl_pack, DG_COMDL_PACKET *packet )
{
	DG_TEX		*tex ;
	u_long64		prim, alpha ;
	int			n ;

	/* ＤＭＡ転送タグ＆パラメータ設定 */
	tex = (DG_TEX*)mdl_pack->tex_id[0] ;

	prim = SCE_GS_SET_PRIM( 0, 1, 1, 1, 0, 0, 0, 0, 0) ;
	alpha = tex->tex_trans.alpha.data ;
	if ( comdl->flag & DG_COMDL_SEMITRANS ){
		prim |= SCE_GS_PRIM_ABE ;	/* 半透明化 */
	}
	if ( comdl->flag & DG_COMDL_NOFOG ){
		prim &= ~SCE_GS_PRIM_FGE ;	/* 半透明化 */
	}

	/* 頂点の転送（端数の場合には切りのいいところまでデータがあることにする） */
	n = ( mdl_pack->n_verts + 1 ) / 2 ;
	packet->verts_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->verts_tag.addr = mdl_pack->verts ;
	packet->verts_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->verts_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+2, n*2, VIF_VERT_PACK, 0 ) ;
	/* 法線の転送 */
	packet->norms_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->norms_tag.addr = mdl_pack->norms ;
	packet->norms_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->norms_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+1, n*2, VIF_NORM_PACK, 0 ) ;
	/* テクスチャ座標の転送 */
	n = ( mdl_pack->n_verts + 3 ) / 4 ;
	packet->uvs_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->uvs_tag.addr = mdl_pack->uvs[0];
	packet->uvs_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->uvs_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+0, n*4, VIF_UV_PACK, 0 ) ;

	/* テクスチャ及び頂点転送用ＧＩＦタグの埋め込み */
	packet->datas_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(packet->dg_comdl_packet_data) );
	packet->datas_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	packet->datas_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x310, SIZEOF_QWORD(packet->dg_comdl_packet_data), VIF_DATA128, 0 ) ;

	/* データパラメータのセット */
	packet->dg_comdl_packet_data.n_verts = mdl_pack->n_verts ;	/* 頂点データ数 */
	packet->dg_comdl_packet_data.data_offset = 5+3+1+1 ;			/* データ開始オフセット */

	/* クリップポリゴン描画用設定 */
	packet->dg_comdl_packet_data.giftag0.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1) ;
	packet->dg_comdl_packet_data.giftag0.regs = GS_REGS_1(GS_REGS_AD) ;
	/* プリミティブパケットの設定 */
	packet->dg_comdl_packet_data.prim.data = prim | SCE_GS_PRIM_TRIFAN ;
	packet->dg_comdl_packet_data.prim.reg = SCE_GS_PRIM ;
	/* プリミティブパケット用ＧＩＦタグの設定 */
	packet->dg_comdl_packet_data.giftag1.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, SCE_GIF_PACKED, 3) ;
	packet->dg_comdl_packet_data.giftag1.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;

	/* テクスチャパラメータパケットの設定 */
	packet->dg_comdl_packet_data.giftag2.tag = SCE_GIF_SET_TAG(4, 1, 0, 0, 0, 1) ;
	packet->dg_comdl_packet_data.giftag2.regs = GS_REGS_1(GS_REGS_AD) ;
	/* クランプパラメータ設定 */
	packet->dg_comdl_packet_data.clamp.data = tex->tex_trans.clamp.data ;
	packet->dg_comdl_packet_data.clamp.reg = SCE_GS_CLAMP_1 ;
	/* テクスチャパラメータ設定 */
	packet->dg_comdl_packet_data.tex2.data = tex->tex_trans.tex2.data ;
	packet->dg_comdl_packet_data.tex2.reg = SCE_GS_TEX2_1 ;
	/* テクスチャパラメータ設定 */
	//packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data | (u_long64)2<<35;
	packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data ;
	//packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data | (u_long64)1<<35;
	packet->dg_comdl_packet_data.tex0.reg = SCE_GS_TEX0_1 ;
	/* アルファブレンティング設定 */
	packet->dg_comdl_packet_data.alpha.data = alpha ;
	packet->dg_comdl_packet_data.alpha.reg = SCE_GS_ALPHA_1 ;
	/* プリミティブパケット用ＧＩＦタグの設定 */
	packet->dg_comdl_packet_data.giftag3.tag = 
	  SCE_GIF_SET_TAG( mdl_pack->n_verts, 1, 1,
					  prim | SCE_GS_PRIM_TRISTRIP,
					  SCE_GIF_PACKED, 3) ;
	packet->dg_comdl_packet_data.giftag3.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;

}

/*----------------------------------------------------------------*/
	/*
		共有モデルオブジェクトの作成
	*/
DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
{
	DG_COMDL	*comdl ;
	int			size ;

	flag |= DG_COMDL_INVISIBLE2|DG_COMDL_INVISIBLE3 ;
	ASSERT( mdl_pack != NULL )
	size = sizeof(DG_COMDL) + sizeof(DG_COMDL_POS) * n_comdl ;
	if ( ( comdl = GV_Malloc( size ) ) == NULL ){
		return NULL ;
	}
	GV_ZeroMemory( comdl, size );

	comdl->flag = flag ;
	comdl->n_objs = n_comdl ;
	comdl->chanl = chanl ;
	comdl->tri_id = DG_SearchTriFromTex( (DG_TEX*)( mdl_pack->tex_id[0] ) );
   comdl->tex = (DG_TEX*)( mdl_pack->tex_id[0] );

	/* パケットの初期化 */
	SetPaket( comdl, mdl_pack, &comdl->packet[0] );
	SetPaket( comdl, mdl_pack, &comdl->packet[1] );

	return ( comdl ) ;
}

	/*
		メモリ開放
	*/
void	DG_FreeComdl( DG_COMDL *comdl )
{
	GV_DelayedFree( comdl );
}
