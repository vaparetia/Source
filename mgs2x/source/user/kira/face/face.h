#ifndef _face_h_
#define _face_h_

typedef struct FC_EVM_FACE {
  /*
   * データ管理用ポインタ
   */
  struct FC_EVM_FACE * prev;     /* 直前のモデルに対応する情報     */
  struct FC_EVM_FACE * next;     /* 直後のモデルに対応する情報     */

  /*
   * モデル、モーション等の情報
   */
  DG_EVMOBJ         * evm;       /* 操作対象 EVM モデル      */
  MT_FACE_CONTROL     f_ctrl;    /* フェイスモーション構造体 */

  /*
   * 制御用変数
   */
  int                 mtn_num;   /* 再生モーション番号       */
  int                 wait_time; /* 待ち時間(固定分)         */
  int                 bias_time; /* 待ち時間(変動分幅)       */
  int                 bias_var;  /* その時点での変動分時間   */
  int                 time_cnt;  /* タイムカウンタ           */
  int                 status;    /* 再生ステータス           */
} FC_EVM_FACE;


#ifndef _face_c_
#define EXT extern
#else
/* #define ACT_LEVEL  GV_ACTOR_USER */
#define ACT_LEVEL GV_ACTOR_MANAGER
#define EXT
#endif /* _face_c_ */


#undef EXT

FC_EVM_FACE * FC_NewFaceControl(DG_EVMOBJ * evm, int name_id, int flag);
int          FC_ReleaseFaceControl(FC_EVM_FACE * face);
int          FC_ExecFaceAnim(FC_EVM_FACE * face, int id,
			     int wait_time, int bias_time);
int          FC_StopFaceAnim(FC_EVM_FACE * face);

void * NewFaceManager_forProg(void);
void * NewFaceManager(int name, int where);

#endif /* _face_c_ */
