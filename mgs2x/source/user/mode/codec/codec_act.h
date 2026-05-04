#ifndef _codec_act_h_
#define _codec_act_h_

#define CDCACT_DefaultAction 0x0008a3fb       /* デフォルトモーション       */
#define CDCACT_Enter         0x0088dd16       /* 無線モード起動             */
#define CDCACT_showPRESS     0x00a07b66       /* "showPRESS"                */
#define CDCACT_hidePRESS     0x00958082       /* "hidePRESS"                */
#define CDCACT_Exit          0x00246d94       /* 無線モード終了             */
#define CDCACT_Select        0x0087a180       /* 周波数選択中               */
#define CDCACT_MemOpen       0x00f9611e       /* メモリー選択モード開き     */
#define CDCACT_OpenMsg       0x0088576d       /* メッセージパネル開き       */
#define CDCACT_MemSelect     0x00d7e4c0       /* メモリー選択モード選択中   */
#define CDCACT_MemClose      0x006a4ce3       /* メモリー選択モード閉じ     */
#define CDCACT_Call          0x002311ec       /* 呼び出し中                 */
#define CDCACT_NoResponseON  0x00c3fbd9       /* 「応答ありません」表示     */
#define CDCACT_NoResponseOFF 0x007f7a7e       /* 「応答ありません」表示終了 */
#define CDCACT_Open          0x00294d0e       /* 接続,通信開始              */
#define CDCACT_Talk          0x002b91eb       /* 会話中                     */
#define CDCACT_CallClose     0x00adeea1       /* 会話終了                   */
#define CDCACT_Close         0x0067cac9       /* 通信終了,切断              */
#define CDCACT_KeyWait       0x00bd2525       /* テキスト送りキー入力待ち   */
#define CDCACT_EnterTalk     0x00941fbc       /* アクション無し無線 IN      */
#define CDCACT_ExitTalk      0x006dd8c4       /* アクション無し無線 OUT     */

#define CDCACT_OpenMovie     0x005bffa6       /* ムービー画面開き           */
#define CDCACT_Movie         0x0009658a       /* ムービー再生中             */
#define CDCACT_CloseMovie    0x00d8fb1c       /* ムービー画面閉じ           */
#define CDCACT_hideMovie     0x0074c754       /* "hideMovie" */


#define CDCNUM_num0          0x0038e1d0       /* 数字 0 */
#define CDCNUM_num1          0x0038e1d1       /* 数字 1 */
#define CDCNUM_num2          0x0038e1d2       /* 数字 2 */
#define CDCNUM_num3          0x0038e1d3       /* 数字 3 */
#define CDCNUM_num4          0x0038e1d4       /* 数字 4 */
#define CDCNUM_num5          0x0038e1d5       /* 数字 5 */
#define CDCNUM_num6          0x0038e1d6       /* 数字 6 */
#define CDCNUM_num7          0x0038e1d7       /* 数字 7 */
#define CDCNUM_num8          0x0038e1d8       /* 数字 8 */
#define CDCNUM_num9          0x0038e1d9       /* 数字 9 */


#define CDCOBJ_num1_00       0x00ea1ea0       /* 1.00 オブジェクト名        */
#define CDCOBJ_num0_10       0x00e99ec0       /* 0.10 オブジェクト名        */
#define CDCOBJ_num0_01       0x00e99ea1       /* 0.01 オブジェクト名        */

#define CDCOBJ_ROOT          0x002a4634       /* ROOT オブジェクト      */
#define CDCOBJ_NoResText     0x002a4634       /* NoResText オブジェクト */
#define CDCOBJ_MovieRect     0x00cb372c       /* MovieRect オブジェクト */

#define CDCLEV_level6        0x008ca294
#define CDCLEV_level7        0x008ca295
#define CDCLEV_level8        0x008ca296
#define CDCLEV_level9        0x008ca297

/* 中央メータのレベル点灯部 */
#define CDCOBJ_level_b_light  0x000c2f1e
#define CDCOBJ_level_t_light  0x000c339e


#endif /* _codec_act_h_ */
