/*
 * 音素リファレンスデータを読み込む
 *
 * 音素リファレンスは,各音素の特徴ベクトルを
 * 10次のメルケプストラムで表したものとする。
 *
 * データファイルの書式は独自形式
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _ref_load_c_
#include "ref_load.h"
#include "token.h"
#include "util.h"
#include "cepstrum.h"

#define BUF_SIZE 256

static char buf[ BUF_SIZE ];  /* トークン用バッファ */


/* =========================================================================
 * トークンもしくはタグの取得
 *
 * Prototype: static char * ref_read_token(TOKEN *tkn);
 *
 * Args:      TOKEN *tkn     トークンストリームのポインタ
 *
 * Return:    char *         読み込んだトークン文字列のポインタ
 *                           読み込めなかった場合は NULL
 *
 * ======================================================================== */
static char * ref_read_token(TOKEN *tkn)
{
  if(NULL == tknReadToken(tkn, buf, BUF_SIZE, "<>")) return NULL;
  if(!strcmp(buf, "<"))   /* タグの開始であれば、タグの終了までを読み込む */
    tknReadWhileTerminator(tkn, buf + 1, BUF_SIZE - 1, ">");

  return buf;
}


/* -------------------------------------------------------------------------
 * 音素リファレンスのラベルを読み込む
 *
 * Prototype: static int ref_read_label(TOKEN *tkn, refR *ref)
 *
 * Args:      TOKEN    * tkn     読み込み元のトークンストリーム
 *            refBlock * ref     読み込み先の音素リファレンス構造体ポインタ
 *
 * Return:    int            0: 正常終了  /  非0: 異常終了
 * ------------------------------------------------------------------------- */
static int ref_read_label(TOKEN *tkn, refR *ref)
{
  size_t len;

  ref_read_token(tkn);  /* ラベルとなる文字列を読み込む */
 
  len = strlen(buf); /* 読み込まれたラベルの長さ */

  if(NULL == (ref->label = malloc(len + 1))) return -1;
  strcpy(ref->label, buf);

  ref_read_token(tkn);
  if(strcmp(buf, "</label>")) return -1;

  return 0;
}

/* -------------------------------------------------------------------------
 * 音素リファレンスのベクトルを読み込む
 *
 * Prototype: static int ref_read_vector(TOKEN *tkn, refR *ref, int ref_num)
 *
 * Args:      TOKEN    * tkn     読み込み元のトークンストリーム
 *            refBlock * ref     読み込み先の音素リファレンス構造体ポインタ
 *            int        ref_num リファレンスの番号
 *
 * Return:    int            0: 正常終了  /  非0: 異常終了
 * ------------------------------------------------------------------------- */
static int ref_read_vector(TOKEN *tkn, refR *ref, int ref_nums)
{
  int nums;
  cepVector *rr;
  int i;

  rr = &ref->r[ref_nums];
  i = 0;

  while(NULL != ref_read_token(tkn))
    {
      if(!strcmp(buf, "</vec>")) break;  /* 終端記号 */
      if(i >= cepVEC_DIM) break;
      rr->c[i] = atof(buf);
      i++;
    }

  return 0;
}

/* -------------------------------------------------------------------------
 * 音素リファレンスを一つ読み込む
 *
 * Prototype: static int ref_read_ref(TOKEN *tkn, refR *ref)
 *
 * Args:      TOKEN    * tkn     読み込み元のトークンストリーム
 *            refBlock * ref     読み込み先の音素リファレンス構造体ポインタ
 *
 * Return:    int            0: 正常終了  /  非0: 異常終了
 * ------------------------------------------------------------------------- */
static int ref_read_ref(TOKEN *tkn, refR *ref)
{
  int vec_num;

  vec_num = 0;
  while(NULL != ref_read_token(tkn))
    {
      if(!strcmp(buf, "</ref>")) break;

      if(!strcmp(buf, "<label>"))
	{
	  ref_read_label(tkn, ref);  /* 音素ラベルの読み込み */
	  continue;
	}
      
      if(!strcmp(buf, "<vec>"))
	{
	  ref_read_vector(tkn, ref, vec_num);  /* ベクトルリファレンスの取得 */
	  vec_num++;
	}
    }
  return 0;
}


/* -------------------------------------------------------------------------
 * リファレンスサブセットブロックを読み込む
 *
 * Prototype: static int ref_read_subset(TOKEN *tkn, refBlock *block)
 *
 * Args:      TOKEN    * tkn     読み込み元のトークンストリーム
 *            refBlock * block   読み込み先のサブセットブロックのポインタ
 *
 * Return:    int            0: 正常終了  /  非0: 異常終了
 * ------------------------------------------------------------------------- */
static int ref_read_subset(TOKEN *tkn, refBlock *block)
{
  void *vpt;
  refR *ref;
  int i;

  block->nums = 0;
  block->refs = NULL;
  while(NULL != ref_read_token(tkn))
    {
      if(!strcmp(buf, "</subset>")) break;  /* 終了タグを発見したら復帰 */

      if(!strcmp(buf, "<ref>"))
	{
	  if(NULL == (ref = malloc(sizeof(refR)))) goto err_0;
	  ref_read_ref(tkn, ref);  /* 読み込み */
	  if(NULL == (vpt
		      = utilAddBuf(block->refs, sizeof(refR *), block->nums)))
	    goto err_0;
	  block->refs = vpt;
	  block->refs[block->nums] = ref;
	  block->nums++;
	}
    }
  return 0;  
  
 err_0:
  for(i = 0; i < block->nums; i++) free(block->refs[i]);
  free(block->refs);
  return -1;
}

/* =========================================================================
 * 音素リファレンスパターンの読み込み
 *
 * Prototype: int refLoadRef(char *fname)
 *
 * Args:     char *fname    音素リファレンスパターンデータファイルのパス
 *
 * Return:   int            0: 正常終了 / 非0: 異常終了
 *
 * ======================================================================== */
int refLoadRef(char *fname)
{
  TOKEN *tkn;
  refBlock * block;

  fprintf(stderr, "DIC file name: %s\n", fname);
  if(NULL == (tkn = tknOpen(fname))) return -1;

  /*
   * 音素リファレンスパターンは,各音素のケプストラムによる特徴ベクトルが
   * テキスト形式で格納されている。
   */
  while(NULL != ref_read_token(tkn))
    {
      if(!strcmp(buf, "<subset>"))   /* サブセット */
	{
	  ref_read_token(tkn);  /* サブセットラベルを得る */

	  if(!strcmp(buf, "W"))  /* W カテゴリ */
	    {
	      block = &refDICT.W;
	    }
	  else if(!strcmp(buf, "C"))
	    {
	      block = &refDICT.C;
	    }
	  else if(!strcmp(buf, "U"))
	    {
	      block = &refDICT.U;
	    }
	  else if(!strcmp(buf, "SV"))
	    {
	      block = &refDICT.SV;
	    }
	  else if(!strcmp(buf, "CW"))
	    {
	      block = &refDICT.CW;
	    }
	  else if(!strcmp(buf, "UW"))
	    {
	      block = &refDICT.UW;
	    }
	  else if(!strcmp(buf, "F"))
	    {
	      block = &refDICT.F;
	    }
	  else if(!strcmp(buf, "X"))
	    {
	      block = &refDICT.X;
	    }
	  ref_read_subset(tkn, block);
	}
    }

  tknClose(tkn);

  fprintf(stderr, "Dictionary read completed.\n"); fflush(stderr);
  return 0;  
}
