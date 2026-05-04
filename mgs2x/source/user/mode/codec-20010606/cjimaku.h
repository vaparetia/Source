/*
	cjimaku.h
		無線機用字幕表示(簡易版)
	2000/07/18	K.Uehara
	$Id: cjimaku.h,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/
#ifndef _cjimaku_h_
#define _cjimaku_h_

struct texarea_packet;

typedef struct {
  FONT_VRAMINFO vinfo;
  void *vram;            /* 文字表示エリアイメージ           */
  int width, height;     /* 表示エリアのピクセル単位サイズ   */
  int vramsize;          /* 表示エリアイメージデータのサイズ */

  DG_DMAPACK * dmapack;  /* 表示エリアイメージをテクスチャとして転送し、
			    所定位置に描画するDMAパケットの管理DMAPACK構造体 */
  struct textarea_packet  * packet;  /* DMA パケット本体 */
  int    panel_dx;       /* パネルの左端位置 */
  int    panel_dy;       /* パネルの上下位置 */
  int    panel_w;
  int    panel_h;

} CODEC_MESG_PANEL;

void CODEC_OpenMesgPanel( CODEC_MESG_PANEL *panel, int width, int height );
void CODEC_CloseMesgPanel( CODEC_MESG_PANEL *panel );

void CODEC_DrawMessage( CODEC_MESG_PANEL *panel, char *mesg );
void CODEC_DrawMesgPanel( CODEC_MESG_PANEL *panel, int x, int y );   // center



#endif /* _cjimaku_h_ */
