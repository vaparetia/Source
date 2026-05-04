#ifndef _codec_signal_h_
#define _codec_signal_h_

#define CDC_SIGNAL_MIND_DIE  0x0001  /* 子Actor「心の声」死亡     */
#define CDC_SIGNAL_SAVE_DIE  0x0002  /* 子Actor「セーブ画面」死亡 */
#define CDC_SIGNAL_MOVIE_DIE 0x0003

#define CDC_SIGNAL_MOVIE_KILL 0x0013  /* 子Actor「ムービー画面」死亡 */
#define CDC_SIGNAL_NOISE_KILL 0x0023  /* ノイズエフェクト強制終了    */
#define CDC_SIGNAL_VIB_KILL   0x0033  /* パッド振動強制終了          */
#define CDC_SIGNAL_VFAST_SE_STOP 0x0043 /* VTR早送り SE 停止 */
#define CDC_SIGNAL_VFAST_KILL 0x0044  /* VFAST タイプのノイズのみ強制終了 */
#define CDC_SIGNAL_KILL_EXIST_SAND  0x0053 /* サンドノイズ以外を殺す */
#define CDC_SIGNAL_KILL_EXIST_VFAST 0x0054 /* VFAST ノイズ以外を殺す */

#endif /* _codec_signal_h_ */
