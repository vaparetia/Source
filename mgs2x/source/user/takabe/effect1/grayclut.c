//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	grayclut.c
	モノクロＣＬＵＴエフェクト実験

	1999/11/15 K.Takabe
	$Id: grayclut.c,v 1.4 2002/11/23 12:24:51 Yoshizawa1 Exp $

*/
/*

	どちらのエフェクトも一回の起動に対して大量のメモリを消費するので
	無駄な多重呼びは避けること！！


	void *NewGrayClut( int time, u_int add_col );
	int	time ;		影響時間（１／３００秒単位）
	int	add_col ;	モノクロ後に加算する色（0x00BBGGRRで指定）

		テクスチャにモノクロ化したＣＬＵＴを一定時間割り当てる
		使用例）
			NewGrayClut( 100, 0x00202020 );


	void *NewContrastClut( int time, SVECTOR *scale, SVECTOR *bias );
	int		time ;		影響時間（１／３００秒単位）
	SVECTOR	*scale ;	カラースケール（各要素に対して256で1.0倍となるスケールを指定）
	SVECTOR	*bias ;		バイアス値（スケールの中心値＝１２８に対するバイアス値を設定）

		テクスチャにコントラスト調整したＣＬＵＴを一定時間割り当てる
		使用例）
			{
				SVECTOR	scale = { 512, 512, 512, 0 };
				SVECTOR	bias = { 32, 32, 32, 0 };
				NewContrastClut( 100, &scale, &bias );
			}

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
#include	"dmapack.h"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

/* ---------------------------------------------------------------- */
#ifdef PSX2
/*
	ＰＳ２用ルーチン
*/

#if 0 //BP_TODO

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

/* ---------------------------------------------------------------- */
typedef struct {
	u_int	*src ;	/* 加工元オリジナルデータアドレス */
	u_int	*dst ;	/* 格納先ＣＬＵＴバッファアドレス */
	int		size ;	/* ＣＬＵＴエントリ数 */
	int		pad ;	/*  */
} TransDataInfo ;

typedef	struct	{
	GV_ACT_EX	actor ;
	int			type ;
	int			time ;
	IVECTOR		scale ;
	IVECTOR		bias ;
	void		*tmp_clut_addr ;
	int			n_list ;
	TransDataInfo	datainfo[0] ;
} Work ;

