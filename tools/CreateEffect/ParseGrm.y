/*

	Conf-file Parser Grammar

	T. Morita
	09 Jul 2000

*/
%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	#include "ParseAll.h"

	WORK parse_work ;
	char debug_buff[4096] = { '\0', } ;
	char file_path[128] ;

	NODE *member ;

	#define PERROR(_s,_n) return (sprintf(debug_buff,(_s)), (_n))
	#define YYPERROR(_s)  return (yyerror(_s),1)
	#define YYPPERROR(_s,_n) return (sprintf(debug_buff,(_s),(_n)),yyerror(""),1)
%}

%union
{
	NODE *node     ;
	LIST *list     ;
	char *string   ;
	float constant ;
	int   code     ;
}

%token<string>   TOKEN_IDENTIFIER TOKEN_STR_VARIABLE
%token<string>   TOKEN_MESSAGE_LITERAL TOKEN_STRING_LITERAL TOKEN_NAME_LITERAL 
%token<constant> TOKEN_CONSTANT

%token<code> TOKEN_AND_OP TOKEN_OR_OP TOKEN_PTR_OP 

%token<code> TOKEN_CHAR TOKEN_SHORT TOKEN_INT TOKEN_LONG
%token<code> TOKEN_UCHAR TOKEN_USHORT TOKEN_UINT TOKEN_ULONG
%token<code> TOKEN_FLOAT TOKEN_VOID
%token<code> TOKEN_FVECTOR TOKEN_IVECTOR TOKEN_SVECTOR TOKEN_FMATRIX
%token<code> TOKEN_CONST
%type <code> type_specifier type_specifier_list unary_operator

%token TOKEN_EFFECT TOKEN_MESSAGE

%type<node> primary_expr postfix_expr unary_expr
%type<node> multiplicative_expr additive_expr and_expr
%type<node> exclusive_or_expr inclusive_or_expr
%type<node> logical_and_expr logical_or_expr conditional_expr
%type<node> expression
%type<node> declarator declarator2 pointer  struct_declaration option_specifier

%type<constant> constant_expr
%type<string> effect_comment

%type<list> parameter_list argument_expr_list struct_option option_specifier_list
%type<list> struct_declaration_specifier struct_declaration_list
%type<list> expressions
%type<list> function_arguments function_argument_list message_arguments message_argument_list

%start file

%%



/*

	Expression

*/
primary_expr
        : TOKEN_IDENTIFIER
		{
			NODE *s = list_search_node( parse_work.name_space, NULL, $1 ) ;

			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->flag = IS_Variable ;
			$$->value.name  = $1 ;
			if ( s )
			{
				$$->type    = s->type    ;
				$$->pointer = s->pointer ;
			}
		}
        | TOKEN_STR_VARIABLE
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->type    = member->type    ;
			$$->pointer = member->pointer ;
			$$->value.name  = $1 ;
			$$->flag = IS_StackVar ;
		}
        | TOKEN_CONSTANT
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->value.constant = $1 ;
			$$->type = TOKEN_FLOAT | 0x4000 ;/* (const float) type*/
			$$->flag = IS_Constant ;
		}
        | TOKEN_STRING_LITERAL
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->value.literal = $1 ;
			$$->type    = TOKEN_CHAR | 0x4000 ;/* (const char *) type*/
			$$->pointer = 1 ;
			$$->flag    = IS_Literal ;
		}
        | '(' expression ')'
		{
			$$ = $2 ;
		}
        ;

postfix_expr
        : primary_expr
        | postfix_expr '(' ')'
		{
			if ( !($$ = node_function( $1, NULL )) )
				YYPERROR( "" ) ;
		}
        | postfix_expr '(' argument_expr_list ')'
		{
			if ( !($$ = node_function( $1, $3 )) )
				YYPERROR( "" ) ;
		}
		| postfix_expr TOKEN_PTR_OP  TOKEN_IDENTIFIER
		{
			if ( !($$ = node_pointto( $1, $3, TOKEN_PTR_OP )) )
				YYPERROR( "" ) ;
		}
        | postfix_expr '.' TOKEN_IDENTIFIER
		{
			if ( !($$ = node_pointto( $1, $3, '.' )) )
				YYPERROR( "" ) ;
		}
        ;

