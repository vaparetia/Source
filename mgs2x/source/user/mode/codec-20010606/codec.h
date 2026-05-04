/*
	codec.h
		無線機モードヘッダファイル
	2000/07/17	K.Uehara
	$Id: codec.h,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/

/*
	解釈部は別スレッドとして起動。
	表示部は解釈部からのリクエストを受け取る。
*/

typedef struct {
/* 解釈部から表示部へのリクエスト */
	int request;
	int req[ 3 ];
/* 表示部から解釈部へのリザルト */
	int result;
	int res[ 3 ];

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
	CODEC_REQ_FACE_LOAD,    // 顔モデルロード中
	CODEC_REQ_FACE_SET,     // 顔モデルの設定
	CODEC_REQ_HAIR_SET,     // 髪の毛データのロード、設定
	CODEC_REQ_DEF_MTN_SET,  // デフォルトモーションの設定
	CODEC_REQ_DISP_CTRL,    // 顔画面表示許可/禁止

	CODEC_REQ_END = 255	// 完全に終了した
};

void CODEC_InitTable( void );
void CODEC_ClearTable( void );
void CODEC_SetBaseCall( int freq, int code );
void CODEC_SetOverCall( int freq, int code );
int CODEC_GetCode( int freq );

int CODEC_ItpInit( CODEC_ITP_PARAM *param );
