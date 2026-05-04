#ifndef _pcmdata_h_
#define _pcmdata_h_

#include "pcminfo.h"
#include "vagdata.h"
#include "wavdata.h"

/* =========================================================================
 * Macros
 * ========================================================================= */
#define PCM_BUFLEN ((size_t)1024)     /* PCM buffering frames */

#define PCM_OUTFREQ 16000.0           /* 取り出すときは 16[KHz] のデータ
					 として取り出す */
/*
 * Type defines
 */
typedef struct pcm_file {
  FILE  * fp;           /* File Pointer                */

  InfoPCM    info;      /* サンプリング周波数変換や、総フレーム数などの情報 */

  union {
    void       * ptr;   /* 単純にポインタとして欲しい場合に使用 */
    VAG_header * vag;   /* VAG 形式のヘッダ構造体ポインタ       */
    WAV_header * wav;   /* MS-RIFF 形式のヘッダ構造体ポインタ   */
  } header;

  pcmMethod * pcm_methods;  /* PCM アクセス関数へのポインタ等 */


  short * buf;            /* Buffer Pointers             */
  long    maxsize;        /* Data size in buffer         */
  char    buf_loaded;     /* Buffer used flags           */

  int     pos;            /* PCM read position           */
  int     total_pos;      /* total position              */

  int     status;         /* PCM data status             */

  int     frames;         /* number of frames by 16[KHz] */

  double  dv, df;

  double  skip_time;      /* cut beginning sirent time   */

  short  last_lev;

} PcmFILE;



/*
 * Global variables section
 */
#ifndef _pcmdata_c_
#define EXT extern
#else
#define EXT
#endif





#undef EXT   /* End of global variables section */


/*
 * Function prototypes
 */
PcmFILE * pcmOpenStdin( FormID format );
PcmFILE * pcmOpen( char *path, FormID format );
void      pcmClose(PcmFILE *pcm);
int       pcmRead(short *buf, int frames, PcmFILE *pcm);

FormID    pcmDitectFormat(char *fname);


#endif /* _pcmdata_h_ */
