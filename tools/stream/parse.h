/*
   汎用パーサーヘッダファイル
*/

/*
   バーポーズモード用
*/

/* in print.c */

extern long verbose_mode;
extern char *program_name;

void MESSAGE( char *format, ... );
void PRINTF( char *format, ... );
void DUMP( char *format, ... );
void ERROR( char *format, ... );
void WARNING( char *format, ... );
void FATAL( char *format, ... );

#define LINE_BUFFER_SIZE	1024

typedef struct parse_info {
	struct parse_info *parent;
	FILE *fp;
	char *filename;
	long line;
	unsigned char *ptr;
	unsigned char line_buffer[ LINE_BUFFER_SIZE ];
} PARSE_INFO;

/* -------------- パーサー関連 -------------- */

/* in parse.c */

void parse_init( void );
void parse_expr_mode( void );
void parse_normal_mode( void );
long is_end_file( void );
long load_file( char *filename );
char get_next_char( void );
char *get_word( char *buf );
long get_new_line( void );
void print_file_current_pos( char *mes );
void print_current_line( void );

void set_define( char *tag, char *to );

typedef char WORDBUF[ 128 ];

