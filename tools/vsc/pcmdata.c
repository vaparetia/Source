/*
 * 長大な .wav データを連続して読み出すためのモジュール
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _pcmdata_c_
#include "pcmdata.h"

/* 各フォーマットに特化したモジュールのヘッダ */
#include "wavdata.h"
#include "vagdata.h"

#ifdef _PCMDATA_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _PCMDATA_DEBUG_ */

#include "debug.h"


const static struct {
  FormID form;
  pcmMethod * methods;
} methods_list[] = {
  { FORM_WAV, &pcmMethodsWAV },
  { FORM_VAG, &pcmMethodsVAG },

  { FORM_unknown, NULL}
};

const static struct {
  char * suffix;
  FormID format;
} form_list[] = {
  { ".wav" , FORM_WAV },
  { ".wave", FORM_WAV },
  { ".pcm",  FORM_VAG },
  { ".vag",  FORM_VAG },
  { ".aif",  FORM_AIFF },
  { ".aiff", FORM_AIFF },
  { NULL, FORM_unknown }
};


/* ------------------------------------------------------------------------- */

static int pcm_read_packet(PcmFILE *pcm)
{
  pcm->pcm_methods->read_packet(pcm->header.ptr, pcm->buf,
				&pcm->maxsize, pcm->fp);
  pcm->buf_loaded = 1;
  pcm->pos = 0;
}


/* =========================================================================
 * ストリーム構造体の示す入力から、PcmFILE 構造体を生成し、そのポインタを返す
 *
 * Prototype:   static PcmFILE * pcm_open_core(FILE *rfp)
 *
 * Args:        rfp        PCM データとして見なす入力ストリーム
 * Return:      PcmFILE *  生成された PcmFILE 構造体のポインタ
 *                         エラー時には NULL を返す
 *
 * 本モジュール内でのみ有効な関数。
 * ========================================================================= */
static PcmFILE * pcm_open_core(FILE *rfp, FormID format)
{
  PcmFILE *pcm;
  short * tmp;
  int i, j, v, vm;
  int fsize, frames;
  unsigned char buf[4];
  int blank;   /* 読み飛ばした、先頭空白のフレーム数 */

  fprintf(stderr, "entering pcm_open_core()\n"); fflush(stderr);
  if(NULL == (pcm = malloc(sizeof(PcmFILE)))) return NULL;

  pcm->pcm_methods = NULL;
  for(i = 0; methods_list[i].form != FORM_unknown; i++)
    {
      fprintf(stderr, "---\n");
      if(methods_list[i].form == format)
	{
	  pcm->pcm_methods = methods_list[i].methods;
	  break;
	}
    }
  /* 対応しているフォーマットではない */
  if(NULL == pcm->pcm_methods)
    {
      free(pcm);
      return NULL;
    }

  fprintf(stderr, "methods: %p\n", pcm->pcm_methods); fflush(stderr);

  /* ヘッダ情報の読み込み */
  pcm->header.ptr = pcm->pcm_methods->load_header(NULL, rfp);

  /* PCM 共通情報の取得 */
  pcm->pcm_methods->info(pcm->header.ptr, &pcm->info);

  DBG("sample_rate: %d\n", pcm->info.sample_rate);
  DBG("total_frames: %d\n", pcm->info.total_frames);
  DBG("packet_frames: %d\n", pcm->info.packet_frames);

  /* 読み込みパケットバッファを確保 */
  if(NULL == (tmp = malloc(sizeof(short) * (pcm->info.packet_frames))))
    {
      free(pcm->header.ptr);
      free(pcm);
      return NULL;
    }

  pcm->buf = tmp;
  pcm->buf_loaded = 0;
  pcm->fp = rfp;

  pcm->pos = 0;
  pcm->total_pos = 0;

  /* 元ファイルに含まれるトータルのフレーム数 */
  frames = pcm->info.total_frames;

  /* 一つのパケットに含まれるフレーム数 */
  fsize  = pcm->info.packet_frames;

  /* サンプリング周波数変換に必要な定数を求めておく */
  pcm->df = 16000.0 / (double)pcm->info.sample_rate;
  fprintf(stderr, "pcm->df = %f\n", pcm->df);
  if(pcm->df <= 1.0)
    pcm->dv = 1.0 - pcm->df;
  else
    pcm->dv = pcm->df;

  /*
   * 空白のフレームを読み飛ばし、読み飛ばした時間を
   * 1/100秒単位で pcm->skip_time に保存する
   */

  /* 最初のパケットを読み込む */
  pcm_read_packet(pcm);

  /* 空白のフレーム数を、分析フレーム単位で計算する
   * 現状では空白フレームをスキップしない仕様にしておく。*/

  blank = 0;

  /* 読み飛ばした先頭の空白分の時間を計算する。
     時系列を表示する際は、この時間を加算した時間を表示する
     (実際の WAVE と同期をとるため)                         */
  pcm->skip_time = 0;

  /* 16[KHz] に換算したフレーム数を求める(先頭の空白除去) */
  pcm->frames = (int)(frames * pcm->df);

  return pcm;
}


