/*

	Config Lexical Parser

	07/06/2000 T.Morita

*/

/* the "incl" state is used for picking up the name of an include file*/
%x incl

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ParseAll.h"
#include "ParseGrm.h"

#define MAX_INCLUDE_DEPTH 10
#define YYERROR(_s,_n)	return (sprintf(debug_buff,(_s),(_n)),YY_NULL)

void yycount( void ) ;
void yystring( void ) ;
void yyliteral( void ) ;

int  yycolumn     ;
char yyline[4096] ;

int             yyinclude_stack_ptr ;
YY_BUFFER_STATE yyinclude_stack[MAX_INCLUDE_DEPTH] ;
int             yyinclude_lines[MAX_INCLUDE_DEPTH] ;
char            yyinclude_paths[MAX_INCLUDE_DEPTH][64] ;
%}

D	[0-9]
L	[a-zA-Z_]
A	({D}|{L})
H	[a-fA-F0-9]
E	[Ee][+-]?{D}+
FS	(f|F|l|L)
IS	(u|U|l|L)*

W	[\x80-\xfe][\x40-\xfe]
LT	\"(\\.|[^\\"])*\"

%%
#include             BEGIN(incl) ;
<incl>[ \t]*		/* eat the whitespace */
<incl>\"[^ \t\n]+\" { /* got the include file name */
				char *c, *p ;

				if ( yyinclude_stack_ptr >= MAX_INCLUDE_DEPTH )
				   YYERROR( "Includes nested too deeply(%s)", yytext ) ;
				for( c=p=file_path ; (c=strchr( p, '\\' )) ; p=c+1 ) ;
				*p = '\0' ;
				strcat( file_path, yytext+1 ) ;
				file_path[strlen(file_path)-1] = '\0' ;
				if ( !(yyin = fopen( file_path, "r" )) )
				   YYERROR( "Cannot open include file(%s)", yytext ) ;
				else
				{
					yyinclude_stack[yyinclude_stack_ptr++] = YY_CURRENT_BUFFER ;
					yyinclude_lines[yyinclude_stack_ptr  ] = 1 ;
					strcpy( yyinclude_paths[yyinclude_stack_ptr], yytext ) ;
					yy_switch_to_buffer( yy_create_buffer( yyin, YY_BUF_SIZE ) ) ;
				}
				BEGIN( INITIAL ) ;
			}


[ \t]|"　"	{ yycount(); /*ホワイトスペースは優先的に取り除く*/}

"/*"		{
				char c, c1 ;
			loop:
				while ( (c = input()) != '*' && c != 0 )
					if ( c == '\n' )
						yyinclude_lines[yyinclude_stack_ptr]++, yycolumn = 0 ;
				if ( (c1 = input()) != '/' && c != 0 )
				{
					unput( c1 ) ;
					goto loop ;
				}
			}
"//"		{ while ( input() != '\n' ) ; yyinclude_lines[yyinclude_stack_ptr]++, yycolumn = 0 ; }

"effect"	{ yycount() ; return TOKEN_EFFECT ; }
"mesg"		{ yycount() ; return TOKEN_MESSAGE ; }

"char"		{ yycount() ; yylval.code = TOKEN_CHAR        ; return TOKEN_CHAR    ; }
"int"		{ yycount() ; yylval.code = TOKEN_INT         ; return TOKEN_INT     ; }
"long"		{ yycount() ; yylval.code = TOKEN_LONG        ; return TOKEN_LONG    ; }
"short"		{ yycount() ; yylval.code = TOKEN_SHORT       ; return TOKEN_SHORT   ; }
"u_char"	{ yycount() ; yylval.code = TOKEN_CHAR |0x8000; return TOKEN_UCHAR   ; }
"u_int"		{ yycount() ; yylval.code = TOKEN_INT  |0x8000; return TOKEN_UINT    ; }
"u_long"	{ yycount() ; yylval.code = TOKEN_LONG |0x8000; return TOKEN_ULONG   ; }
"u_short"	{ yycount() ; yylval.code = TOKEN_SHORT|0x8000; return TOKEN_USHORT  ; }
"float"		{ yycount() ; yylval.code = TOKEN_FLOAT       ; return TOKEN_FLOAT   ; }
"void"		{ yycount() ; yylval.code = TOKEN_VOID        ; return TOKEN_VOID    ; }
"FVECTOR"	{ yycount() ; yylval.code = TOKEN_FVECTOR     ; return TOKEN_FVECTOR ; }
"SVECTOR"	{ yycount() ; yylval.code = TOKEN_SVECTOR     ; return TOKEN_SVECTOR ; }
"IVECTOR"	{ yycount() ; yylval.code = TOKEN_IVECTOR     ; return TOKEN_IVECTOR ; }
"FMATRIX"	{ yycount() ; yylval.code = TOKEN_FMATRIX     ; return TOKEN_FMATRIX ; }
"const"		{ yycount() ; yylval.code = 0x4000            ; return TOKEN_CONST   ; }

