//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	packet.c
	定型パケット生成ルーチン

	1999/10/22 K.Takabe
	$Id: packet.c,v 1.1.1.3 2002/11/19 11:42:17 Yoshizawa1 Exp $

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

#define __PRIM_C__
#include	"libdg.h"
#undef __PRIM_C__
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"packet.h"

#define USE_FOG	1
#define USE_ABE	1
#define USE_AA1	0


#define GS_REGS_0()
#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_2(r0, r1) \
((r0) << 0x00 | (r1) << 0x04)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_4(r0, r1, r2, r3) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)
#define GS_REGS_6(r0, r1, r2, r3, r4, r5) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10 | (r5) << 0x14)


/*----------------------------------------------------------------*/



#if 0
/* マイクロプログラムの使用する固定変数群 */
static ALIGN16_PRE float	work_default[32] ALIGN16_POST = {
	/* <0x0>座標クランプ値（最小） */
	//2048.0F-256, 2048.0F-112, 0.0F,-1.0F,
	0, 0, 0.0F,-1.0F,
	/* <0x1>座標クランプ値（最大） */
	//2048.0F+256, 2048.0F+112, 65536.0F*256.0F, 1.0F,
	4095, 4095, 65536.0F*256.0F, 1.0F,
	/* <0x2>フォグパラメータ */
	0.0F, 0.0F, 0.0F, 0.0F,
	/* <0x3>特定用途向け定数（カラークリップ値、Ｚクリップ値、スペキュラー乗算値＆クリップ値） */
	//255.0F, CLIP_NEAR_1, 128.0F/0.2F, 0.8F,
	255.0F, 0.0F, 0.0F/0.2F, 0.8F,
	/* <0x4>プリミティブ座標へのスケーリングパラメータ（描画サイズにより可変） */
	DRAW_WIDTH/2+1, DRAW_HEIGHT/2+1, DRAW_Z_MAX-DRAW_Z_MIN, 0.0F,
	/* <0x5>プリミティブ座標へのオフセット値 */
	2048.0F, 2048.0F, DRAW_Z_MIN, 0.0F,
	/* <0x6>特定用途向け定数２（Ｗ補正値） */
	1.0F/4096.0F, 0.5F, 0.0F, 0.0F,
	/* <0x7> */
	0.0F, 0.0F, 0.0F, 0.0F
} ;
#endif

/* マイクロプログラム読み込みタグ */
#if 0 //BP_PS2
extern qword Vu1DrawObject0 ;
extern qword Vu1DrawObject1 ;
extern qword Vu1DrawObject2 ;
extern qword Vu1DrawObject3 ;
extern qword Vu1DrawShadowObject ;
extern qword Vu1DrawShadowObject2 ;
#define SELECT_VU1_PROGRAM Vu1DrawObject1
/* マイクロプログラム実行アドレスリスト */
extern qword Vu1DrawObject0_Func ;
extern qword Vu1DrawObject1_Func ;
extern qword Vu1DrawObject2_Func ;
extern qword Vu1DrawObject3_Func ;
extern qword Vu1DrawShadowObject_Func ;
extern qword Vu1DrawShadowObject2_Func ;
#define SELECT_VU1_PROGRAM_FUNC Vu1DrawObject1_Func
extern qword	Vu1DrawComdlObject1 ;
extern qword	Vu1DrawComdlObject1_Func ;
#endif

/* ---------------------------------------------------------------- */

	/*
		ＶＩＦ１初期化パケットの書き出し
	*/
void *DG_WritePacket_VIF1Init( void *addr )
{
	/* ＶＵ１ダブルバッファの設定（フレームの最初に一度だけ付ける） */
	static ALIGN16_PRE int		init_buffer_base[] ALIGN16_POST= {
		SCE_VIF1_SET_BASE( 0x40, 0 ),			/* ダブルバッファのベースアドレス */
		SCE_VIF1_SET_OFFSET( 0x1E0, 0 ),		/* ダブルバッファのオフセット */
		SCE_VIF1_SET_STMOD( 0, 0 ),				/* 加算書き込みＯＦＦ */
		SCE_VIF1_SET_STMASK( 0 ), 0x00FFFF10,	/* １サイクルめのＺにROWの値を書き込む（ＵＶデータ用設定） */
		SCE_VIF1_SET_STROW( 0 ),0,0,4096,0,		/* Ｚメンバに4096（小数点以下１２ビットの固定小数点で1.0を表す） */
		SCE_VIF1_SET_MSKPATH3( 0, 0 ),
		SCE_VIF1_SET_NOP(0)
	};
	DG_DMATAG	*tag = addr ;

	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 );
	tag->addr = init_buffer_base;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_NOP(0) ;

	return ( &tag[1] );
}

	/*
		ＶＵ１マイクロプログラム転送パケット書き出し
	*/
