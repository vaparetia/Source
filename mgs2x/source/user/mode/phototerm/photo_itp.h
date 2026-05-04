#ifndef _photo_itp_h_
#define _photo_itp_h_

#define ITP_STACK_SIZE (16*1024)   /* インタプリタが占有するスタックのサイズ */

#define PHOTO_ACK  0
#define PHOTO_ERR -1

typedef struct {
  /* 解釈部から表示部へのリクエスト */
  int request;
  int req[3];
  
  /* 表示部から解釈部へのリザルト */
  int result;
  int res[3];

  /* 解釈スレッドの情報 */
  void * data;
  void * data_buffer;
  void * stack;
  int stacksize;
  int id;
} PHOTO_ITP_PARAM;

enum {
  PHOTO_REQ_SETUP = -1,  /* 初期化中                         */
  PHOTO_REQ_NOREQ = 0,   /* 何らかの処理中                   */
  PHOTO_REQ_MESG_WAIT,   /* メッセージ表示後,入力待ち        */
  PHOTO_REQ_VOX_START,   /* 音声再生開始                     */
  PHOTO_REQ_SET_FRAME,   /* コマ番号を設定                   */
  PHOTO_REQ_GET_FRAME,   /* コマ番号を取得                   */
  PHOTO_REQ_LOAD_PUPPET, /* パペットのロード                 */
  PHOTO_REQ_SET_DEFACT,  /* パペットデフォルトアクション指定 */
  PHOTO_REQ_SET_ACT,     /* パペットアクション指定           */
  PHOTO_REQ_KEY_WAIT,    /* 任意キー入力待ち                 */
  PHOTO_REQ_TIME_WAIT,   /* 時間待ち                         */
  PHOTO_REQ_PHOTO_SAVE,  /* 指定の写真を保存                 */
  PHOTO_REQ_SOUND,       /* SE の発生                        */
  PHOTO_REQ_END = 255    /* 完全に終了                       */
};

#ifndef _photo_itp_c_
#define EXT extern
#else
#define EXT

#endif /* _photo_itp_c_ */


#undef EXT


int PHOTO_ItpInit(PHOTO_ITP_PARAM * param);

#endif /* _photo_itp_h_ */
