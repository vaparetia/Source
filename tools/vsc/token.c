#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _token_c_
#include "token.h"

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * static 宣言された関数のプロトタイプ
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
static int tkn_read_line(TOKEN *tkn);
static int tkn_char_match(int c, char *list);
static char * tkn_read_token(TOKEN *tkn, char *buf, int size, char *delim);
static char * tkn_read_while_terminator(TOKEN *tkn,
					char *buf, int size, char *term);



/* =========================================================================
 * ストリーム構造体から、TOKEN 構造体を生成する。
 *
 * Prototype:    TOKEN * tknOpenByStream(FILE *rfp)
 *
 * Args:         rfp     データを取得する元になるストリーム構造体
 * Return:       TOKEN * 生成された TOKEN 構造体のポインタ
 *                       エラー時には NULL を返す
 * ========================================================================= */
TOKEN * tknOpenByStream(FILE *rfp)
{
  TOKEN *tkn;

  if(NULL == (tkn = malloc(sizeof(TOKEN)))) return NULL;
  tkn->fp = rfp;
  tkn->op = 0;
  tkn->lnum = 0;
  tkn->line_buf = NULL;
  tkn->line_size = 0;

  tkn_read_line(tkn);   /* 最初の一行を読み、行バッファに格納 */

  return tkn;  
}

/* =========================================================================
 * 指定されたファイルを開き、そこから TOKEN 構造体を生成する。
 *
 * Prototype:    TOKEN * tknOpen(char *path)
 *
 * Args:         path    データとして見做すファイルのパス
 * Return:       TOKEN * 生成された TOKEN 構造体のポインタ
 *                       エラー時には NULL を返す
 * ========================================================================= */
TOKEN * tknOpen(char *fname)
{
  TOKEN *tkn;
  FILE *rfp;

  if(NULL == (rfp = fopen(fname, "rb")))return NULL;
  if(NULL == (tkn = tknOpenByStream(rfp)))
    {
      fclose(rfp);
      return NULL;
    }
  tkn->op = 1;
  return tkn;
}

/* =========================================================================
 * トークン構造体を解放する
 *
 * Prototype:    void tknClose(TOKEN *tkn)
 *
 * Args:         tkn        解放するトークン構造体
 * Return:       void
 *
 * ファイル名ではなくストリームから生成した場合、そのストリームは別口に
 * fclose() を呼び出す必要がある。
 * ========================================================================= */
void tknClose(TOKEN *tkn)
{
  if(tkn->op) fclose(tkn->fp);
  if(NULL != tkn->line_buf) free(tkn->line_buf);
  free(tkn);
}


/* =========================================================================
 * トークンの読み出し
 *
 * Prototype:    char *  tknReadToken(TOKEN *tkn,
 *                                    char * buf, int size,
 *                                    char *delim)
 *                                  
 *
 * Args:         tkn     読み出し元とするデータストリームの TOKEN 構造体
 *
 *               buf     読み出したトークンを格納するバッファ
 *               size    バッファの最大サイズ
 *
 *               delim   デリミタとして認識させる文字集合の文字列。
 *                       空白、水平タブ、改行等の空白文字は、自動的に
 *                       デリミタとして認識される。また、NULL が指定された
 *                       場合は、デフォルトのデリミタ文字集合を使用する。
 *
 * Return:       char *  読み出した token が格納されている文字列のポインタ
 *                       読み出し失敗時は、NULL を返す。
 * 
 * ※ buf と size の指定による、バッファ確保の挙動
 *
 *    buf   |  size  |                       挙動
 *   -------+--------+---------------------------------------------------
 *    指定  | 2未満  | 失敗
 *    指定  | 2以上  | 指定されたバッファに、指定の長さを上限として格納
 *    NULL  | 2以上  | size で指定されたサイズのバッファを内部で確保
 *    NULL  | 2未満  | 内部でデフォルトサイズを確保して格納
 *   -------+--------+---------------------------------------------------
 *               
 * ========================================================================= */
char * tknReadToken(TOKEN * tkn, char *buf, int size, char *delim)
{
  char *retbuf;
  char *ret;
  int maxsize;

  /* バッファの確保 */
  if(NULL == buf)
    {
      maxsize = (size < 2) ? TKN_AUTOBUF_SIZE : size;
      if(NULL == (retbuf = malloc(maxsize))) return NULL;
    }
  else if(size >= 2)
    {
      retbuf = buf;
      maxsize = size;
    }
  else
    /* バッファが指定されていても、最大サイズが 2未満の場合は、失敗と見做す */
    return NULL; 

  /* 下請に廻す */
  if(NULL == (ret = tkn_read_token(tkn, retbuf, maxsize, delim)))
    if(NULL == buf) free(retbuf);

  return ret;
}



