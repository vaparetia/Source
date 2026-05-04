/*
 * 長大な .wav データを連続して読み出すためのモジュール
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _pcmdata_c_
#include "pcmdata.h"
#include "debug.h"

typedef struct InfoPCM {
  int sample_rate;     /* サンプリングレート */
  int total_frames;    /* 全フレーム数       */
} InfoPCM;






const static struct {
  int freq;
  int status;
} freq_list[] = {
  {  8000,  PCMST_FREQ_8K },
  { 11025,  PCMST_FREQ_11K},
  { 16000,  PCMST_FREQ_16K},
  { 22050,  PCMST_FREQ_22K},
  { 44100,  PCMST_FREQ_44K},
  {     0,  0             }
};

/* ------------------------------------------------------------------------- */

/* ファイルから、16bit 値をリトルエンディアンとして読み出す */
static int fgetw(unsigned short *w, FILE *rfp)
{
  int l, h;

  if(EOF == (l = fgetc(rfp))) return EOF;
  if(EOF == (h = fgetc(rfp))) return EOF;

  *w = ((h & 0xff) << 8) | (l & 0xff);
  return (int)*w;
}

/* ファイルから、32bit 値をリトルエンディアンとして読み出す */
static int fgetl(unsigned long *l, FILE *rfp)
{
  int i;
  unsigned short w;

  if(EOF == fgetw(&w, rfp)) return EOF;
  *l = w;

  if(EOF == fgetw(&w, rfp)) return EOF;
  *l |= (unsigned long)w << 16;

  return 0;
}

/*
 * Windows RIFF 形式の fmt chunk を読み出す
 */
static int pcm_read_fmt_chunk(fmt_chunk *fmt, FILE *rfp)
{
  int i;

  /* まずは読む */

  /* fmt chunk が来るまで読み飛ばす */
  do {
    if(fread(fmt->chunkID, 1, 4, rfp) < 4) goto err0;
    if(EOF == fgetl(&(fmt->chunkSize), rfp)) goto err0;

    /* 要求される fmt chunk でなければ読み飛ばす */
    if(strncmp(fmt->chunkID, "fmt ", 4))
      fseek(rfp, fmt->chunkSize, SEEK_CUR);
  } while(strncmp(fmt->chunkID, "fmt ", 4));

  if(EOF == fgetw(&(fmt->waveFormatType), rfp)) goto err0;
  if(EOF == fgetw(&(fmt->channel), rfp)) goto err0;
  if(EOF == fgetl(&(fmt->samplesPerSec), rfp)) goto err0;
  if(EOF == fgetl(&(fmt->bytesPerSec), rfp)) goto err0;
  if(EOF == fgetw(&(fmt->blockSize), rfp)) goto err0;
  if(EOF == fgetw(&(fmt->bitsPerSample), rfp)) goto err0;

  DEBUG("fmt chunk read success.\n");

  /* chunk 等を検査する */
  if(strncmp(fmt->chunkID, "fmt", 3)) goto err1;
  if(fmt->chunkSize != 16) goto err1;

  DEBUG("fmt chunk check success.\n");

  return 0;  

 err0:
  DEBUG("fmt chunk read failed.\n");
  return -1;

 err1:
  DEBUG("fmt chunk check failed.\n");
  return -1;
}


/*
 * Windows RIFF 形式の data chunk を読み出す
 */
static int pcm_read_data_chunk(data_chunk *data, FILE *rfp)
{
  int i;

  /* まずは読む */

  /* data chunk が来るまで読み飛ばす */
  do {
    if(fread(data->chunkID, 1, 4, rfp) < 4) goto err0;
    if(EOF == fgetl(&(data->chunkSize), rfp)) goto err0;

    /* 要求される data chunk でなければ読み飛ばす */
    if(strncmp(data->chunkID, "data", 4))
      fseek(rfp, data->chunkSize, SEEK_CUR);
  } while(strncmp(data->chunkID, "data", 4));

  DEBUG("data chunk read success.\n");

  /* チェック */
  if(strncmp(data->chunkID, "data", 4)) goto err1;

  DEBUG("data chunk check success.\n");

  return 0;

 err0:
  DEBUG("data chunk read failed.\n");
  return -1;
 err1:
  printf("[%s]\n", data->chunkID);
  DEBUG("data chunk check failed.\n");
  return -1;
}