void *DG_WritePacket_Vu1ModelProg( void *addr, int param )
{
	u_long128	*tag = addr ;
#if 0 //BP_PS2
	*tag = *(u_long128*)SELECT_VU1_PROGRAM ;
#endif
#if 0
	printf("vu1 micro program size: %08x\n", ((DG_DMATAG*)SELECT_VU1_PROGRAM)->qwc & 0x7fff );
#endif
	return ( &tag[1] );
}

	/*
		ＶＵ１マイクロプログラム転送パケット書き出し
	*/
void *DG_WritePacket_Vu1ModelProgEx( void *addr, int param )
{
	u_long128	*tag = addr ;
#if 0 //BP_PS2
	switch ( param ){
	  case 0:
		*tag = *(u_long128*)Vu1DrawObject2 ;
		break ;
	  case 1:
		*tag = *(u_long128*)Vu1DrawObject3 ;
		break ;
	}
#endif
#if 0
	printf("vu1 micro program size: %08x\n", ((DG_DMATAG*)Vu1DrawObject2)->qwc & 0x7fff );
#endif
	return ( &tag[1] );
}

	/*
		ＶＵ１マイクロプログラム転送パケット書き出し
	*/
void *DG_WritePacket_Vu1ComdlProg( void *addr, int param, int **func_list )
{
	u_long128	*tag = addr ;
#if 0 //BP_PS2
	*tag = *(u_long128*)Vu1DrawComdlObject1 ;
	*func_list = (int*)Vu1DrawComdlObject1_Func ;
#endif
	return ( &tag[1] );
}

	/*
		ＶＵ１マイクロプログラム転送パケット書き出し
	*/
void *DG_WritePacket_Vu1ShadowProg( void *addr, int param )
{
	u_long128	*tag = addr ;
#if 0 //BP_PS2
	*tag = *(u_long128*)Vu1DrawShadowObject2 ;
#endif
#if 0
	printf("vu1 micro program size: %08x\n", ((DG_DMATAG*)Vu1DrawShadowObject)->qwc & 0x7fff );
#endif
	return ( &tag[1] );
}


void *DG_WritePacket_DefaultModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	test ;
			DG_GSREG	tex1 ;
			DG_GSREG	texflush ;
			DG_GSREG	colclamp ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ),SCE_GS_TEST_1},
			{SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0),SCE_GS_TEX1_1},
			{0,SCE_GS_TEXFLUSH},
			{SCE_GS_SET_COLCLAMP(1),SCE_GS_COLCLAMP},
			{SCE_GS_SET_PRMODECONT(1),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 1, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

void *DG_WritePacket_DefaultSemiTransModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	tex1 ;
			DG_GSREG	texflush ;
			DG_GSREG	colclamp ;
			DG_GSREG	zbuf ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0),SCE_GS_TEX1_1},
			{0,SCE_GS_TEXFLUSH},
			{SCE_GS_SET_COLCLAMP(1),SCE_GS_COLCLAMP},
			{SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 1),SCE_GS_ZBUF_1},
			{SCE_GS_SET_PRMODECONT(1),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 1, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

void *DG_WritePacket_MakeShadowModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	frame ;
			DG_GSREG	zbuff ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	xyz0 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	test1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048+4, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_ZBUF( BUFFER_PAGE(2)/2048, Z_BUFFER_COLOR_MODE(), 1),SCE_GS_ZBUF_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 0, 0, 0, 0, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_XYZ2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_XYZ2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 1, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			//{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, 1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

void *DG_WritePacket_WriteShadowModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			//DG_GSREG	zbuf ;
			DG_GSREG	test ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode ;
			DG_GSREG	texflush ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			//{SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048, Z_BUFFER_COLOR_MODE(), 1),SCE_GS_ZBUF_1},
			//{SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 2 ),SCE_GS_TEST_1},
			{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 2 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRMODECONT(1),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 1, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE},
			//{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			//{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, 0, USE_AA1, 0, 0, 0),SCE_GS_PRMODE},
			{0,SCE_GS_TEXFLUSH}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

void *DG_WritePacket_MakeShadowMaskGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	frame ;
			DG_GSREG	zbuff ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	xyz0 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
			DG_GSREG	test1 ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048+4, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_ZBUF( BUFFER_PAGE(2)/2048+0, Z_BUFFER_COLOR_MODE(), 0),SCE_GS_ZBUF_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 0, 0, 0, 0, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_XYZF2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_XYZF2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 2 ),SCE_GS_TEST_1},
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}


