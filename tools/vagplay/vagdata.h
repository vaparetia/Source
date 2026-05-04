#ifndef _vagdata_h_
#define _vagdata_h_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>

#include "vxcodec.h"

typedef struct VAG_header {
  unsigned long  wav_siz;    /* 波形サイズ
				(ヘッダを除いた実データのサイズ)  */
  unsigned short volume;     /* 音量                              */
  unsigned short rate;       /* 周波数(サンプリングレート?)       */
  unsigned char  channel;    /* チャネル数(1: MONO  /  2: STEREO) */
  unsigned char  BGMvol_flag;/* BGM ボリュームを下げるフラグ      */

} VAG_header;

VAG_header * vagReadHeader(VAG_header * arghead, FILE *rfp);
UBYTE      * vagLoadPacket(UBYTE * vag, FILE *rfp);
LONG         vagConvPacket(WORD * raw, UBYTE * vag);
int          vagSetDSPdevicee(int fd, VAG_header * head);
WORD       * vagDecodeAll(LONG *retsiz, VAG_header *head, FILE *rfp);

#endif /* _vagdata_h_ */
