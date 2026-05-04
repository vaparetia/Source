/*
 * .pcm 形式のサウンドデータを、16bit little endian PCM データに変換する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define _pcm2wav_c_
#include "vagdata.h"
/* #include "pcm2wav.h" */
#include "vxcodec.h"

typedef struct {
  struct _riff {
    char chunkID[4];
    long chunkSize;
    char formType[4];
    struct _fmt {
      char chunkID[4];
      long chunkSize;

      unsigned long channel_waveFormatType;
      unsigned long samplePerSec;
      unsigned long bytesPerSec;
      unsigned long bitsPerSample_blockSize;
    } fmt;
    struct _data {
      char chunkID[4];
      long chunkSize;
    } data;
  } riff;
} WAVhead;

static WAVhead def_head = {
  riff:
  {
    chunkID: { 'R', 'I', 'F', 'F'},
    chunkSize: 0,
    formType: {'W', 'A', 'V', 'E'},

    fmt:
    {
      chunkID:{'f', 'm', 't', ' '},
      chunkSize: 16,
      channel_waveFormatType: 0x00010001,
      samplePerSec: 44100,
      bytesPerSec: 88200,
      bitsPerSample_blockSize: 0x00100002
    },
    data:
    {
      chunkID: {'d', 'a', 't', 'a'},
      chunkSize: 0
    }
  }
};


/* ブロック数からリニアPCMのサイズを求めるマクロ */
#define RENIER_SIZE(blk)   ((size_t)(56 * (blk)))
#define PACKET_SIZE        (size_t)0x800


static int conv_pcm(char * pcm_name, char * wav_name)
{
  FILE * rfp, * wfp;
  UBYTE * buf;
  WORD  * wav;
  VAG_header vag_h;
  WAVhead wav_h;
  int packets, i, j;
  long siz, total;

  if(NULL == (rfp = fopen(pcm_name, "rb"))) return -1;
  if(NULL == (wfp = fopen(wav_name, "wb"))) goto err_0;

  if(NULL == vagReadHeader(&vag_h, rfp)) goto err_1;
  packets = (vag_h.wav_siz + PACKET_SIZE - 1) / PACKET_SIZE;

  if(NULL == (buf = malloc(PACKET_SIZE))) goto err_1;
  if(NULL == (wav = malloc(RENIER_SIZE(128)))) goto err_2;

  /* WAV ヘッダの領域を書き出す */
  wav_h = def_head;
  fwrite(&wav_h, sizeof(WAVhead), 1, wfp);

  initWork();
  /* 全パケットにわたり、変換を行う */
  total = siz = 0;
  for(i = 0; i < packets; i++)
    {
      /* パケットを読む */
      if(NULL == vagLoadPacket(buf, rfp))
	{
	  fprintf(stderr, "read err.\n");
	  fflush(stderr);
	}
      
      /* 変換する */
      siz = vagConvPacket(wav, buf);
      printf("siz: %d\n", siz);
      total += siz;
      /* 変換されたデータのエンディアンを変更する */
      /*
	for(j = 0; j < 24; j++)
	wav[j] = ((wav[j] << 8) & 0xff00) | (wav[j] >> 8);
      */

      /* 書き出す */
      fwrite(wav, 1, siz, wfp);
    }
      
  /* WAV ファイルヘッダの必要部分を書き出す */

  wav_h.riff.chunkSize = total + 36;
  wav_h.riff.data.chunkSize = total;

  fseek(wfp, 0L, SEEK_SET);
  fwrite(&wav_h, sizeof(WAVhead), 1, wfp);

  fclose(wfp);
  fclose(rfp);
  
  return 0;

 err_2:
  free(buf);
 err_1:
  fclose(wfp);
 err_0:
  fclose(rfp);
  return -1;
}


int main(int argc, char **argv)
{
  conv_pcm(argv[1], argv[2]);

  printf("fmt SIZE: %d\n", sizeof(struct _fmt));
}

