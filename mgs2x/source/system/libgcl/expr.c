//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   式の評価関数
   スクラッチパッドをスタックに使う

	1999/07/08 K.Uehara
	$Id: expr.c,v 1.1.1.3 2002/11/19 11:42:42 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	"libgcl.h"

enum {
	OP_END,						/* 終端記号	*/
	OP_MNS, OP_NOT, OP_NEG,				/* 単項演算 */
	OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,		/* 四則演算	*/
	OP_LSL, OP_LSR,								/* 論理シフト */
	OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,	/* 比較演算	*/
	OP_OR, OP_AND, OP_XOR,					/* 論理演算	*/
	OP_OROR, OP_ANDAND,					/* 論理演算	*/
	OP_SET,								/* 代入 */
	OP_COMMA
} ;

static int calc( int op, int value1, int value2 )
{
//	printf( "CALC %x\n", op );
	switch( op ){
	  case OP_MNS: return -value2;
	  case OP_NOT: return !value2;
	  case OP_NEG: return ~value2;
	  case OP_ADD: return ( value1 + value2 );
	  case OP_SUB: return ( value1 - value2 );
	  case OP_MUL: return ( value1 * value2 );
	  case OP_DIV: {
		  GCL_ASSERT( value2 != 0 );
		  return ( value1 / value2 );
	  }
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
	  case OP_COMMA: return value2;
	}
	return 0;
}

typedef struct {
	int value;
	char *ptr;
} EXPR_STACK;

#define MAX_STACK	8

int GCL_Expr( char *data, int *value )
{
	int type;
	unsigned char *p;
	EXPR_STACK *sp;
	EXPR_STACK expr_stack[ MAX_STACK ];

	sp = expr_stack;

	p = ( unsigned char * )data;

	for( ;; ){
		type = *p;

		if( ( type & GCL_OP_MASK ) == GCL_OP ){
			/* 演算子 */
			/* スタックから値を２つ取出し、値をスタックに代入する */
			int op;

			op = type & ~GCL_OP_MASK;
			sp --;
			if( op == OP_END ){
				/* 計算終了 */
				if( value != NULL ){
					*value = sp[ 0 ].value;
				}
				return sp[ 0 ].value;
			} else if( op == OP_SET ){
				/* 代入処理 */
				GCL_ASSERT( sp[ -1 ].ptr != NULL );

				if( GCL_TAG( GCL_GetByte( sp[ -1 ].ptr ) ) == GCL_LOCAL ){
					int no;
					no = GCL_GetByte( sp[ -1 ].ptr ) & 0x0F;
					GCL_SetLocalArgs( no, sp[ 0 ].value );
				} else {
					GCL_SetVar( sp[ -1 ].ptr, sp[ 0 ].value );
				}
				sp[ -1 ].value = sp[ 0 ].value;
			} else {
				sp[ -1 ].value = calc( op, sp[ -1 ].value, sp[ 0 ].value );
				sp[ -1 ].ptr = NULL;
			}
			p ++;
		} else {
			int val;
			sp->ptr = p;
			p = GCL_GetNextValue( p, &type, &val );
			if( type == GCL_BLOCK ){
				GCL_ExecBlockBody( ( char * )val, NULL, 0 );
				sp->value = GCL_Status;
			} else {
				sp->value = val;
			}
			sp++;
		}
	}
}