"||"		{ yycount() ; yylval.code = TOKEN_OR_OP  ; return TOKEN_OR_OP  ; }
"&&"		{ yycount() ; yylval.code = TOKEN_AND_OP ; return TOKEN_AND_OP ; }
"->"        { yycount() ; yylval.code = TOKEN_PTR_OP ; return TOKEN_PTR_OP ; }


{L}{A}*		{
				yycount() ;
				yystring() ;
				return TOKEN_IDENTIFIER ;
			}

"%%"		|
"%"{D}+		{
				yycount() ;
				yystring() ;
				return TOKEN_STR_VARIABLE;
			}

0[xX]{H}+{IS}?	{
				yycount() ;
				yylval.constant = (float)strtol( yytext, NULL, 16 ) ;
				return TOKEN_CONSTANT ;
			}
0{D}+{IS}?	{
				yycount() ;
				yylval.constant = (float)strtol( yytext, NULL, 8 ) ;
				return TOKEN_CONSTANT ;
			}
{D}+{IS}?	{
				yycount() ;
				yylval.constant = (float)strtol( yytext, NULL, 10 ) ;
				return TOKEN_CONSTANT ;
			}

{D}*"."{D}+({E})?{FS}?  |
{D}+"."{D}*({E})?{FS}?  |
{D}+{E}{FS}? {
				yycount() ;
				yylval.constant = (float)atof( yytext ) ;
				return TOKEN_CONSTANT ;
			}

{LT} 		{
				yycount() ;
				yyliteral() ;
				return TOKEN_STRING_LITERAL ;
			}


\$[bwist]:({W}|{L})({W}|{A})*	{
				yycount() ;
				yystring() ;
				return TOKEN_MESSAGE_LITERAL ;
			}


{W}({W}|{A})*	{
				yycount() ;
				yystring() ;
				return TOKEN_NAME_LITERAL ;
			}
";"         { yycount(); return ';' ; }
"{"         { yycount(); return '{' ; }
"}"         { yycount(); return '}' ; }
","         { yycount(); return ',' ; }
":"         { yycount(); return ':' ; }
"="         { yycount(); return '=' ; }
"("         { yycount(); return '(' ; }
")"         { yycount(); return ')' ; }
"["         { yycount(); return '[' ; }
"]"         { yycount(); return ']' ; }
"&"         { yycount(); return '&' ; }
"!"         { yycount(); return '!' ; }
"~"         { yycount(); return '~' ; }
"-"         { yycount(); return '-' ; }
"+"         { yycount(); return '+' ; }
"*"         { yycount(); return '*' ; }
"/"         { yycount(); return '/' ; }
"<"         { yycount(); return '<' ; }
">"         { yycount(); return '>' ; }
"^"         { yycount(); return '^' ; }
"|"         { yycount(); return '|' ; }
"?"         { yycount(); return '?' ; }

[\v\n\f]    { yycount(); }
.           { yycount(); }

<<EOF>>		{
				/*ネストしたインクルードから復帰する。*/
				if ( --yyinclude_stack_ptr < 0 )
				   yyterminate() ;
				else
				{
					/*ネストしてたファイルに戻る*/
				   yy_delete_buffer( YY_CURRENT_BUFFER ) ;
				   yy_switch_to_buffer( yyinclude_stack[yyinclude_stack_ptr] ) ;
				}
			}

%%

int yywrap( void )
{
    return 1 ;
}

extern LIST *list_regist( LIST *head, void *data, int size ) ;
extern WORK parse_work ;

void yystring( void )
{
	parse_work.name_pool = list_regist( parse_work.name_pool, yytext, yyleng+1 ) ;
	yylval.string = parse_work.name_pool->data ;
	yylval.string[ yyleng ] = '\0' ;
}
void yyliteral( void )
{
	parse_work.name_pool = list_regist( parse_work.name_pool, yytext+1, yyleng-1 ) ;
	yylval.string = parse_work.name_pool->data ;
	yylval.string[ yyleng-2 ] = '\0' ;
}

void yycount( void )
{
    int i ;
	char buffer[4096] ;

	if ( !yycolumn )
		yyline[0] = '\0' ;
	strncpy( buffer, yytext, yyleng ) ;
	buffer[yyleng] = '\0' ;
	strcat( yyline, buffer ) ;

    for ( i=0 ; yytext[i] != '\0' ; i++ )
        if ( yytext[i] == '\n' )
            yycolumn = 0, yyinclude_lines[yyinclude_stack_ptr]++ ;
        else if ( yytext[i] == '\t' )
            yycolumn += 8 - (yycolumn % 8) ;
		else
			yycolumn++ ;
}

void yylex_init( void )
{
	yycolumn  = 0    ;
	yyline[0] = '\0' ;
	yy_init   = 1    ;

	yyinclude_stack_ptr = 0;
	yyinclude_lines[0] = 1 ;
}