argument_expr_list
        : expression
		{
			$1->flag |= IS_Arguments ;
			$$ = list_regist( NULL, $1, 0 ) ;/* 引数はリスト化する(こっちが先頭) */
		}
        | argument_expr_list ',' expression
		{
			$3->flag |= IS_Arguments ;
			$$ = list_regist( $1, $3, 0 ) ;/* 引数はリスト化する */
		}
        ;

unary_expr
        : postfix_expr
        | unary_operator unary_expr
		{
			if ( !($$ = node_unary( $2, $1 )) )
				YYPERROR( "" ) ;
		}
        ;

unary_operator
        : '&' { $$ = '&' ; }
        | '*' { $$ = '*' ; }
        | '+' { $$ = '+' ; }
        | '-' { $$ = '-' ; }
        | '~' { $$ = '~' ; }
        | '!' { $$ = '!' ; }
        ;

multiplicative_expr
        : unary_expr
        | multiplicative_expr '*' unary_expr
		{
			if ( !($$ = node_operator( $1, $3, '*' )) )
				YYPERROR( "" ) ;
		}
        | multiplicative_expr '/' unary_expr
		{
			if ( !($$ = node_operator( $1, $3, '/' )) )
				YYPERROR( "" ) ;
		}
        ;

additive_expr
        : multiplicative_expr
        | additive_expr '+' multiplicative_expr
		{
			if ( !($$ = node_operator( $1, $3, '+' )) )
				YYPERROR( "" ) ;
		}
        | additive_expr '-' multiplicative_expr
		{
			if ( !($$ = node_operator( $1, $3, '-' )) )
				YYPERROR( "" ) ;
		}
        ;

and_expr
        : additive_expr
        | and_expr '&' additive_expr
		{
			if ( !($$ = node_operator( $1, $3, '&' )) )
				YYPERROR( "" ) ;
		}
        ;

exclusive_or_expr
        : and_expr
        | exclusive_or_expr '^' and_expr
		{
			if ( !($$ = node_operator( $1, $3, '^' )) )
				YYPERROR( "" ) ;
		}
        ;

inclusive_or_expr
        : exclusive_or_expr
        | inclusive_or_expr '|' exclusive_or_expr
		{
			if ( !($$ = node_operator( $1, $3, '|' )) )
				YYPERROR( "" ) ;
		}
        ;

logical_and_expr
        : inclusive_or_expr
        | logical_and_expr TOKEN_AND_OP inclusive_or_expr
		{
			if ( !($$ = node_operator( $1, $3, TOKEN_AND_OP )) )
				YYPERROR( "" ) ;
		}
        ;

logical_or_expr
        : logical_and_expr
        | logical_or_expr TOKEN_OR_OP logical_and_expr
		{
			if ( !($$ = node_operator( $1, $3, TOKEN_OR_OP )) )
				YYPERROR( "" ) ;
		}
        ;

conditional_expr
        : logical_or_expr
        | logical_or_expr '?' logical_or_expr ':' conditional_expr
		{
			PERROR( "Not allowed expression", -8 ) ;
		}
        ;

expression
        : conditional_expr
		| TOKEN_STR_VARIABLE '=' conditional_expr
		{
			if ( !($$ = node_assign( $1, $3, 0 )) )
				YYPERROR( "" ) ;
		}

		| TOKEN_IDENTIFIER   '=' conditional_expr
		{
			if ( !($$ = node_assign( $1, $3, 1 )) )
				YYPERROR( "" ) ;
		}
        ;

expressions
        : expression ';'
		{
			$$ = list_regist( NULL, $1, 0 ) ;/* 式はリスト化する(こっちが先頭) */
		}

        | expressions expression ';'
		{
			$$ = list_regist( $1, $2, 0 ) ;  /* 式はリスト化して持っておく */
		}
        ;

constant_expr
        : conditional_expr
		{
			if ( !($1->flag & IS_Constant) )
				YYPERROR( "Has to be constant." ) ;
			$$ = $1->value.constant ;
			free( $1 ) ;
		}
        ;



/*

	Declarator

*/
declarator
        : declarator2

        | pointer declarator2
		{
			$$ = $2 ;
			$$->type    = $1->type    ;
			$$->pointer = $1->pointer ;
			node_delete( $1 ) ;
		}
        ;

