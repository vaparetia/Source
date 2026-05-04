//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	store.c
	イメージストア処理ユニット

	2000/02/01 K.Takabe
	$Id: store.c,v 1.1.1.3 2002/11/19 11:42:26 Yoshizawa1 Exp $

*/
/*

	void		DG_StoreChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	イメージをストアするＤＭＡパケットを接続する


	void DG_SetFrameStore( void *store_addr, int mode )
	void	*store_addr ;	データ書き出しアドレス
	int		mode ;			モード

	前のフレームの画像を指定したアドレスに１６ビットイメージとして書き出す

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
#include <libpc.h>

//BP_PS2 #include	"break.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"

/*----------------------------------------------------------------*/
static void		*ImageStoreAddr = NULL ;
static int		ImageStoreMode = 0 ;
static int		RetryStoreFlag = 0 ;
static void		*RetryStoreAddr = NULL ;
static int     BP_ImageStoreMode = 0;

/*----------------------------------------------------------------*/
typedef struct _draw_setup{
	DG_DMATAG	dmatag ;
	struct _draw_setup_data{
		DG_GIFTAG	giftag ;
		/* 描画環境設定 */
		DG_GSREG	frame ;
		DG_GSREG	zbuffer ;
		DG_GSREG	xyoffset ;
		DG_GSREG	scissor ;
		DG_GSREG	dthe ;
	} data ;
} DrawSetup ;
typedef struct _copy_image{
	DG_DMATAG	dmatag ;
	struct _copy_image_data{
		DG_GIFTAG	giftag ;
		/* 描画 */
		DG_GSREG	test ;
		DG_GSREG	texflush ;
		DG_GSREG	tex0 ;
		DG_GSREG	clamp ;
		DG_GSREG	prim ;
		DG_GSREG	rgba ;
		DG_GSREG	uv0 ;
		DG_GSREG	xyz0 ;
		DG_GSREG	uv1 ;
		DG_GSREG	xyz1 ;
		DG_GSREG	texflush2 ;
	} data ;
} CopyImage ;

typedef struct _store_image_packet{
	DrawSetup		draw_setup ;
	CopyImage		copy_image ;
	DG_STOREIMAGE	store_image ;
} StoreImagePacket ;
/*----------------------------------------------------------------*/
typedef struct _scrpad {
	u_long128	dma_buffer[ 128 ];
	u_long128	local_work[0] ;
} ScrpadWork ;

