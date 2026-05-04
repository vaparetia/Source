/*
	fontconv.h
		1999/12/20	K.Uehara
		$Id: fontconv.h,v 1.8 2002/06/11 23:57:48 usr01475 Exp $
*/

/* -------------- デバッグ用フォントチェック関連 ------------- */

typedef struct {
	int width;
	int height;
	int c_skip;
	int l_skip;
	int kinsoku;
	char fontfile[ 256 ];
} FONT_AREA_CHECK;

extern int font_area_check_flag;
extern FONT_AREA_CHECK font_area_check;

#define FONTCONV_RESULT_OVER		0x01
#define FONTCONV_RESULT_ORIKAESHI	0x02
#define FONTCONV_NO_ZENKAKU			0x80

/* -------------- 関数 ------------- */

void font_set_load_path( char *path );
void font_init_font_file( int font_code_top, char *all_font_file );
void font_load_table( char *filename );
void font_reset_font_table( void );
int font_conv_buffer( char *dest, char *src );
void font_output_font_data( FILE *fp );
int fontconv_area_check( char *string, FONT_AREA_CHECK *area );

