//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  	vr_clear_eft.c
	VRクリア画面エフェクト
	2002/05/31 Y.Yano
	
	$Id: vr_clear_eft2.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
*/

/* TODO:::::
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

/**************************************************************************************************/
#include 	"../include/dmatags.h"
#include 	"../../shibata/util/dma_set.h"

#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_RenderBufferTypes.h"


//#define TYPE1
///////////////////////////////////////////////
#ifdef TYPE1
#define RUST_NUM 84 /* 128 */
#define RUST_MAX_ALPHA 20
#else
#define RUST_NUM 128 /* 128 */
#define RUST_MAX_ALPHA 10
#endif

extern char _program_top[];
#ifdef PSX2
#define SAND_ADDR  ((void *)(_program_top + 0x001140))  /* 適当に汚れているメモリを指定 */
#endif
#ifdef KP_XBOX
#define SAND_ADDR  ((void *)( (int)(_program_top + 0x00008000) & 0x0fffff00 )) /* 128バイト境界 */
#endif

#define NOISE2_TEX_WIDTH 	384
#define NOISE2_TEX_HEIGHT 	384
#define NOISE2_WIDTH	256
#define NOISE2_HEIGHT	256
#define NOISE2_RANDU	(NOISE2_TEX_WIDTH-NOISE2_WIDTH)
#define NOISE2_RANDV	(NOISE2_TEX_HEIGHT-NOISE2_HEIGHT)

/////////////////////////


#define	BG_CLAMP		(SCE_GS_SET_CLAMP( 2, 2, 0, DRAW_WIDTH, 0, DRAW_HEIGHT ))
#ifdef PAL
#define CODEC_MEM_SIZE  (1024 * 1024 * 3 + 256*1024)
#else
#define CODEC_MEM_SIZE  (1024 * 1024 * 3)
#endif

//extern void *NewGrayClut( int time, u_int add_col );
extern void	*NewDgCamShuterSight( void );
extern void *DG_ResizePacketMemory( int size );



#ifdef PSX2
typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[1];
} SPRT_DATA_BG;
typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[1];
} SPRT_DATA_BROWN;
typedef struct {
	/*
	  待機バッファをテクスチャとして使う SPRITE prim
	*/
	SET_DRAW_PRIM		bg_set_prim;			// 描画の設定
	SPRT_DATA_BG		bg_sprt_data;			// TriangleStrip設定
	/*
	  茶色 SPRITE prim
	 */
	SET_DRAW_PRIM		brown_set_prim;			// 描画の設定
	SPRT_DATA_BROWN		brown_sprt_data;		// スプライト設定
} CLREFT_PRIM;
typedef struct {
	DG_LOADIMAGE		bg_image;				// グレースケールノイズデータをテクスチャに使用するため
	DG_DMATAG			prim_dmatag;				
	CLREFT_PRIM			prim_data;				// GIFtagでつないだ prim 一連
	DG_DMATAG			recov_dmatag;			// 描画環境復元
	DG_DMATAG			end_dmatag;				// END dmatag( IDがRET )	
} CLREFT_PACK;
#endif //PSX2
#ifdef KP_XBOX
typedef struct {
	DG_DMAPACK_TEX		tex; //DG_SetDmapackTexLin();
	DG_DMAPACK_ALPHA	alp; //DG_SetDmapackAlpha();
	DG_DMAPACK_SPRT		spr; //DG_SetDmapackSprt();
	DG_DMAPACK_TAG		end; //DG_SetDmapackEnd();
} CLREFT_PACK_XBOX;
#endif //KP_XBOX


/*
 * Work
 */