typedef struct {
	u_int		color_buffer[2][2048] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
static void *ClutAnimeChara = NULL ;

/* ---------------------------------------------------------------- */
static void Mono_Clut( u_int *col, int i, IVECTOR *bias )
{
#if 1
	u_long128 mask1 = 0xff000000ff000000ff000000ff000000 ;
	u_long128 mask2 = 0x00000000ffffffff00000000ffffffff ;
	u_long128 scale = 0x00000000000000550000000000000055 ;
	asm volatile ("
		lq			$13,0x00(%4)
0:
		lq			$12,0x00(%0)		# 
		pextlb		$8,$0,$12			# 前半２ワード生成
		pextub		$9,$0,$12			# 後半２ワード生成
		psrlw		$10,$8,16			# 前
		pexeh		$11,$8				# 前
		padduh		$8,$8,$10			# 前
		padduh		$8,$8,$11			# 前
		pand		$8,$8,%5			# 
		pmulth		$8,$8,%2			# 前
		psrlw		$10,$9,16			# 後
		pexeh		$11,$9				# 後
		padduh		$9,$9,$10			# 後
		padduh		$9,$9,$11			# 後
		pand		$9,$9,%5			# 
		pmulth		$9,$9,%2			# 後
		psrlw		$8,$8,8			# 前
		pand		$12,$12,%3			# アルファの保存
		psrlw		$9,$9,8			# 後
		ppacb		$8,$9,$8			# 合成
		psllw		$10,$8,8			# 緑成分生成
		psllw		$11,$8,16			# 青成分生成
		por			$8,$8,$10			# 緑成分合成
		por			$8,$8,$11			# 青成分合成
		por			$12,$12,$8			# アルファの合成
		paddub		$12,$12,$13			# バイアス値加算
		addi		%1,%1,-4			# 
		sq			$12,0x00(%0)		# 
		addiu		%0,%0,16			# 
		bgtz		%1,0b				# 
		nop
		nop
	"::
	"r"(col), "r"(i), "r"(scale), "r"(mask1),"r"(bias),"r"(mask2):
	"$8","$9","$10","$11","$12","$13" );
#else
	int		tmp ;
	for (  ; i > 0 ; i-- ){
		tmp = ( ( *col ) & 255 ) + ( ( *col >> 8 ) & 255 ) + ( ( *col >> 16 ) & 255 ) ;
		tmp = ( tmp * ( 256/3 ) ) >> 8 ;
		*col = ( *col & 0xff000000 ) | ( tmp << 16 ) | ( tmp << 8 ) | tmp ;
		col++ ;
	}
#endif
}
static void Contrast_Clut( u_int *col, int i, IVECTOR *scale, IVECTOR *bias )
{
#if 1
	static FVECTOR	param = { 255.0f, 0, 0, 0 };
	u_long128 mask1 = 0xff000000ff000000ff000000ff000000 ;
	asm volatile ("
		lqc2		$vf01,0x00(%0)		# 
		lqc2		$vf02,0x00(%1)		# 
		lqc2		$vf03,0x00(%2)		# 
		vitof12		$vf01,$vf01			# 
		vitof0		$vf02,$vf02			#
	"::"r"(scale),"r"(bias),"r"(&param));
	asm volatile ("
0:
		lq			$12,0x00(%0)		# 
		pextlb		$8,$0,$12			# 前半２ワード生成
		pextub		$9,$0,$12			# 後半２ワード生成
		pextlh		$10,$0,$8			# 第１カラー
		pextuh		$11,$0,$8			# 第２カラー
		qmtc2		$10,vf04			# 第１カラー
		qmtc2		$11,vf05			# 第２カラー
		pextlh		$10,$0,$9			# 第３カラー
		pextuh		$11,$0,$9			# 第４カラー
		qmtc2		$10,vf06			# 第３カラー
		qmtc2		$11,vf07			# 第４カラー
		vitof0.xyzw	vf04,vf04			# 
		vitof0.xyzw	vf05,vf05			# 
		vitof0.xyzw	vf06,vf06			# 
		vitof0.xyzw	vf07,vf07			# 
		vadd.xyzw	vf04,vf04,vf02		# 
		vadd.xyzw	vf05,vf05,vf02		# 
		vadd.xyzw	vf06,vf06,vf02		# 
		vadd.xyzw	vf07,vf07,vf02		# 
		vmul.xyzw	vf04,vf04,vf01		# 
		vmul.xyzw	vf05,vf05,vf01		# 
		vmul.xyzw	vf06,vf06,vf01		# 
		vmul.xyzw	vf07,vf07,vf01		# 
		vminix.xyzw	vf04,vf04,vf03		# 
		vminix.xyzw	vf05,vf05,vf03		# 
		vminix.xyzw	vf06,vf06,vf03		# 
		vminix.xyzw	vf07,vf07,vf03		# 
		vmaxx.xyzw	vf04,vf04,vf00		# 
		vmaxx.xyzw	vf05,vf05,vf00		# 
		vmaxx.xyzw	vf06,vf06,vf00		# 
		vmaxx.xyzw	vf07,vf07,vf00		# 
		vftoi0.xyzw	vf04,vf04			# 
		vftoi0.xyzw	vf05,vf05			# 
		vftoi0.xyzw	vf06,vf06			# 
		vftoi0.xyzw	vf07,vf07			# 
		qmfc2		$8,vf04			# 
		qmfc2		$9,vf05			# 
		qmfc2		$10,vf06			# 
		qmfc2		$11,vf07			# 
		ppach		$8,$9,$8			# 
		ppach		$10,$11,$10			# 
		ppacb		$8,$10,$8			# 
		pand		$12,$12,%2			# アルファの保存
		por			$12,$12,$8			# アルファの合成
		addi		%1,%1,-4			# 
		sq			$12,0x00(%0)		# 
		addiu		%0,%0,16			# 
		bgtz		%1,0b				# 
	"::
	"r"(col), "r"(i), "r"(mask1):
	"$8","$9","$10","$11","$12" );
#else
	int		r, g, b ;
	for (  ; i > 0 ; i-- ){
		r = ( *col ) & 255 ;
		g = ( *col >> 8 ) & 255 ;
		b = ( *col >> 16 ) & 255 ;
		r = ( ( r + bias->vx ) * scale->vx ) >> 12 ;
		g = ( ( g + bias->vy ) * scale->vy ) >> 12 ;
		b = ( ( b + bias->vz ) * scale->vz ) >> 12 ;
		if ( r < 0 ) r = 0 ;
		if ( r > 255 ) r = 255 ;
		if ( g < 0 ) g = 0 ;
		if ( g > 255 ) g = 255 ;
		if ( b < 0 ) b = 0 ;
		if ( b > 255 ) b = 255 ;
		*col = ( *col & 0xff000000 ) | ( b << 16 ) | ( g << 8 ) | r ;
		col++ ;
	}
#endif
}
#if 0
static void MonoContrast_Clut( u_int *col, int i, int scale, int bias )
{
	int		tmp ;
	for (  ; i > 0 ; i-- ){
		tmp = ( ( *col ) & 255 ) + ( ( *col >> 8 ) & 255 ) + ( ( *col >> 16 ) & 255 ) ;
		tmp = ( tmp * ( 256/3 ) ) >> 8 ;
		tmp = ( tmp + bias ) * scale >> 8 ;
		if ( tmp < 0 ) tmp = 0 ;
		if ( tmp > 255 ) tmp = 255 ;
		*col = ( *col & 0xff000000 ) | ( tmp << 16 ) | ( tmp << 8 ) | tmp ;
		col++ ;
	}
}
#endif

static void MakeClut( Work *work, TransDataInfo *datainfo, int n_list )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int			buffer = 0 ;
	int			s, first_flag = 1 ;

	/* 最初のデータをスクラッチパッドへ */
	DG_StartMemToSpr( scrpad->color_buffer[ buffer ], datainfo->src, datainfo->size / 4 );

	while ( n_list != 0 ){
		DG_EndMemToSpr();

		/* 次のデータがある場合には次のデータをスクラッチパッドへ */
		if ( n_list > 1 ){
			DG_StartMemToSpr( scrpad->color_buffer[ 1 - buffer ], datainfo[1].src, datainfo[1].size / 4 );
		}
		s = datainfo->size ;
		/* ＣＬＵＴの加工処理 */
		switch ( work->type ){
		  case 0:
			Mono_Clut( scrpad->color_buffer[ buffer ], s, &work->bias );
			//MonoContrast_Clut( scrpad->color_buffer[ buffer ], s, 512, -32 );
			break ;
		  case 1:
			Contrast_Clut( scrpad->color_buffer[ buffer ], s, &work->scale, &work->bias );
			break ;
		}

		if ( first_flag == 0 ){
			DG_EndSprToMem();
		} else {
			first_flag = 0 ;
		}
		DG_StartSprToMem( datainfo->dst, scrpad->color_buffer[ buffer ], s / 4 );

		datainfo++ ;
		n_list-- ;
		buffer = 1 - buffer ;
	}

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i ;
	DG_TEXTURE_LIST		*list, **list_p ;
	void				*mem ;

	//if ( GV_PadData[0].status & PAD_Y ) return ;
	if ( work->time != -1 ){
		/* カウンタチェック */
		if ( work->time <= 0 ){
			GV_DestroyActor( work );
			return ;
		}
		if ( GV_PauseLevel == 0 ) work->time -= CLOCK_COUNT ;
		if ( work->time < 0 ) work->time = 0 ;
	}

	/* 初期化したメモリを一時ＣＬＵＴアドレスに設定する */
	mem = work->tmp_clut_addr ;
	list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i--, list_p++ ){
		list = *list_p;
		if ( list->header->compress_flag & TRI_FLAG_STATIC ) continue ;
		list->tmp_clut_image = mem ;
		mem = (void*)( (int)mem + list->header->clut_size * 64 * 4 ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	ClutAnimeChara = NULL ;
	if ( work->tmp_clut_addr != NULL ) GV_DelayedFree( work->tmp_clut_addr );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int type, int time )
{
	int		all_clut_size = 0, static_clut_size = 0 ;
	int		i ;
	DG_TEXTURE_LIST		*list, **list_p ;
	void				*mem ;
	TransDataInfo		*datainfo ;

	work->time = time ;
	work->type = type ;

	if ( ClutAnimeChara != NULL ){
		GV_DestroyActor( ClutAnimeChara );
	}
	ClutAnimeChara = work ;

	/* ＣＬＵＴの全サイズを求める */
	list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i-- ){
		list = *list_p;
		if ( !( list->header->compress_flag & TRI_FLAG_STATIC ) ){
			//printf("%d textures, clut %d use block\n", list->header->n_textures, list->header->clut_size );
			//all_clut_size += ( list->header->clut_size * 64 + 2047 ) & 0xfffff800 ;
			all_clut_size += list->header->clut_size * 64 ;
			//printf("%d %d, %d\n", list->header->n_textures, list->header->clut_size , list->header->clut_size * 64 * 4 );
		} else {
			//static_clut_size += list->header->clut_size * 64 + 2047 ) & 0xfffff800 ;
			static_clut_size += list->header->clut_size * 64  ;
		}
		list_p++ ;
	}
	printf("%s:total %d tri, %d word, %d bytes\n", __FILE__, DG_MaxTextures, all_clut_size, all_clut_size * 4 );

	/* １２８バイトにアラインしたメモリを確保 */
	mem = work->tmp_clut_addr = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, ALIGNSIZE128( all_clut_size*4 ) , 128 );
	if ( work->tmp_clut_addr == NULL ) return ( -1 );

	/* データ変換リストを作成する */
	datainfo = work->datainfo ;
	list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i--, list_p++ ){
		u_int	*src, *dst, s, size ;

		list = *list_p;
		if ( list->header->compress_flag & TRI_FLAG_STATIC ) continue ;
		src = list->clut_image ;
		dst = mem ;
		size = list->header->clut_size * 64 ;
		while ( size != 0 ){
			s = ( size > 2048 ) ? 2048 : size ;
			size -= s ;
			datainfo->src = src ;
			datainfo->dst = dst ;
			datainfo->size = s ;
			src += s ;
			dst += s ;
			datainfo++ ;
			work->n_list++ ;
		}
		mem = (void*)( (int)mem + list->header->clut_size * 64 * 4 ) ;
	}

	/* まとめてＣＬＵＴ変換 */
	FlushCache( 0 );
	MakeClut( work, work->datainfo, work->n_list );
	FlushCache( 0 );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewGrayClut( int time, u_int add_col )
{
	Work		*work ;
	int			clut_buffer_entry = 0 ;

	{
		/* 書き換える必要のあるＣＬＵＴエントリバッファ量を算出 */
		int		i ;
		DG_TEXTURE_LIST		**list ;
		/* ＣＬＵＴの全サイズを求める */
		list = DG_TextureList ;
		for ( i = DG_MaxTextures ; i > 0 ; i-- ){
			clut_buffer_entry += ( ( *list )->header->clut_size * 64 + 2047 ) / 2048 ;
			list++ ;
		}
	}

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) + sizeof(TransDataInfo) * clut_buffer_entry ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->bias.vx = add_col ;
		work->bias.vy = add_col ;
		work->bias.vz = add_col ;
		work->bias.vw = add_col ;
		if ( GetResources( work, 0, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewContrastClut( int time, SVECTOR *scale, SVECTOR *bias )
{
	Work		*work ;
	int			clut_buffer_entry = 0 ;

	{
		/* 書き換える必要のあるＣＬＵＴエントリバッファ量を算出 */
		int		i ;
		DG_TEXTURE_LIST		**list ;
		/* ＣＬＵＴの全サイズを求める */
		list = DG_TextureList ;
		for ( i = DG_MaxTextures ; i > 0 ; i-- ){
			clut_buffer_entry += ( (*list)->header->clut_size * 64 + 2047 ) / 2048 ;
			list++ ;
		}
	}

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) + sizeof(TransDataInfo) * clut_buffer_entry ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->scale.vx = scale->vx << 4 ;
		work->scale.vy = scale->vy << 4 ;
		work->scale.vz = scale->vz << 4 ;
		work->scale.vw = 0 ;
		work->bias.vx = -( 128 * scale->vx / 256 - 128 ) + bias->vx ;
		work->bias.vy = -( 128 * scale->vy / 256 - 128 ) + bias->vy ;
		work->bias.vz = -( 128 * scale->vz / 256 - 128 ) + bias->vz ;
		work->bias.vw = 0 ;
		if ( GetResources( work, 1, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#else //BP
#endif

#else //PSX2
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			type ;
	int			time ;
	IVECTOR		scale ;
	IVECTOR		bias ;
	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;
} Work ;


typedef struct {
	DG_DMAPACK_SPRT			sprt ;
} SCREEN_DRAW ;
typedef struct {
	/* 初期化パケット */
	DG_DMAPACK_PARAM		frame_backup ;
	DG_DMAPACK_PARAM		use_frametex ;
	DG_DMAPACK_CALLBACK		set_pixel_shader ;

	/* メイン描画パケット */
	SCREEN_DRAW				screen_draw ;

	/* 変更環境の復元パケット */
	DG_DMAPACK_CALLBACK		reset_pixel_shader ;

	DG_DMAPACK_PARAM		end ;
} ALL_PACKET ;

#define DMAPACK_PRIO	(135)	/* ２Ｄシステムより低いir_mode.cよりさらに低く */
/* ---------------------------------------------------------------- */
static void *ClutAnimeChara = NULL ;
extern DG_PIXELSHADER	DG_DmapackPixelShader[4] ;	/* 予めLIBDGで用意されているものを使用する */
//static int MakePixelShaderFlag = 0 ;
//static DG_PIXELSHADER	MonoPixelShader ;

/* ---------------------------------------------------------------- */
static void Dmapacket_MonoPixelShaderSet( int param )
{
	DG_SetPixelShader( &DG_DmapackPixelShader[0] );	/* モノクロ化シェーダー */
#ifndef KP_WINDOWS

   BP_RENDER_TODO_BREAK;
#if 0 //BP
	DG_SetRenderState( D3DRS_PSCONSTANT0_1, param );
#endif

#endif
}
static void Dmapacket_MonoPixelShaderReset( int param )
{
	DG_SetPixelShader( NULL );	/* モノクロ化シェーダー */
}
/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int offset )
{
	/* 初期化パケットの初期化 */
	DG_SetDmapackBackupFrame( &packet->frame_backup, 0 );
	DG_SetDmapackUseFrameTex( &packet->use_frametex, 2 );
	DG_SetDmapackPacketCallback( &packet->set_pixel_shader, Dmapacket_MonoPixelShaderSet, offset );
	DG_SetDmapackPacketCallback( &packet->reset_pixel_shader, Dmapacket_MonoPixelShaderReset, 0 );
	/* 終了パケットの初期化 */
	DG_SetDmapackEnd( &packet->end );
}
/* 入力は実座標系なので注意！ */
static void SetScreenDrawArea( SCREEN_DRAW *screen_draw, int rgba )
{
	DG_SetDmapackSprt( &screen_draw->sprt,
					  0, 0,
					  DG_FRAME_U( 0 + 0.5f ), DG_FRAME_V( 0 + 0.5f ),
					  DRAW_WIDTH, DRAW_HEIGHT,
					  DG_FRAME_U( DRAW_WIDTH + 0.5f ), DG_FRAME_V( DRAW_HEIGHT + 0.5f ),
					  rgba );

}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i ;

	//if ( GV_PadData[0].status & PAD_Y ) return ;
	if ( work->time != -1 ){
		/* カウンタチェック */
		if ( work->time <= 0 ){
			GV_DestroyActor( work );
			return ;
		}
		if ( GV_PauseLevel == 0 ) work->time -= CLOCK_COUNT ;
		if ( work->time < 0 ) work->time = 0 ;
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	ClutAnimeChara = NULL ;
	//if ( work->tmp_clut_addr != NULL ) GV_DelayedFree( work->tmp_clut_addr );
	/* パケットメモリ開放 */
	if ( work->packet_mem != NULL ) GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int type, int time )
{
	int		i ;
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	int		dmapack_flag ;
	int		r, g, b, col ;

	work->time = time ;
	work->type = type ;

	if ( ClutAnimeChara != NULL ){
		GV_DestroyActor( ClutAnimeChara );
	}
	ClutAnimeChara = work ;

	/* ＤＭＡパケット型オブジェクト作成 */
	dmapack_flag = DG_DMAPACK_NORMAL | DG_DMAPACK_INVISIBLE1 | DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 ;
	work->dmapack = dmapack = DG_MakeDmapack2( dmapack_flag, DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO );
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	dmapack->autopacket = packet ;

	r = work->bias.vx ; if ( r > 255 ) r = 255 ;
	g = work->bias.vy ; if ( g > 255 ) g = 255 ;
	b = work->bias.vz ; if ( b > 255 ) b = 255 ;
	col = b | ( g << 8 ) | ( r << 16 );
	InitDmaPacket( packet, col );
	switch ( work->type ){
	  case 0:
		col = 0x80808080 ;
		break ;
	  case 1:
		r = work->scale.vx / 32 ; if ( r > 255 ) r = 255 ;
		g = work->scale.vy / 32 ; if ( g > 255 ) g = 255 ;
		b = work->scale.vz / 32 ; if ( b > 255 ) b = 255 ;
		col = r | ( g << 8 ) | ( b << 16 ) | 0x80000000 ;
		break ;
	  case 2:
		r = work->scale.vx ; if ( r > 255 ) r = 255 ;
		g = work->scale.vy ; if ( g > 255 ) g = 255 ;
		b = work->scale.vz ; if ( b > 255 ) b = 255 ;
		col = r | ( g << 8 ) | ( b << 16 ) | 0x80000000 ;
		break ;
	}
	SetScreenDrawArea( &packet->screen_draw, col );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewGrayClut( int time, u_int add_col )
{
	Work		*work ;
	int			clut_buffer_entry = 0 ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->bias.vx = add_col & 0xff ;
		work->bias.vy = ( add_col >> 8 ) & 0xff ;
		work->bias.vz = ( add_col >> 16 ) & 0xff ;
		work->bias.vw = ( add_col >> 24 ) & 0xff ;
		if ( GetResources( work, 0, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void *NewGrayClut2( int time, u_int add_col, u_int mod_col )
{
	Work		*work ;
	int			clut_buffer_entry = 0 ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->bias.vx = add_col & 0xff ;
		work->bias.vy = ( add_col >> 8 ) & 0xff ;
		work->bias.vz = ( add_col >> 16 ) & 0xff ;
		work->bias.vw = ( add_col >> 24 ) & 0xff ;
		work->scale.vx = mod_col & 0xff ;
		work->scale.vy = ( mod_col >> 8 ) & 0xff ;
		work->scale.vz = ( mod_col >> 16 ) & 0xff ;
		work->scale.vw = ( mod_col >> 24 ) & 0xff ;
		if ( GetResources( work, 2, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewContrastClut( int time, SVECTOR *scale, SVECTOR *bias )
{
	Work		*work ;
	int			clut_buffer_entry = 0 ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->scale.vx = scale->vx << 4 ;
		work->scale.vy = scale->vy << 4 ;
		work->scale.vz = scale->vz << 4 ;
		work->scale.vw = 0 ;
		work->bias.vx = -( 128 * scale->vx / 256 - 128 ) + bias->vx ;
		work->bias.vy = -( 128 * scale->vy / 256 - 128 ) + bias->vy ;
		work->bias.vz = -( 128 * scale->vz / 256 - 128 ) + bias->vz ;
		work->bias.vw = 0 ;
		if ( GetResources( work, 1, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#endif

