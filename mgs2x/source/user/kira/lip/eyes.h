/*
 * 目パチ関連の定数
 */
#ifndef _eyes_h_
#define _eyes_h_

#define EYES_STAT_OPEN     0    /* 開いている状態(通常) */
#define EYES_STAT_CLOSING  1    /* 閉じている途中       */
#define EYES_STAT_CLOSED   2    /* 閉じている状態       */
#define EYES_STAT_OPENING  3    /* 開いている途中       */

#define EYES_FLAG_ENABLE   0x00000001  /* 目パチ許可 */

/*
 * 自動目パチの、各種時間制御用 (時間単位: 1/300[s])
 *
 * 会話非同期目パチの間隔: 約4秒前後(参考資料: 王立宇宙軍オネアミスの翼)
 */
#define EYES_BLANK_MIN     1200 /* 最低でもこの間隔を空ける             */
#define EYES_BLANK_MERGINE  800 /* 乱数でこの値までの時間を上乗せする   */


/*
 * 開閉のスピードを表す値は、この時間でまぶたが開閉するという意味。
 * 値を小さくすれば速く、大きくすれば遅くなる。時間単位は 1/300[s]
 */
#define EYES_CLOSE_SPEED   15   /* まぶたの閉じるスピード               */
#define EYES_OPEN_SPEED    23   /* まぶたの開くスピード                 */

#define EYES_RND(n)  (BP_PS2_rand() % (n))  /* 0～(n - 1) の乱数を発生するマクロ */

#endif /* _eyes_h_ */
