#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _token_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"

/*
 * 行バッファを初期化
 */
static int tkn_init_linebuffer(tknFILE * tfp)
{
  fseek(tfp->rfp, 0L, SEEK_SET);    /* 先頭に移動 */

  /* 行番号を初期化 */
  tfp->lnum = 1;
  tfp->last_lnum = -1; /* 読み出されたトークンはまだ無いため、-1 */

  if(NULL == fgets(tfp->buf, TKN_BUFLEN, tfp->rfp)) return -1;
  tfp->pos = 0;
  return 0;
}

/*
 * 行バッファに次のテキストを読み込む
 */
static int tkn_read_text(tknFILE * tfp)
{
  size_t len;

  /* 先程までの行バッファの末尾が改行であれば、行番号を更新
     改行でなければ、同じ行の続き */
  len = strlen(tfp->buf);
  if(len > 0)
    if('\n' == tfp->buf[len - 1]) tfp->lnum++;

  if(NULL == fgets(tfp->buf, TKN_BUFLEN, tfp->rfp)) return -1;
  tfp->pos = 0;

  return 0;
}

/*
 * トークンの区切り条件を設定する
 */
int tknSetDelim(tknFILE * tfp, char * delim, char * blank, char * quote)
{
  char *d, *b, *q;

  /* エラーの場合は、現在の設定を保持する */
  if(NULL == (d = strdup(delim))) return -1;
  if(NULL == (b = strdup(blank)))
    {
      FREE(d);
      return -1;
    }
  if(NULL == (q = strdup(quote)))
    {
      FREE(d);
      FREE(b);
      return -1;
    }
  if(tfp->delim != NULL) FREE(tfp->delim);
  if(tfp->blank != NULL) FREE(tfp->blank);
  if(tfp->quote != NULL) FREE(tfp->quote);
  tfp->delim = d;
  tfp->blank = b;
  tfp->quote = q;

  DBG("delim:[%s]\nblank:[%s]\nquote:[%s]\n", tfp->delim, tfp->blank, tfp->quote);
  return 0;
}

tknFILE * tknOpenByStream(FILE * rfp)
{
  tknFILE * tfp;

  DBG("<1>\n");
  if(NULL == (tfp = MALLOC(sizeof(tknFILE)))) return NULL;

  DBG("<2>\n");
  tfp->stream = 1;
  tfp->rfp = rfp;
  if(tkn_init_linebuffer(tfp))
    {
      FREE(tfp);
      return NULL;
    }
  DBG("<3>\n");
  /* デリミタの設定を初期化 */
  tfp->delim = tfp->blank = tfp->quote = NULL;
  if(tknSetDelim(tfp, TKN_DEF_DELIM, TKN_DEF_BLANK, TKN_DEF_QUOTE))
    {
      FREE(tfp);
      return NULL;
    }
  tfp->last_stat = TKN_UNKNOWN;
  return tfp;
}

tknFILE * tknOpen(char * fname)
{
  FILE *rfp;
  tknFILE * tfp;

  if(NULL == (rfp = fopen(fname, "r"))) return NULL;
  DBG("[1]\n");
  if(NULL == (tfp = tknOpenByStream(rfp)))
    {
      fclose(rfp);
      return NULL;
    }
  DBG("[2]\n");

  tfp->stream = 0;  /* ストリームからの生成ではないので、0 にする */

  return tfp;
}

void tknClose(tknFILE * tfp)
{
  /* ストリームから生成されたのでなければ、閉じる */
  if(!tfp->stream) fclose(tfp->rfp);
  if(NULL != tfp->delim) FREE(tfp->delim);
  if(NULL != tfp->blank) FREE(tfp->blank);
  if(NULL != tfp->quote) FREE(tfp->quote);
  FREE(tfp);
}

/*
 * 最後に読み出したトークンの含まれる行番号を得る
 */
int tknGetLine(tknFILE * tfp)
{
  return tfp->last_lnum;
}

/*
 * 最後に読み出したトークンの、分離条件を得る
 */
tknStat tknGetStat(tknFILE * tfp)
{
  return tfp->last_stat;
}

/*
 * 現在位置から読み出せる最初の文字を得る
 */
static int tkn_get_char(tknFILE * tfp)
{
  int c;

  while(!(c = tfp->buf[ tfp->pos++ ]))
    if(tkn_read_text(tfp)) return -1;

  return c;
}

/*
 * 現時点で読み出し対象となる、最初の文字を得る。
 * ポインタは更新しない。
 */
static int tkn_look_char(tknFILE * tfp)
{
  int c;

  while(!(c = tfp->buf[ tfp->pos ]))
    {
      if(tkn_read_text(tfp)) return -1;
      tfp->pos++;
    }

  return c;
}



/*
 * トークンの読みだし
 */
char * tknGetToken(tknFILE * tfp, char * buf, size_t siz)
{
  char * delim;
  char * blank;
  char * quote;
  size_t cnt;
  int c, q;

  delim = tfp->delim;
  blank = tfp->blank;
  quote = tfp->quote;

  /* バッファのサイズが 2 バイト以下ならば、
     いかなるトークンも記録できないため、NULL を返す */
  if(siz < 2) return NULL;

  /* 現在のポイントからの空白を読み飛ばす */
  do {
    if((c = tkn_get_char(tfp)) < 0) return NULL;  /* これ以上読めない場合 */
  } while(search_char(c, blank));  /* 拾った文字が空白に分類される限り続ける */

  /* 最初の文字の含まれる行番号をトークンの行番号として記録 */
  tfp->last_lnum = tfp->lnum;
  tfp->last_stat = TKN_UNKNOWN;
  cnt = 0;

  buf[ cnt++ ] = c;
  q = search_char(c, quote);
  if(!search_char(c, delim) || q)
    {
      if(q)
	{
	  int escape = 0;
	  int q_code;
	  
	  q_code = c;
	  
	  /* クォート文字であった場合は、
	     対になるクォートまでをトークンとする */
	  while(((c = tkn_look_char(tfp)) >= 0) && (cnt < (siz - 1)))
	    {
	      buf[ cnt++ ] = c = tkn_get_char(tfp);
	      if((q_code == c) && !escape) break;
	      escape = (('\\' == c) && !escape );
	    }
	  tfp->last_stat = TKN_QUOTED;
	}
      else
	{
	  while(((c = tkn_look_char(tfp)) >= 0) && (cnt < (siz - 1)))
	    {
	      if(search_char(c, delim) ||
		 search_char(c, blank) ||
		 search_char(c, quote)) break;
	      c = tkn_get_char(tfp);
	      buf[cnt++] = c;
	    }
	  tfp->last_stat = TKN_TOKEN;
	}
    }
  else
    {
      tfp->last_stat = TKN_DELIM;
    }

  buf[cnt] = 0;
  
  return buf;
}

/*
 * クォート文字列の、クォート記号を除去する
 * クォート記号は、先頭と最後尾にあるものとする。
 */
char * tknStripQuote(tknFILE * tfp, char * str)
{
  size_t len, p;
  
  len = strlen(str);
  /* クォート文字列である以上、長さが2文字以下はあり得ないため、エラー */
  if(len < 2) return NULL;

  /* 先頭と最後尾が、クォート記号でなければエラー */
  if(!search_char(*str, tfp->quote) ||
     !search_char(*(str + len - 1), tfp->quote)) return NULL;

  for(p = 0; p < (len - 2); p++) str[p] = str[p + 1];
  str[p] = 0;

  return str;
}