typedef struct _Work{
	GV_ACT_EX			actor ;

	DG_DMAPACK			*dmapack;
#ifdef PSX2
	CLREFT_PACK			*draw_packet;
	int 				clut[16*16];
	sceGsStoreImage 	sp;
#ifdef PAL
	sceGsStoreImage 	sp2;
#endif
#endif //PSX2
#ifdef KP_XBOX
	CLREFT_PACK_XBOX	*draw_packet;
	void 				*prim_x;
	DG_TEX_LIN 			*lintex;
	int 				alp_x;
#endif //KP_XBOX
	int 				*yamashita_flag;
	int 				last_yamashita_flag;



////////////////////////////////
	float step_pos[RUST_NUM];
	int   rust_alp[RUST_NUM];
	int flag;
	int rust_cnt;
	short *scrn;
	int 	handle_2d;
	SPR_OBJ *root;

	float black_height;
	int phase;
	int cnt;

	int n_cnt;
	int s_cnt;
	int c_cnt;
	int l_cnt;

	float u_pos[2];
	int p_m[2];

	float f_alpha;

	////デバック用変数
	  int rrr, ggg, bbb, aaa;
#if 1 // BP_RENDER
  float grayScalePercent;
#endif
} Work;

/* ---------------------------------------------------------------- */
/* グレースケール化:
   16階調のグレースケールにする。これにより1pixel を4bit で扱うことができる。
*/
static int scrn_grayscale(Work * work, unsigned char * tmp)
{
  unsigned char * src;
  int x, y;
  int even, max;
  int rate;
  int w;

  /*
   * 16bpp (RGBA16) の場合
   */
  unsigned short *dst;

  dst = (unsigned short *)work->scrn; /* 16階調グレースケールが格納されるバッファのポインタ */
  src = tmp;        /* オーバーライドで縮小された画像のポインタ           */

  max = 0;
  rate = 16000;
  w = DRAW_WIDTH;

  for(y = 0; y < DRAW_HEIGHT/2; y++)
    for(x = 0; x < w/2; x++){
		/* 偶数ピクセルの処理 */
		even  = *(src++) * 299;
		even += *(src++) * 587;
		even += *(src++) * 114;
		src++;
		even /= rate;
		even *= 0x0421;   /* 結果をRGBA16 形式にまとめる */
#ifdef KP_XBOX 
		/* tex alpha=128と同意にするため最上位ビットに1を入れておく */
		even |= (1 << 15);
#endif
		*dst++ = even;
	}
  

  return max;
}

static void scrn_shrink(Work * work, void * tmp)
{
	unsigned char *src, *dst;
	int x, y;
	src = tmp;
	dst = tmp; /* 縮小画像で、テンポラリバッファをオーバーライドする */
	
	/* 4ピクセルを1ピクセルにブレンディング */
	for(y = 0; y < DRAW_HEIGHT; y++ ){
		for(x = 0; x < DRAW_WIDTH; x++){
			if( (x%2)==0 && (y%2)==0 ){
				int r = 0, g = 0, b = 0;
				r += src[x*4 + y*DRAW_WIDTH*4];
				g += src[x*4 + y*DRAW_WIDTH*4 + 1];
				b += src[x*4 + y*DRAW_WIDTH*4 + 2];
				
				r += src[(x+1)*4 + y*DRAW_WIDTH*4];
				g += src[(x+1)*4 + y*DRAW_WIDTH*4 + 1];
				b += src[(x+1)*4 + y*DRAW_WIDTH*4 + 2];
				
				r += src[x*4 + (y+1)*DRAW_WIDTH*4];
				g += src[x*4 + (y+1)*DRAW_WIDTH*4 + 1];
				b += src[x*4 + (y+1)*DRAW_WIDTH*4 + 2];
				
				r += src[(x+1)*4 + (y+1)*DRAW_WIDTH*4];
				g += src[(x+1)*4 + (y+1)*DRAW_WIDTH*4 + 1];
				b += src[(x+1)*4 + (y+1)*DRAW_WIDTH*4 + 2];
				
				dst[0] = r / 4;
				dst[1] = g / 4;
				dst[2] = b / 4;
				dst += 4;
			}
		}
	}
	
}

