/*
	codec.h
		無線機モードヘッダファイル
	2000/07/17	K.Uehara
	$Id: codec.h,v 1.1.1.3 2002/11/19 11:45:00 Yoshizawa1 Exp $
*/

/*
	解釈部は別スレッドとして起動。
	表示部は解釈部からのリクエストを受け取る。
*/

#include <stdlib.h>

typedef struct {
/* 解釈部から表示部へのリクエスト */
	int request;
	int req[ 4 ];
/* 表示部から解釈部へのリザルト */
	int result;
	int res[ 4 ];

	/* 解釈スレッドの情報 */
	void *data;
	void *data_buffer;
	void *stack;
	int stacksize;
	int id;
} CODEC_ITP_PARAM;

enum {
	CODEC_REQ_SETUP	= -1,	// 初期化中
	CODEC_REQ_NOREQ	= 0,	// なんらかの処理中
	CODEC_REQ_MESG_WAIT,	// メッセージ表示後,入力待ち
	CODEC_REQ_VOX_START,	// 音声再生開始
	CODEC_REQ_MOVIE_START,  // 動画の開始
	CODEC_REQ_FACE_LOAD,    // 顔モデルロード中
	CODEC_REQ_FACE_SET,     // 顔モデルの設定
	CODEC_REQ_HAIR_SET,     // 髪の毛データのロード、設定
	CODEC_REQ_OBJECT_SET,   // 付属物追加
	CODEC_REQ_DEF_MTN_SET,  // デフォルトモーションの設定
	CODEC_REQ_DISP_CTRL,    // 顔画面表示許可/禁止
	CODEC_REQ_SET_PLAYER,   // プレイヤ名登録
	CODEC_REQ_MIND_MASK,    // うなづき禁止/許可
	CODEC_REQ_SELECT,       // 項目選択
	CODEC_REQ_SAVE_MODE,    // セーブ画面起動
	CODEC_REQ_CONT_CALL,    // 終了後継続コール予約
	CODEC_REQ_FREQ_SET,     // 周波数変更
	CODEC_REQ_TIME_WAIT,    // 時間待ち
	CODEC_REQ_EARTH_Q,      // 地震
	CODEC_REQ_VIBRATION,    // コントローラ振動
	CODEC_REQ_VIBPLAY,      // コントローラ振動(2)
	CODEC_REQ_BUG_FACE,     // バグり顔
	CODEC_REQ_NOISE,        // 任意ノイズ ON/OFF
	CODEC_REQ_BUGEYES,      // 白目むく
	CODEC_REQ_END = 255	// 完全に終了した
};

#define CallACT_CallStart  0x00b1b691
#define CallACT_Calling    0x00f7c3af


void CODEC_CallIconSwitch(int sw);
void CODEC_Block(int sw);
void CODEC_InitTable( void );
void CODEC_ClearTable( void );
void CODEC_SetBaseCall( int freq, int code );
void CODEC_SetOverCall( int freq, int code );
int  CODEC_GetCode( int freq );

int  CODEC_ItpInit( CODEC_ITP_PARAM *param );

int codecGetLastResponsedFreq(void);
int codecGetResponseCount(void);

static inline unsigned int  _x_rand()
{
	unsigned int r;
	r = rand();
	return r | ( r << 16 );
}

#define rand _x_rand