/*----------------------------------------------------------------*/
static void MakeDrawSetupPacket( DrawSetup *draw_setup, int which )
{
	/*  */
	draw_setup->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _draw_setup_data) );
	draw_setup->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	draw_setup->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _draw_setup_data), 0 );
	/*  */
	draw_setup->data.giftag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct _draw_setup_data)-1, 1, 0, 0, SCE_GIF_PACKED, 1 );
	draw_setup->data.giftag.regs = 0xe ;
	/*  */
	draw_setup->data.frame.reg = SCE_GS_FRAME_1 ;
	draw_setup->data.frame.data =
	  SCE_GS_SET_FRAME( TEXTURE_TOP_PAGE()/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT16, 0x00000000 ) ;
	draw_setup->data.zbuffer.reg = SCE_GS_ZBUF_1 ;
	draw_setup->data.zbuffer.data =
	  SCE_GS_SET_ZBUF( ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 0 ) ;
	draw_setup->data.xyoffset.reg = SCE_GS_XYOFFSET_1 ;
	draw_setup->data.xyoffset.data = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 ) << 4 ), ( ( 2048 - DRAW_HEIGHT / 2 ) << 4 ) ) ;
	draw_setup->data.scissor.reg = SCE_GS_SCISSOR_1 ;
	draw_setup->data.scissor.data =
	  SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH-1, 0, DRAW_HEIGHT-1 ) ;
	draw_setup->data.dthe.reg = SCE_GS_DTHE ;
	draw_setup->data.dthe.data = SCE_GS_SET_DTHE( 1 ) ;
}
static void MakeCopyImagePacket( CopyImage *copy_image, int which )
{
	/*  */
	copy_image->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _copy_image_data) );
	copy_image->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	copy_image->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _copy_image_data), 0 );
	/*  */
	copy_image->data.giftag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct _copy_image_data)-1, 1, 0, 0, SCE_GIF_PACKED, 1 );
	copy_image->data.giftag.regs = 0xe ;
	/*  */
	copy_image->data.test.reg = SCE_GS_TEST_1 ;
	copy_image->data.test.data = SCE_GS_SET_TEST( 0, 1, 64, 0, 0, 0, 1, 1 ) ;
	copy_image->data.texflush.reg = SCE_GS_TEXFLUSH ;
	copy_image->data.texflush.data = 0 ;
	copy_image->data.tex0.reg = SCE_GS_TEX0_1 ;
	copy_image->data.tex0.data =
	  SCE_GS_SET_TEX0( BUFFER_PAGE(which) / 64 ,
					  BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,0,0,0,0,0,0,0) ;
	copy_image->data.clamp.reg = SCE_GS_CLAMP_1 ;
	copy_image->data.clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) ;
	copy_image->data.prim.reg = SCE_GS_PRIM ;
	copy_image->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) ;
	copy_image->data.rgba.reg = SCE_GS_RGBAQ ;
	copy_image->data.rgba.data = 0x80808080 ;
	copy_image->data.uv0.reg = SCE_GS_UV ;
	copy_image->data.uv0.data = SCE_GS_SET_UV( 8, 8 ) ;
	copy_image->data.xyz0.reg = SCE_GS_XYZ2 ;
	copy_image->data.xyz0.data =
	  SCE_GS_SET_XYZ( ( 2048 - DRAW_WIDTH  / 2 ) * 16, ( 2048 - DRAW_HEIGHT / 2 ) * 16, DRAW_Z_MAX ) ;
	copy_image->data.uv1.reg = SCE_GS_UV ;
	copy_image->data.uv1.data = SCE_GS_SET_UV( 8+(DRAW_WIDTH)*16, 8+(DRAW_HEIGHT)*16 ) ;
	copy_image->data.xyz1.reg = SCE_GS_XYZ2 ;
	copy_image->data.xyz1.data =
	  SCE_GS_SET_XYZ( ( 2048 + DRAW_WIDTH  / 2 ) * 16, ( 2048 + DRAW_HEIGHT / 2 ) * 16, DRAW_Z_MAX ) ;
	copy_image->data.texflush2.reg = SCE_GS_TEXFLUSH ;
	copy_image->data.texflush2.data = 0 ;
}
/*----------------------------------------------------------------*/
void *DG_MakeStoreImagePacket( void *tag_addr,
							 int src_addr, int src_width, int src_fmt, int src_x, int src_y,
							 int width, int height, int dir )
{
	/* 使用例
		tag_mem = DG_MakeStoreImagePacket( tag_mem,
					BUFFER_PAGE(0), 512, SCE_GS_PSMCT32 , 0, 0,
					512, 256, 0 );
	*/
	DG_STOREIMAGE	*store_image ;

	store_image = (DG_STOREIMAGE*)tag_addr ;
	/* ＧＩＦ設定パケット転送用ＤＭＡタグ用意 */
	store_image->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_END, 6 ) ;
	store_image->dmatag.addr = NULL ;
	store_image->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	store_image->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( 6 , 0 ) ;
	/* ＧＩＦ設定パケット用ＧＩＦタグ用意 */
	store_image->giftag.tag = SCE_GIF_SET_TAG( 5, 1, 0, 0, SCE_GIF_PACKED, 1 );
	store_image->giftag.regs = 0x0000000e ;
	/* イメージ転送用設定 */
	store_image->bitbltbuf.reg = SCE_GS_BITBLTBUF ;
	store_image->bitbltbuf.data = SCE_GS_SET_BITBLTBUF( src_addr/64, src_width/64, src_fmt, 0, 0, 0 ) ;
	store_image->trxpos.reg = SCE_GS_TRXPOS ;
	store_image->trxpos.data = SCE_GS_SET_TRXPOS( src_x, src_y, 0, 0, dir ) ;
	store_image->trxreg.reg = SCE_GS_TRXREG ;
	store_image->trxreg.data = SCE_GS_SET_TRXREG( width, height ) ;
	store_image->finish.reg = SCE_GS_FINISH ;
	store_image->finish.data = 0 ;
	store_image->trxdir.reg = SCE_GS_TRXDIR ;
	store_image->trxdir.data = SCE_GS_SET_TRXDIR( 1 ) ;

	return ( &store_image[1] );

}
/*----------------------------------------------------------------*/
static int MakeStoreImagePacket( void *tag, int which )
{
	StoreImagePacket	*packet ;

	packet = (StoreImagePacket*)tag ;

	MakeDrawSetupPacket( &packet->draw_setup, which );

	MakeCopyImagePacket( &packet->copy_image, which );

	/* データ転送パケット生成 */
	if ( ImageStoreMode == 0 ){
		DG_MakeStoreImagePacket( &packet->store_image,
								TEXTURE_TOP_PAGE(), DRAW_WIDTH, SCE_GS_PSMCT16, 0, 0,
								DRAW_WIDTH, DRAW_HEIGHT, 0 );
	} else {
		DG_MakeStoreImagePacket( &packet->store_image,
								TEXTURE_TOP_PAGE(), 64, SCE_GS_PSMCT32, 0, 0,
								64, DRAW_WIDTH*DRAW_HEIGHT/64/2, 0 );
	}

	return ( SIZEOF_QWORD(StoreImagePacket) );
}

