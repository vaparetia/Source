/*
	gclconv 式解釈ルーチン

	解釈後の構文ツリーを返す物と、値を計算して返す物を両方用意する。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mymalloc.h"
#include "gclconv.h"
#include "expr.h"

	/*
		優先順位
	*/
static	short	expr_priorities[] = {
	255,			/* 終端記号		*/
	1, 1, 1,
	3, 3, 2, 2, 2,		/* 四則演算		*/
	4, 4,				/* 論理シフト */
	6, 6, 5, 5, 5, 5,	/* 比較演算		*/
	10, 10, 10,			/* 論理演算		*/
	11, 11,			/* 論理演算		*/
	12
} ;

void init_expr( EXPR_INFO *exinfo, char term_char )
{
	exinfo->op_buf = my_malloc( sizeof( OPERAND ) * EXPR_MAX_OP_BUF );
	exinfo->tmp_var_work = my_malloc( sizeof( char ) * EXPR_MAX_TMP_VER_WORK );
	
	exinfo->op_buf_p = 0;
	exinfo->tmp_var_work_p = exinfo->tmp_var_work;
	exinfo->term_char = term_char;
}

void close_expr( EXPR_INFO *exinfo )
{
	my_free( exinfo->op_buf );
	my_free( exinfo->tmp_var_work );
}

static OPERAND *new_operand( EXPR_INFO *exinfo )
{
	OPERAND *new;

	new = &exinfo->op_buf[ exinfo->op_buf_p ++ ];
	if( exinfo->op_buf_p >= EXPR_MAX_OP_BUF ){
		ERROR( "OPBUF OVER\n" );
	}
	new->prev.type = new->next.type = TYPE_UNSET;
	new->operand = OP_END;
	return new;
}

static char *set_expr_variable( NODE *node, EXPR_INFO *exinfo, char *buf )
{
	char *p;
	WORDBUF word;
	
	p = exinfo->tmp_var_work_p;
	strcpy( p, buf );
	p = p + strlen( buf ) + 1;

	node->type = TYPE_VARIABLE;
	node->value = ( int )exinfo->tmp_var_work_p;

	if( get_word( word ) != NULL ){
		if( word[ 0 ] == '[' ){
			int nest = 0;
			/* 配列変数:配列名の後に追加していく */
			node->type = TYPE_ARRAY;
			for( ;; ){
				/* とりあえず配列の添字もバッファにためる */
				strcpy( p, word );
				p = p + strlen( word );
				if( word[ 0 ] == '[' ){
					nest ++;
				} else if( word[ 0 ] == ']' ){
					--nest;
					if( nest == 0){
						break;
					}
				}
				if( get_word( word ) == NULL ){
					ERROR( "配列の添字が途中で終わっています\n" );
				}
			}
		} else {
PRINTF( "EXPR_UNGET\n" );
			unget_word( word );
		}
	}
	exinfo->tmp_var_work_p = p;

	return p;
}

