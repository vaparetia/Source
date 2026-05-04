#ifndef _vagdata_h_
#define _vagdata_h_

#include "pcminfo.h"

typedef struct VAG_header {
  unsigned long  wav_size;   /* 波形ブロックのトータルサイズ    */
  unsigned short volume;     /* 音量                            */
  unsigned short rate;       /* サンプリングレート              */
  unsigned char  channel;    /* チャネル数                      */
  unsigned char  BGMvol;     /* 再生中のBGMボリューム落しフラグ */
} VAG_header;



#ifdef _vagdata_c_

/* デコード前のパケットを展開するために必要なサイズ */
#define VAG_PACKET_SIZE  ((size_t)2048)

/* デコード前のパケットに含まれるデータブロック数 */
#define VAG_PACKET_BLOCK 128

/* 一つのパケットをデコード後に生成されるリニア PCM データのサイズ */
#define RENIER_PACKET    ((size_t)(56 * 128))

/* パケットをデコード後生成されるフレーム数 */
#define RENIER_FRAMES    (28 * 128)

#else
extern pcmMethod pcmMethodsVAG;   /* アクセス関数構造体 */
#endif /* _vagdata_c_ */



VAG_header * vagLoadVAGheader(VAG_header * arghead, FILE *rfp);
int          vagReadPacket(VAG_header * head,
			   short * retbuf, long * retsiz, FILE *rfp);
InfoPCM    * vagInfo(VAG_header * head, InfoPCM * info);
   
#endif /* _vagdata_h_ */