enum{
	FLAG_WAIT = 0,
	FLAG_L2DSTART,
	FLAG_L2DPLAY,
	FLAG_SHUTSTART,
	FLAG_SHUTPLAY,
	FLAG_MONOSTART,
	FLAG_BG_ON,
};

static void Act( Work *work )
{
	int stat;
	int vr_clr_eft_flag = *(work->yamashita_flag);

#ifdef KP_XBOX
	work->prim_x = work->dmapack->autopacket;
#endif

	if( vr_clr_eft_flag != work->last_yamashita_flag ){
		switch( vr_clr_eft_flag ){
		  case 0:
			/* 発動前 */
			work->flag = FLAG_WAIT;
			break;
		  case 1:
			/* l2dスタート */
			work->flag = FLAG_L2DSTART;
			break;
		  case 2:
			/* カッシャン */
			work->flag = FLAG_SHUTSTART;
			break;
		  case 3:
			/* 減色開始 */
			work->flag = FLAG_MONOSTART;
			break;
		  default:
			printf("yamashita_flag に不正な値が入っています\n");
			work->flag = FLAG_WAIT;
		}
	}
	work->last_yamashita_flag = vr_clr_eft_flag;


	switch( work->flag ){
	  case FLAG_WAIT:
		/* 待機状態 */
		SPR_HIDE( work->root );
		break;
	  case FLAG_L2DSTART:
		/* フォーカスL2D再生開始 */
		stat = L2D_ActionStatus( work->handle_2d );
		if( stat == L2D_STAT_ACK ){
			printf("フォーカスL2D再生\n");
			L2D_EvokeAction( work->handle_2d, 13328255/* Action-1 */ );
			//SPR_SHOW( work->root );
			work->flag = FLAG_L2DPLAY;
		} else {
			printf("フォーカスL2D再生失敗\n");
			work->flag = FLAG_WAIT;
		}
		break;
	  case FLAG_L2DPLAY:
		/* フォーカスL2D再生中 */
		SPR_SHOW( work->root );
		break;
	  case FLAG_SHUTSTART:
		/* カッシャン開始 */
		printf("カシャ\n");		
		GV_SetActorChild( work , NewDgCamShuterSight() );
		//GM_SeSet( 0x00, GM_MAX_VOL, SD_S_R_CURSOR ) ;	
		GM_SeSet3D( 0x00, GM_MAX_VOL, SD_A_V_GOALEF, -M_PI/2 ) ;	
		SPR_HIDE( work->root );
		work->rust_cnt = 0;
		work->flag = FLAG_SHUTPLAY;
		break;
	  case FLAG_SHUTPLAY:
		/* カッシャンした後タイムカウント */
		work->rust_cnt ++;
		if( work->rust_cnt > 15 ){
			work->flag = FLAG_MONOSTART;
		}
		break;
	  case FLAG_MONOSTART:


		/* モノクロ化開始 */
		{
			char *codec_area;
			char *addr;

         codec_area = DG_ResizePacketMemory(CODEC_MEM_SIZE);
			
			if ( codec_area == NULL )
         {
				ASSERT( 0 );
				return ;
			}
			
#if 0 //BP_RENDER - custom callback function

#ifdef PSX2
#ifndef PAL
			/* 画面を取得 */
			{    
				int flip;
			
				flip = (DG_LastWhich < 0) ? DG_Clock : ( 1 - DG_LastWhich );
				sceGsSetDefStoreImage(&work->sp,
									  BUFFER_PAGE(flip), DRAW_WIDTH / 64,
									  SCE_GS_PSMCT32, 
									  0, 0, DRAW_WIDTH, DRAW_HEIGHT);
			}
			
			FlushCache(0);
			
			{
				int stat = 0;
				while(!stat) stat = DG_DmaCheckEnd();
			}
			
			sceGsSyncPath(0, 0);
			sceGsExecStoreImage(&work->sp, codec_area);
#else
			/* PAL版だとデータが大きすぎて(NTSC:512*448 -> PAL:512*512)
			   1回では送りきれないので2回に分ける */
			/* 画面を取得 */
			{    
				int flip;
			
				flip = (DG_LastWhich < 0) ? DG_Clock : ( 1 - DG_LastWhich );
				sceGsSetDefStoreImage(&work->sp,
									  BUFFER_PAGE(flip), DRAW_WIDTH / 64,
									  SCE_GS_PSMCT32, 
									  0, 0, DRAW_WIDTH, DRAW_HEIGHT/2 );
			}
			
			FlushCache(0);
			
			{
				int stat = 0;
				while(!stat) stat = DG_DmaCheckEnd();
			}
			
			sceGsSyncPath(0, 0);
			sceGsExecStoreImage(&work->sp, codec_area);

			{    
				int flip;
			
				flip = (DG_LastWhich < 0) ? DG_Clock : ( 1 - DG_LastWhich );
				sceGsSetDefStoreImage(&work->sp2,
									  BUFFER_PAGE(flip), DRAW_WIDTH / 64,
									  SCE_GS_PSMCT32, 
									  0, DRAW_HEIGHT/2, DRAW_WIDTH, DRAW_HEIGHT/2 );
			}
			
			FlushCache(0);
			
			{
				int stat = 0;
				while(!stat) stat = DG_DmaCheckEnd();
			}
			
			sceGsSyncPath(0, 0);
			sceGsExecStoreImage(&work->sp2, (codec_area + DRAW_WIDTH*DRAW_HEIGHT/2*4 ));

#endif			

#endif //BP_RENDER

			/* 16bitキャプチャー画面用メモリ確保 */
			work->scrn = GV_Malloc( DRAW_WIDTH*DRAW_HEIGHT/2 );
			if( work->scrn == NULL ){
				//ASSERT( 0 );
				work->flag = FLAG_WAIT;
				printf("vr_clr_eft メモリが足りませんでした\n");
				return ;
			}
			/* キャプチャー画面を1/4に縮小する */
			scrn_shrink( work, codec_area );
			/*
			 * 16階調のグレースケールに変換する
			 */
			scrn_grayscale( work, codec_area );

			addr = &work->draw_packet[DG_Clock].bg_image;
			addr = DG_MakeLoadImagePacket( addr, /* 背景データ転送 */
										  SCE_GS_PSMCT16, DRAW_WIDTH/2, DRAW_HEIGHT/2,
										  TEXTURE_TOP_PAGE(), DRAW_WIDTH, work->scrn );
			addr = &work->draw_packet[1-DG_Clock].bg_image;
			addr = DG_MakeLoadImagePacket( addr, /* 背景データ転送 */
										  SCE_GS_PSMCT16, DRAW_WIDTH/2, DRAW_HEIGHT/2,
										  TEXTURE_TOP_PAGE(), DRAW_WIDTH, work->scrn );

#endif //PSX2
#ifdef KP_XBOX
			/* 画面を取得 */
#ifndef KP_WINDOWS
			DG_StoreImage( codec_area , 1 /* 表示中バッファ */, 
						  0, 16	/*フレームパッファは480。真中448をとる為*/, 
						  DRAW_WIDTH, DRAW_HEIGHT,
						  0		/* 32bit */ );
#else
			DG_StoreImage( codec_area , 1 /* 表示中バッファ */, 
						  (DISPLAY_WIDTH-DRAW_WIDTH) >> 1,
						  (DISPLAY_HEIGHT-DRAW_HEIGHT) >> 1, /*フレームパッファ真中をとる為*/
						  DRAW_WIDTH, DRAW_HEIGHT,
						  0		/* 32bit */ );

#endif
			/* 16bitキャプチャー画面用メモリ確保 */
			work->scrn = GV_Malloc( DRAW_WIDTH*DRAW_HEIGHT/2 );/* 画像サイズは1/4に縮められるのでこのサイズ */
			if( work->scrn == NULL ){
				//ASSERT( 0 );
				work->flag = FLAG_WAIT;
				printf("vr_clr_eft メモリが足りませんでした\n");
				return ;
			}

			scrn_shrink( work, codec_area );
			scrn_grayscale( work, codec_area );
			/* 線形テクスチャにデータを入れ込む */
			memcpy( work->lintex->image, work->scrn, DRAW_WIDTH*DRAW_HEIGHT/2 /* (/4*2) */ );
			/* 減色全画面スプライトを描く */
			work->prim_x = DG_SetDmapackTexLin( work->prim_x, work->lintex );
			work->prim_x = DG_SetDmapackAlpha( work->prim_x, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
			work->prim_x = DG_SetDmapackSprt( work->prim_x, 0.0f, 0.0f, /* x0,y0 */
											  0.5f/(DRAW_WIDTH/2), 0.5f/(DRAW_HEIGHT/2), /* u0,v0 */
											  (float)DRAW_WIDTH, (float)DRAW_HEIGHT,/* x1,y1 */ 
											  (DRAW_WIDTH/2-0.5f)/(DRAW_WIDTH/2), 
											  (DRAW_HEIGHT/2-0.5f)/(DRAW_HEIGHT/2),/* u1,v1 */
											  0x00808080 /* col */ );
			
#endif //KP_XBOX

			DG_ResizePacketMemory(0);
		}

		work->flag = FLAG_BG_ON;
		break;
	  case FLAG_BG_ON:
#if 1 // BP_RENDER
        {
           work->grayScalePercent += 10/128.0f;
           if( work->grayScalePercent > 1.0f )
           {
               work->grayScalePercent = 1.0f;
           }
        }
#endif
#ifdef PSX2
		{
			/* 背景 */
			int alp;
			alp = work->draw_packet[1-DG_Clock].prim_data.bg_sprt_data.sprts[0].rgba.a;
			alp += 10;
			if( alp > 128 ){
				alp = 128;
			}
			work->draw_packet[DG_Clock].prim_data.bg_sprt_data.sprts[0].rgba.a = alp;
		}
#endif //PSX2
#ifdef KP_XBOX
		{
			int alp, col;			
			alp = work->alp_x;
			alp += 10;
			if( alp > 128 ){
				alp = 128;
				/* 0チャンネルを非表示(描画ひきつりを回避) */
				DG_Chanls[0].flag = 0; 
			}
			work->alp_x = alp;
			col = 0x00808080 | ( alp << 24 );
			/* 減色全画面スプライトを描く */
			work->prim_x = DG_SetDmapackTexLin( work->prim_x, work->lintex );
			work->prim_x = DG_SetDmapackAlpha( work->prim_x, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
			work->prim_x = DG_SetDmapackSprt( work->prim_x, 0.0f, 0.0f, /* x0,y0 */
											  0.5f/(DRAW_WIDTH/2), 0.5f/(DRAW_HEIGHT/2), /* u0,v0 */
											  (float)DRAW_WIDTH, (float)DRAW_HEIGHT,/* x1,y1 */ 
											  (DRAW_WIDTH/2-0.5f)/(DRAW_WIDTH/2), 
											  (DRAW_HEIGHT/2-0.5f)/(DRAW_HEIGHT/2),/* u1,v1 */
											  col );
		}
#endif
		break;
	  default:
		printf("hen-yo(vr_clr_eft2.c)\n");
		work->flag = FLAG_WAIT;
	}

	work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
		
#ifdef KP_XBOX
	DG_SetDmapackEnd( work->prim_x );
#endif

	return;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	if( work->draw_packet ) {
		printf("!!!\n");
		GV_Free( work->draw_packet );
	}

	if( work->scrn ){
		GV_Free( work->scrn );
	}
#ifdef KP_XBOX
	if( work->lintex ){
		DG_FreeLinerTexture( work->lintex );
	}
#endif

	/* 0チャンネルを表示、復帰 */
	DG_Chanls[0].flag = 1; 

}
/* ---------------------------------------------------------------- */

#ifdef PSX2 /* PS2専用関数 */
static void *InitBgSprtData( void *addr, int n_verts )
{
	SPRT_DATA_BG	*sprt_data = (SPRT_DATA_BG*)addr;
	_VERT_SPRT_UV	*verts = sprt_data->sprts;

	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 0/* 後続primあり */, 0, 0, 0, SIZEOF_QWORD(_VERT_SPRT_UV));
	sprt_data->giftag.regs = GIFREG_VERT_SPRT_UV;

	verts->rgba.r = 128;
	verts->rgba.g = 128;
	verts->rgba.b = 128;
	verts->rgba.a = 0 ;
#define SIRO_X 0.0f
#define SIRO_Y 0.0f
	verts->uv0.u = _FTOI4(0.5f + SIRO_X);
	verts->uv0.v = _FTOI4(0.5f + SIRO_Y);
	verts->uv1.u = _FTOI4(DRAW_WIDTH/2-0.5f - SIRO_X);
	verts->uv1.v = _FTOI4(DRAW_HEIGHT/2-0.5f - SIRO_Y);
	verts->xyz0.x = _FTOI4(2048.0f-DRAW_WIDTH/2.0f);
	verts->xyz0.y = _FTOI4(2048.0f-DRAW_HEIGHT/2.0f);
	verts->xyz0.z = 0 ;
	verts->xyz0.flags = 0x8000;
	verts->xyz1.x = _FTOI4(2048.0f+DRAW_WIDTH/2.0f);
	verts->xyz1.y = _FTOI4(2048.0f+DRAW_HEIGHT/2.0f);
	verts->xyz1.z = 0 ;
	verts->xyz1.flags = 0x0000;

	sprt_data++;
	return sprt_data;

}

static void *InitBrownSprtData( void *addr, int n_verts )
{
	SPRT_DATA_BROWN	*sprt_data = (SPRT_DATA_BROWN*)addr;
	_VERT_SPRT_UV	*verts = sprt_data->sprts;
	int i;
	
	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 1/* 後続primなし */, 0, 0, 0, SIZEOF_QWORD(_VERT_SPRT_UV));
	sprt_data->giftag.regs = GIFREG_VERT_SPRT_UV;

	for( i = 0; i < n_verts; i++ ){
		verts->rgba.r = 0 ;
		verts->rgba.g = 0 ;
		verts->rgba.b = 0 ;
		verts->rgba.a = 0 ;

		verts->uv0.u = _FTOI4(0.5f);
		verts->uv0.v = _FTOI4(0.5f);
		verts->uv1.u = _FTOI4(DRAW_WIDTH-0.5f);
		verts->uv1.v = _FTOI4(DRAW_HEIGHT-0.5f);

		verts->xyz0.x = _FTOI4(2048.0f-DRAW_WIDTH/2.0f);
		verts->xyz0.y = _FTOI4(2048.0f-DRAW_HEIGHT/2.0f);
		verts->xyz0.z = 0 ;
		verts->xyz0.flags = 0x8000;
		verts->xyz1.x = _FTOI4(2048.0f+DRAW_WIDTH/2.0f);
		verts->xyz1.y = _FTOI4(2048.0f+DRAW_HEIGHT/2.0f);
		verts->xyz1.z = 0 ;
		verts->xyz1.flags = 0x0000;

		verts ++;
	}

	sprt_data++;
	return sprt_data;

}
#endif //PSX2

