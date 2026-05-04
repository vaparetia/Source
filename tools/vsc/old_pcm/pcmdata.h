#ifndef _pcmdata_h_
#define _pcmdata_h_


typedef struct InfoPCM {
  int sample_rate;     /* サンプリングレート */
  int total_frames;    /* 全フレーム数       */
} InfoPCM;

/* =========================================================================
 * Macros
 * ========================================================================= */
#define PCM_BUFLEN ((size_t)1024)     /* PCM buffering frames */

#define PCM_OUTFREQ 16000.0           /* 取り出すときは 16[KHz] のデータ
					 として取り出す */
/*
 * PCM status
 */

/* Sampling rate */
#define PCMST_FREQ_MASK   0x0007
#define PCMST_FREQ_8K     0x0000  /* 8000 [Hz] */
#define PCMST_FREQ_11K    0x0001  /* 11025[Hz] */
#define PCMST_FREQ_16K    0x0002  /* 16000[Hz] */
#define PCMST_FREQ_22K    0x0003  /* 22050[Hz] */
#define PCMST_FREQ_44K    0x0004  /* 55100[Hz] */

/* Sampling Bits */
#define PCMST_BITS_MASK   0x0008
#define PCMST_BITS_8      0x0000  /* 8bit Sampling  */
#define PCMST_BITS_16     0x0008  /* 16bit Sampling */

/* Sampling Channel */
#define PCMST_CHAN_MASK   0x0010
#define PCMST_CHAN_MONO   0x0000  /* MONO channel   */
#define PCMST_CHAN_STEREO 0x0010  /* STEREO channel */

/*
 * Type defines
 */

typedef struct fmt_chunk {
  char           chunkID[4];             /* "fmt"                    */
  unsigned long  chunkSize;              /* size of chunk (=16)      */
  unsigned short waveFormatType;         /* 1 = Renier PCM           */
  unsigned short channel;                /* 1 = MONO / 2 = STEREO    */
  unsigned long  samplesPerSec;          /* 11025, 22050, 44100 etc. */
  unsigned long  bytesPerSec;            /* bytes per second         */
  unsigned short blockSize;              /* bytes per frame          */
  unsigned short bitsPerSample;          /* sampling bits (8 or 16)  */
} fmt_chunk;

typedef struct data_chunk {
  char           chunkID[4];            /* "data"               */
  unsigned long  chunkSize;             /* size of chunk        */
} data_chunk;


/* MS RIFF(.wav) のヘッダ情報構造体 */
typedef struct _ms_riff_header {

  /* RIFF chunk */
  char          chunkID[4];             /* "RIFF"                */
  unsigned long chunkSize;              /* size of chunk (=sz+36)*/
  char          formType[4];            /* "WAVE"                */

  fmt_chunk     fmt;                    /* fmt chunk section     */
  data_chunk    data;                   /* data chunk_section    */
  
} MS_RIFF_Header;

typedef struct pcm_file {
  FILE  * fp;             /* File Pointer                */

  short * buf;            /* Buffer Pointers             */
  size_t  maxsize;        /* Data size in buffer         */
  char    buf_loaded;     /* Buffer used flags           */

  int     fgBuf;          /* Foreground Buffer           */
  int     pos;            /* PCM read position           */

  int     status;         /* PCM data status             */
  MS_RIFF_Header header;  /* .wav header (Windows RIFF)  */

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
PcmFILE * pcmOpenStdin(void);
PcmFILE * pcmOpen( char *path );
void      pcmClose(PcmFILE *pcm);
int       pcmRead(short *buf, int frames, PcmFILE *pcm);

#endif /* _pcmdata_h_ */
