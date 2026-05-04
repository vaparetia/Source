//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   cdc_noise.c
   やっととりかかれる無線ノイズ Actor (無線モード専用(?))
   
   2001/06/08  Yoshihito Kira
   $Id: cdc_noise.c,v 1.6 2002/11/25 14:15:28 takaki Exp $
   */
#if 0 // BP_Render #ifdef PSX2
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
#endif
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libdg.cnf"
#include "libutl.h"
#include "dmapack.h"
#include "def_dma.h"

#define _codeccrt_c_
#include "codeccrt.h"
#include "codecmem.h"

#include "codec_config.h"
#include "c_layout.h"
#include "cdc_face.h"
#include "cdc_noise.h"
#include "codec_signal.h"

#include "BP_RenderFX.h"

#if 0 // BP_Render #ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...) printf(__VA_ARGS__)
#endif /* _DEBUG_ */
#endif
#if 1 // BP_Render #ifdef KP_XBOX
#define DBG
#endif

#define F_PI  ((float)M_PI)


#define ALL_MASK  (DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 | \
				   DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 | \
				   DG_DMAPACK_INVISIBLEMENU)

#define TEX_BASE_4	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_BASE_16     (TEXTURE_TOP_PAGE()/64)
#define CLUT_BASE	(0x100000/64-3)
#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)
#define DSIZ(_type)   ((sizeof(_type) + 7) / 8)

#define FINISH_TIME  30

#define MAX_ALPHA  128

#define SAND_MODE_16
// #undef SAND_MODE_16

extern char _program_top[];

// AS(JM) - Disables printf for whole CPP. If it's already disabled, redisable it
#ifdef printf
#  undef printf
#endif

#define printf 

#define DIST_ENABLE

typedef ALIGN16_DECL(struct) {
	DG_DMATAG  dmatag0;
	DG_GIFTAG  giftag0;
	DG_GSREG   gsregs[4];
	DG_GIFTAG  giftag1;
	DG_DMATAG  dmatag1;
} TexTrans ;

typedef ALIGN16_DECL(struct) {
	DG_DMATAG  dmatag0;
	DG_GIFTAG  giftag0;
	DG_GSREG   bitbltbuf;
	DG_GSREG   trxpos;
	DG_GSREG   trxreg;
	DG_GSREG   trxdir;

	DG_GIFTAG  giftag1;
	DG_DMATAG  dmatag1;
} ClutTrans ;

typedef ALIGN16_DECL(struct) {

	DG_DMATAG    dmatag;

	ALIGN16_DECL(struct) dma_pack {
		/*
		 * REGLIST mode で転送てきないレジスタの内容を PACKED mode で転送
		 */    
		DG_GIFTAG    giftag0;	/* prim も一緒に送信 */
		struct gif_pack_1 {
			DG_GSREG   texflush;
			DG_GSREG   texa;
			DG_GSREG   test;
			DG_GSREG   alpha;
		} gif_1;

		/*
		 * REGLIST mode で主要な描画部分を転送
		 */
		DG_GIFTAG    giftag1;
		ALIGN16_DECL(struct) gif_pack_2 {
     
			u_long64  clamp;
			u_long64  tex0;

			u_long64  prim;
			u_long64  rgbq;
      
			u_long64  uv0;
			u_long64  xyz0;
      
			u_long64  uv1;
			u_long64  xyz1;
      
		} gif_2 ;
	}  dmap;

} SpriteDraw ;

/*
 * 砂嵐転送用
 */
typedef ALIGN16_DECL(struct) {
	TexTrans    trans;			/* テクスチャの転送 */
	SpriteDraw  sprite;			/* スプライト描画   */
} sand_trans ;


typedef ALIGN16_DECL(struct) {
	DG_DMATAG   dmatag;
	struct _dma_vfast {
		DG_GIFTAG  giftag0;
		struct _gif_vfast_0 {
			DG_GSREG   texflush;
			DG_GSREG   texa;
			DG_GSREG   test;
			DG_GSREG   alpha;
		} gif0;
		DG_GIFTAG  giftag1;
		struct _gif_vfast_1 {
			u_long64 clamp;
			u_long64 tex0;
			u_long64 prim;
			u_long64 rgbq;
		} gif1;

		DG_GIFTAG  giftag2;
		struct _gif_vfast_2 {
			u_long64 uv0;
			u_long64 xyz0;
			u_long64 uv1;
			u_long64 xyz1;
		} gif2[ VFAST_NOISE ];
	} dma;
} VFastNoise ;

typedef ALIGN16_DECL(struct) {
	DG_DMATAG   dmatag;
	struct _vfpic_dma {
		DG_GIFTAG giftag0;
		struct _vfpic_gif0 {
			DG_GSREG  prim;
			DG_GSREG  alpha;
			DG_GSREG  rgbaq;
		} gif0;

		DG_GIFTAG giftag1;
		struct _vfpic_gif1 {

			u_long64 rgbaq0;
			u_long64 uv0;
			u_long64 xyz0;/* 頂点キックなし */
			u_long64 uv1;
			u_long64 xyz1;/* 頂点キックなし */

			u_long64 rgbaq1;
			u_long64 uv2;
			u_long64 xyz2;
			u_long64 uv3;
			u_long64 xyz3;
      
		} gif1[ VFAST_NOISE ];
	} dma;
} VFastPict ;


/*
 * ビデオ早送りノイズ転送用
 */
