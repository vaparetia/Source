/*

	Conf-file Parser Grammar

	T. Morita
	09 Jul 2000

*/

#ifndef _PARSE_ALL_H_
#define _PARSE_ALL_H_

typedef unsigned int    u_int  ;
typedef struct node_t   NODE   ;
typedef struct decl_t   DECL   ;
typedef struct effect_t EFFECT ;
typedef struct list_t   LIST   ;
typedef struct work_t   WORK   ;

enum
{
  IS_Constant  = 0x0001,/* 定数である     */
  IS_Literal   = 0x0002,/* 文字列定数である */
  IS_Assigned  = 0x0004,/* 代入されている */
  IS_Function  = 0x0008,/* 関数である     */
  IS_Array     = 0x0010,/* 配列である     */
  IS_StackVar  = 0x0020,/* 特別変数である */
  IS_Variable  = 0x0040,/* 変数である     */
  IS_Expression= 0x0080,/* 式である       */
  IS_Arguments = 0x0100,/* 引数である     */
  IS_Member    = 0x0200,/* effectのメンバーである   */
  IS_BuiltIn   = 0x0400,/* ビルトイン関数である     */
  IS_PointTo   = 0x0800,/* 構造体のメンバーである   */
  IS_PointRef  = 0x1000,/* 構造体のメンバーを参照している */
} ;

struct node_t 
{
	NODE      *parent   ;
	NODE      *left     ;
	NODE      *right    ;

	LIST      *data     ;

	int        type     ; /* char, int, FVECTOR, ... etc */
	int        pointer  ; /* pointer level */
	int        flag     ;

	union
	{
		char  *name     ;
		float  constant ;
		char  *literal  ;
		u_int *address  ;
		int    operand  ;
	} value ;
} ;

struct list_t 
{
	LIST *next  ;
	LIST *prev  ;
	int   count ;
	void *data  ;
} ;

struct effect_t 
{
	char *name    ;
	char *func_nm ; /* 呼び出し関数名前 　　　*/
	int   func_id ; /* 呼び出し関数ＩＤ(LSTファイルの出力に使う)　*/
	LIST *func_ar ; /* 呼び出し関数引数リスト(現在未拡張　ef_createで対応すれば出来る) */

	LIST *option  ; /* オプションコマンド(エフェクト生成に関する拡張コマンド)*/
	LIST *member  ;
	LIST *func_ls ; /* メンバ内に使われている関数リスト */
	LIST *message ; /* メッセージ　*/

	char *comment ; /* エフェクト用コメント */

	LIST *str_var  ;/*未使用*/
	LIST *variable ;/*未使用*/
} ;

struct work_t 
{
	LIST *func_decl  ;/* ビルトイン関数(DummyEffect Export.cppなど広範囲で使用) */
	LIST *effect     ;/* エフェクト(DummyEffect Export.cppなど広範囲で使用) */
	LIST *name_space ;/* 名前空間(構文解析中に一時的なものとして使用) */
	LIST *func_space ;/* 関数空間(構文解析中に一時的なものとして使用)*/
	LIST *name_pool  ;/* 名前を貯めておく場所　散らばった名前のメモリを一気に消すため */
	LIST *mesg_pool  ;/* メッセージ空間(BI_Message.cppで使用する) */
} ;

