//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
  photo_pic.c
  写真転送端末の、転送画像操作用緩衝ライブラリ。

  2001/05/02  Y,Kira
  $Id: photo_pic.c,v 1.2 2002/12/19 14:06:02 takaki Exp $
*/
#endif
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

#include "gameheader.h"

#include "g_define.h"
#include "font.h"
#include "libfs.h"
#include "libmt.h"
#include "libutl.h"
#include "libdg.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"

#include "sprite_2d.h"
#include "photo_config.h"
#include "photo_pic.h"
#include "jpeg.h"

#include "../../skoba/equip/capture.h"

#include "BP_EndianSupport.h"

#ifdef _DEBUG_
#define DBG printf
#else
#define DBG
#endif  /* _DEBUG_ */

#define RESIDENT_NAME  0x1f6af7a3

/*
 * 圧縮状態の画像データの開始ポインタを、撮影コマ番号で得る。
 * エラー時(不正なコマ番号や、そのコマに画像がない)には NULL を返す。
 */
PIC_PTR * PIC_GetPicture(int num)
{
  char * resident = GV_GetCache( RESIDENT_NAME );
  PIC_PTR * ptr;


  printf("picture number = %d\n", num);
  ASSERT((num >= 0) && (num < TANKER_MAX_CAPTURE));
  if(num >=TANKER_MAX_CAPTURE) return NULL;
  if(num < 0) return NULL;

  /*
   * ここで取得
   */
  ptr = resident + ( THUMB_W * THUMB_H * sizeof(short) * TANKER_MAX_CAPTURE)
    + TANKER_MAX_CODE * num;

  return ptr;
}

/*
 * 圧縮状態の画像を、指定した領域に展開する。
 */
unsigned int * PIC_DecodePicture(unsigned int * buf, PIC_PTR * pic)
{
  ASSERT(!((int)buf & 0x0f));  /* 展開先が 16 バイト境界でなければエラー */

  /*
   * ここで展開
   */
  {
    int i;
    printf("decode :");
    for(i = 0; i < 16; i++)
      printf(" %02x", ((char *)pic)[i]);
    printf("\n");
  }

printf( "%08x Get From HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", pic ) ;

  UTL_JpegDecode(buf, DRAW_WIDTH, DRAW_HEIGHT, pic);

  return buf;
}


/*
 * 画像を1/4 サイズに縮小する
 */
static int shrink_picture(unsigned short * shrink, unsigned int * raw)
{
  int sx, sy;
  int x, y;
  int r, g, b;
  unsigned int * src;
  unsigned short * dst;
  
  unsigned int rawPixel;

  printf("shrink buf = %p\n", shrink);
  for(sy = 0; sy < PHOTO_TEX_HEIGHT; sy++)
    {
      y = sy * 2;
      for(sx = 0; sx < PHOTO_TEX_WIDTH; sx++)
	{
	  x = sx * 2;

	  src = raw + y * DRAW_WIDTH + x;
	  dst = shrink + sy * PHOTO_TEX_WIDTH + sx;

     {
        rawPixel = *src;
        BP_LE_SwapSInt_Inp(&rawPixel);
        r = rawPixel & 0xff;
        g = rawPixel & 0xff00;
        b = rawPixel & 0xff0000;
     }

     {
        rawPixel = *(src+1);
        BP_LE_SwapSInt_Inp(&rawPixel);
        r += rawPixel & 0xff;
        g += rawPixel & 0xff00;
        b += rawPixel & 0xff0000;
     }

     {
        rawPixel = *(src+DRAW_WIDTH);
        BP_LE_SwapSInt_Inp(&rawPixel);
        r += rawPixel & 0xff;
        g += rawPixel & 0xff00;
        b += rawPixel & 0xff0000;
     }

     {
        rawPixel = *(src+DRAW_WIDTH+1);
        BP_LE_SwapSInt_Inp(&rawPixel);
        r += rawPixel & 0xff;
        g += rawPixel & 0xff00;
        b += rawPixel & 0xff0000;
     }

	  r = (r >> 2) & 0xff;
	  g = (g >> 2) & 0xff00;
	  b = (b >> 2) & 0xff0000;

	  if(r > 0xff)     r = 0xff;
	  if(g > 0xff00)   g = 0xff00;
	  if(b > 0xff0000) b = 0xff0000;

	  r = (r >> 3) & 0x1f;
	  g = (g >> 6) & 0x03e0;
	  b = (b >> 9) & 0x7c00;

#ifdef PSX2
	  *dst = (r | g | b) ;
#else
	  *dst = (r | g | b)|0x8000 ;
#endif
	}
    }

  return 0;
}