typedef ALIGN16_DECL(struct) {
	DG_DMATAG   dmatag_fb;		/* フレームバッファテクスチャを使用するタグ */
#ifdef DIST_ENABLE
	VFastPict   picdraw;		/* フレームバッファの内容を変形させて描画   */
#endif /* DIST_ENABLE */
	TexTrans    trans;			/* ノイズテクスチャ転送部 */
	VFastNoise   vfdraw;		/* ノイズ部描画           */

} vfast_trans ;


/*
 * ゴースト用パケット
 */
typedef ALIGN16_DECL(struct) {
	DG_DMATAG dmatag_fb;		/* フレームバッファテクスチャを使用するタグ */

	DG_DMATAG dmatag;
	struct _ghost_dma {
		DG_GIFTAG giftag0;
		struct _ghost_gif0 {
			DG_GSREG prim;
			DG_GSREG alpha;
			DG_GSREG rgbaq;
		} gif0;
		DG_GIFTAG giftag1;
		struct _ghost_gif1 {
			u_long64 uv0;
			u_long64 xyz0;
			u_long64 uv1;
			u_long64 xyz1;
		} gif1;
	} dma;
} ghost_trans ;


/*
 * 大規模な同期ずれ表現転送用パケット
 */
typedef ALIGN16_DECL(struct) {
	DG_DMATAG     dmatag_fb;	/* フレームバッファテクスチャ参照パケット */

	DG_DMATAG     dmatag;
	struct _bw_dma {
		DG_GIFTAG   giftag0;
		struct _bw_gif0 {
			DG_GSREG prim;
			DG_GSREG alpha;
			DG_GSREG rgbaq;
		} gif0;
		DG_GIFTAG   giftag1;
		struct _bw_gif1 {
			struct _bw_vert {
				u_long64 rgbaq;
				u_long64 uv;
				u_long64 xyz;
				u_long64 nop;
			} vert[10];
		} gif1;
	} dma;
} bigwave_trans ;


/*
 * 垂直同期ずれ用パケット
 */
typedef ALIGN16_DECL(struct) {
	DG_DMATAG   dmatag_fb;

	DG_DMATAG   dmatag;

	struct _usyncV_dma {
		DG_GIFTAG  giftag0;

		/* 基礎的なパラメータの転送 */
		struct _usyncV_gif0 {
			DG_GSREG  prim;
			DG_GSREG  alpha;
			DG_GSREG  rgbaq;
		} gif0;

		/* 上半分, 下半分の描画 */
		DG_GIFTAG giftag1;
		struct _unsyncV_gif1 {
			u_long64 uv0;
			u_long64 xyz0;
			u_long64 uv1;
			u_long64 xyz1;
		} gif1[2];
      
		/* 間の黒い帯 */
		DG_GIFTAG giftag2;
		struct _unsyncV_gif2 {
			u_long64 prim;
			u_long64 rgbaq;
			u_long64 xyz0;
			u_long64 xyz1;
		} gif2;
	} dma;
} unsyncV_trans ;




typedef struct _cdc_noise_Work {
	GV_ACT_EX    actor;

	int          side;
	int          chanl;

	int          noise_type;	/* ノイズのタイプ               */
	int          noise_time;	/* ノイズ開始からの経過時間     */
	int          noise_len;		/* ノイズを発生させつづける時間 */
	int          finish_time;

	void       (*act_func)(struct Work * work);
	void       (*die_func)(struct Work * work);
	int          step;

	DG_DMAPACK * dmapack;

	union {
		/*
		 * サンドノイズ特有の領域
		 */
		struct {
			void * packet[1];
			DG_TEX_LIN* texlin;
			int         alpha;
			int         step;	/* 表示ステップ
								   0   正位置   / 1   左右反転
								   2   上下反転 / 3   上下左右反転  */
			unsigned int clut[16];
		} sand;

		/*
		 * 早送り特有の領域
		 */
		struct {
			void  * packet[1];
			DG_TEX_LIN* ltex ;

			int            se_cnt;
			int            vfast_se:1; /* 非0 で有効 */
		} vfast;

		/*
		 * ゴースト特有の領域
		 */
		struct {
			ghost_trans * packet[2];
			int           delay;
		} ghost;

		/*
		 * 垂直同期ずれ特有の領域
		 */
		struct {
			unsyncV_trans * packet[2];
			float           speed; /* スクロールの速度 */
			float           gap; /* 現在のずれ       */
		} unsyncV;

		/*
		 * 大規模な同期ずれ特有の領域
		 */
		struct {
			bigwave_trans * packet[2];

			float bias;
			int   cnt;
		} bigwave;
	} params;


	int          finish:1;		/* 非 0 で終了 */
} Work;


static Work * now_work[2] = {NULL, NULL };
static int    noise_cnt[2] = {0, 0};

static void setup_tex_trans(TexTrans * trans,
							void * tex, int width, int height)
{
}

/*
 * スプライト転送パケットの初期化
 */
static void setup_sprite(SpriteDraw * spr,
						 u_long64 alpha,
						 u_long64 tex0,
						 u_long64 uv0,
						 u_long64 uv1,
						 u_long64 xyz0,
						 u_long64 xyz1)
{
}

static void setup_sand_trans(Work * work, sand_trans * sand)
{
}

