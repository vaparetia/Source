/*
 * 口の動きのシーケンスを読む
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _read_seq_c_
#include "read_seq.h"
#include "mem_util.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)  fprintf(stderr, args); fflush(stderr)
#endif /* _DEBUG_ */




/* PASS-1 で処理されるテンポラリ用構造体 */
typedef struct {
  int    time;
  char * label;
  int    power;
} tmpSeq;


static int read_float(float * ret, char * line, int pos)
{
  float f_tmp, a, b, c;
  int p;

  /* 空白のスキップ */
  p = pos;
  while(line[p] == ' ' || line[p] == '\t') p++;

  f_tmp = 0.0F, a = 10.0F, b = c = 1.0F;
  while((line[p] >= '0' && line[p] <= '9') || (line[p] == '.'))
    {
      if(line[p] == '.')
	{
	  a = 1.0;
	  b = c = 0.1F;
	  p++;
	  continue;
	}
      f_tmp = f_tmp * a;
      f_tmp += (float)(line[p] - '0') * b;
      b = b * c;
      p++;
    }
  *ret = f_tmp;
  return p;
}

static tmpSeq * rseq_read_lip_sequence_pass1(FILE * rfp)
{
  tmpSeq * seq;
  void   * vpt;
  char     line[ LINE_MAX ];
  int      cnt;
  int      pos;
  int      i, j;

  
  seq = NULL;
  cnt = 0;

  while(NULL != fgets(line, LINE_MAX, rfp))
    {
      /* 行末の改行を除去 */
      for(i = 0; line[i]; i++)
	if(line[i] == '\n' || line[i] == '\r') { line[i] = 0; break; }

      pos = 0;
      /* 空白をスキップ */
      while(line[pos] == ' ' || line[pos] == '\t') pos++;

      /* 空行であれば、次の行へ */
      if(!line[pos]) continue;
      
      /* ワークを一つ追加 */
      if(NULL == (vpt = mem_add(seq, sizeof(tmpSeq), cnt))) goto err;
      seq = vpt;

      /* 時系列の取得 */
      {
	float tmp;
	pos = read_float(&tmp, line, pos);
	seq[cnt].time = (int)tmp;
	if(line[pos] == ':') pos++;
      }

      /* ラベルの取得 */
      {
	int p, num;
	char * label;

	/* 空白をスキップ */
	while(line[pos] == ' ' || line[pos] == '\t') pos++;

	/* ラベルの文字数をカウント */
	num = 0;
	while(line[pos + num] != ' ' && line[pos + num] != '\t') num++;

	/* 文字数分の長さの文字列が入るだけのメモリを確保 */
	if(NULL == (label = malloc(num + 1))) goto err;
	strncpy(label, line + pos, num);
	label[num] = 0;

	seq[cnt].label = label;
	pos += num;
      }

      /* パワーの取得 */
      {
	float tmp;
	pos = read_float(&tmp, line, pos);
	seq[cnt].power = (int)(255.0F * tmp);
      }
      cnt++;
    }

  /* エンドマークをつける */
  if(NULL == (vpt = mem_add(seq, sizeof(tmpSeq), cnt))) goto err;
  seq = vpt;
  seq[cnt].time = -1;
  seq[cnt].label = NULL;
  seq[cnt].power = 0;

  return seq;

 err:
  for(i = 0; i < cnt; i++) free(seq[i].label);
  free(seq);
  return NULL;
}


seqStep * rseqReadLipSequenceByStream(lipMotionList * list, FILE * rfp)
{
  tmpSeq  * t_seq;
  seqStep * seq;
  void    * vpt;
  int       cnt;
  int       step;
  int       length;
  int       i;

  /* シーケンス全体を一旦読み込む */
  DBG("[-a]\n");
  if(NULL == (t_seq = rseq_read_lip_sequence_pass1(rfp))) return NULL;
  DBG("[-b]\n");
  cnt = 0;
  for(step = 0; t_seq[step].time >= 0; step++)
    {
      DBG("label: [%s]\n", t_seq[step].label);
      /* 無音セグメントはそのまま反映 */
      if(!strcmp("sil", t_seq[step].label))
	{
	  if(NULL == (vpt = mem_add(seq, sizeof(seqStep), cnt))) goto err;
	  seq = vpt;
	  
	  seq[cnt].time  = t_seq[step].time;
	  seq[cnt].power = t_seq[step].power;
	  seq[cnt].id    = 0;  /* sil */

	  cnt++;
	  continue;
	}

      /* セグメントの長さを求める */
      length = (t_seq[step + 1].time < 0)
	? 0 : (t_seq[step + 1].time - t_seq[step].time);
      DBG("length: %d\n", length);
      /* 有音セグメントは、音素が結合している場合があるため、
	 それらを分割する。 */
      {
	int vcnt;   /* 分割数カウント */
	int pos;
	int len;
	int begin;
	int n, t;

	pos = 0;
	begin = cnt;
	vcnt = 0;
	while(t_seq[step].label[pos])
	  {
	    for(i = 0; i < list->voices; i++)
	      {
		len = strlen(list->mtn[i].label);
		if(!strncmp(list->mtn[i].label, t_seq[step].label + pos, len))
		  {
		    /* 一つ領域を確保 */
		    if(NULL == (vpt = mem_add(seq, sizeof(seqStep), cnt)))
		      goto err;

		    seq = vpt;

		    seq[cnt].time  = list->mtn[i].vow_flg + 1; /* 割り当て数 */
		    seq[cnt].power = t_seq[step].power;
		    seq[cnt].id    = list->mtn[i].vid;

		    vcnt += seq[cnt].time;

		    pos += len;
		    cnt++;
		    break;
		  }
	      }
	    if(i == list->voices)
	      {
		fprintf(stderr, "unknown voice element '%s'.\n",
			t_seq[step].label + pos);
		exit(EXIT_FAILURE);
	      }
	  }

	/* トータルの時間を分割し、割り当て数分だけ割り当てる */
	n = 0;
	for(i = begin; i < cnt; i++)
	  {
	    t = t_seq[step].time + length * n / vcnt;
	    n += seq[i].time;
	    seq[i].time = t;
	  }
      }
    }

  if(NULL == (vpt = mem_add(seq, sizeof(seqStep), cnt))) goto err;
  seq = vpt;

  seq[cnt].time  = -1;
  seq[cnt].id    = 0;
  seq[cnt].power = 0;


  DBG("~~~~~~~~~~~~~~~~~~~~~~\n");
  for(i = 0; t_seq[i].time >= 0; i++) free(t_seq[i].label);
  free(t_seq);
  DBG("~~~~~~~~~~~~~~~~~~~~~~\n");

  return seq;

 err:
  for(i = 0; t_seq[i].time >= 0; i++) free(t_seq[i].label);
  free(t_seq);
  if(seq != NULL) free(seq);
  return NULL;
}

seqStep * rseqReadLipSequence(lipMotionList * list, char * fname)
{
  FILE *rfp;
  seqStep * seq;

  if(NULL == (rfp = fopen(fname, "r"))) return NULL;
  seq = rseqReadLipSequenceByStream(list, rfp);
  fclose(rfp);

  return seq;
}

void rseqReleaseSequence(seqStep * seq)
{
  free(seq);
}
