#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _split_c_
#include "split.h"
#include "util.h"
#include "main.h"

#ifdef _DEBUG_
#define DBG(args...)  fprintf(stderr, args);  fflush(stderr)
#else
#define DBG(args...)
#endif /* _DEBUG_ */


/*
 * 生シーケンスデータを読み込む
 */
VoiceSeq * splLoadSequence(FILE *rfp)
{
  VoiceSeq * ret;
  void *vpt;
  int nums;

  nums = 0;
  DBG("0\n");

  do {
    DBG("1\n");
    vpt = (!nums)
      ? malloc(sizeof(VoiceSeq))
      : realloc(ret, sizeof(VoiceSeq) * (nums + 1));

    DBG("2\n");
    if(NULL == vpt)
      {
	free(ret);
	return NULL;
      }
    DBG("3\n");
    ret = vpt;
    if(fread(ret + nums, 1, sizeof(VoiceSeq), rfp) == 0)
      {
	free(ret);
	return NULL;
      }
    DBG("4\n");

    DBG("%5d: %3d  (%d)\n", ret[nums].time, ret[nums].phone_id, ret[nums].power);
  } while(ret[nums++].time != 0xffff);

  DBG("5\n");

  return ret;
}

void splRelease(VoiceSeq ** seq)
{
  int i;

  for(i = 0; seq[i] != NULL; i++) free(seq[i]);
  free(seq);
}

VoiceSeq ** splAutoSplit(VoiceSeq * seq, int packet_size)
{
  int blocks;
  VoiceSeq ** seq_block;
  void *vpt;
  int cnt, term;
  int begin;
  int begin_time;
  unsigned short final_time;
  int rec_size;
  int i;

  blocks = 0;
  begin = 0;
  begin_time = 0;

  rec_size = min(packet_size, max_size);
  DBG("rec_size = %d\n", rec_size);

  if(rec_size < (MIN_SIZE + sizeof(VoiceSeq)))
    {
      fprintf(stderr,
	      "Can't create packet.(packet or max size < %d)\n",
	      MIN_SIZE + sizeof(VoiceSeq));
      return NULL;
    }

  do {
    cnt = 0;
    term = -1;
    /* サイズがパケットの最大サイズ未満である間継続 */
    while((seq[begin + cnt].time != 0xffff) &&
	  (MIN_SIZE + cnt * sizeof(VoiceSeq)) < rec_size)
      {
	if(((seq[begin + cnt].phone_id == PHONE_sil) &&
	    ((seq[begin + cnt + 1].time - seq[begin + cnt].time) > BLANK_TIME))
	   ||
	   (seq[begin + cnt + 1].time == 0xffff))
	  term = cnt + 1;
	cnt++;
      }
    if(term < 0) term = cnt;
    
    vpt = (!blocks)
      ? malloc(sizeof(VoiceSeq *))
      : realloc(seq_block, sizeof(VoiceSeq *) * (blocks + 1));
    
    if(NULL == vpt) goto err_0;
    seq_block = vpt;
    
    if(NULL == (vpt = malloc(sizeof(VoiceSeq) * (term + 2)))) goto err_1;
    
    seq_block[blocks] = vpt;
    
    for(i = 0; i < term; i++)
      {
	final_time = seq[begin + i].time;
	seq_block[blocks][i].time     = seq[begin + i].time - begin_time;

	/* ビッグエンディアンの場合は、上位、下位を入れ換える。 */
	if(big_endian) 
	  {
	    int t, l, h;
	    t = seq_block[blocks][i].time;
	    l = t & 0xff;
	    h = t >> 8;
	    seq_block[blocks][i].time = (l << 8) | h;
	  }
	seq_block[blocks][i].phone_id = seq[begin + i].phone_id;
	seq_block[blocks][i].power    = seq[begin + i].power;
      }
    /* 終端コードの記録 */
    seq_block[blocks][i].time     = 0xffff;
    seq_block[blocks][i].phone_id = PHONE_unknown;
    seq_block[blocks][i].power    = 0;

    blocks++;
    begin = begin + term;
    begin_time = final_time;
  } while(seq[begin].time != 0xffff);


  vpt = (!blocks)
    ? malloc(sizeof(VoiceSeq *))
    : realloc(seq_block, sizeof(VoiceSeq *) * (blocks + 1));
  if(NULL == vpt) goto err_0;

  seq_block = vpt;
  seq_block[blocks] = NULL;

  return seq_block;

 err_1:
 err_0:
  for(i = 0; i < blocks; i++) free(seq_block[i]);
  if(blocks > 0) free(seq_block);

  return NULL;
}

/*
 * パケット単位に分割済のシーケンスデータを出力する
 */
void splOutput(FILE *wfp, VoiceSeq ** sp_seq, int packet_size)
{
  int i, j;
  VoiceSeq * seq;
  long siz;
  long total_time;
  long top_time;
  long flags;

  top_time = 0;
  flags = 0;
  for(i = 0; sp_seq[i] != NULL; i++)
    {
      seq = sp_seq[i];
      siz = 0;
      total_time = 0;
      if(sp_seq[i + 1] == NULL) flags |= FLAGS_FINAL_PACKET;

      /* 全体のサイズと時間を計測する */
      for(j = 0; seq[j].time != 0xffff; j++)
	{
	  /* サイズの計測 */
	  siz += sizeof(VoiceSeq);

	  /* 時間の計測 */
	  /* トータルの時間は 1/300秒単位で出力する
	     (シーケンスデータ内部は 1/100秒単位)   */
	  total_time = seq[j].time * 3;
	}
      siz += MIN_SIZE;    /* その他、必要な項目のサイズを追加 */

      if(!big_endian)
	{
	  /* Little Endian 時の書き込み */
	  utWriteLong(0,        wfp);  /* 予約 */
	  utWriteLong(siz,      wfp);  /* ヘッダを含むデータのサイズ      */
	  utWriteLong(top_time, wfp);  /* 1/300秒単位での、先頭からの時刻 */
	  utWriteLong(flags,    wfp);  /* 各種フラグ                      */
	}
      else
	{
	  /* Big Endian 時の書き込み */
	  utWriteLongBE(0,        wfp);  /* 予約 */
	  utWriteLongBE(siz,      wfp);  /* ヘッダを含むデータのサイズ      */
	  utWriteLongBE(top_time, wfp);  /* 1/300秒単位での、先頭からの時刻 */
	  utWriteLongBE(flags,    wfp);  /* 予約 */
	}
      fwrite(seq, 1, siz - 16, wfp);
      top_time += total_time;

      /* パケットの残りサイズを 0 で埋める */
      if(!fips_mode) for(j = 0; j < packet_size - siz; j++) fputc(0, wfp);
    }
}