static char *get_expr_word( char *buf )
{
	if( get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

unsigned int get_expr_strcode( char *buf )
{
	/* 24Bit or 24Bit + 8Bit を返す */
	/* buf は書き換えられて、baseのみになっている */
	char *p;

	if( ( p = strchr( buf, ':' ) ) == NULL ){
		return get_strcode( buf );
	} else {
		char *pp;
		unsigned int v = 0;
		if( *( p + 1 ) == '\0' ){
			ERROR( "name:num / 数字が必要です\n" );
		}
		for( pp = p + 1; *pp != '\0'; pp++ ){
			int c;
			c = *pp | 0x20;
			if( c >= '0' && c <= '9' ){
				v = ( v * 16 ) + ( *pp - '0' );
			} else if( c >= 'a' && c <= 'f' ){
				v = ( v * 16 ) + ( c - 'a' + 10 );
			} else {
				ERROR( "不正な16進数値です\n" );
			}
		}
		if( v < 0 || v > 255 ){
			ERROR( "name:num 名前の後の数値(%d)が大きすぎます。\n", v );
		}
		*p = '\0';

		return get_strcode( buf ) | ( ( unsigned int )v << 24 );
	}
}

int is_expr_num( char *buf, int *value )
{
	char *p;
	int v;
	int minus_flag;

	v = 0;
	if( buf[ 0 ] == '-' ){
		minus_flag = 1;
		buf ++;
	} else {
		minus_flag = 0;
	}
	if( buf[ 0 ] == '0' ){
		if( ( buf[ 1 ] | 0x20 ) == 'x' ){
			for( p = buf + 2; *p != '\0'; p++ ){
				int c;
				if( *p == '_' ) continue;
				c = *p | 0x20;
				if( c >= '0' && c <= '9' ){
					v = ( v * 16 ) + ( *p - '0' );
				} else if( c >= 'a' && c <= 'f' ){
					v = ( v * 16 ) + ( c - 'a' + 10 );
				} else {
					ERROR( "不正な16進数値です\n" );
				}
			}
			goto END;
		} else if( ( buf[ 1 ] | 0x20 ) == 'b' ){
			for( p = buf + 2; *p != '\0'; p++ ){
				int c;
				c = *p;
				if( c == '_' ) continue;
				if( c == '0' || c == '1' ){
					v = ( v << 1 ) | ( c - '0' );
				} else {
					ERROR( "不正な2進数値です\n" );
				}
			}
			goto END;
		}
	}
	if( strchr( buf, '.' ) != NULL ){
		/* FLOAT */
		float val = 0.0F;
		float pf = 0.1F;

		for( p = buf; *p != '.'; p++ ){
			if( *p >= '0' && *p <= '9' ){
				val = ( val * 10.0F ) + ( *p - '0' ) * 1.0F;
			} else {
				return 0;
			}
		}
		for( p = p + 1; *p != '\0'; p++ ){
			if( *p == 'F' || *p == 'f' ) continue;
			if( *p >= '0' && *p <= '9' ){
				val = val + ( *p - '0' ) * pf;
				pf = pf * 0.1F;
			} else {
				return 0;
			}
		}
		PRINTF( "FLOAT %f\n", val );
		if( minus_flag ) val = -val;
		*value = *( int * )&val;
		return 2;
	}
	for( p = buf; *p != '\0'; p++ ){
		if( *p >= '0' && *p <= '9' ){
			v = ( v * 10 ) + ( *p - '0' );
		} else {
			return 0;
		}
	}
END:
	if( minus_flag ) v = -v;
	*value = v;
	return 1;
}

OPERAND *set_expr( EXPR_INFO *exinfo );

static char alone_operands_char[] = "-!~";
static int alone_operands[] = {
	OP_MNS, OP_NOT, OP_NEG
};

static int get_node( EXPR_INFO *exinfo, NODE *node )
{
	WORDBUF buf;
	char *p;

	get_expr_word( buf );
	if( strlen( buf ) == 1 && ( p = strchr( alone_operands_char, *buf ) ) != NULL ){
		/* 単項演算子 */
		/* ダミーで演算対象に０を入れておく */
		OPERAND *new;
		new = new_operand( exinfo );
		new->prev.type = TYPE_VALUE;
		new->prev.value = 0;
		new->operand = alone_operands[ p - alone_operands_char ];
		new->operand_pri = expr_priorities[ new->operand ];
		get_node( exinfo, &new->next );

		node->type = TYPE_OPERAND;
		node->value = ( int )new;
		return 1;
	} else if( *buf == '$' ){
		/* 変数 */
		/* とりあえず変数名をテンポラリのバッファにためておく */
		set_expr_variable( node, exinfo, buf );
	} else if( *buf == '(' ){
		/* ( .. ) ブロック */
		node->type = TYPE_BLOCK;
		/* 再帰的に式の評価ルーチンを呼び出す */
		node->value = ( int )set_expr( exinfo );
	} else if( *buf == '\'' ){
		unsigned int c;
		/* 'RSRC'型定数 */
PRINTF( "BUF = %s\n", buf );
		c = 0;
		for( p = buf + 1; *p != '\'' && *p != '\0'; p++ ){
			c = ( c << 8 ) | *p;
		}
		node->type = TYPE_VALUE;
		node->value = c;
	} else {
		/* 数字が来るはず */
		int res = is_expr_num( buf, &node->value );
		if( res == 1  ){
			node->type = TYPE_VALUE;
		} else if( res == 2 ){
			node->type = TYPE_FLOAT;
		} else {
			/* 文字列コードにする */
			node->type = TYPE_STRID;
			node->value = get_expr_strcode( buf );
//			node->value = get_strcode( buf );
		}
	}
	return 0;
}

static void get_operand( OPERAND *op, char *buf )
{
	int id;

	if( buf[ 1 ] != '\0' ){
		ERROR( "不正な演算子です\n" );
	}

	switch( *buf ){
	  case '+':		id = OP_ADD; break;
	  case '-':		id = OP_SUB; break;
	  case '*':		id = OP_MUL; break;
	  case '/':		id = OP_DIV; break;
	  case '%':		id = OP_MOD; break;
	  case '^':		id = OP_XOR; break;
	  case '=':
		if( get_next_char() == '=' ){
			get_expr_word( buf );
			id = OP_EQ;
		} else {
			id = OP_SET;
		}
		break;
	  case '!':
		if( get_next_char() == '=' ){
			get_expr_word( buf );
			id = OP_NE;
		} else {
			ERROR( "不正な演算子です\n" );
		}
		break;
	  case '<':
		{
			int c;
			c = get_next_char();
			if( c == '=' ){
				get_expr_word( buf );
				id = OP_LE;
			} else if( c == '<' ){
				get_expr_word( buf );
				id = OP_LSL;
			} else {
				id = OP_LT;
			}
		}
		break;
	  case '>':
		{
			int c;
			c = get_next_char();
			if( c == '=' ){
				get_expr_word( buf );
				id = OP_GE;
			} else if( c == '>' ){
				get_expr_word( buf );
				id = OP_LSR;
			} else {
				id = OP_GT;
			}
		}
		break;
	  case '|':
		if( get_next_char() == '|' ){
			get_expr_word( buf );
			id = OP_OROR;
		} else {
			id = OP_OR;
		}
		break;
	  case '&':
		if( get_next_char() == '&' ){
			get_expr_word( buf );
			id = OP_ANDAND;
		} else {
			id = OP_AND;
		}
		break;
	  default:
		ERROR( "不正な演算子です\n" );
	}
	op->operand = id;
	op->operand_pri = expr_priorities[ id ];
}

OPERAND *set_expr( EXPR_INFO *exinfo )
{
	WORDBUF buf;
	OPERAND *top;

	top = new_operand( exinfo );
	get_node( exinfo, &top->next );
	top->operand = OP_END;
	top->operand_pri = expr_priorities[ OP_END ];

	while( get_word( buf ) != NULL ){
		/* 次に来るのは演算子か終端 */
		if( *buf == exinfo->term_char ){
			exinfo->term_flag = 1;
			return top;
		}
		{
			OPERAND *new, *p;
			/* 演算子 */
			new = new_operand( exinfo );
			get_operand( new, buf );

			/* 優先順位を加味してどこにいれるか検索する */
			for( p = top ;; ){
				int type;
				type = p->next.type;
				if( type != TYPE_OPERAND ){
					/* 次が演算子以外ならそこにいれる */
					break;
				} else {
					OPERAND *next;
					next = ( OPERAND * )p->next.value;
					if( next->operand_pri <= new->operand_pri ){
						/* 次の演算子の優先が自分と同じか高ければ、その前 */
						/* 優先は高いほどpriは小 */
						break;
					} else {
						/* もう1段階下がってみる */
						p = next;
					}
				}
			}
			/* 優先順位の正しい位置に追加 */
			new->prev = p->next;
			p->next.type = TYPE_OPERAND;
			p->next.value = ( int )new;
			/* この演算子の被演算数を取得 */
			get_node( exinfo, &new->next );
		}
	}
	exinfo->term_flag = 0;
	return top;
}

#define VAR_DEFINE 1
#define VAR_REFERD 0

static int calc_node( EXPR_INFO *exinfo, NODE *node )
{
PRINTF( "CALC_NODE\n" );
	/* 定数値なら1、それ以外なら０を返す */
	switch( node->type ){
	  case TYPE_FLOAT:
		return 2;
	  case TYPE_VALUE:
	  case TYPE_STRID:
		return 1;
	  case TYPE_BLOCK:
	  case TYPE_OPERAND:
		{
			int value;
			int res;
			res = calc_expr( exinfo, ( OPERAND * )node->value, &value );
			if( res == 1 ){
				node->type = TYPE_VALUE;
				node->value = value;
				return 1;
			} else if( res == 2 ){
				node->type = TYPE_FLOAT;
				node->value = value;
				return 2;
			}
			break;
		}
	}
	return 0;
}

static int calc_opr( OPERAND *op )
{
	int value1, value2;

	value1 = op->prev.value;
	value2 = op->next.value;

	switch( op->operand ){
	  case OP_MNS: return -value2;
	  case OP_NOT: return !value2;
	  case OP_NEG: return ~value2;
	  case OP_ADD: return ( value1 + value2 );
	  case OP_SUB: return ( value1 - value2 );
	  case OP_MUL: return ( value1 * value2 );
	  case OP_DIV: return ( value1 / value2 );
	  case OP_MOD: return ( value1 % value2 );
	  case OP_LSL: return ( ( unsigned int )value1 << value2 );
	  case OP_LSR: return ( ( unsigned int )value1 >> value2 );
	  case OP_EQ:  return ( value1 == value2 );
	  case OP_NE:  return ( value1 != value2 );
	  case OP_LT:  return ( value1 < value2 );
	  case OP_LE:  return ( value1 <= value2 );
	  case OP_GT:  return ( value1 > value2 );
	  case OP_GE:  return ( value1 >= value2 );
	  case OP_OR:  return ( value1 | value2 );
	  case OP_AND: return ( value1 & value2 );
	  case OP_XOR: return ( value1 ^ value2 );
	  case OP_OROR:   return ( value1 || value2 );
	  case OP_ANDAND: return ( value1 && value2 );
	  default:
		PRINTF( "calc_opr %d\n", op->operand );
	}
	return 0;
}

static float calc_opr_float( OPERAND *op )
{
	float value1, value2;

	if( op->prev.type == TYPE_FLOAT ){
		value1 = *( float * )&op->prev.value;
	} else {
		value1 = ( float )op->prev.value;
	}
	if( op->next.type == TYPE_FLOAT ){
		value2 = *( float * )&op->next.value;
	} else {
		value2 = ( float )op->next.value;
	}
PRINTF( "CALC FLOAT %d, %f %f\n", op->operand, value1, value2 );
	switch( op->operand ){
	  case OP_MNS: return -value2;
	  case OP_NOT: return !value2;
	  case OP_ADD: return ( value1 + value2 );
	  case OP_SUB: return ( value1 - value2 );
	  case OP_MUL: return ( value1 * value2 );
	  case OP_DIV: return ( value1 / value2 );
	  case OP_EQ:  return ( value1 == value2 );
	  case OP_NE:  return ( value1 != value2 );
	  case OP_LT:  return ( value1 < value2 );
	  case OP_LE:  return ( value1 <= value2 );
	  case OP_GT:  return ( value1 > value2 );
	  case OP_GE:  return ( value1 >= value2 );

	  case OP_MOD:
	  case OP_NEG:
	  case OP_LSL:
	  case OP_LSR:
	  case OP_OR:
	  case OP_AND:
	  case OP_XOR:
	  case OP_OROR:
	  case OP_ANDAND:
		ERROR( "FLOATには使用できません\n" );
		break;
	  default:
		PRINTF( "calc_opr %d\n", op->operand );
	}
	return 0;
}

int calc_expr( EXPR_INFO *exinfo, OPERAND *top, int *value )
{
	/* 各ノードが定数演算だった場合、できるだけ計算してノードをまとめる */
	/* 結果が定数になったら１を返す */

	int flag;
PRINTF( "CALC_EXPR\n" );
	flag = ( top->operand == OP_SET ) ? VAR_DEFINE : VAR_REFERD;
	if( flag == VAR_DEFINE ){
		/* 代入先が変数かどうかをチェック */
		if( top->prev.type != TYPE_VARIABLE && top->prev.type != TYPE_ARRAY ){
			ERROR( "変数以外に代入しようとしています\n" );
		}
		if( top->next.type == TYPE_VALUE ){
			/* 代入元と代入先の型チェック */
			if( ( ( char * )top->prev.value )[ 1 ] == 's' ){
				ERROR( "数値定数を文字列型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		} else if( top->next.type == TYPE_STRID ){
			if( ( ( char * )top->prev.value )[ 1 ] != 's' ){
				ERROR( "文字列定数を数値型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		}
	}

	if( top->operand != OP_END ){
		int flag1, flag2;
		flag1 = calc_node( exinfo, &( top->prev ) );
		flag2 = calc_node( exinfo, &( top->next ) );
		if( flag1 != 0 && flag2 != 0 ){
			/* 定数値なので、計算する */
			if( flag1 == 2 || flag2 == 2 ){
				float tmp;
				tmp = calc_opr_float( top );
				*value = *( int * )&tmp;
				return 2;
			} else {
				*value = calc_opr( top );
				return 1;
			}
		}
	} else {
		if( calc_node( exinfo, &( top->next ) ) ){
			*value = top->next.value;
			if( top->next.type == TYPE_FLOAT ) {
				return 2;
			} else {
				return 1;
			}
		}
	}
	return 0;
}

#if 0

static void output_expr( OPERAND *top );

static void output_node( NODE *node, int flag )
{
	switch( node->type ){
	  case TYPE_UNSET:
		PRINTF( "START " );
		break;
	  case TYPE_VALUE:
#if 0
		if( node->value == ( unsigned char )node->value ){
			set_byte( bi, GCL_BYTE );
			set_byte( bi, node->value );
		} else {
			set_byte( bi, GCL_SHORT );
			set_short( bi, node->value );
		}
#endif
		PRINTF( "%d ", node->value );
		break;
	  case TYPE_STRID:
#if 0
		set_byte( bi, GCL_STRID );
		set_short( bi, node->value );
#endif
		break;
	  case TYPE_VARIABLE:
#if 0
		set_variable( bi, ( char * )node->value, flag );
#endif
		PRINTF( "%s ", ( char * )node->value );
		break;
	  case TYPE_BLOCK:
		PRINTF( "BLOCK " );
	  case TYPE_OPERAND:
		output_expr( ( OPERAND * )node->value );
	}
}

static void output_expr( OPERAND *top )
{
	static char *operands[] = {
		"OP_END",						/* 終端記号	*/
		"OP_MNS", "OP_NOT",	"OP_NEG",					/* 単項演算 */
		"OP_ADD", "OP_SUB", "OP_MUL", "OP_DIV", "OP_MOD",		/* 四則演算	*/
		"OP_EQ", "OP_NE", "OP_LT", "OP_LE", "OP_GT", "OP_GE",	/* 比較演算	*/
		"OP_OR", "OP_AND",  "OP_XOR",				/* 論理演算	*/
		"OP_OROR", "OP_ANDAND",					/* 論理演算	*/
		"OP_SET"								/* 代入 */
	};

	int flag;
	flag = ( top->operand == OP_SET ) ? VAR_DEFINE : VAR_REFERD;
	if( flag == VAR_DEFINE ){
		/* 代入先が変数かどうかをチェック */
		if( top->prev.type != TYPE_VARIABLE ){
			ERROR( "変数以外に代入しようとしています\n" );
		}
		if( top->next.type == TYPE_VALUE ){
			/* 代入元と代入先の型チェック */
			if( ( ( char * )top->prev.value )[ 1 ] == 's' ){
				ERROR( "数値定数を文字列型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		} else if( top->next.type == TYPE_STRID ){
			if( ( ( char * )top->prev.value )[ 1 ] != 's' ){
				ERROR( "文字列定数を数値型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		}
	}
	if( top->operand != OP_END ){
		output_node( &( top->prev ), flag );
		output_node( &( top->next ), VAR_REFERD );
#if 0
		set_byte( bi, GCL_OP );
		set_byte( bi, top->operand );
#endif
		PRINTF( "%s ", operands[ top->operand ] );
	} else {
		output_node( &( top->next ), VAR_REFERD );
	}
}

static int analyze_expr( BUFFER_INFO *bi )
{
	/* その行の終端で終わったら 0、
	   最後がマッチしない')'なら、1 */
	char *top, *len;
	int l;
	OPERAND *op;

	set_byte( bi, GCL_EXPR );
	top = len = get_now_pos( bi );
	set_byte( bi, 0 );			/* length */

	parse_expr_mode();
	init_op_buf();

	op = set_expr();

	output_expr( bi, op );
	set_byte( bi, GCL_OP );
	set_byte( bi, GCL_END );
	PRINTF( "\n" );

	parse_normal_mode();

	if( ( l = get_now_pos( bi ) - top ) > 255 ){
		ERROR( "式が複雑すぎます\n" );
	}
	*len = l;
	return term_flag;
}

extern int verbose_mode;

void expr_test( void )
{
	EXPR_INFO info, *ep;
	ep = &info;

	verbose_mode = 1;
	init_op_buf( ep );

	parse_expr_mode();

	while( get_line() ){
		WORDBUF word;

		if( get_word( word ) != NULL && word[ 0 ] == '(' ){
			OPERAND *top;
			int value;

			top = set_expr( ep );
			if( calc_expr( ep, top, &value ) ){
				PRINTF( "\n\n" );
				printf( "VALUE = %d\n", value );
			} else {
				PRINTF( "\n\n" );
				output_expr( top );
				PRINTF( "\n" );
			}
		}
	}

	close_op_buf( ep );
}

#endif