void *DG_WritePacket_MakeSpotModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	frame ;
			DG_GSREG	zbuff ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	xyz0 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048+4, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_ZBUF( BUFFER_PAGE(2)/2048, Z_BUFFER_COLOR_MODE(), 1),SCE_GS_ZBUF_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			//{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 0, 0, 0, 0, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_XYZF2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0xffffff, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_XYZF2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0xffffff, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			//{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, 0, 1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

void *DG_WritePacket_MakeSpotShadowModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	frame ;
			DG_GSREG	zbuff ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	xyz0 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	test1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
			DG_GSREG	scissor2 ;		/* テクスチャの端に描画をしないように描画領域を狭める */
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048+4, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_ZBUF( BUFFER_PAGE(2)/2048, Z_BUFFER_COLOR_MODE(), 1),SCE_GS_ZBUF_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 0, 0, 0, 0, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_XYZF2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0xffffff, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_XYZF2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0xffffff, 0 ),SCE_GS_XYZF2},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 1, 0, 1, 2 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			//{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, 0, 1, 0, 0, 0),SCE_GS_PRMODE},
			{SCE_GS_SET_SCISSOR( 3, 252, 3, 252 ),SCE_GS_SCISSOR_1}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}


void *DG_WritePacket_Circle( void *addr, int color )
{
	static struct _packet {
		DG_GIFTAG	giftag0 ;
		struct _gif_data0 {
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
		} data0 ;
		DG_GIFTAG	giftag1 ;
		struct _gif_data1 {
			DG_GSREG	rgba ;
			DG_GSREG	xyz2 ;
		} data1[18] ;
		DG_GIFTAG	giftag2 ;
		struct _gif_data2 {
			DG_GSREG	test ;
		} data2 ;
	} *gif_data ;
	DG_DMATAG	*tag = addr ;
	int		packet_size, i ;
	static u_long64	gif_buffer[32] ;
	static int		init_flag = 0 ;

	packet_size = sizeof(struct _packet)/16 ;
	//packet_size = 3 ;
	packet_size = 4+36+2 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, packet_size );
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	gif_data = (struct _packet*)&tag[1] ;
	gif_data->giftag0.tag = SCE_GIF_SET_TAG(sizeof(struct _gif_data0)/16, 1, 0, 0, 0, 1) ;
	gif_data->giftag0.regs = GS_REGS_1(GS_REGS_AD) ;
	gif_data->data0.prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 0, 0, 0, 0, 0, 0, 0 ) ;
	gif_data->data0.prim.reg = SCE_GS_PRIM ;
	gif_data->data0.prmode.data = SCE_GS_SET_PRMODE( 1, 0, 0, 0, 0, 0, 0, 0 ) ;
	gif_data->data0.prmode.reg = SCE_GS_PRMODE ;
	gif_data->giftag1.tag = SCE_GIF_SET_TAG(18*2, 1, 0, 0, 0, 1) ;
	gif_data->giftag1.regs = GS_REGS_2(GS_REGS_AD, GS_REGS_AD) ;
	gif_data->giftag2.tag = SCE_GIF_SET_TAG(sizeof(struct _gif_data2)/16, 1, 0, 0, 0, 1) ;
	gif_data->giftag2.regs = GS_REGS_1(GS_REGS_AD) ;
	gif_data->data2.test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 2 ) ;
	gif_data->data2.test.reg = SCE_GS_TEST_1 ;

	if ( init_flag == 0 ){
		float	a ;
		for ( i = 0 ; i < 18 ; i++ ){
			a = (float)((i-1)&15) / 8.0f * (float)3.14159265f ; 
			gif_buffer[i] =
			  SCE_GS_SET_XYZ2( (int)(2048.0f+cosf(a)*120)*16, (int)(2048.0f+sinf(a)*120)*16, 0 );
		}
		gif_buffer[0] = SCE_GS_SET_XYZ2( ( 2048 - 0 ) * 16, ( 2048 - 0 ) * 16, 0 ) ;
		init_flag = 1 ;
	}

	for ( i = 0 ; i < 18 ; i++ ){
		//gif_data->data1[i].rgba.data = SCE_GS_SET_RGBAQ( 0, 0, 0, 128, 0x00000000 );
		//gif_data->data1[i].rgba.data = color & SCE_GS_SET_RGBAQ( 255, 255, 255, 0, 0x00000000 );/* ブレンドの場合 */
		gif_data->data1[i].rgba.data = color & SCE_GS_SET_RGBAQ( 0, 0, 0, 128, 0x00000000 );/* 加算の場合 */
		gif_data->data1[i].rgba.reg = SCE_GS_RGBAQ ;
		gif_data->data1[i].xyz2.data = gif_buffer[i] ;
		gif_data->data1[i].xyz2.reg = SCE_GS_XYZ2 ;
	}
	gif_data->data1[0].rgba.data = color ;
	//gif_data->data1[0].rgba.data |= SCE_GS_SET_RGBAQ( 0, 0, 0, 255, 0x00000000 ) ;

	return ( &gif_data[1] ) ;
}

