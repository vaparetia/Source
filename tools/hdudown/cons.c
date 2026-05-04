/*
	cons.c
		全くダブルバッファを使わないコンソール画面ドライバ

	2001/01/22	K.Uehara
	$Id: cons.c,v 1.2 2001/05/31 12:50:59 usr01475 Exp $
*/

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <stdio.h>
#include <libcdvd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define SCREEN_W	512
#define SCREEN_H	448

#define CONS_W		480
#define CONS_H		400

#define POS(_x)	(((_x)+2048)<<4)

#define XMIN	POS(-CONS_W/2)
#define YMIN	POS(-CONS_H/2)
#define XMAX	POS(+CONS_W/2)
#define YMAX	POS(+CONS_H/2)

#define FONT_H		8
#define FONT_W		8
#define LINE_H		10

/* ---------------------------------------------------------------------- */
/*
	画面表示モジュール
*/

#include "font.h"

/* GIF TAG を設定 */

#define GV_UNCACHE( _ptr )	( ( typeof(_ptr) )( ( unsigned int )_ptr | 0x20000000 ) )

#define DMATAG_PCE_0		(2<<26)
#define DMATAG_PCE_1		(3<<26)
#define DMATAG_ID_REFE		(0<<28)
#define DMATAG_ID_CNT		(1<<28)
#define DMATAG_ID_NEXT		(2<<28)
#define DMATAG_ID_REF		(3<<28)
#define DMATAG_ID_REFS		(4<<28)
#define DMATAG_ID_CALL		(5<<28)
#define DMATAG_ID_RET		(6<<28)
#define DMATAG_ID_END		(7<<28)
#define DMATAG_IRQ			(1<<31)
#define DMATAG_SET_QWC( flag, qwc ) ( (flag) | (qwc) )
#define DMATAG_SET_ADDR( addr ) ( (int)(addr) )
#define DMA_SIZE_OF( n )	( ( sizeof( n ) + 3 ) / 4 )

/* ＶＩＦｃｏｄｅ関連定義 */
#define VIF_VERT_PACK		(0x0d)	/* X(1.15.0),Y(1.15.0),Z(1.15.0),W(1.3.12) */
#define VIF_NORM_PACK		(0x0d)	/* X(1.3.12),Y(1.3.12),Z(1.3.12),W(0,16,0) */
#define VIF_UV_PACK			(0x15)	/* U(1.3.12),V(1.3.12) */
#define VIF_RGBA_PACK		(0x0e)	/* R(0.8.0),G(0.8.0),B(0.8.0),A(0.8.0) */
#define VIF_DATA128			(0x0c)	/*  */

enum {
	GS_REGS_PRIM,
	GS_REGS_RGBA,
	GS_REGS_STQ,
	GS_REGS_UV,
	GS_REGS_XYZF2,
	GS_REGS_XYZ2,
	GS_REGS_TEX0_1,
	GS_REGS_TEX0_2,
	GS_REGS_CLAMP_1,
	GS_REGS_CLAMP_2,
	GS_REGS_FOG,
	GS_REGS_0B_UNDEFINED,
	GS_REGS_XYZF3,
	GS_REGS_XYZ3,
	GS_REGS_AD,
	GS_REGS_NOP,
};

typedef struct _dg_dmatag{
	u_int	qwc ;
	void	*addr ;
	u_int	vifcode[2] ;
} DG_DMATAG;

typedef struct _dg_gsreg {
	u_long	data ;
	u_long	reg ;
} DG_GSREG;

#define DG_COPY128( a, b ) { *( u_long128 * )(a) = *( u_long128 * )(b); }

#define DG_SET_GIFTAG( a, b... ) \
{ \
	static const sceGifTag __tmp = { b } ; \
	DG_COPY128( a, &__tmp );\
}

/* GS レジスタを設定 */

#define DG_SET_GS_REG( a, b... ) \
{ \
	static const typeof( *a ) __tmp = { b } ; \
	*a = __tmp;\
}

typedef struct {
	sceGifTag gif;
	DG_GSREG texflush;
	DG_GSREG frame1;
	DG_GSREG zbuf1;
	DG_GSREG offset1;
	DG_GSREG scissor1;
	DG_GSREG alpha1;
	DG_GSREG test1;
	DG_GSREG prmodecont;
	DG_GSREG pabe;
} GS_HEADER;

