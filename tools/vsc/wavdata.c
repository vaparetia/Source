#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _wavdata_c_
#include "wavdata.h"
#include "fileutil.h"

#ifdef _PCMDATA_DEBUG_
#ifndef _WAVDATA_DEBUG_
#define _WAVDATA_DEBUG_
#endif /* _WAVDATA_DEBUG_ */
#endif /* _PCMDATA_DEBUG_ */

#ifdef _WAVDATA_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _WAVDATA_DEBUG_ */

#include "debug.h"

pcmMethod pcmMethodsWAV = {
  wavLoadWAVheader,
  wavReadPacket,
  wavInfo
};

WAV_header * wavLoadWAVheader(WAV_header * arghead, FILE *rfp)
{
  WAV_header * head;
  char chunk[4];
  int doing = 1;
  long siz;

  DBG("entering wavLoadWAVheader()\n");

  head = (arghead == NULL) ? malloc(sizeof(WAV_header)) : arghead;
  if(NULL == head) return NULL;

  fseek(rfp, 0L, SEEK_SET);

  /* MS-RIFF header を読み込み、必要な fmt chunk データのみを抽出する。 */
  while(doing)
    {
      fread(chunk, 1, 4, rfp);  /* chunk ID の読み込み */
      fgetlLE(&siz, rfp);       /* chunk size の読み込み */
      if(!strncmp(chunk, "RIFF", 4))
	{
	  DBG("'RIFF' chunk reading\n");
	  fread(chunk, 1, 4, rfp);
	  /* 'WAVE' の文字列がなければ、フォーマットが不正 */
	  if(strncmp(chunk, "WAVE", 4))
	    {
	      if(NULL == arghead) free(head);
	      return NULL;
	    }
	  
	  continue;
	}
      if(!strncmp(chunk, "data", 4))
	{
	  DBG("'data' chunk reading.\n");
	  head->datasize = siz;   /* chunk のサイズが、
				     そのままリニア PCM のサイズとなる */
	  doing = 0;
	  continue;
	}
      if(strncmp(chunk, "fmt ", 4))
	{
	  DBG("unknown chunk skipping...\n");
	  fseek(rfp, siz, SEEK_CUR);  /* 必要な chunk ではない場合読み飛ばす */
	  continue;
	}

      fprintf(stderr, "'fmt ' chunk reading...\n");
      fgetwLE(&(head->waveFormatType), rfp);
      fgetwLE(&(head->channel),        rfp);
      fgetlLE(&(head->samplesPerSec),   rfp);
      fgetlLE(&(head->bytesPerSec),    rfp);
      fgetwLE(&(head->blockSize),      rfp);
      fgetwLE(&(head->bitsPerSample),  rfp);
    }
  return head;
}


int wavReadPacket(WAV_header * head,
		  short * retbuf, long * retsiz, FILE *rfp)
{
  int i, tmp;
  signed short l, r;

  DBG("entering wavReadPacket().\n");

  for(i = 0; i < RENIER_FRAMES; i++)
    {
      if(head->bitsPerSample == 8)
	{
	  if(EOF == (tmp = fgetc(rfp))) break;
	  l = (tmp << 8) - 32768;
	}
      else 
	if(EOF == fgetwLE(&l, rfp)) break;

      if(head->channel == 2)
	{
	  if(head->bitsPerSample == 8)
	    {
	      if(EOF == (tmp = fgetc(rfp))) break;
	      r = (tmp << 8) - 32768;
	    }
	  else 
	    if(EOF == fgetwLE(&r, rfp)) break;
	  tmp = (int)r + (int)l;
	  l = tmp / 2;
	}
      retbuf[i] = l;
    }
  *retsiz = i;
  return 0;
}

/*
 * PCM 共通情報を返す
 */
InfoPCM * wavInfo(WAV_header * head, InfoPCM * info)
{
  info->form_id       = FORM_WAV;
  info->sample_rate   = head->samplesPerSec;
  info->total_frames  = head->datasize / head->blockSize;
  info->packet_frames = RENIER_FRAMES;

  DBG("sample_rate: %d\n", info->sample_rate);
  DBG("total_frames: %d\n", info->total_frames);
  DBG("packet_frames: %d\n", info->packet_frames);

  return info;
}