/* サンドノイズ初期化 */
static void InitSand(Work * work)
{
	int i;
	unsigned int col;

	work->params.sand.alpha = MAX_ALPHA;

	/* clut の中身を用意 */
	for(i = 0; i < 16; i++)
	  {
		  col = 0x80000000;
		  if(i & 1) col |= 0x80;
		  if(i & 2) col |= 0x8000;
		  if(i & 4) col |= 0x800000;
		  if(i & 8) col |= 0x7f7f7f;
		  work->params.sand.clut[i] = col;
	  }

	/*
	 * 転送パケット領域を用意
	 */
	if ( !(work->params.sand.packet[0] = codecMalloc( 256 )) ) {
		//	  return -1 ;
		//printf("in 'cdc_noise.c' cannot make memory!\n");
		ASSERT( 0 );
	}
	work->dmapack->autopacket = work->params.sand.packet[0];
	DG_SetDmapackEnd( work->params.sand.packet[0] );

   {
      int y, x;
      u_int* pImage;
      int face_tex_width = (int)FACE_WIDTH;
      int face_tex_height = (int)FACE_HEIGHT;
      unsigned char* pRandomNoise = GetRandomNoiseValues512x512();

      work->params.sand.texlin = DG_MakeLinerTexture( face_tex_width, face_tex_height, DG_TEXLIN_FORMAT_A8R8G8B8 );
      pImage = (u_int*)work->params.sand.texlin->image;
      for( y=0;y<face_tex_height;++y )
      {
         for( x=0;x<face_tex_width;++x )
         {
            int randomValue = pRandomNoise[x+y*face_tex_width]&0xf;
            pImage[x+y*face_tex_width] = work->params.sand.clut[randomValue];
         }
      }
   }
}

/*
 * サンドノイズの主処理
 *
 * いわゆる夜中の砂嵐(最近は夜中でも砂嵐は見掛けない…)
 */
static void ActSand(Work * work)
{
	int r;
	int u[2], v[2];
	int w, h;
	
	/* 終了していたら、表示を徐々に薄くする。 */
	work->params.sand.alpha = MAX_ALPHA;
	if(work->finish)
	  {
		  int left_t = work->finish_time;
		  int alpha;

		  alpha = (MAX_ALPHA * (FINISH_TIME - left_t)) / FINISH_TIME;
		  if(alpha < 0) alpha = 0;
		  work->params.sand.alpha = alpha;
	  }

	/* 直前の表示以外の三種のうちから、ランダムに選択する */
	r = (BP_PS2_rand() >> 16) % 3;
	if(r >= work->params.sand.step) r++;
	work->params.sand.step = r;

	w = FACE_WIDTH - ((BP_PS2_rand() >> 16) % 32);
	h = FACE_HEIGHT - ((BP_PS2_rand() >> 16) % 32);

	u[0] = 8;
	v[0] = 8;
	u[1] = w * 16 - 8;
	v[1] = h * 16 - 8;

	/* 状態にあわせて上下左右の反転をかける */
	if(r & 1) u[1] ^= u[0], u[0] ^= u[1], u[1] ^= u[0];
	if(r & 2) v[1] ^= v[0], v[0] ^= v[1], v[1] ^= v[0];
	
	{
		void			*prim ;
		unsigned int	col = 0x80808080;
		float chanlw, chanlh, offset_x, offset_y ;
		DG_TEX_LIN *tex = work->params.sand.texlin;
		prim = work->dmapack->autopacket ;
		prim = DG_SetDmapackTexLin( prim, tex );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->params.sand.alpha) );
 		chanlw = (float)DG_Chanls[work->chanl].width;
		chanlh = (float)DG_Chanls[work->chanl].height;
		prim = DG_SetDmapackSprt(prim, 0.0f, 0.0f, 
								 (float)(u[0]/16.0f)/FACE_WIDTH, (float)(v[0]/16.0f)/FACE_HEIGHT,
								 (float)chanlw, (float)chanlh,
								 (float)(u[1]/16.0f)/FACE_WIDTH, (float)(v[1]/16.0f)/FACE_HEIGHT, 
								 DG_MakeDmaPackColorFromInt(col) );
		prim = DG_SetDmapackEnd( prim );
	}
}

static void DieSand(Work * work)
{
	codecDelayedFree(work->params.sand.packet[0]);
	DG_FreeLinerTexture( work->params.sand.texlin );
}




/*
 * ビデオ早送りノイズ
 *
 * VHS のビデオを早送りしたときに、画面を3～5分割する間隔でノイズエリアが走り,
 * 映像がある部分も同期ずれのために画像がゆがむノイズ。
 */
static void vfast_set_vertex(Work * work, int clock)
{
}

static void vfast_set_uv(Work * work, int clock)
{
}

#ifdef DIST_ENABLE
static void vfast_set_dist(Work * work, vfast_trans * pack)
{
}
#endif /* DIST_ENABLE */