void *DG_WritePacket_MakeSoftTexture( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	texflush ;
			DG_GSREG	frame ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	tex0 ;
			DG_GSREG	clamp ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{0,SCE_GS_TEXFLUSH},
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			{SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64+4*32, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 8, 8, 1, 0,
							 0, 0, 0, 0, 0 ),SCE_GS_TEX0_1},
			{SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 ),SCE_GS_CLAMP_1},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_UV( ( 0+1 ) * 16, ( 0+1 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_UV( ( 256+1 ) * 16, ( 256+1 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

#if 0
	if ( GV_PadData[1].press & PAD_A ){
		gif_data.data.uv0.data = SCE_GS_SET_UV( ( 0 ) * 16+8, ( 0 ) * 16+8 ) ;
		gif_data.data.uv1.data = SCE_GS_SET_UV( ( 256 ) * 16+8, ( 256 ) * 16+8 ) ;
	}
	if ( GV_PadData[1].release & PAD_A ){
		gif_data.data.uv0.data = SCE_GS_SET_UV( ( 0 ) * 16, ( 0 ) * 16 ) ;
		gif_data.data.uv1.data = SCE_GS_SET_UV( ( 256 ) * 16, ( 256 ) * 16 ) ;
	}
#endif

	return ( &tag[1] );
}

void *DG_WritePacket_MakeSoftTexture2( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	frame ;
			DG_GSREG	scissor ;
			DG_GSREG	xyoffset ;
			DG_GSREG	tex0 ;
			DG_GSREG	clamp ;
			DG_GSREG	test0 ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode_def ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0 ),SCE_GS_FRAME_1},
			{SCE_GS_SET_SCISSOR( 0, 255, 0, 255 ),SCE_GS_SCISSOR_1},
			{SCE_GS_SET_XYOFFSET( (2048-128)*16, (2048-128)*16 ),SCE_GS_XYOFFSET_1},
			{SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 8, 8, 1, 0,
							 0, 0, 0, 0, 0 ),SCE_GS_TEX0_1},
			{SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 ),SCE_GS_CLAMP_1},
			{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_UV( ( 256 ) * 16, ( 256 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_UV( ( 0 ) * 16, ( 0 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 1, 0, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

#if 0
	if ( GV_PadData[1].press & PAD_A ){
		gif_data.data.uv0.data = SCE_GS_SET_UV( ( 0 ) * 16+8, ( 0 ) * 16+8 ) ;
		gif_data.data.uv1.data = SCE_GS_SET_UV( ( 256 ) * 16+8, ( 256 ) * 16+8 ) ;
	}
	if ( GV_PadData[1].release & PAD_A ){
		gif_data.data.uv0.data = SCE_GS_SET_UV( ( 0 ) * 16, ( 0 ) * 16 ) ;
		gif_data.data.uv1.data = SCE_GS_SET_UV( ( 256 ) * 16, ( 256 ) * 16 ) ;
	}
#endif

	return ( &tag[1] );
}

#if 0
/* スポットライト用ベースイメージ転送パケット生成 */
void *DG_WritePacket_TransSpotImage( void *addr, void *data )
{
	static struct _packet {
		DG_GIFTAG	giftag1 ;
		struct _gif_data {
			DG_GSREG	bitbltbuf ;
			DG_GSREG	trxpos ;
			DG_GSREG	trxreg ;
			DG_GSREG	trxdir ;
		} data ;
		DG_GIFTAG	giftag2 ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_BITBLTBUF( 0, 0, 0, BUFFER_PAGE(2)/64+4*32, BUFFER_WIDTH/64, SCE_GS_PSMCT32 ),
			   SCE_GS_BITBLTBUF},
			{SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0 ),SCE_GS_TRXPOS},
			{SCE_GS_SET_TRXREG( 256, 256 ),SCE_GS_TRXREG},
			{SCE_GS_SET_TRXDIR( 0 ),SCE_GS_TRXDIR}
		},
		{SCE_GIF_SET_TAG(256*256*4/16, 1, 0, 0, 2, 1),0}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT( packet_size, 0 ) ;
	tag++ ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 256*256*4/16 );
	tag->addr = data ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT( 256*256*4/16, 0 ) ;

	return ( &tag[1] );
}
#else
/* スポットライト用ベースイメージ転送パケット生成 */
void *DG_WritePacket_TransSpotImage( void *addr, void *data, int color )
{
	static struct _packet {
		DG_GIFTAG	giftag1 ;
		struct _gif_data {
			DG_GSREG	bitbltbuf ;
			DG_GSREG	trxpos ;
			DG_GSREG	trxreg ;
			DG_GSREG	trxdir ;
		} data ;
		DG_GIFTAG	giftag2 ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_BITBLTBUF( 0, 0, 0, BUFFER_PAGE(2)/64+BUFFER_PAGE(1)/64/2, BUFFER_WIDTH/64, SCE_GS_PSMCT16 ),
			   SCE_GS_BITBLTBUF},
			{SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0 ),SCE_GS_TRXPOS},
			{SCE_GS_SET_TRXREG( 256, 256 ),SCE_GS_TRXREG},
			{SCE_GS_SET_TRXDIR( 0 ),SCE_GS_TRXDIR}
		},
		{SCE_GIF_SET_TAG(256*256*4/16, 1, 0, 0, 2, 1),0}
	} ;
	static struct _packet2 {
		DG_GIFTAG	giftag ;
		struct _gif_data2 {
			DG_GSREG	texflush ;
			DG_GSREG	tex0 ;
			DG_GSREG	clamp ;
			DG_GSREG	test ;
			DG_GSREG	prim ;
			DG_GSREG	prmode ;
			DG_GSREG	rgba ;
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	test2 ;
		} data ;
	} gif_data2 = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data2)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{0,SCE_GS_TEXFLUSH},
			{SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64+BUFFER_PAGE(1)/64/2, BUFFER_WIDTH/64, SCE_GS_PSMCT16, 8, 8, 1, 0,
							 0, 0, 0, 0, 0 ),SCE_GS_TEX0_1},
			{SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 ),SCE_GS_CLAMP_1},
			//{SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRIM},
			{SCE_GS_SET_PRMODE( 0, 1, 0, 0, 0, 1, 0, 0 ),SCE_GS_PRMODE},
			{SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0x00000000 ),SCE_GS_RGBAQ},
			{SCE_GS_SET_UV( ( 0+1 ) * 16, ( 0+1 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 - 128 ) * 16, ( 2048 - 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_UV( ( 256+1 ) * 16, ( 256+1 ) * 16 ),SCE_GS_UV},
			{SCE_GS_SET_XYZ2( ( 2048 + 128 ) * 16, ( 2048 + 128 ) * 16, 0 ),SCE_GS_XYZ2},
			{SCE_GS_SET_TEST( 1, 1, 64, 1, 0, 0, 1, 2 ),SCE_GS_TEST_1}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	struct _packet2		*write_gif_data ;
	int		packet_size ;

	/* 空き領域にテクスチャを転送する */
	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	//tag->vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT( packet_size, 0 ) ;
	tag++ ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 256*256*4/16 );
	tag->addr = data ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT( 256*256*4/16, 0 ) ;
	tag++ ;
	/* 空き領域に転送したテクスチャをテクスチャ生成フレームバッファに描画（フォーマット変換も兼ねる） */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(gif_data2) );
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(gif_data2), 0 ) ;
	tag++ ;
	write_gif_data = (struct _packet2*)tag ;
	*write_gif_data = gif_data2 ;
	write_gif_data->data.rgba.data = color ;	/* 描画色の変更 */

	return ( &write_gif_data[1] );
}
#endif

