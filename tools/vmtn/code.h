#ifndef _code_h_
#define _code_h_ 

#ifndef _code_c_
#define EXT extern
#else
#define EXT
#endif /* _code_c_ */

#undef EXT

void code_exchange(char * str);  /* 文字列中の Shift_JIS を EUC-JP に直す */

/* !!!警告!!!  JIS X 0201 kana 文字集合に該当する文字には対応していない。 */

#endif /* _code_h_ */
