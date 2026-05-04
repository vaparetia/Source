#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define _vagdata_c_
#include "vagdata.h"
#include "util.h"
#include "vxcodec.h"


#ifdef _DEBUG_
#define DBG(args...) fprintf(stderr, args);  fflush(stderr)
#else
#define DBG(args...)
#endif /* _DEBUG_ */


VAG_header * vagReadHeader(VAG_header * arghead, FILE *rfp)
{
  VAG_header * head;

  head = (arghead == NULL) ? malloc(sizeof(VAG_header)) : arghead;
  if(NULL == head) return NULL;

  utReadLongBE(&(head->wav_siz), rfp);

  utReadWordBE(&(head->volume), rfp);
  utReadWordBE(&(head->rate), rfp);

  head->channel     = fgetc(rfp);
  head->BGMvol_flag = fgetc(rfp);

  fseek(rfp, 0x800, SEEK_SET);

#ifdef _DEBUG_
  fprintf(stderr, "wav_siz: %d\n", head->wav_siz);
  fprintf(stderr, "volume:  %d\n", head->volume);
  fprintf(stderr, "rate:    %d\n", head->rate);
  fprintf(stderr, "channel: %d\n", head->channel);
  fprintf(stderr, "BGMvol:  %d\n", head->BGMvol_flag);
#endif /* _DEBUG_ */

  return head;
}

/*
 * 1パケット分のデータを読む
 */
UBYTE * vagLoadPacket(UBYTE *buf, FILE *rfp)
{
  DBG("entering: vagLoadPacket()\n");

  if(fread(buf, 0x800, 1, rfp) < 1) return NULL;
  return buf;
}

/*
 * 1パケット分のデータを変換する
 */
LONG vagConvPacket(WORD * raw, UBYTE *vag)
{
  LONG siz;

  DBG("entering vagConvPacket()\n");
  /* VAGデータは、1パケット = 128ブロック(???) */
  siz = decodeVAG(vag, raw, 128);
  DBG("WAV siz by 1 packet = %d\n", siz);
  return siz;
}

/*
 * VAG_header を参照し, DSP デバイスの再生条件を設定する
 */
int vagSetDSPdevice(int fd, VAG_header *head)
{
  int status;
  int rate, channel, sample;

  rate    = head->rate;
   channel = head->channel;
  sample  = 16;

  if(0 > (status = ioctl(fd, SNDCTL_DSP_RESET))) goto err;
  if(0 > (status = ioctl(fd, SNDCTL_DSP_SPEED, &rate))) goto err;
  if(0 > (status = ioctl(fd, SNDCTL_DSP_CHANNELS, &channel))) goto err;
  if(0 > (status = ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &sample))) goto err;

  return 0;
 err:
  DBG("DSP setup error.\n");
  return status;
}


/*
 * .vag データをメモリバッファにリニア VAG データとして展開する
 */
WORD * vagDecodeAll(LONG *retsize, VAG_header *head, FILE *rfp)
{
  UBYTE * vag;
  WORD * buf;
  long siz, pos, s, sz;
  int packets;
  int f, i;
  /* 考えられるかぎり最大のメモリバッファを、最初に確保する */
  packets = ((size_t)head->wav_siz + 0x7ff) / 0x800;   /* パケット数 */
  DBG("packet = %d, siz = %d\n", packets, packets * 7168);

  if(NULL == (buf = malloc(packets * 7168))) return NULL;
  if(NULL == (vag = malloc(0x800)))
    {
      free(buf);
      return NULL;
    }


  siz = head->wav_siz;
  pos = 0;
  initWork();
  f = 0;
  while(siz > 0)
    {
      sz = fread(vag, 1, 0x800, rfp);
      s = decodeVAG(vag, buf + pos, sz / 16);
      pos += s / 2;
      siz -= sz;
    }
  free(vag);
  buf = realloc(buf, pos * sizeof(WORD));
  *retsize = pos * sizeof(WORD);
  return (WORD *)buf;
}
