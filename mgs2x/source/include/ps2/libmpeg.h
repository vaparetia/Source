//BP - Stub file
#ifndef __LIBMPEG_H__
#define __LIBMPEG_H__

#ifndef SYS_COMMON_TYPES
typedef unsigned char        u_char;
typedef unsigned int         u_int;
typedef unsigned long long   u_long64;
#endif

typedef enum {
   sceMpegCbError      = 0,
   sceMpegCbNodata     = 1,
   sceMpegCbStopDMA    = 2,
   sceMpegCbRestartDMA = 3,
   sceMpegCbBackground = 4,
   sceMpegCbTimeStamp  = 5,
   sceMpegCbStr        = 6
} sceMpegCbType;

/* data structure for sceMpegCbError callback */
typedef struct {
   sceMpegCbType type;
   char *errMessage;
} sceMpegCbDataError;

/* data structure for sceMpegCbTimeStamp callback */
typedef struct {
   sceMpegCbType type;
   int64 pts;       /* PTS value; valid only when pts >= 0 */
   int64 dts;       /* DTS value; valid only when dts >= 0 */
} sceMpegCbDataTimeStamp;

/* data structure for sceMpegCbStr callback */
typedef struct {
   sceMpegCbType type;
   u_char *header; /* the begining of packet header */
   u_char *data;   /* the begining of packet data */
   u_int  len;     /* length of packet data */
   int64   pts;     /* PTS value; valid only when pts >= 0 */
   int64   dts;     /* DTS value; valid only when dts >= 0 */
} sceMpegCbDataStr;

/* Universal callback data structure */
/* sceMpegCbData is used as the default callback data structure */
typedef union {
   sceMpegCbType type;
   sceMpegCbDataError error;
   sceMpegCbDataTimeStamp ts;
   sceMpegCbDataStr str;
} sceMpegCbData;

typedef struct {
   int width;      /*  width of decoded image */
   int height;     /*  height of decoded image */
   int frameCount; /*  frame number in the stream */

   int64 pts;       /*  PTS(Presentation Time Stamp) value  */
   /*  pts is valid only when pts >= 0 */

   int64 dts;       /*  DTS(Decoding Time Stamp) value */
   /*  dts is valid only when dts >= 0 */

   u_long64 flags;   /*  flags */

   int64 pts2nd;    /*  PTS for 2nd field(for future use) */
   int64 dts2nd;    /*  DTS for 2nd field(for future use) */
   u_long64 flags2nd;    /*  flags for 2nd field(for future use) */

   void *sys;      /*  system data for decoding */
} sceMpeg;

typedef int (*sceMpegCallback)(sceMpeg *mp,
                               sceMpegCbData *cbData, void *anyData);

int sceMpegDemuxPss(sceMpeg *mp, u_char *pss, int pss_size);

#define SCE_MPEG_BUFFER_SIZE(w, h)  ((w)*(h)*9/2 + 512 + 24*64 + 8192)

typedef enum {
   sceMpegStrM2V       = 0,    /* MPEG2 video stream */
   sceMpegStrIPU       = 1,    /* IPU stream */
   sceMpegStrPCM       = 2,    /* PCM stream */
   sceMpegStrADPCM     = 3,    /* ADPCM stream */
   sceMpegStrDATA      = 4     /* DATA stream */
} sceMpegStrType;

#endif//__LIBMPEG_H__
