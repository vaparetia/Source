/*
	zen_table 作成プログラム
	( file code is shift jis )

	標準の zen_tableを作成する
*/

#include <stdio.h>

#define TOP_KINSOKU_MASK 0x4000
#define BACK_KINSOKU_MASK 0x2000

/*
static char top_kinsoku_table[] = "！？、。｝）」』］〉》】”’ー－…‥";
static char back_kinsoku_table[] = "（「『｛〈《［【“‘";
*/

typedef struct {
	unsigned char *zen;
	unsigned short code;
} ZENHAN_TABLE;

static ZENHAN_TABLE table[] = {
	/* 半角変換を行なう全角文字。アルファベット以外 */
	{ "，", 0x8000 | ',' | TOP_KINSOKU_MASK },
	{ "．", 0x8000 | '.' | TOP_KINSOKU_MASK },
	{ "・", 0x8000 | 0x7F },
	{ "：", 0x8000 | ':' },
	{ "；", 0x8000 | ';' },
	{ "♯", 0x8000 | '#' },
	{ "＆", 0x8000 | '&' },
	{ "（", 0x8000 | '(' | TOP_KINSOKU_MASK },
	{ "）", 0x8000 | ')' | BACK_KINSOKU_MASK },
	{ "｛", 0x8000 | '{' | TOP_KINSOKU_MASK },
	{ "｝", 0x8000 | '}' | BACK_KINSOKU_MASK },
	{ NULL, 0 },
};

static ZENHAN_TABLE special[] = {
	/* 特殊記号 */
	{ "　", 0x8301 },	/* 全角空白 */

	{ "◯", 0x8302 },
	{ "○", 0x8302 },	/* ○は統一 */
	{ "△", 0x8303 },
	{ "□", 0x8304 },
	{ "×", 0x8305 },

	{ "！", 0x8306 | TOP_KINSOKU_MASK },
	{ "？", 0x8307 | TOP_KINSOKU_MASK },
	{ "、", 0x8308 | TOP_KINSOKU_MASK },
	{ "。", 0x8309 | TOP_KINSOKU_MASK },
	{ "」", 0x830a | TOP_KINSOKU_MASK },
	{ "』", 0x830b | TOP_KINSOKU_MASK },
	{ "］", 0x830c | TOP_KINSOKU_MASK },
	{ "〉", 0x830d | TOP_KINSOKU_MASK },
	{ "》", 0x830e | TOP_KINSOKU_MASK },
	{ "】", 0x830f | TOP_KINSOKU_MASK },
	{ "”", 0x8310 | TOP_KINSOKU_MASK },
	{ "’", 0x8311 | TOP_KINSOKU_MASK },

	{ "ー", 0x8312 },
	{ "－", 0x8313 },
	{ "…", 0x8314 },
	{ "‥", 0x8315 },

	{ "「", 0x8316 | BACK_KINSOKU_MASK },
	{ "『", 0x8317 | BACK_KINSOKU_MASK },
	{ "〈", 0x8318 | BACK_KINSOKU_MASK },
	{ "《", 0x8319 | BACK_KINSOKU_MASK },
	{ "［", 0x831a | BACK_KINSOKU_MASK },
	{ "【", 0x831b | BACK_KINSOKU_MASK },
	{ "“", 0x831c | BACK_KINSOKU_MASK },
	{ "‘", 0x831d | BACK_KINSOKU_MASK },

	{ NULL, 0 },
};

static void out_table( ZENHAN_TABLE *table, FILE *fp )
{
	for( ;; ){
		if( table->zen == NULL ) return;

		fprintf( fp, "%X %X %c%c\n"
				 , ( table->zen[ 0 ] << 8 ) | table->zen[ 1 ]
				 , table->code
				 , table->zen[ 0 ], table->zen[ 1 ] );
		table ++;
	}
}

int main()
{
	FILE *fp;

	printf( "make zen_table\n" );

	if( ( fp = fopen( "zen_table", "wt" ) ) == NULL ){
		printf( "zen_table open error\n" );
		return 1;
	}

	out_table( special, fp );
	out_table( table, fp );

	/* 数字 */
	{
		int sjis;
		int code;
		int i;

		code = 0x8030;
		sjis = 0x824F;

		for( i = 0; i < 10; i++ ){
			fprintf( fp, "%X %X %c%c\n", sjis, code, ( sjis >> 8 ), sjis & 0xff );
			code ++; sjis ++;
		}
	}

	/* アルファベット大文字 */
	{
		int sjis;
		int code;
		int i;

		code = 0x8041;
		sjis = 0x8260;

		for( i = 0; i < 26; i++ ){
			fprintf( fp, "%X %X %c%c\n", sjis, code, ( sjis >> 8 ), sjis & 0xff );
			code ++; sjis ++;
		}
	}
	
	/* アルファベット小文字 */
	{
		int sjis;
		int code;
		int i;

		code = 0x8061;
		sjis = 0x8281;

		for( i = 0; i < 26; i++ ){
			fprintf( fp, "%X %X %c%c\n", sjis, code, ( sjis >> 8 ), sjis & 0xff );
			code ++; sjis ++;
		}
	}

	/* 平仮名 */
	{
		int sjis;
		int code;

		code = 0x8101;
		sjis = 0x829F;

		for( ; sjis <= 0x82f1; sjis ++ ){
			if( sjis == 0x82EE || sjis == 0x82EF ) continue;

			fprintf( fp, "%X %X %c%c\n", sjis, code, ( sjis >> 8 ), sjis & 0xff );
			code ++;
		}
	}
	
	/* 片仮名 */
	{
		int sjis;
		int code;

		code = 0x8201;
		sjis = 0x8340;

		for( ; sjis <= 0x8396; sjis ++ ){
			if( sjis == 0x837F || sjis == 0x8390 || sjis == 0x8391 ) continue;

			fprintf( fp, "%X %X %c%c\n", sjis, code, ( sjis >> 8 ), sjis & 0xff );
			code ++;
		}
	}

	fclose( fp );

	return 0;
}


