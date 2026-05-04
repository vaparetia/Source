#ifndef __PARSE_H__
#define __PARSE_H__

/**
   @file
   汎用パーサーヘッダファイル
*/

/*
   バーポーズモード用
*/

/* in print.c */

extern int verbose_mode;
extern char *program_name;

#ifdef __GNUC__
#define NORETURN	__attribute__((noreturn))
#else
#define NORETURN
#endif

void MESSAGE( char *format, ... );
void PRINTF( char *format, ... );
void DUMP( char *format, ... );
void ERROR( char *format, ... ) NORETURN;
void WARNING( char *format, ... );
void FATAL( char *format, ... ) NORETURN;

#define LINE_BUFFER_SIZE	1024
#define MAX_PARSE_FILES		256

/// パーサー制御構造体
typedef struct parse_info {
	struct parse_info *parent;
	FILE *fp;
	int fileno;
	int line;
	int continue_line_flag;
	int def_buffer_flag;
	int pp_if_stack_level;
	unsigned char *ptr;
	unsigned char *back_ptr;
	unsigned char *line_buffer;
	int line_buffer_size;
} PARSE_INFO;

/// パース時の特殊文字モードを表す。
enum {
	PARSE_NORMAL_MODE,
	PARSE_EXPR_MODE,
	PARSE_STRING_MODE,
};

#define MAX_WORDBUF 256
/// 標準的な取得用バッファ
typedef char WORDBUF[ MAX_WORDBUF ];

/* -------------- パーサー関連 -------------- */

/* in parse.c */

void parse_init( void );
void parse_expr_mode( void );
void parse_normal_mode( void );
int parse_change_mode( int mode );
void parse_end( void );
int is_end_file( void );
int load_file( char *filename );
char get_next_char( void );
char *get_word( char *buf );
void unget_word( char *buf );
int get_line( void );
void back_word_ptr( void );

void print_file_current_pos( char *mes );
void print_current_line( void );

void parse_set_depend_log_file( char *filename );
void parse_close_depend_log_file( void );
void parse_set_depend_file( char *depfile );
void parse_set_depend_outfile( char *outfile );

void set_define( char *tag, char *to );
void parse_enum( void );

void parse_init_define( void );
void parse_free_define( void );
void parse_check_define_mode( int mode );

void set_include_path( char *path );
void parse_define_check_disable( int mode );

/* ユーティリティマクロ */

void get_line_ex( void );
void open_brance_check( void );
void get_word_with_error_check( char *word, char *def, char *errormes );
char *get_word_cl( char *word );
char *get_word_ex( char *word );
char *get_word_pp( char *buf );
char *get_word_no_pp( char *buf );

// そのバッファだけをローカルに扱う(バッファ終端でparse終了)
#define PARSE_BUFFER_LOCAL	0
// そのバッファをグローバルに扱う(バッファ終端で元のコンテキストに戻る)
#define PARSE_BUFFER_GLOBAL	1	// そのバッファを

//void set_parse_buffer( char *ptr, int parse_flag );
void set_parse_buffer( char *ptr );
void reset_parse_buffer( void );

char *parse_get_current_filename( void );
int parse_get_current_linenum( void );
char *parse_get_current_linebuf( void );

#endif