/*
 * MS RIFF 形式のヘッダをファイルから読み込む
 */
static int pcm_read_riff_header(MS_RIFF_Header *header, FILE *rfp)
{
  /*
   * まず、とにかく読む
   */
  DEBUG("  read header-----------\n");

  if(fread(header->chunkID, 1, 4, rfp) < 4) return -1;
  if(EOF == fgetl(&(header->chunkSize), rfp)) return -1;
  if(fread(header->formType, 1, 4, rfp) < 4) return -1;

  DEBUG("RIFF header read success.\n");

  if(pcm_read_fmt_chunk(&(header->fmt), rfp) < 0) return -1;
  if(pcm_read_data_chunk(&(header->data), rfp) < 0) return -1;

  /*
   * 読み込んだ内容を検査し、正しい .wav かどうかを確認する
   */
  /* chunk ID の比較 */
  if(strncmp(header->chunkID, "RIFF", 4)) return -1;
  if(strncmp(header->formType, "WAVE", 4)) return -1;

  return 0;
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
static PcmFILE * pcm_open_core(FILE *rfp)
{
  PcmFILE *pcm;
  short * tmp;
  int i, j, v, vm;
  int fsize, frames;
  unsigned char buf[4];
  int blank;   /* 読み飛ばした、先頭空白のフレーム数 */

  if(NULL == (tmp = malloc(sizeof(short) * PCM_BUFLEN))) return NULL;

  if(NULL == (pcm = malloc(sizeof(PcmFILE))))
    {
      free(tmp);
      return NULL;
    }

  for(i = 0; i < 2; i++)
    {
      pcm->buf = tmp;
      pcm->buf_loaded = 0;
    }
  pcm->fp = rfp;

  pcm->pos = 0;
  pcm->fgBuf = 0;

  /* ヘッダ情報の読み込み */
  pcm_read_riff_header(&(pcm->header), pcm->fp);

  /* PCM データステータスの作成 */
  pcm->status = 0;

  /* サンプリング周波数は、
     8000  [Hz]
     11025 [Hz]
     16000 [Hz]
     22050 [Hz]
     44100 [Hz]
     の中で、もっとも近いものと見做す。 */
  vm = 65536;
  for(i = 0; freq_list[i].freq; i++)
    {
      v = pcm->header.fmt.samplesPerSec - freq_list[i].freq;
      if(v < 0) v *= -1;
      if(v < vm)
	{
	  vm = v;
	  pcm->status = freq_list[i].status;
	  pcm->df = PCM_OUTFREQ / (double)freq_list[i].freq;
	}
    }
  pcm->dv = (pcm->df <= 1.0) ? (1.0 - pcm->df) : 0.5;

  fsize = 1;
  /* チャネルは MONO/STEREO のいずれか */
  switch(pcm->header.fmt.channel)
    {
    case 1:   /* MONO */
      pcm->status |= PCMST_CHAN_MONO;
      break;
    case 2:
      pcm->status |= PCMST_CHAN_STEREO;
      fsize *= 2;
      break;
    }

  /* サンプリングビットは 8/16 のいずれか */
  switch(pcm->header.fmt.bitsPerSample)
    {
    case 8:
      pcm->status |= PCMST_BITS_8;
      break;
    case 16:
      pcm->status |= PCMST_BITS_16;
      fsize *= 2;
      break;
    }

  /* 元ファイルに含まれるトータルのフレーム数 */
  frames = (pcm->header.data.chunkSize / fsize);

  /* 空白のフレームを読み飛ばす */
  blank = 0;
  while(fread(buf, fsize, 1, pcm->fp))
    {
      for(i = v = 0; i < fsize; i++) v |= buf[i];
      if(v != 0)
	{
	  fseek(rfp, -fsize, SEEK_CUR);
	  break;
	}
      frames--;
      blank++;
    }

  /* 16[KHz] に換算したフレーム数を求める(先頭の空白除去) */
  pcm->frames = (int)((double)frames * pcm->df);

  /* 読み飛ばした先頭の空白分の時間を計算する。
     時系列を表示する際は、この時間を加算した時間を表示する
     (実際の WAVE と同期をとるため)                         */
  pcm->skip_time = (double)(blank * pcm->df * 100) / 16000;
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
PcmFILE * pcmOpenStdin(void)
{
  return pcm_open_core(stdin);
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
PcmFILE * pcmOpen(char *path)
{
  FILE *rfp;
  PcmFILE *pcm;

  if(NULL == (rfp = fopen(path, "rb"))) return NULL;
  if(NULL == (pcm = pcm_open_core(rfp))) fclose(rfp);

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
 * PcmFILE 構造体の読み込みバッファを満たす。
 * その際に、入力元のPCMデータを 16[KHz]/16bit のデータに変換する。
 *
 * Prototype:  static int pcm_read_buffer(PcmFILE *pcm)
 *
 * Args:       pcm        バッファを満たす対象となる PcmFILE 構造体のポインタ
 * Return:     int        バッファ内の有効フレーム数
 * 
 * 本モジュール内においてのみ参照可能
 * ========================================================================= */
static int pcm_read_buffer(PcmFILE *pcm)
{
  int i, j, c;
  int left, right, lev;

  j = 0;
  if(pcm->df <= 1.0)
    {
      /* 
       * サンプリングレートが PCM_OUTFREQ 以上の場合
       */
      while(j < PCM_BUFLEN)
	{
	  if(EOF == (left = fgetc(pcm->fp))) return j;
	  if(pcm->status & PCMST_BITS_16)
	    {
	      if(EOF == (c = fgetc(pcm->fp))) return j;
	      left &= 0xff;
	      left |= (c & 0xff) << 8;
	    }
	  if(pcm->status & PCMST_CHAN_STEREO)
	    {
	      if(EOF == (right = fgetc(pcm->fp))) return j;
	      if(pcm->status & PCMST_BITS_16)
		{
		  if(EOF == (c = fgetc(pcm->fp))) return j;
		  right &= 0xff;
		  right |= (c & 0xff) << 8;
		}
	      lev = (left + right) / 2;
	    }
	  else lev = left;
	  
	  if((pcm->dv += pcm->df) >= 1.0)
	    {
	      if(pcm->status & PCMST_BITS_8) lev = (lev - 0x80) * 256;
	      else if(lev & 0x8000) lev = -(((lev ^ 0xffff) + 1) & 0xffff);
	      *(pcm->buf + j) = lev;
	      pcm->dv -= 1.0;
	      j++;
	    }
	}
    }
  else
    {
      /*
       * サンプリングレートが PCM_OUTFREQ 未満の場合
       */
      while(j < PCM_BUFLEN)
	{
	  if(pcm->dv < 1.0)
	    {
	      if(EOF == (left = fgetc(pcm->fp))) return j;
	      if(pcm->status & PCMST_BITS_16)
		{
		  if(EOF == (c = fgetc(pcm->fp))) return j;
		  left &= 0xff;
		  left |= (c & 0xff) << 8;
		}
	      if(pcm->status & PCMST_CHAN_STEREO)
		{
		  if(EOF == (right = fgetc(pcm->fp))) return j;
		  if(pcm->status & PCMST_BITS_16)
		    {
		      if(EOF == (c = fgetc(pcm->fp))) return j;
		      right &= 0xff;
		      right |= (c & 0xff) << 8;
		    }
		  lev = (left + right) / 2;
		}
	      else lev = left;
	      
	      if(pcm->status & PCMST_BITS_8) lev = (lev - 0x80) * 256;
	      else if(lev & 0x8000) lev = -(((lev ^ 0xffff) + 1) & 0xffff);
	      pcm->last_lev = lev;
	      pcm->dv += pcm->df;
	    }
	  while((pcm->dv >= 1.0) && j < PCM_BUFLEN)
	    {
	      *(pcm->buf + j) = pcm->last_lev;
	      pcm->dv -= 1.0;
	      j++;
	    }
	}
    }
  return j;  
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
  for(f = 0; f < frames; f++)
    {
      if(!pcm->buf_loaded)
	{
	  pcm->maxsize = pcm_read_buffer(pcm);
	  pcm->pos = 0;
	  pcm->buf_loaded = 1;
	}
      if(pcm->pos >= pcm->maxsize) break;
      buf[f] = *(pcm->buf + pcm->pos);
      cnt++;
      if(++pcm->pos >= PCM_BUFLEN) pcm->buf_loaded = 0;
    }
  return cnt;  
}
