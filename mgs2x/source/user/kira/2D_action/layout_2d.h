#ifndef _layout_2d_h_
#define _layout_2d_h_

#define L2D_STAT_ACT_NOT_FOUND -5
#define L2D_STAT_BAD_HANDLE    -4
#define L2D_STAT_BUSY          -3
#define L2D_STAT_UNLOAD        -2
#define L2D_STAT_NOT_ENOUGH    -1
#define L2D_STAT_ACK            0

/*
 * ポーズ状態型
 */
typedef enum {
  L2D_CONTINUE,
  L2D_PAUSE
} L2D_PAUSE_STAT;


/* ------------------------------------------------------------------------- */
#ifndef _layout_2d_c_
#define EXT extern
#else
#define EXT
#endif /* _layout_2d_c_ */

/*
 * グローバル変数定義
 */
#undef EXT
/* ------------------------------------------------------------------------- */
#ifdef _layout_2d_c_
/*
 * 局所的に使用するマクロ定義など
 */

#endif /* _layout_2d_c_ */
/* ------------------------------------------------------------------------- */
/*
 * プロトタイプ宣言
 */

/* レイアウトのロード */
int       L2D_LoadLayout(int strcode, int chanl, int base_pri, int add_flag);
int       L2D_SetupLayout(void * entry_ptr, int chanl,
			  int base_pri, int add_flag, float safeZoneOffsetY);

int       L2D_LoadLayout2(int strcode, int chanl,
			  int base_pri, int add_flag, int pause_level);
int       L2D_SetupLayout2(void * entry_ptr, int chanl, int base_pri, int add_flag, int pause_level, float safeZoneOffsetY);

int       L2D_ReleaseLayout(int handle);            /* レイアウトの開放     */
SPR_OBJ * L2D_GetObject(int handle, int strcode);   /* オブジェクトの取得   */
void    * L2D_GetParts(int handle, int strcode);    /* パーツポインタの取得 */
int       L2D_MorfObject(void * parts, int code1, int code2, float rate);

/* シグナルハンドラの設定 */
int       L2D_SetSignalHandle(int handle, void * workp,
			      void (*func)(void * workp, int sign, int value));

/* シグナルハンドラの取得 */
void    (* L2D_GetSignalHandle(int handle))(void * workp, int sign, int value);

/* 再生速度係数の設定 */
int       L2D_SetActionPlaySpeed(int handle, int speed);

/* アクション数の取得   */
int       L2D_GetActionNumber(int handle);

/* アクションの強制停止 */
int       L2D_BreakAction(int handle);

/* アクションの一時停止 */
int       L2D_PauseAction(int handle, L2D_PAUSE_STAT pause);
int       L2D_PauseAllAction(L2D_PAUSE_STAT pause);

/* アクションの起動     */
int       L2D_EvokeAction(int handle, int strcode);
int       L2D_EvokeActionByNumber(int handle, int actnum);

/*
 * その時点におけるレイアウトの、アクション再生状態を得る。
 * 戻り値は,以下の値をとる。
 *
 * L2D_STAT_BAD_HANDLE    渡されたハンドルの値が不正
 * L2D_STAT_BUSY          アクション再生中
 * L2D_STAT_UNLOAD        そのハンドルにはレイアウトがロードされていない
 * L2D_STAT_NOT_ENOUGH    レイアウトマネージャが起動されていない
 * L2D_STAT_ACK           待機中
 */
 
int       L2D_ActionStatus(int handle);

void * NewLayout2D_Player(int name, int where);  /* Actor 起動関数(シナリオ) */
void * NewLayout2D_Player_for_Prog(void);

#endif /* _layout_2d_h_ */