declarator2
        : TOKEN_IDENTIFIER
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->value.name = $1 ;
		}

        | '(' declarator ')'
		{
			$$ = $2 ;
		}

        | declarator2 '[' ']'
		{
			$$ = $1 ;
			$$->flag |= IS_Array ;
			$$->pointer++ ;
		}

        | declarator2 '[' constant_expr ']'
		{
			$$ = $1 ;
			$$->flag |= IS_Array ;
			$$->value.constant = $3 ;
			$$->pointer++ ;
		}

		| declarator2 '(' ')'
		{
			$$ = $1 ;
			$$->flag |= IS_Function ;
		}

        | declarator2 '(' parameter_list ')'
		{
			$$ = $1 ;
			$$->flag |= IS_Function ;
			$$->data = $3 ;
		}
		;

parameter_list
        : type_specifier_list declarator
		{
			$2->flag |= IS_Arguments ;
			$2->type = $1 ;
			$$ = list_regist( NULL, $2, 0 ) ;/* 引数はリスト化する(こっちが先頭) */
		}

        | parameter_list ',' type_specifier_list declarator
		{
			$4->type = $3 ;
			$4->flag |= IS_Arguments ;
			$$ = list_regist( $1, $4, 0 ) ;/* 引数はリスト化する(こっちが先頭) */
		}
        ;


pointer
        : '*'
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->pointer = 1 ;
		}
        | '*' pointer
		{
			$$ = $2 ;
			$$->pointer++ ;
		}
        | '*' type_specifier_list
		{
			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->type= $2 ;
			$$->pointer = 1 ;
		}
        | '*' type_specifier_list pointer
		{
			$$ = $3 ;
			$$->type = $2 ;
			$$->pointer = 1 ;
		}
        ;

type_specifier_list
        : type_specifier
        | TOKEN_CONST type_specifier { $$ = $2 | $1 ; }
        ;

type_specifier
        : TOKEN_CHAR
        | TOKEN_SHORT
        | TOKEN_INT
        | TOKEN_LONG
        | TOKEN_UCHAR
        | TOKEN_USHORT
        | TOKEN_UINT
        | TOKEN_ULONG
        | TOKEN_FLOAT
        | TOKEN_FVECTOR
        | TOKEN_SVECTOR
        | TOKEN_IVECTOR
        | TOKEN_FMATRIX
        | TOKEN_VOID
        ;




/*

	Declaration

*/
declaration
        : effect_specifier ';'
        | message_specifier ';'
        | type_specifier_list declarator ';'
		{
			if ( !$2->value.name )
				YYPERROR( "Function declarations requires Identifier." ) ;
			if ( list_search_node( parse_work.func_decl, NULL, $2->value.name ) )
				YYPERROR( "Duplicate function declarations<%s>." ) ;
			$2->type = $1 ;
			parse_work.func_decl = list_regist( parse_work.func_decl, $2, 0 ) ;
		}
        ;

message_specifier
        : TOKEN_MESSAGE TOKEN_STRING_LITERAL TOKEN_NAME_LITERAL '[' TOKEN_CONSTANT ']' message_arguments
		{
			EFFECT *ef ;
			NODE mesg = { NULL, NULL, NULL, $7, (int)$5, 0, 0, { $3 } } ;
			char name[128] ;

			if ( (ef = list_search_effect( parse_work.effect, $2 )) )
				ef->message = list_regist( ef->message, &mesg, sizeof(NODE) ) ;

			strcpy( name, $2  ) ;
			strcat( name, " " ) ;
			strcat( name, $3  ) ;

			parse_work.name_pool = list_regist( parse_work.name_pool, name, strlen(name)+1 ) ;
			mesg.value.name = parse_work.name_pool->data ;
			parse_work.mesg_pool = list_regist( parse_work.mesg_pool, &mesg, sizeof(NODE) ) ;
		}
		;

message_arguments
		: /* NULL expression */
		{ $$ = NULL ; }
		| message_argument_list
		| TOKEN_STRING_LITERAL
		{
			$$ = list_regist( NULL, $1, 0 ) ;
		}
		| message_argument_list TOKEN_STRING_LITERAL
		{
			$$ = list_regist( $1  , $2, 0 ) ;
		}
		;
message_argument_list
		: TOKEN_MESSAGE_LITERAL
		{
			$$ = list_regist( NULL, $1, 0 ) ;
		}
		| message_argument_list TOKEN_MESSAGE_LITERAL
		{
			$$ = list_regist( $1  , $2, 0 ) ;
		}
		;


