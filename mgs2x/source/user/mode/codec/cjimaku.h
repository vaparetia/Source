/*
	cjimaku.h
		無線機用字幕表示(簡易版)
	2000/07/18	K.Uehara
	$Id: cjimaku.h,v 1.1.1.3 2002/11/19 11:44:59 Yoshizawa1 Exp $
*/
#ifndef _cjimaku_h_
#define _cjimaku_h_

struct textarea_packet;

#include "BP_BuildDefines.h"
#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Font.h"
#endif

typedef struct cjimaku_CODEC_MESG_PANEL 
{
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   struct _SBP_BufferedTexture* pbp_BufferedTexture;
   void* bp_packTextureLocation;
#else
   FONT_VRAMINFO vinfo;
   void *vram;            /* 文字表示エリアイメージ           */
   int vramsize;          /* 表示エリアイメージデータのサイズ */
   DG_TEX_LIN *tex;
   int clut[ 16 ];
#endif

   int width, height;     /* 表示エリアのピクセル単位サイズ   */

  DG_DMAPACK * dmapack;  /* 表示エリアイメージをテクスチャとして転送し、
			    所定位置に描画するDMAパケットの管理DMAPACK構造体 */

  void *packet;

  int    panel_dx;       /* パネルの左端位置 */
  int    panel_dy;       /* パネルの上下位置 */
  int    panel_w;
  int    panel_h;

} CODEC_MESG_PANEL;

void CODEC_OpenMesgPanel( CODEC_MESG_PANEL *panel, int width, int height );
void CODEC_CloseMesgPanel( CODEC_MESG_PANEL *panel );
void CODEC_DrawSelect( CODEC_MESG_PANEL * panel,
		       int sel, int total, int num, char * msg);
void CODEC_DrawMessage( CODEC_MESG_PANEL *panel, char *mesg );
void CODEC_DrawMesgPanel( CODEC_MESG_PANEL *panel, int x, int y );   // center
void CODEC_ShowMesgPanel(CODEC_MESG_PANEL * panel);
void CODEC_HideMesgPanel(CODEC_MESG_PANEL * panel);



#endif /* _cjimaku_h_ */
