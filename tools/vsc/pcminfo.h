#ifndef _pcminfo_h_
#define _pcminfo_h_

/*
 * PCM データのフォーマット ID
 */
typedef enum {
  FORM_unknown, /* 不明なフォーマット  */

  FORM_WAV,     /* MS-RIFF 形式        */
  FORM_VAG,     /* VAG 形式            */
  FORM_AIFF     /* AIFF 形式(未対応)   */
} FormID;


typedef struct InfoPCM {
  FormID  form_id;         /* フォーマット識別用                 */
  int     sample_rate;     /* サンプリングレート                 */
  int     total_frames;    /* 全フレーム数                       */
  int     packet_frames;   /* 一回の read で読み込めるフレーム数 */
} InfoPCM;


/*
 * PCM データインタフェース関数へのポインタをまとめる構造体
 */
typedef struct pcmMethod {

  /* ヘッダ読み込み関数 */
  void * (*load_header)(void *, FILE *);

  /* パケット読み込み関数 */
  int (*read_packet)(void *, short *, long *, FILE *);

  /* PCM 共通情報を返す関数 */
  InfoPCM * (*info)(void *, InfoPCM *);
  
} pcmMethod;


#endif /* _pcminfo_h_ */
