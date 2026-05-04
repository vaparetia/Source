/*
	clearcode.h
		クリアコード表示関連

	2001/08/09  K.Kano
	$Id: clearcode.h,v 1.2 2002/12/04 10:29:00 takaki Exp $
 */


#ifndef _clearcode_h_
#define _clearcode_h_

#ifdef TOOL
#include	<stdio.h>
#include	<assert.h>

typedef unsigned char u_char;
typedef unsigned int u_int;

#ifndef TRUE
#define TRUE 	(-1)
#define FALSE 	0
#endif


#define GV_ZeroMemory( p, size )	bzero( p, size )
#define ASSERT	assert
#define irnd rand

#else	// TOOL

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"

#include "../mcman/mcman.h"

#include "../resman/resman.h"

#include "mode/menu/xtextscn.h"

/* text_scn.c */

/* プロトタイプ宣言 */
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
void MENU_ClearPartTextTexture( void *work, int start_line, int height );
/* 説明文を表示する */
void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
void *NewTextScreenControl( void );
void *NewTextScreenControlForTitle( void );
void *NewTextScreenControlForCodec( void );


/* sk_printf.c */

extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え


/* clearcode.c */
void EncodeClearCode(unsigned int *code);
void CalcBitTo26Code(unsigned int *in,unsigned char *out);


#define SCN_SCODE_BITLEN	110
#define SCN_CODE_BITLEN		122
#define SCN_CODE_LEN		((SCN_CODE_BITLEN+32-1)/32)
#define SCN_CODE_LEN_S		((SCN_CODE_BITLEN+16-1)/16)
#define SCN_CODE_LEN_C		((SCN_CODE_BITLEN+8-1)/8)
#define CLR_CODE_LEN		26

#define MAX_CLR_CODE_LEN	32


#define DISP_CHANL			4
#define DISP_PRIORITY		0


#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */

#define CODE_L2D_CLEAR_CODE		0x00f28a1b		/* clear_code */

#define CODE_OPEN_TNK_CODE_WIN	0x00dc6199		/* openCodeTnk */
#define CODE_OPEN_PLT_CODE_WIN	0x008a2c0f		/* openCodePlnt */
#define CODE_OPEN_TP_CODE_WIN	0x00c7844d		/* openCodeTnkPlnt */
#define CODE_CLOSE_CODE_WIN		0x00d9c591		/* closeCode */

#define CODE_OPEN_RESULT		0x00dea62c		/* openResult */
#define CODE_CLOSE_RESULT		0x007e1578		/* closeResult */

#define CODE_TYPETEST			0x00e31354		/* typeCodeTest */
#define CODE_TESTBLINK			0x005f3734		/* blinkDot_test */
#define CODE_BLINKDOT			0x0019ca2d		/* blinkDot */

#define CODE_GET_NEW_ITEM			0x00b08dff		/* NewItemMsg */
#define CODE_OPEN_ITEM_BANDANA		0x00f01b1c		/* openItemBnd */
#define CODE_CLOSE_ITEM_BANDANA		0x00de0490		/* closeItemBnd */
#define CODE_OPEN_ITEM_CAMERA		0x00f01d85		/* openItemCam */
#define CODE_CLOSE_ITEM_CAMERA		0x00de06f9		/* closeItemCam */
#define CODE_OPEN_ITEM_STELTH		0x00f05d85		/* openItemSam */
#define CODE_CLOSE_ITEM_STELTH		0x00de46f9		/* closeItemSam */
#define CODE_OPEN_ITEM_WIG1			0x000dd02f		/* openItemWig1 */
#define CODE_CLOSE_ITEM_WIG1		0x00cafeac		/* closeItemWig1 */
#define CODE_OPEN_ITEM_WIG2			0x000dd030		/* openItemWig2 */
#define CODE_CLOSE_ITEM_WIG2		0x00cafead		/* closeItemWig2 */
#define CODE_OPEN_ITEM_WIG3			0x000dd031		/* openItemWig3 */
#define CODE_CLOSE_ITEM_WIG3		0x00cafeae		/* closeItemWig3 */

#define CODE_OPEN_SAVEMSG		0x000ea7f6		/* openSave */
#define CODE_CLOSE_SAVEMSG		0x00e18fd1		/* closeSave */

#define CODE_NODE_FONT			0x00965c04		/* node_font */

#define CODE_NODE_DOT			0x002c1e9a		/* dot_null */

