/*
	KCEJ DEBUG UTILITY for EE
		EXCEPTION MANAGER

	2000/05/12	K.Uehara
	$Id: exception.c,v 1.18 2002/09/11 06:18:37 usr01475 Exp $
*/

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sifcmd.h>
#include <sifdev.h>

//#define IOP_EXCEPTION	1

#ifndef MASTER

extern void DEBUG_ExceptionIn( void );	// in exception_in.s

static int exception_in_flag = 0;

/* ---------------------------------------------------------------------- */
/*
	例外発生時スクリーンダンプ
*/

/* コンフィグレーション */

#define FFI			1		// フリッカーフリーインターレス

#define VRAM_ADDR	0

#define FRAME_W		512
#define FRAME_H		448

#define SCREEN_W	480
#define SCREEN_H	384

#define INFO_TOP_X	8
#define INFO_TOP_Y	8

#define MOJI_COLOR	SCE_GS_SET_RGBAQ( 200, 200, 200, 128, 0 )
#define MOJI_COLOR2	SCE_GS_SET_RGBAQ( 100, 100, 100, 128, 0 )

#define MAX_DEBUGLOG	512
#define MAX_LINES		16

/* 各種デファイン */

#include "font.h"	// フォントファイル

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
	static const sceGifTag __tmp = { ##b }; \
	DG_COPY128( a, &__tmp );\
}

/* GS レジスタを設定 */

