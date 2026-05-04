#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _binseq_c_
#include "main.h"
#include "lang_env.h"
#include "binseq.h"
#include "lip_japanese.h"
#include "lip_english.h"
#include "lip.h"

BinSeq * seq2bin(int *retnums, Seq ** list, ConvTable * conv_list)
{
  BinSeq * bseq;
  void   * vpt;
  int nums;
  int i, j;

  nums = 0;
  for(i = 0; list[i] != NULL; i++)
    {
      vpt = (!nums)
	? malloc(sizeof(BinSeq))
	: realloc(bseq, sizeof(BinSeq) * (nums + 1));
      if(NULL == vpt)
	{
	  free(bseq);
	  return NULL;
	}
      bseq = vpt;

      fprintf(stderr, "%5d  %s   %8.3f\n",
	      list[i]->t, list[i]->phone, list[i]->power);

      bseq[nums].t = list[i]->t;         /* 時刻の登録   */
      bseq[nums].power = (unsigned char)(list[i]->power * 255); /* パワーの登録 */

      /* 音素ラベルを ID に変換する */
      for(j = 0; conv_list[j].label != NULL; j++)
	if(!strcmp(conv_list[j].label, list[i]->phone))
	  {
	    bseq[nums].phone_id = conv_list[j].phone_id;
	    break;
	  }
      nums++;
    }

  /*
   * 終端として、LIP_unknown のセグメントを書き込む
   */
  vpt = (!nums)
    ? malloc(sizeof(BinSeq))
    : realloc(bseq, sizeof(BinSeq) * (nums + 1));
  if(NULL == vpt)
    {
      free(bseq);
      return NULL;
    }
  bseq = vpt;
  bseq[nums].t = TERM_NUMBER;
  bseq[nums].phone_id = LIP_unknown;
  bseq[nums].power = 0.0;
  nums++;
  *retnums = nums;
  return bseq;
}
