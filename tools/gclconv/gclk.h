/*
   gclk.h
*/

/*
   変数の情報フォーマット
   変数は基本的にbyte, shortのみ

   情報は24ビットで、
   ( type << 20 ) | ( bit << 16 ) | ( offset );
   typeはグローバル、ローカル、リンク
*/

#define TRUE	1
#define FALSE	0

#define VAR_TYPE_GLOBAL	0x000000
#define VAR_TYPE_LOCAL	0x100000
#define VAR_TYPE_LINK	0x800000

#define MAX_VAR_BUF		(8 * 1024)		/* 実機用変数バッファのサイズ */
#define MAX_LOCAL_VAR_BUF	(1024)

/* in variable.c */

extern char *varsym_version;
extern int gco_version;
extern int header_output_flag;
extern int block_output_mode;

void init_variable_table( void );
void set_variable_buffer_size( int size );
void set_local_variable_buffer_size( int size );
void read_variable_from_file( char *filename );
int load_variable_table( char *filename );
void write_binarys( char *infile, char *outfile );
void output_variable_table( char *filename, int dump_flag );
void load_linkvar_table( char *filename );
int write_binarys_block_mode( char *infile, FILE *out, FILE *tp );
void read_variable_block_mode( char *filename );
void dump_var_report( void );

#include "parse.h"