#define DG_SET_GS_REG( a, b... ) \
{ \
	static const typeof( *a ) __tmp = { ##b }; \
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

static INITPRIMS initpacks __attribute__((aligned(16)));
static FONTPRIMS fontprims __attribute__((aligned(16)));

static int fn_x1, fn_y1;

#define XMIN	((-SCREEN_W/2+INFO_TOP_X+2048)<<4)
#define YMIN	((-SCREEN_H/2+INFO_TOP_Y+2048)<<4)

static void vif_dma_send( void *top )
{
	/* DMA起動 */
	sceDmaChan *vif;
	vif = sceDmaGetChan( 1 /* VIF1 */ );
	vif->chcr.TTE = 1;

	sceDmaSend( vif, top );
	/* 終了待ち */
//	sceDmaSync( vif, 0, 260 );
	while( *D1_CHCR & 0x100 );
}

static void init_font( void )
{
	DG_DMATAG *dp = GV_UNCACHE( &( fontprims.dmatag ) );
	FONTPRIMS *fp = GV_UNCACHE( &( fontprims ) );
	int size;

	size = sizeof( FONTPRIMS ) / sizeof( u_long128 ) - 1;
	dp->qwc = DMATAG_SET_QWC( DMATAG_ID_END, size );
	dp->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 );
	dp->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( size, 0 );

	DG_SET_GIFTAG( &fp->gif, .FLG = SCE_GIF_REGLIST
				   , .NREG = 10, .NLOOP = 1, .EOP = 1
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA
				   , GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2
				   , GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2, GS_REGS_XYZ2 );
	DG_SET_GS_REG( &fp->prim, .PRIM = 0 );
	*( u_long * )&( fp->rgb1 ) = MOJI_COLOR;
	fn_x1 = XMIN;
	fn_y1 = YMIN;
}

#define Y_STEP (8+2)

static void draw_font( unsigned char *str )
{
	FONTPRIMS *fp = GV_UNCACHE( &fontprims );
	int x, y;

	x = fn_x1;
	y = fn_y1;

	/* フォントを点の集合として１ラインずつ描画する */

	for( ; *str != '\0'; str ++ ){
		const u_char *p;
		int c;
		int i, j, n, yy;
		int mask;
		sceGsXyz *xp;
		c = *str;
		if( c == '\n' ){
			y += ( Y_STEP << 4 );
			x = XMIN;
			continue;
		}
		if( c >= 0x80 ){
			c = '?';
		}
		if( c >= 0x60 ) c -= 0x20;
		c -= 0x20;

		p = font8[ c ].image;

		yy = 0;
		for( j = 0; j < 8; j++ ){
			n = 0;
			mask = 0x01;
			xp = fp->xyz;
			*( u_long * )&( fp->rgb1 ) = MOJI_COLOR;
			for( i = 0; i < 8; i++ ){
				if( mask & *p ){
					xp->X = x + i * (1<<4);
					xp->Y = y + yy * (1<<4);
				}
				xp++;
				n++;
				mask <<= 1;
			}
			if( n > 0 ){
				fp->gif.NREG = n + 2;
				vif_dma_send( &fontprims );
			}
			yy++;
			p++;
		}
		x += ( 8 << 4 );
	}
	fn_x1 = x;
	fn_y1 = y;
}

static void draw_printf( char *format, ...  )
{
	va_list args;
	char buf[ 128 ];

	va_start( args, format );
	vsprintf( buf, format, args );

	draw_font( buf );

	va_end( args );
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

static void setup_drawenv( void )
{
	/* 表示バッファ設定 SCREEN_W * SCREEN_H, frame, hiresoにする */
	static tGS_DISPFB1 fb1 = { .FBP = 0, .FBW = FRAME_W / 64, .PSM = 0 };
//	static tGS_DISPFB2 fb2 = { .FBP = 0, .FBW = FRAME_W / 64, .PSM = 0 };
	static tGS_DISPLAY1 disp1 = { .DH = FRAME_H - 1, .DW = 0x9FF
										 , .MAGH = 4, .MAGV = 0
										 , .DY = 50+8, .DX = 0x28c };
	static tGS_DISPLAY2 disp2 = { .DH = FRAME_H - 1, .DW = 0x9FF
										 , .MAGH = 4, .MAGV = 0
										 , .DY = 50+1+8, .DX = 0x28c };

	*GS_SMODE2 = 0x01;

	*GS_DISPFB1 = *( u_long * )&fb1;
	*GS_DISPFB2 = *( u_long * )&fb1;
	*GS_DISPLAY1 = *( u_long * )&disp1;

	*GS_DISPLAY2 = *( u_long * )&disp2;

	*GS_PMODE = 0x8023;

	asm( "sync.p" );
	asm( "sync.l" );
}

static void setup_init_packet( void )
{
	/* パケットを初期設定 */
	DG_DMATAG *dp = GV_UNCACHE( &( initpacks.dmatag ) );
	GS_HEADER *hp = GV_UNCACHE( &( initpacks.head ) );
	GS_BOX *bp = GV_UNCACHE( &( initpacks.box ) );
	int size;

	size = sizeof( initpacks ) / sizeof( u_long128 ) - 1;

	dp->qwc = DMATAG_SET_QWC( DMATAG_ID_END, size );
	dp->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 );
	dp->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( size, 0 );

	/* フレームバッファ等の設定 */

	DG_SET_GIFTAG( &hp->gif, .FLG = SCE_GIF_PACKED
				   , .NREG = 1, .NLOOP = sizeof( GS_HEADER ) / 16 - 1
				   , .REGS0 = GS_REGS_AD );

	hp->texflush.reg = SCE_GS_TEXFLUSH;

	DG_SET_GS_REG( ( sceGsFrame * )&hp->frame1.data, .FBP = VRAM_ADDR
				   , .FBW = FRAME_W/64, .PSM = 0 );
	hp->frame1.reg = SCE_GS_FRAME_1;

	DG_SET_GS_REG( ( sceGsZbuf * )&hp->zbuf1.data, .ZMSK = 1 );
	hp->zbuf1.reg = SCE_GS_ZBUF_1;

	DG_SET_GS_REG( ( sceGsXyoffset * )&hp->offset1.data
				   , .OFX = (2048-FRAME_W/2)*16, .OFY = (2048-FRAME_H/2)*16 );
	hp->offset1.reg = SCE_GS_XYOFFSET_1;

	DG_SET_GS_REG( ( sceGsScissor * )&hp->scissor1.data
				   , .SCAX0 = 0, .SCAY0 = 0, .SCAX1 = FRAME_W, .SCAY1 = FRAME_H );
	hp->scissor1.reg = SCE_GS_SCISSOR_1;

	DG_SET_GS_REG( ( sceGsTest * )&hp->test1.data, .ZTE = 1, .ZTST = 1 );
	hp->test1.reg = SCE_GS_TEST_1;

	hp->alpha1.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 );
	hp->alpha1.reg = SCE_GS_ALPHA_1;

	hp->prmodecont.data = SCE_GS_SET_PRMODECONT( 1 );
	hp->prmodecont.reg = SCE_GS_PRMODECONT;

	hp->pabe.data = SCE_GS_SET_PABE( 0 );
	hp->pabe.reg = SCE_GS_PABE;

	/* 半透明のベースウインドウ */
	DG_SET_GIFTAG( &bp->gif, .FLG = SCE_GIF_REGLIST
				   , .REGS0 = GS_REGS_PRIM, GS_REGS_RGBA, GS_REGS_XYZ2, GS_REGS_XYZ2
				   , .NREG = 4, .NLOOP = 1, .EOP = 1 );

	DG_SET_GS_REG( &bp->prim, .PRIM = 6, .ABE  = 1, .FST = 1, .FIX = 1 );

	*( u_long * )&( bp->rgb1 ) = SCE_GS_SET_RGBAQ( 0, 0, 0, 64, 10 );
		
	bp->xy1.X = ( -(SCREEN_W/2) << 4 ) + ( 2048 << 4 );
	bp->xy1.Y = ( -(SCREEN_H/2) << 4 ) + ( 2048 << 4 );
	bp->xy2.X = ( (SCREEN_W/2) << 4 ) + ( 2048 << 4 );
	bp->xy2.Y = ( (SCREEN_H/2) << 4 ) + ( 2048 << 4 );

	asm( "sync.p" );
	asm( "sync.l" );
}