typedef struct {
	sceGifTag	gif;
	sceGsPrim	prim;
	sceGsRgbaq	rgb1;
	sceGsXyz	xy1;
	sceGsXyz	xy2;
	u_long		pad;
} GS_BOX;

typedef struct {
	DG_DMATAG dmatag;
	GS_HEADER head;
	GS_BOX box;
} INITPRIMS;

typedef struct {
	DG_DMATAG	dmatag;
	sceGifTag	gif;
	sceGsPrim	prim;
	sceGsRgbaq	rgb1;
	sceGsXyz	xyz[ 8 ];
} FONTPRIMS;

typedef struct {
	DG_DMATAG	dmatag;
	GS_BOX		box;
} FONTBOXPRIMS;

typedef struct {
	DG_DMATAG	dmatag;
	sceGifTag	gif;
	DG_GSREG	bitbltbuf;
	DG_GSREG	trxpos;
	DG_GSREG	trxreg;
	DG_GSREG	trxdir;
	DG_GSREG	texflush;
} SCROLLPRIMS;

static INITPRIMS initpacks __attribute__((aligned(64)));
static FONTPRIMS fontprims __attribute__((aligned(64)));
static FONTBOXPRIMS fontboxprims __attribute__((aligned(64)));

static int fn_x1, fn_y1;

/* ---------------------------------------------------------------------- */
/*
	DMA転送関数
*/

static void vif_dma_send( void *top )
{
	/* DMA起動 */
	sceDmaChan *vif;
	vif = sceDmaGetChan( 1 /* VIF1 */ );
	vif->chcr.TTE = 1;

	sceDmaSend( vif, top );
	/* 終了待ち */
	asm volatile ( "sync.p" );
	asm volatile ( "sync.l" );

	while( *D1_CHCR & 0x100 );
}

static void dma_stop( void )
{
	/* DMA終了待ち */
	*D_STAT = *D_STAT;
	while( *D1_CHCR & 0x100 );
	while( *D2_CHCR & 0x100 );

	/* すべてのDMAを強制終了 */
	*D_ENABLEW = 0x10000;
	while( ( *D_ENABLER & 0x10000 ) == 0 );

	*D1_CHCR = 0;
	*D2_CHCR = 0;
	while( *D1_CHCR & 0x100 );
	while( *D2_CHCR & 0x100 );

	*D_ENABLEW = 0x00000;
	while( ( *D_ENABLER & 0x10000 ) != 0 );
}

/* ---------------------------------------------------------------------- */
/*
	初期設定パケット作成
*/

static void set_dmatag( DG_DMATAG *dp, int size )
{
	size = size / sizeof( u_long128 ) - 1;

	dp->qwc = DMATAG_SET_QWC( DMATAG_ID_END, size );
	dp->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 );
	dp->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( size, 0 );
}

static void set_box( GS_BOX *bp, int x1, int y1, int x2, int y2, u_long alpha )
{
	DG_SET_GIFTAG( &bp->gif, .FLG = SCE_GIF_REGLIST
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA, GS_REGS_XYZ2, GS_REGS_XYZ2
				   , .NREG = 4, .NLOOP = 1 );

	DG_SET_GS_REG( &bp->prim, .PRIM = 6, .ABE  = 0, .FST = 1, .FIX = 1 );

	*( u_long * )&( bp->rgb1 ) = alpha;
		
	bp->xy1.X = x1;
	bp->xy1.Y = y1;
	bp->xy2.X = x2;
	bp->xy2.Y = y2;

	asm volatile ( "sync.p" );
	asm volatile ( "sync.l" );
}

static void init_font( void )
{
	DG_DMATAG *dp = GV_UNCACHE( &( fontprims.dmatag ) );
	FONTPRIMS *fp = GV_UNCACHE( &( fontprims ) );

	set_dmatag( dp, sizeof( FONTPRIMS ) );

	DG_SET_GIFTAG( &fp->gif, .FLG = SCE_GIF_REGLIST
				   , .NREG = 10, .NLOOP = 1, .EOP = 1
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA
				   , GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2
				   , GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2 );
	DG_SET_GS_REG( &fp->prim, .PRIM = 0 );
	*( u_long * )&( fp->rgb1 ) = SCE_GS_SET_RGBAQ( 200, 200, 200, 200, 10 );
	fn_x1 = XMIN;
	fn_y1 = YMIN;

	set_dmatag( GV_UNCACHE( &fontboxprims.dmatag ), sizeof( fontboxprims ) );
}