effect_specifier
        : TOKEN_EFFECT TOKEN_STRING_LITERAL '[' TOKEN_IDENTIFIER function_arguments ']' struct_option
		{
			parse_work.func_space = NULL ;
			parse_work.name_space = NULL ;
		}
		  struct_declaration_specifier effect_comment
		{
			EFFECT effect = { $2, $4, 0, $5,
							  $7, $9, parse_work.func_space, NULL,
							  $10, NULL, parse_work.name_space } ;
			NODE   *id = list_search_option( effect.option, "id" ) ;
			LIST   *ls ;

			/*他に同じ名前のエフェクトが無いかどうかをチェックする*/
			if ( list_search_effect( parse_work.effect, $2 ) )
				YYPPERROR( "Duplicate EFFECT<%s> declaration was found.\n", $2 ) ;

			/*IDのエフェクトがちゃんとあるかどうかをチェックする*/
			if ( id )
				effect.func_id = (int)id->value.constant ;
			else
				YYPPERROR( "Duplicate EFFECT<%s> declaration was found.\n", $2 ) ;

			/*他に同じIDのエフェクトが無いかどうかをチェックする*/
			for( ls=parse_work.effect ; ls ; ls=ls->next )
				if ( ((EFFECT*)ls->data)->func_id == effect.func_id )
					YYPPERROR( "Duplicate EFFECT<%s> ID was found", $2 ) ;

			parse_work.effect = list_regist( parse_work.effect, &effect, sizeof(EFFECT) ) ;
		}
        ;

effect_comment
		: /* NULL expression */
		{ $$ = NULL ; }
		| TOKEN_STRING_LITERAL
		{ $$ = $1   ; }
		;

function_argument_list
		: TOKEN_STR_VARIABLE
		{
			$$ = list_regist( NULL, $1, 0 ) ;
		}
		| function_argument_list ',' TOKEN_STR_VARIABLE
		{
			$$ = list_regist( $1  , $3, 0 ) ;
		}
		;

function_arguments
		: /* NULL expression */
		{ $$ = NULL ; }
		| '(' ')'
		{ $$ = NULL ; }
		| '(' function_argument_list ')'
		{ $$ = $2   ; }
		;

struct_declaration_specifier
		: /* NULL expression */
		{ $$ = NULL ; }
		| '{' struct_declaration_list '}'
		{ $$ = $2   ; }
		;

struct_declaration_list
        : struct_declaration ';'
		{
			if ( ! $1->value.name )
				YYPERROR( "EFFECT declaration member needs Identifier\n" ) ;

			$$ = list_regist( NULL, $1, 0 ) ;
		}

        | struct_declaration_list struct_declaration ';'
		{
			if ( ! $2->value.name )
				YYPERROR( "EFFECT declaration member needs Identifier\n" ) ;
			if ( list_search_node( $1, $2, NULL ) )
				YYPERROR( "Duplicate member name.\n" ) ;
			$$ = list_regist( $1, $2, 0 ) ;
		}
        ;

struct_declaration
        : type_specifier_list declarator
		{
			$$ = $2 ;
			$$->type  = $1 ;
			$$->flag |= IS_Member ;
		}

        | type_specifier_list declarator
		{
			member = $2 ;
			member->type = $1 ;
		}
		 '{' expressions '}'
		{
			$$ = $2 ;
			$$->type  = $1 ;
			$$->data  = $5 ;
			$$->flag |= IS_Member ;
		}
        ;

struct_option
		: /* NULL expression */
		{ $$ = NULL ; }
		| option_specifier_list
		{ $$ = $1 ; }
		;

option_specifier_list
        : option_specifier
		{
			$$ = list_regist( NULL, $1, sizeof(NODE) ) ;
			free( $1 ) ;
		}

        | option_specifier_list option_specifier
		{
			$$ = list_regist( $1  , $2, sizeof(NODE) ) ;
			free( $2 ) ;
		}
        ;

