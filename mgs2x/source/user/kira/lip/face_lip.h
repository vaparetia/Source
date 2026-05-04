#ifndef _face_lip_h_
#define _face_lip_h_

#define PHONE_sil     0x00    /* 無音を表す音素 ID (全言語共通)       */
#define PHONE_unknown 0xff    /* 不明な音素                           */

#define TIME_term   0xffff    /* パケット分口パクの終了を示す(時刻値) */


#define FLAGS_FINAL_PACKET   0x00000001  /* 最終パケット */


/* パケット全体のサイズ */
#if 0
#define LIP_PACKET_SIZE 0x800
#else
#define LIP_PACKET_SIZE 0x80      
#endif

/*
 * 口パクのモーションを生成するために必要な情報をまとめた構造体
 */
typedef struct {

  int pre_phone_id;    /* 直前の音素 ID       */
  int now_phone_id;    /* 現在発音中の音素 ID */

  int length;          /* 音素セグメント全体の時間的長さ(1/300秒単位) */
  int begin_time;      /* 音素セグメントの開始時刻                    */
  int delta_time;      /* セグメント内での経過時間(1/300秒単位)       */

  float pre_power;     /* 直前の音素のパワー       */
  float now_power;     /* 現在発音中の音素のパワー */

} LipInfo;

LipInfo * GetLipInfo(void);

#endif /* _face_lip_h_ */
