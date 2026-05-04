#ifndef _token_h_
#define _token_h_

#define TKN_BUFLEN   ((size_t)1024)

#define TKN_DEF_DELIM   "[{(:,;)}]\n"  /* デフォルトのデリミタ     */
#define TKN_DEF_BLANK   "\t "             /* デフォルトの空白         */
#define TKN_DEF_QUOTE   "\"\'"            /* デフォルトのクォート記号 */

typedef enum {
  TKN_UNKNOWN,  /* 不明                         */
  TKN_TOKEN,    /* 通常のトークン               */
  TKN_QUOTED,   /* クォート記号で挟まれた文字列 */
  TKN_DELIM     /* デリミタ                     */
} tknStat;



typedef struct {
  FILE * rfp;

  char  * delim;     /* デリミタとして認識する文字の集合     */
  char  * blank;     /* 空白として認識する文字の集合         */
  char  * quote;     /* クォート記号として認識する文字の集合 */

  int     pos;       /* 読みだし開始位置 */
  int     lnum;      /* 現在のバッファに格納されているテキストの行番号 */
  int     last_lnum; /* 最後に読みだしたトークンが含まれる行の番号     */
  tknStat last_stat; /* 最後に読み出したトークンの分類                 */

  /* ファイル名からでなく、ストリームから作成された場合に 1 になる */
  int stream:1;

  unsigned char buf[ TKN_BUFLEN ];

} tknFILE;

#ifndef _token_c_
#define EXT extern
#else
#define EXT
#endif /* _token_c_ */

#undef EXT

tknFILE * tknOpenByStream(FILE * stream);
tknFILE * tknOpen(char * fname);
void      tknClose(tknFILE * tfp);
int       tknSetDelim(tknFILE * tfp, char * delim, char * blank, char * quote);
int       tknGetLine(tknFILE * tfp);
tknStat   tknGetStat(tknFILE * tfp);
char    * tknGetToken(tknFILE * tfp, char * buf, size_t siz);
char    * tknStripQuote(tknFILE * tfp, char * str);

#endif /* _token_h_ */