void *DG_WritePacket_CoverShadowModelGifInit( void *addr )
{
	static struct _packet {
		DG_GIFTAG	giftag ;
		struct _gif_data {
			DG_GSREG	test ;
			DG_GSREG	prmodecont ;
			DG_GSREG	prmode ;
		} data ;
	} gif_data = {
		{SCE_GIF_SET_TAG(sizeof(struct _gif_data)/16, 1, 0, 0, 0, 1),GS_REGS_1(GS_REGS_AD)},
		{
			{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 1 ),SCE_GS_TEST_1},
			{SCE_GS_SET_PRMODECONT(0),SCE_GS_PRMODECONT},
			{SCE_GS_SET_PRMODE( 0, 0, 0, 0, 0, 0, 0, 0),SCE_GS_PRMODE}
		}
	} ;
	DG_DMATAG	*tag = addr ;
	int		packet_size ;

	packet_size = sizeof(gif_data)/16 ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet_size );
	tag->addr = &gif_data ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(packet_size,0) ;

	return ( &tag[1] );
}

/* ---------------------------------------------------------------- */
/* メイン画面をテクスチャ領域にぼかして転送 */
static DG_PACKET_DRAWSCREEN	DG_SoftImageTurnOut[2] ;
void *DG_WritePacket_SoftImageTurnOut( void *addr, int which )
{
	DG_DMATAG	*tag = addr ;

	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_SoftImageTurnOut[which]) );
	tag->addr = &DG_SoftImageTurnOut[which] ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(DG_SoftImageTurnOut[which]),0) ;

	return ( &tag[1] );
}