option_specifier
        : '<' TOKEN_IDENTIFIER ':' conditional_expr '>'
		{
			int i, flag = 1 ;

			if ( !($4->flag & (IS_Constant|IS_Literal)) )
				YYPERROR( "Option argument must be constant." ) ;
			$$ = $4 ;
			$$->pointer = $$->type = 0 ;
			for ( i=0 ; i<4 && flag ; i++ )
				$$->type    |= (flag = $2[i  ]) << (i*8) ;
			for ( i=0 ; i<4 && flag ; i++ )
				$$->pointer |= (flag = $2[i+4]) << (i*8) ;
		}

        | '<' TOKEN_IDENTIFIER '>'
		{
			int i, flag = 1 ;

			if ( !($$ = node_create()) )
				YYPERROR( "" ) ;
			$$->pointer = $$->type = 0 ;
			for ( i=0 ; i<4 && flag ; i++ )
				$$->type    |= (flag = $2[i  ]) << (i*8) ;
			for ( i=0 ; i<4 && flag ; i++ )
				$$->pointer |= (flag = $2[i+4]) << (i*8) ;
		}
		;


/*

	Files

*/
file
        :
		{
			if ( !(yyin = fopen( file_path, "r" ) ) )
			{
			    sprintf( debug_buff, "%sCan't open conf-file<%s>\n", debug_buff, file_path ) ;
				return 1 ;
			}
			yylex_init() ;
			yyparse_init() ;
		}
		 declarations
		{
			fclose( yyin ) ;
		}
        ;

declarations
        : declaration
        | declarations declaration
        ;





%%
/*****************************************************************************
	Parse System
 *****************************************************************************/
void yyparse_init( void )
{
	char *c, *p ;

	for( c=p=file_path ; (c=strchr( p, '\\' )) ; p=c+1 ) ;
	strcpy( yyinclude_paths[yyinclude_stack_ptr], p ) ;

	parse_work.name_pool  = NULL ;
	parse_work.mesg_pool  = NULL ;
	parse_work.func_decl  = NULL ;
	parse_work.effect     = NULL ;
	parse_work.name_space = NULL ;
	parse_work.func_space = NULL ;
}

void yyparse_exit( void )
{
	LIST *l, *e ;

	for( e=parse_work.effect ; e ; e=e->next )
	{
		for( l=((EFFECT*)e->data)->member ; l ; l=l->next )
			node_delete( (NODE *)l->data ) ;
		list_delete( ((EFFECT*)e->data)->func_ar ) ;
		list_delete( ((EFFECT*)e->data)->member  ) ;
		list_delete( ((EFFECT*)e->data)->option  ) ;
		list_delete( ((EFFECT*)e->data)->message ) ;
		list_delete( ((EFFECT*)e->data)->func_ls ) ;
		list_delete( ((EFFECT*)e->data)->variable ) ;
	}
	for( l=parse_work.func_decl ; l ; l=l->next )
		node_delete( (NODE *)l->data ) ;
	list_delete( parse_work.effect ) ;
	list_delete( parse_work.func_decl ) ;
	list_delete( parse_work.name_pool ) ;
	list_delete( parse_work.mesg_pool ) ;
	parse_work.effect = NULL ;
	parse_work.func_decl = NULL ;
	parse_work.func_space = NULL ;
	parse_work.name_space = NULL ;
	parse_work.name_pool = NULL ;
}

int yyerror( char *s )
{
    sprintf( debug_buff, "%s%s\n%*s\n%s in <%s> line %d\n", debug_buff,
			 yyline, yycolumn, "^", s, yyinclude_paths[yyinclude_stack_ptr],  yyinclude_lines[yyinclude_stack_ptr] ) ;
	if ( yyin )
		fclose( yyin ) ;
	yyparse_exit() ;
	return 0 ;
}




/*****************************************************************************
	NODE Control
 *****************************************************************************/
NODE *node_create()
{
	NODE *n = (NODE *)calloc( 1, sizeof(NODE) ) ;

	if ( !n )
		PERROR( "No memory for NODE\n", NULL ) ;
	return n ;
}

void node_delete( NODE *n )
{
	if ( n )
	{
		if ( n->data )
			list_delete( n->data  ) ;
		if ( n->right )
			node_delete( n->right ) ;
		if ( n->left )
			node_delete( n->left  ) ;
		if ( n->parent )
			if ( n->parent->right == n )
				n->parent->right = NULL ;
			else
				n->parent->left  = NULL ;
		free( n ) ;
	}
}

NODE *node_search_flag( NODE *n, int flag )
{
	NODE *ret = NULL ;

	if ( n )
	{
		if ( flag & n->flag )
			return n ;
		if ( n->right )
			ret = node_search_flag( n->right, flag ) ;
		if ( n->left && !ret )
			ret = node_search_flag( n->left , flag ) ;
	}
	return ret ;
}