typedef struct  
{
   float grayScalePercent;
}
SVRClearEft2RenderWork;

static void *_vr_clear_eft2_buffer_callback(void *pParam, DG_DMAPACK *pDstDmaPack)
{
   Work *work = (Work *)pParam;
   SVRClearEft2RenderWork *dst_work = (SVRClearEft2RenderWork *)DG_AS_SceneBufferAlloc(sizeof(SVRClearEft2RenderWork));
   dst_work->grayScalePercent = work->grayScalePercent;

   return dst_work;
}

static void BP_VRClearCallback(void* pWork)
{
   int i;

   SVRClearEft2RenderWork *work = (SVRClearEft2RenderWork*)pWork;

   SBP_PFX_VRClear* pPacket = (SBP_PFX_VRClear*)BP_RB_Alloc(sizeof(SBP_PFX_VRClear));

   pPacket->percentage = work->grayScalePercent;

   BP_RB_AddCommand(kCmd_PostFX_VRClear, (char*)pPacket);
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	/*
	   TODO:::::
	   
	   dmapack作り、設定
	   
	   */
	int i;
	int handle, status;
	SPR_OBJ *spr;

	// シグナルの登録
	GV_SetActorSignalFunc( work , GV_DefaultSignalFunc );

	work->rust_cnt = -5;

	//GV_SetActorChild( work, NewGrayClut( -1, 0x000e1113 ) );
	//NewGrayClut( 300, 0x00000000 );

	//// debug
	work->rrr = 30;
	work->ggg = 30;
	work->bbb = 12;
	work->aaa = 110;

	for( i = 0; i < RUST_NUM; i ++ ){
		work->step_pos[i] = (BP_PS2_rand()%60)/10.0f  - 3.0f;
	}

#if 0
	work->scrn = GV_Malloc( DRAW_WIDTH*DRAW_HEIGHT/2 );
	if( work->scrn == NULL ){
		printf("vr_clr_eft メモリが足りませんでした\n");
		return -1;
	}
#endif	

	/* フォーカスL2D */
	handle = L2D_LoadLayout2( 5316032/* vr_dcm.l2d */ ,
							  DG_CHANL_MENU , 0, 0, GV_PAUSE_STOP );
	if( !(handle < 0) ){
		work->handle_2d = handle;
	}
	status = L2D_EvokeAction( work->handle_2d, 566267/* DefaultAction */ );
	if( status < 0 ){
		printf("DefaultAction 再生失敗\n");
		return -1;
	}
	spr = L2D_GetObject( work->handle_2d, 2770484/* root */ );
	if ( spr == NULL ){
		printf("root strcode違うよ!!!!\n");
		return -1 ;
	}
	work->root = spr;

	 
	/*
	   Workの初期化
	 */
	
#ifdef PSX2
	/* dmapack初期化 */
	{
		DG_DMAPACK		*dmapack;
		CLREFT_PACK	*packet;
		void			*addr;
		int				i, j;
		
		for( j = 0; j < 16*16; j++ ){
			unsigned int r, g, b, a;
			r = j;
			g = r;
			b = r;
			a = 128;
			work->clut[j] = r | (g << 8) | (b << 16) | (a << 24);
		}

		//パケットメモリ確保
		packet  = work->draw_packet = (CLREFT_PACK*)GV_Malloc( sizeof(CLREFT_PACK)*2 );
		if( packet == NULL ) return -1;

		//ＤＭＡパケット作成
		work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, 130 );/* L2Dより前 */
		if(dmapack==NULL){
			printf("ERR!! MAKE DMAPACK!!\n");
			return -1;
		}
		DG_QueueDmapack( dmapack );
		dmapack->packet[0] = &packet[0] ;
		dmapack->packet[1] = &packet[1];
		dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
#if 1 // BP_Render
      DG_AS_DMAPackSetCallbacks(dmapack, BP_VRClearCallback, _vr_clear_eft2_buffer_callback);
      dmapack->BP_callbackParam = work;
#endif

		for( i = 0; i < 2; i++ ){
			addr = &work->draw_packet[i];
			
			addr = &work->draw_packet[i].bg_image;
			addr = DG_MakeLoadImagePacket( addr,/* 背景データ転送 */
										  SCE_GS_PSMCT16, DRAW_WIDTH/2, DRAW_HEIGHT/2,
										  TEXTURE_TOP_PAGE(), DRAW_WIDTH, work->scrn );
			
			/*
			 * 一連のprimデータの設定
			 */
			addr = &work->draw_packet[i].prim_dmatag;
			addr = InitDmaVifCodeTags( addr,
									  DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(CLREFT_PRIM) ),
									  NULL,
									  SCE_VIF1_SET_FLUSHA( 0 ),
									  SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(CLREFT_PRIM), 0 ) );
			/* 待機バッファをテクスチャとして使う SPRITE prim */
			addr = &work->draw_packet[i].prim_data.bg_set_prim;
			addr = InitSetDrawPrim( addr,
								   SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0), /* Sprite,
																									テクスチャあり
																									(u,v指定),
																									アルファあり */
								   SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ),	/* 通常半透明 */
								   SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
												   BUFFER_WIDTH/64,
												   SCE_GS_PSMCT16,
												   9,9,0,0,0,0,0,0,0 ),
								   BG_CLAMP, 1 );
			addr = InitBgSprtData( addr, 1 );
			/* 茶色 SPRITE prim */
			addr = &work->draw_packet[i].prim_data.brown_set_prim;			
			addr = InitSetDrawPrim( addr,
								   SCE_GS_SET_PRIM( 6, 0, 0, 0, 1, 0, 0, 0, 0), /* SPRITE,テクスチャなし,
																				   アルファあり */
								   SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ),	/* 通常半透明 */
								   0,	/* テクスチャなし */
								   0,	/* テクスチャなし */
								   1 );
			addr = InitBrownSprtData( addr, 1 );

			/*
			 *   描画環境復元設定
			 */
			addr = &work->draw_packet[i].recov_dmatag;
			addr = InitDmaVifCodeTags( addr,
									  DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWENV) ),
									  &( DG_Chanl( 0 )->draw_env[ i ] ) ,
									  SCE_VIF1_SET_NOP( 0 ),
									  SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWENV), 0 ) );
			/*
			 *   終了設定
			 */
			addr = &work->draw_packet[i].end_dmatag;
			addr = InitDmaVifCodeTags( addr,
									  DMATAG_SET_QWC( DMATAG_ID_RET, 0 ),
									  NULL,
									  //SCE_VIF1_SET_NOP( 0 ),
									  SCE_VIF1_SET_FLUSHA( 0 ),
									  SCE_VIF1_SET_NOP( 0 ) );
			
			
		}
	}