/* ---------------------------------------------------------------------- */
/*
	レジスタダンプ
*/

unsigned long DEBUG_exception_save_regs[ 32 + 1 ] __attribute__((aligned(64)));

static char *exception_mes[] = {
	"",
	"TLBINV",
	"TLBMISS(LD)",
	"TLBMISS(ST)",
	"ade(ld)",
	"ade(st)",
	"ibe",
	"dbe",
	"",
	"BP(/0)",
	"RI",
	"Cop",
	"OVF",
};

static void dump_reg( void )
{
	unsigned long *p;
	
	int etype, ca, vadd, epc, stat;

	asm( "mfc0 %0, $12" : "=r"(stat) );
	asm( "mfc0 %0, $13" : "=r"(ca) );
	asm( "mfc0 %0, $8" : "=r"(vadd) );
	asm( "mfc0 %0, $14" : "=r"(epc) );

	etype = ( ( ca >> 2 ) & 0xF );
	draw_printf( "EXCEPTION %s (%d)\n", exception_mes[ etype ], etype );
	draw_printf( "CAUSE=%08X EPC=%08X VAD=%08X STA=%08X\n", ca, epc, vadd, stat );

	p = GV_UNCACHE( ( void * )DEBUG_exception_save_regs );

	p[ 32 - 1 ] = epc;
	draw_printf( "gp=%08X sp=%08X fp=%08X ra=%08X\n",
				 p[ 28 - 1 ], p[ 29 - 1 ], p[ 30 - 1 ], p[ 31 - 1 ] );
#if 0
	{
		int i;
		for( i = 0; i < 24; i+= 6 ){
			// $1 - $24
			draw_printf( "%08X %08X %08X %08X %08X %08X\n"
						 , p[0], p[1], p[2], p[3], p[4], p[5] );
			p += 6;
		}
	}
#endif
}

/* ---------------------------------------------------------------------- */
/*
	printf ダンプ
*/

static char *lines[ MAX_LINES ] = { NULL };
static int lines_p = 0;
#define LINES( a )	( lines[ (a) % MAX_LINES ] )

