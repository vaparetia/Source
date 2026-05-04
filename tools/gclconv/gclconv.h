/*
   gclconv シナリオデータコンバーター
		for MGS2 on PS2
*/

/* ----- gclに埋め込まれるフォントコード ----- */

#define GCL_FONT_CODE	0x8c00

/* ----- 型 ------ */

#define GCL_END		0x00

#define GCL_SHORT	0x01		/* 16ビット符合付き整数 */
#define GCL_BYTE	0x02		/* 8ビット符合なし整数 */
#define GCL_CHAR	0x03		/* 8ビット文字 */
#define GCL_BOOL	0x04		/* 1ビットフラグ */
#define GCL_VECTOR	0x05		/* 32ビット * 3 ベクトル */
#define GCL_STRID	0x06		/* 24ビット文字列ID */
#define GCL_STRING	0x07		/* 文字列。パスカル風ストリング */
#define GCL_PROCID	0x08		/* 関数ID */
#define GCL_INT		0x09		/* 32ビット符合付き整数 */
#define GCL_SYMBOL	0x0a		/* 外部ファイル解決シンボル（long） */
//#define GCL_SYMBOL2	0x0b		/* t:シンボルで解決できないときは-1 */
//#define GCL_SYMBOL3	0x0c		/* t:シンボルで解決できないときはERROR */
#define GCL_STRID2	0x0d		/* 24ビット文字列ID + 8BitID */
#define GCL_STRRES	0x0e		/* リソース参照文字列 */

#define GCL_TYPEMASK	0x0f	/* タイプ情報を取得するためのマスク */
#define GCL_TYPE( a )	( (a) & GCL_TYPEMASK )

#define GCL_CONST	0x00		/* 定数 */
#define GCL_VAR		0x10		/* 変数 (このあとに24ビットでオフセットがつく) */
#define GCL_ARRAY	0x20		/* 配列 */

#define GCL_EXPR	0x30		/* 式 */
//#define GCL_OP		0x31		/* 演算子 */

#define GCL_ARG		0x40		/* 引数文字 */
#define GCL_STATUS	0x41		/* 内部使用。ARG[0]に変換 */

#define GCL_OPTION	0x50		/* オプション文字 */
#define GCL_COMMAND	0x60		/* コマンド */
#define GCL_PROC	0x70		/* ユーザー関数 */
#define GCL_BLOCK	0x80		/* 実行ブロック */
#define GCL_LOCAL	0x90		/* procローカル変数 */

#define GCL_VALUE	0xC0		/* 小さい値のヘッダ */
#define GCL_OP		0xA0		/* 演算子マスク */


#define GCL_WILD	0xF0		/* ワイルドカード文字 (フォーマット用) */
#define GCL_REPEAT	0xE0		/* 繰り返し (フォーマット用) */

#define GCL_EXTSYM		0xD0	/* 外部シンボル: 最終的にはGCL_SYMBOLになる */
#define GCL_SYMBOL2		0xDB	/* t:シンボルで解決できないときは-1 */
#define GCL_SYMBOL3		0xDC	/* t:シンボルで解決できないときはERROR */
#define GCL_EXTCHARA	0xD1	/* デモシンボル: TSYMBOL扱い */
#define GCL_EXTCHARA2	0xD2	/* デモシンボル: STRING扱い */

/* -------------- 文字列エンコード -------------- */

long get_strcode( char *str );

/* -------------- コマンドテーブル関連 ------------- */

#define CHARA_COMMAND	"chara"		/* 特別扱い */
#define CMD_COMMAND		"command"	/* 特別扱い */
#define MESG_COMMAND	"mesg"		/* 特別扱い */
#define EXTERN_COMMAND	"EXTERN"	/* 特別扱い */
#define BLOCK_COMMAND	"block"		/* 特別扱い */

#define EVAL_COMMAND	"eval"		/* 式記述：特別扱い */

enum {
	COMMAND_TYPE_NORMAL,
	COMMAND_TYPE_CHARA,
	COMMAND_TYPE_COMMAND,
	COMMAND_TYPE_MESG,
	COMMAND_TYPE_EXTERN,
	COMMAND_TYPE_BLOCK,
	COMMAND_TYPE_MAX,
} ;

#define TABLE_TAG_NORMAL	0
#define TABLE_TAG_OMITABLE	1

typedef struct _table_tag {
	struct _table_tag *prev;
	struct _table_tag *next;
	int flag;
	int type;
	char data[ 0 ];
} COMMAND_TABLE_TAG;

typedef struct {
	long call_flag;
	int type;					// 種別ID
	char *name;
	unsigned int name_id;
	char *class;				/* chara コマンドのみ使用 */
	unsigned int class_id;		/* chara コマンドのみ使用 */
	char *command;				/* mesg コマンド */
	unsigned int command_id;	/* mesg コマンド */
	COMMAND_TABLE_TAG *tag;
} COMMAND_TABLE;

/* in prototable.c */
void setup_table( char *table_file );
COMMAND_TABLE *get_extern_command_table( char *command, int call_proc );
COMMAND_TABLE *get_cmd_command_table( char *class, int call_proc );
COMMAND_TABLE *get_block_command_table( char *class );
COMMAND_TABLE *get_chara_command_table( char *class, int call_proc );
COMMAND_TABLE *get_mesg_command_table( char *class, char *command, int call_proc );
void output_stage_file( char *infile, char *outfile );
void print_chara_list( void );
void dump_chara_function( char *dump_function );
void set_all_chara_output( void );
void command_proc_call_set( int proc_id );
void set_external_chara_list( char *filename );
void set_external_chara_list_path( char *path );

/* symbol.c */
void init_sym_table( void );
int load_symbol_file( char *filename );
int get_symbol_code( char *name, int flag );
					/* flag は、テーブルにないシンボルを許すかどうか 1:許す*/
void check_duplicate_symbol( void );

/* in convert.c */
void convert_script( char *infile, char *outfile );
void convert_script_block_mode( char *infile, char *outfile );

/* -------------- グループチェック関連 --------------- */

void init_group( void );
void analyze_group( void );
int check_group( char *group, char *tag );

#include "parse.h"
