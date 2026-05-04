#ifndef _yacc_parser_h_
#define _yacc_parser_h_

#include "2d_data.h"

extern FILE * yyin;  /* 入力ストリーム */
extern FILE * yyout; /* 出力ストリーム */

int yyparse();

#endif /* _yacc_parser_h_ */