static void InitVFast(Work * work)
{
	int dmasize;

	dmasize = sizeof(DG_DMAPACK_PARAM)*2+sizeof(DG_DMAPACK_ALPHA)
	  +sizeof(DG_DMAPACK_TRIANGLESTRIP)*VFAST_NOISE+sizeof(DG_DMAPACK_VERTEX)*4*VFAST_NOISE
		+sizeof(DG_DMAPACK_TEX)+sizeof(DG_DMAPACK_ALPHA)+sizeof(DG_DMAPACK_SPRT)*VFAST_NOISE
		  +sizeof(DG_DMAPACK_TAG);
	work->params.vfast.packet[0] = codecMalloc( dmasize );

	ASSERT( NULL != work->params.vfast.packet[0] );

	work->dmapack->autopacket = work->params.vfast.packet[0];
	DG_SetDmapackEnd( work->params.vfast.packet[0] );
	{/* 線形テクスチャ作り */
		DG_TEX_LIN *tex;
		int i;

		work->params.vfast.ltex = DG_MakeLinerTexture( (int)(FACE_WIDTH), (int)(FACE_HEIGHT),
													   DG_TEXLIN_FORMAT_A1R5G5B5 );
		if( work->params.vfast.ltex == NULL ){
			//printf("no mem for vfast ltex\n");
			ASSERT( 0 );
		}
		tex = work->params.vfast.ltex;
		memcpy( tex->image, GetRandomNoiseValues512x512(), (int)(tex->width*tex->height*sizeof(short)) );
		for( i = 0; i < tex->width*tex->height; i++ ){
			((short*)tex->image)[i] |= (1<<15); /* alpha = 1 を代入 */
		}
	}

	work->params.vfast.se_cnt = 0;
	work->params.vfast.vfast_se = 1;
}


static void ActVFast(Work * work)
{
	/* 早送りの SE が入る */
	if(work->side && work->params.vfast.vfast_se)
	  {
		  /*
		   * 重複を防ぐために、プレイヤ側でのみ処理
		   */
		  work->params.vfast.se_cnt += TIME_BASE;
		  if(work->params.vfast.se_cnt >= 15)
			{
				work->params.vfast.se_cnt -= 15;
				CodecSE(CODEC_SE_VTRFAST);
			}
	  }
	{
		void *prim;
		DG_TEX_LIN *ltex;
		prim = work->dmapack->autopacket ;

#ifdef DIST_ENABLE
		
		/* フレームバッファを変形描画 */
		prim = DG_SetDmapackBackupFrame( prim, 0 );

      // AS(JM) - Commented out UseFrameTex below, as UseFrameTex in the codec is NG for Vita
//		prim = DG_SetDmapackUseFrameTex( prim, 2 );	/* 待機バッファをテクスチャとして使用する宣言 */

      // AS(JM) - Changed alpha mode to standard alpha blend, since we're not using
      // a frame texture here anymore
      prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 0, 1, 128) );
		{
			float base_u, base_v;
			float base_x, base_y;
			float w, h;
			float u[2], v[2], begin_v, step_v;
			float x[2], y[2];
			float x_bias;
			int i;
			unsigned int col, col2;

			w = DG_Chanls[work->chanl].width;
			h = DG_Chanls[work->chanl].height;

         // BP_RENDER - Without + 1 uv is actually right on edge, actually sampling half
         // black border half cdc portrait
			base_u = DRAW_WIDTH  / 2 + (DG_Chanls[work->chanl].offset_x + 1 - w / 2);
			base_v = DRAW_HEIGHT / 2 + (DG_Chanls[work->chanl].offset_y + 1 - h / 2);
			begin_v = h * 15 / 100;
			step_v = (h * 70) / (VFAST_SPLIT * 100);
			
			x_bias = (BP_PS2_rand() % 10 + 1) * 8.0F;
			
			for(i = 0; i < VFAST_NOISE; i++){

				u[0] = base_u ;
				u[1] = (base_u + w);
				v[0] = (base_v + begin_v + i * step_v);
				v[1] = (base_v + begin_v + i * step_v + step_v / 3);

				x[0] = 0.0f ;
				x[1] = w ;
				y[0] = begin_v + i * step_v + VFAST_HEIGHT_NOISE ;
				y[1] = begin_v + i * step_v + step_v / 3 ;

				prim = DG_SetDmapackTriangleStrip( prim, 4 );/* 頂点数分設定必要!! */
				// AS(JM) - Modified start color since this is just an alpha blended poly now
            col = 0x20FFFFFF;

				prim = DG_SetDmapackVertex( prim, x[0] + (x_bias)/16.0f, y[0], DG_FRAME_U(u[0]), DG_FRAME_V(v[0]), DG_MakeDmaPackColorFromInt(col) );
				prim = DG_SetDmapackVertex( prim, x[1] + (x_bias)/16.0f, y[0], DG_FRAME_U(u[1]), DG_FRAME_V(v[0]), DG_MakeDmaPackColorFromInt(col) );

            // AS(JM) - Modified end color since this is just an alpha blended poly now too
            col = 0x00FFFFFF;
				prim = DG_SetDmapackVertex( prim, x[0]         , y[1], DG_FRAME_U(u[0]), DG_FRAME_V(v[1]), DG_MakeDmaPackColorFromInt(col) );
				prim = DG_SetDmapackVertex( prim, x[1]         , y[1], DG_FRAME_U(u[1]), DG_FRAME_V(v[1]), DG_MakeDmaPackColorFromInt(col) );

			}
			
		}