/* =========================================================================
 * 現在の読み出し開始位置から、指定された文字集合にある文字が発見されるか、
 * 行が終了するまでをバッファに取り込む
 *
 * Prototype:    char *  tknReadWhileTerminator(TOKEN *tkn,
 *                                              char * buf, int size,
 *                                              char *term)
 *                                  
 *
 * Args:         tkn     読み出し元とするデータストリームの TOKEN 構造体
 *
 *               buf     読み出したトークンを格納するバッファ
 *               size    バッファの最大サイズ
 *
 *               term    終端文字として見做す文字の集合。
 *
 * Return:       char *  読み出した token が格納されている文字列のポインタ
 *                       読み出し失敗時は、NULL を返す。
 * 
 *
 * 終端文字が見つかるか、行末に至るまでの間、デリミタや空白、改行があっても
 * バッファに取り込み続ける。主にクォートやブラケットの処理に使用。
 *
 * ※ buf と size の指定による、バッファ確保の挙動については、tknReadToken()
 *    を参照。
 *
 * ========================================================================= */
char * tknReadWhileTerminator(TOKEN *tkn, char *buf, int size, char *term)
{
  char *retbuf;
  char *ret;
  int maxsize;

  /* バッファの確保 */
  if(NULL == buf)
    {
      maxsize = (size < 2) ? TKN_AUTOBUF_SIZE : size;
      if(NULL == (retbuf = malloc(maxsize))) return NULL;
    }
  else if(size >= 2)
    {
      retbuf = buf;
      maxsize = size;
    }
  else
    /* バッファが指定されていても、最大サイズが 2未満の場合は、失敗と見做す */
    return NULL; 

  /* 下請に廻す */
  if(NULL == (ret = tkn_read_while_terminator(tkn, retbuf, maxsize, term)))
    if(NULL == buf) free(retbuf);

  return ret;
}


/* =========================================================================
 * 現在読み出しポイントがある行番号を得る
 *
 * Prototype:    int     tknGetLine(TOKEN *tkn)
 *
 * Args:         tkn     対象の TOKEN 構造体
 * Return:       int     現在読み出しポイントがある行番号。
 * ========================================================================= */
int tknGetLine(TOKEN *tkn)
{
  return tkn->lnum;
}

/* =========================================================================
 * 最後に読んだトークンの行番号を得る
 *
 * Prototype:    int     tknLastLine(TOKEN *tkn)
 *
 * Args:         tkn     対象の TOKEN 構造体
 * Return:       int     最後に読んだトークンの行番号
 * ========================================================================= */
int tknLastLine(TOKEN *tkn)
{
  return tkn->last_lnum;
}

/* -------------------------------------------------------------------------
 * ストリームから、データを一行分取得する。
 * 行の長さにかかわらず、確実に行末まで取り込むか、あるいはエラーを返す。
 *
 * Prototype:   static int  tkn_read_line(TOKEN *tkn)
 *
 * Args:        tkn      行を読み込む TOKEN 構造体
 * Return:      int      一行として読み込んだ文字数(octet)。エラー時は -1
 *
 * ※ static 宣言につき、外部からの参照不可
 * ------------------------------------------------------------------------- */
static int tkn_read_line(TOKEN *tkn)
{
  size_t siz;
  char *buf, *tmp;
  int pos;
  void *vpt;

  if(NULL != tkn->line_buf)
    {
      free(tkn->line_buf);
      tkn->line_buf = NULL;
    }

  if(NULL == (buf = malloc(TKN_LBUF_SPAN))) return -1;
  pos = 0;
  siz = TKN_LBUF_SPAN;
  while(NULL != (tmp = fgets(buf + pos, siz - pos, tkn->fp)))
    {
      /* 読み込んだ最後が改行であれば、一行分を読み終わったことになる */
      if(*(buf + pos + strlen(buf + pos) - 1) == '\n') break;

      /* テキストの行が、読み込んだ分だけでは終わっていない場合 */
      siz += TKN_LBUF_SPAN;
      if(NULL == (vpt = realloc(buf, siz)))
	{
	  free(buf);
	  return -1;
	}
      buf = vpt;
      pos = strlen(buf);
    }
  if(NULL == tmp)
    {
      free(buf);
      return -1;
    }
  if(NULL == (vpt = realloc(buf, strlen(buf) + 1)))
    {
      free(buf);
      return -1;
    }
  tkn->line_buf = (char *)vpt;  /* 新しく確保した行バッファを登録 */
  tkn->rd_pos = 0;              /* 読み出し開始位置をリセットする */
  tkn->lnum++;                  /* 行番号を更新する               */
  
  return strlen(tkn->line_buf);
}



/* -------------------------------------------------------------------------
 * 文字が文字集合に含まれるかどうかを調べる
 *
 * Prototype:    static int tkn_char_match(int c, char *list)
 *
 * Args:         c       文字集合に含まれるかどうか調査する対象の文字
 *               list    文字集合をあらわす文字列
 *
 * Return:       int     文字が文字集合に含まれる場合はその文字、
 *                       含まれない場合は 0 を返す。
 * 
 * ※ static 宣言につき、外部からの参照不可
 * ------------------------------------------------------------------------- */