NODE *node_function( NODE *func, LIST *argument )
{
	/*宣言されている関数リストから探し出す*/
	NODE *s = list_search_node( parse_work.func_decl, NULL, func->value.name ) ;

	if ( !s )
		PERROR( "The function is not defined.", NULL ) ;
	if ( func->flag & (IS_Constant| IS_Literal) )
		PERROR( "Constant cannot be function.", NULL ) ;
	if ( func->flag &  IS_StackVar )
		PERROR( "STR-VARIABLE cannot be function.", NULL ) ;
	/*宣言されている関数リストの方があっているかどうかチェックする*/
	if ( !argument_matching( s->data, argument ) )
		return NULL ;
	func->data    = argument    ;
	func->flag    = IS_Function ;
	func->pointer = s->pointer  ;
	func->type    = s->type     ;

	/*ビルトイン関数かどうかのチェック*/
	if ( strstr( func->value.name, "MaxInput" ) == func->value.name )
		func->flag |= IS_BuiltIn  ;

	/*EFFECT構造体にそのエフェクトに使っている関数リストとなる*/
	parse_work.func_space = list_regist( parse_work.func_space, func, 0 ) ;

	return func ;
}

NODE *node_pointto( NODE *expr, char *member, int operand )
{
	NODE *new ;

	if ( expr->flag & IS_Constant )
		PERROR( "Constant is not allowed to have pointer or member operator\n", NULL ) ;
	else if ( expr->flag & (IS_Variable| IS_StackVar| IS_Function ) )
		if ( (expr->type & 0x0fff) < TOKEN_FVECTOR )
			PERROR( "Does not have any member to refer.\n", NULL ) ;
		else if ( operand == TOKEN_PTR_OP )
		{
			if ( !expr->pointer )
				PERROR( "Reference of member must be '.'.\n", NULL ) ;
		}
		else
		{
			if ( expr->pointer )
				PERROR( "Reference of member must be '->'.\n", NULL ) ;
		}
	else
		PERROR( "Illeagal PointTo member operator.\n", NULL ) ;

	/*operandは、生成時にrightの子供がポインタかどうかで判断する*/
	if ( !(new = node_create()) )
		PERROR( "No memory.\n", NULL ) ;
	new->right = expr ;
	new->flag  = IS_PointTo ;
	new->value.name = member ;
	switch ( expr->type & 0x0fff )
	{
	case TOKEN_FVECTOR:
	case TOKEN_FMATRIX:
		new->type = TOKEN_FLOAT ;
		break ;
	case TOKEN_SVECTOR:
		new->type = TOKEN_SHORT ;
		break ;
	case TOKEN_IVECTOR:
		new->type = TOKEN_INT   ;
		break ;
	}
	expr->flag |= IS_PointRef ;

	return new ;
}


NODE *node_unary( NODE *expr, int operand )
{
	if ( expr->flag & IS_Constant )
	{
		if ( operand == '-' )
			expr->value.constant = (float)( -     expr->value.constant ) ;
		else if ( operand == '~' )
			expr->value.constant = (float)( ~(int)expr->value.constant ) ;
		else if ( operand == '!' )
			expr->value.constant = (float)( !(int)expr->value.constant ) ;
		else
			PERROR( "Constant is not allowed to have pointer operator\n", NULL ) ;
	}
	else if ( expr->flag & (IS_Variable| IS_StackVar| IS_Function ) )
	{
		if ( operand == '&' )
			expr->pointer++ ;
		else if ( operand == '*' )
		{
			if ( expr->pointer <= 0 )
				PERROR( "No faurther pointer operator\n", NULL ) ;
			expr->pointer-- ;
		}
		else
			PERROR( "Variable is not allowed to have constant unary operator\n", NULL ) ;
	}
	else
		PERROR( "Illeagal UNARY operator.\n", NULL ) ;
	return expr ;
}