#endif /* DIST_ENABLE */

		/* ノイズの描画 */
		ltex = work->params.vfast.ltex;
		prim = DG_SetDmapackTexLin( prim , ltex );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, 128) );
		{/* 頂点の設定  */
			int j;
			
			float y_base, y_step, y_begin;
			float home_x, home_y;
			float w, h;
						
			float u[2], v[2], tmp;
			int r, i;
			int fw, fh;
			unsigned int col = 0x80808080 ;
			
			/* 座標pre計算 */
			w = (float)DG_Chanls[work->chanl].width;
			h = (float)DG_Chanls[work->chanl].height;

			y_begin = h * 15 / 100;
			y_step = (h * 70) / (VFAST_SPLIT * 100);

			/* UV値pre計算 */
			r = BP_PS2_rand() % 3;
			fw = FACE_WIDTH - (BP_PS2_rand() % 32);
			fh = FACE_HEIGHT - (BP_PS2_rand() % 32);

			for(j = 0; j < VFAST_NOISE; j++){
				/* 座標 */
				y_base = y_begin + y_step * j;
				
				/* UV値 */
				/* 数ラスタ分のノイズとして何処を使うかを選出する */
				u[0] = 0.5F;
				u[1] = (float)fw - 0.5F;
				v[0] = (float)(BP_PS2_rand() % (fh - VFAST_HEIGHT_NOISE)) - 0.5F;
				v[1] = (v[0] + (float)VFAST_HEIGHT_NOISE) - 0.5F;				  
				r = BP_PS2_rand() % 3;
				if(r & 1) { tmp = u[0]; u[0] = u[1]; u[1] = tmp; }
				if(r & 2) { tmp = v[0]; v[0] = v[1]; v[1] = tmp; }
				
				/* スプライト書き込み */
				prim = DG_SetDmapackSprt( prim, 
										  0.0f, y_base, u[0], v[0], 
										  w   , y_base + VFAST_HEIGHT_NOISE, u[1], v[1], DG_MakeDmaPackColorFromInt(col) );
			}
			
		}

		prim = DG_SetDmapackEnd( prim );
	}
}

static void DieVFast(Work * work)
{
	/* [0] と [1] は連続したバッファ */
	codecDelayedFree(work->params.vfast.packet[0]);
	DG_FreeLinerTexture( work->params.vfast.ltex );
}


/* 
 * ゴーストノイズ。結構簡単。
 *
 * これは、山肌などに反射した映像波が,同じ受像機に受け取られることにより、
 * 像が二重映しになる現象。
 */

static void ghost_set_vertex(Work * work, ghost_trans * pack)
{
	float base_u, base_v;
	float base_x, base_y;
	float left, right;
	float v, y, delay;
	int w, h;

	delay  = work->params.ghost.delay + ((BP_PS2_rand() >> 16) % 3) - 2;

	w = DG_Chanls[work->chanl].width;
	h = DG_Chanls[work->chanl].height;

	base_u = DRAW_WIDTH  / 2 + (DG_Chanls[work->chanl].offset_x - w / 2);
	base_v = DRAW_HEIGHT / 2 + (DG_Chanls[work->chanl].offset_y - h / 2);

	{
		void			*prim ;
		unsigned int	col = 0x30808080;
		float chanlw, chanlh, offset_x, offset_y ;
		float x0, y0, x1, y1 ;
		prim = work->dmapack->autopacket ;
		prim = DG_SetDmapackBackupFrame( prim, 0 ); /* 描画中フレームバッファを待避バッファへ */
		prim = DG_SetDmapackUseFrameTex( prim, 2 );	/* 待避バッファをテクスチャとして使用する宣言 */
		/*		
		   prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0) );
		   PS2版はバグっていた(regにdataの値をいれていた)ので、修正
		   */
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0) );/* 通常半透明 */


		prim = DG_SetDmapackSprt(prim, 0.0f + (float)delay, 0.0f, 
								 DG_FRAME_U( base_u + 0.5f ), DG_FRAME_V( base_v + 0.5f ),
								 w + (float)delay, h,
								 DG_FRAME_U( base_u + w - 0.5f ), DG_FRAME_V( base_v + h - 0.5f ), 
								 DG_MakeDmaPackColorFromInt(col) );
		prim = DG_SetDmapackEnd( prim );
	}	
}



static void InitGhost(Work * work)
{
	int i;
	work->params.ghost.delay = 5 + ((BP_PS2_rand() >> 16) % 3);


	/*
	 * 基礎部分の用意
	 */
	if ( !(work->params.sand.packet[0] = codecMalloc( 256 )) ) {
		printf("in 'cdc_noise.c' cannot make memory!\n");
		ASSERT( 0 );
	}
	work->dmapack->autopacket = work->params.sand.packet[0];
	DG_SetDmapackEnd( work->params.sand.packet[0] );
}

static void ActGhost(Work * work)
{
	ghost_set_vertex(work, work->params.ghost.packet[ DG_Clock ]);
}


static void DieGhost(Work * work)
{
	/* [0] と [1] は連続したバッファ */
	codecDelayedFree(work->params.ghost.packet[0]);
}


/*
 * 垂直同期ずれ
 *
 * 垂直同期ずれにより、像が上下方向にスクロールして見える。
 */