static int tkn_char_match(int c, char *list)
{
  char *ptr;

  for(ptr = list; *ptr; ptr++)
    if(c == *ptr) return c;
  return 0;
}

/* -------------------------------------------------------------------------
 * トークンの読み出し
 *
 * Prototype:    static char *  tkn_read_token(TOKEN *tkn,
 *                                             char * buf, int size,
 *                                             char *delim)
 *
 *
 * Args:         tkn     読み出し元とするデータストリームの TOKEN 構造体
 *
 *               buf     読み出したトークンを格納するバッファ
 *               size    バッファの最大サイズ
 *
 *               delim   デリミタ文字集合
 *
 * Return:       char *  読み出した token が格納された文字列のポインタ
 *                       読み出し失敗時は、NULL を返す。
 * 
 * ※1 static 宣言につき、外部からの参照不可
 * ※2 buf は確実に NULL 以外の有効なバッファを指し、size は必ず 2 以上で
 *     なければならない。
 * ------------------------------------------------------------------------- */
static char * tkn_read_token(TOKEN *tkn, char *buf, int size, char *delim)
{
  int c;
  int pos;
  int lnum;

  /* 行が読み込まれていない場合は、エラーを返す */
  if(NULL == tkn->line_buf) return NULL;

  /* 現在の読み出しポイントからの空白をスキップする */
  do {
    while(tkn_char_match(tkn->line_buf[tkn->rd_pos], TKN_BLANK_LIST))
      {
	tkn->rd_pos++;
	
	/* 行末に至った場合 */
	if(!tkn->line_buf[tkn->rd_pos] || tkn->line_buf[tkn->rd_pos] == '#')
	  if(tkn_read_line(tkn) < 0) goto err_entry;
      }
    lnum = tkn->lnum;
    pos = 0;
    c = tkn->line_buf[tkn->rd_pos];  /* 空白ではない最初の文字 */
    tkn->rd_pos++;
    if(c == '#') 
      if(tkn_read_line(tkn) < 0) goto err_entry;
  } while(c == '#');

  buf[pos++] = c;
  
  /* デリミタとマッチしたら、そのデリミタのみの文字列をつくり、返す */
  if(tkn_char_match(c, delim))
    {
      buf[pos] = 0;
      tkn->last_lnum = lnum;
      return buf;
    }
  /*
   * 空白でもデリミタでもなければ、
   * 空白かデリミタが登場する直前までをバッファに格納
   */
  while((pos < size - 1) && (c = tkn->line_buf[tkn->rd_pos]))
    {
      if(c == '#') break;  /* # 以降は注釈と認識する */
      if(tkn_char_match(c, delim) || tkn_char_match(c, TKN_BLANK_LIST))
	break;
      buf[pos++] = c;
      tkn->rd_pos++;
    }
  
  buf[pos] = 0;
  tkn->last_lnum = lnum;
  return buf;

 err_entry:
  tkn->last_lnum = -1;
  return NULL;
}


/* -------------------------------------------------------------------------
 * 現在の読み出し開始位置から、指定された文字集合にある文字が発見されるか、
 * 行が終了するまでをバッファに取り込む
 *
 * Prototype:    static char *  tkn_read_while_terminator(TOKEN *tkn,
 *                                                        char * buf, int size,
 *                                                        char *delim)
 *
 *
 * Args:         tkn     読み出し元とするデータストリームの TOKEN 構造体
 *
 *               buf     読み出したトークンを格納するバッファ
 *               size    バッファの最大サイズ
 *
 *               term    終端文字として見做す文字の集合。
 *
 * Return:       char *  読み出した token が格納された文字列のポインタ
 *                       読み出し失敗時は、NULL を返す。
 * 
 * ※1 static 宣言につき、外部からの参照不可
 * ※2 buf は確実に NULL 以外の有効なバッファを指し、size は必ず 2 以上で
 *     なければならない。
 * ------------------------------------------------------------------------- */
static char * tkn_read_while_terminator(TOKEN *tkn,
					char *buf, int size, char *term)
{
  int pos;
  int c;
  int lnum;

  /* 行が読み込まれていない場合は、エラーを返す */
  if(NULL == tkn->line_buf) return NULL;
  
  lnum = tkn->lnum;

  pos = 0;
  do {
    c = tkn->line_buf[tkn->rd_pos];
    tkn->rd_pos++;
    buf[pos] = c;
    pos++;
  } while(!tkn_char_match(c, term) &&
	  pos < (size - 1) &&
	  tkn->line_buf[tkn->rd_pos]);

  buf[pos] = 0;
  tkn->last_lnum = lnum;

  if(!tkn->line_buf[tkn->rd_pos])  /* 行末に至った場合 */
    tkn_read_line(tkn);            /* 次の行を読んでおく */

  return buf;  
}

/* EOF */
