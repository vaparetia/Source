//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  	vr_clear_eft.c
	VRクリア画面エフェクト
	2002/05/31 Y.Yano
	
	$Id: vr_clear_eft.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
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

#include "bp_ps2emu.h"

/**************************************************************************************************/
#include 	"../include/dmatags.h"
#include 	"../../shibata/util/dma_set.h"

extern void	*NewDgCamShuterSight( void );

//#define TYPE1

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

#define		BG_CLAMP	(SCE_GS_SET_CLAMP( 2, 2, 0, DRAW_WIDTH, 0, DRAW_HEIGHT ))

extern void *NewGrayClut( int time, u_int add_col );


#ifdef KP_XBOX
typedef struct {
	DG_DMAPACK_TAG		end; //DG_SetDmapackEnd();
} CLREFT_PACK_XBOX;
#endif

typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[1];
} SPRT_DATA_NOISE;
typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[1];
} SPRT_DATA_BG;
typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[1];
} SPRT_DATA_BROWN;
typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	_VERT_SPRT_UV		sprts[2];
} SPRT_DATA_BLACK;
typedef struct {
	DG_GIFTAG	giftag;		
	struct _line_pack{
		_GIF_RGBAQ		rgba0;
		_GIF_XYZ2		xyz0;
		_GIF_RGBAQ		rgba1;
		_GIF_XYZ2		xyz1; 
	} line_pack[ RUST_NUM ]; 
} SPRT_DATA_RUST;
typedef struct {
	/*
	  待機バッファをテクスチャとして使う SPRITE prim
	*/
	SET_DRAW_PRIM		bg_set_prim;			// 描画の設定
	SPRT_DATA_BG		bg_sprt_data;			// TriangleStrip設定
	/*
	  茶色 SPRITE prim
	 */
	SET_DRAW_PRIM		brown_set_prim;				// 描画の設定
	SPRT_DATA_BROWN		brown_sprt_data;				// スプライト設定
#if 0
	/*
	  黒 SPRITE prim
	 */
	SET_DRAW_PRIM		black_set_prim;				// 描画の設定
	SPRT_DATA_BLACK		black_sprt_data;				// スプライト設定
#endif
#if 0
	/*
	  走査線 LINE prim
	 */
	SET_DRAW_PRIM		rust_set_prim;				// 描画の設定
	SPRT_DATA_RUST	rust_sprt_data;				// スプライト設定
#endif
} CLREFT_PRIM;
typedef struct {
#if 0
	DG_DMATAG			tr_dmatag;				
	TR_BUFFER			tr_buffer;				// 退避(フレームをバックに)  user/shibata/util/dma_set.h に記述
#endif

	DG_LOADIMAGE		bg_image;				// グレースケールノイズデータをテクスチャに使用するため

	DG_DMATAG			prim_dmatag;				
	CLREFT_PRIM			prim_data;				// GIFtagでつないだ prim 一連

	DG_DMATAG			recov_dmatag;			// 描画環境復元

	DG_DMATAG			end_dmatag;				// END dmatag( IDがRET )	
} CLREFT_PACK;


/*
 * Work
 */