static char mesgbuf[ MAX_DEBUGLOG ];
#define MESGBUFBOTTOM ( mesgbuf + MAX_DEBUGLOG )
static char *mesgbuf_bottom = mesgbuf;

static void set_printf_log( char *buf )
{
	char *s, *d;
	s = buf;
	d = mesgbuf_bottom;
	lines[ lines_p ] = d;
	lines_p = ( lines_p + 1 ) % MAX_LINES;
	for( ;; ){
		*d = *s;
		d++;
		if( d == MESGBUFBOTTOM ) d = mesgbuf;
		if( *s == '\0' ) break;
		s++;
	}
	mesgbuf_bottom = d;
}

static char *get_printf_log( char *mes, char *logptr )
{
	char *s, *d;

	d = mes;
	s = logptr;
	if( s == NULL ) return "";
	for( ;; ){
		*d = *s;
		d++;
		if( *s == '\0' ) break;
		s++;
		if( s == MESGBUFBOTTOM ) s = mesgbuf;
	}
	return mes;
}

static void dump_printf_log( void )
{
	char buf[ 128 ];
	int i;

	draw_printf( get_printf_log( buf, LINES( lines_p + MAX_LINES - 1 ) ) );
	draw_printf( "\n--- log ---\n" );
	for( i = 0; i < MAX_LINES; i++ ){
		draw_printf( get_printf_log( buf, LINES( lines_p + i ) ) );
	}
}

/* ---------------------------------------------------------------------- */
/*
	例外ハンドラ
*/

static void (*user_exception_func)( unsigned long *regs ) = NULL;

void DEBUG_SetExceptionCallback( void (*func)( unsigned long *regs ) )
{
	user_exception_func = func;
}

void DEBUG_ExceptionCallback( void )
{
	// レジスタをすべて保存した後,
	// ExceptionInから呼ばれる
	{
		// break 0x7 対処
		unsigned int cause, epc;
		asm( "mfc0 %0, $13" : "=r"(cause) );
		asm( "mfc0 %0, $14" : "=r"(epc) );
		if( ( ( cause >> 2 ) & 0x0F ) == 9 ){
			/* break */
			int code;
			if( cause & 0x80000000 ){
				epc = epc + 4;
			}
			code = ( *( unsigned int * )epc >> 6 );
			if( code != 7 ){
				/* 普通のbreak */
				return;
			}
		}
	}

	if( exception_in_flag != 0 ){
		// 発生するとすれば,user_exception_funcの中
		draw_printf( "double exception!!\n\n" );
		if( exception_in_flag < 2 ){
			dump_printf_log();
		}
		return;
	}
	exception_in_flag = 1;

	dma_stop();
	setup_drawenv();
	setup_init_packet();
	vif_dma_send( &initpacks );

	init_font();
	dump_reg();

	if( user_exception_func != NULL ){
		(*user_exception_func)( GV_UNCACHE( ( unsigned long * )DEBUG_exception_save_regs ) );
	}

	dump_printf_log();

	exception_in_flag = 2;
}

#ifdef IOP_EXCEPTION

#include <libexcep.h>

static char *iop_exception_mes[] = {
	"INT", "MOD", "TBL", "TBS",
	"AEL", "AES", "IBE", "DBE",
	"SYS", "BP", "RI", "CoP",
	"OVF", "???", "???", "???"
};

void DEBUG_IOPException( void *pkt, void *data )
{
	sceExcepIOPExceptionData *p = ( void * )DEBUG_exception_save_regs;

	dma_stop();
	setup_drawenv();
	setup_init_packet();
	vif_dma_send( &initpacks );

	init_font();

	draw_printf( "IOP EXCEPTION %s\n"
				 , iop_exception_mes[ ( p->reg[ IOP_CAUSE ] >> 2 ) & 0x0f ] );
	draw_printf( "MODULE %s VERSION %x\n", p->module, p->version );
	draw_printf( "PC %08X ( OFFSET = %08X )\n", p->reg[ IOP_EPC ], p->offset );
	draw_printf( "CAUSE %08x VAD %08x RA %08x(%08x) \n"
				 , p->reg[ IOP_CAUSE ], p->reg[ IOP_BADVADDR ]
				 , p->reg[ GPR_ra ]
				 , p->reg[ GPR_ra ] - (  p->reg[ IOP_EPC ] - p->offset ) );
	for( ;; );
}