static void unsyncV_set_vertex(Work * work, unsyncV_trans * pack)
{
	float w, h;
	float base_u, base_v;
	float base_x, base_y;
	float begin_y, end_y;

	w = DG_Chanls[work->chanl].width;
	h = DG_Chanls[work->chanl].height;

	base_u = DRAW_WIDTH  / 2 + (DG_Chanls[work->chanl].offset_x - w / 2);
	base_v = DRAW_HEIGHT / 2 + (DG_Chanls[work->chanl].offset_y - h / 2);

	base_x = 2048.0F - w / 2;
	base_y = 2048.0F - h / 2;

	/*
	 * 黒帯に区切られた下半分(元の絵の上半分)
	 */
	end_y = h - work->params.unsyncV.gap;
	begin_y = 0.0F;
	pack->dma.gif1[0].uv0 = SCE_GS_SET_UV(16.0F * base_u,
										  16.0F * (base_v + begin_y));
	pack->dma.gif1[0].uv1 = SCE_GS_SET_UV(16.0F * (base_u + w),
										  16.0F * (base_v + end_y));

	begin_y = work->params.unsyncV.gap;
	end_y = h;
	pack->dma.gif1[0].xyz0 = SCE_GS_SET_XYZ(16.0F * base_x,
											16.0F * (base_y + begin_y), -1);
	pack->dma.gif1[0].xyz1 = SCE_GS_SET_XYZ(16.0F * (base_x + w),
											16.0F * (base_y + end_y), -1);


	/*
	 * 黒帯に区切られた上半分(元の絵の下半分)
	 */
	begin_y = h - work->params.unsyncV.gap + UnsyncV_HEIGHT;
	end_y = h;

	pack->dma.gif1[1].uv0 = SCE_GS_SET_UV(16.0F * base_u,
										  16.0F * (base_v + begin_y));
	pack->dma.gif1[1].uv1 = SCE_GS_SET_UV(16.0F * (base_u + w),
										  16.0F * (base_v + end_y));

	begin_y = 0.0F;
	end_y = work->params.unsyncV.gap - UnsyncV_HEIGHT;
	pack->dma.gif1[1].xyz0 = SCE_GS_SET_XYZ(16.0F * base_x,
											16.0F * (base_y + begin_y), -1);
	pack->dma.gif1[1].xyz1 = SCE_GS_SET_XYZ(16.0F * (base_x + w),
											16.0F * (base_y + end_y), -1);

	/*
	 * 黒帯
	 */
	pack->dma.gif2.xyz0 = SCE_GS_SET_XYZ(16.0F * base_x,
										 16.0F * (base_y + end_y), -1);

	pack->dma.gif2.xyz1 =
	  SCE_GS_SET_XYZ(16.0F * (base_x + w),
					 16.0F * (base_y + end_y + UnsyncV_HEIGHT), -1);
}

static void InitUnsyncV(Work * work)
{
}

static void ActUnsyncV(Work * work)
{
	float max;

	unsyncV_set_vertex(work, work->params.unsyncV.packet[ DG_Clock ]);
	work->params.unsyncV.gap -= work->params.unsyncV.speed;

	max = DG_Chanls[work->chanl].height + UnsyncV_HEIGHT;
	if(work->params.unsyncV.gap >= max) work->params.unsyncV.gap -= max;
	if(work->params.unsyncV.gap < 0)  work->params.unsyncV.gap += max;
}

static void DieUnsyncV(Work * work)
{
	codecDelayedFree(work->params.unsyncV.packet[0]);
}


/*
 * 頂点設定
 */
static void bw_vert_set(Work * work, bigwave_trans * pack)
{
	float base_u, base_v;		/* UV 値の左上にあたる値  */
	float base_x, base_y;		/* XYZ 値の左上にあたる値 */
	float b;
	float x[2], y;
	float u[2], v;
	float width, height;
	int l, vt, blight;
	u_long64 rgbaq;


	printf("pack = %p\n", pack);


	width  = (float)DG_Chanls[work->chanl].width;
	height = (float)DG_Chanls[work->chanl].height;

	/* 現在の画面の大きさに合わせる */
	base_x = 2048.0F - width / 2.0F;
	base_y = 2048.0F - height / 2.0F;

	base_u = DRAW_WIDTH  / 2 + (DG_Chanls[work->chanl].offset_x - width / 2.0F);
	base_v = DRAW_HEIGHT / 2 + (DG_Chanls[work->chanl].offset_y - height / 2.0F);

	blight = (int)(work->params.bigwave.bias * 128.0F) + 128;
	rgbaq = SCE_GS_SET_RGBAQ(blight, blight, blight, blight, 0);

	u[0] = base_u;
	u[1] = base_u + width;

	for(l = 0; l < 5; l++)
	  {
		  b = work->params.bigwave.bias * width;
		  if(l & 1) b = -b;		/* 奇数ラインでは、シフト方向が逆になる */

		  vt = l * 2;

		  y = height * (float)l / 5.0F + base_y;
		  v = base_v + height * (float)l / 5.0F;

		  x[0] = base_x + b;
		  x[1] = base_x + b + width;

		  pack->dma.gif1.vert[vt].xyz     = SCE_GS_SET_XYZ(x[0] * 16, y * 16, -1);
		  pack->dma.gif1.vert[vt + 1].xyz = SCE_GS_SET_XYZ(x[1] * 16, y * 16, -1);
		  pack->dma.gif1.vert[vt].uv      = SCE_GS_SET_UV(u[0] * 16 + 8, v * 16);
		  pack->dma.gif1.vert[vt + 1].uv  = SCE_GS_SET_UV(u[1] * 16 - 8, v * 16);

		  pack->dma.gif1.vert[vt + (l & 1)].rgbaq = rgbaq;
		  pack->dma.gif1.vert[vt + 1 - (l & 1)].rgbaq =
			SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);
	  }
}

/*
 * 大規模な同期ずれノイズ。
 *
 * 画面がわしっと壊れる。(どんな表現やそれは)
 */
static void InitBigWave(Work * work)
{
	{
		int packsize;

		packsize = sizeof(DG_DMAPACK_PARAM)*2 + sizeof(DG_DMAPACK_ALPHA) + sizeof(DG_DMAPACK_TRIANGLESTRIP)
		  + sizeof(DG_DMAPACK_VERTEX)*10 + sizeof(DG_DMAPACK_TAG);

		work->params.bigwave.bias = 0.0F;
		work->params.bigwave.cnt  = 0;
		if ( !(work->params.bigwave.packet[0] = codecMalloc( packsize )) ) {
			printf("in 'cdc_noise.c' cannot make memory!\n");
			ASSERT( 0 );
		}
		work->dmapack->autopacket = work->params.bigwave.packet[0];
		DG_SetDmapackEnd( work->params.bigwave.packet[0] );
	}
}