typedef struct _Work{
	GV_ACT_EX		actor ;

	DG_DMAPACK		*dmapack;
#ifdef PSX2
	CLREFT_PACK	*draw_packet;
	int clut[16*16];
	sceGsStoreImage sp;
#endif
#ifdef KP_XBOX
	CLREFT_PACK_XBOX	*draw_packet;
	void *prim_x;
	int  noise_alpha;
	int  noise_alpha2;
	int  white_alpha;
	DG_TEX_LIN *noise_tex;
	DG_TEX_LIN noise_tex2;/* DG_MakeLinerTexture2使用 */
#endif

	float step_pos[RUST_NUM];
	int   rust_alp[RUST_NUM];
	int flag;
	int rust_cnt;
	short *scrn;
	int 	handle_2d;
	SPR_OBJ *root;

//#ifndef TYPE1
	float black_height;
//#endif

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

  dst = work->scrn; /* 16階調グレースケールが格納されるバッファのポインタ */
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



static void Act( Work *work )
{
	int stat;


#ifdef KP_XBOX
	work->prim_x = work->dmapack->autopacket;
#endif

	stat = L2D_ActionStatus( work->handle_2d );
	if( stat == L2D_STAT_ACK && work->flag == 0 ){/* def-actd終了 */
		printf("再生\n");
		L2D_EvokeAction( work->handle_2d, 13328255/* Action-1 */ );
		work->flag = 1;
	} else if( stat == L2D_STAT_ACK && work->flag == 1 ){/* Action-1終了 */
		printf("カシャ\n");		
		//GV_SetActorChild( work , NewDEMODgCamShuterSight() );
		GV_SetActorChild( work , NewDgCamShuterSight() );
		GM_SeSet3D( 0x00, GM_MAX_VOL, SD_I_CAMERA02, -M_PI/2 ) ;	
		SPR_HIDE( work->root );
		work->flag = 2;
	} else if( work->flag == 1 ){/* Action-1再生中 */
		SPR_SHOW( work->root );
	}		
	
#if 0
	if( work->rust_cnt < 10 ){
		_VERT_SPRT_UV  *sprts0 = &work->draw_packet[DG_Clock].prim_data.black_sprt_data.sprts[0];
		_VERT_SPRT_UV  *sprts1 = &work->draw_packet[DG_Clock].prim_data.black_sprt_data.sprts[1];
		
		printf("AAA\n");
		sprts0->rgba.a = 128;
		sprts1->rgba.a = 128;

		if( work->rust_cnt >= 5 ){
			sprts0->xyz0.y = _FTOI4( 2048.0f - DRAW_HEIGHT/2.0f );
			sprts0->xyz1.y = _FTOI4( 2048.0f - DRAW_HEIGHT/2.0f + (9-work->rust_cnt)*50 );
			
			sprts1->xyz0.y = _FTOI4( 2048.0f + DRAW_HEIGHT/2.0f );
			sprts1->xyz1.y = _FTOI4( 2048.0f + DRAW_HEIGHT/2.0f - (9-work->rust_cnt)*50 );
		}
		if( work->rust_cnt < 5 ){
			sprts0->xyz0.y = _FTOI4( 2048.0f - DRAW_HEIGHT/2.0f );
			sprts0->xyz1.y = _FTOI4( 2048.0f - DRAW_HEIGHT/2.0f + work->rust_cnt*50 );
			
			sprts1->xyz0.y = _FTOI4( 2048.0f + DRAW_HEIGHT/2.0f );
			sprts1->xyz1.y = _FTOI4( 2048.0f + DRAW_HEIGHT/2.0f - work->rust_cnt*50 );
		}
		
	}		
	if( work->rust_cnt == 30 ){
		char *addr;
/*
		_VERT_SPRT_UV  *sprts0 = &work->draw_packet[DG_Clock].prim_data.black_sprt_data.sprts[0];
		_VERT_SPRT_UV  *sprts1 = &work->draw_packet[DG_Clock].prim_data.black_sprt_data.sprts[1];
*/
		addr = &work->draw_packet[DG_Clock].prim_data.black_sprt_data;
		InitBlackSprtData( work, addr, 2 );
		addr = &work->draw_packet[1-DG_Clock].prim_data.black_sprt_data;
		InitBlackSprtData( work, addr, 2 );
/*
		sprts0->rgba.a = 0;
		sprts1->rgba.a = 0;
		sprts0 = &work->draw_packet[1-DG_Clock].prim_data.black_sprt_data.sprts[0];
		sprts1 = &work->draw_packet[1-DG_Clock].prim_data.black_sprt_data.sprts[1];
		sprts0->rgba.a = 0;
		sprts1->rgba.a = 0;
*/
	}		
#endif
	if( work->rust_cnt == 45 ){
		char *codec_area;
		char *addr;
		
		printf("AAB\n");

#ifdef PAL
#define CODEC_MEM_SIZE  (1024 * 1024 * 3 + 256*1024)
#else
#define CODEC_MEM_SIZE  (1024 * 1024 * 3)
#endif
		if( NULL == (codec_area = DG_ResizePacketMemory(CODEC_MEM_SIZE)) ){
			ASSERT( 0 );
			return ;
		}
		
#ifdef PSX2
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
#else // XBOX
		/* 画面を取得 */
#ifndef KP_WINDOWS
		DG_StoreImage( codec_area , 1 /* 表示中バッファ */, 
					  0, 16		/*フレームパッファは480。真中448をとる為*/, 
					  DRAW_WIDTH, DRAW_HEIGHT,
					  0			/* 32bit */ );
#else
		DG_StoreImage( codec_area , 1 /* 表示中バッファ */, 
					  (DISPLAY_WIDTH-DRAW_WIDTH) >> 1,
					  (DISPLAY_HEIGHT-DRAW_HEIGHT) >> 1, /*フレームパッファ真中8をとる為*/
					  DRAW_WIDTH, DRAW_HEIGHT,
					  0			/* 32bit */ );
#endif
#endif
		/*
		 * 16階調のグレースケールに変換する
		 */
		work->scrn = GV_Malloc( 512*256*4 );
		if( work->scrn == NULL ){
			ASSERT( 0 );
			return ;
		}
		scrn_shrink( work, codec_area );
		scrn_grayscale( work, codec_area );

#ifdef PSX2
		addr = &work->draw_packet[DG_Clock].bg_image;
		addr = DG_MakeLoadImagePacket( addr,/* 背景データ転送 */
									  SCE_GS_PSMCT16, 256, 256,
									  TEXTURE_TOP_PAGE(), 512, work->scrn );

		addr = &work->draw_packet[1-DG_Clock].bg_image;
		addr = DG_MakeLoadImagePacket( addr,/* 背景データ転送 */
									  SCE_GS_PSMCT16, 256, 256,
									  TEXTURE_TOP_PAGE(), 512, work->scrn );
#endif

		DG_ResizePacketMemory(0);

	}

#ifdef PSX2
	if( work->rust_cnt > 45 ){
		/* 背景 */
		int alp;
		alp = work->draw_packet[1-DG_Clock].prim_data.bg_sprt_data.sprts[0].rgba.a;
		alp += 10;
		if( alp > 128 ){
			alp = 128;
		}
		work->draw_packet[DG_Clock].prim_data.bg_sprt_data.sprts[0].rgba.a = alp;
	}
#endif


	work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
	//printf("clr%d\n",work->rust_cnt);

#ifdef PSX2
	/* 茶色 */
	work->draw_packet[DG_Clock].prim_data.brown_sprt_data.sprts[0].rgba.r = work->rrr;
	work->draw_packet[DG_Clock].prim_data.brown_sprt_data.sprts[0].rgba.g = work->ggg;
	work->draw_packet[DG_Clock].prim_data.brown_sprt_data.sprts[0].rgba.b = work->bbb;
	work->draw_packet[DG_Clock].prim_data.brown_sprt_data.sprts[0].rgba.a = 30;
#endif

	work->rust_cnt ++;

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

	GV_Free( work->scrn );
	printf("!!!?\n");
}
/* ---------------------------------------------------------------- */

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

static void *InitBlackSprtData( Work *work, void *addr, int n_verts )
{
	SPRT_DATA_BROWN	*sprt_data = (SPRT_DATA_BROWN*)addr;
	_VERT_SPRT_UV	*verts = sprt_data->sprts;
	int i;
	
	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 0/* 後続primあり */, 0, 0, 0, SIZEOF_QWORD(_VERT_SPRT_UV));
	sprt_data->giftag.regs = GIFREG_VERT_SPRT_UV;

	//work->black_height = (float)(BP_PS2_rand()%40) + 80.0f;
	
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
		verts->xyz1.y = _FTOI4(2048.0f-DRAW_HEIGHT/2.0f);
		verts->xyz1.z = 0 ;
		verts->xyz1.flags = 0x0000;

		verts ++;
	}

	sprt_data++;
	return sprt_data;

}

