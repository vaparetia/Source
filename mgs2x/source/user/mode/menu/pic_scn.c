//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pic_scn.c
	画像イメージ表示補助ルーチン

	2001/05/28	K.Takabe
	$Id: pic_scn.c,v 1.1.1.3 2002/11/19 11:45:13 Yoshizawa1 Exp $
*/
/*

	void *NewPictureDrawManager( int prio );
	int		prio ;		描画プライオリティ指定

	このマネージャーを起動した上で下の関数を呼び出すとイメージを描画することが出来る。


	void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
	int		x ;			表示Ｘ座標（仮想座標）
	int		y ;			表示Ｙ座標（仮想座標）
	int		w ;			表示幅（仮想座標）
	int		h ;			表示高さ（仮想座標）
	int		color ;		スプライトカラー
	void	*image ;	１６ビットイメージデータへのポインタ（qword境界から始まること）
	int		image_w ;	イメージ幅
	int		image_h ;	イメージ高さ
	int		flag ;		フラグ（現在未使用）

		指定したベタイメージを画面に表示する（１フレーム中で８枚まで）
		イメージのサイズは最大２５６ＫＢ以下であること
		また、実際に画面に表示されるのは実際のイメージの0.5ドット内側なので注意すること


	void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );

		３２ビットカラー版

*/

#if 0 //BP_Render - Using xpic_scrn.c version

/* ---------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#include	"gameheader.h"
#include	"def_dma.h"
#include	"libdg.cnf"
#include	"dmapack.h"
#include	"menu.h"

/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* 符号なし８ビットから符号なし１６ビットへ拡張 */
#define PUBEXT(_d) ({ u_long128 __tmp ; asm ("pextlb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビットから８ビットへパッキング */
#define PUBPAC(_d) ({ u_long128 __tmp ; asm ("ppacb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* ３２ビットから１６ビットへパッキング */
#define PUHPAC(_d) ({ u_long128 __tmp ; asm ("ppacw %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビット乗算（符号付き） */
#define PHMUL(_a,_b) ({ u_long128 __tmp ; asm ("pmulth $0,%1,%2;pmfhl.sh %0":"=r"(__tmp):"r"(_a),"r"(_b) ); __tmp ;})

/* ---------------------------------------------------------------- */
/* 単一のテクスチャ設定付きスプライト描画プリミティブ */
typedef struct {
	DG_DMATAG		dmatag_sprt ;	/* スプライト本体転送タグ */
	DG_GIFTAG		sprt_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_SPRT	sprt ;			/* スプライトプリミティブ */
} PRIM_SPRITE_L ;

typedef struct {
	DG_DMATAG		dmatag ;		/* 本体転送タグ */
	DG_GIFTAG		giftag ;		/* ＧＳ設定用GIFタグ */
	struct {
		DG_GSREG	texflush ;		/* テクスチャキャッシュのフラッシュ */
		DG_GSREG	tex2 ;
		DG_GSREG	tex0 ;
		DG_GSREG	clamp ;
		DG_GSREG	alpha ;
	} data ;
} PRIM_SETUP ;


typedef struct {
	DG_LOADIMAGE	loadimage ;
	PRIM_SETUP		setup ;
	PRIM_SPRITE_L	sprt ;
} PRIM_DRAWPICTURE ;



/* プリミティブ表示コントロール構造体 */
typedef struct {
	DG_DMAPACK		*dmapack ;
	int				tri_id ;			/* 使用TRI */
	DG_TEXTURE_LIST	*tex_list ;			/* TRIデータポインタ */
	int				current_offset ;	/* 書き込みデータ最終位置オフセット（1qword単位） */
	int				end_offset ;		/* 書き込みデータ限界位置オフセット（1qword単位） */
	u_long128		*buffer[2] ;		/* 転送バッファ */
	u_long128		*current_buffer ;	/* 最終書き込み位置アドレス */
} PRIM_CONTROL ;


typedef struct {
	GV_ACT_EX	actor ;
	PRIM_CONTROL	prim_ctrl ;	/* パネル表示用 */
} Work ;

static Work	*work_ptr = NULL ;

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	///* テクスチャパケットの取得 */
	//prim_ctrl->tri_id = tri_id ;
	//prim_ctrl->tex_list = DG_GetTextureList( tri_id );
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
	/* その他初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->end_offset = buffer_size ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
}

/* プリミティブコントロールの開放 */
static void FreePrimControl( PRIM_CONTROL *prim_ctrl )
{
	DG_DequeueDmapack( prim_ctrl->dmapack );
	DG_FreeDmapack( prim_ctrl->dmapack );
	GV_DelayedFree( prim_ctrl->buffer[0] );
}

/* 終端タグの書き込み */
static void ClosePrimControl( PRIM_CONTROL *prim_ctrl )
{
	const DG_DMATAG	endtag = { DMATAG_SET_QWC( DMATAG_ID_RET, 0 ), NULL,
								 {SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ) } } ;
	DG_DMATAG	*tag ;
	tag = (DG_DMATAG*)prim_ctrl->current_buffer ;
	//tag = GV_UNCACHEA( tag );
	*tag = endtag ;
}