#endif

/* ---------------------------------------------------------------------- */
/*
	登録関数
*/

void *DEBUG_ExceptionStackTop = ( void * )0x001FFFF0;

void DEBUG_InitException( void *stacktop )
{
	extern void SetVCommonHandler( int code, int addr );
	extern void SetVTLBRefillHandler( int code, int addr );

	if( stacktop != NULL ){
		DEBUG_ExceptionStackTop = stacktop;
	}

	// SCEのライブラリでもサポートされたが
	// こっちの方がデバッガに戻れるので、そのまま。

	SetVTLBRefillHandler( 1, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVTLBRefillHandler( 2, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVTLBRefillHandler( 3, ( int )DEBUG_ExceptionIn | 0x80000000 );

	SetVCommonHandler( 4, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 5, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 6, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 7, ( int )DEBUG_ExceptionIn | 0x80000000 );

	/* SYSCALLはさすがにやばいようだ */

	//SetVCommonHandler( 9, ( int )DEBUG_ExceptionIn | 0x80000000 );
	// ??? 何故か 0x00000000 にしないと動かない？？
	SetVCommonHandler( 9, ( int )DEBUG_ExceptionIn | 0x00000000 );	

	SetVCommonHandler( 10, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 11, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 12, ( int )DEBUG_ExceptionIn | 0x80000000 );
	SetVCommonHandler( 13, ( int )DEBUG_ExceptionIn | 0x80000000 );
}

void DEBUG_InitIOPException( char *module )
{
#ifdef IOP_EXCEPTION
	char addr[ 16 ];

	sprintf( addr, "%d", ( int )DEBUG_exception_save_regs );

	if( sceSifLoadModule( module, strlen( addr ) + 1, addr ) < 0 ){
		printf( "can't load module %s\n", module );
		return;
	}

	DI();
	sceSifAddCmdHandler( 0x80000010, DEBUG_IOPException
						 , ( void * )DEBUG_exception_save_regs );
	EI();
#endif
}

/* ---------------------------------------------------------------------- */
/*
	ダンプ用printf系関数群
*/

static void default_puts( char *buf )
{
//	fputs( buf, stdout );
	scePrintf( "%s", buf );
}

static void (*mts_puts)( char *buf ) = default_puts;

void MTS_SetPrintFunc( void (*func)( char *mes ) )
{
	mts_puts = func;
}

int cprintf( const char *format, ...  )
{
	va_list args;
	va_start( args, format );
	vfprintf( stdout, format, args );
	va_end( args );

	return 0;
}

int printf( const char *format, ...  )
{
	va_list args;
	char buf[ 128 ];
	va_start( args, format );
	vsprintf( buf, format, args );
	if( strlen( buf ) >= 128 ){
		cprintf( "PRINTF OVER FLOW !! : %s\n", buf );
		*( int * )1 = 0;	// HANGUP;
	}
	set_printf_log( buf );
	if( exception_in_flag == 0 ){
		(*mts_puts)( buf );
	}
	va_end( args );
	return 0;
}

void excep_printf( const char *format, ...  )
{
	va_list args;
	char buf[ 128 ];

	va_start( args, format );
	vsprintf( buf, format, args );

	if( exception_in_flag == 0 ){
		set_printf_log( buf );
	} else {
		draw_font( buf );
	}

	va_end( args );
}

#else	/* ndef MASTER */

/* マスター時にはすべてダミー扱い */

void MTS_SetPrintFunc( void (*func)( char *mes ) )
{
}

int cprintf( const char *format, ...  )
{
	return 0;
}

int printf( const char *format, ...  )
{
	return 0;
}

void excep_printf( const char *format, ...  )
{
}

#endif	/* MASTER */