static void ActBigWave(Work * work)
{
	float cnt;
	float bias;

	cnt = (float)work->params.bigwave.cnt * F_PI / 30.0F;
	if((work->params.bigwave.cnt += TIME_BASE) >= 30)
	  work->params.bigwave.cnt -= 30;

	//cnt = (float)work->noise_time * F_PI / (float)work->noise_len;

	bias = vu0_Sin(cnt) + vu0_Sin(cnt * 2.0F);
	work->params.bigwave.bias = bias / 4;
	printf("bias = %8.3f\n", work->params.bigwave.bias);

	{
		void *prim;
		prim = work->dmapack->autopacket ;

		prim = DG_SetDmapackBackupFrame( prim, 0 ); /* 描画中フレームバッファを待機バッファへ */
		prim = DG_SetDmapackUseFrameTex( prim, 2 );	/* 待機バッファをテクスチャとして使用する宣言 */
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 2, 2, 1, 64) );
		prim = DG_SetDmapackTriangleStrip( prim, 10 );/* 頂点数分設定必要!! */
		/* 頂点の設定 */
		{
			float base_u, base_v;		/* UV 値の左上にあたる値  */
			float b;
			float x[2], y;
			float u[2], v;
			float width, height;
			int l, vt, blight;
			unsigned int rgbaq, rgbaq0, rgbaq1;
			
			width  = (float)DG_Chanls[work->chanl].width;
			height = (float)DG_Chanls[work->chanl].height;

			/* 現在の画面の大きさに合わせる */
			base_u = DRAW_WIDTH  / 2 + (DG_Chanls[work->chanl].offset_x - width / 2.0F);
			base_v = DRAW_HEIGHT / 2 + (DG_Chanls[work->chanl].offset_y - height / 2.0F);
			
			blight = (int)(work->params.bigwave.bias * 128.0F) + 128;
			rgbaq = blight | ( blight << 8 ) | ( blight << 16 ) | ( blight << 24 );
			
			u[0] = base_u;
			u[1] = base_u + width;
			
			for(l = 0; l < 5; l++){
				b = work->params.bigwave.bias * width;
				if(l & 1) b = -b;		/* 奇数ラインでは、シフト方向が逆になる */
				
				vt = l * 2;
				  
				y = height * (float)l / 5.0F ;
				v = base_v + height * (float)l / 5.0F;
				
				x[0] = b;
				x[1] = b + width;
				
				if( l & 1 ){
					rgbaq0 = 128 | ( 128 << 8 ) | ( 128 << 16 ) | ( 128 << 24 );
					rgbaq1 = rgbaq;
				} else {
					rgbaq0 = rgbaq;
					rgbaq1 = 128 | ( 128 << 8 ) | ( 128 << 16 ) | ( 128 << 24 );
				}					
									
				prim = DG_SetDmapackVertex( prim, x[0] , y, DG_FRAME_U(u[0]+0.5f), DG_FRAME_V( v ), DG_MakeDmaPackColorFromInt(rgbaq0) );
				prim = DG_SetDmapackVertex( prim, x[1] , y, DG_FRAME_U(u[1]-0.5f), DG_FRAME_V( v ), DG_MakeDmaPackColorFromInt(rgbaq1) );
			}
		}

		prim = DG_SetDmapackEnd( prim );

	}
}

static void DieBigWave(Work * work)
{
	codecDelayedFree(work->params.bigwave.packet[0]);
}

/*
 * 水平同期ずれ
 *
 * 水平同期ずれで、一部ラスタが左右方向にずれて見える
 */
static void ActUnsyncH(Work * work)
{



}

/*
 * カラーバースト信号不良による色の不良
 *
 * たまにカラー放送なのに色が抜ける現象
 */
static void ActUnColor(Work * work)
{



}



/* ------------------------------------------------------------------------- */


/*
 * デジタルブロックノイズ
 *
 * デジタル映像風の転送エラーによるデコード不良ノイズ
 */
static void ActBlock(Work * work)
{


}

/*
 * フレーム落ち
 *
 * PC での動画再生時などにおこる、途中フレームの欠落
 */
static void ActFrameOut(Work * work)
{



}