/*
 * 展開後の画像のポインタを、PictDraw 構造体に設定する
 */
#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)
#define ON  1
#define OFF 0
#define STQ 0
#define UV  1

#define PRIM_SHADE     OFF
#define PRIM_TEX       ON
#define PRIM_FOG       OFF
#define PRIM_ALPHA     ON
#define PRIM_ANTIALIAS OFF
#define PRIM_TEXEL     UV

PictDraw * PIC_SetPictDraw(PictDraw * draw,
			   unsigned short * shrink, unsigned int * buf)
{
  static PictDMA dma_master ;

  int i;
  int w;

  /* 画像を縮小 */
  printf("<-------- A -------->\n");
  //shrink_picture(shrink, buf);
  printf("<-------- b -------->\n");

  {
	  DG_TEX_LIN *tex ;
	  unsigned int *s ;
	  unsigned int *p ;
     int r, g, b;

	  //tex = DG_MakeLinerTexture( PHOTO_TEX_WIDTH, PHOTO_TEX_HEIGHT, DG_TEXLIN_FORMAT_X1R5G5B5 );
     tex = DG_MakeLinerTexture( DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_A8R8G8B8 );
	  s = ( unsigned int * )buf     ;
	  p = ( unsigned int * )tex->image ;
	  for( i=DRAW_WIDTH*DRAW_HEIGHT ; --i>=0 ; )
     {
        unsigned int data = *s;
		  //*p = data|0xff000000;

//         unsigned int argb = ((data&0x0000ff00)<<16) | ((data&0xff000000)>>16) | ((data&0x00ff00ff));
//         unsigned int rgba = ((argb&0x000000ff)<<24) | ((argb&0xffffff00)>>8);

        *p = data;

        ++p;
        ++s;
	  }
     tex->image_dirty = 1;
	  return tex ;
  }
}
/*2002.07.24 sigeno追加****************************************/
/*シナリオから指定番号のタンカーデジカメ画像をセーブ領域に退避*/
void PIC_TankerPicSaveScn(void){
	int num ;
	void * pic;
	u_int *pTmp;

	num = GCL_GetNextInt();
	printf("SAVE PHOTO NUM %d\n",num);
	pic = PIC_GetPicture(num);
	ASSERT(GM_TankerPicture != NULL ) ;
	ASSERT(pic != NULL ) ;
#if 1 // BP_RENDER
   // Our pitcure are stored in render targets that stick around for the life of the application
   // all we need to know is which pitcureNum to use
   memcpy(GM_TankerPicture, &num, sizeof(num));
#else
	memcpy(GM_TankerPicture, pic, GM_TANKER_PICTURE_SIZE);


	/*画像データ取得*/
	pTmp = ( u_int * )GM_TankerPicture;

	if ( ( pTmp[ 0 ] + pTmp[ 1 ] == 0 ) || ( pTmp[ 0 ] + pTmp[ 1 ] > 16 * 1024 ) ){ // データが無いまたは不正
		printf("DataSize = %d + %d = %d\n" , pTmp[ 0 ] , pTmp[ 1 ] , pTmp[ 0 ] + pTmp[ 1 ] );
		printf("TANKER PHOTO SAVE FAILED!!! \n");
//		ASSERT(0) ;
	}else {
		printf("TANKER PHOTO SAVE SUCCESS!!! \n");
	}
#endif
}
/***************************************************************/