#endif //PSX2
#ifdef KP_XBOX
		{
			DG_DMAPACK		*dmapack;
			CLREFT_PACK_XBOX *packet;
			unsigned int psize ;
			
			psize = sizeof( CLREFT_PACK_XBOX );
			printf("CLREFT dmapack size [%d]\n",psize);  
			work->draw_packet = (CLREFT_PACK_XBOX*)GV_Malloc( psize );
			packet = (CLREFT_PACK_XBOX*)work->draw_packet;
			if( packet == NULL ) return -1;
			
			//ＤＭＡパケット作成
			work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, 130 );/* L2Dより前 */
			if( dmapack == NULL ){
				printf("ERR!! MAKE DMAPACK!!\n");
				return -1;
			}
			DG_QueueDmapack( dmapack );
			dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;

			work->dmapack->autopacket = packet ;
			DG_SetDmapackEnd( packet );
			work->prim_x = packet;
			{
				/* 線形テクスチャのメモリ確保 */
				DG_TEX_LIN *tex;
				tex = DG_MakeLinerTexture( DRAW_WIDTH/2, DRAW_HEIGHT/2, DG_TEXLIN_FORMAT_A1R5G5B5 );
				if( tex == NULL ){
					return -1;
				}
				work->lintex = tex;
			}
		}
#endif //KP_XBOX

	
   work->grayScalePercent = 0.0f;

	return 0;
}


void *NewVRClearEffect2( int mode, int *flag  )/* modeの意味はない */
{
	Work		*work ;

	printf("VR_CLR_EFT\n");

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;

		work->yamashita_flag = flag;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	
	return (void *)work ;
}