#define CODE_SHOW_ITEM_NAME		0x009c44ea		/* showName */
#define CODE_HIDE_ITEM_NAME		0x007bed13		/* hideName */


#define SE_SEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
// #define SE_START()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01)
#define SE_START()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define SE_OK()			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define SE_CANCEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02)


#define SE_DISP_A_FONT()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING01)
#define SE_DISP_FINISH()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING02)

#define SE_GET_NEWITEM()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_N_START1)


/* ウィンドウ開閉音 */
#define SE_WINOPEN()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPN01)
#define SE_WINCLOSE()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLS01)

#endif	// TOOL

/////////////////////
// 可変長クリアコード

#define CODE_CRC_LEN	7
#define CODE_RND_LEN	5
#define CODE_ADD_LEN	( CODE_CRC_LEN + CODE_RND_LEN )

#define CODE_MAX_BYTE	64

typedef struct CODE_BIT_DATA {
	u_char	data[ CODE_MAX_BYTE ];	// データ
	int		bitlen;					// 長さ（ビット単位）
} CODE_BIT_DATA;
extern void ClearCodeEncodeEx( CODE_BIT_DATA* pDst, const CODE_BIT_DATA* pSrc, int rnd_code );
extern int ClearCodeDecodeEx( CODE_BIT_DATA* pDst, const CODE_BIT_DATA* pSrc );
extern void ClearCodeChangeRadix( u_char* pDst, int dst_len, int dst_radix,
								  const u_char* pSrc, int src_len, int src_radix );

extern int ClearCodeAddRedundantBit( CODE_BIT_DATA* p, int dst_len, int dst_radix, int seed );

extern u_int ClearCodeCalcStringsCrc( char* pstr );

#ifndef KP_WINDOWS
extern int ClearCodeGenerate( u_char* pDst, int code_len, int dst_radix,
							  void* pSrcbit, int bitlen, int seed );
#else
extern int ClearCodeGenerate( int mode, u_char* pDst, int code_len, int dst_radix,
							  void* pSrcbit, int bitlen, int seed );
#endif
/*
  ・2002/08/07 下記エンコード作業をラップしました
   
  int ClearCodeGenerate( u_char* pDst, int code_len, int dst_radix,
  					 	  void* pSrcbit, int bitlen, int seed );
  code_len  希望文字数
  dst_radix 変換後基数(26)
  pSrcbit   入力データ
  bitlen	入力データビット単位長さ
  seed		乱数生成シード( 5 bit のみ有効 )
  
  結果の文字数を返します。pDst に dst_radix 進数の数値が入ります。

  名前データは pSrcbit の中に含めておいてください

・エンコード手順
  データ準備
  CODE_BIT_DATA src;
  CODE_BIT_DATA crypt;
  u_char code[ CODE_LEN ];
  
  src.data にデータを入れる
  src.bit にデータ長（ビット単位）を入れる

  ClearCodeEncodeEx( &crypt, &src, irnd(), irnd() );
  ここでＣＲＣコード付加＋暗号化
  
  ClearCodeChangeRadix( code, CODE_LEN, 26,
  						crypt.data, (crypt.bitlen + 7) / 8, 256 );
  ここで 256進数の crypt.data ( 桁数は (crypt.bitlen + 7) / 8 になる)
  を CODE_LEN の桁数の 26 進数に変換

  最終的に code[n] + 'A' が n 文字目となる
  
・名前の扱い
  名前をコード内に含める場合は、最初の元データビット列に含めて
  しまってください( 上記の src.data の部分 )

  任意の文字列から数値を生成するサービス関数を用意しました。
  u_int ClearCodeCalcStringsCrc( char* pstr );
  32 bit 出すので、適当な長さを下位ビットから切って使ってください
  例）
  src.data[ 0 ] = ClearCodeCalcStringsCrc( "name strings" ) & 0xff;

   
・デコード手順
  マスターでは必要ありませんが、デバッグのために
  作成したデータをデコードして元データと合致するか確かめてもらえるとありがたいです
  手順はエンコードの全く逆で
  ClearCodeChangeRadix( crypt.data, (crypt.bitlen + 7) / 8, 256,
  						code, CODE_LEN, 26 );
  if( ClearCodeDecodeEx( &src2, &crypt ) ) {
  	// src2.data はもとのデータになっているはず
  } else {
  	// FALSE が返ったときはデータが不正(CRC error など)
  }
  
 */

#endif