/*
*/
/* ---------------------------------------------------------------- */
/*
	パケット初期化サブルーチン
*/

static void InitPacket_FrameBuffer( DG_PACKET_FRAME_BUFFER *packet,
								   int base, int bw, int w, int h, int zbase, int flag )
{
	int		reg_offset = 0 ;
	int		col_mode = SCE_GS_PSMCT32 ;
	int		z_mode = SCE_GS_PSMZ24 ;
	if ( flag & DG_PACKET_FLAG_CONTEXT2 ) reg_offset = 1 ;
	if ( flag & DG_PACKET_FLAG_COLDEPTH16 ) col_mode = SCE_GS_PSMCT16 ;
	if ( flag & DG_PACKET_FLAG_ZDEPTH16 ) z_mode = SCE_GS_PSMZ16 ;

	packet->frame.reg = SCE_GS_FRAME_1 + reg_offset ;
	packet->frame.data = SCE_GS_SET_FRAME( base/2048, bw/64, col_mode, 0 );
	packet->zbuf.reg = SCE_GS_ZBUF_1 + reg_offset ;
	packet->zbuf.data = SCE_GS_SET_ZBUF( zbase, z_mode, ( flag & DG_PACKET_FLAG_NODEPTH ) != 0 );
	packet->xyoffset.reg = SCE_GS_XYOFFSET_1 + reg_offset ;
	packet->xyoffset.data = SCE_GS_SET_XYOFFSET( (2048-w/2)*16, (2048-h/2)*16 );
	packet->scissor.reg = SCE_GS_SCISSOR_1 + reg_offset ;
	packet->scissor.data = SCE_GS_SET_SCISSOR( 0, w-1, 0, h-1 );
}
static void InitPacket_Texture( DG_PACKET_TEXTURE *packet,
							   int base, int bw, int w, int h, int flag )
{
	int		tw, th, tmp ;
	int		reg_offset = 0 ;
	int		col_mode = SCE_GS_PSMCT32 ;
	if ( flag & DG_PACKET_FLAG_CONTEXT2 ) reg_offset = 1 ;
	if ( flag & DG_PACKET_FLAG_TEXDEPTH16 ) col_mode = SCE_GS_PSMCT16 ;

	for ( tmp = w, tw = 0 ; tmp > 0 ; tw++, tmp >>=1 );
	for ( tmp = h, th = 0 ; tmp > 0 ; th++, tmp >>=1 );
	packet->tex0.reg = SCE_GS_TEX0_1 + reg_offset ;
	packet->tex0.data = SCE_GS_SET_TEX0( base/64, bw/64, col_mode, tw, th, ( flag & DG_PACKET_FLAG_USETEXALPHA ) != 0,
										0, 0, 0, 0, 0, 0 );
	packet->clamp.reg = SCE_GS_CLAMP_1 + reg_offset ;
	packet->clamp.data = SCE_GS_SET_CLAMP( 2, 2, 1, w-2, 1, h-2 );
}

