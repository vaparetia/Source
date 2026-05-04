/*
	font.h
		フォント描画ルーチンヘッダ
	2000/01/05	K.Uehara
	$Id: font.h,v 1.2 2002/04/05 07:29:44 usr01475 Exp $
*/


/*
	コンフィグレーション
*/

#define TOP_KINSOKU_MASK	0x4000
#define BACK_KINSOKU_MASK	0x2000

#define FONT_OP_MASK		0x6000

/* 文字サイズ */

#define FONT_SIZE_W		24
#define FONT_SIZE_H		24
#define RUBI_SIZE_W		12
#define RUBI_SIZE_H		12

extern int font_han_max_offset;

#define FONT_ONE_SIZE	(24*24/4)	// 全角一文字あたりのバイト数
#define FONT_LINE_LEN	36			// 引き棒の長さ
#define DOWN_MARGINE	(font_han_max_offset+1)

#define FONT_BUFFER_HEIGHT( _line, _lskip ) \
		( (_line)*(FONT_SIZE_H+(_lskip)) + DOWN_MARGINE )
#define FONT_BUFFER_WIDTH( _col, _cskip ) \
		( (_col)*(FONT_SIZE_W+(_cskip) ) )

/* 平仮名とカタカナと特殊文字の数 ( zen_table 変更時に注意 ) */

#define FONT_HIRA_NUM	0x51
#define FONT_KATA_NUM	0x54
#define FONT_KIGOU_NUM	0x1D

#define FONT_HIRA_TOP	0
#define FONT_KATA_TOP	(FONT_HIRA_NUM)
#define FONT_KIGOU_TOP	(FONT_HIRA_NUM+FONT_KATA_NUM)
#define FONT_ZEN_TOP	(FONT_HIRA_NUM+FONT_KATA_NUM+FONT_KIGOU_NUM)

/* 特殊文字列 */

#define SP_COMMAND	0x8023		// '#'
#define SP_SPACE	0x8301		// 全角スペース
#define SP_JTEN		0x8308		// 全角'、'
#define SP_JMARU	0x8309		// 全角'。'

#define CODE( a )	( (a) | 0x8000 )
#define IS_HANKAKU( a )		( (a) < 0x8100 )

#define IS_BLOCK_END_CODE( _code ) \
	( ( (_code) == SP_JTEN ) || ( (_code) == SP_JMARU ) \
	  || ( (_code) == CODE( '}' ) || ( (_code) == CODE( ',' ) ) ) )

typedef struct _vraminfo FONT_VRAMINFO;
typedef struct _drawinfo FONT_DRAWINFO;
	 
typedef int ( *font_draw_func )( FONT_VRAMINFO *out, int x, int y, int code, int flag );
typedef int ( *rubi_draw_func )( FONT_DRAWINFO *out, int x, int y, unsigned char *rubi );

/*
	フォント描画バッファ環境
		バッファが生成されてから変更されない。
*/

struct _vraminfo {
	unsigned char c_width;
	unsigned char c_height;
	unsigned char c_skip;
	unsigned char l_skip;

	unsigned char color;
	unsigned char flag;
	unsigned char rubi_display_flag;
	unsigned char y_step;

	short row;
	short width;
	short height;
	short max_width;

	void *vramtop;

	font_draw_func han_func;
	font_draw_func zen_func;
	rubi_draw_func rubi_func;
};

/*
	フォント描画関数環境
		フォントを描画する直前に確保。描画後は解放してもいい。
*/

struct _drawinfo {
	FONT_VRAMINFO *outinfo;
	int xtop;
	int ytop;
	int color;

	int put_Tdot_flag;
	int rubi_left_x;
	int rubi_left_y;
	int rubi_orikaeshi_xmax;

	int block_flag;
	int rubi_flag;
	int cr_flag;

	int nowx;
	int nowy;
	int yc;

	char *prev_char;
};

#define FONT_TYPE_RESIDENT	0x00
#define FONT_TYPE_MENU		0x01
#define FONT_TYPE_GCL		0x02
#define FONT_TYPE_VOX		0x03
#define FONT_TYPE_GCL2		0x04
#define MAX_FONT_TYPE		0x05

#define FONT_TYPE( _code )			(((_code)-0x8400)/0x400)
#define FONT_CODE_TOP( _type )		((_type)*0x400+0x8401)

#define FONT_NO_ORIKAESHI	0x01
#define FONT_NO_KINSOKU		0x02

#define FONT_RESULT_OVER		0x01
#define FONT_RESULT_ORIKAESHI	0x02

#define FONT_RGB( r, g, b )		(((r)<<0)|((g)<<8)|((b)<<16)|0x80000000)

/*
	関数プロトタイプ
*/

void font_resident_load_set( char *fonttop );
void font_resident_rubi_set( char *rubitop );
void font_set_top_addr( int type, void *addr );

void font_set_vraminfo( FONT_VRAMINFO *vinfo
						, void *vramtop, int width, int height
						, int c_skip, int l_skip, int flag );

void font_set_clut4( unsigned int *clut_buf, int no, int fg, int bg );

int font_draw_string( FONT_DRAWINFO *work, char *string );
void font_open_drawinfo( FONT_DRAWINFO *draw, FONT_VRAMINFO *vram );
void font_set_locate( FONT_DRAWINFO *draw, int x, int y );
void font_set_color( FONT_DRAWINFO *draw, int color );
void font_set_refnum( int refnum );

/*
	インライン
*/

extern inline int font_get_draw_width( FONT_DRAWINFO *draw )
{
	return draw->outinfo->max_width;
}

/*
	ツール専用
	( -D CHECK_ONLY でコンパイルする必要あり )
*/

int font_draw_area_check( char *mesg, int width, int height, int c_skip, int l_skip, int flag );