NODE *node_operator( NODE *left, NODE *right, int ope )
{
	NODE *new ;

	if ( !type_matching( left, right ) )
		PERROR( "\n", NULL ) ;
	if ( left->flag & IS_Constant && right->flag & IS_Constant )/* 定数ならノードを増やさず畳み込む */
	{
		new = right ;
		if ( ope == '+' )
			new->value.constant = left->value.constant + right->value.constant ;
		else if ( ope == '-' )
			new->value.constant = left->value.constant - right->value.constant ;
		else if ( ope == '/' )
			new->value.constant = left->value.constant / right->value.constant ;
		else if ( ope == '*' )
			new->value.constant = left->value.constant * right->value.constant ;
		else if ( ope == '&' )
			new->value.constant = (float)((int)left->value.constant &  (int)right->value.constant) ;
		else if ( ope == '|' )
			new->value.constant = (float)((int)left->value.constant |  (int)right->value.constant) ;
		else if ( ope == '^' )
			new->value.constant = (float)((int)left->value.constant ^  (int)right->value.constant) ;
		else if ( ope == TOKEN_AND_OP )
			new->value.constant = (float)((int)left->value.constant && (int)right->value.constant) ;
		else if ( ope == TOKEN_OR_OP  )
			new->value.constant = (float)((int)left->value.constant || (int)right->value.constant) ;
		node_delete( left ) ;
	}
	else/* ノードを作り左右の葉にくっ付ける */
	{
		if ( !(new = node_create()) )
			return NULL ;
		new->left  = left ;
		new->right = right ;
		new->type  = left->type ;
		new->flag  = IS_Expression ;
		new->value.operand = ope ;
	}

	return new ;
}


NODE *node_assign( char *name, NODE *expr, int variable )
{
	NODE *n ;
	NODE *s ;

	if ( variable )
	{
		if ( (s = list_search_node( parse_work.name_space, NULL, name )) )
			if ( !type_matching( expr, s ) )
				PERROR( "\n", NULL ) ;
	}
	else if ( !type_matching( expr, member ) )
		PERROR( "\n", NULL ) ;

	
	if ( !(n = node_create()) )
		return NULL ;
	n->right   = expr ;
	n->type    = expr->type ;
	n->pointer = expr->pointer ;
	n->flag = IS_Assigned ;
	n->value.name  = name ;

	if ( variable )
		n->flag |= IS_Variable ;
	else
		n->flag |= IS_StackVar ;

	/*変数を登録する*/
	parse_work.name_space = list_regist( parse_work.name_space, n, 0 ) ;

	return n ;
}





/*****************************************************************************
	LIST Control
 *****************************************************************************/
LIST *list_regist( LIST *head, void *data, int size )
{
	LIST *node ;

	if ( size > 0 )/* サイズが正の数だったら、その大きさの分だけ dataに size分だけコピーする */
	{
		node = (LIST *)malloc( sizeof(LIST) + size ) ;
		node->data = (void *)( (int)node + sizeof(LIST) ) ;
		memcpy( node->data, data, size ) ;
	}
	else           /* サイズが０以下だったら、dataにポインタをコピーする */
	{
		node = (LIST *)malloc( sizeof(LIST) ) ;
		node->data = data ;
	}

	/* リストを接合する prevはリングリストにし、nextは、NULLターミネイト */
	if ( head )
		node->count = head->count+1, node->prev = head->prev, head->prev = node ;
	else
		node->count = 1            , node->prev = node ;
	node->next = head ;

	return node ;
}

/*
void list_remove( LIST *node )
{
	if ( node )
	{
		if ( node->prev )
			node->prev->next = node->next ;
		if ( node->next )
			node->next->prev = node->prev ;
		free( node ) ;
	}
}
*/
void list_delete( LIST *node )
{
	LIST *next ;

	while( node )
	{
		next = node->next ;
		free( node ) ;
		node = next ;
	}
}


NODE *list_search_node( LIST *list, NODE *n, char *name )
{
	name = name ? name : n ? n->value.name : "-never-come-here-" ;
	while( list )
	{
		if ( list->data )
			if ( !strcmp( ((NODE*)list->data)->value.name, name ) )
				return (NODE *)list->data ;
		list = list->next ;
	}
	return NULL ;
}

EFFECT *list_search_effect( LIST *list, char *name )
{
	while( list )
	{
		if ( list->data )
			if ( !strcmp( ((EFFECT*)list->data)->name, name ) )
				return (EFFECT *)list->data ;
		list = list->next ;
	}
	return NULL ;
}

void *list_search_idx( LIST *list, int idx )
{
	while( list )
	{
		if ( list->count == idx )
			return list->data ;
		list = list->next ;
	}
	return NULL ;
}