static void setup_drawenv( void )
{
	/* 表示バッファ設定 SCREEN_W * SCREEN_H, frame, hiresoにする */
	static tGS_DISPFB1 fb1 = { .FBP = 0, .FBW = SCREEN_W / 64, .PSM = 0 };
	static tGS_DISPFB2 fb2 = { .FBP = 0, .FBW = SCREEN_W / 64, .PSM = 0 };
	static tGS_DISPLAY1 disp1 = { .DH = SCREEN_H - 1, .DW = 0x9FF
										 , .MAGH = 4, .MAGV = 0
										 , .DY = 50+8, .DX = 0x28c };
	static tGS_DISPLAY2 disp2 = { .DH = SCREEN_H - 1, .DW = 0x9FF
										 , .MAGH = 4, .MAGV = 0
										 , .DY = 50+1+8, .DX = 0x28c };

	*GS_SMODE2 = 0x01;

	*GS_DISPFB1 = *( u_long * )&fb1;
	*GS_DISPLAY1 = *( u_long * )&disp1;
	*GS_DISPFB2 = *( u_long * )&fb2;

	*GS_DISPLAY2 = *( u_long * )&disp2;

	*GS_PMODE = 0x8023;

	asm volatile ( "sync.p" );
	asm volatile ( "sync.l" );
}

static void setup_init_packet( void )
{
	/* パケットを初期設定 */
	DG_DMATAG *dp = GV_UNCACHE( &( initpacks.dmatag ) );
	GS_HEADER *hp = GV_UNCACHE( &( initpacks.head ) );
	GS_BOX *bp = GV_UNCACHE( &( initpacks.box ) );

	set_dmatag( dp, sizeof( initpacks ) );

	/* フレームバッファ等の設定 */
	DG_SET_GIFTAG( &hp->gif, .FLG = SCE_GIF_PACKED
				   , .NREG = 1, .NLOOP = sizeof( GS_HEADER ) / 16 - 1
				   , .REGS0 = GS_REGS_AD );

	hp->texflush.reg = SCE_GS_TEXFLUSH;

	DG_SET_GS_REG( ( sceGsFrame * )&hp->frame1.data, .FBP = 0, .FBW = SCREEN_W/64, .PSM = 0 );
	hp->frame1.reg = SCE_GS_FRAME_1;

	DG_SET_GS_REG( ( sceGsZbuf * )&hp->zbuf1.data, .ZMSK = 1 );
	hp->zbuf1.reg = SCE_GS_ZBUF_1;

	DG_SET_GS_REG( ( sceGsXyoffset * )&hp->offset1.data
				   , .OFX = (2048-SCREEN_W/2)*16, .OFY = (2048-SCREEN_H/2)*16 );
	hp->offset1.reg = SCE_GS_XYOFFSET_1;

	DG_SET_GS_REG( ( sceGsScissor * )&hp->scissor1.data
				   , .SCAX0 = 0, .SCAY0 = 0, .SCAX1 = SCREEN_W, .SCAY1 = SCREEN_H );
	hp->scissor1.reg = SCE_GS_SCISSOR_1;

	DG_SET_GS_REG( ( sceGsTest * )&hp->test1.data, .ZTE = 1, .ZTST = 1 );
	hp->test1.reg = SCE_GS_TEST_1;

	hp->alpha1.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 );
	hp->alpha1.reg = SCE_GS_ALPHA_1;

	hp->prmodecont.data = SCE_GS_SET_PRMODECONT( 1 );
	hp->prmodecont.reg = SCE_GS_PRMODECONT;

	hp->pabe.data = SCE_GS_SET_PABE( 0 );
	hp->pabe.reg = SCE_GS_PABE;

	set_box( bp, POS(-SCREEN_W/2), POS(-SCREEN_H/2), POS(SCREEN_W/2), POS(SCREEN_H/2)
			 , SCE_GS_SET_RGBAQ( 0, 0, 0, 128, 10 ) );
	bp->gif.EOP = 1;
}

/* ---------------------------------------------------------------------- */
/*
	描画パケット作成／描画
*/

// 一番下の行を残してスクロール

static SCROLLPRIMS scroll __attribute__((aligned(16)));

