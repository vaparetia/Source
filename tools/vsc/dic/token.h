#ifndef _token_h_
#define _token_h_

/* =========================================================================
 * Macros
 * ========================================================================= */
#define TKN_LBUF_SPAN   ((size_t)1024)    /* 行バッファ確保の単位     */
#define TKN_AUTOBUF_SIZE  ((size_t)1024)  /* 自動確保バッファのサイズ */

/* デフォルトの各種文字集合 */
#define TKN_BLANK_LIST  " \t\n"     /* 空白、タブ、改行             */
#define TKN_DELIM_LIST  "<[{()}]>"  /* デフォルトのデリミタ文字集合 */


/* =========================================================================
 * Data type defines
 * ========================================================================= */

typedef struct _token {
  FILE *fp;             /* トークンを読み出すストリームのポインタ        */
  int  op;              /* 0: ストリームから生成 / 1: ファイル名から生成 */
  
  int lnum;             /* 現在行バッファに読み込まれている行の行番号    */
  int rd_pos;           /* 現在の行バッファ内読み出し開始位置            */

  int last_lnum;        /* 最後に読んだトークンが含まれる行の番号        */

  size_t line_size;     /* 行バッファのサイズ                            */
  char * line_buf;      /* 行バッファ                                    */

} TOKEN;

/*
 * Function prototypes
 */
TOKEN * tknOpenByStream(FILE * rfp);
TOKEN * tknOpen(char *fname);

void    tknClose(TOKEN *tkn);

int     tknGetLine(TOKEN *tkn);
int     tknLastLine(TOKEN *tkn);

char  * tknReadToken(TOKEN *tkn, char *buf, int size, char *delim);
char  * tknReadWhileTerminator(TOKEN *tkn, char *buf, int size, char *term);

#endif /* _token_h_ */