static void *InitBrownSprtData( void *addr, int n_verts )
{
	SPRT_DATA_BROWN	*sprt_data = (SPRT_DATA_BROWN*)addr;
	_VERT_SPRT_UV	*verts = sprt_data->sprts;
	int i;
	
	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 0/* 後続primあり */, 0, 0, 0, SIZEOF_QWORD(_VERT_SPRT_UV));
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
static void *InitRustSprtData( Work *work, void *addr, int n_verts )
{
	SPRT_DATA_RUST	*sprt_data = (SPRT_DATA_RUST*)addr;
	SPRT_DATA_RUST	*sprt_data2 = (SPRT_DATA_RUST*)((char*)addr + sizeof(CLREFT_PACK));
	struct _line_pack	*verts = sprt_data->line_pack;
	struct _line_pack	*verts2 = sprt_data2->line_pack;
	int i;
	float pos;
	
	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 1/* 後続primなし */, 0, 0, 0, SIZEOF_QWORD(struct _line_pack));
	sprt_data->giftag.regs = 0x5151;
	sprt_data2->giftag.tag = SCE_GIF_SET_TAG( n_verts, 1/* 後続primなし */, 0, 0, 0, SIZEOF_QWORD(struct _line_pack));
	sprt_data2->giftag.regs = 0x5151;

	for( i = 0; i < n_verts; i++ ){
		verts->rgba0.r = verts2->rgba0.r = 0 ;
		verts->rgba0.g = verts2->rgba0.g = 0 ;
		verts->rgba0.b = verts2->rgba0.b = 0 ;
		verts->rgba0.a = verts2->rgba0.a = 0 ;
		work->rust_alp[i] = BP_PS2_rand() % RUST_MAX_ALPHA ;

		pos = (float)(BP_PS2_rand() % DRAW_HEIGHT);

		verts->xyz0.x = verts2->xyz0.x = _FTOI4(2048.0f-DRAW_WIDTH/2.0f);
		verts->xyz0.y = verts2->xyz0.y = _FTOI4(2048.0f-DRAW_HEIGHT/2.0f + pos);
		verts->xyz0.z = verts2->xyz0.z = 0 ;
		verts->xyz0.flags = verts2->xyz0.flags = 0x8000;
		//verts->xyz0.flags = 0x0000;
		
		verts->rgba1.r = verts2->rgba1.r = 0 ;
		verts->rgba1.g = verts2->rgba1.g = 0 ;
		verts->rgba1.b = verts2->rgba1.b = 0 ;
		verts->rgba1.a = verts2->rgba1.a = verts->rgba0.a ;
		verts->xyz1.x = verts2->xyz1.x = _FTOI4(2048.0f+DRAW_WIDTH/2.0f);
		verts->xyz1.y = verts2->xyz1.y = verts->xyz0.y;
		verts->xyz1.z = verts2->xyz1.z = 0 ;
		verts->xyz1.flags = verts2->xyz1.flags = 0x0000;
		
		verts ++;
		verts2 ++;
	}
	
	sprt_data++;
	return sprt_data;
}