NODE *list_search_option( LIST *list, char *name )
{
	NODE *node ;
	int i, flag=1 ;
	int type, pointer ;

	type = pointer = 0 ;
	for ( i=0 ; i<4 && flag ; i++ )
		type    |= (flag = name[i  ]) << (i*8) ;
	for ( i=0 ; i<4 && flag ; i++ )
		pointer |= (flag = name[i+4]) << (i*8) ;

	if ( strlen( name ) >= 2 )
		while( list )
		{
			if ( (node = (NODE*)list->data) )
				if ( node->type==type && node->pointer==pointer )
					return node ;
			list = list->next ;
		}
	return NULL ;
}



/*****************************************************************************
	Type Matching
 *****************************************************************************/
int type_matching( NODE *typ1, NODE *typ2 )
{
	if ( !typ1->type && !typ2->type ) /*　両方の型が決まっていない　*/
		PERROR( "Both types are indecidable\n", 0 ) ;
	if ( !typ1->type )
		typ1->pointer = typ2->pointer, typ1->type = typ2->type ;/*型が決まってなかったので合わせる*/
	else if ( !typ2->type )
		typ2->pointer = typ1->pointer, typ2->type = typ1->type ;/*型が決まってなかったので合わせる*/
	else if ( (typ1->type == TOKEN_VOID && typ1->pointer<=0) || (typ2->type == TOKEN_VOID && typ2->pointer<=0) )
	{
		strcat( debug_buff, "VOID does not match at all." ) ;
		return 0 ;
	}
	else if ( typ1->type == TOKEN_VOID )
	{
		if ( typ2->pointer<=0 )
		{
			strcat( debug_buff, "Miss match between VOID pointer and non-pointer value." ) ;
			return 0 ;
		}
	}
	else if ( typ2->type == TOKEN_VOID )
	{
		if ( typ1->pointer<=0 )
		{
			strcat( debug_buff, "Miss match between VOID pointer and non-pointer value." ) ;
			return 0 ;
		}
	}
	else if ( typ1->pointer != typ2->pointer )
	{
		sprintf( debug_buff, "Type miss match (%s %d,%d==%s %d,%d).",
			typ1->value.name, typ1->type, typ1->pointer, typ2->value.name, typ2->type, typ2->pointer ) ;
		return 0 ;
	}
	else if (  typ1->pointer )
	{
		if ( (typ1->type & ~0xc000) != (typ2->type & ~0xc000) )
		{
			sprintf( debug_buff, "Type miss match (%s %d,%d==%s %d,%d).",
				typ1->value.name, typ1->type, typ1->pointer, typ2->value.name, typ2->type, typ2->pointer ) ;
			return 0 ;
		}
	}
	else if  ( (typ1->type & ~0xc000) > TOKEN_LONG && (typ2->type & ~0xc000) > TOKEN_LONG )
		if ( (typ1->type & ~0xc000) != (typ2->type & ~0xc000) )
		{
			sprintf( debug_buff, "Type miss match (%s %d,%d==%s %d,%d).",
				typ1->value.name, typ1->type, typ1->pointer, typ2->value.name, typ2->type, typ2->pointer ) ;
			return 0 ;
		}
	
	return 1 ;/* 型は合っているものとする */
}

int argument_matching( LIST *typ1, LIST *typ2 )
{
	char msg[4096] = { '\0' } ;

	if ( !typ1 && !typ2 )/*両方引数が無いということ*/
		return 1 ;
	else if ( !typ2 )
		strcat( msg, "Too few arguments\n"  ) ;
	else if ( !typ1 )
		strcat( msg, "Too many arguments\n"  ) ;
	else if ( typ1->count == typ2->count )
		while( typ1 && typ2 )
		{
			if ( !type_matching( (NODE*)typ1->data, (NODE*)typ2->data ) )
				sprintf( msg, "%s Type miss matches on %d%s argument\n", msg, typ1->count,
						 typ1->count==1 ? "st" :
						 typ1->count==2 ? "nd" :
						 typ1->count==3 ? "rd" : "th" ) ;
			typ1 = typ1->next ;
			typ2 = typ2->next ;
		}
	else if ( typ1->count > typ2->count )
		strcat( msg, "Too few arguments\n"  ) ;
	else
		strcat( msg, "Too many arguments\n" ) ;
	if ( strlen(msg) )
		PERROR( msg, 0 ) ;
	return 1 ;
}
