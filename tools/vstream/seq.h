#ifndef _seq_h_
#define _seq_h_

typedef struct {
  unsigned short time;     /* 時刻    */
  unsigned char  phone_id; /* 音素 ID */
  unsigned char  power;    /* パワー  */
} VoiceSeq;

#define PHONE_sil     0x00
#define PHONE_unknown 0xff

#define BLANK_TIME   40

#define FLAGS_FINAL_PACKET  0x00000001



#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))

#define DEFAULT_PACKET_SIZE  2048    /* デフォルトのパケットサイズ */

/* パケットの最小サイズ。このサイズよりは小さくできない。
 * (ヘッダ 8 バイト + 予約 8 バイト + 終端コード1ブロック) */
#define MIN_SIZE (16 + sizeof(VoiceSeq))

/* パケットの最大収録サイズ。このサイズ以上をパケットに収録しない。
 * 実際の収録サイズは、パケットのサイズもしくはこの値の、
 * 小さなほうが最大となる。*/
#ifdef _DEBUG_
/*
 *  デバッグ用。サイズが小さな口パクシーケンスを、無理矢理分割する
 */
#define MAX_SIZE  128   /* デバッグ用に最大128bytes に制限 */
#else
/*
 * 本番で使用する値。最大サイズはパケットのサイズと等しくなる。
 */
#define MAX_SIZE  DEFAULT_PACKET_SIZE
#endif /* _DEBUG_ */

#endif /* _seq_h_ */
