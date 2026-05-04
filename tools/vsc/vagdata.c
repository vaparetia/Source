#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _vagdata_c_
#include "vxcodec.h"
#include "vagdata.h"
#include "fileutil.h"

#ifdef _PCMDATA_DEBUG_
#ifndef _VAGDATA_DEBUG_
#define _VAGDATA_DEBUG_
#endif /* _VAGDATA_DEBUG_ */
#endif /* _PCMDATA_DEBUG_ */

#ifdef _VAGDATA_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _VAGDATA_DEBUG_ */

#include "debug.h"

pcmMethod pcmMethodsVAG = {
  vagLoadVAGheader,
  vagReadPacket,
  vagInfo
};

/* ヘッダを読み込み、ファイルのアクセス位置を波形データ先頭に移動する */
VAG_header * vagLoadVAGheader(VAG_header * arghead, FILE *rfp)
{
  VAG_header * head;
  int c;

  head = (arghead == NULL) ? malloc(sizeof(VAG_header)) : arghead;
  if(NULL == head) return NULL;

  fseek(rfp, 0L, SEEK_SET);
  if(EOF == fgetlBE(&head->wav_size, rfp)) goto err;
  if(EOF == fgetwBE(&head->volume,   rfp)) goto err;
  if(EOF == fgetwBE(&head->rate,     rfp)) goto err;
  if(EOF == (c = fgetc(rfp))) goto err;  head->channel = c;
  if(EOF == (c = fgetc(rfp))) goto err;  head->BGMvol = c;

  DBG("wav_size: %d\n", head->wav_size);
  DBG("volume:   %d\n", head->volume);
  DBG("rate:     %d\n", head->rate);
  DBG("channel:  %d\n", head->channel);
  DBG("BGM flag: %d\n", head->BGMvol);
  
  fseek(rfp, 0x800, SEEK_SET);
  initWork();
  return head;

 err:
  if(arghead == NULL) free(head);
  return NULL;
}

/*
 * 1パケット分(ステレオの場合は2パケット分)を読み込み、
 * モノラル16bit の PCM データに変換する(ステレオの場合も同様)
 */
int vagReadPacket(VAG_header * head, short * retbuf, long * retsiz, FILE *rfp)
{
  static WORD right_buf[RENIER_FRAMES]; /* 右チャネルパケット用バッファ */
  static UBYTE vag[VAG_PACKET_SIZE];
  int i, tmp;


  DBG("entering vagReadPacket().\n");

  /* パケットを一つ読み込む
     ( ステレオの場合、このパケットが左チャネル用の音声となる ) */
  fread(vag, 1, VAG_PACKET_SIZE, rfp);
  /* パケットをリニア PCM として展開する */

  /* 左チャネル用は直接展開する */
  *retsiz = decodeVAG(vag, retbuf, VAG_PACKET_BLOCK) / sizeof(short);

  DBG("read packets = %d\n", *retsiz);

  if(head->channel == 1) return 0;  /* 1チャネルだけの場合はここで終了 */

  /* 右チャネルの展開 */
  fread(vag, 1, VAG_PACKET_SIZE, rfp);
  /* パケットをリニア PCM として展開する */
  /* 右チャネル用は一旦別バッファに展開 */
  *retsiz = decodeVAG(vag, right_buf, VAG_PACKET_BLOCK) / sizeof(short);

  /* 右チャネルと左チャネルの合成 */
  for(i = 0; i < RENIER_FRAMES; i++)
    {
      tmp = retbuf[i];
      tmp += right_buf[i];
      retbuf[i] = tmp / 2;
    }

  return 0;

 err_0:
  free(vag);
  return -1;
}

/*
 * PCM 共通情報を返す
 */
InfoPCM * vagInfo(VAG_header * head, InfoPCM * info)
{
  info->form_id       = FORM_VAG;
  info->sample_rate   = head->rate;
  info->total_frames  = (head->wav_size / (16 * head->channel)) * 28;
  info->packet_frames = RENIER_FRAMES;

  DBG("sample_rate: %d\n", info->sample_rate);
  DBG("total_frames: %d\n", info->total_frames);
  DBG("packet_frames: %d\n", info->packet_frames);
  return info;
}