/* プリミティブコントロールの初期化 */
static void InitPrimControl( PRIM_CONTROL *prim_ctrl )
{
	//int		size ;

	if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
		printf("menu_sub.c: prim buffer over error!!!\n");
	}
	/* 書き込み位置の初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ DG_Clock ] ;
	///* テクスチャ転送パケットの設定 */
	//size = DG_WriteTextureChangePacks( prim_ctrl->current_buffer,
	//								  (void*)&prim_ctrl->tex_list->tex_packet[ DG_Clock ] );
	//prim_ctrl->current_offset += size ;
	//prim_ctrl->current_buffer += size ;
	/* 終端タグの書き込み */
	ClosePrimControl( prim_ctrl );
}

/* スプライト描画プリミティブの設定 */
/* 初期設定 */
static void SetSprite( PRIM_SPRITE_L *sprt )
{
	/* DMAタグ＆GIFタグの設定 */
	sprt->dmatag_sprt.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) );
	sprt->dmatag_sprt.vifcode[0] = SCE_VIF1_SET_NOP(0);
	sprt->dmatag_sprt.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	sprt->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&sprt->sprt_tag |= 1 ;	/* GIFパケット転送データを１に設定 */
	sprt->sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
}
///* テクスチャ＆ＵＶ設定 */
//static void SetSpriteTex( PRIM_SPRITE_L *sprt, DG_TEX *tex )
//{
//	/* テクスチャ設定 */
//	sprt->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
//	sprt->dmatag_tex.addr = &tex->tex_trans ;
//	sprt->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP(0);
//	sprt->dmatag_tex.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_TEX_TRANS) ,0 );
//}
/* 座標設定 */
static void SetSpritePos( DG_MENU_SPRT *sprt, int x1, int y1, int x2, int y2 )
{
#ifdef BP_PSX2_ASM
	static FVECTOR	scale = { (float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT, 
								(float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT };
	static FVECTOR	offset = { 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2,
								 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2 };
	IVECTOR			tmp ;
	tmp.vx = x1 ;
	tmp.vy = y1 ;
	tmp.vz = x2 ;
	tmp.vw = y2 ;
	asm volatile("
		lqc2			vf1,0x00(%0)	# 
		lqc2			vf2,0x00(%1)	# 
		lqc2			vf3,0x00(%2)	# 
		vitof0.xyzw		vf1,vf1			# 整数から浮動小数点へ
		vmula.xyzw		ACC,vf2,vf1		# スケール乗算
		vmaddw.xyzw		vf1,vf3,vf0		# オフセット加算
		vftoi4.xyzw		vf1,vf1			# 浮動小数点から整数へ
		sqc2			vf1,0x00(%0)	# 
	"::"r"(&tmp),"r"(&scale),"r"(&offset):"memory" );
	DG_SET_XY1( sprt, tmp.vx, tmp.vy );
	DG_SET_XY2( sprt, tmp.vz, tmp.vw );
#else
	x1 = ( (float)DRAW_WIDTH/VR_WIDTH * x1 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y1 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y1 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	x2 = ( (float)DRAW_WIDTH/VR_WIDTH * x2 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y2 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y2 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	DG_SET_XY1( sprt, x1, y1 );
	DG_SET_XY2( sprt, x2, y2 );
#endif
}
/* ＵＶ設定 */
static void SetSpriteUv( DG_MENU_SPRT *sprt, int u1, int v1, int u2, int v2 )
{
	/* UV設定 */
	DG_SET_UV1( sprt, u1, v1 );
	DG_SET_UV2( sprt, u2, v2 );
}
/* 色設定 */
inline static void SetSpriteCol( DG_MENU_SPRT *sprt, int col )
{
	//DG_SET_RGBA1( sprt, r, g, b, a );
	*(int*)&sprt->rgba1.r = col ;
}

/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/*  */
void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	PRIM_DRAWPICTURE	*draw_pic ;
	PRIM_SETUP			*setup ;
	PRIM_SPRITE_L		*sprt ;
	//DG_TEX				*tex ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;
	draw_pic = (PRIM_DRAWPICTURE*)prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += SIZEOF_QWORD(PRIM_DRAWPICTURE) ;
	prim_ctrl->current_offset += SIZEOF_QWORD(PRIM_DRAWPICTURE) ;
	/* 先に終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );

	/* ロードイメージ設定 */
	DG_MakeLoadImagePacket( &draw_pic->loadimage, SCE_GS_PSMCT16, image_w, image_h,
						   TEXTURE_TOP_PAGE(), BUFFER_WIDTH, image );

	/* セットアップパケット設定 */
	setup = &draw_pic->setup ;
	setup->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PRIM_SETUP) - 1 );
	setup->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	setup->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PRIM_SETUP) - 1, 0 );
	setup->giftag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(setup->data), 0, 0, 0, SCE_GIF_PACKED, 1 );
	setup->giftag.regs = 0x0000000e ;
	setup->data.texflush.reg = SCE_GS_TEXFLUSH ;
	setup->data.tex2 = DG_SystemTexture[3].tex_trans.tex2 ;
	setup->data.tex0 = DG_SystemTexture[3].tex_trans.tex0 ;
	setup->data.clamp = DG_SystemTexture[3].tex_trans.clamp ;
	setup->data.alpha = DG_SystemTexture[3].tex_trans.alpha ;

	/* 背景ウィンドウスプライトの設定 */
	sprt = &draw_pic->sprt ;
	SetSprite( sprt );														/* パケット初期化 */
	//SetSpriteTex( sprt, &DG_SystemTexture[3] );								/* テクスチャ設定 */
	image_w <<= 4 ;
	image_h <<= 4 ;
	SetSpriteUv( &sprt->sprt, 0 + 8, 0 + 8, image_w - 8, image_h - 8 );		/* 0.5ドット内側に設定 */
	SetSpritePos( &sprt->sprt, x, y, x + w, y + h );						/* 座標設定（仮想座標） */
	SetSpriteCol( &sprt->sprt, color );										/* 色設定 */
}

