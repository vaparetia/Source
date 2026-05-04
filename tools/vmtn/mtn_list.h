#ifndef _mtn_list_h_
#define _mtn_list_h_


#define LINE_MAX  240

/*
 * モーションリスト用のデータ構造
 */
/* モーションの組合せ情報 */
typedef struct mtn_cel {
  float time;
  float rate;     /* レート(大きいほど B に近い) */
  int   mtn_a;    /* モーション A                */
  int   mtn_b;    /* モーション B                */
} mtn_cel;

/* 音素ごとのモーション情報 */
typedef struct mtn_voice {
  int   vid;      /* 音素 ID                     */
  int   vow_flg;  /* 母音的音素の場合、ON になる */
  char  label[5]; /* 音素の表記                  */

  mtn_cel   mtn[3];   /* 最大3ステップまで */

} mtn_voice;

typedef struct lipMotionList {
  int         voices;   /* 音素の種類数       */
  mtn_voice * mtn;      /* モーション管理情報 */
} lipMotionList;

#ifndef _mtn_list_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)  = n
#endif /* _mtn_list_c_ */



#undef EXT
#undef INIT(n)

lipMotionList * lipLoadMotionList(char * fname);
void            lipReleaseMotionList(lipMotionList * list);

#endif /* _mtn_list_h_ */