static void *InitNOISESprtData( void *addr, int n_verts )
{
	SPRT_DATA_NOISE	*sprt_data = (SPRT_DATA_NOISE*)addr;
	_VERT_SPRT_UV	*verts = sprt_data->sprts;
	
	sprt_data->giftag.tag = SCE_GIF_SET_TAG( n_verts, 1, 0, 0, 0, SIZEOF_QWORD(_VERT_SPRT_UV));
	sprt_data->giftag.regs = GIFREG_VERT_SPRT_UV;

	verts->rgba.r = 128 ;
	verts->rgba.g = 128 ;
	verts->rgba.b = 128 ;
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
	 
	sprt_data++;
	return sprt_data;

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

		for( i = 0; i < 2; i++ ){
			addr = &work->draw_packet[i];
			
#if 0
			/*
			 *  テクスチャ領域をバックバッファとして描画フレームを退避する設定
			 */
			addr = &work->draw_packet[i].tr_dmatag;
			addr = InitDmaVifCodeTags( addr,
									  DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TR_BUFFER) ),
									  NULL,
									  SCE_VIF1_SET_FLUSHA(0),/* 念のため */
									  SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TR_BUFFER), 0 ) );
			InitTrBuffer( &packet[i].tr_buffer, i );// user/shibata/util/dma_set.h より
			addr += sizeof( TR_BUFFER );
