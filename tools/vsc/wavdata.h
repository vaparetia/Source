#ifndef _wavdata_h_
#define _wavdata_h_

#include "pcminfo.h"

/*
 * MS-RIFF .wav 形式の実質上ヘッダと呼べるのは、fmt chunk のみ
 */
typedef struct WAV_header {
  unsigned short waveFormatType;         /* 1 = Renier PCM           */
  unsigned short channel;                /* 1 = MONO / 2 = STEREO    */
  unsigned long  samplesPerSec;          /* 11025, 22050, 44100 etc. */
  unsigned long  bytesPerSec;            /* bytes per second         */
  unsigned short blockSize;              /* bytes per frame          */
  unsigned short bitsPerSample;          /* sampling bits (8 or 16)  */

  unsigned long  datasize;               /* renier PCM size.         */
} WAV_header;


#ifdef _wavdata_c_

#define RENIER_PACKET ((size_t)0x1000)
#define RENIER_FRAMES 0x800
#else
extern pcmMethod pcmMethodsWAV;
#endif /* _wavdata_c_ */

WAV_header * wavLoadWAVheader(WAV_header * arghead, FILE *rfp);
int          wavReadPacket(WAV_header * head,
			   short * retbuf, long * retsiz, FILE *rfp);
InfoPCM    * wavInfo(WAV_header * head, InfoPCM * info);

#endif /* _wavdata_h_ */