static void InitPacket_Draw2D( DG_PACKET_DRAW2D *packet, int col, int flag )
{
	u_long64	alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) ;
	u_long64	prim_mode = SCE_GS_PRIM_SPRITE|SCE_GS_PRIM_FST ;

	int		reg_offset = 0 ;
	if ( flag & DG_PACKET_FLAG_CONTEXT2 ) reg_offset = 1 ;
	if ( flag & DG_PACKET_FLAG_CONTEXT2 ) prim_mode |= SCE_GS_PRIM_CTXT2 ;
	if ( flag & DG_PACKET_FLAG_USETEXTURE ) prim_mode |= SCE_GS_PRIM_TME ;
	if ( flag & DG_PACKET_FLAG_ALPHABLEND ) prim_mode |= SCE_GS_PRIM_ABE ;
	if ( flag & DG_PACKET_FLAG_ADDBLEND ) alpha = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 );
	if ( flag & DG_PACKET_FLAG_SUBBLEND ) alpha = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 128 );

	packet->alpha.reg = SCE_GS_ALPHA_1 + reg_offset ;
	packet->alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 );
	packet->test.reg = SCE_GS_TEST_1 + reg_offset ;
	packet->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0,
										1, ( flag & DG_PACKET_FLAG_NODEPTH ) ? 1 : 2 );

	packet->prim.reg = SCE_GS_PRIM ;
	packet->prim.data = prim_mode ;
	packet->prmode.reg = SCE_GS_PRMODE ;
	packet->prmode.data = prim_mode ;
	packet->rgbaq.reg = SCE_GS_RGBAQ ;
	packet->rgbaq.data = col ;
}

static void InitPacket_Verts( DG_PACKET_VERTS *packet, int x, int y, int z, int u, int v )
{
	packet->uv.reg = SCE_GS_UV ;
	packet->uv.data = SCE_GS_SET_UV( u, v );
	packet->xyzf2.reg = SCE_GS_XYZF2 ;
	packet->xyzf2.data = SCE_GS_SET_XYZF2( x, y, z, 255 );
}

/* ---------------------------------------------------------------- */
/* パケットメモリ初期化 */
void DG_InitSystemPacket( void )
{
	int		i, flag ;

	/* 画面退避パケットの初期化 */
	flag = DG_PACKET_FLAG_CONTEXT2|DG_PACKET_FLAG_NODEPTH|DG_PACKET_FLAG_USETEXTURE ;
	for ( i = 0 ; i < 2 ; i++ ){
		INIT_PACKET_GIFTAG( DG_SoftImageTurnOut[i] );
		InitPacket_FrameBuffer( &DG_SoftImageTurnOut[i].frame,
							   TEXTURE_TOP_PAGE(), DRAW_WIDTH, DRAW_WIDTH, DRAW_HEIGHT, ZBUFFER_PAGE(), flag );
		InitPacket_Texture( &DG_SoftImageTurnOut[i].texture,
						   BUFFER_PAGE(i), DRAW_WIDTH, DRAW_WIDTH, DRAW_HEIGHT, flag );
		InitPacket_Draw2D( &DG_SoftImageTurnOut[i].draw2d,
						  0x80808080, flag );
		{
			int		j ;
			int		x, y, w, h, u, v, uw, vh ;
			x = ( 2048 - DRAW_WIDTH  / 2 ) * 16 ;
			u = 16 ;
			w =   ( DRAW_WIDTH  ) * 16 / 8 ;
			uw =  ( DRAW_WIDTH  ) * 16 / 8 ;
			y = ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
			v = 16 ;
			h =   ( DRAW_HEIGHT ) * 16 ;
			vh =  ( DRAW_HEIGHT ) * 16 ;
			for ( j = 0 ; j < 8 ; j++ ){
				InitPacket_Verts( &DG_SoftImageTurnOut[i].verts[j][0], x, y, 0, u, v );
				InitPacket_Verts( &DG_SoftImageTurnOut[i].verts[j][1], x+w, y+h, 0, u+uw, v+vh );
				x += w ;
				u += uw ;
			}
		}
	}
}