/*----------------------------------------------------------------*/

	/*
		イメージストア用ＤＭＡパケットを接続する
	*/
void DG_StoreChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR;
	int			size ;

	if ( RetryStoreFlag && RetryStoreAddr != NULL ){
		ImageStoreAddr = RetryStoreAddr ;
	}
	RetryStoreAddr = NULL ;
	RetryStoreFlag = 0 ;
	if ( ImageStoreAddr == NULL ) return ;

   // BP_Render
   if(BP_ImageStoreMode == 0)
   {
      /* ＤＭＡバッファオープン(VIF1) */
      DG_OpenDmaTask( DG_OPEN_DMA_STORE, ImageStoreAddr, DRAW_WIDTH*DRAW_HEIGHT*2/16 );
      FlushCache( 0 );
      {/* パケット生成 */
         DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );
         {
            size = MakeStoreImagePacket( scrpad->dma_buffer, which );
            DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
         }
         DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );
      }
      /* ＤＭＡバッファクローズ */
      DG_CloseDmaTask();
   }
   else
   {
      SBP_FrameStorePacket* pPacket = (SBP_FrameStorePacket*)BP_RB_Alloc(sizeof(SBP_FrameStorePacket));
      pPacket->bpTexture = ImageStoreAddr; // This is really a BP_Texture*
      pPacket->x0 = 0;
      pPacket->y0 = 0;
      pPacket->x1 = DRAW_WIDTH;
      pPacket->y1 = DRAW_HEIGHT;
      BP_RB_AddCommand(kCmd_FrameStore_End, (char*)pPacket);
      BP_ImageStoreMode = 0;
   }

	RetryStoreAddr = ImageStoreAddr ;
	ImageStoreAddr = NULL ;

}
/*----------------------------------------------------------------*/
	/*
		イメージストアをリトライする
	*/
void DG_RetryStoreChanl( void )
{
	RetryStoreFlag = 1 ;
}
/*----------------------------------------------------------------*/
	/*
		前のフレームの画像を指定したアドレスに１６ビットイメージとして書き出す
	*/
void DG_SetFrameStore( void *store_addr, int mode )
{
	ImageStoreAddr = store_addr ;
	ImageStoreMode = mode ;

   BP_ImageStoreMode = 0;
}

void DG_SetFrameStore_SafeForBPUse( void *store_addr, int mode )
{
   ImageStoreAddr = store_addr ;
   ImageStoreMode = mode ;

   BP_ImageStoreMode = 1;
}