/*  */
void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	PRIM_DRAWPICTURE	*draw_pic ;
	PRIM_SETUP			*setup ;
	PRIM_SPRITE_L		*sprt ;
	//DG_TEX				*tex ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;
	draw_pic = (PRIM_DRAWPICTURE*)prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += SIZEOF_QWORD(PRIM_DRAWPICTURE) ;
	prim_ctrl->current_offset += SIZEOF_QWORD(PRIM_DRAWPICTURE) ;
	/* 先に終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );

	/* ロードイメージ設定 */
	DG_MakeLoadImagePacket( &draw_pic->loadimage, SCE_GS_PSMCT32, image_w, image_h,
						   TEXTURE_TOP_PAGE(), BUFFER_WIDTH, image );

	/* セットアップパケット設定 */
	setup = &draw_pic->setup ;
	setup->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PRIM_SETUP) - 1 );
	setup->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	setup->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PRIM_SETUP) - 1, 0 );
	setup->giftag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(setup->data), 0, 0, 0, SCE_GIF_PACKED, 1 );
	setup->giftag.regs = 0x0000000e ;
	setup->data.texflush.reg = SCE_GS_TEXFLUSH ;
	setup->data.tex2 = DG_SystemTexture[2].tex_trans.tex2 ;
	setup->data.tex0 = DG_SystemTexture[2].tex_trans.tex0 ;
	setup->data.clamp = DG_SystemTexture[2].tex_trans.clamp ;
	setup->data.alpha = DG_SystemTexture[2].tex_trans.alpha ;

	/* 背景ウィンドウスプライトの設定 */
	sprt = &draw_pic->sprt ;
	SetSprite( sprt );														/* パケット初期化 */
	//SetSpriteTex( sprt, &DG_SystemTexture[3] );								/* テクスチャ設定 */
	image_w <<= 4 ;
	image_h <<= 4 ;
	SetSpriteUv( &sprt->sprt, 0 + 8, 0 + 8, image_w - 8, image_h - 8 );		/* 0.5ドット内側に設定 */
	SetSpritePos( &sprt->sprt, x, y, x + w, y + h );						/* 座標設定（仮想座標） */
	SetSpriteCol( &sprt->sprt, color );										/* 色設定 */
}

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

static void Act( Work *work )
{
	/* 書き込み位置の初期化のみ行う */
	InitPrimControl( &work->prim_ctrl );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	FreePrimControl( &work->prim_ctrl );
	work_ptr = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int prio )
{
	/* プリミティブメモリ確保 */
	MakePrimControl( &work->prim_ctrl,
					SIZEOF_QWORD(PRIM_DRAWPICTURE) * 8 + 32,
					prio, 0 );
	Act( work );


	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		アイテムメニュー用プリミティブ管理デーモン
	*/
void *NewPictureDrawManager( int prio )
{
	Work			*work ;

	if ( work_ptr != NULL ) return (NULL);

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, prio ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work_ptr = work ;
	return work ;
}


#if 0
/* 動作テスト用サンプルルーチン */
	{
		extern void MENU_DrawPicture16( int x, int y, int w, int h, int color
									 , void *image, int image_w, int image_h, int flag );

		static u_long128		dummy[ 32 * 32 * 2 / 16 ];
		int		i ;
		u_short	*data ;
		data = (u_short*)dummy ;
		for ( i = 0 ; i < 32*32 ; i++, data++ ){
			*data = 0xfc00 | i ;
		}
		MENU_DrawPicture16( 256, 256, 96, 96, 0x80808080, dummy, 32, 32, 0 );
	}
#endif

#endif // BP_Render - Using xpic_scrn.c version