static void set_noise_type(Work * work, int type)
{
	static void (*func_list[])(Work *work) = {
		ActSand,
		ActVFast,
		NULL, //ActGhost,
		ActBigWave, /* 以下未使用 */

		ActUnsyncV, /* 未使用 */

		ActUnsyncH, /* 未使用 */
		ActUnColor, /* 未使用 */

		ActBlock,   /* 未使用 */
		ActFrameOut,/* 未使用 */
	};
	static void (*die_list[])(Work * work) = {
		DieSand,
		DieVFast,
		NULL, //DieGhost,				/* オバケが死ぬとはまた滑稽な。 */
		DieBigWave,

		NULL, // DieUnsyncV,

		NULL,
		NULL,

		NULL,
		NULL,
	};    
	static void (*init_func[])(Work * work) = {
		InitSand,
		InitVFast,
		NULL, //InitGhost,
		InitBigWave,
	   
		NULL, // InitUnsyncV,
	   
		NULL,					/* InitUnsyncH */
		NULL,					/* InitUnColor */
	   
		NULL,					/* InitBlock   */
		NULL,					/* InitFrameOut */
	};
	static int chanl_masks[] = {
		DG_DMAPACK_INVISIBLE0,
		DG_DMAPACK_INVISIBLE1,
		DG_DMAPACK_INVISIBLE2,
		DG_DMAPACK_INVISIBLE3,
		DG_DMAPACK_INVISIBLEMENU
	};

    
	work->dmapack = NULL;
	if((type < 0) || (type >= CDC_NOISE_MAX)) return;


	/* とりあえず転送用の DMAPACK を用意する */
	work->dmapack = DG_MakeDmapack2(DG_DMAPACK_PRIVILEGE |
								DG_DMAPACK_NORMAL | ALL_MASK,
									DG_DMAPACK_PHASE_AFTER, 150);
	work->dmapack->flag &= ~chanl_masks[ work->chanl ];

	if(NULL != init_func[type])
	  {
		  work->step = 0;
		  work->act_func = func_list[type];
		  work->die_func = die_list[type];
		  work->noise_type = type;

		  (init_func[type])(work);
		  DG_QueueDmapack(work->dmapack);

	  }
	else
	  {
		  work->act_func = NULL;
		  work->die_func = NULL;
		  DG_FreeDmapack(work->dmapack); /* 使っていないので開放 */
		  work->dmapack = NULL;
	  }
}

static void Act(Work * work)
{
	if(NULL != work->act_func) (work->act_func)(work);

	work->noise_time += TIME_BASE;
	if(work->finish)
	  {

		  /* 終了所要時間を過ぎていたら自殺 */
		  if(work->finish_time >= FINISH_TIME)
			GV_DestroyActor(work);
		  else
			work->finish_time += TIME_BASE;
	  }
	else
	  {
		  if(work->noise_len > 0)
			if(work->noise_time > work->noise_len)
			  {
				  work->finish = 1;
				  work->finish_time = work->noise_time - work->noise_len;
			  }
		
	  }
}


static void Die(Work * work)
{
	if(work == now_work[work->side]) now_work[work->side] = NULL;

	if(NULL != work->dmapack)
	  {
		  DG_DequeueDmapack(work->dmapack);
		  DG_FreeDmapack(work->dmapack);
	  }
	if(NULL != work->die_func) (work->die_func)(work);
	noise_cnt[work->side]--;

}

static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int ret = 0;
	switch(signal)
	  {
		case CDC_SIGNAL_NOISE_KILL:
		  printf("CDC_SIGNAL_NOISE_KILL\n");
		  work->finish = 1;
		  //      work->finish_time = 0;
		  work->finish_time = FINISH_TIME;		// DEBUG BY K.Uehara 01/08/12
			break;
		case CDC_SIGNAL_VFAST_SE_STOP:
		  printf("CDC_SIGNAL_VFAST_SE_STOP\n");
		  if(work->noise_type == CDC_NOISE_VFAST)
			work->params.vfast.vfast_se = 0;
		  break;
		case CDC_SIGNAL_VFAST_KILL:
		  printf("CDC_SIGNAL_VFAST_KILL\n");
		  /* 早送りだったら死ぬ */
		  if(work->noise_type == CDC_NOISE_VFAST)
			{
				work->finish = 1;
				//	  work->finish_time = 0;
				work->finish_time = FINISH_TIME;		// DEBUG BY K.Uehara 01/08/12
			}
		  break;
		case CDC_SIGNAL_KILL_EXIST_SAND:
		  printf("CDC_SIGNAL_KILL_EXIST_SAND\n");
		  if(work->noise_type != CDC_NOISE_SAND)
			{
				work->finish = 1;
				work->finish_time = FINISH_TIME;
			}
		  break;
		case CDC_SIGNAL_KILL_EXIST_VFAST:
		  printf("CDC_SIGNAL_KILL_EXIST_VFAST\n");
		  if(work->noise_type != CDC_NOISE_VFAST)
			{
				work->finish = 1;
				work->finish_time = FINISH_TIME;
			}
		  break;
		default:
		  ret = GV_DefaultSignalFunc(workp, signal, value);
		  break;
	  }
	return ret;
}

static int GetResources(Work * work, int side, int type, int t_len)
{
	work->side = side;
	work->chanl = side + 2;		/* 無線の顔は、左が 2, 右が 3 ch. */
	work->noise_time = 0;
	work->noise_len = t_len;
	work->finish = 0;

	set_noise_type(work, type);  

	GV_SetActorSignalFunc(work, ReceiveSignal);

	now_work[side] = work;
	noise_cnt[side] ++;

	// printf("noise cnt[0]: %d\n", noise_cnt[side]);

	return 0;
}

void * NewCodecNoise(int side, int type, int t_len)
{
	Work * work;

#if 0
	if(now_work[side] != NULL)
	  {
		  /*
		   * 既に動作しているノイズの優先度が新しいものより小さければ(高ければ)
		   * 古いものをキャンセルして新しいものを有効にする。
		   */
		  if(type < (now_work[side]->noise_type))
			{
				now_work[side]->finish = 1;
				now_work[side]->finish_time = 0;
			}
		  else
			return NULL;		/* 以前のものを残す */
	  }
#endif

	if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xef)))
	  return NULL;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));

	// printf("Noise Side = %d\n", side);
	if(GetResources(work, side, type, t_len))
	  {
		  GV_DestroyActor(work);
		  return NULL;
	  }

	return work;
}
