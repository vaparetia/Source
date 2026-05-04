#ifndef __PARSE_H__
#define __PARSE_H__

/*
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

typedef struct parse_info {
	struct parse_info *parent;
	FILE *fp;
	char *filename;
	long line;
	unsigned char *ptr;
	unsigned char line_buffer[ LINE_BUFFER_SIZE ];
} PARSE_INFO;

enum {
	PARSE_NORMAL_MODE,
	PARSE_EXPR_MODE,
};

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
void print_file_current_pos( char *mes );
void print_current_line( void );

void set_define( char *tag, char *to );
void parse_init_define( void );
void parse_free_define( void );

#define MAX_WORDBUF 128
typedef char WORDBUF[ MAX_WORDBUF ];

/* ユーティリティマクロ */

void get_line_ex( void );
void open_brance_check( void );
void get_word_with_error_check( char *word, char *def, char *errormes );
char *get_word_cl( char *word );
char *get_word_ex( char *word );

void set_parse_buffer( char *ptr );
void reset_parse_buffer( void );

#endif
