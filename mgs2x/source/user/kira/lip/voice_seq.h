#ifndef _voice_seq_h_
#define _voice_seq_h_

/* 定義すると、1ブロック4バイト仕様に準拠 */
#define SEQ_BLOCK_IS_4BYTES


#ifdef SEQ_BLOCK_IS_4BYTES
/* -------------------------------------------------------------------------
 *
 *  音素セグメント一つ分を4バイトで表現する仕様に準拠
 *
 * ------------------------------------------------------------------------- */
typedef struct VoiceSeq {
  unsigned short   time;      /* 時刻    */
  unsigned char    phone_id;  /* 音素 ID */
  unsigned char    power;     /* パワー  */
} VoiceSeq;

#define VOICE_TERM   0xffff

/* 終端かどうかを判別するマクロ */
#define IsVoiceTerminal(v)   ((v)->time == VOICE_TERM)
#define VoicePower(v)        ((float)(v)->power / (float)255)

#else /* SEQ_BLOCK_IS_4BYTES *************************************************/

/* -------------------------------------------------------------------------
 *
 *  音素セグメント一つ分を8バイトで表現する仕様に準拠
 *
 * ------------------------------------------------------------------------- */
typedef struct VoiceSeq {
  int   time;
  short phone_id;
  short power;
} VoiceSeq;

#define VOICE_TERM   (-1)

#define IsVoiceTerminal(v)   ((v)->time < 0)
#define VoicePower(v)        ((float)(v)->power / (float)256)

#endif /* SEQ_BLOCK_IS_4BYTES */

#endif /* _voice_seq_h_ */