/* =========================================================================
 * 標準入力をPCMデータファイルとしてオープン
 *
 * Prototype:  PcmFILE * pcmOpenStdin(void)
 *
 * Args:       void
 * Return:     PcmFILE *  オープンした PCM データに対応する
 *                        PcmFILE 構造体へのポインタ
 *                        エラー時には NULL を返す
 * ========================================================================= */
PcmFILE * pcmOpenStdin( FormID format )
{
  return pcm_open_core(stdin, format);
}


/* =========================================================================
 * PCM ファイルのオープン(具体的には .wav ファイル)
 *
 * Prototype:  PcmFILE * pcmOpen(char *path);
 *
 * Args:       path       オープンする PCM データファイルのパス
 * Return:     PcmFILE *  オープンした PCM データファイルに対応する
 *                        PcmFILE 構造体へのポインタ
 *                        エラー時には NULL を返す
 * ========================================================================= */
PcmFILE * pcmOpen(char *path, FormID format)
{
  FILE *rfp;
  PcmFILE *pcm;

  if(NULL == (rfp = fopen(path, "rb"))) return NULL;
  if(NULL == (pcm = pcm_open_core(rfp, format))) fclose(rfp);

  return pcm;
}


/* =========================================================================
 * PCM ファイルのクローズ
 *
 * Prototype:  void pcmClose(PcmFILE *pcm);
 *
 * Args:       pcm        クローズする PCM データファイル管理構造体のポインタ
 * Return:     void
 * ========================================================================= */
void pcmClose(PcmFILE *pcm)
{
  free(pcm->buf);

  /* 入力ストリームが標準入力でなければ close する */
  if(pcm->fp != stdin) fclose(pcm->fp);

  free(pcm);
}


/* =========================================================================
 * 任意バッファに、任意フレーム数の PCM データを読み込む
 * 読み込まれたデータは、16KHz の PCM データに変換されている。
 *
 * Prototype:  int pcmRead(short * buf, int frames, PcmFILE *pcm)
 *
 * Args:       buf        フレームデータを読み込むバッファのポインタ
 *             frames     読み込むフレーム数
 *             pcm        読み込みの対象とする PCM ファイルの管理構造体
 *
 * Return:     int        実際に読み込んだフレーム数
 * ========================================================================= */

int pcmRead(short *buf, int frames, PcmFILE *pcm)
{
  int f, cnt;

  cnt = 0;
  if(pcm->df < 1.0)   /* 元データのサンプリング周波数が 16KHz 以上である場合 */
    {
      f = 0;
      for(cnt = 0; cnt < frames; cnt++)
	{
	  /* 間引きを行う */
	  while((pcm->dv += pcm->df) < 1.0)
	    {
	      pcm->pos++;
	      pcm->total_pos++;
	      if(pcm->total_pos >= pcm->info.total_frames)
		{
		  f = 1;
		  break;
		}
	      if(pcm->pos >= pcm->maxsize) pcm_read_packet(pcm);
	    }
	  if(f) break;
	  pcm->dv -= 1.0;
	  buf[cnt] = pcm->buf[pcm->pos];
	  pcm->pos++;
	  pcm->total_pos++;
	}
    }
  else                /* 元データのサンプリング周波数が 16KHz 未満である場合 */
    {
      for(cnt = 0; cnt < frames; cnt++)
	{
	  buf[cnt] = pcm->buf[pcm->pos];
	  if((pcm->dv - 1.0) < 1.0)
	    {
	      pcm->dv += pcm->df;
	      pcm->pos++;
	      pcm->total_pos++;
	      if(pcm->total_pos >= pcm->info.total_frames) break;
	      if(pcm->pos >= pcm->maxsize) pcm_read_packet(pcm);
	    }
	}
    }
  return cnt;
}

/* =========================================================================
 * ファイル名から、PCM データのフォーマット種別を推測する。
 *
 * 具体的には、suffix に対応するフォーマット ID を返す。
 * suffix がない、ファイル名の長さが0であるなど、推測が不可能な場合は、
 * FORM_unknown を返す。
 * ========================================================================= */
FormID pcmDitectFormat(char *fname)
{
  char * suffix;
  int pos, i;

  /* suffix を抽出する */
  pos = strlen(fname) - 1;

  while(pos >= 0)
    {
      if(fname[pos] == '/') return FORM_unknown;
      if(fname[pos] == '.') break;
      pos--;
    }
  if(pos < 0) return FORM_unknown;

  for(i = 0; form_list[i].suffix != NULL; i++)
    if(!strcmp(form_list[i].suffix, fname + pos)) return form_list[i].format;

  return FORM_unknown;
}