static void scroll_line( void )
{
	SCROLLPRIMS *sp = GV_UNCACHE( &scroll );

	set_dmatag( GV_UNCACHE( &( sp->dmatag ) ), sizeof( scroll ) );
	
	DG_SET_GIFTAG( &sp->gif, .FLG = SCE_GIF_PACKED
				   , .NREG = 1, .NLOOP = sizeof( SCROLLPRIMS ) / 16 - 2
				   , .REGS0 = GS_REGS_AD, .EOP = 1 );

	DG_SET_GS_REG( ( sceGsBitbltbuf * )&sp->bitbltbuf.data
				   , .SBP = 0, .SBW = SCREEN_W / 64, .SPSM = 0
				   , .DBP = 0, .DBW = SCREEN_W / 64, .DPSM = 0 );
	sp->bitbltbuf.reg = SCE_GS_BITBLTBUF;
	DG_SET_GS_REG( ( sceGsTrxpos * )&sp->trxpos.data
				   , .DIR = 0
				   , .SSAX = ( SCREEN_W - CONS_W ) / 2
				   , .SSAY = ( SCREEN_H - CONS_H ) / 2 + LINE_H
				   , .DSAX = ( SCREEN_W - CONS_W ) / 2
				   , .DSAY = ( SCREEN_H - CONS_H ) / 2
				   );
	sp->trxpos.reg = SCE_GS_TRXPOS;
	DG_SET_GS_REG( ( sceGsTrxreg * )&sp->trxreg.data
				   , .RRW = CONS_W, .RRH = CONS_H );
	sp->trxreg.reg = SCE_GS_TRXREG;
	DG_SET_GS_REG( ( sceGsTrxdir * )&sp->trxdir.data, .XDR = 2 );
	sp->trxdir.reg = SCE_GS_TRXDIR;
	sp->texflush.reg = SCE_GS_TEXFLUSH;

	asm volatile ( "sync.p" );
	asm volatile ( "sync.l" );
}

static void draw_font( char *str )
{
	FONTPRIMS *fp = GV_UNCACHE( &fontprims );
	FONTBOXPRIMS *bp = GV_UNCACHE( &fontboxprims );
	int x, y;

	x = fn_x1;
	y = fn_y1;

	/* フォントを点の集合として１ラインずつ描画する */

	for( ; *str != '\0'; str ++ ){
		const u_char *p;
		int c;
		int i, j, n;
		int mask;
		sceGsXyz *xp;

		c = *str;
		if( c == '\n' || x >= XMAX - FONT_W ){
			if( y < YMAX - ( LINE_H << 4 ) ){
				y += ( LINE_H << 4 );
			} else {
				scroll_line();
				vif_dma_send( &scroll );
			}
			x = XMIN;
			if( c == '\n' ){
				continue;
			}
		}
		if( c == '\r' ){
			x = XMIN;
			continue;
		}

		set_box( &bp->box, x, y, x + ( FONT_W << 4 ), y + ( FONT_H << 4 )
				 , SCE_GS_SET_RGBAQ( 0, 0, 0, 128, 10 ) );
		vif_dma_send( &fontboxprims );

		if( c >= 0x60 ) c -= 0x20;
		c -= 0x20;

		p = font8[ c ].image;

		for( j = 0; j < FONT_W; j++ ){
			n = 0;
			mask = 0x01;
			xp = fp->xyz;
			for( i = 0; i < FONT_H; i++ ){
				if( mask & *p ){
					xp->X = x + i * (1<<4);
					xp->Y = y + j * (1<<4);
				} else {
					xp->X = 0;
					xp->Y = 0;
				}
				xp++;
				n++;
				mask <<= 1;
			}
			if( n >= 0 ){
				fp->gif.NREG = n + 2;
				vif_dma_send( &fontprims );
			}
			p++;
		}
		x += ( FONT_W << 4 );
	}
	fn_x1 = x;
	fn_y1 = y;
}

/* ---------------------------------------------------------------------- */
/*
	インターフェース
*/

void cons_init( void )
{
	setup_init_packet();
	setup_drawenv();
	scroll_line();
	vif_dma_send( &initpacks );

	init_font();
}

void cons_printf( char *format, ...  )
{
	va_list args;
	char buf[ 256 ];

	va_start( args, format );
	vsprintf( buf, format, args );

	draw_font( buf );

	va_end( args );
}