#endif
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
#if 0
			/* 黒 SPRITE prim */
			addr = &work->draw_packet[i].prim_data.black_set_prim;			
			addr = InitSetDrawPrim( addr,
								   SCE_GS_SET_PRIM( 6, 0, 0, 0, 1, 0, 0, 0, 0), /* SPRITE,テクスチャなし,
																				   アルファあり */
								   SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ),	/* 通常半透明 */
								   0,	/* テクスチャなし */
								   0,	/* テクスチャなし */
								   1 );
			addr = InitBlackSprtData( work, addr, 2 );
#endif
#if 0
			/* 走査線 LINE prim */
			addr = &work->draw_packet[i].prim_data.rust_set_prim;			
			addr = InitSetDrawPrim( addr,
								   SCE_GS_SET_PRIM( SCE_GS_PRIM_LINE, 0, 0, 0, 1, 0, 0, 0, 0), /* Line,
																								  フラットシェーディング
																								  テクスチャなし,
																								  アルファあり */
								   SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ),	/* 通常半透明 */
								   0,	/* テクスチャなし */
								   0,	/* テクスチャなし */
								   1 );
			if( i == 0 ){
				addr = InitRustSprtData( work, addr, RUST_NUM );
			}
#endif

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
#endif
#ifdef KP_XBOX
#if 0
		{
			DG_DMAPACK		*dmapack;
			unsigned int psize ;
			
			psize = sizeof( POWEROFF_PACK_XBOX );
			printf("size [%d]\n",psize);  
			work->draw_packet = (POWEROFF_PACK_XBOX*)GV_Malloc( psize );
			//work->draw_packet = (POWEROFF_PACK_XBOX*)GV_Malloc( 1024 );
			packet = (POWEROFF_PACK_XBOX*)work->draw_packet;
			if( packet == NULL ) return -1;
			
			//ＤＭＡパケット作成
			work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_LAST, 254 );
			if( dmapack == NULL ){
				printf("ERR!! MAKE DMAPACK!!\n");
				return -1;
			}
			DG_QueueDmapack( dmapack );
			dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
			dmapack->flag |= DG_DMAPACK_PRIVILEGE;/* 特権モード(他のメニューなどをみえなくするため)*/

			work->dmapack->autopacket = packet ;
			//InitPowerOffPacket_XBOX( work, work->dmapack->autopacket );
			DG_SetDmapackEnd( packet );
			work->prim_x = packet;

			{
				//線形テクスチャでノイズテクスチャ作り
				DG_TEX_LIN *tex;
				tex = DG_MakeLinerTexture( DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_A1R5G5B5 );
				memcpy( tex->image, SAND_ADDR, DRAW_WIDTH*DRAW_HEIGHT*2 );
				work->noise_tex = tex;
				// 毎フレームで設定する↓
				//DG_MakeLinerTexture2( &work->noise_tex2, DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_P8, SAND_ADDR );
			}
		}
#endif
#endif

	
	return 0;
}


void *NewVRClearEffect( int mode  )/* modeの意味はない */
{
	Work		*work ;

	printf("VR_CLR_EFT\n");

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return (void *)work ;
}