/*
  effect
    name   　エフェクト名
    func_nm　エフェクト関数名（ＰＳ２上）
    func_id　エフェクト関数ID（ＰＳ２上）
	option(LIST)-+-メンバ１(NODE 子供なし)
				 |   value　		オプションによって型は異なる
				 |   type			オプションコマンドの1～4文字目
				 |   pointer		オプションコマンドの5～8文字目
				 +-...
				 +-メンバＮ(NODE 子供なし)

	member(LIST)-+-メンバ１(NODE 子供なし)
				 |   value.name		メンバの名前
				 |   type, pointer	メンバの型
				 |   data(LIST)-+-動作コード１(NODE　子供あり)　後述
				 |				+-動作コード２(NODE　子供あり)
				 |				+-...
				 |				+-動作コードＮ(NODE　子供あり)
				 +-...
				 +-メンバＮ(NODE 子供なし)

	func_ls(LIST)-+-メンバ１(NODE 子供なし)
				  |   value.name	関数の名前
				  |   type, pointer	関数の返戻値の型
				  |   data(LIST)-+-第一引数 (NODE　子供あり)
				  |				 +-第二引数 (NODE　子供あり)
				  |				 +-...
				  |				 +-第Ｎ引数 (NODE　子供あり)
				  +-...
				  +-メンバＮ(NODE 子供なし)

	message(LIST)-+-メンバ１(NODE 子供なし)
				  |   value　	メッセージの名前
				  |   type　　　メッセージのコマンドID
				  |   data(LIST)-+-第一引数 (NODE　子供あり)
				  |				 +-第二引数 (NODE　子供あり)
				  |				 +-...
				  |				 +-第Ｎ引数 (NODE　子供あり)
				  |				 +-コメント (NODE　子供あり)　あればコメントが最後にくる
				  +-...
				  +-メンバＮ(NODE 子供なし)

  動作コードは、以下のようにツリー上に連ねたものである。';'を一区切りとして式としている。
  動作コード(NODE) 
	+right -+-足し算(NODE)
	|		+right -+-数値(NODE)
	|		+left  -+-変数(NODE)
	+leftt -+-掛け算(NODE)
			+right -+-数値(NODE)
			+left  -+-関数(NODE)
					+ data(LIST)-+-第１引数
								-+-第２引数
								-+-...
								-+-第Ｎ引数
	基本的に、数値、変数、関数は、原始式なので子供(right,left)を持たない。
	このツリーを辿ってコードを再構成することが出来る。


  message

*/

#if C_LANG
#define CPP_EXP
#else
#define CPP_EXP "C"
#endif

extern CPP_EXP  NODE *node_create( void ) ;
extern CPP_EXP  void  node_delete( NODE *n ) ;
extern CPP_EXP  NODE *node_unary( NODE *expr, int operand ) ;
extern CPP_EXP  NODE *node_function( NODE *func, LIST *argument ) ;
extern CPP_EXP  NODE *node_operator( NODE *left, NODE *right, int ope ) ;
extern CPP_EXP  NODE *node_assign( char *name, NODE *expr, int variable ) ;
extern CPP_EXP  NODE *node_pointto( NODE *expr, char *name, int ope  ) ;
extern CPP_EXP  NODE *node_search_flag( NODE *n, int flag ) ;

extern CPP_EXP  LIST *list_regist( LIST *head, void *data, int size ) ;
extern CPP_EXP  void  list_delete( LIST *node ) ;
extern CPP_EXP  void  list_free( LIST *node ) ;
extern CPP_EXP  NODE   *list_search_node( LIST *list, NODE *n, char *name ) ;
extern CPP_EXP  EFFECT *list_search_effect( LIST *list, char *name ) ;
extern CPP_EXP  void   *list_search_idx( LIST *list, int idx ) ;
extern CPP_EXP  NODE   *list_search_option( LIST *list, char *name ) ;

extern CPP_EXP  int type_matching( NODE *typ1, NODE *typ2 ) ;
extern CPP_EXP  int argument_matching( LIST *typ1, LIST *typ2 ) ;


extern CPP_EXP  void yyparse_init( void ) ;
extern CPP_EXP  void yyparse_exit( void ) ;
extern CPP_EXP  int  yyerror( char *s ) ;
extern CPP_EXP  char yyline[] ;
extern CPP_EXP  int  yyinclude_stack_ptr ;
extern CPP_EXP  int  yyinclude_lines[] ;
extern CPP_EXP  char yyinclude_paths[][64] ;
extern CPP_EXP  void yylex_init( void ) ;
extern CPP_EXP  int  yylex( void ) ;
extern CPP_EXP  int  yycolumn ;
extern CPP_EXP  FILE *yyin ;

extern CPP_EXP  int  yyparse( void ) ;	/* パーズする関数コンフファイルを読み出しparse_workに入れる */
extern CPP_EXP  WORK parse_work ;		/* パーズされたデータ (LISTやNODEで表現されている) */
extern CPP_EXP  char debug_buff[4096] ;		/* エラーメッセージ用のバッファ(4096文字まで) */

extern CPP_EXP  char file_path[128] ;		/* コンフィグファイルパス */
#define CONF_FILE "demoeffect.cnf"

#endif /*_PARSE_ALL_H_*/

